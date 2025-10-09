#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void selectFile();
    void copySelectedFile();

private:
    QTextEdit *textEdit;
    QString selectedFilePath;  // store the selected file path
};

#endif // MAINWINDOW_H
