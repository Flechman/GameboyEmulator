#pragma once

#include "alu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief adjusts the value of a previous alu_output using various flags to the bcd format
 *
 * @param result alu_output_t pointer use the value and flags from and to write into
 * @return error code
 */
int alu_bcd_adjust(alu_output_t* result);


/**
 * @brief logic AND of two inputs
 *
 * @param result alu_output_t pointer to write into
 * @param x value to and
 * @param y value to and
 * @return error code
 */
int alu_and(alu_output_t* result, uint8_t x, uint8_t y);


/**
 * @brief logic OR of two inputs
 *
 * @param result alu_output_t pointer to write into
 * @param x value to or
 * @param y value to or
 * @return error code
 */
int alu_or(alu_output_t* result, uint8_t x, uint8_t y);


/**
 * @brief logic XOR of two inputs
 *
 * @param result alu_output_t pointer to write into
 * @param x value to xor
 * @param y value to xor
 * @return error code
 */
int alu_xor(alu_output_t* result, uint8_t x, uint8_t y);


/**
 * @brief swaps 4lsb with 4msb
 *
 * @param result alu_output_t pointer to write into
 * @param x value to swap the bits from
 * @return error code
 */
int alu_swap4(alu_output_t* result, uint8_t x);


#ifdef __cplusplus
}
#endif
