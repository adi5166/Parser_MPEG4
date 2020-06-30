#include <iostream>
#include <cstdio>

#include "tsCommon.h"
#include "tsTransportStream.h"

const int TS_SIZE = xTS::TS_PacketLength;

int main(int argc, char *argv[], char *envp[]) {

    FILE *plik = fopen("example_new.ts", "rb");
    if (!plik) {
        printf("Wrong file name\n");
        return EXIT_FAILURE;
    }

    FILE *audio = fopen("audio.mp2", "wb");
    fclose(audio);

    xTS_PacketHeader TS_PacketHeader;
    xTS_AdaptationField TS_AdaptationField;
    xPES_PacketHeader PES_PacketHeader;
    xPES_Writer PES_Writer;
    //const uint8_t *TS_PacketBuffer;
    uint8_t TS_PacketBuffer[TS_SIZE];
    int32_t TS_PacketId = 0;
    //uint8_t buffor[TS_SIZE];

    uint8_t *wsk;

    while (!feof(plik)) {
        //if (TS_PacketId == 20) return 445;
        size_t odczytane = fread(TS_PacketBuffer, 1, TS_SIZE, plik);

        if (odczytane != TS_SIZE) { return -5; }
        //if (odczytane == 0) continue;
        //if (odczytane != TS_SIZE) return -5;
        //TS_PacketBuffer = buffor;

        TS_PacketHeader.Reset();
        wsk = TS_PacketHeader.Parse(TS_PacketBuffer);

        if (TS_PacketHeader.get_SB() == 'G' && TS_PacketHeader.get_PID() == 136) {
            //printf("\n%010d ", TS_PacketId);
            //TS_PacketHeader.Print();

            if (TS_PacketHeader.hasPayload()) {

                if (TS_PacketHeader.hasAdaptationField()) {
                    TS_AdaptationField.Reset();
                    wsk = TS_AdaptationField.Parse(wsk, TS_PacketHeader.get_AFC());
                    //TS_AdaptationField.Print();
                }
                if (TS_PacketHeader.get_S() == 1) {
                    PES_PacketHeader.Reset();
                    wsk = PES_PacketHeader.Parse(wsk, TS_AdaptationField.getNumBytes(),
                                                 TS_PacketHeader.hasAdaptationField());

                    PES_Writer.Reset(PES_PacketHeader.getPacketLength());
                    //PES_PacketHeader.Print();

                    PES_Writer.SetNumBytes(PES_PacketHeader.getExpectedByte());
                } else PES_Writer.SetNumBytes(184);

                xPES_Writer::eResult Result =PES_Writer.WriteData(wsk, &TS_PacketHeader ,PES_PacketHeader.getPacketLength());
                switch (Result) {

                }
            }


        }
        TS_PacketId++;
    }

    fclose(plik);
    printf("koniec");
    return 1;
}