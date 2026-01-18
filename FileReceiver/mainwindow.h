#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>

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
    void onNewConnection();
    void receiveFile();

private:
    Ui::MainWindow *ui;

    QTcpServer *server;
    QTcpSocket *socket;

    QFile file;
    QString fileName;
    qint64 fileSize;
    qint64 receivedBytes;
};

#endif // MAINWINDOW_H
