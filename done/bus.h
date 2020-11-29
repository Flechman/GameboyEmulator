#pragma once


#include <stdint.h>
#include <stdbool.h>

#include "memory.h"     // addr_t and data_t
#include "component.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BUS_SIZE 65536

/**
 * @ brief Bus Type, a table of memory pointer pointing to the various component memories
 */
typedef data_t* bus_t[BUS_SIZE];


/**
 * @brief Plug a component into the bus
 *
 * @param bus bus to plug into
 * @param c component to plug into bus
 * @param start address from where to plug (included)
 * @param end address until where to plug (included)
 * @return error code
 */
int bus_plug(bus_t bus, component_t* c, addr_t start, addr_t end);


/**
 * @brief Plug forcibly a component into the bus (allows to use outside of range start and end).
 *        For example, if we want to map a component to somewhere else than the initialy described area.
 *
 * @param bus bus to plug into
 * @param c component to plug into bus
 * @param start address from where to plug to (included)
 * @param end address until where to plug to (included)
 * @param offset offset where to start in the component
 * @return error code
 */
int bus_forced_plug(bus_t bus, component_t* c, addr_t start, addr_t end, addr_t offset);


/**
 * @brief Remap the memory of a component to the bus
 *
 * @param bus bus to remap to
 * @param c component to remap
 * @param offset new offset to use
 * @return error code
 */
int bus_remap(bus_t bus, component_t* c, addr_t offset);


/**
 * @brief Unplug a component from the bus
 *
 * @param bus bus to unplug from
 * @param c component to plug into bus
 * @return error code
 */
int bus_unplug(bus_t bus, component_t* c);


/**
 * @brief Read the bus at a given address
 *
 * @param bus bus to read from
 * @param address address to read at
 * @param data pointer to write read data to
 * @return error code
 */
int bus_read(const bus_t bus, addr_t address, data_t* data);


/**
 * @brief Write to the bus at a given address
 *
 * @param bus bus to write to
 * @param address address to write at
 * @param data data to write
 * @return error code
 */
int bus_write(bus_t bus, addr_t address, data_t data);

/**
 * @brief Read the bus at a given address (reads 16 bits)
 *
 * @param bus bus to read from
 * @param address address to read at
 * @param data16 pointer to write read data to
 * @return error code
 */
int bus_read16(const bus_t bus, addr_t address, addr_t* data16);

/**
 * @brief Write to the bus at a given address (writes 16 bits)
 *
 * @param bus bus to write to
 * @param address address to write at
 * @param data16 data to write
 * @return error code
 */
int bus_write16(bus_t bus, addr_t address, addr_t data16);

#ifdef __cplusplus
}
#endif
