#pragma once
#include "tsCommon.h"
#include <string>

/*
MPEG-TS packet:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |                             Header                            | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   4 |                  Adaptation field + Payload                   | `
`     |                                                               | `
` 184 |                                                               | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `


MPEG-TS packet header:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |       SB      |E|S|T|           PID           |TSC|AFC|   CC  | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `

Sync byte                    (SB ) :  8 bits
Transport error indicator    (E  ) :  1 bit
Payload unit start indicator (S  ) :  1 bit
Transport priority           (T  ) :  1 bit
Packet Identifier            (PID) : 13 bits
Transport scrambling control (TSC) :  2 bits
Adaptation field control     (AFC) :  2 bits
Continuity counter           (CC ) :  4 bits
*/


//=============================================================================================================================================================================
class xTS
{
public:
    static constexpr uint32_t TS_PacketLength = 188;
    static constexpr uint32_t TS_HeaderLength = 4;

    static constexpr uint32_t PES_HeaderLength = 6;

    static constexpr uint32_t BaseClockFrequency_Hz         =    90000; //Hz
    static constexpr uint32_t ExtendedClockFrequency_Hz     = 27000000; //Hz
    static constexpr uint32_t BaseClockFrequency_kHz        =       90; //kHz
    static constexpr uint32_t ExtendedClockFrequency_kHz    =    27000; //kHz
    static constexpr uint32_t BaseToExtendedClockMultiplier =      300;
};

//=============================================================================================================================================================================

class xTS_PacketHeader
{
public:
    enum class ePID : uint16_t
    {
        PAT  = 0x0000,
        CAT  = 0x0001,
        TSDT = 0x0002,
        IPMT = 0x0003,
        NIT  = 0x0010, //DVB specific PID
        SDT  = 0x0011, //DVB specific PID
        NuLL = 0x1FFF,
    };

protected:
    uint8_t SB=1;
    uint8_t E;
    uint8_t S;
    uint8_t T;
    uint16_t PID;
    uint8_t TSC;
    uint8_t AFC;
    uint8_t CC;

public:
    void        Reset();
    uint8_t*   Parse(const uint8_t* Input);
    void        Print() const;

public:
    uint8_t get_SB ()   const { return SB;}
    uint8_t get_E ()    const { return E;}
    uint8_t get_S ()    const { return S;}
    uint8_t get_T ()    const { return T;}
    uint16_t get_PID () const { return PID;}
    uint8_t get_TSC ()  const { return TSC;}
    uint8_t get_AFC ()  const { return AFC;}
    uint8_t get_CC ()   const { return CC;}
    //TODO - direct acces to header values

public:
    bool     hasAdaptationField() const { return 0x02 == (AFC & 0x02);}
    bool     hasPayload        () const { return 0x01 == (AFC & 0x01);}
};

//=============================================================================================================================================================================
class xTS_AdaptationField {
protected:
    uint8_t L;
    uint8_t DC;
    uint8_t RA;
    uint8_t P;
    uint8_t PR;
    uint8_t OR;
    uint8_t SP;
    uint8_t TP;
    uint8_t EX;

public:
    void        Reset();
    uint8_t*   Parse(const uint8_t *Input, uint8_t AdaptationFieldControl);
    void        Print() const;

public:
    //derrived values
    uint8_t get_L ()    const {return L;}
    uint8_t get_DC ()   const {return DC;}
    uint8_t get_RA ()   const {return RA;}
    uint8_t get_P ()    const {return P;}
    uint8_t get_PR ()   const {return PR;}
    uint8_t get_OR ()   const {return OR;}
    uint8_t get_SP ()   const {return SP;}
    uint8_t get_TP ()   const {return TP;}
    uint8_t get_EX ()   const {return EX;}
    uint32_t getNumBytes() const {return L+1;}
};

//=============================================================================================================================================================================
class xPES_PacketHeader
{
public:
    enum eStreamId : uint8_t
    {
        eStreamId_program_stream_map       = 0xBC,
        eStreamId_padding_stream           = 0xBE,
        eStreamId_private_stream_2         = 0xBF,
        eStreamId_ECM                      = 0xF0,
        eStreamId_EMM                      = 0xF1,
        eStreamId_program_stream_directory = 0xFF,
        eStreamId_DSMCC_stream             = 0xF2,
        eStreamId_ITUT_H222_1_type_E       = 0xF8,
    };

protected:
    //PES packet header
    uint32_t m_PacketStartCodePrefix;
    uint8_t  m_StreamId;
    uint16_t m_PacketLength;
    //
    uint8_t HeaderLength;
    uint8_t ToEnd=6;
    uint8_t ExpectedByte;
    uint8_t hed1,hed2,hed3,hed4,hed5,hed6;


public:
    void     Reset();
    uint8_t* Parse(const uint8_t* Input, uint8_t NumBytes, bool AF);
    void     Print() const;

public:
    //PES packet header
    uint32_t getPacketStartCodePrefix() const { return m_PacketStartCodePrefix; }
    uint8_t  getStreamId             () const { return m_StreamId; }
    uint16_t getPacketLength         () const { return m_PacketLength; }
    uint8_t  getToEnd ()                const { return ToEnd; }
    uint8_t  getHeaderLength()          const { return HeaderLength; }
    uint8_t  getExpectedByte()          const { return ExpectedByte; }
};

//=============================================================================================================================================================================
class xPES_Writer{
public:
    enum class eResult : int32_t
    {
        UnexpectedPID     = 1,
        StreamBytesLost   ,
        StreamPackedLost  ,
        AssemblingStarted ,
        AssemblingContinue,
        AssemblingFinished,
    };

protected:
    //dane
    uint8_t* Buffor;
    uint32_t BufforSize;

    uint16_t DataLength;
    uint16_t ExpectedDataLenght;
    uint8_t  ExpectedBytes;
public:
    //funkcje
    eResult WriteData(const uint8_t *Input, const xTS_PacketHeader* PacketHeader, uint16_t DataL);
    void Reset(uint16_t EDL);
    void SetNumBytes(uint8_t EB);

};


//=============================================================================================================================================================================
class xPES_Assembler
{
public:
    enum class eResult : int32_t
    {
        UnexpectedPID     = 1,
        StreamPackedLost  ,
        AssemblingStarted ,
        AssemblingContinue,
        AssemblingFinished,
    };

protected:
    //setup
    int32_t  m_PID;
    //buffer
    uint8_t* m_Buffer;
    uint32_t m_BufferSize;
    uint32_t m_DataOffset;
    //operation
    int8_t            m_LastContinuityCounter;
    bool              m_Started;
    xPES_PacketHeader m_PESH;

public:
    xPES_Assembler ();
    ~xPES_Assembler();

    void    Init        (int32_t PID);
    eResult AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField);

    void     PrintPESH        () const { m_PESH.Print(); }
    uint8_t* getPacket        ()       { return m_Buffer; }
    int32_t  getNumPacketBytes() const { return m_DataOffset; }

protected:
    void xBufferReset ();
    void xBufferAppend(const uint8_t* Data, int32_t Size);
};