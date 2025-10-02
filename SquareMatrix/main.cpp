#include "mainwindow.h"     // Include MainWindow class (our custom heatmap viewer)
#include <QApplication>     // Include QApplication (manages the Qt application lifecycle)

// ============================
// Function: main
// Entry point of the program
// ============================
int main(int argc, char *argv[])
{
    // Create a QApplication object
    // argc, argv allow Qt to handle command-line arguments (like style/theme options)
    QApplication app(argc, argv);

    // Create our main window (the heatmap viewer)
    MainWindow w;

    // Set the window title (appears on the window bar)
    w.setWindowTitle("Dynamic Heatmap Viewer");

    // Show the main window on the screen
    w.show();

    // Start the Qt event loop
    // This keeps the application running and responding to user events (mouse, paint, etc.)
    return app.exec();
}
