#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "memory.h"
#include "opcode.h"
#include "cpu.h"

/**
 * @brief Reads data from the bus at a given adress
 *
 * @param cpu cpu to read from
 * @param addr address to read at
 *
 * @return data read
 */
data_t cpu_read_at_idx(const cpu_t* cpu, addr_t addr);

/**
 * @brief Reads data at HL address from bus
 */
#define cpu_read_at_HL(cpu) \
    cpu_read_at_idx(cpu, cpu_HL_get(cpu))

/**
 * @brief Reads data after opcode from bus
 */
#define cpu_read_data_after_opcode(cpu)\
    cpu_read_at_idx(cpu,(addr_t)((cpu)->PC + 1))

/**
 * @brief Reads 16bit data from the bus at a given adress
 *
 * @param cpu cpu to read from
 * @param addr address to read at
 *
 * @return data16 read
 */
addr_t cpu_read16_at_idx(const cpu_t* cpu, addr_t addr);

/**
 * @brief Reads 16bit data after opcode from bus
 */
#define cpu_read_addr_after_opcode(cpu) \
    FROM_GameBoy_16(cpu_read16_at_idx(cpu, (addr_t)((cpu)->PC + 1)))

/**
 * @brief Write data to the bus at a given adress
 *
 * @param cpu cpu to write to
 * @param addr address to write at
 * @param data data to write
 *
 * @return error code
 */
int cpu_write_at_idx(cpu_t* cpu, addr_t addr, data_t data);

#define cpu_write_at_HL(cpu, data) \
    cpu_write_at_idx(cpu, cpu_HL_get(cpu), data)

/**
 * @brief Write 16bit data to the bus at a given adress
 *
 * @param cpu cpu to write to
 * @param addr address to write at
 * @param data16 16bit data to write
 *
 * @return error code
 */
int cpu_write16_at_idx(cpu_t* cpu, addr_t addr, addr_t data16);

/**
 * @brief Executes a cpu storage instruction
 * @param lu instruction
 * @param cpu, the CPU which shall execute
 * @return error code
 *
 * See opcode.h and cpu.h
 */
int cpu_dispatch_storage(const instruction_t* lu, cpu_t* cpu);


/**
 * @brief Push 16bit data to the stack
 *
 * @param cpu cpu to use
 * @param data16 16bit data to push to stack
 *
 * @return error code
 */
int cpu_SP_push(cpu_t* cpu, addr_t data16);


/**
 * @brief Pop 16bit data from the stack
 *
 * @param cpu cpu to use
 *
 * @return 16bit poped data
 */
addr_t cpu_SP_pop(cpu_t* cpu);


#ifdef __cplusplus
}
#endif
