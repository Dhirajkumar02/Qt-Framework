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
    // RUN MODE
    // =============================
    enum class RunMode
    {
        Analysis,
        Replay
    };

    RunMode currentMode {RunMode::Analysis};


    // =============================
    // FILTER VALUE STRUCT (UNIVERSAL)
    // =============================
    struct FilterValues
    {
        double range = 0;
        double azm   = 0;
        double ele   = 0;
        double time  = 0;
    };


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
    bool passFilters(const PSP_DATA &data);
    bool validateReplayInputs();


    // =============================
    // MODE CONTROL
    // =============================
    void onReplayMode();
    void onAnalysisMode();


    // =============================
    // FILTER SYSTEM
    // =============================
    bool isWithinWindow() const;
    bool checkWithinWindow(double range, double azm, double ele, double time) const;
    QFile* getAnalysisFile(bool filterPassed);
    double convertRangeToMeters(double value) const;


    // =============================
    // DISPLAY (Replay Mode)
    // =============================
    void sendToDisplay(const PSP_DATA &data);   // you implement later


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
    QLineEdit    *trackLineEdit;

    // Filter box
    QGroupBox *filterBox;

    // Filter checkboxes
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
    QFile   subStsOutputFile;   // ⭐ FILTERED OUTPUT FILE
    QFile   spOutputFile;
    QFile   spCenOutputFile;
    QFile   logOutputFile;
    QFile   txtOutputFile;


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
    Track_Request strctTrkReq;


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
    bool isPassed = false;
    double rangeTrkReq[0];
    double azmTrkReq[0];
    double eleTrkReq[0];

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
