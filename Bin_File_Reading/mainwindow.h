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
class BinaryFileReader : public QWidget
{
    Q_OBJECT

public:
    explicit BinaryFileReader(QWidget *parent = nullptr);
    ~BinaryFileReader();

private slots:
    void setupUI();
    void setupConnections();
    void openFile();
    void analysisFile();
    void skipPercentage();
    void onSliderReleased();

private:
    QPushButton *openFileButton;
    QLineEdit *fileNameEdit;
    QString selectedFilePath;
    QLineEdit *skipLineEdit;
    QPushButton *processButton;
    QPushButton *skipButton;
    QCheckBox *showCheckBox;
    QSlider *progressSlider;
    quint32 skipPercent;

    DLOG_HEADER strctLogHdr;
    PSP_DATA strctPspData;
    DWELL_DATA strctDwlData;
    RPTS strctRpts;

    QString binfilePath;
    QFile binfile;
    QString spOutputFilePath;
    QFile spOutputFile;

    void writePspData(const PSP_DATA &strctPspData, QFile &spOutputFile);
    void reprocessFromPosition(quint64 position);
};

#endif // MAINWINDOW_H
