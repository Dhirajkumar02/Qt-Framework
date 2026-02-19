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
#include <QDoubleValidator>
#include <QPropertyAnimation>
#include <QComboBox>

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

    void updateProgress(int value);
    void connectMinMaxValidation(QLineEdit *minEdit, QLineEdit *maxEdit);
    void validateMinMax(QLineEdit *minEdit, QLineEdit *maxEdit);

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

    bool validateReplayInputs();

private:
    // =============================
    // UI ELEMENTS
    // =============================

    // Top
    QPushButton *openFileButton;
    QLineEdit   *fileNameEdit;
    QLineEdit   *skipLineEdit;
    QPushButton *skipButton;

    // Bottom
    QPushButton *processButton;
    QPushButton *pauseButton;
    QPushButton *cancelButton;
    QSlider     *progressSlider;
    QLabel      *progressLabel;
    QComboBox   *speedCombo;

    // Analysis
    QGroupBox    *analysisBox;
    QRadioButton *analysisRadio;
    QCheckBox    *generateAllFiles;
    QCheckBox    *showCheckBox;

    // Replay
    QGroupBox    *replayBox;
    QRadioButton *replayRadio;

    QCheckBox *allTracks;
    QCheckBox *selectedTracks;
    QLineEdit *trackLineEdit;

    QCheckBox *chkRange;
    QCheckBox *chkAzm;
    QCheckBox *chkEle;
    QCheckBox *chkTime;

    QLineEdit *rangeMinEdit;
    QLineEdit *rangeMaxEdit;

    QLineEdit *azmMinEdit;
    QLineEdit *azmMaxEdit;

    QLineEdit *eleMinEdit;
    QLineEdit *eleMaxEdit;

    QLineEdit *timeMinEdit;
    QLineEdit *timeMaxEdit;

    QGridLayout *replayLayout;

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

    struct PspFrame
    {
        DLOG_HEADER hdr;
        DWELL_DATA  dwell;
        PSP_DATA    psp;
    };

    QVector<PspFrame> pspFrames;
};

#endif // MAINWINDOW_H
