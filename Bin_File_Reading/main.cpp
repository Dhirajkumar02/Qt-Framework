#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    BinaryFileReader br;
    br.show();
    return app.exec();
}
