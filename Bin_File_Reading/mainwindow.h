#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "messages.h"

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QFile>
#include <QCheckBox>
#include <QSlider>

namespace Ui {
class BinaryFileReader;
}

/**
 * @class BinaryFileReader
 * @brief Reads and analyzes binary files (.bin), allowing skipping and reprocessing using slider or percentage.
 *
 * Features:
 *  - Open and analyze .bin file
 *  - Skip initial percentage via button or slider
 *  - Reprocess from slider position (mouse drag or click)
 *  - Live progress slider updates
 */
class BinaryFileReader : public QWidget
{
    Q_OBJECT

public:
    explicit BinaryFileReader(QWidget *parent = nullptr);
    ~BinaryFileReader();

private slots:
    // UI setup and signal-slot connections
    void setupUI();
    void setupConnections();

    // File and analysis actions
    void openFile();              // Select .bin file
    void analysisFile();          // Process and analyze binary file
    void skipPercentage();        // Handle Skip button press
    void onSliderReleased();      // Handle user slider drag/click

private:
    // ------------------------------
    // UI Elements
    // ------------------------------
    QPushButton *openFileButton;   // Open File button
    QLineEdit *fileNameEdit;       // Shows selected file name
    QLineEdit *skipLineEdit;       // Input skip percentage
    QPushButton *processButton;    // Start analysis
    QPushButton *skipButton;       // Skip % button
    QCheckBox *showCheckBox;       // Optional “Show” checkbox
    QSlider *progressSlider;       // Interactive progress/seek slider

    // ------------------------------
    // Data & State Members
    // ------------------------------
    QString binfilePath;           // Full path of selected binary file
    QFile binfile;                 // Input binary file handle
    QString spOutputFilePath;      // Output text file path
    QFile spOutputFile;            // Output file handle
    quint32 skipPercent;           // Skip percentage (0–99)
    bool isProcessing;             // True if analysis is ongoing

    // ------------------------------
    // Structures for binary data
    // ------------------------------
    DLOG_HEADER strctLogHdr;
    PSP_DATA strctPspData;
    DWELL_DATA strctDwlData;
    RPTS strctRpts;

    // ------------------------------
    // Helper Methods
    // ------------------------------
    void writePspData(const PSP_DATA &strctPspData, QFile &spOutputFile);
    void reprocessFromPosition(quint64 position);
};

#endif // MAINWINDOW_H
