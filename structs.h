#include "stdint.h"

typedef struct dataNetRepresent{
    char niceLook[16];
    int32_t netIp; // ip sieci
    int32_t via; // przez jaki dokladny adres idzie ta siec
    int32_t distance; // odleglosc od sieci
    short howLongUnreachable; //
    short lastReceive; //
    int8_t netMask; //maska tej sieci
} dataNetRepresent;

typedef struct directConnectedWebs{
    int32_t ip; //nasze ip w tej sieci
    int32_t distance;
    short mask;
    int8_t isDisconnected;
    int8_t isNotOptimal;
} directConnectedWebs;

typedef struct dataToNet{
    int32_t ip;
    int32_t distance;
    int8_t mask;
} dataToNet;