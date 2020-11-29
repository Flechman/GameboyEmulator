#pragma once


#include <stdint.h>
#include <stdlib.h>

#include "bit.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief type flags_t , a uint8_t whose bits hold the value of various flags
 */
typedef uint8_t flags_t;

/**
 * @brief Enum for the various flag values
 */
typedef enum {
    FLAG_Z = 0x80, // Zero
    FLAG_N = 0x40, // Substraction overflow/underflow
    FLAG_H = 0x20, // Carry 4bit
    FLAG_C = 0x10, // Carry 8bit
} flag_bit_t;


/**
 * @brief type alu_output_t holds a structure with a 16bit value field (usualy using only the 8 lsb),
 *        and a flags_t flag field holding the various flags
 */
typedef struct {
    uint16_t value;
    flags_t flags;
} alu_output_t;



/**
 * @brief get flag value
 *
 * @param flags flag set to get flag from
 * @param flag flag value to get
 * @return flag value
 */
flag_bit_t get_flag(flags_t flags, flag_bit_t flag);

#define get_C(X) get_flag(X, FLAG_C)
#define get_H(X) get_flag(X, FLAG_H)
#define get_N(X) get_flag(X, FLAG_N)
#define get_Z(X) get_flag(X, FLAG_Z)


/**
 * @brief set flag
 *
 * @param flags (modified) set of flags
 * @param flag flag to be set
 */
void set_flag(flags_t* flags, flag_bit_t flag);

#define set_C(X) set_flag(X, FLAG_C)
#define set_H(X) set_flag(X, FLAG_H)
#define set_N(X) set_flag(X, FLAG_N)
#define set_Z(X) set_flag(X, FLAG_Z)

/**
 * @brief unset flag
 * 
 * @param flags (modified) set of flags
 * @param flag flag to be unset
 */
void unset_flag(flags_t* flags, flag_bit_t flag);

#define unset_C(X) unset_flag(X, FLAG_C)
#define unset_H(X) unset_flag(X, FLAG_H)
#define unset_N(X) unset_flag(X, FLAG_N)
#define unset_Z(X) unset_flag(X, FLAG_Z)

/**
 * @brief unset all flags of 'result'
 * 
 * @param result flags of result to be unset
 */
void unset_all(alu_output_t* result);

/**
 * @brief adds two uint8 and writes the results and flags into an alu_output_t structure
 *
 * @param result alu_output_t pointer to write into
 * @param x value to sum
 * @param y value to sum
 * @param c0 carry in
 * @return error code
 */
int alu_add8(alu_output_t* result, uint8_t x, uint8_t y, bit_t c0);


/**
 * @brief subtract two uint8 and writes the results and flags into an alu_output_t structure
 *
 * @param result alu_output_t pointer to write into
 * @param x value to subtract from
 * @param y value to subtract
 * @param b0 initial borrow bit
 * @return error code
 */
int alu_sub8(alu_output_t* result, uint8_t x, uint8_t y, bit_t b0);


/**
 * @brief sum two uint16 and writes the results and flags into an alu_output_t structure,
 *        the H & C flags are being placed according to the 8 lsb
 *
 * @param result alu_output_t pointer to write into
 * @param x value to sum
 * @param y value to sum
 * @return error code
 */
int alu_add16_low(alu_output_t* result, uint16_t x, uint16_t y);


/**
 * @brief sum two uint16 and writes the results and flags into an alu_output_t structure,
 *        the H & C flags are being placed according to the 8 msb
 *
 * @param result alu_output_t pointer to write into
 * @param x value to sum
 * @param y value to sum
 * @return error code
 */
int alu_add16_high(alu_output_t* result, uint16_t x, uint16_t y);


/**
 * @brief logic shift
 *
 * @param result alu_output_t pointer to write into
 * @param x value to shift
 * @param dir shift direction
 * @return error code
 */
int alu_shift(alu_output_t* result, uint8_t x, rot_dir_t dir);


/**
 * @brief arithmetic shift
 *
 * @param result alu_output_t pointer to write into
 * @param x value to shift
 * @return error code
 */
int alu_shiftR_A(alu_output_t* result, uint8_t x);


/**
 * @brief logic rotate
 *
 * @param result alu_output_t pointer to write into
 * @param x value to rotate
 * @param dir rotation direction
 * @return error code
 */
int alu_rotate(alu_output_t* result, uint8_t x, rot_dir_t dir);


/**
 * @brief logic rotate with carry taken into account
 *
 * @param result alu_output_t pointer to write into
 * @param x value to rotate
 * @param dir rotation direction
 * @param flags carry flag
 * @return error code
 */
int alu_carry_rotate(alu_output_t* result, uint8_t x, rot_dir_t dir, flags_t flags);

#ifdef __cplusplus
}
#endif
