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
#include <QListWidgetItem>
#include <QListWidget>

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
    header=false;
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

    // SLIDER
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
    filePath = QFileDialog::getOpenFileName(
        this,"Open Binary File","","Binary Files (*.bin);;All Files (*)");

    if(filePath.isEmpty()) return;

    fileNameEdit->setText(filePath);
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
    if (isProcessing) {
        stopRequested = true;
        return;
    }

    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "File not selected.");
        return;
    }

    stopRequested   = false;
    pauseRequested  = false;
    cancelRequested = false;
    isProcessing    = true;

    // ---- Open binary file ----
    binFile.setFileName(filePath);
    if (!binFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open binary file.");
        isProcessing = false;
        return;
    }

    // ---- Open output files (helper) ----
    QFileInfo fileInfo(filePath);
    if (!openAllOutputFiles(fileInfo))
    {
        binFile.close();
        isProcessing = false;
        return;
    }


    const quint64 totalSize = binFile.size();

    // ---- Buffered byte-wise scan ----
    const int BUF_SIZE = 64 * 1024;   // 64 KB
    QByteArray buffer(BUF_SIZE, 0);

    quint32 syncReg = 0;
    quint64 processed = 0;

    while (!binFile.atEnd())
    {
        if (cancelRequested || stopRequested)
            break;

        while (pauseRequested) {
            qApp->processEvents();
            QThread::msleep(20);
        }

        qint64 n = binFile.read(buffer.data(), buffer.size());
        if (n <= 0)
            break;

        for (qint64 i = 0; i < n; ++i)
        {
            quint8 byte = static_cast<quint8>(buffer[i]);

            // ---- rolling window (little-endian safe) ----
            syncReg = (syncReg >> 8) | (quint32(byte) << 24);

            if (syncReg != 0xEEEEEEEE)
                continue;

            // ===== SYNC FOUND =====
            syncReg = 0;

            // ---- Read rest of DLOG_HEADER (except sync) ----
            if (binFile.read(reinterpret_cast<char*>(&strctLogHdr.m_ulTime),
                             sizeof(DLOG_HEADER) - sizeof(quint32))
                != (sizeof(DLOG_HEADER) - sizeof(quint32)))
                break;

            // ---- Read 16-bit message ID ----
            quint16 msgID16;
            if (binFile.read(reinterpret_cast<char*>(&msgID16), sizeof(msgID16))
                != sizeof(msgID16))
                break;

            msgID16 = qFromLittleEndian(msgID16);

            // ---- Handle sub-messages ----
            switch (msgID16)
            {
            case 0xAAA1:   // PSP
            {
                if (binFile.read(reinterpret_cast<char*>(&strctDwlData),
                                 sizeof(DWELL_DATA)) != sizeof(DWELL_DATA))
                    break;

                binFile.read(reinterpret_cast<char*>(&strctPspData.no_of_rpt),
                             sizeof(strctPspData.no_of_rpt));

                binFile.read(reinterpret_cast<char*>(&strctPspData.RMS_IQ),
                             sizeof(strctPspData.RMS_IQ));

                for (int r = 0; r < strctPspData.no_of_rpt && r < 50; ++r)
                {
                    binFile.read(reinterpret_cast<char*>(&strctPspData.srch_rpts[r]),
                                 sizeof(RPTS));
                }

                // ---- write outputs ----
                writePspData(strctPspData, pspOutputFile);

            }
            break;

                // ---- ADD YOUR OTHER 10+ CASES HERE ----
                // case 0xAAAA:
                // case 0xA801:
                // case 0xAA03:
                // ...

            default:
                break;
            }
        }

        processed += n;

        // ---- UI update (throttled) ----
        static int uiTick = 0;
        if (++uiTick % 200 == 0) {
            int progress = static_cast<int>((processed * 100) / totalSize);
            progressSlider->setValue(progress);
            QApplication::processEvents();
        }
    }

    progressSlider->setValue(100);

    binFile.close();
    pspOutputFile.close();
    stsOutputFile.close();
    //spOutputFile.close();
    //spCenOutputFile.close();

    isProcessing = false;

    if (!cancelRequested && stopRequested)
        analysisFile();
}

