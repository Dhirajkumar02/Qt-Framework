#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    textEdit = new QTextEdit(this);
    QPushButton *openBtn = new QPushButton("Open Binary File", this);
    QPushButton *saveBtn = new QPushButton("Save Binary File", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(textEdit);
    layout->addWidget(openBtn);
    layout->addWidget(saveBtn);
    setLayout(layout);

    setWindowTitle("Binary File Read & Write ");

    connect(openBtn, &QPushButton::clicked, this, &MainWindow::openBinaryFile);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveBinaryFile);
}

void MainWindow::saveBinaryFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Binary File", "", "Binary Files (*.bin);;All Files (*.*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Error", "Cannot save file!");
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_6_0);

    // Example: save a number, a float, and the text content
    int number = 42;
    double pi = 3.14159;
    QString text = textEdit->toPlainText();

    out << number << pi << text;
    file.close();

    QMessageBox::information(this, "Saved", "Binary file saved successfully!");
}

void MainWindow::openBinaryFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*.*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Cannot open file!");
        return;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_0);

    int number;
    double pi;
    QString text;

    in >> number >> pi >> text;
    file.close();

    // Show content in text box
    textEdit->setPlainText(QString("Number: %1\nPi: %2\nText: %3")
                               .arg(number)
                               .arg(pi)
                               .arg(text));
}

