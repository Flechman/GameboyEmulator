#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "bit_vector.h"

#include <stdint.h>

//=========================================================================
typedef uint8_t palette_t;

#define PALETTE_COLOR_COUNT 4

#define DEFAULT_PALETTE 0xE4 // 11, 10, 01, 00

//=========================================================================
/**
 * @brief Type to represent image lines
 */
struct image_line_ {
    bit_vector_t* msb;
    bit_vector_t* lsb;
    bit_vector_t* opacity;
};
typedef struct image_line_ image_line_t;

#define IMAGE_LINE_WORD_BITS 32


//=========================================================================
/**
 * @brief Type to represent images
 */
struct image_ {
    size_t height;
    image_line_t* content;
};
typedef struct image_ image_t;

//=========================================================================
/**
 * @brief Create an image
 * @param piml pointer to image line
 * @param size length of line in pixels
 * @return Error code
 */
int image_line_create(image_line_t* piml, size_t size);

//=========================================================================
/**
 * @brief Sets value of a word in the image line
 * @param piml pointer to image line
 * @param index word index to set value
 * @param msb value of msb to use
 * @param lsb value of lsb to use
 * @return Error code
 */
int image_line_set_word(image_line_t* piml, size_t index, uint32_t msb, uint32_t lsb);

//=========================================================================
/**
 * @brief Shift image line
 * @param output pointer to write output to
 * @param iml image line to shift
 * @param shift shift amount
 * @return Error code
 */
int image_line_shift(image_line_t* output, image_line_t iml, int64_t shift);

//=========================================================================
/**
 * @brief Extract image line (wrapping)
 * @param output pointer to write output to
 * @param iml image line to extract
 * @param index index from which to extract
 * @param size size of new vector
 * @return Error code
 */
int image_line_extract_wrap_ext(image_line_t* output, image_line_t iml, int64_t index, size_t size);

//=========================================================================
/**
 * @brief Apply Palette to image line
 * @param output pointer to write output to
 * @param iml image line to use palette on
 * @param map palette to use
 * @return Error code
 */
int image_line_map_colors(image_line_t* output, image_line_t iml, palette_t map);

//=========================================================================
/**
 * @brief Combine two image lines using opacity
 * @param output pointer to write output to
 * @param iml1 image line to combine
 * @param iml2 image line to combine
 * @param p_opacity bit vector pointer to use for opacity
 * @return Error code
 */
int image_line_below_with_opacity(image_line_t* output, image_line_t iml1, image_line_t iml2, bit_vector_t* p_opacity);

//=========================================================================
/**
 * @brief Combine two image lines (using iml2 opacity)
 * @param output pointer to write output to
 * @param iml1 image line to combine
 * @param iml2 image line to combine
 * @return Error code
 */
int image_line_below(image_line_t* output, image_line_t iml1, image_line_t iml2);

//=========================================================================
/**
 * @brief Join two image lines
 * @param output pointer to write output to
 * @param iml1 image line to join (values from 0 to start)
 * @param iml2 image line to join (values from start to end)
 * @param start index from which to use iml2 values
 * @return Error code
 */
int image_line_join(image_line_t* output, image_line_t iml1, image_line_t iml2, int64_t start);

//=========================================================================
/**
 * @brief Free image line
 * @param piml pointer to image line
 */
void image_line_free(image_line_t* piml);

//=========================================================================
/**
 * @brief Creates an image of given width and height
 * @param pim pointer to image
 * @param width image width
 * @param height image height
 * @return Error code
 */
int image_create(image_t* pim, size_t width, size_t height);

//=========================================================================
/**
 * @brief Set line content of image (copying values)
 * @param pim pointer to image
 * @param y line index
 * @param line new line value to use
 * @return Error code
 */
int image_set_line(image_t* pim, size_t y, image_line_t line);

//=========================================================================
/**
 * @brief Get pixel value from image
 * @param output pointer to write pixel value to
 * @param pim pointer to image
 * @param x row index of pixel
 * @param y line index of pixel
 * @return Error code
 */
int image_get_pixel(uint8_t* output, image_t* pim, size_t x, size_t y);

//=========================================================================
/**
 * @brief Set line content of image (using provided bit vectors pointers)
 * @param pim pointer to image
 * @param y line index to set
 * @param line line to use bit vectors from
 * @return Error code
 */
int image_own_line_content(image_t* pim, size_t y, image_line_t line);

//=========================================================================
/**
 * @brief Free image
 * @param pim pointer to image
 */
void image_free(image_t* pim);


#ifdef __cplusplus
}
#endif
