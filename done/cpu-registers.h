#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu.h"     // cpu_t
#include <stdint.h> // uint8_t

// ======================================================================
/**
 * @brief Register codes and register pair codes in OpCodes
 *
 */
typedef enum {
    REG_B_CODE = 0x00,
    REG_C_CODE = 0x01,
    REG_D_CODE = 0x02,
    REG_E_CODE = 0x03,
    REG_H_CODE = 0x04,
    REG_L_CODE = 0x05,
    REG_A_CODE = 0x07
} reg_kind;

typedef enum {
    REG_BC_CODE = 0x00,
    REG_DE_CODE = 0x01,
    REG_HL_CODE = 0x02,
    REG_AF_CODE = 0x03
} reg_pair_kind;

// ======================================================================
/**
 * @brief returns a register given the register value
 *
 * @params cpu pointer to the cpu
 * @param reg register type
 *
 * @return value of the desired register
 */
uint8_t cpu_reg_get(const cpu_t* cpu, reg_kind reg);

#define cpu_AF_get(cpu) \
    cpu_reg_pair_get(cpu, REG_AF_CODE)

#define cpu_BC_get(cpu) \
    cpu_reg_pair_get(cpu, REG_BC_CODE)

#define cpu_DE_get(cpu) \
    cpu_reg_pair_get(cpu, REG_DE_CODE)

#define cpu_HL_get(cpu) \
    cpu_reg_pair_get(cpu, REG_HL_CODE)


/**
 * @brief writes to a register given the register value
 *
 * @params cpu pointer to the cpu
 * @param reg register type
 * @param value value to write to desired register
 */
void cpu_reg_set(cpu_t* cpu, reg_kind reg, uint8_t value);

#define cpu_AF_set(cpu, value) \
    cpu_reg_pair_set(cpu, REG_AF_CODE, value)

#define cpu_BC_set(cpu, value) \
    cpu_reg_pair_set(cpu, REG_BC_CODE, value)

#define cpu_DE_set(cpu, value) \
    cpu_reg_pair_set(cpu, REG_DE_CODE, value)

#define cpu_HL_set(cpu, value) \
    cpu_reg_pair_set(cpu, REG_HL_CODE, value)

/**
 * @brief writes to a register the 8 LSB from ALU
 *
 * @params cpu pointer to the cpu
 * @param reg register type
 */
#define cpu_reg_set_from_alu8(cpu, reg) \
    cpu_reg_set(cpu, reg, lsb8((cpu)->alu.value))

/**
 * @brief returns a register given the register pair value
 *
 * @params cpu pointer to the cpu
 * @param reg register pair type
 *
 * @return value of the desired register pair
 */
uint16_t cpu_reg_pair_get(const cpu_t* cpu, reg_pair_kind reg);

#define cpu_reg_pair_SP_get(cpu, reg) \
  (reg == REG_AF_CODE ? ((cpu)->SP) : cpu_reg_pair_get(cpu, reg))


/**
 * @brief writes to a register given the register pair value
 *
 * @params cpu pointer to the cpu
 * @param reg register pair type
 * @param value value to write to desired register pair
 */
void cpu_reg_pair_set(cpu_t* cpu, reg_pair_kind reg, uint16_t value);

#define cpu_reg_pair_SP_set(cpu, reg, value) \
  (reg == REG_AF_CODE ? (void)((cpu)->SP = value) : cpu_reg_pair_set(cpu,reg,value))


#ifdef __cplusplus
}
#endif
