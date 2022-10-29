#include "message.h"

#define MAX_BUF_SIZE 16

int send_s_frame(int fd, uint8_t address, uint8_t control, command response) {
    uint8_t *buffer = (uint8_t*)malloc(5);
    memset(buffer, 0, 5);
    int bytes;

    buffer[0] = FLAG;
    buffer[1] = address;
    buffer[2] = control;
    buffer[3] = BCC(buffer[1], buffer[2]);
    buffer[4] = FLAG;

    if ((bytes = send_message(fd, buffer, 5, response)) == -1){
        free(buffer);
        return -1;
    }

    free(buffer);
    return bytes;
}

int send_i_frame(int fd, uint8_t *data, int data_len, int packet) {
    int msg_len = data_len + 5; 
    uint8_t *buffer = (uint8_t*)malloc(msg_len);
    int bytes;

    // unsigned char buffer[data_len + 6] = {0};

    // flag A C BCC (4B)
    // DATA
    // BCC2 FLAG
    memset(buffer, 0, msg_len);
    buffer[0] = FLAG;
    buffer[1] = ADDR_E;
    //(get_curr_role() == RECEIVER) ? 0x03 : 0x01;
    buffer[2] = (packet << 6);
    
    buffer[3] = BCC(buffer[1], buffer[2]);

    uint8_t bcc2 = 0;
    for (int i = 0; i < data_len; i++) {
        buffer[i + 4] = data[i];
        bcc2 ^= data[i];
    }

    buffer[data_len + 4] = bcc2;
    

    uint8_t stuffed_msg[msg_len * 2];
    msg_len = msg_stuff(buffer, 4, msg_len, stuffed_msg);
    stuffed_msg[msg_len] = FLAG;
    msg_len++;

    if ((bytes = send_message(fd, stuffed_msg, msg_len, R_RR_REJ)) == -1){  
        free(buffer);
        return -1;
    }

    free(buffer);

    if ((packet == 0 && get_prev_response()==RR_1) || (packet == 1 && get_prev_response()==RR_0)){
        return bytes;
    }

    // handle REJ
    if ((packet == 0 && get_prev_response()==REJ_1) || (packet == 1 && get_prev_response()==REJ_0)){
        printf("Invalid message received and rejected\n");
        return -1;
    }

    return -1;
}

int send_message(int fd, uint8_t *frame, int msg_size, command response){
    int bytes;
    if (response == NO_RESP) { // no response expected
        if ((bytes = write(fd, frame, msg_size)) == -1) {
            printf("Write failed\n");
            return -1;
        }
        return bytes;
    } 

    set_command(response);

    reset_alarm_count();

    reset_state();

    while (get_alarm_count() < 3 && get_curr_state() != STOP) {
        set_alarm_flag(FALSE);
        if ((bytes = write(fd, frame, msg_size)) == -1){
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
            int read_byte = read(fd, buf + i, 1);

            printf("%x\n", buf[i]);
            if (read_byte !=-1){
                update_state(buf[i]);
            }
            i++;
        }

    }

    if (get_curr_state() != STOP) {
        printf("Failed to get response!\n");
        return -1;
    }

    return bytes;
}

int read_message(int fd, uint8_t * buf, int buf_size, command response){
    int i = 0;
    reset_state();
    set_command(response);

    while (get_curr_state() != STOP && !get_alarm_flag()){
        if (i >= buf_size){
            break;
        }
        int bytes = read(fd, buf + i, 1);


        printf("received -> %x\n", buf[i]);
        if (bytes !=-1){
            update_state(buf[i]);
        }  

        i++;
    }

    if (get_curr_state() != STOP) {
        printf("Failed to get response!\n");
        return -1;
    }
    return i;
}
