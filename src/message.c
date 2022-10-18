#include "message.h"

#define MAX_BUF_SIZE 256

int send_s_frame(int fd, uint8_t address, uint8_t control, command response) {
    uint8_t *buffer = (uint8_t*)malloc(5);
    memset(buffer, 0, 5);

    buffer[0] = FLAG;
    buffer[1] = address;
    buffer[2] = control;
    buffer[3] = BCC(buffer[1], buffer[2]);
    buffer[4] = FLAG;

    if (send_message(fd, buffer, 5, response) != 0){
        free(buffer);
        return -1;
    }

    free(buffer);
    return 0;
}

int send_i_frame(int fd, uint8_t *data, int data_len, int packet) {
    int msg_len = data_len + 6; 
    uint8_t *buffer = (uint8_t*)malloc(msg_len);

    // unsigned char buffer[data_len + 6] = {0};

    // flag A C BCC (4B)
    // DATA
    // BCC2 FLAG
    memset(buffer, 0, msg_len);
    buffer[0] = FLAG;
    buffer[1] = ADDR_E;
    //(get_curr_role() == RECEIVER) ? 0x03 : 0x01;
    buffer[2] = CTRL_S(buffer);
    
    buffer[3] = BCC(buffer[1], buffer[2]);

    uint8_t bcc2 = 0;
    for (int i = 0; i < data_len; ++i) {
        buffer[i + 4] = data[i];
        bcc2 ^= data[i];
    }

    buffer[data_len + 4] = bcc2;

    uint8_t stuffed_msg[msg_len * 2];
    msg_len = msg_stuff(buffer, 1, msg_len, stuffed_msg);
    stuffed_msg[msg_len++] = FLAG;
    
    reset_alarm_count();
    int ack_received = FALSE;

    while (get_alarm_count() < 3 && !ack_received) {
        respon 

        alarm_handler();
    }


    

    return buffer;
}

int send_message(int fd, uint8_t *frame, int msg_size, command response){
    if (response == NO_RESP) { // no response expected
        if (write(fd, frame, msg_size) == -1) {
            printf("Write failed\n");
            return -1;
        }
        return 0;
    } 

    reset_alarm_count();

    reset_state();

    while (get_alarm_count() < 3 && get_curr_state() != STOP) {
        set_alarm_flag(FALSE);
        if (write(fd, frame, msg_size) == -1){
            printf("Write failed\n");
            return -1;
        }
        printf("Message sent\n");

        unsigned char buf[MAX_BUF_SIZE] = {0};

        alarm(3);

        int i = 0;
        

        while (get_curr_state() != STOP && !get_alarm_flag()){
            if (i >= MAX_BUF_SIZE){
                continue;
            }
            int bytes = read(fd, buf + i, 1);


            printf("%x\n", buf[i]);
            if (bytes !=-1){
                update_state(buf[i]);
            }

            i++;
        }

    }

    if (get_curr_state() != STOP) {
        printf("Failed to get response!\n");
        return -1;
    }

    return 0;
}

int read_message(int fd, uint8_t * buf, int buf_size){
    int i = 0;
    reset_state();

    while (get_curr_state() != STOP && !get_alarm_flag()){
        if (i >= buf_size){
            break;
        }
        int bytes = read(fd, buf + i, 1);


        printf("%x\n", buf[i]);
        if (bytes !=-1){
            update_state(buf[i]);
        }  

        i++;
    }

    if (get_curr_state() != STOP) {
        printf("Failed to get response!\n");
        return -1;
    }

    return 0;
}
