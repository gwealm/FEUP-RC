// Link layer protocol implementation

#include "link_layer.h"
#include "state.h"
#include "constants.h"
#include "alarm.h"
#include "common.h"
#include "message.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source

struct termios oldtio;
int fd;
static uint8_t sequence_number = 0;

int start_transmissor(int fd)
{
    return send_s_frame(fd, ADDR, 0x03, R_UA);
}

int start_receiver(int fd)
{
    unsigned char message[5];
    if (read_message(fd, message, 5, CMD_SET) < 0)
        return -1;
    return send_s_frame(fd, ADDR, 0x07, NO_RESP);
}

////////////////////////////////////////////////
/// LLOPEN                                   ///
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters)
{

    // Open serial port device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);

    if (fd < 0)
    {
        printf("%s", connectionParameters.serialPort);
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag = connectionParameters.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    switch (connectionParameters.role)
    {
    case TRANSMITTER:
        newtio.c_cc[VMIN] = 0;                             // Blocking read until n chars received
        newtio.c_cc[VTIME] = connectionParameters.timeout; // Inter-character timer unused
        break;
    case RECEIVER:
        newtio.c_cc[VMIN] = 1; // Blocking read until n chars received
        break;
    }
    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    switch (connectionParameters.role)
    {
    case TRANSMITTER:
        set_role(TRANSMITTER);
        (void)signal(SIGALRM, alarm_handler);
        if (start_transmissor(fd) < 0)
        {
            printf("Could not start TRANSMITTER\n");
            return -1;
        }
        break;

    case RECEIVER:
        set_role(RECEIVER);
        (void)signal(SIGALRM, alarm_handler);

        if (start_receiver(fd) < 0)
        {
            printf("Could not start RECEIVER\n");
            return -1;
        }

        break;
    }

    printf("Connection open\n");
    return fd;
}

int close_receiver(int fd)
{
    printf("Disconnecting receiver\n");
    unsigned char message[5];
    if (read_message(fd, message, 5, CMD_DISC) < 0)
        return -1;
    return send_s_frame(fd, ADDR, 0x0B, R_UA);
}

int close_transmissor(int fd)
{
    printf("DISCONNECTING TRANSMITTER...\n");
    if (send_s_frame(fd, ADDR, 0x0B, CMD_DISC) < 0)
        return -1;
    return send_s_frame(fd, ADDR, 0x07, NO_RESP);
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize)
{
    int bytes;

    if ((bytes = send_i_frame(fd, buf, bufSize, sequence_number)) == -1)
    {
        return -1;
    }

    printf("llwrite: %d bytes written\n", bytes);

    sequence_number ^= 0x01; // if 0 -> 1, if 1 -> 0
    return bytes;
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet)
{

    unsigned char *buf = (unsigned char *)malloc(MSG_MAX_SIZE * 2);

    int read_bytes = 0;

    if ((read_bytes = read_message(fd, buf, MSG_MAX_SIZE * 2, CMD_DATA)) < 0){
        free(buf);
        return -1;
    }

    uint8_t *destuffed_msg = (uint8_t *)malloc(MSG_MAX_SIZE);

    int msg_size;

    if ((msg_size = msg_destuff(buf, 4, read_bytes, destuffed_msg)) < 0)
    {
        free(buf);
        free(destuffed_msg);
        printf("Read failed\n");
        return -1;
    }

    free(buf);

    printf("bytes: %d\n", read_bytes);
    printf("destuff size: %d\2\n", msg_size);
    unsigned char rcv_bcc2 = destuffed_msg[msg_size - 2];
    printf("rcv_bcc2: %x\n", rcv_bcc2);
    unsigned char bcc2 = generate_bcc2(destuffed_msg + 4, msg_size - 6); // data and data length (check these args)
    printf("bcc2: %x\n", bcc2);

    // case correct bcc2 and correct sequence_number
    if ((rcv_bcc2 == bcc2) && ((get_control() == 0x00 && sequence_number == 0) || (get_control() == 0x40 && sequence_number == 1)))
    { // rewrite condition
        send_s_frame(fd, ADDR, 0x05 | ((sequence_number ^ 0x01) << 7), NO_RESP);
        memcpy(packet, destuffed_msg + 4, msg_size - 6); // only copy data to packet
        free(destuffed_msg);
        sequence_number ^= 0x01;
        return msg_size - 6; // return size of data
    }

    // case correct bcc2 and incorrect sequence_number (ignore)
    if ((rcv_bcc2 == bcc2))
    {
        send_s_frame(fd, ADDR, 0x05 | (sequence_number << 7), NO_RESP); // accept wrong sequence number (duplicate)
        free(destuffed_msg);
        return -1;
    }

    // case incorrect bcc2 (reject)
    send_s_frame(fd, ADDR, 0x01 | ((sequence_number ^ 0x01) << 7), NO_RESP);
    free(destuffed_msg);
    return -1;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics)
{

    switch (get_curr_role())
    {
    case TRANSMITTER:
        if (close_transmissor(fd) < 0)
        {
            printf("Could not close TRANSMITTER\n");
            return -1;
        }
        break;

    case RECEIVER:
        if (close_receiver(fd) < 0)
        {
            printf("Could not close RECEIVER\n");
            return -1;
        }
        break;
    }

    printf("Connection closing\n");

    sleep(1);
    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}
