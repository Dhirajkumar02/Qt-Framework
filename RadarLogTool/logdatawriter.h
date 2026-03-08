#ifndef LOGDATAWRITER_H
#define LOGDATAWRITER_H

#include <QFile>
#include <QTextStream>
#include "messages.h"

class LogDataWriter
{
public:

    LogDataWriter();

    void writePspData(const PSP_DATA &data);

private:

    QFile pspFile;

};

#endif
