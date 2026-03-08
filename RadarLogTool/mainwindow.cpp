#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "messages.h"
#include "logreader.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QThread>
#include <QDebug>
#include <QListWidgetItem>
#include <QListWidget>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
    skipPercent(0),
    isProcessing(false),
    stopRequested(false),
    pauseRequested(false),
    cancelRequested(false)
{
    setWindowTitle("Radar Log Analyzer & Replay Tool");
    resize(1000, 650);
    setupUI();
    setupConnections();
    header=false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{

    // =============================
    // FILE SECTION (TOP)
    // =============================
    openFileButton = new QPushButton("Browse");
    fileNameEdit = new QLineEdit;
    fileNameEdit->setPlaceholderText("Select .bin file");
    fileNameEdit->setMinimumWidth(250);

    skipLineEdit = new QLineEdit;
    skipLineEdit->setPlaceholderText("Skip %");
    skipLineEdit->setMaximumWidth(80);

    skipButton = new QPushButton("Skip");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(openFileButton);
    topLayout->addWidget(fileNameEdit, 1);
    topLayout->addWidget(skipLineEdit);
    topLayout->addWidget(skipButton);


    // =============================
    // ANALYSIS GROUP
    // =============================
    analysisBox = new QGroupBox("Analysis");

    analysisRadio = new QRadioButton("Analysis Mode");
    generateAllFiles = new QCheckBox("Generate All");
    showCheckBox = new QCheckBox("Show All");

    QGridLayout *analysisLayout = new QGridLayout;

    int row1 = 0;
    analysisLayout->addWidget(analysisRadio, row1++, 0, 1, 5, Qt::AlignHCenter);
    analysisLayout->addWidget(generateAllFiles, row1++, 0);
    analysisLayout->addWidget(showCheckBox, row1++, 0);

    analysisLayout->setVerticalSpacing(10);
    analysisLayout->setRowStretch(row1, 1);
    analysisBox->setLayout(analysisLayout);


    // =============================
    // REPLAY GROUP
    // =============================
    replayBox = new QGroupBox("Replay");

    replayRadio = new QRadioButton("Replay Mode");
    allTracks = new QRadioButton("All Tracks");
    selectAllTracks = new QRadioButton("Select All Tracks");

    trackLineEdit = new QLineEdit;
    trackLineEdit->setPlaceholderText("Track ID (1-1000)");
    //trackLineEdit->setMinimumWidth(150);

    // =============================
    // REPLAY LAYOUT
    // =============================
    QGridLayout *replayLayout = new QGridLayout;

    int row = 0;

    replayLayout->addWidget(replayRadio, row++, 0, 1, 5, Qt::AlignHCenter);
    replayLayout->addWidget(allTracks, row++, 0);
    replayLayout->addWidget(selectAllTracks, row, 0);
    replayLayout->addWidget(new QLabel("Track ID:"), row, 1);
    replayLayout->addWidget(trackLineEdit, row++, 1, 1, 4);


    replayLayout->setVerticalSpacing(8);
    replayLayout->setRowStretch(row, 1);

    replayBox->setLayout(replayLayout);

    QButtonGroup *trackGroup = new QButtonGroup(this);
    trackGroup->addButton(allTracks);
    trackGroup->addButton(selectAllTracks);
    trackGroup->setExclusive(true);

    // =============================
    // FILTER GROUP
    // =============================
    filterBox = new QGroupBox("Filters");

    chkRange = new QCheckBox("Range");
    chkAzm   = new QCheckBox("Azm");
    chkEle   = new QCheckBox("Ele");
    chkTime  = new QCheckBox("Time");

    rangeMinEdit = new QLineEdit;
    rangeMaxEdit = new QLineEdit;

    azmMinEdit = new QLineEdit;
    azmMaxEdit = new QLineEdit;

    eleMinEdit = new QLineEdit;
    eleMaxEdit = new QLineEdit;

    timeMinEdit = new QLineEdit;
    timeMaxEdit = new QLineEdit;

    QLabel *rangeMinLbl = new QLabel("Min");
    QLabel *rangeMaxLbl = new QLabel("Max");

    QLabel *azmMinLbl = new QLabel("Min");
    QLabel *azmMaxLbl = new QLabel("Max");

    QLabel *eleMinLbl = new QLabel("Min");
    QLabel *eleMaxLbl = new QLabel("Max");

    QLabel *timeMinLbl = new QLabel("Min");
    QLabel *timeMaxLbl = new QLabel("Max");


    QGridLayout *filterLayout = new QGridLayout;

    // LEFT SIDE
    filterLayout->addWidget(chkRange, 0, 0);
    filterLayout->addWidget(rangeMinLbl, 0, 1);
    filterLayout->addWidget(rangeMinEdit, 0, 2);
    filterLayout->addWidget(rangeMaxLbl, 0, 3);
    filterLayout->addWidget(rangeMaxEdit, 0, 4);

    filterLayout->addWidget(chkEle, 1, 0);
    filterLayout->addWidget(eleMinLbl, 1, 1);
    filterLayout->addWidget(eleMinEdit, 1, 2);
    filterLayout->addWidget(eleMaxLbl, 1, 3);
    filterLayout->addWidget(eleMaxEdit, 1, 4);

    // RIGHT SIDE
    filterLayout->addWidget(chkAzm, 0, 6);
    filterLayout->addWidget(azmMinLbl, 0, 7);
    filterLayout->addWidget(azmMinEdit, 0, 8);
    filterLayout->addWidget(azmMaxLbl, 0, 9);
    filterLayout->addWidget(azmMaxEdit, 0, 10);

    filterLayout->addWidget(chkTime, 1, 6);
    filterLayout->addWidget(timeMinLbl, 1, 7);
    filterLayout->addWidget(timeMinEdit, 1, 8);
    filterLayout->addWidget(timeMaxLbl, 1, 9);
    filterLayout->addWidget(timeMaxEdit, 1, 10);


    // ⭐ important stretch between left and right
    filterLayout->setColumnStretch(5, 1);

    filterLayout->setHorizontalSpacing(12);
    filterLayout->setVerticalSpacing(8);

    filterBox->setLayout(filterLayout);


    // =============================
    // MODE GROUP
    // =============================
    modeGroup = new QButtonGroup(this);
    modeGroup->addButton(analysisRadio);
    modeGroup->addButton(replayRadio);
    modeGroup->setExclusive(true);


    // =============================
    // MIDDLE SECTION
    // =============================
    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addWidget(analysisBox,1);
    middleLayout->addWidget(replayBox,1);

    QHBoxLayout *filterChkLayout = new QHBoxLayout;
    filterChkLayout->addWidget(filterBox);


    // =============================
    // BOTTOM CONTROLS
    // =============================
    processButton = new QPushButton("Start");
    pauseButton = new QPushButton("Pause");
    cancelButton = new QPushButton("Cancel");

    progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setRange(0, 100);

    progressLabel = new QLabel("0 %");
    progressLabel->setMinimumWidth(50);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(processButton);
    bottomLayout->addWidget(progressSlider, 1);
    bottomLayout->addWidget(progressLabel);
    bottomLayout->addWidget(pauseButton);
    bottomLayout->addWidget(cancelButton);


    // =============================
    // MAIN LAYOUT
    // =============================
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(middleLayout, 1);
    mainLayout->addLayout(filterChkLayout);
    mainLayout->addLayout(bottomLayout);

    //StyleSheet
    setStyleSheet(R"(
QWidget {
    background-color: #F4F6F7;
    font-family: Segoe UI;
    font-size: 13px;
}
QPushButton {
    background-color: #2E86C1;
    color: white;
    border-radius: 5px;
    padding: 6px 12px;
}
QPushButton:hover {
    background-color: #5DADE2;
}
QPushButton:pressed {
    background-color: #1B4F72;
}
QLineEdit {
    border: 1px solid #BDC3C7;
    border-radius: 4px;
    padding: 4px;
    background: white;
}
QGroupBox {
    border: 1px solid #D0D7DE;
    border-radius: 8px;
    margin-top: 12px;
    background: white;
}

QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 3px 8px;
    background: #2E86C1;
    color: white;
    border-radius: 5px;
}
QSlider::groove:horizontal {
    height: 8px;
    background: #D5D8DC;
    border-radius: 4px;
}
QSlider::handle:horizontal {
    background: #2E86C1;
    border: 2px solid #1B4F72;
    width: 18px; height: 18px;
    margin: -6px 0;
    border-radius: 9px;
}
QSlider::sub-page:horizontal {
    background: #5DADE2;
border-radius: 4px;
}
QCheckBox {
    spacing: 6px;
}

QCheckBox::indicator {
    width: 16px;
    height: 16px;
}

QCheckBox::indicator:unchecked {
    border: 1px solid #7F8C8D;
    background: white;
}

QCheckBox::indicator:checked {
    border: 1px solid #2E86C1;
    background: #2E86C1;
}

QCheckBox::indicator:checked::after {
    content: "";
}

QCheckBox {
    padding: 2px 4px;
    border-radius: 4px;
}

QCheckBox:hover {
    background-color: #EBF5FB;
}
QLabel {
    padding: 1px 4px;
    border-radius: 3px;
}
QLabel:hover {
    background-color: #F2F3F4;
})");

}

