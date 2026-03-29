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
    resize(1000, 650);
    setupUI();
    setupConnections();
    header=false;
}

BinaryFileReader::~BinaryFileReader() {}

void BinaryFileReader::setupUI()
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

void BinaryFileReader::setupConnections()
{
    connect(openFileButton,&QPushButton::clicked,this,&BinaryFileReader::openFile);
    connect(processButton,&QPushButton::clicked,this,&BinaryFileReader::analysisFile);
    connect(skipButton,&QPushButton::clicked,this,&BinaryFileReader::skipPressed);
    connect(progressSlider,&QSlider::sliderReleased,this,&BinaryFileReader::sliderMoved);
    connect(pauseButton,&QPushButton::clicked,this,&BinaryFileReader::togglePause);
    connect(cancelButton,&QPushButton::clicked,this,&BinaryFileReader::cancelProcessing);

    connect(fileNameEdit,&QLineEdit::textChanged,this,[this](QString t){
        processButton->setEnabled(!t.isEmpty());
    });

    connect(skipLineEdit,&QLineEdit::textChanged,this,[this](QString t){
        bool ok; int p=t.toInt(&ok);
        skipButton->setEnabled(ok && p>=0 && p<100);
    });

    connect(analysisRadio, &QRadioButton::toggled,
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
            });

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

void BinaryFileReader::openFile()
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

