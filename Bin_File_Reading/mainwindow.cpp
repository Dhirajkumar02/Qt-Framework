#include "mainwindow.h"
#include "messages.h"
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QDebug>
#include <QSlider>
#include <QCheckBox>

BinaryFileReader::BinaryFileReader(QWidget *parent)
    : QWidget(parent), skipPercent(0)
{
    setWindowTitle("Binary File Reader");
    resize(700, 500);

    setupUI();
    setupConnections();
}

BinaryFileReader::~BinaryFileReader() = default;

// -----------------------------
// Setup UI Elements
// -----------------------------
void BinaryFileReader::setupUI()
{
    // File Selection
    openFileButton = new QPushButton("Open File");
    fileNameEdit = new QLineEdit;
    fileNameEdit->setPlaceholderText("Select a .bin file");

    showCheckBox = new QCheckBox("Show");
    showCheckBox->setChecked(false); // default unchecked

    // Skip Percentage
    skipLineEdit = new QLineEdit;
    skipLineEdit->setPlaceholderText("Skip by %");
    skipLineEdit->setMaximumWidth(100);

    skipButton = new QPushButton("Skip");
    skipButton->setEnabled(false);

    // Process File
    processButton = new QPushButton("Analysis File");
    processButton->setFixedWidth(120);
    processButton->setEnabled(false);

    // Interactive Progress Slider
    progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setRange(0, 100);
    progressSlider->setValue(0);
    progressSlider->setFixedWidth(200);
    progressSlider->setTracking(false); // update only when released

    // Layouts
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(openFileButton);
    topLayout->addWidget(fileNameEdit);
    topLayout->addWidget(skipLineEdit);
    topLayout->addWidget(skipButton);

    QHBoxLayout *processLayout = new QHBoxLayout;
    processLayout->addWidget(processButton);
    processLayout->addWidget(progressSlider);
    processLayout->addSpacing(10);
    processLayout->addWidget(showCheckBox);
    processLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(processLayout);
    mainLayout->addStretch();
}

// -----------------------------
// Setup Signal-Slot Connections
// -----------------------------
void BinaryFileReader::setupConnections()
{
    connect(openFileButton, &QPushButton::clicked, this, &BinaryFileReader::openFile);
    connect(skipButton, &QPushButton::clicked, this, &BinaryFileReader::skipPercentage);
    connect(processButton, &QPushButton::clicked, this, &BinaryFileReader::analysisFile);

    // Enable process button only when a file is selected
    connect(fileNameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        processButton->setEnabled(!text.isEmpty());
    });

    // Enable skip button only for valid percentage values (0-99)
    connect(skipLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        bool ok;
        int percent = text.toInt(&ok);
        skipButton->setEnabled(ok && percent >= 0 && percent < 100);
    });

    // Handle user drag on progress slider
    connect(progressSlider, &QSlider::sliderReleased, this, &BinaryFileReader::onSliderReleased);
}

// -----------------------------
// Browse and select a binary file
// -----------------------------
void BinaryFileReader::openFile()
{
    binfilePath = QFileDialog::getOpenFileName(this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*)");
    if (binfilePath.isEmpty())
        return;

    fileNameEdit->setText(binfilePath);
    processButton->setEnabled(true);

    openFileButton->setStyleSheet("background-color: lightgreen;");
}

// -----------------------------
// Set skip percentage manually (Skip Button)
// -----------------------------
void BinaryFileReader::skipPercentage()
{
    bool ok;
    int percent = skipLineEdit->text().toInt(&ok);

    if (ok && percent >= 0 && percent < 100) {
        skipPercent = percent;
        progressSlider->setValue(skipPercent);  // show skip progress immediately
        qDebug() << "Skip percentage set to:" << skipPercent;
    } else {
        skipPercent = 0;
        progressSlider->setValue(0);
        qDebug() << "Invalid skip percentage, defaulting to 0";
    }

    qApp->processEvents(); // refresh UI
}

// -----------------------------
// Process and display the binary file
// -----------------------------
void BinaryFileReader::analysisFile()
{
    if (binfilePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No file selected.");
        return;
    }

    binfile.setFileName(binfilePath);
    if (!binfile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open file!");
        return;
    }

    QFileInfo fileInfo(binfilePath);
    QString folderPath = fileInfo.absolutePath();
    spOutputFilePath = folderPath + "/PspOutput.txt";

    spOutputFile.setFileName(spOutputFilePath);
    if (!spOutputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open output file for writing");
        return;
    }

    quint64 totalSize = binfile.size();
    quint64 skipBytes = (skipPercent * totalSize) / 100;

    if (!binfile.seek(skipBytes)) {
        QMessageBox::critical(this, "Error", "Failed to skip bytes in file!");
        binfile.close();
        return;
    }

    progressSlider->setValue(skipPercent);

    QDataStream in(&binfile);
    in.setByteOrder(QDataStream::LittleEndian);

    quint64 bytesReadTotal = skipBytes;
    qint64 posAfter;
    quint32 msgID32;
    quint16 msgID16;

    while (!in.atEnd()) {
        in >> msgID32;

        if (msgID32 == 0xEEEEEEEE) {
            posAfter = in.device()->pos();
            in.device()->seek(posAfter - sizeof(msgID32));

            in.readRawData(reinterpret_cast<char*>(&strctLogHdr), sizeof(DLOG_HEADER));
            in >> msgID16;

            switch (msgID16) {
            case 0xAAA1:
                posAfter = in.device()->pos();
                in.device()->seek(posAfter - sizeof(msgID16));

                in.readRawData(reinterpret_cast<char*>(&strctDwlData), sizeof(DWELL_DATA));
                in >> strctPspData.no_of_rpt;
                in >> strctPspData.RMS_IQ;

                for (int i = 0; i < strctPspData.no_of_rpt; ++i) {
                    in.readRawData(reinterpret_cast<char*>(&strctRpts), sizeof(RPTS));
                }

                writePspData(strctPspData, spOutputFile);
                break;

            default:
                qDebug() << "Message type not matched";
                break;
            }

        } else {
            if (in.atEnd()) break;
            posAfter = in.device()->pos();
            in.device()->seek(posAfter - sizeof(msgID32) - 1);
        }

        // Update progress bar relative to skipPercent
        bytesReadTotal = in.device()->pos();
        int progressPercent = skipPercent + static_cast<int>(
                                  ((bytesReadTotal - skipBytes) * (100 - skipPercent)) / (totalSize - skipBytes)
                                  );
        progressSlider->setValue(progressPercent);

        qApp->processEvents(); // keep UI responsive
    }

    progressSlider->setValue(100);
    binfile.close();
    spOutputFile.close();

    qDebug() << "Skipped first" << skipPercent << "% (" << skipBytes << " bytes)";
}

