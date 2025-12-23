#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "messages.h"

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QFile>
#include <QFileInfo>
#include <QSlider>
#include <QCheckBox>
#include <QVector>
#include <QTextStream>

/**
 * @class BinaryFileReader
 * @brief
 *  High-performance binary log reader with:
 *  - Forward-only buffered byte-wise scan (NO SEEK)
 *  - Skip % jump
 *  - Slider-based restart
 *  - Pause / Resume
 *  - Cancel
 *  - Multiple output files
 *  - Throttled real-time progress updates
 *
 *  Designed for large radar / PSP binary logs.
 */
class BinaryFileReader : public QWidget
{
    Q_OBJECT

public:
    explicit BinaryFileReader(QWidget *parent = nullptr);
    ~BinaryFileReader();

private slots:
    void openFile();             // Select input .bin file
    void analysisFile();         // Start / Restart analysis
    void skipPressed();          // Manual skip % (before analysis)
    void sliderMoved();          // Slider seek (restart-based)
    void togglePause();          // Pause / Resume processing
    void cancelProcessing();     // Cancel processing immediately

private:
    // -------- UI SETUP --------
    void setupUI();
    void setupConnections();

    // -------- FILE HELPERS --------
    bool openOutputFile(QFile &file,
                        const QFileInfo &fileInfo,
                        const QString &extension);

    bool openAllOutputFiles(const QFileInfo &fileInfo);

    // -------- DATA WRITERS --------
    void writePspData(const PSP_DATA &data, QFile &pspOutputFile);

    static QString toHex(quint32 value, int width = 4);

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

    // -------- FILES --------
    QString filePath;
    QFile   binFile;

    QFile   pspOutputFile;
    QFile   stsOutputFile;
    QFile   spOutputFile;
    QFile   spCenOutputFile;
    QFile   logOutputFile;

    // -------- STATE FLAGS --------
    quint32 skipPercent {0};
    bool isProcessing   {false};
    bool stopRequested  {false};
    bool pauseRequested {false};
    bool cancelRequested{false};

    bool header {true};
    quint32 count {0};

    // -------- BINARY STRUCTS --------
    DLOG_HEADER strctLogHdr;
    PSP_DATA    strctPspData;
    DWELL_DATA  strctDwlData;
    RPTS        strctRpts;

    // -------- OPTIONAL: FRAME STORAGE --------
    struct PspFrame
    {
        DLOG_HEADER hdr;
        DWELL_DATA  dwell;
        PSP_DATA    psp;
    };

    QVector<PspFrame> pspFrames;

    struct OutputSpec
    {
        QFile* file;
        QString extension;
    };

};

#endif // MAINWINDOW_H
