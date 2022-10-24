#pragma once

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
#include <unistd.h>

int send_s_frame(int fd, uint8_t address, uint8_t control, command response);
int send_i_frame(int fd, uint8_t *data, int data_len, int packet);
int send_message(int fd, uint8_t *frame, int msg_size, command response);
int read_message(int fd, uint8_t * buf, int buf_size, command response);