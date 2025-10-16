#include "mainwindow.h"
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QDebug>

BinaryFileReader::BinaryFileReader(QWidget *parent) : QWidget(parent)
{
setWindowTitle("Binary File Reader");
resize(600, 500);

// --- UI Elements ---
openButton = new QPushButton("Browse File");
processButton = new QPushButton("Process File");
fileNameEdit = new QLineEdit;
fileNameEdit->setPlaceholderText("Select a .bin file");

table = new QTableWidget;
table->setColumnCount(2);
table->setHorizontalHeaderLabels({"Time", "MsgId"});
table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

// In your constructor or setup function
processButton->setEnabled(false);  // Initially disabled

// --- Layout ---
QHBoxLayout *topLayout = new QHBoxLayout;
topLayout->addWidget(openButton);
topLayout->addWidget(fileNameEdit);

QVBoxLayout *mainLayout = new QVBoxLayout(this);
mainLayout->addLayout(topLayout);
mainLayout->addWidget(processButton);
mainLayout->addWidget(table);

// --- Connections ---
connect(openButton, &QPushButton::clicked, this, &BinaryFileReader::browseFile);
connect(processButton, &QPushButton::clicked, this, &BinaryFileReader::processFile);
}

void BinaryFileReader::browseFile() {
QString filePath = QFileDialog::getOpenFileName(this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*)");
    if (!filePath.isEmpty()) {
        fileNameEdit->setText(filePath);
        selectedFilePath = filePath;
        openButton->setStyleSheet("background-color: lightgreen;");
    }
}


// Slot for "Open File" button
void BinaryFileReader::onOpenFileButtonClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*)");
    if (!filePath.isEmpty()) {
        selectedFilePath = filePath;
        processButton->setEnabled(true);   // Enable "Process File"
    } else {
        selectedFilePath.clear();
        processButton->setEnabled(false);  // Disable if no file selected
    }
}

// Slot for "Process File" button
void BinaryFileReader::processFile() {
    if (selectedFilePath.isEmpty()) {
        return;  // Button should be disabled anyway
    }

    QFile file(selectedFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open file!");
        return;
    }

    table->setRowCount(0);  // Clear table
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    quint32 headMsgID;
    quint32 time;
    quint32 msgId;
    quint32 counter = 0;

    while (!in.atEnd()) {
        // Read 4 bytes for header
        if (file.bytesAvailable() < sizeof(quint32))
            break;

        in >> headMsgID;

        // Debug: print what we read
        qDebug() << QString("Read headMsgID: 0x%1").arg(headMsgID, 8, 16, QLatin1Char('0')).toUpper();

        if (headMsgID == 0xEEEEEEEE) {
            // Make sure enough bytes are available
            if (file.bytesAvailable() < 2 * sizeof(quint32))
                break;

            in >> time;
            in >> msgId;

            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(time)));
            table->setItem(row, 1, new QTableWidgetItem(QString("0x%1").arg(msgId, 8, 16, QLatin1Char('0')).toUpper()));

            counter++;
        } else {
            // Move forward by one byte to resync
            file.seek(file.pos() - 3);
        }
    }

    file.close();
    qDebug() << "Total Messages Processed:" << counter;
    QMessageBox::information(this, "Done", QString("Processed %1 messages!").arg(counter));
}