void BinaryFileReader::skipPressed()
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
void BinaryFileReader::sliderMoved()
{
    // BEFORE analysis: only set skip%
    skipPercent = progressSlider->value();
    if(!isProcessing)
    {
        return;
    }
    stopRequested = true;

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

    QDataStream in(&binFile);
    in.setByteOrder(QDataStream::LittleEndian);
    qint64 totalSize = binFile.size();
    quint64 skipBytes = (skipPercent * totalSize)/100;

    if(!binFile.seek(skipBytes)){
        QMessageBox::critical(this, "Error", "Failed to skipbytes in file");
        binFile.close();
        return;
    }

    quint64 byteReadTotal = skipBytes;

    while (!binFile.atEnd())
    {
        if (cancelRequested || stopRequested)
            break;

        while (pauseRequested) {
            qApp->processEvents();
            QThread::msleep(20);
        }
        quint32 msgID32;
        quint16 msgID16;

        in>>msgID32;
        if(msgID32 == 0xEEEEEEEE){
            in.device()->seek(in.device()->pos() - sizeof(msgID32));

            in.readRawData(reinterpret_cast<char*>(&strctLogHdr), sizeof(DLOG_HEADER));

            QTextStream out(&txtOutputFile);
            out<<strctLogHdr.m_ulTime;
            in>>msgID16;
            out<<toHex(msgID16)<<"\n";

            switch (msgID16)
            {

            case 0xAAA1:
            case 0xAAA2:        // PSP
            {
                in.device()->seek(in.device()->pos() - sizeof(WORD));

                in.readRawData(reinterpret_cast<char*>(&strctPspData.dwell_data),
                               sizeof(DWELL_DATA));

                in.readRawData(reinterpret_cast<char*>(&strctPspData.no_of_rpt),
                               sizeof(strctPspData.no_of_rpt));

                in.readRawData(reinterpret_cast<char*>(&strctPspData.RMS_IQ),
                               sizeof(strctPspData.RMS_IQ));

                for (int r = 0; r < strctPspData.no_of_rpt && r < 50; ++r)
                {
                    in.readRawData(reinterpret_cast<char*>(&strctPspData.srch_rpts[r]),
                                   sizeof(RPTS));

                    double pspRange = strctPspData.srch_rpts[r].m_frange;
                    double pspAzm   = strctPspData.srch_rpts[r].m_fDelAlpha;
                    double pspEle   = strctPspData.srch_rpts[r].m_fDelBeta;

                    // optional window filter
                    if (allTracks->isChecked())
                    {
                        if (isWithinWindow())
                        {
                            isPassed = checkWithinWindow(pspRange,
                                                         pspAzm,
                                                         pspEle,
                                                         strctPspData.dwell_data.dTime);
                        }
                        else
                        {
                            isPassed = true;
                        }
                    }
                    else
                    {
                        isPassed = true;
                    }

                    // Replay mode only
                    if (currentMode == RunMode::Replay && selectAllTracks->isChecked())
                    {
                        // check all selected Track_Request IDs
                        for (int i = 0; i < noOfTracks; i++)
                        {
                            int trackId = selectedTracks[i];

                            // if no Track_Request received for this trackId yet
                            if (!rangeTrkReq.contains(trackId))
                                continue;

                            double rangeReq = rangeTrkReq[trackId];
                            double azmReq   = azmTrkReq[trackId];
                            double eleReq   = eleTrkReq[trackId];

                            bool match =
                                (pspRange >= rangeReq - 1000 && pspRange <= rangeReq + 1000) &&
                                (pspAzm   >= azmReq   - 4    && pspAzm   <= azmReq   + 4) &&
                                (pspEle   >= eleReq   - 4    && pspEle   <= eleReq   + 4);

                            if (match)
                            {
                                // ---- Write in same TrackId file ----
                                if (trackFiles.contains(trackId))
                                {
                                    QFile *trackFile = trackFiles[trackId];

                                    QTextStream out(trackFile);

                                    out << "PSP_Data "
                                        << "TrackId " << trackId
                                        << " Time: "  << strctPspData.dwell_data.dTime
                                        << " Range: " << pspRange
                                        << " Azm: "   << pspAzm
                                        << " Ele: "   << pspEle
                                        << "\n";
                                }

                                // ---- Send to display ----
                                memset(&strctDisplayTrackData, 0, sizeof(strctDisplayTrackData));

                                strctDisplayTrackData.msgId   = 0xFD00;
                                strctDisplayTrackData.trkId   = trackId;
                                strctDisplayTrackData.trkType = PSP;
                                strctDisplayTrackData.time    = strctPspData.dwell_data.dTime;

                                oUtility.polarToCartesian(pspRange,
                                                          pspAzm,
                                                          pspEle,
                                                          &dX, &dY, &dZ);

                                strctDisplayTrackData.x = dX;
                                strctDisplayTrackData.y = dY;
                                strctDisplayTrackData.z = dZ;

                                if (isPassed)
                                {
                                    oDisplaySender.sendToDisplay(strctDisplayTrackData);
                                }

                                // one PSP report should match only one track
                                break;
                            }
                        }
                    }

                    // analysis mode existing behavior
                    if (currentMode == RunMode::Analysis)
                    {
                        writePspData(strctPspData, pspOutputFile);

                        if (isPassed)
                        {
                            writePspData(strctPspData, subStsOutputFile);
                        }
                    }
                }
            }
            break;

            case 0xAA11:      // Track Request
            {
                in.readRawData(reinterpret_cast<char*>(&strctTrkReq),
                               sizeof(Track_Request));

                in.readRawData(reinterpret_cast<char*>(&temp),
                               sizeof(temp));

                // window filter
                if (isWithinWindow())
                {
                    isPassed = checkWithinWindow(strctTrkReq.range,
                                                 strctTrkReq.azm,
                                                 strctTrkReq.ele,
                                                 strctTrkReq.dTime);
                }
                else
                {
                    isPassed = true;
                }

                // Analysis mode existing logic
                if (currentMode == RunMode::Analysis)
                {
                    writeTrackRequestData(strctTrkReq, stsOutputFile);

                    if (isPassed)
                    {
                        writeTrackRequestData(strctTrkReq, subStsOutputFile);
                    }
                }
                else if (currentMode == RunMode::Replay)
                {
                    int trackId = strctTrkReq.trkId;

                    // -------- Save request values for later PSP matching --------
                    if (trackFiles.contains(trackId))
                    {
                        rangeTrkReq[trackId] = strctTrkReq.range;
                        azmTrkReq[trackId]   = strctTrkReq.azm;
                        eleTrkReq[trackId]   = strctTrkReq.ele;
                    }

                    // -------- Display Data --------
                    memset(&strctDisplayTrackData, 0, sizeof(strctDisplayTrackData));

                    strctDisplayTrackData.msgId = 0xFD00;
                    strctDisplayTrackData.trkId = trackId;
                    strctDisplayTrackData.time  = strctTrkReq.dTime;

                    oUtility.polarToCartesian(strctTrkReq.range,
                                              strctTrkReq.azm,
                                              strctTrkReq.ele,
                                              &dX, &dY, &dZ);

                    strctDisplayTrackData.x = dX;
                    strctDisplayTrackData.y = dY;
                    strctDisplayTrackData.z = dZ;

                    // Track type
                    if (strctTrkReq.mode == 1)
                        strctDisplayTrackData.trkType = TrackReqRelook;
                    else if (strctTrkReq.mode == 2)
                        strctDisplayTrackData.trkType = TrackReqIQ;
                    else
                        strctDisplayTrackData.trkType = TrackReq;

                    // -------- Write in same TrackId file --------
                    if (trackFiles.contains(trackId))
                    {
                        QFile *trackFile = trackFiles[trackId];
                        QTextStream out(trackFile);

                        out << "Track_Request "
                            << "TrackId " << trackId << " ";

                        // Mode text
                        if (strctTrkReq.mode == 1)
                        {
                            out << "RLQ ";
                        }
                        else if (strctTrkReq.mode == 2)
                        {
                            if (strctTrkReq.initCount == 1)
                                out << "IQ1 ";
                            else if (strctTrkReq.initCount == 2)
                                out << "IQ2 ";
                            else if (strctTrkReq.initCount == 3)
                                out << "IQ3 ";
                            else if (strctTrkReq.initCount == 4)
                                out << "IQ4 ";
                        }
                        else if (strctTrkReq.mode == 3)
                            out << "RFC ";
                        else if (strctTrkReq.mode == 4)
                            out << "ACQ ";
                        else if (strctTrkReq.mode == 5)
                            out << "DTQ ";
                        else if (strctTrkReq.mode == 6)
                            out << "DGQ ";
                        else if (strctTrkReq.mode == 7)
                            out << "ADS ";
                        else if (strctTrkReq.mode == 8)
                            out << "CAL ";

                        out << "Time: "  << strctTrkReq.dTime
                            << " Range: " << strctTrkReq.range
                            << " Azm: "   << strctTrkReq.azm
                            << " Ele: "   << strctTrkReq.ele
                            << "\n";
                    }

                    // -------- Send to display --------
                    bool sendThisTrack = false;

                    if (allTracks->isChecked())
                    {
                        sendThisTrack = true;
                    }
                    else if (selectAllTracks->isChecked())
                    {
                        sendThisTrack = trackFiles.contains(trackId);
                    }

                    if (sendThisTrack && isPassed)
                    {
                        oDisplaySender.sendToDisplay(strctDisplayTrackData);
                    }
                }
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
        else{
            if(!in.atEnd()){
                in.device()->seek(in.device()->pos() - (sizeof(msgID32-1)));
            }
        }

        byteReadTotal = in.device()->pos();
        int progressPercent = static_cast<int>((byteReadTotal * 100) / totalSize);
        progressSlider->setValue(progressPercent);
        QApplication::processEvents();

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
            { &logOutputFile,   ".log"   },
            { &txtOutputFile, ".txt"}
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

bool BinaryFileReader::validateReplayInputs()
{
    // -------- Range --------
    if (chkRange->isChecked())
    {
        bool ok1, ok2;

        int minVal = rangeMinEdit->text().toInt(&ok1);
        int maxVal = rangeMaxEdit->text().toInt(&ok2);

        if (!ok1 || !ok2 || (minVal < 1000) || (maxVal > 2000) || minVal > maxVal)
        {
            QMessageBox::warning(this,
                                 "Range Error",
                                 "Range must be between 1000 and 2000");
            return false;
        }
    }

    // -------- Azimuth --------
    if (chkAzm->isChecked())
    {
        bool ok1, ok2;

        int minVal = azmMinEdit->text().toInt(&ok1);
        int maxVal = azmMaxEdit->text().toInt(&ok2);

        if (!ok1 || !ok2 || (minVal < 100) || (maxVal > 200) || minVal > maxVal)
        {
            QMessageBox::warning(this,
                                 "Azimuth Error",
                                 "Azm must be between 100 and 200");
            return false;
        }
    }

    // -------- Elevation --------
    if (chkEle->isChecked())
    {
        bool ok1, ok2;

        int minVal = eleMinEdit->text().toInt(&ok1);
        int maxVal = eleMaxEdit->text().toInt(&ok2);

        if (!ok1 || !ok2 || minVal > maxVal)
        {
            QMessageBox::warning(this,
                                 "Elevation Error",
                                 "Invalid elevation range");
            return false;
        }
    }

    // -------- Time --------
    if (chkTime->isChecked())
    {
        bool ok1, ok2;

        int minVal = timeMinEdit->text().toInt(&ok1);
        int maxVal = timeMaxEdit->text().toInt(&ok2);

        if (!ok1 || !ok2 || minVal > maxVal)
        {
            QMessageBox::warning(this,
                                 "Time Error",
                                 "Invalid time range");
            return false;
        }
    }

    return true;   // ✅ all good
}

void BinaryFileReader::onReplayMode()
{
    // ---- Analysis controls disabled ----
    generateAllFiles->setEnabled(false);
    showCheckBox->setEnabled(false);
    generateAllFiles->setChecked(false);
    showCheckBox->setChecked(false);

    // ---- Replay controls enabled ----
    allTracks->setEnabled(true);
    selectAllTracks->setEnabled(true);
    trackLineEdit->setEnabled(selectAllTracks->isChecked());
    //trackLineEdit->setEnabled(true);

    chkRange->setEnabled(true);
    chkAzm->setEnabled(true);
    chkEle->setEnabled(true);
    chkTime->setEnabled(true);

    // ⭐ Min/Max should remain disabled until checkbox ticked
    rangeMinEdit->setEnabled(false);
    rangeMaxEdit->setEnabled(false);

    azmMinEdit->setEnabled(false);
    azmMaxEdit->setEnabled(false);

    eleMinEdit->setEnabled(false);
    eleMaxEdit->setEnabled(false);

    timeMinEdit->setEnabled(false);
    timeMaxEdit->setEnabled(false);
}

void BinaryFileReader::onAnalysisMode()
{
    // ---- Analysis controls enabled ----
    generateAllFiles->setEnabled(true);
    showCheckBox->setEnabled(true);

    // ---- Replay controls disabled ----
    allTracks->setEnabled(false);
    selectAllTracks->setEnabled(false);
    trackLineEdit->setEnabled(false);

    chkRange->setEnabled(true);
    chkAzm->setEnabled(true);
    chkEle->setEnabled(true);
    chkTime->setEnabled(true);

    // ---- Reset replay selections ----
    allTracks->setChecked(false);
    selectAllTracks->setChecked(false);

    chkRange->setChecked(false);
    chkAzm->setChecked(false);
    chkEle->setChecked(false);
    chkTime->setChecked(false);

    trackLineEdit->clear();

    rangeMinEdit->clear();
    rangeMaxEdit->clear();

    azmMinEdit->clear();
    azmMaxEdit->clear();

    eleMinEdit->clear();
    eleMaxEdit->clear();

    timeMinEdit->clear();
    timeMaxEdit->clear();

    // ---- Keep disabled ----
    rangeMinEdit->setEnabled(false);
    rangeMaxEdit->setEnabled(false);

    azmMinEdit->setEnabled(false);
    azmMaxEdit->setEnabled(false);

    eleMinEdit->setEnabled(false);
    eleMaxEdit->setEnabled(false);

    timeMinEdit->setEnabled(false);
    timeMaxEdit->setEnabled(false);
}
bool BinaryFileReader::checkWithinWindow(double range, double azm, double ele, double time) const
{
    // Range
    if (chkRange->isChecked())
    {
        double minVal = rangeMinEdit->text().toDouble();
        double maxVal = rangeMaxEdit->text().toDouble();

        if (range < minVal || range > maxVal)
            return false;
    }

    // Azimuth
    if (chkAzm->isChecked())
    {
        double minVal = azmMinEdit->text().toDouble();
        double maxVal = azmMaxEdit->text().toDouble();

        if (azm < minVal || azm > maxVal)
            return false;
    }

    // Elevation
    if (chkEle->isChecked())
    {
        double minVal = eleMinEdit->text().toDouble();
        double maxVal = eleMaxEdit->text().toDouble();

        if (ele < minVal || ele > maxVal)
            return false;
    }

    // Time
    if (chkTime->isChecked())
    {
        double minVal = timeMinEdit->text().toDouble();
        double maxVal = timeMaxEdit->text().toDouble();

        if (time < minVal || time > maxVal)
            return false;
    }

    return true;   // passed all filters
}
