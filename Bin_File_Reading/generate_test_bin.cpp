#include "messages.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QRandomGenerator>

int main() {
    QFile file("testdata.bin");
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "❌ Failed to open file for writing!";
        return -1;
    }

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);

    // Generate a few (3) message blocks
    for (int msg = 0; msg < 3; ++msg) {
        // -----------------------------
        // 1️⃣ Create header (DLOG_HEADER)
        // -----------------------------
        DLOG_HEADER header;
        header.m_ulMsgId = 0xEEEEEEEE;  // sync word
        header.m_ulTime  = 1000 + msg * 10;  // dummy timestamp

        // Write header to file
        out.writeRawData(reinterpret_cast<const char*>(&header), sizeof(DLOG_HEADER));

        // -----------------------------
        // 2️⃣ Fill DWELL_DATA
        // -----------------------------
        DWELL_DATA dwell = {};

        dwell.Word0_Msg_id = 0xAAA1;     // PSP message ID
        dwell.Dwell_count  = msg + 1;    // count increases per message
        dwell.dTime        = 1000 + msg * 50;  // arbitrary time

        dwell.alpha = 1.1f * (msg + 1);
        dwell.beta = 2.2f * (msg + 1);
        dwell.boresight = 3.3f * (msg + 1);
        dwell.pitch = 4.4f * (msg + 1);
        dwell.roll = 5.5f * (msg + 1);

        // Fill word unions with test values
        dwell.Word1.word = 0x1234;
        dwell.Word2.word = 0x5678;
        dwell.Word3.word = 0x9ABC;
        dwell.Word4.word = 0xDEF0;
        dwell.Word5.word = 0xAAAA;
        dwell.Word13.word = 0x1111;
        dwell.Word14.word = 0x2222;
        dwell.Word15.word = 0x3333;
        dwell.Word16.word = 0x4444;
        dwell.Word17.word = 0x5555;
        dwell.Word18.word = 0x6666;
        dwell.Word19.word = 0x7777;

        dwell.StartPredictedRange = 10 + msg * 5;
        dwell.StopPredictedRange  = 100 + msg * 10;

        // -----------------------------
        // 3️⃣ Fill PSP_DATA
        // -----------------------------
        PSP_DATA psp = {};
        psp.dwell_data = dwell;
        psp.no_of_rpt = 3;               // 3 reports
        psp.RMS_IQ = 0.95f + msg * 0.1f; // example RMS value

        for (int i = 0; i < 3; ++i) {
            psp.SrchRpts[i].m_frange = 10.0f * (i + 1) + msg * 5;
            psp.SrchRpts[i].m_fStrength = 20.0f * (i + 1);
            psp.SrchRpts[i].m_fNoise = 0.5f * (i + 1);
            psp.SrchRpts[i].m_fDelAlpha = 0.1f * (i + 1);
            psp.SrchRpts[i].m_fDelBeta = 0.2f * (i + 1);
            psp.SrchRpts[i].m_fFilterNo = i + 1;
        }

        // -----------------------------
        // 4️⃣ Write all to file
        // -----------------------------
        out.writeRawData(reinterpret_cast<const char*>(&psp.dwell_data), sizeof(DWELL_DATA));
        out << psp.no_of_rpt;
        out << psp.RMS_IQ;

        for (int i = 0; i < psp.no_of_rpt; ++i)
            out.writeRawData(reinterpret_cast<const char*>(&psp.SrchRpts[i]), sizeof(RPTS));

        // Optionally, add a little gap/junk between packets
        QByteArray junk = QByteArray::fromRawData("JUNKDATA", 8);
        if (msg < 2)  // not after the last one
            out.writeRawData(junk.data(), junk.size());
    }

    file.close();
    qDebug() << "✅ Test binary file 'testdata.bin' generated successfully!";
    return 0;
}
