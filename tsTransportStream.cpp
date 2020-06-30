#include "tsTransportStream.h"

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_PacketHeader::Reset() {
    //printf("\nreset");
}

void xTS_PacketHeader::Print() const {
    printf("TS: ");
    printf("SB=%3d ", SB);
    printf("E=%1d ", E);
    printf("S=%1d ", S);
    printf("T=%1d ", T);
    printf("PID=%4d ", PID);
    printf("TSC=%1d ", TSC);
    printf("AFC=%1d ", AFC);
    printf("CC=%2d ", CC);
}

uint8_t* xTS_PacketHeader::Parse(const uint8_t *Input) {

    uint8_t pom;
    uint16_t pom2, pom3;
    uint32_t wynik;
    //1 bajt
    //wynik = wynik | (*Input << 24);
    SB = *Input;
    //2 bajt
    Input++;
    //wynik = wynik | (*Input << 16);
    pom = *Input & 0x80;
    E = pom >> 7;
    pom = *Input & 0x40;
    S = pom >> 6;
    pom = *Input & 0x20;
    T = pom >> 5;
    pom2 = *Input & 0x1F;
    pom3 = pom2 << 8;
    //3 bajt
    Input++;
    //wynik = wynik | (*Input << 8);
    pom3 = pom3 | *Input;
    PID = pom3;
    //4 bajt
    Input++;
    //wynik = wynik | *Input;
    pom = *Input & 0xC0;
    TSC = pom >> 6;
    pom = *Input & 0x30;
    AFC = pom >> 4;
    pom = *Input & 0x0F;
    CC = pom;


    return const_cast<uint8_t *>(Input);
}

//=============================================================================================================================================================================
// xTS_AdaptationField
//=============================================================================================================================================================================
void xTS_AdaptationField::Reset() {

}

uint8_t* xTS_AdaptationField::Parse(const uint8_t *Input, uint8_t AdaptationFieldControl) {


    if(AdaptationFieldControl == 0x01) {
        L=0;
        return const_cast<uint8_t *>(Input);
    }

    uint8_t pom;
    //Input += 4;
    Input++;
    L = *Input;

    Input++;
    pom = *Input & 0x80;
    DC = pom >> 7;
    pom = *Input & 0x40;
    RA = pom >> 6;
    pom = *Input & 0x20;
    P = pom >> 5;
    pom = *Input & 0x10;
    PR = pom >> 4;
    pom = *Input & 0x08;
    OR = pom >> 3;
    pom = *Input & 0x04;
    SP = pom >> 2;
    pom = *Input & 0x02;
    TP = pom >> 1;
    pom = *Input & 0x01;
    EX = pom;
    return const_cast<uint8_t *>(Input);
}

void xTS_AdaptationField::Print() const {
    printf("\n           AF: ");
    printf("L=%3d ", L);
    printf("DC=%1d ", DC);
    printf("RA=%1d ", RA);
    printf("P=%1d ", P);
    printf("PR=%1d ", PR);
    printf("OR=%1d ", OR);
    printf("SP=%1d ", SP);
    printf("TP=%1d ", TP);
    printf("EX=%1d ", EX);

}

//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================
void xPES_PacketHeader::Reset() {
    ToEnd = 6;
}

uint8_t* xPES_PacketHeader::Parse(const uint8_t *Input, uint8_t NumBytes, bool AF) {
    uint8_t Num;
    uint16_t pom2;
    uint32_t pom32=0;

    if(ToEnd==0) return const_cast<uint8_t *>(Input);

    if(AF) {
        //Input += 4 + NumBytes;
        Num = xTS::TS_PacketLength-xTS::TS_HeaderLength-NumBytes;

    } else{
        //Input += 4;
        Num = xTS::TS_PacketLength-xTS::TS_HeaderLength;
    }
    if(Num>xTS::PES_HeaderLength && ToEnd==6){
        Input++;
        pom2 = *Input << 8;
        Input++;
        pom32 = (pom2 | *Input) <<8;
        Input++;
        m_PacketStartCodePrefix = pom32 | *Input;

        Input++;
        m_StreamId = *Input;

        Input++;
        pom2 = *Input<<8;
        Input++;
        m_PacketLength = pom2 | *Input;

        ToEnd=0;

        if(m_StreamId != eStreamId_program_stream_map
        && m_StreamId != eStreamId_padding_stream
        && m_StreamId != eStreamId_private_stream_2
        && m_StreamId != eStreamId_ECM
        && m_StreamId != eStreamId_EMM
        && m_StreamId != eStreamId_program_stream_directory
        && m_StreamId != eStreamId_DSMCC_stream
        && m_StreamId != eStreamId_ITUT_H222_1_type_E
        &&Num>(xTS::PES_HeaderLength+3)){
            /*
            Input++;
            hed1=*Input;
            Input++;
            hed2=*Input;
            Input++;
            hed3=*Input;
            Input++;
            hed4=*Input;
            Input++;
            hed5=*Input;
            Input++;
            hed6=*Input;
            HeaderLength=hed3+6;
             */
            Input+=3;
            HeaderLength = *Input+xTS::PES_HeaderLength;


        } else HeaderLength = xTS::PES_HeaderLength;
        //przeskoczenie nagłówka
        Input+=HeaderLength-xTS::PES_HeaderLength;

        Input++;
        ExpectedByte = Num - HeaderLength;
    } else{
        m_PacketLength=111;
        m_PacketStartCodePrefix=111;
        m_StreamId=111;
    }

    return const_cast<uint8_t *>(Input);
}

void xPES_PacketHeader::Print() const {
    printf("\n           PES: ");
    printf("PSCP=%2d ", m_PacketStartCodePrefix);
    printf("SID=%3d ", m_StreamId);
    printf("L=%5d ", m_PacketLength);
    printf("Hed-Len=%3d ",HeaderLength);
    /*
    printf("\n           HED: ");
    printf("h1=%3o ",hed1);
    printf("h2=%3o ",hed2);
    printf("h3=%3o ",hed3);
    printf("h4=%3o ",hed4);
    printf("h5=%3o ",hed5);
    printf("h6=%3o ",hed6);
     */
}

//=============================================================================================================================================================================
// xPES_Writer
//=============================================================================================================================================================================

xPES_Writer::eResult xPES_Writer::WriteData(const uint8_t *Input, const xTS_PacketHeader* PacketHeader, uint16_t DataL) {
    uint16_t DL=DataL;

    uint16_t mPID = PacketHeader->get_PID();
    uint8_t  mCC  = PacketHeader->get_CC();

    if(mPID != 136) return eResult::UnexpectedPID;

    uint8_t SavedBytes =0;
    //otwarcie pliku
    FILE *zapis = fopen("audio.mp2", "ab");


    for(uint8_t i=0; i<ExpectedBytes; i++){

        uint8_t  buffor[1] = {*Input};
        fwrite(buffor, sizeof(uint8_t), 1, zapis);
        SavedBytes++;
        Input++;

        //zapisywanie bajtów
    }
    fclose(zapis);

    if(SavedBytes != ExpectedBytes) return eResult::StreamBytesLost;
    // porównanie CC

    ExpectedDataLenght=DataL-SavedBytes;

    if(PacketHeader->get_S()==1) return eResult::AssemblingStarted;
    return eResult::AssemblingFinished;
}

void xPES_Writer::SetNumBytes(uint8_t EB){
    ExpectedBytes = EB;
}

void xPES_Writer::Reset(uint16_t EDL) {
    ExpectedDataLenght = EDL;
}
//=============================================================================================================================================================================