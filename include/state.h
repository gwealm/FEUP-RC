#pragma once

#define FLAG 0x7E
#define ADDR 0x03

/**
 * @brief Control Bit Options for Emissor
 */
typedef enum{
    // Set up (0x03)
    SET,

    // Disconnect (0x0B)
    DISC,
} command;

/**
 * @brief Machine Role Options
 */
typedef enum{

    TRANSMITTER,
    
    RECEIVER
} role;

/**
 * @brief Control Bit Options for Receiver
 */
typedef enum{

    R_NULL,

    // Unnumbered Acknowledgment (0x07)
    UA,

    // Receiver Ready / Positive ACK (0x05)
    RR,

    // Reject / Negative ACK (0x01)
    REJ
} response;

/**
 * @brief Possible states for the state machine
 */
typedef enum {
    // Starting State
    START,

    // Flag Received State
    FLAG_RCV,

    // Address Received State
    A_RCV,

    // Control Received State
    C_RCV,

    // BCC Verification Passed State
    BCC_OK,

    // Stop State
    STOP
} state;

/**
 * @brief Represents a State Machine
 */
typedef struct {
    // Current State
    state curr_state;

    // Address Byte
    unsigned char address;

    // Control Byte
    unsigned char control;

    // Previous response
    response prev_response;

    // Role
    role curr_role;

} state_machine;

// Getters
unsigned char get_address();
unsigned char get_control();
unsigned char get_flag();
state get_curr_state();
role get_curr_role();

// Setters
void set_address(unsigned char s);
void set_control(unsigned char s);
void set_state(state s);
void set_role(role r);

/**
 * @brief Updates the State Machine
 * 
 * @param byte 
 */
void update_state(unsigned char byte);

void reset_state();