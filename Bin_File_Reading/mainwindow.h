#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QProgressBar>

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
    void openFile();
    void processFile();
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
};

#endif // MAINWINDOW_H
