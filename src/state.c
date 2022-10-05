#include "state.h"

#include <stdio.h>

state_machine state_m;

unsigned char get_address() {
    return state_m.address;
}

unsigned char get_control() {
    return state_m.control;
}

state get_curr_state() {
    return state_m.curr_state;
}

void set_address(unsigned char s) {
    state_m.address = s;
}

void set_control(unsigned char c) {
    state_m.control = c;
}

void set_state(state s) {
    state_m.curr_state = s;
}

void update_state (unsigned char byte){
    switch(state_m.curr_state){
        case START:
            if (byte == FLAG) 
                state_m.curr_state = FLAG_RCV;
            break;
        case FLAG_RCV:
            if (byte == FLAG)
                break;
            else if (byte == ADDR)
                state_m.curr_state = A_RCV; 
            else 
                state_m.curr_state = START;

        case A_RCV:
            if (byte == FLAG) 
                state_m.curr_state = FLAG_RCV;
            else if (byte == 0x03)
                state_m.curr_state = C_RCV; 
            else 
                state_m.curr_state = START;    
            break;
        case C_RCV:
            if (byte == (state_m.address ^ state_m.control)) 
                state_m.curr_state = BCC_OK;
            else if (byte == FLAG) 
                state_m.curr_state = FLAG_RCV;
            else 
                state_m.curr_state = START;    
            break;
        case BCC_OK:
            if (byte == FLAG) 
                state_m.curr_state = STOP;
            else 
                state_m.curr_state = START;    
            break;
        case STOP:
            break;
    }
}


void reset_state() {
    state_m.curr_state = START;
    state_m.prev_response = NULL;
}