void BinaryFileReader::writePspData(const PSP_DATA &data, QFile &pspOutputFile)
{
    QTextStream out(&pspOutputFile);
    out.setRealNumberPrecision(4);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setFieldAlignment(QTextStream::AlignRight);

    if(header){
        if(showCheckBox->isChecked()){
            out<<"1. word0_msg_id"<<" 2. Word1(beamType:4, FncNmbr: 4, OpMode:1, CfarThreshold:4, CFarType:3)"<<" 3. Word2(wBeamNo:8, wTrkId:8)"
                <<" 4. Word3(ECCM_OnOff:1, WECCMType:5, NECCMFreq:2, WJmdLocn_OnOff:1, WECCM_Auto_Mnl: 1, WZoneNo: 2, wBroadBeam:2, wReserved7_5:2)"
                <<" 5. Word4(Integration:4, ZFiltDrop:1, MapCntr:1, MapThreshold:3, IMTS:1, AMTI:1, MTI:2, VideoSelection:1, SLB_OnOff:1, Splog:1)"
                <<" 6. Word4(WDPC:1, BinaryThreshold:7, RangeWindowSize:3, Reserved_1:1, wHybridMode: 1, wDPCWindowOnOff:1, SysMode:2)"
                <<" 7. StartPredictedRange "<<" 8. StopPredictedRange"<<" 9. alpha "<<" 10. beta "<<" 11. boresight "<<" 12. Pitch "<<" 13. Roll "
                <<" 14. dTime "<<" 15. Word13(BTTE_attenuation : 6, DopplerCode:4, TgtSpacing:2, TgtInOut:1, BITE_RF_Field:1, BITE_OnOff:1, STAMO_BITE:1)"
                <<" 16. Word14 "<<" 17. Word15 "<<" 18. Word16 "<<" 19. Word17 "<<" 20. Word18 "<<" 21. DC "<<" 22. Word19 "<<"\n";
        }else
        {
            out<<"1. word0_msg_id"<<" 2. Word1 "<<" 3. Word2 "<<" 4. Word3 "<<" 5. Word4"<<" 6. Word4"
                <<" 7. StartPredictedRange "<<" 8. StopPredictedRange "<<" 9. alpha "<<" 10. beta "
                <<" 11. boresight "<<" 12. Pitch "<<" 13. Roll "<<" 14. dTime "<<" 15. Word13 "
                <<" 16. Word14 "<<" 17. Word15 "<<" 18. Word16 "<<" 19. Word17 "<<" 20. Word18 "<<" 21. DC "<<" 22. Word19 "<<"\n";
        }
        header = false;
    }
    if(showCheckBox->isChecked()){
        out<<"1. "<<toHex(strctPspData.dwell_data.Word0_Msg_id)<<toHex(strctPspData.dwell_data.Word1.bits.beamType)<<toHex(strctPspData.dwell_data.Word1.bits.FncNmbr)<<" ";
        out<<" 23. "<<strctPspData.no_of_rpt;
        for(int i=0;i<strctPspData.no_of_rpt && i < 50;i++)
        {
            const RPTS &r = data.srch_rpts[i];
            out <<" "<<r.m_frange
                <<" "<<r.m_fStrength
                <<" "<<r.m_fNoise
                <<" "<<r.m_fDelAlpha<<" "<<r.m_fDelBeta<<" "<<r.m_fFilterNo
                << "\n";
        }
    }
    else{
        out<<"1. "<<strctPspData.dwell_data.Word0_Msg_id<<" 2. "<<strctPspData.dwell_data.Word1.word
            <<" 3. "<<strctPspData.dwell_data.Word2.word<<" 4. "<<strctPspData.dwell_data.Word3.word
            <<" 5. "<<strctPspData.dwell_data.Word4.word<<" 6. "<<strctPspData.dwell_data.Word5.word
            <<" 7. "<<strctPspData.dwell_data.StartPredictedRange<<" 8. "<<strctPspData.dwell_data.StopPredictedRange
            <<" 9. "<<strctPspData.dwell_data.alpha<<" 10. "<<strctPspData.dwell_data.beta<<" 11. "
            <<strctPspData.dwell_data.boresight<<" 12. "<<strctPspData.dwell_data.pitch<<" 13. "
            <<strctPspData.dwell_data.roll<<" 14. "<<strctPspData.dwell_data.dTime<<" 15. "<<strctPspData.dwell_data.Word13.word
            <<" 16. "<<strctPspData.dwell_data.Word14.word<<" 17. "<<strctPspData.dwell_data.Word15.word<<" 18. "
            <<strctPspData.dwell_data.Word16.word<<" 19. "<<strctPspData.dwell_data.Word17.word<<" 20. "
            <<strctPspData.dwell_data.Word18.word <<" 21. "<<strctPspData.dwell_data.Dwell_count<<" 22. "
            <<strctPspData.dwell_data.Word19.word<<"\n";

        out<<" 23. "<<strctPspData.no_of_rpt;
        for(int i=0;i<strctPspData.no_of_rpt && i < 50;i++)
        {
            const RPTS &r = data.srch_rpts[i];
            out <<" "<<r.m_frange
                <<" "<<r.m_fStrength
                <<" "<<r.m_fNoise
                <<" "<<r.m_fDelAlpha<<" "<<r.m_fDelBeta<<" "<<r.m_fFilterNo
                << " ";
        }
        out<<"\n";
    }
}

QString BinaryFileReader::toHex(quint32 value, int width){
    return QString("0x%1").arg(value, width, 16, QLatin1Char('0')).toUpper();
}

bool BinaryFileReader::openAllOutputFiles(const QFileInfo &fileInfo)
{
    struct OutputSpec
    {
        QFile* file;
        QString extension;
    };

    const QVector<OutputSpec> files =
        {
            { &pspOutputFile,   ".txt"   },
            { &stsOutputFile,   ".sts"   },
            { &spOutputFile,    ".sp"    },
            { &spCenOutputFile, ".spcen" },
            { &logOutputFile,   ".log"   }
        };

    for (const auto &out : files)
    {
        if (!openOutputFile(*out.file, fileInfo, out.extension))
        {
            QMessageBox::critical(
                this,
                "File Error",
                "Cannot create output file: " + out.extension
                );
            return false;
        }
    }
    return true;
}
bool BinaryFileReader::openOutputFile(QFile &file,
                                      const QFileInfo &fileInfo,
                                      const QString &extension)
{
    const QString path =
        fileInfo.absolutePath() + "/" +
        fileInfo.completeBaseName() + extension;

    if (file.isOpen())
        file.close();

    file.setFileName(path);

    return file.open(QIODevice::WriteOnly | QIODevice::Text);
}
