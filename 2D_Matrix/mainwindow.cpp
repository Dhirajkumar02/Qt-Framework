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
    /*std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);*/  // Random values in [1, 100]

    // Resize zValues to create a 2D matrix (rows × cols)

    int x = 0;
    zValues.resize(rows);

    for (int i = 0; i < rows; ++i) {
        zValues[i].resize(cols);
        for (int j = 0; j < cols; ++j) {
            // Assign random integer (1–100) to each cell
            zValues[i][j] =x++;  // dis(gen);
            if(x > 100)
                x = 0;
        }
    }
}

// Destructor
MainWindow::~MainWindow(){}


// Paint grid of squares
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // --- Define your logical grid size ---
    int rowsToDraw = rows;   // 60
    int colsToDraw = cols;   // 360

    // --- Cell size (you can adjust scale) ---
    double cellWidth = 4;    // width of one cell in pixels
    double cellHeight = 4;   // height of one cell in pixels

    double gridWidth = colsToDraw * cellWidth;
    double gridHeight = rowsToDraw * cellHeight;

    // --- Center position ---
    double offsetX = (w - gridWidth) / 2.0;
    double offsetY = (h - gridHeight) / 2.0;

    // --- Draw background ---
    painter.fillRect(offsetX, offsetY, gridWidth, gridHeight, Qt::white);

    // --- Draw grid cells ---
    for (int i = 0; i < rowsToDraw; ++i) {
        for (int j = 0; j < colsToDraw; ++j) {
            int displayRow = rowsToDraw - 1 - i; // invert Y (bottom-up)
            QRectF rect(offsetX + j * cellWidth,
                        offsetY + displayRow * cellHeight,
                        cellWidth, cellHeight);

            int z = zValues[i][j];
            double intensity = z / 100.0;

            QColor color;
            if (z < 40)
                color = QColor::fromRgbF(0.0, intensity, 0.0);
            else if (z < 70)
                color = QColor::fromRgbF(intensity, intensity, 0.0);
            else
                color = QColor::fromRgbF(intensity, 0.0, 0.0);

            painter.setBrush(color);
            painter.setPen(Qt::NoPen);
            painter.drawRect(rect);
        }
    }

    // --- Draw coordinate axes ---
    painter.setPen(QPen(Qt::black, 2));

    // X-axis (bottom)
    painter.drawLine(offsetX, offsetY + gridHeight, offsetX + gridWidth, offsetY + gridHeight);
    // Y-axis (left)
    painter.drawLine(offsetX, offsetY, offsetX, offsetY + gridHeight);

    // --- Axis labels ---
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 8));

    int xStep = 60;
    for (int x = 0; x <= colsToDraw; x += xStep) {
        double px = offsetX + x * cellWidth;
        painter.drawText(px - 10, offsetY + gridHeight + 15, QString::number(x));
    }

    int yStep = 10;
    for (int y = 0; y <= rowsToDraw; y += yStep) {
        double py = offsetY + gridHeight - y * cellHeight;
        painter.drawText(offsetX - 25, py + 3, QString::number(y));
    }

    // --- Axis titles ---
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(offsetX + gridWidth + 10, offsetY + gridHeight + 10, "X");
    painter.drawText(offsetX - 15, offsetY - 10, "Y");
}

// Handle mouse clicks (detect which cell was clicked)
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    int w = width();
    int h = height();

    double cellWidth = 4;
    double cellHeight = 4;

    double gridWidth = cols * cellWidth;
    double gridHeight = rows * cellHeight;

    double offsetX = (w - gridWidth) / 2.0;
    double offsetY = (h - gridHeight) / 2.0;

    double x = event->pos().x();
    double y = event->pos().y();

    // Check if click is inside centered grid
    if (x < offsetX || x > offsetX + gridWidth ||
        y < offsetY || y > offsetY + gridHeight)
        return;

    int j = (x - offsetX) / cellWidth;
    int i = rows - 1 - ((y - offsetY) / cellHeight);

    if (i >= 0 && i < rows && j >= 0 && j < cols) {
        int z = zValues[i][j];
        QString msg = QString("X = %1\nY = %2\nZ = %3")
                          .arg(j).arg(i).arg(z);
        QMessageBox::information(this, "Cell Info", msg);
    }
}
