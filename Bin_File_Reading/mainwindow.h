#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "messages.h"
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QFile>
#include <QSlider>
#include <QCheckBox>
#include <QListWidget>
#include <QLabel>
#include <QVector>

// ===============================
//  GLOBAL FRAME STRUCT (LRDE standard)
// ===============================
struct Frame
{
    DLOG_HEADER hdr;     // 8 bytes (MsgID + Time)
    DWELL_DATA dwell;    // Full dwell data
    WORD no_of_rpt;      // Number of reports
    float RMS_IQ;        // RMS-IQ value
    QVector<RPTS> rpts;  // Report list (0–50)
};

class BinaryFileReader : public QWidget
{
    Q_OBJECT

public:
    explicit BinaryFileReader(QWidget *parent = nullptr);
    ~BinaryFileReader();

private slots:
    void openFile();
    void analysisFile();
    void skipPressed();
    void sliderMoved();
    void togglePause();
    void cancelProcessing();

private:
    void setupUI();
    void setupConnections();
    void writeFrameData(const Frame &pspData, QTextStream &outputFile);
    static QString toHex(quint32 value, int width = 4);

private:
    // UI Elements
    QPushButton *openFileButton;
    QLineEdit   *fileNameEdit;
    QLineEdit   *skipLineEdit;
    QPushButton *skipButton;
    QPushButton *processButton;
    QPushButton *pauseButton;
    QPushButton *cancelButton;
    QSlider     *progressSlider;
    QCheckBox   *showCheckBox;

    QLabel *missingLabel;
    QListWidget *missingListWidget;

    // States
    QString filePath;
    QFile binFile;

    QString pspOutputFilePath;
    QFile pspOutputFile;

    bool headerWritten;
    quint32 skipPercent;
    bool isProcessing;
    bool stopRequested;
    bool pauseRequested;
    bool cancelRequested;

    // Temporary read buffers
    DLOG_HEADER strctLogHdr;
    DWELL_DATA  strctDwlData;

    // MAIN STORAGE (sorted + processed)
    QVector<Frame> frames;
};

#endif // MAINWINDOW_H
