#pragma once


#include "memory.h"     // addr_t and data_t
#include "cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Joypad register bus address
#define REG_P1  0xFF00

// Number of (electronic) key rows
#define NB_GB_KEY_ROWS 2

// Number of (electronic) key columns
#define NB_GB_KEY_COLS 4

/**
 * @brief joypad type
 */
typedef struct {
    cpu_t* cpu;
    data_t* p_P1;  // a pointer to P1 content (bus exposed)
    data_t intern; // internal P1 state, hidden from bus ; this is simply to prevent write on forbiden P1 bits
    uint8_t old_state;
    uint8_t keys_state[NB_GB_KEY_ROWS];
} joypad_t;


/**
 * @brief Game Boy keys
 */
typedef enum {
    RIGHT_KEY, LEFT_KEY, UP_KEY,     DOWN_KEY,
    A_KEY,     B_KEY,    SELECT_KEY, START_KEY,
    NB_GB_KEYS
} gb_key_t;


/**
 * @brief Initiates a joypad and plugs it onto the bus (from CPU)
 *
 * @param pad joypad to initiate
 * @return error code
 */
int joypad_init_and_plug(joypad_t* pad, cpu_t* cpu);


/**
 * @brief Joypad bus listening handler
 *
 * @param pad joypad
 * @param address trigger address
 * @return error code
 */
int joypad_bus_listener(joypad_t* pad, addr_t addr);


/**
 * @brief Joypad key press handler
 *
 * @param pad joypad
 * @param key the key pressed
 * @return error code
 */
int joypad_key_pressed(joypad_t* pad, gb_key_t key);


/**
 * @brief Joypad key release handler
 *
 * @param pad joypad
 * @param key the key released
 * @return error code
 */
int joypad_key_released(joypad_t* pad, gb_key_t key);

#ifdef __cplusplus
}
#endif
