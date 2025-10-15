#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>

class BinaryFileReader : public QWidget {
    Q_OBJECT

public:
    BinaryFileReader(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("Binary File Reader");
        resize(600, 400);

        // --- UI Elements ---
        openButton = new QPushButton("Open File");
        processButton = new QPushButton("Process File");
        fileNameEdit = new QLineEdit;
        fileNameEdit->setPlaceholderText("Select a .bin file");

        table = new QTableWidget;
        table->setColumnCount(2);
        table->setHorizontalHeaderLabels({"Time", "MsgId"});
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        // --- Layout ---
        QHBoxLayout *topLayout = new QHBoxLayout;
        topLayout->addWidget(openButton);
        topLayout->addWidget(fileNameEdit);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addLayout(topLayout);
        mainLayout->addWidget(processButton);
        mainLayout->addWidget(table);

        // --- Connections ---
        connect(openButton, &QPushButton::clicked, this, &BinaryFileReader::openFile);
        connect(processButton, &QPushButton::clicked, this, &BinaryFileReader::processFile);
    }

private slots:
    void openFile() {
        QString filePath = QFileDialog::getOpenFileName(this, "Open Binary File", "", "Binary Files (*.bin);;All Files (*)");
        if (!filePath.isEmpty()) {
            fileNameEdit->setText(filePath);
            selectedFilePath = filePath;
            openButton->setStyleSheet("background-color: lightgreen;");
        }
    }

    void processFile() {
        if (selectedFilePath.isEmpty()) {
            QMessageBox::warning(this, "Warning", "Please open a binary file first!");
            return;
        }

        QFile file(selectedFilePath);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Error", "Failed to open file!");
            return;
        }

        table->setRowCount(0);
        QDataStream in(&file);
        in.setByteOrder(QDataStream::LittleEndian); // adjust if needed

        while (!in.atEnd()) {
            quint32 time;
            quint32 msgId;

            in >> time;
            in >> msgId;

            if (in.status() != QDataStream::Ok) break;
            if ((qint32)time < 0) continue;  // skip negative time values

            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(time)));
            table->setItem(row, 1, new QTableWidgetItem(QString("0x%1").arg(msgId, 8, 16, QLatin1Char('0')).toUpper()));
        }

        file.close();
    }

private:
    QPushButton *openButton;
    QPushButton *processButton;
    QLineEdit *fileNameEdit;
    QTableWidget *table;
    QString selectedFilePath;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    BinaryFileReader window;
    window.show();
    return app.exec();
}
