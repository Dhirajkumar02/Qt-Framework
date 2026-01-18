#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDataStream>
#include <QFileInfo>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    udpSocket = new QUdpSocket(this);

    fileSize = 0;
    totalPackets = 0;
    receivedPackets = 0;

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

    if(udpSocket->bind(QHostAddress::Any, port))
    {
        ui->textEditLog->append("✅ UDP Server Started on port: " + QString::number(port));
        connect(udpSocket, &QUdpSocket::readyRead, this, &MainWindow::readPendingDatagrams);
    }
    else
    {
        ui->textEditLog->append("❌ Failed to bind UDP port!");
    }
}

void MainWindow::readPendingDatagrams()
{
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(int(udpSocket->pendingDatagramSize()));

        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QDataStream in(&datagram, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_5_15);

        QString type;
        in >> type;

        // 1) META packet
        if(type == "META")
        {
            packetBuffer.clear();

            in >> fileName;
            in >> fileSize;
            in >> totalPackets;

            receivedPackets = 0;

            ui->textEditLog->append("📁 Incoming File: " + fileName);
            ui->textEditLog->append("📦 File Size: " + QString::number(fileSize));
            ui->textEditLog->append("📦 Total Packets: " + QString::number(totalPackets));

            ui->progressBar->setValue(0);
            ui->lblStatus->setText("Receiving...");

            continue;
        }

        // 2) DATA packet
        if(type == "DATA")
        {
            int packetNo;
            QByteArray chunk;
            in >> packetNo;
            in >> chunk;

            if(!packetBuffer.contains(packetNo))
            {
                packetBuffer[packetNo] = chunk;
                receivedPackets++;
            }

            int progress = (receivedPackets * 100) / totalPackets;
            ui->progressBar->setValue(progress);

            ui->lblStatus->setText("Packets: " + QString::number(receivedPackets) +
                                   "/" + QString::number(totalPackets));

            continue;
        }

        // 3) END packet
        if(type == "END")
        {
            ui->textEditLog->append("✅ All packets received. Saving file...");

            QString savePath = QDir::homePath() + "/Desktop/UDP_Received_" + fileName;

            file.setFileName(savePath);

            if(!file.open(QIODevice::WriteOnly))
            {
                ui->textEditLog->append("❌ Cannot save file!");
                return;
            }

            for(int i = 0; i < totalPackets; i++)
            {
                file.write(packetBuffer[i]);
            }

            file.close();

            ui->textEditLog->append("✅ File Saved: " + savePath);
            ui->lblStatus->setText("✅ Completed!");
            ui->progressBar->setValue(100);
        }
    }
}
