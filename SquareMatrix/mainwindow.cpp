#include "mainwindow.h"       // Include the MainWindow header (class definition)
#include <QPainter>           // For drawing images and shapes
#include <QMessageBox>        // For showing message boxes
#include <QInputDialog>       // For asking user input dialogs (rows & cols)
#include <random>             // For random number generation

// ============================
// Function: getColorFromValue
// Maps a normalized value (0.0 → 1.0) into a QColor (used for heatmap coloring)
// ============================
QColor MainWindow::getColorFromValue(double value)
{
    // Clamp the value between 0 and 1 (safety check)
    value = std::max(0.0, std::min(1.0, value));

    // Different color ranges depending on the normalized value
    if (value < 0.125) { // Very low values → Light Green to Green
        double t = value / 0.125;
        return QColor(144 - t*44, 238 - t*83, 144 - t*44);
    } else if (value < 0.25) { // Green → Dark Green
        double t = (value - 0.125) / 0.125;
        return QColor(100 - t*50, 155 - t*55, 100 - t*50);
    } else if (value < 0.375) { // Dark Green → Light Yellow
        double t = (value - 0.25) / 0.125;
        return QColor(50 + t*205, 100 + t*155, 50);
    } else if (value < 0.5) { // Light Yellow → Yellow
        double t = (value - 0.375) / 0.125;
        return QColor(255, 255 - t*55, 50 + t*205);
    } else if (value < 0.625) { // Yellow → Orange
        double t = (value - 0.5) / 0.125;
        return QColor(255, 255 - t*127, 0);
    } else if (value < 0.75) { // Orange → Light Red
        double t = (value - 0.625) / 0.125;
        return QColor(255, 128 - t*50, 0 + t*128);
    } else if (value < 0.875) { // Light Red → Red
        double t = (value - 0.75) / 0.125;
        return QColor(255, 78 - t*78, 128 - t*128);
    } else { // Red → Dark Red (highest values)
        double t = (value - 0.875) / 0.125;
        return QColor(255 - t*100, 0, 0);
    }
}

// ============================
// Constructor: MainWindow
// Initializes the main window, asks for rows/columns, generates data & heatmap
// ============================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), rows(0), cols(0), minVal(1), maxVal(1000) // Initialize variables
{
    bool ok; // Flag to check if user clicked "OK" in input dialogs

    // Ask user for number of rows (between 10 and 5000, default = 100)
    rows = QInputDialog::getInt(this, "Rows", "Enter number of rows:", 100, 10, 5000, 1, &ok);

    // Ask user for number of columns (between 10 and 5000, default = 100)
    cols = QInputDialog::getInt(this, "Columns", "Enter number of columns:", 100, 10, 5000, 1, &ok);

    // Generate random data for heatmap
    generateData();

    // Convert the data into a heatmap image
    generateHeatmap();

    // Resize window to 800x600 pixels
    resize(800, 600);
}

// ============================
// Destructor: MainWindow
// Cleans up resources (empty because Qt handles most cleanup automatically)
// ============================
MainWindow::~MainWindow() {}

// ============================
// Function: generateData
// Creates a random 2D grid of values between minVal and maxVal
// ============================
void MainWindow::generateData()
{
    std::random_device rd;                        // Random device (entropy source)
    std::mt19937 gen(rd());                       // Mersenne Twister random number generator
    std::uniform_int_distribution<> dis(minVal, maxVal); // Uniform distribution between minVal & maxVal

    zValues.resize(rows);                         // Resize vector to number of rows
    for (int i = 0; i < rows; ++i) {
        zValues[i].resize(cols);                  // Resize each row to number of columns
        for (int j = 0; j < cols; ++j) {
            zValues[i][j] = dis(gen);             // Assign random value to each cell
        }
    }
}

// ============================
// Function: generateHeatmap
// Converts the 2D data grid into a QImage (heatmap colors applied)
// ============================
void MainWindow::generateHeatmap()
{
    // Create an image with same size as the grid
    heatmap = QImage(cols, rows, QImage::Format_RGB32);

    // Loop over every cell in the matrix
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Normalize the value between 0 and 1
            double value = double(zValues[i][j] - minVal) / (maxVal - minVal);

            // Convert normalized value to a QColor
            QColor color = getColorFromValue(value);

            // Set the pixel color in the image
            heatmap.setPixelColor(j, i, color);
        }
    }
}

// ============================
// Function: paintEvent
// Called automatically whenever the window needs to be redrawn
// Draws the heatmap image scaled to the window size
// ============================
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);             // Prevent unused variable warning
    QPainter painter(this);      // Create painter to draw on the window
    painter.drawImage(rect(), heatmap); // Draw the heatmap stretched to fit window
}

// ============================
// Function: mousePressEvent
// Called when user clicks inside the window
// Detects which cell was clicked and shows its value
// ============================
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    int w = width();             // Current window width
    int h = height();            // Current window height

    // Calculate width & height of each cell (scaled to window size)
    double cellW = double(w) / cols;
    double cellH = double(h) / rows;

    // Find column (j) and row (i) clicked based on mouse position
    int j = event->pos().x() / cellW;
    int i = event->pos().y() / cellH;

    // Make sure the clicked point is inside grid bounds
    if (i >= 0 && i < rows && j >= 0 && j < cols) {
        int z = zValues[i][j];   // Get the actual value at clicked cell

        // Compute angular representation (optional feature)
        double xAngle = (360.0 / cols) * j;  // Map column to X angle (0–360°)
        double yAngle = (60.0 / rows) * i;   // Map row to Y angle (0–60°)

        // Prepare message string with coordinates & value
        QString msg = QString("X = %1°\nY = %2°\nZ = %3")
                          .arg(xAngle, 0, 'f', 2)  // X angle, 2 decimal places
                          .arg(yAngle, 0, 'f', 2)  // Y angle, 2 decimal places
                          .arg(z);                 // Actual Z value

        // Show the message box with cell info
        QMessageBox::information(this, "Cell Info", msg);
    }
}
