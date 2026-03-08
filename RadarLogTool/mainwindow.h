#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include "logreader.h"
#include <QPushButton>
#include <QLineEdit>
#include <QFile>
#include <QFileInfo>
#include <QSlider>
#include <QCheckBox>
#include <QTextStream>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QButtonGroup>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:

    void openFile();
    void startProcessing();
    void skipPressed();
    void sliderMoved();
    void togglePause();
    void cancelProcessing();

private:

    void setupUI();
    void setupConnections();

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
    LogReader reader;

    QString filePath;

    enum class RunMode
    {
        Analysis,
        Replay
    };

    RunMode currentMode {RunMode::Analysis};

    quint32 skipPercent {0};
    bool isProcessing   {false};
    bool stopRequested  {false};
    bool pauseRequested {false};
    bool cancelRequested{false};

    bool header {true};
    quint32 count {0};

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
