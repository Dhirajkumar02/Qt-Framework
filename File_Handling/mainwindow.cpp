#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    textEdit = new QTextEdit(this);
    QPushButton *selectBtn = new QPushButton("Select File", this);
    QPushButton *copyBtn = new QPushButton("Copy File", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);
    layout->addWidget(selectBtn);
    layout->addWidget(copyBtn);
    setLayout(layout);

    setWindowTitle("File ");

    // Initialize selected file as empty
    selectedFilePath = "";

    connect(selectBtn, &QPushButton::clicked, this, &MainWindow::selectFile);
    connect(copyBtn, &QPushButton::clicked, this, &MainWindow::copySelectedFile);
}

void MainWindow::selectFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select a File", "", "All Files (*.*)");
    if (fileName.isEmpty())
        return;

    selectedFilePath = fileName; // store selected file path

    // Display some info or content
    QFileInfo fileInfo(fileName);
    QString fileType = fileInfo.suffix().toLower();

    if (fileType == "txt") {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            textEdit->setPlainText(in.readAll());
            file.close();
        }
    } else {
        textEdit->setPlainText(QString("Selected file:\n%1").arg(fileName));
    }

    QMessageBox::information(this, "File Selected", "Selected file:\n" + fileName);
}

void MainWindow::copySelectedFile()
{
    if (selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, "No File", "Please select a file first!");
        return;
    }

    QFileInfo fileInfo(selectedFilePath);
    QString copyPath = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + "_copy." + fileInfo.suffix();

    if (QFile::exists(copyPath))
        QFile::remove(copyPath);

    if (QFile::copy(selectedFilePath, copyPath)) {
        QMessageBox::information(this, "Success", "File copied successfully!\n\n" + copyPath);
    } else {
        QMessageBox::warning(this, "Error", "Failed to copy file!");
    }
}
