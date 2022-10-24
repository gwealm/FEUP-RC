#pragma once

#include <stdint.h>

/**
 * @brief Control Bit Options for Emissor
 */
typedef enum{
    // Set up (0x03)
    CMD_SET,

    // Disconnect (0x0B)
    CMD_DISC,


    CMD_DATA,

    R_UA,

    R_RR_REJ
    
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

    // Receiver Ready / Positive ACK (RR_0 -> 0x05 / RR_1 -> 0x85)
    RR_0,
    RR_1,

    // Reject / Negative ACK (REJ_0 -> 0x01 / REJ_1 -> 0x81)
    REJ_0,
    REJ_1
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

    CDISCARD,

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
    uint8_t address;

    // Control Byte
    uint8_t control;

    // Previous response
    response prev_response;

    // Role
    role curr_role;

    // Currently awaitng command
    command curr_command

} state_machine;

// Getters
uint8_t get_address();
uint8_t get_control();
uint8_t get_flag();
state get_curr_state();
role get_curr_role();
command get_curr_command();
response get_prev_response();

// Setters
void set_address(uint8_t s);
void set_control(uint8_t s);
void set_state(state s);
void set_role(role r);
void set_command(command c);
void set_reponse(response r);

/**
 * @brief Updates the State Machine
 * 
 * @param byte 
 */
void update_state(uint8_t byte);

void reset_state();