#include "mainwindow.h"
#include <QPainter>
#include <QMessageBox>
#include <random>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , rows(60), cols(360)
{
    //fill z matrix with random values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<>dis(1, 100);

    zValues.resize(rows);

    for(int i = 0; i<rows; ++i){
        zValues[i].resize(cols);
        for(int j=0; j<cols; ++j){
            zValues[i][j] = dis(gen); // random value b/w 1 to 100
        }
    }
}

MainWindow::~MainWindow(){}

//Draw grid of squares
void MainWindow::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int w = width();
        int h = height();

        // perfect square cells maintain करने के लिए
        double cellWidth = double(w) / cols;
        double cellHeight = double(h) / rows;


        // Draw each cell
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                QRectF rect(j * cellWidth, i * cellHeight, cellWidth, cellHeight);

                int z = zValues[i][j];
                double intensity = z / 100.0; // brightness factor 0.0 - 1.0

                QColor color;

                if (z < 40) {
                    // Green shades
                    color = QColor::fromRgbF(0.0, intensity, 0.0);
                }
                else if (z < 70) {
                    // Yellow shades (Red + Green)
                    color = QColor::fromRgbF(intensity, intensity, 0.0);
                }
                else {
                    // Red shades
                    color = QColor::fromRgbF(intensity, 0.0, 0.0);
                }

                painter.setBrush(color);
                painter.setPen(QPen(Qt::black, 0.3));
                painter.drawRect(rect);
            }
        }
    }

//Detect which cell was clicked
    void MainWindow::mousePressEvent(QMouseEvent *event){
        int w = width();
        int h = height();

        double cellW = double(w)/cols;
        double cellH = double(h)/rows;

        int j = event->pos().x()/cellW;  // column index
        int i = event->pos().y()/cellH;  // row index

        if(i >= 0 && i < rows && j >= 0 && j < cols){
            int z = zValues[i][j];

            QString msg = QString("X = %1 \nY = %2 \nZ = %3")
                              .arg(j) //column = X
                              .arg(i) //Row = Y
                              .arg(z);

            QMessageBox::information(this, "Cell Info", msg);
        }
    }
