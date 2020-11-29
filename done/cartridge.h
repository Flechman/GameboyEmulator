#pragma once

#include <stdint.h>

#include "component.h"
#include "bus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BANK_ROM0_START  0x0000
#define BANK_ROM0_END    0x3FFF
#define BANK_ROM0_SIZE   ((BANK_ROM0_END - BANK_ROM0_START) + 1)

#define BANK_ROM1_START  0x4000
#define BANK_ROM1_END    0x7FFF
#define BANK_ROM1_SIZE   ((BANK_ROM1_END - BANK_ROM1_START) + 1)

#define BANK_ROM_SIZE    (BANK_ROM0_SIZE + BANK_ROM1_SIZE)

#define CARTRIDGE_GAME_TITLE_START 0x0134
#define CARTRIDGE_GAME_TITLE_END   0x0143
#define CARTRIDGE_TYPE_ADDR        0x0147

/**
 * @brief Cartridge type
 */
typedef struct {
    component_t c;
} cartridge_t;

/**
 * @brief Reads a file into the memory of a component
 *
 * @param c component to write to
 * @param filename file to read from
 * @return error code
 */
int cartridge_init_from_file(component_t* c, const char* filename);


/**
 * @brief Initiates a cartridge given a filename
 *
 * @param ct cartridge to initiate
 * @param filename file to read from
 * @return error code
 */
int cartridge_init(cartridge_t* ct, const char* filename);


/**
 * @brief Plugs a cartridge to the bus
 *
 * @param ct cartridge to plug
 * @param bus bus to plug into
 * @return error code
 */
int cartridge_plug(cartridge_t* ct, bus_t bus);


/**
 * @brief Frees a cartridge
 *
 * @param ct cartridge to free
 */
void cartridge_free(cartridge_t* ct);

#ifdef __cplusplus
}
#endif
