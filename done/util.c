

#include <stdint.h> // for uint16_t

// see util.h
uint16_t b2l_16(const uint16_t input)
{
    const unsigned char * const bytes = (const unsigned char *) &input;
    return ((uint16_t) bytes[1] << 8) | (uint16_t) bytes[0];
}

// see util.h
uint16_t l2b_16(const uint16_t input)
{
    const unsigned char * const bytes = (const unsigned char *) &input;
    return ((uint16_t) bytes[0] << 8) | (uint16_t) bytes[1];
}
