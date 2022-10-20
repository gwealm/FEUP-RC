#pragma once

#include <stdint.h>

uint8_t generate_bcc2(uint8_t *data, int len);

int msg_stuff(uint8_t *buffer, int start, int msg_size, uint8_t *stuffed_msg);

int msg_destuff(uint8_t *buffer, int start, int msg_size, uint8_t *destuffed_msg);