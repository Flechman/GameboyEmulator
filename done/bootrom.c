

#include <stdio.h>
#include "error.h"
#include "bootrom.h"
#include "cartridge.h"

// ==== see bootrom.h ========================================
int bootrom_init(component_t* c){
    M_REQUIRE_NO_ERR(component_create(c, MEM_SIZE(BOOT_ROM)));
    data_t boottab[] = GAMEBOY_BOOT_ROM_CONTENT;
    memcpy((c->mem->memory), boottab, MEM_SIZE(BOOT_ROM));
    return ERR_NONE;
}

// ==== see bootrom.h ========================================
int bootrom_bus_listener(gameboy_t* gameboy, addr_t addr) {
    M_REQUIRE_NON_NULL(gameboy);
    if(addr == REG_BOOT_ROM_DISABLE && gameboy->boot != 0) {
        M_REQUIRE_NO_ERR(bus_unplug(gameboy->bus, &gameboy->bootrom));
        M_REQUIRE_NO_ERR(bus_forced_plug(gameboy->bus, &(gameboy->cartridge.c), BANK_ROM0_START, BANK_ROM0_END, 0));
        gameboy->boot = 0;
    }
    return ERR_NONE;
}
