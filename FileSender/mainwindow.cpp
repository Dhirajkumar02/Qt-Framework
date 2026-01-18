#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDataStream>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);

    ui->progressBar->setValue(0);

    connect(ui->btnBrowse, &QPushButton::clicked, this, &MainWindow::browseFile);
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::sendFile);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::browseFile()
{
    filePath = QFileDialog::getOpenFileName(this, "Select File");

    if(filePath.isEmpty())
        return;

    ui->filePathEdit->setText(filePath);
    ui->textEditLog->append("✅ Selected: " + filePath);
}

void MainWindow::sendFile()
{
    QString ip = ui->ipEdit->text();
    int port = ui->portEdit->text().toInt();

    if(filePath.isEmpty())
    {
        ui->textEditLog->append("❌ Please select a file first!");
        return;
    }

    file.setFileName(filePath);

    if(!file.open(QIODevice::ReadOnly))
    {
        ui->textEditLog->append("❌ Cannot open file!");
        return;
    }

    socket->connectToHost(ip, port);

    if(!socket->waitForConnected(5000))
    {
        ui->textEditLog->append("❌ Connection failed!");
        file.close();
        return;
    }

    ui->textEditLog->append("✅ Connected to server!");

    QString fileName = QFileInfo(file).fileName();
    qint64 fileSize = file.size();

    // Send filename + filesize
    QDataStream out(socket);
    out.setVersion(QDataStream::Qt_5_15);

    out << fileName;
    out << fileSize;

    qint64 sentBytes = 0;

    while(!file.atEnd())
    {
        QByteArray buffer = file.read(4096);
        sentBytes += socket->write(buffer);
        socket->waitForBytesWritten();

        int progress = (int)((sentBytes * 100) / fileSize);
        ui->progressBar->setValue(progress);
    }

    ui->textEditLog->append("✅ File Sent Successfully!");
    file.close();
    socket->disconnectFromHost();
}
