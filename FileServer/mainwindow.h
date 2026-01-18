#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QFile>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startServer();
    void readPendingDatagrams();

private:
    Ui::MainWindow *ui;

    QUdpSocket *udpSocket;

    QString fileName;
    qint64 fileSize;
    int totalPackets;
    int receivedPackets;

    QFile file;
    QMap<int, QByteArray> packetBuffer;
};

#endif // MAINWINDOW_H
