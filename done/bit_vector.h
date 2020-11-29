#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "bit.h"

#include <stddef.h> // for size_t
#include <stdint.h>

//=========================================================================
/**
 * @brief Type to represent image lines
 */
typedef struct {
    size_t size; //In bits
    uint32_t content[];
} bit_vector_t;

//=========================================================================
/**
 * @brief Create a bit vector of a given size and fill it with bit value
 * @param size, size in bits of the vector
 * @param value, bit value
 * @return pointer to created bit vector
 */
bit_vector_t* bit_vector_create(size_t size, bit_t value);

//=========================================================================
/**
 * @brief Create a copy of a bit vector
 * @param pbv pointer to the bit vector to copy
 * @return pointer to the copy of given bit vector
 */
bit_vector_t* bit_vector_cpy(const bit_vector_t* pbv);

//=========================================================================
/**
 * @brief Get the value of a given bit in a bit vector
 * @param pbv pointer to the bit vector
 * @param index index of bit
 * @return value of bit at index
 */
bit_t bit_vector_get(const bit_vector_t* pbv, size_t index);

//=========================================================================
/**
 * @brief Compute logical NOT of a bit vector
 * @param pbv pointer to the bit vector
 * @return pointer to the bit vector
 */
bit_vector_t* bit_vector_not(bit_vector_t* pbv);

//=========================================================================
/**
 * @brief Compute logical AND of two bit vectors
 * @param pbv1 pointer to first bit vector
 * @param pbv2 pointer to second bit vector
 * @return pointer to the first bit vector
 */
bit_vector_t* bit_vector_and(bit_vector_t* pbv1, const bit_vector_t* pbv2);

//=========================================================================
/**
 * @brief Compute logical OR of two bit vectors
 * @param pbv1 pointer to first bit vector
 * @param pbv2 pointer to second bit vector
 * @return pointer to the first bit vector
 */
bit_vector_t* bit_vector_or(bit_vector_t* pbv1, const bit_vector_t* pbv2);

//=========================================================================
/**
 * @brief Compute logical XOR of two bit vectors
 * @param pbv1 pointer to first bit vector
 * @param pbv2 pointer to second bit vector
 * @return pointer to the first bit vector
 */
bit_vector_t* bit_vector_xor(bit_vector_t* pbv1, const bit_vector_t* pbv2);

//=========================================================================
/**
 * @brief Create a new bit vector extracted from another bit vector (zero extended)
 * @param pbv pointer to bit vector
 * @param index index from where to start extraction
 * @param size size in bit of new bit vector
 * @return pointer new bit vector
 */
bit_vector_t* bit_vector_extract_zero_ext(const bit_vector_t* pbv, int64_t index, size_t size);

//=========================================================================
/**
 * @brief Create a new bit vector extracted from another bit vector (wrap extended)
 * @param pbv pointer to bit vector
 * @param index index from where to start extraction
 * @param size size in bit of new bit vector
 * @return pointer new bit vector
 */
bit_vector_t* bit_vector_extract_wrap_ext(const bit_vector_t* pbv, int64_t index, size_t size);

//=========================================================================
/**
 * @brief Create a new bit vector shifted from another bit vector
 * @param pbv pointer to bit vector
 * @param shift bit shift count
 * @return pointer new bit vector
 */
bit_vector_t* bit_vector_shift(const bit_vector_t* pbv, int64_t shift);

//=========================================================================
/**
 * @brief Join two bit vectors into a new bit vector
 * @param pbv1 pointer to first bit vector
 * @param pbv2 pointer to second bit vector
 * @param shift bit shift count
 * @return pointer new bit vector containing pbv1 values until shift (excluded) followed by values from pbv2
 */
bit_vector_t* bit_vector_join(const bit_vector_t* pbv1, const bit_vector_t* pbv2, int64_t shift);

//=========================================================================
/**
 * @brief Print bit vector values
 * @param pbv pointer to bit vector
 * @return count of printed characters
 */
int bit_vector_print(const bit_vector_t* pbv);

//=========================================================================
/**
 * @brief Print a bit vector as a line with a prefix
 * @param prefix string to use as prefix
 * @param pbv pointer to bit vector
 * @return count of printed characters
 */
int bit_vector_println(const char* prefix, const bit_vector_t* pbv);

//=========================================================================
/**
 * @brief Frees a bit vector
 * @param pbv pointer to bit vector pointer
 */
void bit_vector_free(bit_vector_t** pbv);

#ifdef __cplusplus
}
#endif
