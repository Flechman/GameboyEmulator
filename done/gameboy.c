
#include <stdio.h>
#include "gameboy.h"
#include "error.h"
#include "bus.h"
#include "component.h"
#include "bootrom.h"
#include "joypad.h"
#include "lcdc.h"

#define init_and_plug(comp,name) \
    M_REQUIRE_NO_ERR(component_create(&comp, MEM_SIZE(name))); \
    M_REQUIRE_NO_ERR(bus_plug(gameboy->bus, &comp, name ## _START, name ## _END));

int gameboy_create(gameboy_t* gameboy, const char* filename) {
    M_REQUIRE_NON_NULL(gameboy);
    

    //Creation of the bus
    for(int i=0; i<BUS_SIZE; ++i) {
        gameboy->bus[i] = NULL;
    }
    
    //Creation of the work RAM and the echo RAM
    component_t workRAM;
    component_t echoRAM;
    component_t registers;
    component_t extRAM;
    component_t videoRAM;
    component_t graphRAM;
    component_t useless;

    init_and_plug(workRAM, WORK_RAM);
    init_and_plug(registers, REGISTERS);
    init_and_plug(extRAM, EXTERN_RAM);
    init_and_plug(videoRAM, VIDEO_RAM);
    init_and_plug(graphRAM, GRAPH_RAM);
    init_and_plug(useless, USELESS);
    
    M_REQUIRE_NO_ERR(bootrom_init(&gameboy->bootrom));
    M_REQUIRE_NO_ERR(cartridge_init(&(gameboy->cartridge),filename));
    
    //Connections to bus
    M_REQUIRE_NO_ERR(cartridge_plug(&gameboy->cartridge, gameboy->bus));
    bootrom_plug(&gameboy->bootrom, gameboy->bus);
    
    //Shares memory
    M_REQUIRE_NO_ERR(component_shared(&echoRAM, &workRAM));
    M_REQUIRE_NO_ERR(bus_plug(gameboy->bus, &echoRAM, ECHO_RAM_START, ECHO_RAM_END));

    //Creation of the array of components
    gameboy->components[0] = workRAM;
    gameboy->components[1] = registers;
    gameboy->components[2] = extRAM;
    gameboy->components[3] = videoRAM;
    gameboy->components[4] = graphRAM;
    gameboy->components[5] = useless;

    M_REQUIRE_NO_ERR(cpu_init(&gameboy->cpu));
    M_REQUIRE_NO_ERR(cpu_plug(&gameboy->cpu, &gameboy->bus));
    M_REQUIRE_NO_ERR(timer_init(&gameboy->timer, &gameboy->cpu));
    M_REQUIRE_NO_ERR(joypad_init_and_plug(&(gameboy->pad), &(gameboy->cpu)));
    M_REQUIRE_NO_ERR(lcdc_init(gameboy));
    M_REQUIRE_NO_ERR(lcdc_plug(&(gameboy->screen), gameboy->bus));
    
    gameboy->cycles=0;
    gameboy->boot=1;
    gameboy->nb_components=GB_NB_COMPONENTS;

    return ERR_NONE;
}

void gameboy_free(gameboy_t* gameboy) {
    if(gameboy != NULL) {
        //free bus
        for(int i=0; i<BUS_SIZE; ++i) {
                gameboy->bus[i] = NULL;
        }

        //free components
        for(int i=0; i<GB_NB_COMPONENTS; ++i) {
            bus_unplug(gameboy->bus, &gameboy->components[i]);
            component_free(&gameboy->components[i]);
        }
        
        component_free(&gameboy->bootrom);
        lcdc_free(&(gameboy->screen));
        cartridge_free(&gameboy->cartridge);
        cpu_free(&gameboy->cpu);

        gameboy = NULL;
    }
}

#ifdef BLARGG
static int blargg_bus_listener(gameboy_t* gameboy, addr_t addr) {
    M_REQUIRE_NON_NULL(gameboy);
    if(addr == BLARGG_REG) {
    data_t data = 0;
        M_REQUIRE_NO_ERR(bus_read(*(gameboy->cpu.bus), BLARGG_REG, &data));
        printf("%c", data);
    }
    return ERR_NONE;
}
#endif

int gameboy_run_until(gameboy_t* gameboy, uint64_t cycle) {
    while(gameboy->cycles < cycle) {
        M_EXIT_IF_ERR(timer_cycle(&gameboy->timer));
        M_EXIT_IF_ERR(cpu_cycle(&gameboy->cpu));
        M_REQUIRE_NO_ERR(lcdc_cycle(&gameboy->screen, gameboy->cycles));
        M_REQUIRE_NO_ERR(lcdc_bus_listener(&gameboy->screen, gameboy->cpu.write_listener));
        M_REQUIRE_NO_ERR(bootrom_bus_listener(gameboy, gameboy->cpu.write_listener));
        M_REQUIRE_NO_ERR(timer_bus_listener(&gameboy->timer, gameboy->cpu.write_listener));
        M_REQUIRE_NO_ERR(joypad_bus_listener(&gameboy->pad, gameboy->cpu.write_listener));
        ++gameboy->cycles;
        #ifdef BLARGG
            M_EXIT_IF_ERR(blargg_bus_listener(gameboy, gameboy->cpu.write_listener));
        #endif
    }
    return ERR_NONE;
}
