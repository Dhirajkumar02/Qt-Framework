#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QString>

class BinaryFileReader : public QWidget
{
    Q_OBJECT

public:
    explicit BinaryFileReader(QWidget *parent = nullptr);

private slots:
    void browseFile();
    void onOpenFileButtonClicked();
    void processFile();

private:
    QPushButton *openButton;
    QPushButton *processButton;
    QLineEdit *fileNameEdit;
    QTableWidget *table;
    QString selectedFilePath;
};

#endif // MAINWINDOW_H
