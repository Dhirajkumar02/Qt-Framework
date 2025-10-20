#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QtEndian>

BinaryFileReader::BinaryFileReader(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Binary File Reader");
    resize(800, 500);

    // --- UI Elements ---
    openButton = new QPushButton("Browse File");
    processButton = new QPushButton("Process File");
    processButton->setFixedWidth(120);
    processButton->setEnabled(false);

    cancelButton = new QPushButton("Cancel");
    cancelButton->setEnabled(false);
    cancelButton->setFixedWidth(80);

    fileNameEdit = new QLineEdit;
    fileNameEdit->setPlaceholderText("Select a .bin file");

    // --- Skip number input ---
    skipNumberInput = new QSpinBox;
    skipNumberInput->setRange(0, 0xFFFFFFFF);
    skipNumberInput->setPrefix("Skip MsgId: ");
    skipNumberInput->setFixedWidth(150);

    skipButton = new QPushButton("Skip");
    skipButton->setFixedWidth(60);

    progressBar = new QProgressBar;
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setFixedWidth(200);

    statusLabel = new QLabel("Idle");

    table = new QTableWidget;
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Time", "MsgId"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // --- Layouts ---
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(openButton);
    topLayout->addWidget(fileNameEdit);
    topLayout->addWidget(skipNumberInput);
    topLayout->addWidget(skipButton);

    QHBoxLayout *processLayout = new QHBoxLayout;
    processLayout->addWidget(processButton);
    processLayout->addWidget(cancelButton);
    processLayout->addWidget(progressBar);
    processLayout->addWidget(statusLabel);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(processLayout);
    mainLayout->addWidget(table);

    // --- Connections ---
    connect(openButton, &QPushButton::clicked, this, &BinaryFileReader::browseFile);
    connect(processButton, &QPushButton::clicked, this, &BinaryFileReader::processFile);
    connect(cancelButton, &QPushButton::clicked, this, [this]() {
        cancelRequested = true;
        cancelButton->setEnabled(false);
        statusLabel->setText("Cancelling...");
    });
    connect(skipButton, &QPushButton::clicked, this, [this]() {
        skipMsgId = static_cast<quint32>(skipNumberInput->value());
        QMessageBox::information(this, "Skip Set",
                                 QString("Will skip MsgId: 0x%1")
                                     .arg(skipMsgId, 4, 16, QLatin1Char('0')).toUpper());
    });
    connect(fileNameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        processButton->setEnabled(!text.isEmpty());
    });
}

// --- Browse File ---
void BinaryFileReader::browseFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*)");
    if (!filePath.isEmpty()) {
        fileNameEdit->setText(filePath);
        selectedFilePath = filePath;
        openButton->setStyleSheet("background-color: lightgreen;");
        processButton->setEnabled(true);
    }
}

// --- Update Progress Bar Color ---
void BinaryFileReader::updateProgressBarColor(int value) {
    QString color;
    if (value < 70)
        color = "#4CAF50";  // Green
    else if (value < 90)
        color = "#FFC107";  // Amber
    else
        color = "#F44336";  // Red

    progressBar->setStyleSheet(QString(
                                   "QProgressBar {"
                                   "  border: 1px solid gray;"
                                   "  border-radius: 5px;"
                                   "  text-align: center;"
                                   "  height: 12px;"
                                   "}"
                                   "QProgressBar::chunk {"
                                   "  background-color: %1;"
                                   "  width: 1px;"
                                   "}"
                                   ).arg(color));
}

// --- Process File ---
void BinaryFileReader::processFile() {
    if (selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No file selected!");
        return;
    }

    QFile file(selectedFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open file!");
        return;
    }

    qint64 totalSize = file.size();
    qint64 bytesRead = 0;
    quint64 counter = 0;
    const qint64 chunkSize = 5 * 1024 * 1024; // 5 MB
    QByteArray leftover;

    cancelRequested = false;
    cancelButton->setEnabled(true);
    processButton->setEnabled(false);
    progressBar->setValue(0);
    statusLabel->setText("Processing...");
    table->setRowCount(0);

    while (!file.atEnd()) {
        if (cancelRequested) {
            statusLabel->setText("Cancelled ❌");
            break;
        }

        QByteArray chunk = leftover + file.read(chunkSize);
        leftover.clear();
        bytesRead += chunk.size();

        const uchar *ptr = reinterpret_cast<const uchar*>(chunk.constData());
        const uchar *end = ptr + chunk.size();

        while (ptr + 12 <= end) {
            if (cancelRequested) break;

            quint32 headMsgID = qFromLittleEndian<quint32>(ptr);
            // Instead of
           // quint32 value = qFromLittleEndian<quint32>(ptr);

            // Use this in Qt 5.8
            //quint32 value = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);

            if (headMsgID == 0xEEEEEEEE) {
                quint32 time = qFromLittleEndian<quint32>(ptr + 4);
                quint32 msgId = qFromLittleEndian<quint32>(ptr + 8);

                // --- Skip logic ---
                if (msgId == skipMsgId) {
                    ptr += 12;
                    continue;
                }

                int row = table->rowCount();
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(QString::number(time)));
                table->setItem(row, 1, new QTableWidgetItem(
                                           QString("0x%1").arg(msgId, 4, 16, QLatin1Char('0')).toUpper()
                                           ));

                ptr += 12;
                counter++;
            } else {
                ptr++;
            }
        }

        qint64 remaining = end - ptr;
        if (remaining > 0)
            leftover = QByteArray(reinterpret_cast<const char*>(ptr), remaining);

        // --- Update Progress ---
        int percent = static_cast<int>((bytesRead * 100.0) / totalSize);
        progressBar->setValue(percent);
        updateProgressBarColor(percent);
        statusLabel->setText(QString("%1 MB / %2 MB (%3%)")
                                 .arg(bytesRead / (1024 * 1024))
                                 .arg(totalSize / (1024 * 1024))
                                 .arg(percent));

        qApp->processEvents();  // Keep UI responsive
    }

    file.close();
    cancelButton->setEnabled(false);
    processButton->setEnabled(true);

    if (!cancelRequested) {
        progressBar->setValue(100);
        updateProgressBarColor(100);
        statusLabel->setText("Completed ✔️");
        QMessageBox::information(this, "Completed",
                                 QString("Processed %1 messages successfully!").arg(counter));
    } else {
        QMessageBox::information(this, "Cancelled",
                                 QString("Stopped after reading %1 MB.").arg(bytesRead / (1024 * 1024)));
    }

    qDebug() << "Processed messages:" << counter;
}
