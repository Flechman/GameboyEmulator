#pragma once


#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t bit_t;

/**
 * @brief number of bits in an uint8_t
 */
#define UINT8_SIZE 8

/**
 * @brief clamp a value to be a bit index between 0 and 7
 */
#define CLAMP07(x) (((x) < 0) || ((x) > 7) ? 0 : (x))

/**
 * @brief Enumeration type of rotation directions: either LEFT or RIGHT
 */
typedef enum {
    LEFT,
    RIGHT
} rot_dir_t;

/**
 * @brief returns the 4 LSB of a uint8_t
 *
 * @param value value to get the 4 LSB from
 * @return 4 LSB of the input
 */
uint8_t lsb4(uint8_t value);


/**
 * @brief returns the 4 MSB of a uint8_t
 *
 * @param value value to get the 4 MSB from
 * @return 4 MSB of the input
 */
uint8_t msb4(uint8_t value);


/**
 * @brief Rotates the bits in a uint8_t
 *
 * @param value pointer to the number to rotate
 * @param dir rotation direction
 * @param d number of rotation steps to execute
 */
void bit_rotate(uint8_t* value, rot_dir_t dir, int d);


/**
 * @brief returns the 8 LSB of a uint16_t
 *
 * @param value value to get the 8 LSB from
 * @return 8 LSB of the input
 */
uint8_t lsb8(uint16_t value);


/**
 * @brief returns the 8 MSB of a bit16_t (uint16_t)
 *
 * @param value value to get the 8 MSB from
 * @return 8 MSB of the input
 */
uint8_t msb8(uint16_t value);


/**
 * @brief Merges two 8bit into 16bits
 *
 * @param v1 value for the 8 LSB
 * @param v2 value for the 8 MSB
 * @return result of merging v1 and v2 into 16bits
 */
uint16_t merge8(uint8_t v1, uint8_t v2);


/**
 * @brief Merges two 4bit into 8bits
 *
 * @param v1 value for the 4 LSB
 * @param v2 value for the 4 MSB
 * @return result of merging v1 and v2 into 8bits
 */
uint8_t merge4(uint8_t v1, uint8_t v2);

/**
* @brief Extends the signed value from 8 to 16 bits
*
* @param v 8 bit signed value to extend
* @return signed value using 16 bits
*/
#define extend_s_16(v) ((int16_t) ((int8_t)v))


/**
* @brief Extends an unsigned value from 8 to 16 bits
*/
#define extend_u_16(v) merge8(0,v)


/**
 * @brief Gets the bit at a given index
 *
 * @param value value to get the bit from
 * @param index index of the bit
 * @return returns the bit at a given index
 */
bit_t bit_get(uint8_t value, int index);


/**
 * @brief Set the bit at a given index to 1
 *
 * @param value value to change the bit in
 * @param index index of the bit
 */
void bit_set(uint8_t* value, int index);


/**
 * @brief Set the bit at a given index to 0
 *
 * @param value value to change the bit in
 * @param index index of the bit
 */
void bit_unset(uint8_t* value, int index);


/**
 * @brief Set/Unset the bit at a given index
 *
 * @param value value to change the bit in
 * @param index index of the bit
 * @param v allows to choose between set and unset
 */
void bit_edit(uint8_t* value, int index, uint8_t v);


#ifdef __cplusplus
}
#endif