void MainWindow::setupConnections()
{
    connect(openFileButton,&QPushButton::clicked,this,&MainWindow::openFile);
    connect(processButton,&QPushButton::clicked,this,&MainWindow::startProcessing);
    connect(skipButton,&QPushButton::clicked,this,&MainWindow::skipPressed);
    connect(progressSlider,&QSlider::sliderReleased,this,&MainWindow::sliderMoved);
    connect(pauseButton,&QPushButton::clicked,this,&MainWindow::togglePause);
    connect(cancelButton,&QPushButton::clicked,this,&MainWindow::cancelProcessing);

    connect(fileNameEdit,&QLineEdit::textChanged,this,[this](QString t){
        processButton->setEnabled(!t.isEmpty());
    });

    connect(skipLineEdit,&QLineEdit::textChanged,this,[this](QString t){
        bool ok; int p=t.toInt(&ok);
        skipButton->setEnabled(ok && p>=0 && p<100);
    });

    /*connect(analysisRadio, &QRadioButton::toggled,
            this, [=](bool checked)
            {
                if (checked)
                    onAnalysisMode();
            });

    connect(replayRadio, &QRadioButton::toggled,
            this, [=](bool checked)
            {
                if (checked)
                    onReplayMode();
            });*/

    connect(allTracks, &QRadioButton::toggled,
            this, [=]()
            {
                trackLineEdit->setEnabled(selectAllTracks->isChecked());
            });

    connect(selectAllTracks, &QRadioButton::toggled,
            this, [=]()
            {
                trackLineEdit->setEnabled(selectAllTracks->isChecked());
            });
    connect(chkRange, &QCheckBox::toggled, this, [=](bool checked){
        rangeMinEdit->setEnabled(checked);
        rangeMaxEdit->setEnabled(checked);
    });
    connect(chkAzm, &QCheckBox::toggled, this, [=](bool checked){
        azmMinEdit->setEnabled(checked);
        azmMaxEdit->setEnabled(checked);
    });
    connect(chkEle, &QCheckBox::toggled, this, [=](bool checked){
        eleMinEdit->setEnabled(checked);
        eleMaxEdit->setEnabled(checked);
    });
    connect(chkTime, &QCheckBox::toggled, this, [=](bool checked){
        timeMinEdit->setEnabled(checked);
        timeMaxEdit->setEnabled(checked);
    });
}