// -----------------------------
// Handle mouse drag on slider for skip
// -----------------------------
void BinaryFileReader::onSliderReleased()
{
    int newPercent = progressSlider->value();
    skipPercent = newPercent;
    qDebug() << "User dragged slider to:" << newPercent << "%";

    if (!binfilePath.isEmpty()) {
        binfile.setFileName(binfilePath);
        if (!binfile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Error", "Failed to open file for seeking!");
            return;
        }

        quint64 totalSize = binfile.size();
        quint64 newPosition = (newPercent * totalSize) / 100;

        if (!binfile.seek(newPosition)) {
            QMessageBox::critical(this, "Error", "Failed to seek to new position in file!");
            return;
        }

        reprocessFromPosition(newPosition);
        binfile.close();
    } else {
        QMessageBox::warning(this, "Warning", "Please open and start file analysis first.");
    }
}

// -----------------------------
// Reprocess file from specific position (for drag skip)
// -----------------------------
void BinaryFileReader::reprocessFromPosition(quint64 position)
{
    if (binfilePath.isEmpty()) return;

    QFile tempFile(binfilePath);
    if (!tempFile.open(QIODevice::ReadOnly)) return;

    tempFile.seek(position);

    QDataStream in(&tempFile);
    in.setByteOrder(QDataStream::LittleEndian);

    quint64 totalSize = tempFile.size();
    quint64 bytesReadTotal = position;
    quint32 msgID32;
    quint16 msgID16;
    qint64 posAfter;

    while (!in.atEnd()) {
        in >> msgID32;

        if (msgID32 == 0xEEEEEEEE) {
            posAfter = in.device()->pos();
            in.device()->seek(posAfter - sizeof(msgID32));
            in.readRawData(reinterpret_cast<char*>(&strctLogHdr), sizeof(DLOG_HEADER));
            in >> msgID16;

            if (msgID16 == 0xAAA1) {
                posAfter = in.device()->pos();
                in.device()->seek(posAfter - sizeof(msgID16));

                in.readRawData(reinterpret_cast<char*>(&strctDwlData), sizeof(DWELL_DATA));
                in >> strctPspData.no_of_rpt;
                in >> strctPspData.RMS_IQ;

                for (int i = 0; i < strctPspData.no_of_rpt; ++i) {
                    in.readRawData(reinterpret_cast<char*>(&strctRpts), sizeof(RPTS));
                }
                writePspData(strctPspData, spOutputFile);
            }
        }

        bytesReadTotal = in.device()->pos();
        int progressPercent = static_cast<int>((bytesReadTotal * 100) / totalSize);
        progressSlider->setValue(progressPercent);
        qApp->processEvents();
    }

    progressSlider->setValue(100);
    qDebug() << "Reprocessed from position" << position;
}

// -----------------------------
// Write PSP Data to Output File
// -----------------------------
void BinaryFileReader::writePspData(const PSP_DATA &strctPspData, QFile &spOutputFile)
{
    QTextStream out(&spOutputFile);
    out.setRealNumberPrecision(6);
    out.setRealNumberNotation(QTextStream::FixedNotation);

    out << "PSP DWELL DATA\n";
    out << "Dwell Count: " << strctPspData.dwell_data.Dwell_count << "\n";
    out << "Time (T): " << strctPspData.dwell_data.dTime << "\n";
    out << "Alpha: " << strctPspData.dwell_data.alpha << "\n";
    out << "Beta: " << strctPspData.dwell_data.beta << "\n";
    out << "Boresight: " << strctPspData.dwell_data.boresight << "\n";
    out << "Pitch: " << strctPspData.dwell_data.pitch << "\n";
    out << "Roll: " << strctPspData.dwell_data.roll << "\n";

    for (int i = 0; i < strctPspData.no_of_rpt && i < 50; ++i) {
        const RPTS &r = strctPspData.SrchRpts[i];
        out << "[" << (i + 1) << "] "
            << "Range: " << r.m_frange
            << ", Strength: " << r.m_fStrength
            << ", Noise: " << r.m_fNoise
            << ", ΔAlpha: " << r.m_fDelAlpha
            << ", ΔBeta: " << r.m_fDelBeta
            << ", FilterNo: " << r.m_fFilterNo
            << "\n\n";
    }

    out << "Number of Reports: " << strctPspData.no_of_rpt << "\n\n";
    qDebug() << "PSP data written to" << spOutputFilePath;
}
