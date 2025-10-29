#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "messages.h"

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QProgressBar>
#include <QFile>

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

private:
    QPushButton *openFileButton;
    QLineEdit *fileNameEdit;
    QString selectedFilePath;
    QLineEdit *skipLineEdit;
    QPushButton *processButton;
    QProgressBar *progressBar;
    QPushButton *skipButton;
    QTableWidget *table;
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
};

#endif // MAINWINDOW_H
