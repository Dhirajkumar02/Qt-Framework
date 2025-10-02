#include "mainwindow.h"
#include <QApplication>

// Entry point of the Qt application
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.setWindowTitle("Square Matrix with Click Info");
    w.resize(800, 600); // set window size
    w.show();

    return app.exec();
}
