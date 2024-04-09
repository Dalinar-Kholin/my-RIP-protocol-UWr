#include <semaphore.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "prototypes.h"

int sendPacket(struct sockaddr_in *si_other, int s, dataToNet* myData){
    char buffer[sizeof(dataToNet)];
    memcpy(buffer, myData, sizeof(dataToNet));
    if (sendto(s, buffer, sizeof(dataToNet), 0, (struct sockaddr *) si_other, sizeof(*si_other)) == -1) {
        printf("data ip%.8x\n", myData->ip);
        return 1;
    }
    return 0;
}

void delFromVector(dataNetRepresent *net, directConnectedWebs* direct){//
    int index=0;
    if (direct!=NULL && direct->isNotOptimal){ // z poprzedniego if wiemy że jest połączony
        net->via=0;
        net->howLongUnreachable=0;
        net->lastReceive=0;
        net->distance= direct->distance;
        direct->isNotOptimal=0;
        return;
    }
    int ip = net->netIp;
    dataNetRepresent* newTab = calloc(howMuchNets-1, sizeof(dataNetRepresent));
    for (int i =0 ; i < howMuchNets; i++){
        if(tabOfNets[i].netIp!= ip){
            newTab[index++] = tabOfNets[i];
        }
    }
    howMuchNets--;
    free(tabOfNets);
    tabOfNets=newTab;
}

int isComeback(int addr){
    struct sockaddr_in si_other;
    int s, broadcastEnable = 1;
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        exit(1);
    }
    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(7);
    si_other.sin_addr.s_addr=addr;
    char buffer[9];
    memcpy(buffer, "benice", 7);
    if (sendto(s, buffer, sizeof(dataToNet), 0, (struct sockaddr *) &si_other, sizeof(si_other)) == -1) {
        return 1;
    }
    return 0;
}

void checkAliveness(directConnectedWebs* direct, int addr){
    if (direct->isDisconnected){
        dataNetRepresent *net = findViaIP(NET_ADDR(direct->ip,direct->mask));
        if (net->howLongUnreachable==NO_LONGER_INFORM || direct->isNotOptimal){
            if(!isComeback(addr)){
                net->lastReceive=0;
                net->howLongUnreachable=SET_LOST_CONNECTION;
                net->distance= findViaNet(net->netIp)->distance;
                direct->isDisconnected=0;
            }
        }
    }
} // rozprawić się, z tym że bezpośrednie połączenie nie musi być najklrótsze

void sendToDirect(struct sockaddr_in *si_other, int s){
    for (int i = 0; i < howMuchDirect; ++i) {
        int addr = htonl(BROADCAST_ADDR(directConn[i].ip,directConn[i].mask));
        si_other->sin_addr.s_addr=  addr;// tutaj dodac wysylanie na adres rozgloszeniowy a nie zwykly
        sem_wait(&sem);
        checkAliveness(&directConn[i], addr);
        if (directConn[i].isDisconnected){
            sem_post(&sem);
            continue;
        }
        sem_post(&sem);

        for (int j = 0; j < howMuchNets; ++j) {

            sem_wait(&sem);
            dataNetRepresent *net = &tabOfNets[j];
            if (net->howLongUnreachable==4){
                sem_post(&sem);
                continue;
            }
            dataToNet data={.ip=net->netIp,
                            .mask=net->netMask,
                            .distance = net->distance};

            if (sendPacket(si_other,s, &data)){
                directConn[i].isDisconnected=1;
                dataNetRepresent *corupttedNet = findViaIP(NET_ADDR(directConn[i].ip,directConn[i].mask));
                corupttedNet->lastReceive=MAX_CONNECTION_TRUST;
                corupttedNet->howLongUnreachable= corupttedNet->howLongUnreachable==SET_LOST_CONNECTION? 1 : corupttedNet->howLongUnreachable== NO_LONGER_INFORM ? NO_LONGER_INFORM : corupttedNet->howLongUnreachable+1;
                sem_post(&sem);
                break;
            }
            sem_post(&sem);
        }
    }
}

void updateTime(){
    for (int i = 0; i < howMuchNets; ++i) {
        sem_wait(&sem);
        dataNetRepresent *net = &tabOfNets[i];
        if (net->via!=DIRECT_CONN){
            net->lastReceive = net->lastReceive==MAX_CONNECTION_TRUST ? MAX_CONNECTION_TRUST : (net->lastReceive+1);
        }
        if (net->lastReceive==MAX_CONNECTION_TRUST){
            net->distance=UNREACHABLE;
            net->howLongUnreachable= net->howLongUnreachable== SET_LOST_CONNECTION ? 1 : net->howLongUnreachable== NO_LONGER_INFORM ? NO_LONGER_INFORM : net->howLongUnreachable+1;
            if(net->howLongUnreachable==NO_LONGER_INFORM ){
                directConnectedWebs *direct = findViaNet(net->netIp);
                if (direct==NULL || (direct->isNotOptimal && !direct->isDisconnected)){
                    delFromVector(net, direct);
                } else{
                    net->distance=UNREACHABLE;
                }
            }
        }
        sem_post(&sem);
    }
}

void *doWork(void *xd){
    struct sockaddr_in si_other;
    int s, broadcastEnable = 1;

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(54321);
    for(;;){
        sendToDirect(&si_other,s);
        printAll();
        sleep(4);
        updateTime();
    }
}