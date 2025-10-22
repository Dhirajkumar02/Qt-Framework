#include "mainwindow.h"

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QDebug>
#include <QProgressBar>

BinaryFileReader::BinaryFileReader(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Binary File Reader");
    resize(700, 500);

    // --- UI Elements ---
    openFileButton = new QPushButton("Open File");
    fileNameEdit = new QLineEdit;
    fileNameEdit->setPlaceholderText("Select a .bin file");

    skipLineEdit = new QLineEdit;
    skipLineEdit->setPlaceholderText("Skip by %");
    skipLineEdit->setMaximumWidth(100);

    skipButton = new QPushButton("Skip");
    skipButton ->setEnabled(false);

    processButton = new QPushButton("Process File");
    processButton->setFixedWidth(120);
    processButton->setEnabled(false);

    progressBar = new QProgressBar;
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setFixedWidth(200);

    table = new QTableWidget;
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"Time", "MsgId", "Size"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // --- Layouts ---
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(openFileButton);
    topLayout->addWidget(fileNameEdit);
    topLayout->addWidget(skipLineEdit);
    topLayout->addWidget(skipButton);

    QHBoxLayout *processLayout = new QHBoxLayout;
    processLayout->addWidget(processButton);
    processLayout->addWidget(progressBar);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(processLayout);
    mainLayout->addWidget(table);

    // --- Connections ---
    connect(openFileButton, &QPushButton::clicked, this, &BinaryFileReader::openFile);
    connect(skipButton, &QPushButton::clicked, this, &BinaryFileReader::skipPercentage);
    connect(processButton, &QPushButton::clicked, this, &BinaryFileReader::processFile);

    connect(fileNameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        processButton->setEnabled(!text.isEmpty());
    });

    // Enable skip button only when valid percentage entered
    connect(skipLineEdit, &QLineEdit::textChanged, this, [this](const QString &text){
        bool ok;
        int percent = text.toInt(&ok);
        skipButton->setEnabled(ok && percent >= 0 && percent < 100);
    });

}

BinaryFileReader::~BinaryFileReader() {}

// --- Browse File ---
void BinaryFileReader::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*)");
    if (filePath.isEmpty())
        return;

    selectedFilePath = filePath;
    fileNameEdit->setText(filePath);
    processButton->setEnabled(true);
    openFileButton->setStyleSheet("background-color: lightgreen;");
}

// --- Skip Percentage ---
void BinaryFileReader::skipPercentage()
{
    bool ok;
    int percent = skipLineEdit->text().toInt(&ok);
    if (ok && percent >= 0 && percent < 100) {
        skipPercent = percent;
        // No MessageBox needed
        qDebug() << "Skip percent set to:" << skipPercent;
    } else {
        skipPercent = 0;
        qDebug() << "Invalid input in skip percentage, defaulting to 0";
    }
}


// --- Process File ---
void BinaryFileReader::processFile()
{
    if (selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No file selected.");
        return;
    }

    QFile file(selectedFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open file!");
        return;
    }

    table->setRowCount(0);

    quint64 totalSize = file.size();
    quint64 skipBytes = (skipPercent * totalSize) / 100;

    // Skip the initial % of the file
    if (!file.seek(skipBytes)) {
        QMessageBox::critical(this, "Error", "Failed to skip bytes in file!");
        file.close();
        return;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    quint64 bytesReadTotal = skipBytes; // Start progress from skipped bytes
    quint64 lineCounter = 0;
    const int maxDisplayRows = 1000; // Limit table rows for huge files

    while (!in.atEnd()) {
        quint32 headMsgID;
        in >> headMsgID;
        if (in.status() != QDataStream::Ok)
            break;

        if (headMsgID == 0xEEEEEEEE) {
            quint32 time;
            quint16 msgId;
            quint32 size;

            in >> time >> msgId >> size;
            if (in.status() != QDataStream::Ok)
                break;

            if (table->rowCount() < maxDisplayRows) {
                int row = table->rowCount();
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(QString::number(time)));
                table->setItem(row, 1, new QTableWidgetItem(QString("0x%1").arg(msgId, 4, 16, QLatin1Char('0')).toUpper()));
                table->setItem(row, 2, new QTableWidgetItem(QString::number(size)));
            }

            lineCounter++;
        } else {
            qint64 posAfter = in.device()->pos();
            if (posAfter < 3)
                break;
            in.device()->seek(posAfter - 3);
        }

        // Update progress bar based on **total file size**
        bytesReadTotal = in.device()->pos();
        int progressPercent = static_cast<int>((bytesReadTotal * 100) / totalSize);
        if (progressPercent > 100) progressPercent = 100;
        progressBar->setValue(progressPercent);
        qApp->processEvents();
    }

    progressBar->setValue(100);
    file.close();

    QMessageBox::information(this, "Processing Complete",
                             QString("Skipped %1% (%2 bytes).\nRead remaining %3 bytes successfully.")
                                 .arg(skipPercent)
                                 .arg(skipBytes)
                                 .arg(totalSize - skipBytes));

    qDebug() << "Processed lines:" << lineCounter;
    qDebug() << "Skipped first" << skipPercent << "% (" << skipBytes << "bytes)";
}
