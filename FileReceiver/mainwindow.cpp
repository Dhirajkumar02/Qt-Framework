#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDataStream>
#include <QDebug>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    server = new QTcpServer(this);
    socket = nullptr;

    fileSize = 0;
    receivedBytes = 0;

    ui->progressBar->setValue(0);

    connect(ui->btnStartServer, &QPushButton::clicked, this, &MainWindow::startServer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startServer()
{
    int port = ui->portEdit->text().toInt();

    if(server->listen(QHostAddress::Any, port))
    {
        ui->textEditLog->append("✅ Server started on port: " + QString::number(port));
        connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);
    }
    else
    {
        ui->textEditLog->append("❌ Server failed to start!");
    }
}

void MainWindow::onNewConnection()
{
    socket = server->nextPendingConnection();
    ui->textEditLog->append("✅ Client connected!");

    fileSize = 0;
    receivedBytes = 0;
    fileName = "";

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::receiveFile);
}

void MainWindow::receiveFile()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);

    // Receive filename + filesize only once
    if(fileSize == 0)
    {
        if(socket->bytesAvailable() < (int)sizeof(qint64))
            return;

        in >> fileName;
        in >> fileSize;

        ui->textEditLog->append("📁 File Name: " + fileName);
        ui->textEditLog->append("📦 File Size: " + QString::number(fileSize) + " bytes");

        // Save to Desktop or current folder
        QString savePath = QDir::homePath() + "/Desktop/Received_" + fileName;
        file.setFileName(savePath);

        if(!file.open(QIODevice::WriteOnly))
        {
            ui->textEditLog->append("❌ Cannot open file for writing!");
            return;
        }

        ui->progressBar->setValue(0);
    }

    QByteArray data = socket->readAll();
    file.write(data);
    receivedBytes += data.size();

    int progress = (int)((receivedBytes * 100) / fileSize);
    ui->progressBar->setValue(progress);

    ui->lblStatus->setText("Receiving: " + QString::number(receivedBytes) +
                           " / " + QString::number(fileSize));

    if(receivedBytes >= fileSize)
    {
        file.close();
        ui->textEditLog->append("✅ File Received & Saved to Desktop!");
        ui->lblStatus->setText("✅ Completed!");
        socket->disconnectFromHost();
    }
}
