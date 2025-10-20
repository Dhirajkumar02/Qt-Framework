#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QProgressBar>
#include <QLabel>
#include <QSpinBox>

class BinaryFileReader : public QWidget
{
    Q_OBJECT

public:
    BinaryFileReader(QWidget *parent = nullptr);

private slots:
    void browseFile();
    void processFile();
    void updateProgressBarColor(int value);

private:
    QPushButton *openButton;
    QPushButton *processButton;
    QPushButton *cancelButton;
    QLineEdit *fileNameEdit;
    QTableWidget *table;
    QProgressBar *progressBar;
    QLabel *statusLabel;

    QSpinBox *skipNumberInput;
    QPushButton *skipButton;

    QString selectedFilePath;
    bool cancelRequested = false;
    quint32 skipMsgId = 0; // MsgId to skip
};

#endif // MAINWINDOW_H
