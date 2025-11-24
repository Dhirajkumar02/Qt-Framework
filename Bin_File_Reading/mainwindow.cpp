#include "mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QThread>
#include <QDebug>
#include <QFileInfo>
#include <algorithm>

BinaryFileReader::BinaryFileReader(QWidget *parent)
    : QWidget(parent),
    headerWritten(false),
    skipPercent(0),
    isProcessing(false),
    stopRequested(false),
    pauseRequested(false),
    cancelRequested(false)
{
    setWindowTitle("Binary File Reader");
    resize(800, 600);
    setupUI();
    setupConnections();
}

BinaryFileReader::~BinaryFileReader() {}

void BinaryFileReader::setupUI()
{
    openFileButton = new QPushButton("Open File");
    fileNameEdit = new QLineEdit;
    fileNameEdit->setPlaceholderText("Select .bin file");

    skipLineEdit = new QLineEdit;
    skipLineEdit->setPlaceholderText("Skip %");
    skipLineEdit->setMaximumWidth(80);

    skipButton = new QPushButton("Skip");
    skipButton->setEnabled(false);

    processButton = new QPushButton("Analysis File");
    processButton->setEnabled(false);

    pauseButton = new QPushButton("Pause");
    pauseButton->setEnabled(false);

    cancelButton = new QPushButton("Cancel");
    cancelButton->setEnabled(false);

    progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setRange(0, 100);
    progressSlider->setTracking(false);

    showCheckBox = new QCheckBox("Show (details)");

    missingLabel = new QLabel("Missing Dwell Count:");
    missingLabel->setStyleSheet(
        "font-size: 15px;"
        "font-weight: bold;"
        "color: #222;"
        "padding: 4px;"
        );

    missingListWidget = new QListWidget();
    missingListWidget->setMinimumHeight(150);

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
    main->addWidget(missingLabel);
    main->addWidget(missingListWidget);
    main->addStretch();
}

void BinaryFileReader::setupConnections()
{
    connect(openFileButton, &QPushButton::clicked, this, &BinaryFileReader::openFile);
    connect(processButton, &QPushButton::clicked, this, &BinaryFileReader::analysisFile);
    connect(skipButton, &QPushButton::clicked, this, &BinaryFileReader::skipPressed);
    connect(progressSlider, &QSlider::sliderReleased, this, &BinaryFileReader::sliderMoved);
    connect(pauseButton, &QPushButton::clicked, this, &BinaryFileReader::togglePause);
    connect(cancelButton, &QPushButton::clicked, this, &BinaryFileReader::cancelProcessing);

    connect(fileNameEdit, &QLineEdit::textChanged, this, [this](const QString &t){
        processButton->setEnabled(!t.isEmpty());
    });

    connect(skipLineEdit, &QLineEdit::textChanged, this, [this](const QString &t){
        bool ok; int p = t.toInt(&ok);
        skipButton->setEnabled(ok && p >= 0 && p < 100);
    });
}

void BinaryFileReader::openFile()
{
    filePath = QFileDialog::getOpenFileName(
        this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*)");

    if (filePath.isEmpty()) return;

    fileNameEdit->setText(filePath);
    openFileButton->setStyleSheet("background: lightgreen;");
}

void BinaryFileReader::skipPressed()
{
    bool ok;
    int p = skipLineEdit->text().toInt(&ok);
    if (!ok) return;
    skipPercent = p;
    progressSlider->setValue(p);
    qDebug() << "Skip % set to" << skipPercent;
}

void BinaryFileReader::sliderMoved()
{
    if (!isProcessing) {
        skipPercent = progressSlider->value();
        qDebug() << "Slider changed to" << skipPercent;
        return;
    }
    skipPercent = progressSlider->value();
    stopRequested = true;
    pauseRequested = false;
    cancelRequested = false;
    qDebug() << "Slider seek during processing → restart at" << skipPercent << "%";
    analysisFile(); // restart
}

