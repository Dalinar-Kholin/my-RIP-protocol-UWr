#include "semaphore.h"
#include "structs.h"

#define IS_CONNECTED 0
#define MAX_CONNECTION_TRUST (5)
#define MAX_DISTANCE (16)
#define SET_LOST_CONNECTION 0
#define NO_LONGER_INFORM 4
#define UNREACHABLE (-1)
#define DIRECT_CONN (0)
#define NET_ADDR(addr, mask) (addr&(-1<<(32-mask)))

#define PREFIX_TO_MASK(prefix) ((0xFFFFFFFF << (32 - (prefix))) & 0xFFFFFFFF)
#define BROADCAST_ADDR(ip, prefix) ((ip) | ~(PREFIX_TO_MASK(prefix)))


extern void *doWork(void * xd);
extern void printAll();
extern int8_t howMuchNets;
extern int8_t howMuchDirect;
extern dataNetRepresent *tabOfNets;
extern directConnectedWebs *directConn;
extern sem_t sem;
directConnectedWebs *findViaNet(int ip);
dataNetRepresent *findViaIP(int ip);
int isDirect(int ip);
int isSelfPacket(int ip);
