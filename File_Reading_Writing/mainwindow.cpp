#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Select Input File
void MainWindow::on_btnSelectInput_clicked()
{
    inputFileName = QFileDialog::getOpenFileName(this, "Select Input File");
    if (!inputFileName.isEmpty()) {
        ui->lineInput->setText(inputFileName);
    }
}

// Select Output File
void MainWindow::on_btnSelectOutput_clicked()
{
    outputFileName = QFileDialog::getSaveFileName(this, "Select Output File");
    if (!outputFileName.isEmpty()) {
        ui->lineOutput->setText(outputFileName);
    }
}

// Copy File Character by Character
void MainWindow::on_btnCopy_clicked()
{
    if (inputFileName.isEmpty() || outputFileName.isEmpty()) {
        QMessageBox::warning(this, "Error", "Select both input and output files!");
        return;
    }

    QFile inputFile(inputFileName);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open input file!");
        return;
    }

    QFile outputFile(outputFileName);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open output file!");
        inputFile.close();
        return;
    }

    qint64 fileSize = inputFile.size();
    const qint64 bufferSize = 1024;
    char buffer[bufferSize];
    qint64 totalRead = 0;

    while (!inputFile.atEnd()) {
        qint64 bytesRead = inputFile.read(buffer, bufferSize);
        if (bytesRead > 0) {
            outputFile.write(buffer, bytesRead);
            totalRead += bytesRead;
            int progress = static_cast<int>((totalRead * 100) / fileSize);
            ui->progressBar->setValue(progress);
            qApp->processEvents(); // Keep UI responsive
        }
    }

    inputFile.close();
    outputFile.close();

    ui->progressBar->setValue(100);
    QMessageBox::information(this, "Success", "File copied successfully!");
}