void BinaryFileReader::togglePause()
{
    pauseRequested = !pauseRequested;
    pauseButton->setText(pauseRequested ? "Resume" : "Pause");
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

    // reset state/UI containers
    missingListWidget->clear();
    frames.clear();
    headerWritten = false;

    stopRequested = false;
    pauseRequested = false;
    cancelRequested = false;

    progressSlider->setValue(skipPercent);
    pauseButton->setEnabled(true);
    cancelButton->setEnabled(true);

    isProcessing = true;

    // open binary file
    binFile.setFileName(filePath);
    if (!binFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Cannot open binary file.");
        isProcessing = false;
        return;
    }

    // choose output filename
    QFileInfo fi(filePath);
    if (showCheckBox->isChecked())
        pspOutputFilePath = fi.absolutePath() + "/" + fi.completeBaseName() + "_details.txt";
    else
        pspOutputFilePath = fi.absolutePath() + "/" + fi.completeBaseName() + ".txt";

    if (pspOutputFile.isOpen()) pspOutputFile.close();
    pspOutputFile.setFileName(pspOutputFilePath);
    if (!pspOutputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot create output file for writing.");
        isProcessing = false;
        binFile.close();
        return;
    }

    QDataStream in(&binFile);
    in.setByteOrder(QDataStream::LittleEndian);

    const quint64 totalSize = binFile.size();
    const quint64 skipBytes = (skipPercent * totalSize) / 100;

    if (!binFile.seek(skipBytes)) {
        QMessageBox::critical(this, "Error", "Failed to skip bytes");
        binFile.close();
        pspOutputFile.close();
        isProcessing = false;
        return;
    }

    quint64 byteReadTotal = skipBytes;
    quint32 msgID32;
    quint16 msgID16;
    qint64 posAfter;

    // READ LOOP — collect AAA1 frames
    while (!binFile.atEnd()) {
        if (cancelRequested) break;
        if (stopRequested) break;

        while (pauseRequested) {
            qApp->processEvents();
            QThread::msleep(20);
        }

        in >> msgID32; // read next 4 bytes

        if (msgID32 == 0xEEEEEEEE) {
            // rewind to start of header and read full header struct
            posAfter = in.device()->pos();
            in.device()->seek(posAfter - static_cast<qint64>(sizeof(msgID32)));

            in.readRawData(reinterpret_cast<char*>(&strctLogHdr), sizeof(DLOG_HEADER));

            // read the 16-bit sub-message id
            in >> msgID16;

            if (msgID16 == 0xAAA1) {
                // rewind to submessage id start (keep aligned)
                posAfter = in.device()->pos();
                in.device()->seek(posAfter - static_cast<qint64>(sizeof(msgID16)));

                // read DWELL_DATA (packed)
                in.readRawData(reinterpret_cast<char*>(&strctDwlData), sizeof(DWELL_DATA));

                // read no_of_rpt and RMS_IQ
                WORD no_of_rpt_local = 0;
                in.readRawData(reinterpret_cast<char*>(&no_of_rpt_local), sizeof(no_of_rpt_local));

                float rms_local = 0.0f;
                in.readRawData(reinterpret_cast<char*>(&rms_local), sizeof(rms_local));

                // read RPTS entries (count limited to 50)
                int count = qMin<int>(static_cast<int>(no_of_rpt_local), 50);
                QVector<RPTS> rpts_local;
                rpts_local.resize(count);
                for (int i = 0; i < count; ++i) {
                    in.readRawData(reinterpret_cast<char*>(&rpts_local[i]), sizeof(RPTS));
                }

                // build Frame and append
                Frame f;
                f.hdr = strctLogHdr;
                f.dwell = strctDwlData;
                f.no_of_rpt = no_of_rpt_local;
                f.RMS_IQ = rms_local;
                f.rpts = std::move(rpts_local);

                frames.append(std::move(f));
            }
            // else: skip other submessages
        } else {
            // move back a byte and continue searching
            if (in.atEnd()) break;
            posAfter = in.device()->pos();
            in.device()->seek(posAfter - static_cast<qint64>(sizeof(msgID32) - 1));
        }

        byteReadTotal = in.device()->pos();
        int progressPercent = totalSize ? static_cast<int>((byteReadTotal * 100) / totalSize) : 0;
        progressSlider->setValue(progressPercent);

        qApp->processEvents();
    }

    // finished reading raw frames
    binFile.close();

    // sort by global header timestamp m_ulTime
    std::sort(frames.begin(), frames.end(), [](const Frame &a, const Frame &b){
        return a.hdr.m_ulTime < b.hdr.m_ulTime;
    });

    // continuity check — detect missing dwell ranges
    if (frames.size() > 1) {
        quint32 prev = frames[0].dwell.Dwell_count;
        for (int i = 1; i < frames.size(); ++i) {
            quint32 curr = frames[i].dwell.Dwell_count;
            if (curr > prev + 1) {
                for (quint32 miss = prev + 1; miss < curr; ++miss) {
                    QListWidgetItem *item = new QListWidgetItem("Missing Dwell: " + QString::number(miss));
                    item->setForeground(Qt::red);
                    missingListWidget->addItem(item);
                }
            }
            // handle duplicates/wrap simply by updating prev
            prev = curr;
        }
    }

    // write sorted frames to output file
    QTextStream out(&pspOutputFile);
    out.setRealNumberPrecision(4);
    out.setRealNumberNotation(QTextStream::FixedNotation);

    if (!frames.isEmpty()) {
        out << "PSP DATA OUTPUT (sorted by m_ulTime)\n";
    }

    for (const Frame &f : frames) {
        writeFrameData(f, out);
    }

    pspOutputFile.close();
    isProcessing = false;
    progressSlider->setValue(100);

    if (!cancelRequested && stopRequested) {
        analysisFile();
    }
}

// The function signature you wanted: use 'struct Frame' type
void BinaryFileReader::writeFrameData(const struct Frame &pspData, QTextStream &outputFile)
{
    // write detailed or compact based on checkbox
    if (showCheckBox->isChecked()) {
        outputFile << "MsgID: " << toHex(pspData.dwell.Word0_Msg_id, 4)
        << "  Time: " << pspData.hdr.m_ulTime
        << "  DwellCount: " << pspData.dwell.Dwell_count
        << "  no_of_rpt: " << pspData.no_of_rpt
        << "  RMS_IQ: " << pspData.RMS_IQ << "\n";

        for (int i = 0; i < pspData.rpts.size(); ++i) {
            const RPTS &r = pspData.rpts.at(i);
            outputFile << "  rpt[" << i << "] "
                       << r.m_frange << " "
                       << r.m_fStrength << " "
                       << r.m_fNoise << " "
                       << r.m_fDelAlpha << " "
                       << r.m_fDelBeta << " "
                       << r.m_fFilterNo << "\n";
        }
        outputFile << "\n";
    } else {
        outputFile << "Time:" << pspData.hdr.m_ulTime
                   << " Dwell:" << pspData.dwell.Dwell_count
                   << " Rpts:" << pspData.no_of_rpt
                   << " RMS_IQ:" << pspData.RMS_IQ << "\n";
    }
}

QString BinaryFileReader::toHex(quint32 value, int width)
{
    return QString("0x%1").arg(value, width, 16, QLatin1Char('0')).toUpper();
}
