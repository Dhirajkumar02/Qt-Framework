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
processButton -> setEnabled(false);

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
connect(fileNameEdit, &QLineEdit::textChanged, this, [this](const QString &text){
    processButton->setEnabled(!text.isEmpty());
});
}

void BinaryFileReader::browseFile() {
QString filePath = QFileDialog::getOpenFileName(this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*)");
    if (!filePath.isEmpty()) {
        fileNameEdit->setText(filePath);
        selectedFilePath = filePath;
        openButton->setStyleSheet("background-color: lightgreen;");
        processButton->setEnabled(true);
    }
}


void BinaryFileReader::disabledProcessFileButton() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*)");
    if (!filePath.isEmpty()) {
        fileNameEdit -> setText(filePath);
        processButton -> setEnabled(true);
    }


}

// Slot for "Process File" button
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

    table->setRowCount(0);  // Clear previous data

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    quint32 headMsgID;
    quint32 time;
    quint32 msgId;
    quint32 counter = 0;

    while (!in.atEnd()) {
        // Read 4 bytes as potential header
        in >> headMsgID;

        if (headMsgID == 0xEEEEEEEE) {
            // Read next 8 bytes (time + msgId)
            in >> time >> msgId;

            if (in.status() != QDataStream::Ok)
                break;

            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(time)));
            table->setItem(row, 1, new QTableWidgetItem(
                                       QString("0x%1").arg(msgId, 4, 16, QLatin1Char('0')).toUpper()
                                       ));

            counter++;
        } else {
            // Move back 3 bytes to shift by 1 byte forward
            file.seek(file.pos() - 3);
        }
    }

    file.close();
    qDebug() << "Processed messages:" << counter;

}
