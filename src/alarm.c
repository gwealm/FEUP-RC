#include "alarm.h"
#include "constants.h"


int alarm_flag = FALSE;
int alarm_count = 0;

void reset_alarm_count(){
    alarm_count = 0;
}

void set_alarm_flag(int flag){
    alarm_flag = flag;
}

int get_alarm_count(){
    return alarm_count;
}

int get_alarm_flag(){
    return alarm_flag;
}

// Alarm function handler
void alarm_handler(int signal)
{
    set_alarm_flag(TRUE);
    alarm_count++;

    printf("Alarm #%d\n", alarm_count);
}