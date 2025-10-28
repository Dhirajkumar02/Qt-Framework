#ifndef MESSAGES_H
#define MESSAGES_H

typedef unsigned short WORD;
typedef unsigned int UINT;
typedef unsigned int DWORD;

#pragma pack(push, 1)

struct DLOG_HEADER {
    DWORD m_ulMsgId;     // 0xEEEEEEEE
    DWORD m_ulTime;
};
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD beamType       : 4;
        WORD FncNmbr        : 4;
        WORD OpMode         : 1;
        WORD CfarThreshold  : 4;
        WORD CFarType       : 3;
    } bits;
} W1;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD wBeamNo : 8;
        WORD wTrkId  : 8;
    } bits;
} W1_1;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD ECCM_OnOff       : 1;
        WORD WECCMType        : 5;
        WORD NECCMFreq        : 2;
        WORD WJmdLocn_OnOff   : 1;
        WORD WECCM_Auto_Mnl   : 1;
        WORD WZoneNo          : 2;
        WORD wBroadBeam       : 2;
        WORD wReserved7_5     : 2;
    } bits;
} W1_2;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD Integration     : 4;
        WORD ZFiltDrop       : 1;
        WORD MapCntr         : 1;
        WORD MapThreshold    : 3;
        WORD IMTS            : 1;
        WORD AMTI            : 1;
        WORD MTI             : 2;
        WORD VideoSelection  : 1;
        WORD SLB_OnOff       : 1;
        WORD Splog           : 1;
    } bits;
} W2;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD WDPC             : 1;
        WORD BinaryThreshold  : 7;
        WORD RangeWindowSize  : 3;
        WORD Reserved_1       : 1;
        WORD wHybridMode      : 1;
        WORD wDPCWindowOnOff  : 1;
        WORD SysMode          : 2;
    } bits;
} W3;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD BTTE_attenuation : 6;
        WORD DopplerCode      : 4;
        WORD TgtSpacing       : 2;
        WORD TgtInOut         : 1;
        WORD BITE_RF_Field    : 1;
        WORD BITE_OnOff       : 1;
        WORD STAMO_BITE       : 1;
    } bits;
} W11;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD Tgt_Speed   : 10;
        WORD NoOfTgts    : 4;
        WORD System_CW   : 1;
        WORD Txt_TxCal   : 1;
    } bits;
} W12;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD TgtStartRange : 8;
        WORD Reserved_3    : 8;
    } bits;
} W13;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD WvFrmBw       : 1;
        WORD WvFrmPw       : 4;
        WORD WrFrmMod      : 3;
        WORD Stage1        : 1;
        WORD Blank         : 1;
        WORD MSTC_law4     : 1;
        WORD Reserved_4    : 1;
        WORD ExRxReset     : 1;
        WORD ReservedBits  : 3;
    } bits;
} W14;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD WPRFCode       : 8;
        WORD GrainCtrlAttn  : 5;
        WORD WstaggerCode   : 3;
    } bits;
} W15;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD NoOfPulses : 8;
        WORD FreqCode   : 8;
    } bits;
} W17;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD R12        : 12;
        WORD OprnlMode  : 4;
    } bits;
} W18;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD NumOfPulses : 8;
        WORD R4          : 4;
        WORD OprnlMod    : 4;
    } bits;
} W18_ppc;
#pragma pack(pop)

#pragma pack(push, 1)
typedef union {
    WORD word;
    struct {
        WORD R4           : 4;
        WORD AttnTaprID   : 4;
        WORD Freq_ID      : 8;
    } bits;
} W19;
#pragma pack(pop)

#pragma pack(push, 1)
struct DWELL_DATA {
    WORD Word0_Msg_id; // 0xAAA1
    W1 Word1;
    W1_1 Word2;
    W1_2 Word3;
    W2 Word4;
    W3 Word5;

    WORD StartPredictedRange;
    WORD StopPredictedRange;

    float alpha;
    float beta;
    float boresight;
    float pitch;
    float roll;

    DWORD dTime;

    W11 Word13;
    W12 Word14;
    W13 Word15;
    W14 Word16;
    W15 Word17;
    W17 Word18;

    WORD Dwell_count;
    W19 Word19;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct RPTS {
    float m_frange;
    float m_fStrength;
    float m_fNoise;
    float m_fDelAlpha;
    float m_fDelBeta;
    float m_fFilterNo;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PSP_DATA {
    struct DWELL_DATA dwell_data;
    WORD no_of_rpt;
    float RMS_IQ;
    struct RPTS SrchRpts[50];
    // WORD eod;
};
#pragma pack(pop)

#endif // MESSAGES_H
