#include "mainwindow.h"
#include "messages.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QThread>
#include <QDebug>

/**
 * Constructor – initializes state variables and UI
 */
BinaryFileReader::BinaryFileReader(QWidget *parent)
    : QWidget(parent),
    skipPercent(0),
    isProcessing(false),
    stopRequested(false),
    pauseRequested(false),
    cancelRequested(false)
{
    setWindowTitle("Binary File Reader");
    resize(700, 500);
    setupUI();
    setupConnections();
}

BinaryFileReader::~BinaryFileReader() {}

void BinaryFileReader::setupUI()
{
    // FILE SELECTION UI
    openFileButton = new QPushButton("Open File");
    fileNameEdit = new QLineEdit;
    fileNameEdit->setPlaceholderText("Select .bin file");

    // SKIP % INPUT
    skipLineEdit = new QLineEdit;
    skipLineEdit->setPlaceholderText("Skip %");
    skipLineEdit->setMaximumWidth(80);

    skipButton = new QPushButton("Skip");
    skipButton->setEnabled(false);

    // MAIN PROCESS BUTTON
    processButton = new QPushButton("Analysis File");
    processButton->setEnabled(false);

    // PLAYBACK CONTROLS
    pauseButton = new QPushButton("Pause");
    pauseButton->setEnabled(false);

    cancelButton = new QPushButton("Cancel");
    cancelButton->setEnabled(false);

    // SLIDER (VIDEO PLAYER STYLE SEEK)
    progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setRange(0, 100);
    progressSlider->setTracking(false); // Trigger only on release

    showCheckBox = new QCheckBox("Show");

    // ---- LAYOUT A (Recommended) ----
    QHBoxLayout *top = new QHBoxLayout;
    top->addWidget(openFileButton);
    top->addWidget(fileNameEdit);
    top->addWidget(skipLineEdit);
    top->addWidget(skipButton);

    QHBoxLayout *mid = new QHBoxLayout;
    mid->addWidget(processButton);
    mid->addWidget(progressSlider);
    mid->addWidget(pauseButton);
    mid->addWidget(cancelButton);
    mid->addWidget(showCheckBox);
    mid->addStretch();

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(top);
    main->addLayout(mid);
    main->addStretch();
}
void BinaryFileReader::setupConnections()
{
    connect(openFileButton,&QPushButton::clicked,this,&BinaryFileReader::openFile);
    connect(processButton,&QPushButton::clicked,this,&BinaryFileReader::analysisFile);
    connect(skipButton,&QPushButton::clicked,this,&BinaryFileReader::skipPressed);
    connect(progressSlider,&QSlider::sliderReleased,this,&BinaryFileReader::sliderMoved);
    connect(pauseButton,&QPushButton::clicked,this,&BinaryFileReader::togglePause);
    connect(cancelButton,&QPushButton::clicked,this,&BinaryFileReader::cancelProcessing);

    // Enable Analysis Button when filename typed
    connect(fileNameEdit,&QLineEdit::textChanged,this,[this](QString t){
        processButton->setEnabled(!t.isEmpty());
    });

    // Enable Skip Button only when skip% valid
    connect(skipLineEdit,&QLineEdit::textChanged,this,[this](QString t){
        bool ok; int p=t.toInt(&ok);
        skipButton->setEnabled(ok && p>=0 && p<100);
    });
}

