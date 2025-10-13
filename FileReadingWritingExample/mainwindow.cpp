#include "mainwindow.h"
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    textEdit(new QTextEdit(this))
{
    setCentralWidget(textEdit);
    setupMenu();
    setWindowTitle("Qt Notepad");
    resize(700, 500);
}

MainWindow::~MainWindow() {}

// ------------------ Menu Setup ------------------
void MainWindow::setupMenu()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");

    QAction *newAct = new QAction("New", this);
    QAction *openAct = new QAction("Open", this);
    QAction *saveAct = new QAction("Save", this);
    QAction *saveAsAct = new QAction("Save As", this);
    QAction *copyAct = new QAction("Copy File", this);
    QAction *exitAct = new QAction("Exit", this);

    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(copyAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(copyAct, &QAction::triggered, this, &MainWindow::copyFile);
    connect(exitAct, &QAction::triggered, this, &MainWindow::exitApp);
}

// ------------------ File Functions ------------------
void MainWindow::newFile()
{
    textEdit->clear();
    currentFilePath.clear();
    setWindowTitle("Untitled - Qt Notepad");
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        textEdit->setText(in.readAll());
        file.close();
        currentFilePath = fileName;
        setWindowTitle(QFileInfo(fileName).fileName() + " - Qt Notepad");
    } else {
        QMessageBox::warning(this, "Error", "Cannot open file: " + file.errorString());
    }
}

void MainWindow::saveFile()
{
    if (currentFilePath.isEmpty()) {
        saveFileAs();
        return;
    }

    QFile file(currentFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << textEdit->toPlainText();
        file.close();
        setWindowTitle(QFileInfo(currentFilePath).fileName() + " - Qt Notepad");
        QMessageBox::information(this, "Saved", "File saved successfully!");
    } else {
        QMessageBox::warning(this, "Error", "Cannot save file: " + file.errorString());
    }
}

void MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save File As", "", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty())
        return;

    currentFilePath = fileName;
    saveFile();
}

void MainWindow::copyFile()
{
    QString sourceFile = QFileDialog::getOpenFileName(this, "Select Source File", "", "Text Files (*.txt);;All Files (*)");
    if (sourceFile.isEmpty())
        return;

    QString destFile = QFileDialog::getSaveFileName(this, "Select Destination File", "", "Text Files (*.txt);;All Files (*)");
    if (destFile.isEmpty())
        return;

    QFile inputFile(sourceFile);
    QFile outputFile(destFile);

    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open source file: " + inputFile.errorString());
        return;
    }

    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Cannot open destination file: " + outputFile.errorString());
        inputFile.close();
        return;
    }

    // ✅ Read and write character by character
    char ch;
    while (!inputFile.atEnd()) {
        inputFile.getChar(&ch);  // read one character
        outputFile.putChar(ch);  // write it immediately
    }

    inputFile.close();
    outputFile.close();

    QMessageBox::information(this, "Success", "File copied successfully!");
}


void MainWindow::exitApp()
{
    QApplication::quit();
}
