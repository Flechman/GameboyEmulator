#pragma once

#include <inttypes.h> // for uint16_t

/**
 * @brief tag a variable as POTENTIALLY unused, to avoid compiler warnings
 */
#define _unused __attribute__((unused))

/**
 * @brief useful to free pointers to const without warning. Use with care!
 */
#define free_const_ptr(X) free((void*)X)

/**
 * @brief useful to init a variable (typically a struct) directly or through a pointer
 */
#define zero_init_var(X) memset(&X, 0, sizeof(X))
#define zero_init_ptr(X) memset(X, 0, sizeof(*X))

/**
 * @brief useful to have C99 (!) %zu to compile in Windows
 */
#if defined _WIN32  || defined _WIN64
#define SIZE_T_FMT "%u"
#else
#define SIZE_T_FMT "%zu"
#endif

/**
 * @brief convert big-endian 16 bits integer to little-endian.
 *    If already little-endian, return same value.
 */
uint16_t b2l_16(const uint16_t);

/**
 * @brief convert little-endian 16 bits integer to big-endian.
 *    If already big-endian, return same value.
 */
uint16_t l2b_16(const uint16_t);

/**
 * @brief macros to ensure little-endian 16-bits representation
 */
#ifdef __ORDER_LITTLE_ENDIAN__
#  if __ORDER_LITTLE_ENDIAN__ == 1234
#    define FROM_GameBoy_16(X) (X)
#    define TO_GameBoy_16(X)   (X)
#  else
#    define FROM_GameBoy_16(X) l2b_16(X)
#    define TO_GameBoy_16(X)   b2l_16(X)
#  endif
#else
#  error Cannot determine endianess: the compiler does not define __ORDER_LITTLE_ENDIAN__
#endif
