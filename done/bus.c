
#include <stdio.h>
#include "bus.h"
#include "error.h"
#include "bit.h"


int bus_remap(bus_t bus, component_t* c, addr_t offset) {
    M_REQUIRE_NON_NULL(c);
    M_REQUIRE_NON_NULL(c->mem);
    M_REQUIRE_NON_NULL(bus);
    M_REQUIRE_NON_NULL(c->mem->memory);
    M_REQUIRE(((c->start) <= (c->end)),ERR_BAD_PARAMETER,"start should smaller or equals to end");
    M_REQUIRE((c->end - c->start + offset) < c->mem->size, ERR_BAD_PARAMETER, "Parameters out of bounds: sould be < %zu\n", c->mem->size);
    
    int index = offset;
    if(c->start == 0 && c->end == 0) { return ERR_NONE; }
    for(int j=c->start; j<=c->end; ++j) {
        bus[j] = &(c->mem->memory[index]);
        ++index;
    }
    return ERR_NONE;
}

int bus_forced_plug(bus_t bus, component_t* c, addr_t start, addr_t end, addr_t offset) {
    M_REQUIRE_NON_NULL(c);
    M_REQUIRE_NON_NULL(c->mem);
    if(end-start + offset > c->mem->size) {
        c->end = 0;
        c->start = 0;
        return ERR_BAD_PARAMETER;
    }
    c->end = end;
    c->start = start;
    return bus_remap(bus, c, offset);
}

int bus_plug(bus_t bus, component_t* c, addr_t start, addr_t end) {
    M_REQUIRE_NON_NULL(c);
    M_REQUIRE_NON_NULL(c->mem);
    
    for(int j=start; j<=end; ++j) {
        if(bus[j] != NULL) {
            c->end = 0;
            c->start = 0;
            return ERR_ADDRESS;
        }
    }
    if(end - start >= c->mem->size) {
        c->end = 0;
        c->start = 0;
        return ERR_ADDRESS;
    }
    return bus_forced_plug(bus, c, start, end, 0);
}

int bus_unplug(bus_t bus, component_t* c) {
    M_REQUIRE_NON_NULL(c);
    if(c->start == 0 && c->end == 0) { return ERR_NONE; }
    for(int j=c->start; j<=c->end; ++j) {
        bus[j] = NULL;
    }
    c->start = 0;
    c->end = 0;
    return ERR_NONE;
}

int bus_read(const bus_t bus, addr_t address, data_t* data){
    M_REQUIRE_NON_NULL(data);
    if(bus[address] != NULL) { *data = *bus[address]; } else { *data = 0xFF; }
    return ERR_NONE;
}

int bus_read16(const bus_t bus, addr_t address, addr_t* data16) {
    M_REQUIRE_NON_NULL(data16);
    M_REQUIRE(address != BUS_SIZE-1, ERR_BAD_PARAMETER, "Parameter out of bounds: %d\n", address);
    
    data_t* data1 = bus[address];
    data_t* data2 = bus[address+1];
   
    if (data1 != NULL && data2 != NULL ) {*data16 = merge8(*data1, *data2); }
    else { *data16 = 0x00FF; }
    return ERR_NONE;
}

int bus_write(bus_t bus, addr_t address, data_t data) {
    #ifdef NO_ROM_WRITE
    if(address < 0x8000) { return ERR_BAD_PARAMETER; } //For Tetris to work, else ignore (prevents from writing in ROM)
    #endif
    
    M_REQUIRE_NON_NULL(bus[address]);
    *bus[address] = data;
    return ERR_NONE;
}

int bus_write16(bus_t bus, addr_t address, addr_t data16) {
    M_REQUIRE_NON_NULL(bus[address]);
    M_REQUIRE_NON_NULL(bus[address+1]);
    M_REQUIRE(address != BUS_SIZE-1, ERR_BAD_PARAMETER, "Parameter out of bounds\n");
    
    #ifdef NO_ROM_WRITE
    if(address < 0x8000) { return ERR_BAD_PARAMETER; } //For Tetris to work, else ignore (prevents from writing in ROM)
    #endif
    
    data_t data1 = lsb8(data16);
    data_t data2 = msb8(data16);
    *bus[address] = data1;
    *bus[address+1] = data2;
    return ERR_NONE;
}

