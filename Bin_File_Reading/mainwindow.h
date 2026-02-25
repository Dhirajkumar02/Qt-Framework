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
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QButtonGroup>
#include <QIntValidator>

/**
 * @class BinaryFileReader
 * High-performance binary reader + analysis + replay UI
 */
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
    // =============================
    // UI SETUP
    // =============================
    void setupUI();
    void setupConnections();

    // =============================
    // FILE HELPERS
    // =============================
    bool openOutputFile(QFile &file,
                        const QFileInfo &fileInfo,
                        const QString &extension);

    bool openAllOutputFiles(const QFileInfo &fileInfo);

    // =============================
    // DATA WRITERS
    // =============================
    void writePspData(const PSP_DATA &data, QFile &pspOutputFile);

    static QString toHex(quint32 value, int width = 4);

    // =============================
    // REPLAY VALIDATION
    // =============================
    bool validateReplayInputs();   // (optional future use)
    void onReplayMode();
    void onAnalysisMode();
    bool passFilters(const PSP_DATA &data);


private:
    // =============================
    // UI ELEMENTS
    // =============================

    // Top controls
    QPushButton *openFileButton;
    QLineEdit   *fileNameEdit;
    QLineEdit   *skipLineEdit;
    QPushButton *skipButton;

    // Bottom controls
    QPushButton *processButton;
    QPushButton *pauseButton;
    QPushButton *cancelButton;
    QSlider     *progressSlider;
    QLabel      *progressLabel;

    // ---------------- Analysis ----------------
    QGroupBox    *analysisBox;
    QRadioButton *analysisRadio;
    QCheckBox    *generateAllFiles;
    QCheckBox    *showCheckBox;

    // ---------------- Replay ----------------
    QGroupBox    *replayBox;
    QRadioButton *replayRadio;
    QRadioButton *allTracks;
    QRadioButton *selectAllTracks;
    QLineEdit *trackLineEdit;

    // Filter checkboxes
    QGroupBox    *filterBox;
    QCheckBox *chkRange;
    QCheckBox *chkAzm;
    QCheckBox *chkEle;
    QCheckBox *chkTime;

    // Range inputs
    QLineEdit *rangeMinEdit;
    QLineEdit *rangeMaxEdit;

    // Azm inputs
    QLineEdit *azmMinEdit;
    QLineEdit *azmMaxEdit;

    // Ele inputs
    QLineEdit *eleMinEdit;
    QLineEdit *eleMaxEdit;

    // Time inputs
    QLineEdit *timeMinEdit;
    QLineEdit *timeMaxEdit;

    // Layout helpers
    QGridLayout *replayLayout;

    // Button groups
    QButtonGroup *modeGroup;
    QButtonGroup *filterGroup;


    // =============================
    // FILES
    // =============================
    QString filePath;
    QFile   binFile;

    QFile   pspOutputFile;
    QFile   stsOutputFile;
    QFile   spOutputFile;
    QFile   spCenOutputFile;
    QFile   logOutputFile;


    // =============================
    // STATE FLAGS
    // =============================
    quint32 skipPercent {0};
    bool isProcessing   {false};
    bool stopRequested  {false};
    bool pauseRequested {false};
    bool cancelRequested{false};

    bool header {true};
    quint32 count {0};


    // =============================
    // BINARY STRUCTS
    // =============================
    DLOG_HEADER strctLogHdr;
    PSP_DATA    strctPspData;
    DWELL_DATA  strctDwlData;
    RPTS        strctRpts;


    // =============================
    // OPTIONAL FRAME STORAGE
    // =============================
    struct PspFrame
    {
        DLOG_HEADER hdr;
        DWELL_DATA  dwell;
        PSP_DATA    psp;
    };

    QVector<PspFrame> pspFrames;


    // =============================
    // OUTPUT SPEC HELPER
    // =============================
    struct OutputSpec
    {
        QFile* file;
        QString extension;
    };
};

#endif // MAINWINDOW_H
