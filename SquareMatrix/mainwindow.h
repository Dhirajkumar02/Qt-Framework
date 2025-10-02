#ifndef MAINWINDOW_H          // Prevents multiple inclusions of this header file
#define MAINWINDOW_H          // Defines MAINWINDOW_H if not already defined

// Qt includes
#include <QMainWindow>        // Base class for main application windows
#include <QImage>             // Provides image handling (used for heatmap rendering)
#include <QVector>            // Dynamic array container (used for storing z-values)
#include <QMouseEvent>        // Handles mouse input events

// MainWindow class declaration (inherits from QMainWindow)
class MainWindow : public QMainWindow
{
    Q_OBJECT                // Required by Qt’s meta-object system (signals/slots support)

public:
    // Constructor: initializes the main window
    explicit MainWindow(QWidget *parent = nullptr);

    // Destructor: cleans up resources
    ~MainWindow();

protected:
    // Handles window painting events (called automatically when window needs to be redrawn)
    void paintEvent(QPaintEvent *event) override;

    // Handles mouse press events (called when user clicks inside the window)
    void mousePressEvent(QMouseEvent *event) override;

private:
    // Number of rows in the data grid
    int rows;

    // Number of columns in the data grid
    int cols;

    // Minimum value in the dataset
    int minVal;

    // Maximum value in the dataset
    int maxVal;

    // 2D grid storing integer values (z-values for the heatmap)
    QVector<QVector<int>> zValues;

    // Image object that will store and display the generated heatmap
    QImage heatmap;

    // Generates random or test data for the heatmap
    void generateData();

    // Converts the data (zValues) into a heatmap image (QImage)
    void generateHeatmap();

    // Maps a normalized value (0.0–1.0) to a QColor for visualization
    QColor getColorFromValue(double value);
};

#endif // MAINWINDOW_H   // End of include guard
