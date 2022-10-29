#pragma once

#define FLAG 0x7E
#define ESCAPE 0x7d
#define ADDR 0x03

// #define TRANSMITTER 0
// #define RECEIVER 1

#define TRUE 1
#define FALSE 0

#define BIT(n) (1 << n)

// Message
#define CTRL_S(s) ((s == 0) ? 0 : BIT(6))
#define BCC(a, b) ((a) ^ (b))
#define ADDR_E 0x03

#define CTRL_SET 0x03
#define CTRL_DISC 0x0B
#define CTRL_UA 0x07
#define CTRL_RR(r) ((r == 0) ? 0x05 : 0x85)
#define CTRL_REJ(r) ((r == 0) ? 0x01 : 0x81)

#define MSG_MAX_SIZE 1000

#define NO_RESP -1