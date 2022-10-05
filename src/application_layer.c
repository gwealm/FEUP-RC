// Application layer protocol implementation

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "application_layer.h"
#include "link_layer.h"
#include "state.h"

#define BUF_SIZE 256


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{   
    
    int ll_role = -1;
    
    if (!strcmp(role, "tx")){
        ll_role = 0;
    } 
    if (!strcmp(role, "rx")){
        ll_role = 1;
    } 
    if(ll_role<0){
        exit(-1);
    }

    LinkLayer ll;
    strncpy(ll.serialPort, serialPort, sizeof(ll.serialPort)-1);
    ll.serialPort[sizeof(ll.serialPort)-1] = '\0';
    ll.role = ll_role;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;

    if (llopen(ll) == -1){
        printf("llopen failed\n");
    }
    switch (ll_role){
        case 0:
            unsigned char buf[BUF_SIZE] = {0};
            buf[0] = FLAG;
            buf[1] = ADDR;
            buf[2] = 0x03;
            buf[4] = FLAG;
            llwrite(buf, BUF_SIZE);
            break;
        case 1:
            unsigned char packet[BUF_SIZE] = {0};
            llread(packet);
            break;
    }
    
    if (llclose(1) == -1){
        printf("llclose failed\n");
    }
}
