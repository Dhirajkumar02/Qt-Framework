#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QDataStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    udpSocket = new QUdpSocket(this);

    ui->progressBar->setValue(0);

    connect(ui->btnBrowse, &QPushButton::clicked, this, &MainWindow::browseFile);
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::sendFileUdp);
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

void MainWindow::sendFileUdp()
{
    QString ip = ui->ipEdit->text();
    int port = ui->portEdit->text().toInt();

    if(filePath.isEmpty())
    {
        ui->textEditLog->append("❌ Select file first!");
        return;
    }

    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly))
    {
        ui->textEditLog->append("❌ Cannot open file!");
        return;
    }

    QString fileName = QFileInfo(file).fileName();
    qint64 fileSize = file.size();

    int chunkSize = 1024;   // UDP safe size
    int totalPackets = (int)((fileSize + chunkSize - 1) / chunkSize);

    ui->textEditLog->append("📤 Sending: " + fileName);
    ui->textEditLog->append("📦 Size: " + QString::number(fileSize));
    ui->textEditLog->append("📦 Packets: " + QString::number(totalPackets));

    // ✅ META datagram
    QByteArray metaDatagram;
    QDataStream metaOut(&metaDatagram, QIODevice::WriteOnly);
    metaOut.setVersion(QDataStream::Qt_5_15);

    metaOut << QString("META") << fileName << fileSize << totalPackets;

    udpSocket->writeDatagram(metaDatagram, QHostAddress(ip), port);

    int sentPackets = 0;

    // ✅ DATA datagrams
    for(int i = 0; i < totalPackets; i++)
    {
        QByteArray chunk = file.read(chunkSize);

        QByteArray dataDatagram;
        QDataStream out(&dataDatagram, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_15);

        out << QString("DATA") << i << chunk;

        udpSocket->writeDatagram(dataDatagram, QHostAddress(ip), port);

        sentPackets++;
        int progress = (sentPackets * 100) / totalPackets;
        ui->progressBar->setValue(progress);
    }

    // ✅ END datagram
    QByteArray endDatagram;
    QDataStream endOut(&endDatagram, QIODevice::WriteOnly);
    endOut.setVersion(QDataStream::Qt_5_15);
    endOut << QString("END");

    udpSocket->writeDatagram(endDatagram, QHostAddress(ip), port);

    ui->textEditLog->append("✅ File Sent Successfully (UDP)!");
    ui->progressBar->setValue(100);
}
