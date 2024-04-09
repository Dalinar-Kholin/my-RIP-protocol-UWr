#include "prototypes.h"
#include "stdio.h"
#include "semaphore.h"





int8_t howMuchNets;
int8_t howMuchDirect;
dataNetRepresent *tabOfNets;
directConnectedWebs *directConn;
sem_t sem;

void printNetData(dataNetRepresent* data){
    printf("hunan ip: %s\n"
           "net Ip : %.8x\n"
           "via: %.8x\n"
           "mask: %d\n"
           "distance: %d\n"
           "last receive %d\n"
           "how long unreachable %d\n",
           data->niceLook,
           data->netIp,
           data->via,
           data->netMask,
           data->distance,
           data->lastReceive,
           data->howLongUnreachable);
}



int isSelfPacket(int ip){
    for(int i =0; i< howMuchDirect;i++){
        if (directConn[i].ip==ip){
            return 1;
        }
    }
    return 0;
}

int isDirect(int ip){
    for(int i =0; i< howMuchDirect;i++){
        if (NET_ADDR(directConn[i].ip,directConn[i].mask) == NET_ADDR(ip,directConn[i].mask)){
            return 1;
        }
    }
    return 0;
}

dataNetRepresent *findViaIP(int ip){
    for (int i = 0; i < howMuchNets; ++i) {
        if (tabOfNets[i].netIp==ip){
            return &tabOfNets[i];
        }
    }
    return NULL;
}


directConnectedWebs *findViaNet(int ip){
    for (int i = 0; i < howMuchDirect; ++i) {
        if (NET_ADDR(directConn[i].ip, directConn[i].mask)==NET_ADDR(ip, directConn[i].mask)){
            return &directConn[i];
        }
    }
    return NULL;
}


void printAll(){

    printf("---------------\n");
    for (int i = 0; i < howMuchNets; ++i) {
        printNetData(&tabOfNets[i]);
        putchar('\n');
    }
    printf("================\n");
}
