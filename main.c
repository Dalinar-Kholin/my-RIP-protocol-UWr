#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include "prototypes.h"


// Kacper Osadowski all right reserved for UWr <3


char takeMaskAndIzolateIp(char *ip){
    char* idx = strchr(ip,'/');
    ip[idx-ip]='\0';
    return atoi(&ip[idx-ip+1]);;
}

void readData(){
    char ip[20];
    char placeholder[10];
    int netCount=0;
    scanf("%d", &netCount);
    howMuchNets= (char)netCount;
    howMuchDirect = (char)netCount;
    tabOfNets = calloc(howMuchNets, sizeof(dataNetRepresent));
    directConn = calloc(howMuchNets, sizeof(directConnectedWebs));
    for (int i = 0; i < netCount; ++i) {
        dataNetRepresent *netP = &tabOfNets[i];
        scanf("%s %s %d", ip, placeholder, (int32_t *)&netP->distance);
        netP->netMask = takeMaskAndIzolateIp(ip);
        directConnectedWebs data = { .mask = netP->netMask, .isDisconnected = 0 };
        if (inet_pton(AF_INET,ip, &netP->netIp)){
            data.ip=(int32_t) htonl(netP->netIp);
            netP->netIp= NET_ADDR(data.ip, netP->netMask);
        }else{
            exit(1);
        }
        int par =htonl(netP->netIp);
        if (inet_ntop(AF_INET, &par, netP->niceLook, INET_ADDRSTRLEN) != NULL) {
        } else {
            fprintf(stderr, "Błąd konwersji na postać tekstową\n");
            exit(EXIT_FAILURE);
        }
        putchar('\n');
        data.distance=netP->distance;
        directConn[i]= data;
    }
}
int makeSocket(){
    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(54321);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}


void addNewNet(dataToNet *data, int viaCounted, char * ipStr){
    tabOfNets = realloc(tabOfNets, sizeof(dataNetRepresent) * (howMuchNets+1));
    dataNetRepresent *net = &tabOfNets[howMuchNets];// to jest poprawne
    strcpy(net->niceLook, ipStr);
    net->netMask= data->mask;
    net->netIp= NET_ADDR(data->ip, data->mask);
    net->distance= data->distance;
    net->via = viaCounted;
    net->lastReceive=0;
    howMuchNets++;
}

void updateNet(dataToNet* data, dataNetRepresent *net, int viaCounted, char *ipStr){

    if (isDirect(net->netIp)){
        directConnectedWebs *direct =findViaNet(net->netIp);
        if ((data->distance!=UNREACHABLE && direct->distance > data->distance)  || direct->isDisconnected){
            direct->isNotOptimal=1;
            net->distance= data->distance;
            net->via=viaCounted;
            net->lastReceive=0;
            net->howLongUnreachable=0;
        }
        return;
    }
    if ((data->distance==UNREACHABLE && net->distance==UNREACHABLE)){
        return;
    }
    if (viaCounted==net->via || net->via==DIRECT_CONN){
        net->lastReceive=0;
    }
    if ((data->distance==UNREACHABLE || data->distance> MAX_DISTANCE) && viaCounted==net->via){//
        net->distance=UNREACHABLE;
        net->howLongUnreachable=SET_LOST_CONNECTION;
        return;
    }
    if ((data->distance!=net->distance && net->via==viaCounted)){
        net->distance= data->distance;
        net->howLongUnreachable=SET_LOST_CONNECTION;
    }
    if ((unsigned )data->distance < net->distance){
        strcpy(net->niceLook, ipStr);
        net->distance= data->distance;
        net->via = viaCounted;
        net->lastReceive=0;
        net->howLongUnreachable=SET_LOST_CONNECTION;
    }
}

void updateData(dataToNet *data, int32_t via){
    dataNetRepresent *net;
    int viaCounted = ntohl(via);
    if (isSelfPacket(viaCounted)){
        return;
    }
    sem_wait(&sem);
    struct directConnectedWebs *sender = findViaNet(viaCounted);
    data->distance+= data->distance==UNREACHABLE ? 0 :  sender->distance;
    struct in_addr humanIp;
    int netAdres = NET_ADDR(data->ip,data->mask);
    sem_post(&sem);
    humanIp.s_addr= htonl(netAdres);
    char *ipStr = inet_ntoa(humanIp);
    for (int i = 0; i < howMuchNets; ++i) {
        if(tabOfNets[i].netIp == netAdres && tabOfNets[i].netMask== data->mask){
            sem_wait(&sem);
            net = &tabOfNets[i];
            updateNet(data,net,viaCounted,ipStr);
            sem_post(&sem);
            return;
        }
    }
    if (data->distance==UNREACHABLE){
        return;
    }else{
        sem_wait(&sem);
        addNewNet(data,viaCounted,ipStr);
        sem_post(&sem);
    }
}



int main(){
    sem_init(&sem, 0, 1);
    readData();
    int sockfd = makeSocket();
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    int len = sizeof(cliaddr);
    pthread_t thread_id;
    int result;
    int port;
    result = pthread_create(&thread_id, NULL, doWork, &port);
    if (result){
        perror("cant create new thread\n");
        exit(1);
    }
    char buffer[9];
    while (1) {
        if (recvfrom(sockfd, (char *)buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *)&cliaddr, (socklen_t*)&len)==-1){
            perror("cant take packet\n");
            continue;
        }
        dataToNet *data = (dataToNet*)buffer;
        int adr = cliaddr.sin_addr.s_addr;
        updateData(data, adr);
        if(strcmp(buffer,"123\n")==0){
            break;
        }
    }

    sem_destroy(&sem);
    close(sockfd);
}
