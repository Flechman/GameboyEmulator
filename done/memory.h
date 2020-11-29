#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief types for Game Boy data (8 bits) and addresses (16 bits)
 */
typedef uint16_t addr_t;
typedef uint8_t data_t;

/**
 * @brief Structure for memory,
 *        Holds a pointer to the memory, its size and if it is writable
 */
typedef struct {
    data_t* memory;
    size_t size;
} memory_t;


/**
 * @brief Creates memory structure
 *
 * @param mem memory structure pointer to initialize
 * @param size size of the memory to create
 * @return error code
 */
int mem_create(memory_t* mem, size_t size);

/**
 * @brief Destroys memory structure
 *
 * @param mem memory structure pointer to destroy
 */
void mem_free(memory_t* mem);

#ifdef __cplusplus
}
#endif
