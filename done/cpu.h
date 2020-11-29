#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "alu.h"
#include "bus.h"

//=========================================================================
/**
 * @brief Type to represent CPU interupts
 */
typedef enum {
    VBLANK, LCD_STAT, TIMER, SERIAL, JOYPAD, INTERRUPT_COUNT
} interrupt_t ;


#define REG_IF          0xFF0F
#define REG_IE          0xFFFF
#define HIGH_RAM_START   0xFF80
#define HIGH_RAM_END     0xFFFE
#define HIGH_RAM_SIZE ((HIGH_RAM_END - HIGH_RAM_START)+1)
#define INTERRUPT_ADDR 0x40
#define CREATE_PAIR(a, b, ab) union { struct { uint8_t a; uint8_t b; }; uint16_t ab; }

//=========================================================================
/**
 * @brief Type to represent CPU
 */
typedef struct {
    CREATE_PAIR(F, A, AF);
    CREATE_PAIR(C, B, BC);
    CREATE_PAIR(E, D, DE);
    CREATE_PAIR(L, H, HL);
    uint16_t PC;
    uint16_t SP;
    alu_output_t alu;
    bus_t* bus;
    bit_t IME;
    uint8_t IE;
    uint8_t IF;
    bit_t HALT; //1 if processor stopped, else 0
    component_t high_ram;
    addr_t write_listener;
    uint8_t idle_time;
} cpu_t;

//=========================================================================
/**
 * @brief Run one CPU cycle
 * @param cpu (modified), the CPU which shall run
 * @param cycle, the cycle number to run, starting from 0
 * @return error code
 */
int cpu_cycle(cpu_t* cpu);


/**
 * @brief Plugs a bus into the cpu
 *
 * @param cpu cpu to plug into
 * @param bus bus to plug
 *
 * @return error code
 */
int cpu_plug(cpu_t* cpu, bus_t* bus);


/**
 * @brief Starts the cpu by initializing all registers at zero
 *
 * @param cpu cpu to start
 *
 * @return error code
 */
int cpu_init(cpu_t* cpu);


/**
 * @brief Frees a cpu
 *
 * @param cpu cpu to free
 */
void cpu_free(cpu_t* cpu);


/**
 * @brief Set an interruption
 */
void cpu_request_interrupt(cpu_t* cpu, interrupt_t i);

/**
 * @brief Auxiliary method for checking pending interrupts
 */
int extract_interrupt_index_if_enable(cpu_t* cpu);


#ifdef __cplusplus
}
#endif
