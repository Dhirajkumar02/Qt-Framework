#include "mainwindow.h"
#include <QPainter>
#include <QMessageBox>
#include <random>

// Helper: Map normalized value (0–1) to a heatmap color
QColor getColorFromValue(double value)
{
    // clamp value between 0 and 1
    value = std::max(0.0, std::min(1.0, value));

    if (value < 0.33) {
        // 0.0–0.33: green (0,255,0) → yellow (255,255,0)
        double t = value / 0.33;
        return QColor(int(0 + t*255), 255, 0);
    }
    else if (value < 0.66) {
        // 0.33–0.66: yellow (255,255,0) → orange (255,128,0)
        double t = (value - 0.33) / 0.33;
        return QColor(255, int(255 - t*127), 0);
    }
    else {
        // 0.66–1.0: orange (255,128,0) → red (255,0,0)
        double t = (value - 0.66) / 0.34;
        return QColor(255, int(128 - t*128), 0);
    }
}

// Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), rows(360), cols(120) // grid size
{
    // Fill Z matrix with random values (1 to 1000)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);

    zValues.resize(rows);
    for (int i = 0; i < rows; ++i) {
        zValues[i].resize(cols);
        for (int j = 0; j < cols; ++j) {
            zValues[i][j] = dis(gen); // random value between 1 and 1000
        }
    }
}

MainWindow::~MainWindow() {}

// Draw grid of squares with heatmap colors
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    double cellW = double(w) / cols; // cell width
    double cellH = double(h) / rows; // cell height

    // Draw each cell
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QRectF rect(j * cellW, i * cellH, cellW, cellH);

            // Normalize value (1–1000 → 0–1)
            double value = double(zValues[i][j]) / 1000.0;

            // Get heatmap color
            QColor color = getColorFromValue(value);

            painter.setBrush(color);
            painter.setPen(QPen(Qt::black, 0.1)); // thin grid lines
            painter.drawRect(rect);
        }
    }
}

// Detect which cell was clicked
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    int w = width();
    int h = height();

    double cellW = double(w) / cols;
    double cellH = double(h) / rows;

    int j = event->pos().x() / cellW; // column index
    int i = event->pos().y() / cellH; // row index

    if (i >= 0 && i < rows && j >= 0 && j < cols) {
        int z = zValues[i][j];

        // Map indices to real-world angles
        double xAngle = (360.0 / cols) * j; // 0 to 360
        double yAngle = (60.0 / rows) * i;  // 0 to 60

        QString msg = QString("\nX = %1°\nY = %2°\nZ = %3")
                          .arg(xAngle, 0, 'f', 2)
                          .arg(yAngle, 0, 'f', 2)
                          .arg(z);

        QMessageBox::information(this, "Cell Info", msg);
    }
}
