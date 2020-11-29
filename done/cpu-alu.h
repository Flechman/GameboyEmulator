#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "opcode.h"
#include "cpu.h"

// ======================================================================
/**
 * @brief Enum for the various flag values
 */
typedef enum {
    CLEAR, // force à 0
    SET, // force à 1
    ALU, // depuis les fanions de l'ALU
    CPU, // depuis les fanions du CPU (c.-à-d. inchangé)
} flag_src_t;


// ======================================================================
/**
* @brief Macros for flag sources for ALU operations (ZNHC)
*/
#define ADD_FLAGS_SRC     ALU,   CLEAR, ALU,   ALU
#define INC_FLAGS_SRC     ALU,   CLEAR, ALU,   CPU
#define SUB_FLAGS_SRC     ALU,   SET,   ALU,   ALU
#define DEC_FLAGS_SRC     ALU,   SET,   ALU,   CPU
#define AND_FLAGS_SRC     ALU,   CLEAR, SET,   CLEAR
#define OR_FLAGS_SRC      ALU,   CLEAR, CLEAR, CLEAR
#define DAA_FLAGS_SRC     ALU,   CPU,   CLEAR, ALU
#define ROT_FLAGS_SRC     CLEAR, CLEAR, CLEAR, ALU
#define SHIFT_FLAGS_SRC   ALU,   CLEAR, CLEAR, ALU
#define ALL_ALU_FLAGS_SRC ALU,   ALU,   ALU,   ALU


// ======================================================================
/**
* @brief some useful macros:
*
*        + combine_flags_set_A:
*             used in do_cpu_arithm to combine ALU flags, set F and set A
*
*        + extract_carry:
*             used in do_cpu_arithm to to extract carry bit from opcode
*
*        + do_cpu_arithm:
*             does(=applies) operation OP from ALU on CPU using ARG as
*             operation argument and FLAGS_SRC flag sources
*             (see for instance ADD_FLAGS_SRC macro above:
*              this are the flag sources to be used for ADD operations.)
*/

#define combine_flags_set_A(cpu, ...) \
    do { \
        M_EXIT_IF_ERR(cpu_combine_alu_flags(cpu, __VA_ARGS__)); \
        cpu->A = lsb8(cpu->alu.value); \
    } while(0)

#define OPCODE_CARRY_IDX 3
#define extract_carry(cpu, op) \
    (bit_get(op, OPCODE_CARRY_IDX) && get_C((cpu)->F))

#define do_cpu_arithm(cpu, op, arg, flags_src)  \
    do { \
        M_EXIT_IF_ERR(op(&cpu->alu, cpu->A, (arg), extract_carry(cpu, lu->opcode))); \
        combine_flags_set_A(cpu, flags_src); \
    } while(0)


// ======================================================================
/**
* @brief Executes an ALU instruction
* @param lu instruction
* @param cpu, the CPU which shall execute
* @return error code
*/
int cpu_dispatch_alu(const instruction_t* lu, cpu_t* cpu);

/**
 * @brief Combine flag sources and write them to F register
 *
 * @param cpu cpu source and target to use
 * @param Z flag source for Z flag bit
 * @param N flat source for N flag bit
 * @param H flag source for H flag bit
 * @param C flag source for C flag bit
 *
 * @return Error code
 */
int cpu_combine_alu_flags(cpu_t* cpu,
                          flag_src_t Z, flag_src_t N, flag_src_t H, flag_src_t C);

#ifdef __cplusplus
}
#endif
