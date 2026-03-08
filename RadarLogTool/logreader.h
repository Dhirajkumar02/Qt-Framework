#ifndef LOGREADER_H
#define LOGREADER_H

#include <QFile>
#include "messages.h"
#include "logdatawriter.h"
#include "filters.h"

class LogReader
{
public:

    LogReader();

    void setFile(const QString &path);
    void processFile();

private:

    QFile binFile;

    LogDataWriter writer;
    Filters filter;

    DLOG_HEADER strctLogHdr;
    PSP_DATA strctPspData;
    DWELL_DATA strctDwlData;

};

#endif // LOGREADER_H
