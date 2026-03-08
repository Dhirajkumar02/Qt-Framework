#include "logreader.h"

LogReader::LogReader()
{
}

void LogReader::setFile(const QString &path)
{
    binFile.setFileName(path);
}
void LogReader::processFile()
{
    if(!binFile.open(QIODevice::ReadOnly))
        return;

    QDataStream in(&binFile);
    in.setByteOrder(QDataStream::LittleEndian);

    while(!binFile.atEnd())
    {
        quint32 msgID32;
        quint16 msgID16;

        in >> msgID32;

        if(msgID32 == 0xEEEEEEEE)
        {
            in.device()->seek(in.device()->pos() - sizeof(msgID32));

            in.readRawData(reinterpret_cast<char*>(&strctLogHdr),
                           sizeof(DLOG_HEADER));

            in >> msgID16;

            switch(msgID16)
            {
            case 0xAAA1:

                in.readRawData(reinterpret_cast<char*>(&strctPspData),
                               sizeof(PSP_DATA));

                writer.writePspData(strctPspData);


                break;
            }
        }
    }

    binFile.close();
}
