// Application layer protocol implementation

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

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
        int res;
        case TRANSMITTER:
            int file_fd;
            struct stat file_stat;

            // read file info using stat
            if (stat(filename, &file_stat)<0){
                perror("Error getting file information.");
                return -1;
            }

            // open file
            if ((file_fd = open(filename, O_RDONLY)) < 0){
                perror("Error opening file.");
                return -1;
            }

            // send start packet (abstract to dedicated function or similar)
            int l1 = sizeof(file_stat.st_size);
            int l2 = strlen(filename);
            int start_size = 5 + l1 + l2;

            unsigned char *packet = (unsigned char *) malloc(start_size);
            memset(packet, 0, start_size);
            packet[0] = 2;
            packet[1] = 0; // file size
            packet[2] = l1;
            memcpy(&packet[3], &(file_stat.st_size), l1);
            packet[3+l1] = 1; // file name
            packet[4+l1] = l2;
            memcpy(&packet[5+l1], filename, l2);
            res = llwrite(packet, start_size);
            if (res < 0)
                printf("Sadge. Huge error, llwrite didn't return :((( start packet btw\n");
            printf ("llwrite return %d :)))\n", res);
            free(packet);



            // send file info
            unsigned char buffer[6] = {0x01, 0x0d, 0x02, 0x7e, 0x50, 0x05};
            res = llwrite(buffer, 6);
            if (res < 0)
                printf("Sadge. Huge error, llwrite didn't return :(((\n");
            printf ("llwrite return %d :)))\n", res);



            // send end packet (abstract to dedicated function or similar)
            l1 = sizeof(file_stat.st_size);
            l2 = strlen(filename);
            int end_size = 5 + l1 + l2;

            unsigned char *packet1 = (unsigned char *) malloc(end_size);
            memset(packet1, 0, end_size);
            packet1[0] = 3;
            packet1[1] = 0; // file size
            packet1[2] = l1;
            memcpy(&packet1[3], &(file_stat.st_size), l1);
            packet1[3+l1] = 1; // file name
            packet1[4+l1] = l2;
            memcpy(&packet1[5+l1], filename, l2);
            res = llwrite(packet1, end_size);
            if (res < 0)
                printf("Sadge. Huge error, llwrite didn't return :((( end packet btw\n");
            printf ("llwrite return %d :)))\n", res);
            free(packet1);

            
            break;

        case RECEIVER:
            unsigned char buf[MSG_MAX_SIZE] = {0};
            do{
                res = llread(buf);
                if (res < 0)
                    printf("Sadge. Huge error, llread didn't return :(((\n");

                printf ("llrread return %d :)))\n", res);
            } while (buf[4]!=3);    
            
    }

    if (llclose(1) == -1)
        printf("llclose failed\n");
}
