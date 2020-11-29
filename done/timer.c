#include <stdio.h>
#include "timer.h"
#include "error.h"
#include "bus.h"

// ==== see timer.h ========================================
int timer_init(gbtimer_t* timer, cpu_t* cpu) {
    M_REQUIRE_NON_NULL(timer);
    M_REQUIRE_NON_NULL(cpu);
    timer->cpu = cpu;
    timer->counter = 0;
    return ERR_NONE;
}

//Auxiliary method to help increment secondary timer
bit_t timer_state(gbtimer_t* timer) {
    //timer has been checked at a higher level
    data_t tac = 0;
    (void)bus_read(*(timer->cpu->bus), REG_TAC, &tac);
    bit_t tac_bit2 = bit_get(tac, 2);
    bit_t counter_bit = 0;
    switch (tac & 3) {
        case 0:
            counter_bit = bit_get(msb8(timer->counter), 1);
            break;
        case 1:
            counter_bit = bit_get(timer->counter, 3);
            break;
        case 2:
            counter_bit = bit_get(timer->counter, 5);
            break;
        case 3:  
            counter_bit = bit_get(timer->counter, 7);
            break;
    }
    return (counter_bit & tac_bit2);
}

//Auxiliary method to help increment secondary timer
int timer_incr_if_state_change(gbtimer_t* timer, bit_t old_state) {
    //timer has been checked at a higher level
    if(old_state == 1 && timer_state(timer) == 0) {
        data_t second_timer = 0;
        M_REQUIRE_NO_ERR(bus_read(*timer->cpu->bus, REG_TIMA, &second_timer));
        if(second_timer == 0xFF) {
            cpu_request_interrupt(timer->cpu, TIMER);
            M_REQUIRE_NO_ERR(bus_read(*timer->cpu->bus, REG_TMA, &second_timer));
            M_REQUIRE_NO_ERR(bus_write(*timer->cpu->bus, REG_TIMA, second_timer));
            return ERR_NONE;
        } else {
            M_REQUIRE_NO_ERR(bus_write(*timer->cpu->bus, REG_TIMA, second_timer+1));
            return ERR_NONE;
        }
    }
    return ERR_NONE;
}

// ==== see timer.h ========================================
int timer_cycle(gbtimer_t* timer) {
    M_REQUIRE_NON_NULL(timer);
    M_REQUIRE_NON_NULL(timer->cpu);
    M_REQUIRE_NON_NULL(timer->cpu->bus);
    bit_t curr_state = timer_state(timer);
    timer->counter += 4;
    M_REQUIRE_NO_ERR(bus_write(*timer->cpu->bus, REG_DIV, msb8(timer->counter)));
    M_REQUIRE_NO_ERR(timer_incr_if_state_change(timer, curr_state));
    return ERR_NONE;
}

// ==== see timer.h ========================================
int timer_bus_listener(gbtimer_t* timer, addr_t addr) {
    M_REQUIRE_NON_NULL(timer);
    M_REQUIRE_NON_NULL(timer->cpu);
    M_REQUIRE_NON_NULL(timer->cpu->bus);
    if(addr == REG_DIV) {
        bit_t curr_state = timer_state(timer);
        timer->counter = 0;
        bus_write(*(timer->cpu->bus), REG_DIV, (data_t)0);
        M_REQUIRE_NO_ERR(timer_incr_if_state_change(timer, curr_state));
    }
    if(addr == REG_TAC) {
        bit_t curr_state = timer_state(timer);
        M_REQUIRE_NO_ERR(timer_incr_if_state_change(timer, curr_state));
    }
    return ERR_NONE;
}
