#include "mainwindow.h"
#include <QPainter>
#include <QMessageBox>
#include <QInputDialog>
#include <random>

// ✅ Helper: Map normalized value (0–1) to heatmap color
QColor getColorFromValue(double value)
{
    value = std::max(0.0, std::min(1.0, value));

    if (value < 0.2) {
        // 0–20% → light green → dark green
        double t = value / 0.2;
        return QColor(int(144 - t*64), int(238 - t*138), int(144 - t*64));
    }
    else if (value < 0.4) {
        // 20–40% → dark green → yellow
        double t = (value - 0.2) / 0.2;
        return QColor(int(0 + t*255), int(100 + t*155), 0);
    }
    else if (value < 0.7) {
        // 40–70% → yellow → orange
        double t = (value - 0.4) / 0.3;
        return QColor(255, int(255 - t*127), 0);
    }
    else {
        // 70–100% → orange → red
        double t = (value - 0.7) / 0.3;
        return QColor(255, int(128 - t*128), 0);
    }
}

// ✅ Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // User input for rows and cols
    bool ok;
    rows = QInputDialog::getInt(this, "Rows", "Enter number of rows:", 100, 10, 2000, 1, &ok);
    if (!ok) rows = 100;

    cols = QInputDialog::getInt(this, "Columns", "Enter number of columns:", 100, 10, 2000, 1, &ok);
    if (!ok) cols = 100;

    resize(800, 600); // Initial window size

    // Fill Z matrix with random values (1 to 1000)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);

    zValues.resize(rows);
    for (int i = 0; i < rows; ++i) {
        zValues[i].resize(cols);
        for (int j = 0; j < cols; ++j) {
            zValues[i][j] = dis(gen);
        }
    }
}

MainWindow::~MainWindow() {}

// ✅ Draw grid of squares with heatmap colors
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    double cellW = double(w) / cols;
    double cellH = double(h) / rows;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QRectF rect(j * cellW, i * cellH, cellW, cellH);

            // Normalize (1–1000 → 0–1)
            double value = double(zValues[i][j] - 1) / 999.0;

            QColor color = getColorFromValue(value);

            painter.setBrush(color);
            painter.setPen(Qt::NoPen);  // faster, clean look
            painter.drawRect(rect);
        }
    }
}

// ✅ Detect clicked cell and show info
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

        // Map to real-world angles
        double xAngle = (360.0 / cols) * j;
        double yAngle = (60.0 / rows) * i;

        QString msg = QString("Row = %1\nCol = %2\nX = %3°\nY = %4°\nValue(Z) = %5")
                          .arg(i).arg(j)
                          .arg(xAngle, 0, 'f', 2)
                          .arg(yAngle, 0, 'f', 2)
                          .arg(z);

        QMessageBox::information(this, "Cell Info", msg);
    }
}
