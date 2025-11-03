#include "mainwindow.h"
#include "messages.h"
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
//#include <QTableWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include <QDebug>
#include <QProgressBar>
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
    showCheckBox ->setChecked(false); //default unchecked

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

    // Progress Bar
    progressBar = new QProgressBar;
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setFixedWidth(200);

    // Table Widget
    /*table = new QTableWidget;
    table->setColumnCount(4);
    table->setHorizontalHeaderLabels({"dTime", "Dwell_count", "Boresight", "No_of_rpts"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);*/

    // Layouts
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(openFileButton);
    topLayout->addWidget(fileNameEdit);
    topLayout->addWidget(skipLineEdit);
    topLayout->addWidget(skipButton);

    QHBoxLayout *processLayout = new QHBoxLayout;
    processLayout->addWidget(processButton);
    processLayout->addWidget(progressBar);
    processLayout->addSpacing(10);
    processLayout->addWidget(showCheckBox);
    processLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(processLayout);
    //mainLayout->addWidget(table);
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
}

// -----------------------------
// Browse and select a binary file
// -----------------------------
void BinaryFileReader::openFile()
{
    binfilePath = QFileDialog::getOpenFileName(this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*)");
    if (binfilePath.isEmpty())
        return;

    //selectedFilePath = filePath;
    fileNameEdit->setText(binfilePath);
    processButton->setEnabled(true);

    // Visual feedback
    openFileButton->setStyleSheet("background-color: lightgreen;");
}

// -----------------------------
// Set skip percentage
// -----------------------------
void BinaryFileReader::skipPercentage()
{
    bool ok;
    int percent = skipLineEdit->text().toInt(&ok);

    if (ok && percent >= 0 && percent < 100) {
        skipPercent = percent;
        progressBar->setValue(skipPercent);  //Show skip progress immediately
        qDebug() << "Skip percentage set to:" << skipPercent;
    } else {
        skipPercent = 0;
        progressBar->setValue(0);  // reset progress
        qDebug() << "Invalid skip percentage, defaulting to 0";
    }

    qApp->processEvents();  // Force UI refresh immediately
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

    // Construct output file path in the same folder as input file
    QFileInfo fileInfo(binfilePath);
    QString folderPath = fileInfo.absolutePath();
    spOutputFilePath = folderPath + "/PspOutput.txt";

    spOutputFile.setFileName(spOutputFilePath);
    if (!spOutputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open output file for writing");
        return;
    }

    //tablea->setRowCount(0); // Clear previous data

    quint64 totalSize = binfile.size();
    quint64 skipBytes = (skipPercent * totalSize) / 100;

    // Skip initial bytes
    if (!binfile.seek(skipBytes)) {
        QMessageBox::critical(this, "Error", "Failed to skip bytes in file!");
        binfile.close();
        return;
    }

    // Start progress from skip percent
    progressBar->setValue(skipPercent);

    QDataStream in(&binfile);
    in.setByteOrder(QDataStream::LittleEndian);

    quint64 bytesReadTotal = skipBytes;
    qint64 posAfter;
    quint32 msgID32;
    quint16 msgID16;
    //int row;

    while (!in.atEnd()) {
        in >> msgID32;

        if (msgID32 == 0xEEEEEEEE) {
            posAfter = in.device()->pos();
            // Move back by the header size
            in.device()->seek(posAfter - sizeof(msgID32));

            // Read DLOG header
            in.readRawData(reinterpret_cast<char*>(&strctLogHdr), sizeof(DLOG_HEADER));
            // Read message type
            in >> msgID16;

            switch (msgID16) {
            case 0xAAA1:
                posAfter = in.device()->pos();
                in.device()->seek(posAfter - sizeof(msgID16));

                // Read DWELL data
                in.readRawData(reinterpret_cast<char*>(&strctDwlData), sizeof(DWELL_DATA));
                in >> strctPspData.no_of_rpt;
                in >> strctPspData.RMS_IQ;

                qDebug() << "Time:" << strctPspData.dwell_data.dTime
                         << "No. of rpt:" << strctPspData.no_of_rpt;

                for (int i = 0; i < strctPspData.no_of_rpt; ++i){
                    // Read RPTS entries
                    in.readRawData(reinterpret_cast<char*>(&strctRpts), sizeof(RPTS));
                }
                // Display in table
                /*if(1){
                row = table->rowCount();
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(QString::number(strctPspData.dwell_data.dTime)));
                table->setItem(row, 1, new QTableWidgetItem(QString::number(strctPspData.dwell_data.Dwell_count)));
                table->setItem(row, 2, new QTableWidgetItem(QString::number(strctPspData.dwell_data.boresight)));
                table->setItem(row, 3, new QTableWidgetItem(QString::number(strctPspData.no_of_rpt)));
                }*/
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
        progressBar->setValue(progressPercent);

        qApp->processEvents(); // Keep UI responsive
    }

    progressBar->setValue(100);
    binfile.close();
    spOutputFile.close();

    qDebug() << "Skipped first" << skipPercent << "% (" << skipBytes << " bytes)";
}

void BinaryFileReader:: writePspData(const PSP_DATA &strctPspData, QFile &spOutputFile){

    QTextStream out(&spOutputFile);
    out.setRealNumberPrecision(6);
    out.setRealNumberNotation(QTextStream::FixedNotation);

    // -------------------------
    // Write Dwell Data
    // -------------------------
    out << "PSP DWELL DATA\n";
    out << "Dwell Count: " << strctPspData.dwell_data.Dwell_count << "\n";
    out << "Time (T): " << strctPspData.dwell_data.dTime << "\n";
    out << "Alpha: " << strctPspData.dwell_data.alpha << "\n";
    out << "Beta: " << strctPspData.dwell_data.beta << "\n";
    out << "Boresight: " << strctPspData.dwell_data.boresight << "\n";
    out << "Pitch: " << strctPspData.dwell_data.pitch << "\n";
    out << "Roll: " << strctPspData.dwell_data.roll << "\n";


    // -------------------------
    // Write Report Data
    // -------------------------
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