void BinaryFileReader::openFile()
{
    binfilePath = QFileDialog::getOpenFileName(
        this,"Open Binary File","","Binary Files (*.bin);;All Files (*)");

    if(binfilePath.isEmpty()) return;

    fileNameEdit->setText(binfilePath);
    openFileButton->setStyleSheet("background:lightgreen;");
}
void BinaryFileReader::skipPressed()
{
    bool ok;
    int p = skipLineEdit->text().toInt(&ok);
    if(!ok) return;

    skipPercent = p;
    progressSlider->setValue(p);

    qDebug() << "Skip % set to" << skipPercent;
}
void BinaryFileReader::sliderMoved()
{
    // BEFORE analysis: only set skip%
    if(!isProcessing)
    {
        skipPercent = progressSlider->value();
        qDebug() << "Slider changed to" << skipPercent << "% before analysis";
        return;
    }

    // DURING analysis: stop + re-seek
    skipPercent = progressSlider->value();
    stopRequested = true;
    pauseRequested = false;
    cancelRequested = false;

    qDebug() << "Slider seek during processing → restart at" << skipPercent << "%";

    analysisFile();  // Restart read from new position
}
void BinaryFileReader::togglePause()
{
    pauseRequested = !pauseRequested;

    if(pauseRequested)
        pauseButton->setText("Resume");
    else
        pauseButton->setText("Pause");
}
void BinaryFileReader::cancelProcessing()
{
    cancelRequested = true;
    stopRequested = true;
}
void BinaryFileReader::analysisFile()
{
    // If analysis already running → request stop and return
    if(isProcessing)
    {
        stopRequested = true;
        return;
    }

    if(binfilePath.isEmpty())
    {
        QMessageBox::warning(this,"Warning","File not selected.");
        return;
    }

    // Reset control flags
    stopRequested = false;
    pauseRequested = false;
    cancelRequested = false;

    progressSlider->setValue(skipPercent);
    pauseButton->setEnabled(true);
    cancelButton->setEnabled(true);

    isProcessing = true;   // Mark analysis active

    // Open input file
    binfile.setFileName(binfilePath);
    if(!binfile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this,"Error","Cannot open binary file.");
        isProcessing = false;
        return;
    }

    // Prepare output text file
    QFileInfo info(binfilePath);
    spOutputFilePath = info.absolutePath() + "/" + info.baseName() + "_Output.txt";

    if(spOutputFile.isOpen()) spOutputFile.close();
    spOutputFile.setFileName(spOutputFilePath);
    spOutputFile.open(QIODevice::WriteOnly | QIODevice::Text);

    // Compute initial seek offset
    quint64 totalSize = binfile.size();
    quint64 skipBytes = (skipPercent * totalSize)/100;

    binfile.seek(skipBytes);

    QDataStream in(&binfile);
    in.setByteOrder(QDataStream::LittleEndian);

    quint32 msgID32;
    quint16 msgID16;

    // ---------------------------
    // MAIN BINARY READ LOOP
    // ---------------------------
    while(!in.atEnd())
    {
        if(cancelRequested) break;   // Hard stop
        if(stopRequested) break;     // Seek request

        while(pauseRequested)        // Pause loop
        {
            qApp->processEvents();
            QThread::msleep(20);
        }

        // Read 32-bit msg header
        in >> msgID32;

        // Check for valid DLOG_HEADER sync
        if(msgID32 == 0xEEEEEEEE)
        {
            // Read full header
            in.readRawData((char*)&strctLogHdr,sizeof(DLOG_HEADER));
            in >> msgID16;

            // PSP Submessage
            if(msgID16 == 0xAAA1)
            {
                // Read DWELL_DATA structure
                in.readRawData((char*)&strctDwlData,sizeof(DWELL_DATA));

                // Read additional PSP data
                in >> strctPspData.no_of_rpt;
                in >> strctPspData.RMS_IQ;

                for(int i=0;i<strctPspData.no_of_rpt && i<50;i++)
                    in.readRawData((char*)&strctPspData.SrchRpts[i],sizeof(RPTS));

                // Write parsed data to output
                writePspData(strctPspData, spOutputFile);
            }
        }

        // Update progress bar
        int percent = (in.device()->pos()*100)/totalSize;
        progressSlider->setValue(percent);

        qApp->processEvents();
    }

    spOutputFile.close();
    binfile.close();

    isProcessing = false;

    // If slider/skip requested restart → call again
    if(!cancelRequested && stopRequested)
        analysisFile();
}
void BinaryFileReader::writePspData(const PSP_DATA &data, QFile &file)
{
    QTextStream out(&file);

    out << "\n--- PSP DWELL DATA ---\n";
    out << "Dwell Count: " << data.dwell_data.Dwell_count << "\n";
    out << "Alpha: " << data.dwell_data.alpha << "\n";
    out << "Beta: " << data.dwell_data.beta << "\n";
    out << "Reports: " << data.no_of_rpt << "\n";

    for(int i=0;i<data.no_of_rpt && i<50;i++)
    {
        const RPTS &r = data.SrchRpts[i];
        out << "["<<i+1<<"] Range="<<r.m_frange
            << ", Strength="<<r.m_fStrength
            << ", Noise="<<r.m_fNoise << "\n";
    }
}

