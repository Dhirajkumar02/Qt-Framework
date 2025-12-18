#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "messages.h"
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QFile>
#include <QSlider>
#include <QCheckBox>

/**
 * @class BinaryFileReader
 * @brief
 *  High-performance binary log reader with:
 *  - Skip % jump
 *  - Slider-based seeking (like video player)
 *  - Pause/Resume
 *  - Cancel
 *  - Single unified analysis function
 *  - Real-time progress bar updates
 *
 *  This class reads radar-like PSP binary logs using fixed structures.
 *  It supports dynamic re-seeking while processing.
 */
class BinaryFileReader : public QWidget
{
    Q_OBJECT

public:
    explicit BinaryFileReader(QWidget *parent = nullptr);
    ~BinaryFileReader();

private slots:
    void openFile();         // Select input .bin file
    void analysisFile();     // Start/Restart full analysis
    void skipPressed();      // Manual skip % (before analysis)
    void sliderMoved();      // Slider seek (before & during analysis)
    void togglePause();      // Pause/Resume processing
    void cancelProcessing(); // Cancel processing immediately

private:
    void setupUI();          // Build UI elements
    void setupConnections(); // Connect signals/slots


private:
    // -------- UI ELEMENTS --------
    QPushButton *openFileButton;
    QLineEdit   *fileNameEdit;
    QLineEdit   *skipLineEdit;
    QPushButton *skipButton;
    QPushButton *processButton;
    QPushButton *pauseButton;
    QPushButton *cancelButton;
    QSlider     *progressSlider;
    QCheckBox   *showCheckBox;

    // -------- DATA / STATES --------
    QString filePath;
    QFile   binFile;

    QString pspOutputFilePath;
    QFile   pspOutputFile;
    QString stsOutputFilePath;
    QFile stsOutputFile;
    bool header;
    quint32 count;

    quint32 skipPercent;     // Current skip % position
    bool isProcessing;       // True while analysis loop runs
    bool stopRequested;      // For slider/skip restart
    bool pauseRequested;     // For pause/resume
    bool cancelRequested;    // For cancel

    // -------- BINARY STRUCTS --------
    DLOG_HEADER strctLogHdr;
    PSP_DATA    strctPspData;
    DWELL_DATA  strctDwlData;
    RPTS        strctRpts;

    // ======== FRAME STRUCT FOR SORTING ========
    struct PspFrame {
        DLOG_HEADER hdr;
        DWELL_DATA dwell;
        PSP_DATA psp;
    };

    QVector<PspFrame> pspFrames;

    void writePspData(const PSP_DATA &strctPspData, QFile &pspOutputFile);
    static QString toHex(quint32 value, int width=4);
};
#endif // MAINWINDOW_H
