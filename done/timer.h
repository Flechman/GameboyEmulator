#pragma once


#include <stdint.h>

#include "component.h"
#include "bit.h"
#include "cpu.h"
#include "bus.h"

#ifdef __cplusplus
extern "C" {
#endif

// TIMER BUS REG ADDR

#define REG_DIV         0xFF04
#define REG_TIMA        0xFF05
#define REG_TMA         0xFF06
#define REG_TAC         0xFF07

#define TIMER_START     REG_DIV
#define TIMER_END       REG_TAC
#define TIMER_SIZE      ((REG_TAC-REG_DIV)+1)

/**
 * @brief Timer type
 */
typedef struct {
    cpu_t* cpu;
    uint16_t counter;
} gbtimer_t;

/**
 * @brief Initiates a timer
 *
 * @param timer timer to initiate
 * @param cpu cpu to use for timer
 * @return error code
 */
int timer_init(gbtimer_t* timer, cpu_t* cpu);


/**
 * @brief Run one Timer cycle
 *
 * @param timer timer to cycle
 * @return error code
 */
int timer_cycle(gbtimer_t* timer);


/**
 * @brief Timer bus listening handler
 *
 * @param timer timer
 * @param address trigger address
 * @return error code
 */
int timer_bus_listener(gbtimer_t* timer, addr_t addr);

#ifdef __cplusplus
}
#endif