void MainWindow::openFile()
{
    filePath = QFileDialog::getOpenFileName(
        this,   // ✅ parent (important)
        "Open Binary File",
        "",
        "Binary Files (*.bin);;All Files (*)",
        nullptr,
        QFileDialog::DontUseNativeDialog   // ✅ GTK warning fix
        );

    if (filePath.isEmpty())
        return;

    fileNameEdit->setText(filePath);
    openFileButton->setStyleSheet("background:lightgreen;");
}
void MainWindow::skipPressed()
{
    bool ok = false;
    int p = skipLineEdit->text().toInt(&ok);  // ✅ int

    if (!ok || p < 0 || (p > 100))
    {
        QMessageBox::warning(
            this,
            "Input Error",
            "Please enter a number between 0 and 100"
            );
        return;
    }

    skipPercent = static_cast<quint32>(p);  // ✅ safe conversion
    progressSlider->setValue(p);
}
void MainWindow::sliderMoved()
{
    // BEFORE analysis: only set skip%
    skipPercent = progressSlider->value();
    if(!isProcessing)
    {
        return;
    }
    stopRequested = true;

}

void MainWindow::togglePause()
{
    pauseRequested = !pauseRequested;

    if(pauseRequested)
        pauseButton->setText("Resume");
    else
        pauseButton->setText("Pause");
}
void MainWindow::cancelProcessing()
{
    cancelRequested = true;
    stopRequested = true;
}

void MainWindow::startProcessing()
{
    reader.setFile(filePath);
    reader.processFile();
}
