#include <stdlib.h>
#include <stdio.h>

#include "error.h"
#include "memory.h"
#include "component.h"


int component_create(component_t* c, size_t mem_size) {
    M_REQUIRE_NON_NULL(c);
    //Component not yet connected to bus (that's why start and end are at 0 initially)
    c->start = 0;
    c->end = 0;
    if(mem_size > 0) {
        c->mem = calloc(1, sizeof(memory_t));
        M_REQUIRE_NON_NULL_CUSTOM_ERR(c->mem, ERR_MEM);
        M_REQUIRE_NO_ERR(mem_create(c->mem, mem_size));
    } else {
        c->mem = NULL;
    }
    return ERR_NONE;
}

int component_shared(component_t* c, component_t* c_old) {
    M_REQUIRE_NON_NULL(c);
    M_REQUIRE_NON_NULL(c_old);
    M_REQUIRE_NON_NULL(c_old->mem);
    c->start = 0;
    c->end = 0;
    c->mem = c_old->mem;
    return ERR_NONE;
}

void component_free(component_t* c) {
    if(c != NULL) {
        if(c->mem != NULL) {
            mem_free(c->mem);
            free(c->mem);
            c->mem = NULL;
        }
        c->start = 0;
        c->end = 0;
    }
}
