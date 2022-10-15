#pragma once

#include <unistd.h>
#include <signal.h>
#include <stdio.h>

void reset_alarm_count();
void set_alarm_flag(int flag);
int get_alarm_count();
int get_alarm_flag();
void alarm_handler(int signal);