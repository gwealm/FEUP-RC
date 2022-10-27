// Application layer protocol implementation

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "application_layer.h"
#include "link_layer.h"
#include "state.h"
#include "constants.h"


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename) {  
                        
    int ll_role = -1;
    
    if (!strcmp(role, "tx")) {
        ll_role = 0;  
    } else if (!strcmp(role, "rx")) {
        ll_role = 1;
    } 

    if (ll_role < 0)
        exit(-1);

    LinkLayer ll;
    strncpy(ll.serialPort, serialPort, sizeof(ll.serialPort)-1);
    ll.serialPort[sizeof(ll.serialPort)-1] = '\0';
    ll.role = ll_role;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;

    if (llopen(ll) == -1)
        printf("llopen failed\n");

    switch (ll.role){
        int a;
        case TRANSMITTER:
            unsigned char buffer[4] = {0x05, 0x0d, 0x02, 0x7e};
            a = llwrite(buffer, 4);
            if (a < 0)
                printf("Sadge. Huge error, llwrite didn't return :(((\n");
            printf ("llwrite return %d :)))\n", a);
            break;
        case RECEIVER:
            unsigned char buf[10] = {0};
            a = llread(buf);
            if (a < 0)
                printf("Sadge. Huge error, llread didn't return :(((\n");

            printf ("llrread return %d :)))\n", a);
            
    }

    if (llclose(1) == -1)
        printf("llclose failed\n");
}
