#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QMouseEvent>

// MainWindow is our custom QWidget that draws the square matrix
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // Handles painting of the matrix
    void paintEvent(QPaintEvent *event) override;
    // Handles mouse clicks
    void mousePressEvent(QMouseEvent *event) override;

private:
    int rows;   // number of rows (Y axis → 0 to 60°)
    int cols;   // number of columns (X axis → 0 to 360°)
    QVector<QVector<int>> zValues; // matrix of Z values
};

#endif // MAINWINDOW_H
