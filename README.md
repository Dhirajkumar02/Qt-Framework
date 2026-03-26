
QFile* trackFiles[20];   // max 20 tracks
int trackIds[20];        // store actual track IDs
int noOfTracks = 0;      // how many tracks user entered

void BinaryFileReader::createTrackFiles()
{
    // reset
    noOfTracks = 0;

    QString text = trackLineEdit->text();
    QStringList ids = text.split(",", Qt::SkipEmptyParts);

    QFileInfo fileInfo(filePath);
    QString basePath = fileInfo.absolutePath();

    for(QString idStr : ids)
    {
        if(noOfTracks >= 20) break;   // limit

        int id = idStr.trimmed().toInt();

        QString filename = basePath + "/Tid_" + QString::number(id) + ".txt";

        QFile *file = new QFile(filename);

        if(file->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            trackFiles[noOfTracks] = file;
            trackIds[noOfTracks] = id;
            noOfTracks++;
        }
    }
}

int trackId = strctFilterData.trackId;

for(int i = 0; i < noOfTracks; i++)
{
    if(trackIds[i] == trackId)
    {
        QFile *file = trackFiles[i];
        QTextStream out(file);

        out << trackId << " "
            << strctFilterData.x << " "
            << strctFilterData.y << " "
            << strctFilterData.z << " "
            << strctFilterData.dTime
            << "\n";

        // send to display
        oDisplaySender.sendToDisplay(strctDisplayTrackData);

        break;  // important (match mil gaya)
    }
}

for(int i = 0; i < noOfTracks; i++)
{
    trackFiles[i]->close();
    delete trackFiles[i];
}

noOfTracks = 0;


QMap<int, QFile*> trackFiles;
void BinaryFileReader::createTrackFiles()
{
    // cleanup old
    for(auto file : trackFiles)
    {
        file->close();
        delete file;
    }
    trackFiles.clear();

    QString text = trackLineEdit->text();
    QStringList ids = text.split(",", Qt::SkipEmptyParts);

    QFileInfo fileInfo(filePath);
    QString basePath = fileInfo.absolutePath();

    int count = 0;

    for(QString idStr : ids)
    {
        if(count >= 20) break;   // same limit like array

        int id = idStr.trimmed().toInt();

        QString filename = basePath + "/Tid_" + QString::number(id) + ".txt";

        QFile *file = new QFile(filename);

        if(file->open(QIODevice::WriteOnly | QIODevice::Text))
        {
            trackFiles[id] = file;
            count++;
        }
    }
}

int trackId = strctFilterData.trackId;

if(trackFiles.contains(trackId))
{
    QTextStream out(trackFiles[trackId]);

    out << trackId << " "
        << strctFilterData.x << " "
        << strctFilterData.y << " "
        << strctFilterData.z << " "
        << strctFilterData.dTime
        << "\n";

    oDisplaySender.sendToDisplay(strctDisplayTrackData);
}
for(auto file : trackFiles)
{
    file->close();
    delete file;
}
trackFiles.clear();
