// Link layer protocol implementation


#include "link_layer.h"
#include "state.h"
#include "constants.h"
#include "alarm.h"
#include "common.h"

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
#define BUF_SIZE 16

////////////////////////////////////////////////
/// LLOPEN                                   ///   
////////////////////////////////////////////////
int llopen(LinkLayer connectionParameters) {

    // Open serial port device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(connectionParameters.serialPort, O_RDWR | O_NOCTTY);
    
    
    if (fd < 0) {
        printf("%s", connectionParameters.serialPort);
        perror(connectionParameters.serialPort);
        exit(-1);
    }

    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1) {
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
    newtio.c_cc[VMIN] = 1;  // Blocking read until n chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    printf("New termios structure set\n");

    switch (connectionParameters.role){
        case TRANSMITTER:
            set_role(TRANSMITTER);
            (void)signal(SIGALRM, alarm_handler);
            if (start_transmissor(fd) < 0){
                printf("Could not start TRANSMITTER\n");
                return -1;
            }
            break;
            /*unsigned char buf[BUF_SIZE] = {0};
            // Set alarm function handler
            (void)signal(SIGALRM, alarm_handler);
            // Set alarm count as 0 (first time sending)
            reset_alarm_count();

            while (get_alarm_count() < 3){
                set_alarm_flag(FALSE);

                memset(buf, 0, BUF_SIZE);
                buf[0] = FLAG;
                buf[1] = ADDR;
                buf[2] = 0x03;
                buf[4] = FLAG;

                if (llwrite(buf, BUF_SIZE)>0)
                    printf("SET message sent successfully\n");

                alarm(3);
                

                memset(buf, 0, BUF_SIZE);
                
                if (llread(buf) == 0){
                    printf("UA received successfully\n");
                    reset_alarm_count();
                    return 1;
                }
                
            }

            free(buf);
            break;*/
        case RECEIVER:
            set_role(RECEIVER);
            (void)signal(SIGALRM, alarm_handler);

            if (start_receiver(fd) < 0){
                printf("Could not start RECEIVER\n");
                return -1;
            }
            break;
            /*
            unsigned char packet[BUF_SIZE] = {0};
            
            if(llread(packet) == 0)
                printf("SET received successfully\n");
            else
                break;
            

            memset(packet, 0, BUF_SIZE);

            packet[0] = FLAG;
            packet[1] = ADDR;
            packet[2] = 0x07;
            packet[3] = 0x04;
            packet[4] = FLAG;

            if (llwrite(packet, BUF_SIZE)>0){
                printf("UA sent successfully\n");
                return 1;
            }

            break;*/
    }

    return 1;
}

int start_receiver(int fd) {
    unsigned char message[5];
    if (read_message(fd, message, 5) != 0) 
        return -1;
    return send_s_frame(fd, ADDR, 0x07, NO_RESP);
}


int start_transmissor(int fd) {
    return send_s_frame(fd, ADDR, 0x03, R_UA);
}

////////////////////////////////////////////////
// LLWRITE
////////////////////////////////////////////////
int llwrite(const unsigned char *buf, int bufSize) {

    /*
    
    
    int len = msg_stuff(buf, )

    int bytes = write(fd, buf, bufSize);

        


    printf("%d bytes written\n", bytes);

    

    return bytes;*/
}

////////////////////////////////////////////////
// LLREAD
////////////////////////////////////////////////
int llread(unsigned char *packet) {
    /*
    int i = 0;
    reset_state();

    while (get_curr_state() != STOP && !get_alarm_flag()) {

        if (i >= BUF_SIZE){
            continue;
        }

        // Returns after 1 char has been input
        int bytes = read(fd, &packet[i], 1);


        printf("%x\n", packet[i]);
        if (bytes !=-1){
            update_state(packet[i]);
        }

        i++;
    }

    if (get_curr_state() != STOP) {
        printf("Failed to get response!\n");
        return -1;
    }



    return 0;*/
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llclose(int showStatistics) {

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 1;
}
