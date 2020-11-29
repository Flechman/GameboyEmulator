
#include <stdlib.h>

#include "error.h"
#include "memory.h"

int mem_create(memory_t* mem, size_t size) {
    M_REQUIRE_NON_NULL(mem);
    M_REQUIRE(size > 0, ERR_BAD_PARAMETER, "Size cannot be 0\n");
    memory_t memry = { NULL, 0 };
    memry.memory = calloc(size, sizeof(data_t));
    M_REQUIRE_NON_NULL_CUSTOM_ERR(memry.memory, ERR_MEM);
    memry.size = size;
    *mem = memry;
    return ERR_NONE;
}

void mem_free(memory_t* mem) {
    if(mem != NULL) {
        if(mem->memory != NULL) {
        free(mem->memory);
        mem->memory = NULL;
        }
        mem->size = 0;
    }
}



