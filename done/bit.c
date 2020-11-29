#include <stdio.h>
#include <stdint.h>   // for uint8_t and uint16_t types
#include <inttypes.h> // for PRIx8, etc.
#include "bit.h"
#include "error.h"
/**
 * @brief a type to represent 1 single bit.
 */
typedef uint8_t bit_t;

uint8_t lsb4(uint8_t value) {
	return value & 0x0f;
}

uint8_t msb4(uint8_t value) {
	return (value >> 4) & 0x0f;
}

uint8_t lsb8(uint16_t value) {
	return value & 0x00ff;
}

uint8_t msb8(uint16_t value) {
	return (value >> 8) & 0x00ff;
}

uint16_t merge8(uint8_t v1, uint8_t v2) {
	return (0x00ff & v1) | ((0x00ff & v2) << 8);
}

uint8_t merge4(uint8_t v1, uint8_t v2) {
	return (v2 << 4) | (v1 & 0x0f);
}

bit_t bit_get(uint8_t value, int index) {
	return (value >> CLAMP07(index)) & 0x01;
}

void bit_set(uint8_t* value, int index) {
    if(value != NULL) {
	    *value = *value | (0x01 << CLAMP07(index));
    }
}

void bit_unset(uint8_t* value, int index) {
    if(value != NULL) {
	    *value = *value & ~(0x01 << CLAMP07(index));
    }
}

void bit_rotate(uint8_t* value, rot_dir_t dir, int d) {
    if(dir==LEFT && value != NULL) {
        uint8_t dLast = *value >> (UINT8_SIZE - CLAMP07(d));
        uint8_t newValue = *value << CLAMP07(d);
        *value = dLast | newValue;
    } 
    else if(dir==RIGHT && value != NULL) {
        uint8_t dLast = *value >> CLAMP07(d);
        uint8_t newValue = *value << (UINT8_SIZE - CLAMP07(d));
        *value = dLast | newValue;
    }
}

void bit_edit(uint8_t* value, int index, uint8_t v) {
    if(value != NULL) {
        if(v==0) {
            bit_unset(value, index);
        } else {
            bit_set(value, index);
        }
    }
}
