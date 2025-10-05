#include "mainwindow.h"
#include <QPainter>
#include <QMessageBox>
#include <random>

// Constructor for MainWindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , rows(60), cols(360)   // Initialize grid size (60 rows × 360 columns)
{
    // Random number generator for filling zValues
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);  // Random values in [1, 100]

    // Resize zValues to create a 2D matrix (rows × cols)
    zValues.resize(rows);

    for (int i = 0; i < rows; ++i) {
        zValues[i].resize(cols);
        for (int j = 0; j < cols; ++j) {
            // Assign random integer (1–100) to each cell
            zValues[i][j] = dis(gen);
        }
    }
}

// Destructor
MainWindow::~MainWindow(){}


// Paint grid of squares
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);   // Avoid unused parameter warning
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // Smooth drawing

    int w = width();   // Current window width
    int h = height();  // Current window height

    // Calculate cell size (so that all rows/columns fit the window)
    double cellWidth = double(w) / cols;
    double cellHeight = double(h) / rows;

    // Draw all cells in the grid
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Rectangle for each cell (x, y, width, height)
            QRectF rect(j * cellWidth, i * cellHeight, cellWidth, cellHeight);

            int z = zValues[i][j];       // Value stored in this cell
            double intensity = z / 100.0; // Normalize to [0,1] (for brightness)

            QColor color;
            // Choose color based on z value
            if (z < 40) {
                // Low values → green shades
                color = QColor::fromRgbF(0.0, intensity, 0.0);
            }
            else if (z < 70) {
                // Medium values → yellow shades (Red + Green)
                color = QColor::fromRgbF(intensity, intensity, 0.0);
            }
            else {
                // High values → red shades
                color = QColor::fromRgbF(intensity, 0.0, 0.0);
            }

            // Apply brush color and thin border for each cell
            painter.setBrush(color);
            painter.setPen(QPen(Qt::black, 0.3));
            painter.drawRect(rect);
        }
    }
}


// Handle mouse clicks (detect which cell was clicked)
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    int w = width();   // Window width
    int h = height();  // Window height

    // Calculate cell width & height
    double cellW = double(w) / cols;
    double cellH = double(h) / rows;

    // Find clicked cell index (j = col, i = row)
    int j = event->pos().x() / cellW;  // Column index
    int i = event->pos().y() / cellH;  // Row index

    // Check if click is inside valid grid bounds
    if (i >= 0 && i < rows && j >= 0 && j < cols) {
        int z = zValues[i][j];  // Get Z value at clicked cell

        // Prepare message with X, Y, Z info
        QString msg = QString("X = %1 \nY = %2 \nZ = %3")
                          .arg(j) // Column = X
                          .arg(i) // Row = Y
                          .arg(z);

        // Show cell info in popup
        QMessageBox::information(this, "Cell Info", msg);
    }
}
