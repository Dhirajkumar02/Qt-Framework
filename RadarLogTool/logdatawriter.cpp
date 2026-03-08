#include "LogDataWriter.h"
LogDataWriter::LogDataWriter()
{
}

void LogDataWriter::writePspData(const PSP_DATA &data)
{
    QTextStream out(&pspFile);

    out << data.dwell_data.alpha
        << " "
        << data.dwell_data.beta
        << "\n";
}
