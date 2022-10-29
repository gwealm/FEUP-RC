// Application layer protocol implementation

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#include "application_layer.h"
#include "link_layer.h"
#include "state.h"
#include "constants.h"

int parse_packet(unsigned char *buffer, int buffer_size, const char *file_path)
{
    static int dest_file_fd;
    switch (buffer[0])
    {
    case 1:
        unsigned data_size = buffer[3] + 256 * buffer[2];
        if (write(dest_file_fd, &buffer[4], data_size) < 0)
        {
            printf("Couldn't write to destination file\n");
            return -1;
        }
        return 0;
    case 2:
        if ((dest_file_fd = open(file_path, O_WRONLY | O_CREAT, 0777)) < 0)
        {
            printf("Couldn't open destination file\n");
            return -1;
        }
        // read size for checking at end
        return 0;
    case 3:
        if (close(dest_file_fd) < 0)
        {
            printf("Couldn't close destination file\n");
            return -1;
        }
        // maybe check file size here
        return 0;
    default:
        printf("Invalid packet received\n");
        return -1;
    }
}

void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{

    int ll_role = -1;

    if (!strcmp(role, "tx"))
    {
        ll_role = 0;
    }
    else if (!strcmp(role, "rx"))
    {
        ll_role = 1;
    }

    if (ll_role < 0)
        exit(-1);

    LinkLayer ll;
    strncpy(ll.serialPort, serialPort, sizeof(ll.serialPort) - 1);
    ll.serialPort[sizeof(ll.serialPort) - 1] = '\0';
    ll.role = ll_role;
    ll.baudRate = baudRate;
    ll.nRetransmissions = nTries;
    ll.timeout = timeout;

    if (llopen(ll) == -1)
        printf("llopen failed\n");

    switch (ll.role)
    {
        int res;
    case TRANSMITTER:
        int file_fd;
        struct stat file_stat;

        // read file info using stat
        if (stat(filename, &file_stat) < 0)
        {
            perror("Error getting file information.");
            exit(-1);
        }

        // open file
        if ((file_fd = open(filename, O_RDONLY)) < 0)
        {
            perror("Error opening file.");
            exit(-1);
        }

        // send start packet (abstract to dedicated function or similar), possibly define some macros
        int l1 = sizeof(file_stat.st_size);
        int l2 = strlen(filename);
        int start_size = 5 + l1 + l2;

        unsigned char *packet = (unsigned char *)malloc(start_size);
        memset(packet, 0, start_size);
        packet[0] = 2;
        packet[1] = 0; // file size
        packet[2] = l1;
        memcpy(&packet[3], &(file_stat.st_size), l1);
        packet[3 + l1] = 1; // file name
        packet[4 + l1] = l2;
        memcpy(&packet[5 + l1], filename, l2);
        res = llwrite(packet, start_size);
        if (res < 0)
            printf("llwrite failed\n");
        printf("llwrite return %d :)))\n", res);
        free(packet);

        unsigned char msg[MSG_MAX_SIZE - 6]; // should have macro for this
        unsigned send_bytes;
        unsigned packet_number = 0;

        while ((send_bytes = read(file_fd, msg, MSG_MAX_SIZE - 10)) > 0)
        { // accounts for header and packet info
            unsigned char data_packet[MSG_MAX_SIZE - 6];
            data_packet[0] = 1; // data
            data_packet[1] = packet_number % 255;
            data_packet[2] = (send_bytes / 256);
            data_packet[3] = (send_bytes % 256);
            memcpy(&data_packet[4], msg, send_bytes);

            if (llwrite(data_packet, send_bytes + 4) < 0)
            {
                printf("llwrite failed\n");
                exit(-1);
            }
            printf("Sent packet %d\n", packet_number);

            packet_number++;
        }

        // send end packet (abstract to dedicated function or similar)
        l1 = sizeof(file_stat.st_size);
        l2 = strlen(filename);
        int end_size = 5 + l1 + l2;

        unsigned char *packet1 = (unsigned char *)malloc(end_size);
        memset(packet1, 0, end_size);
        packet1[0] = 3;
        packet1[1] = 0; // file size
        packet1[2] = l1;
        memcpy(&packet1[3], &(file_stat.st_size), l1);
        packet1[3 + l1] = 1; // file name
        packet1[4 + l1] = l2;
        memcpy(&packet1[5 + l1], filename, l2);
        res = llwrite(packet1, end_size);
        if (res < 0)
            printf("llwrite failed\n");
        printf("llwrite return %d :)))\n", res);
        free(packet1);

        break;

    case RECEIVER:
        unsigned char buf[MSG_MAX_SIZE - 6] = {0}; // allocate max size of data packet

        res = llread(buf);
        if (res < 0)
        {
            printf("llread failed\n");
            return;
        }
        printf("llread return %d\n", res);

        // parse control packet (start packet)
        if (parse_packet(buf, res, filename) < 0)
        {
            return;
        }

        do
        {
            res = llread(buf);
            if (res < 0)
            {
                printf("llread failed\n");
                continue;
            }
            printf("llrread return %d\n", res);
            // parse packet
            if (parse_packet(buf, res, filename) < 0)
            {
                return;
            }

        } while (buf[0] != 3);
    }

    if (llclose(1) == -1)
        printf("llclose failed\n");
}
