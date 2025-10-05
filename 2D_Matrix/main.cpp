#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("Square Matrix with click Info");
    w.resize(1800, 300);
    w.show();
    return a.exec();
}
