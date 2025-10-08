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
    void openBinaryFile();
    void saveBinaryFile();

private:
    QTextEdit *textEdit;
};

#endif // MAINWINDOW_H
