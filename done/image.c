

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "error.h"
#include "image.h"

// ======================================================================
#define size_to_content_size(size) (((size) / IMAGE_LINE_WORD_BITS) + (((size) % IMAGE_LINE_WORD_BITS==0)?0:1))

#define index_to_content_index(index) ((index)/ IMAGE_LINE_WORD_BITS)

#define do_image_line(piml) do_imlc(piml, lsb); do_imlc(piml, msb); do_imlc(piml, opacity)

// ======================================================================
#define M_REQUIRE_NON_NULL_IMAGE_LINE(iml)\
    do { \
        M_REQUIRE_NON_NULL((iml).lsb); \
        M_REQUIRE_NON_NULL((iml).msb); \
        M_REQUIRE_NON_NULL((iml).opacity); \
    } while(0)

// ======================================================================
#define M_REQUIRE_MATCHING_IMAGE_LINE_SIZE(iml1, iml2)\
    do { \
        M_REQUIRE_NON_NULL_IMAGE_LINE(iml1);\
        M_REQUIRE_NON_NULL_IMAGE_LINE(iml2);\
        M_REQUIRE((iml1).lsb->size == (iml2).lsb->size, ERR_BAD_PARAMETER, "%s", "Sizes do not match"); \
        M_REQUIRE((iml1).msb->size == (iml2).msb->size, ERR_BAD_PARAMETER, "%s", "Sizes do not match"); \
        M_REQUIRE((iml1).opacity->size == (iml2).opacity->size, ERR_BAD_PARAMETER, "%s", "Sizes do not match"); \
    } while(0)

// ======================================================================
static int valid(image_line_t* piml)
{
    if (piml->msb == NULL || piml->lsb == NULL || piml->opacity == NULL) {
        image_line_free(piml);
        return ERR_MEM;
    }

    return ERR_NONE;
}

// ======================================================================
int image_line_create(image_line_t* piml, size_t size)
{
    M_REQUIRE_NON_NULL(piml);
    M_REQUIRE(size > 0, ERR_BAD_PARAMETER, "Invalid Size: %zu is zero", size);

#define do_imlc(I, X) \
    I->X = bit_vector_create(size, 0)

    do_image_line(piml);
#undef do_imlc

    return valid(piml);
}

// ======================================================================
int image_line_set_word(image_line_t* piml, size_t index, uint32_t msb, uint32_t lsb)
{
    M_REQUIRE_NON_NULL(piml);
    M_REQUIRE_NON_NULL_IMAGE_LINE(*piml);
    M_REQUIRE((piml->msb->size == piml->lsb->size) &&
              (piml->lsb->size == piml->opacity->size), ERR_BAD_PARAMETER,
              "Incorrect sizes in image_line (%zu, %zu, %zu)",
              piml->lsb->size, piml->msb->size, piml->opacity->size);
    M_REQUIRE(index < size_to_content_size(piml->msb->size), ERR_BAD_PARAMETER,
              "Incorrect index (%zu >= %zu)", index, size_to_content_size(piml->msb->size));

    piml->msb    ->content[index] = msb;
    piml->lsb    ->content[index] = lsb;
    piml->opacity->content[index] = msb | lsb;

    return ERR_NONE;
}

// ======================================================================
int image_line_shift(image_line_t* output, image_line_t iml, int64_t shift)
{
    M_REQUIRE_NON_NULL(output);
    M_REQUIRE_NON_NULL_IMAGE_LINE(iml);

#define do_imlc(I, X) \
    I->X = bit_vector_shift(iml.X, shift)

    do_image_line(output);
#undef do_imlc

    return valid(output);
}

// ======================================================================
int image_line_extract_wrap_ext(image_line_t* output, image_line_t iml, int64_t index, size_t size)
{
    M_REQUIRE_NON_NULL(output);
    M_REQUIRE_NON_NULL_IMAGE_LINE(iml);
    M_REQUIRE(size > 0, ERR_BAD_PARAMETER, "%s", "Size argument cannot be zero");

#define do_imlc(I, X) \
    I->X = bit_vector_extract_wrap_ext(iml.X, index, size)

    do_image_line(output);
#undef do_imlc

    return valid(output);
}

// ======================================================================
int image_line_map_colors(image_line_t* output, image_line_t iml, palette_t map)
{
    M_REQUIRE_NON_NULL(output);
    M_REQUIRE_NON_NULL_IMAGE_LINE(iml);

    if (map == DEFAULT_PALETTE) {
#define do_imlc(I, X) \
        I->X = bit_vector_cpy(iml.X)

        do_image_line(output);
#undef do_imlc
        return ERR_NONE;
    }

    output->lsb = bit_vector_create(iml.lsb->size, 0);
    output->msb = bit_vector_create(iml.msb->size, 0);
    output->opacity = bit_vector_cpy(iml.opacity);

    for (size_t i = 0; i < PALETTE_COLOR_COUNT; ++i) {
        bit_vector_t* mask = NULL;

        const bit_t color_bit_0 = (bit_t) (map & (1 << (i * 2    )));
        const bit_t color_bit_1 = (bit_t) (map & (1 << (i * 2 + 1)));

        if (color_bit_0 || color_bit_1) {
            switch (i) {
            case 0: {
                bit_vector_t* tmp = bit_vector_not(bit_vector_cpy(iml.lsb));

                if (tmp == NULL) {
                    image_line_free(output);
                    return ERR_MEM;
                }

                mask = bit_vector_and(bit_vector_not(bit_vector_cpy(iml.msb)), tmp);
                bit_vector_free(&tmp);
            } break;

            case 1: {
                mask = bit_vector_and(bit_vector_not(bit_vector_cpy(iml.msb)), iml.lsb);
            } break;

            case 2: {
                mask = bit_vector_and(bit_vector_not(bit_vector_cpy(iml.lsb)), iml.msb);
            } break;

            case 3: {
                mask = bit_vector_and(bit_vector_cpy(iml.lsb), iml.msb);
            } break;
            }

            if (mask == NULL) {
                image_line_free(output);
                return ERR_MEM;
            }

            if (color_bit_0) {
                output->lsb = bit_vector_or(output->lsb, mask);

                if (output->lsb == NULL) {
                    image_line_free(output);
                    return ERR_MEM;
                }
            }

            if (color_bit_1) {
                output->msb = bit_vector_or(output->msb, mask);

                if (output->msb == NULL) {
                    image_line_free(output);
                    return ERR_MEM;
                }
            }

            bit_vector_free(&mask);
        }
    }

    return ERR_NONE;
}

// ======================================================================
int image_line_below_with_opacity(image_line_t* output, image_line_t iml1, image_line_t iml2, bit_vector_t* p_opacity)
{
    M_REQUIRE_NON_NULL(output);
    M_REQUIRE_NON_NULL_IMAGE_LINE(iml1);
    M_REQUIRE_NON_NULL_IMAGE_LINE(iml2);
    M_REQUIRE_MATCHING_IMAGE_LINE_SIZE(iml1, iml2);

    bit_vector_t* notopacity = bit_vector_not(bit_vector_cpy(p_opacity));

    bit_vector_t* m_above_and_opacity    = bit_vector_and(bit_vector_cpy(iml2.msb), p_opacity );
    bit_vector_t* l_above_and_opacity    = bit_vector_and(bit_vector_cpy(iml2.lsb), p_opacity );
    bit_vector_t* m_below_and_notopacity = bit_vector_and(bit_vector_cpy(iml1.msb), notopacity);
    bit_vector_t* l_below_and_notopacity = bit_vector_and(bit_vector_cpy(iml1.lsb), notopacity);

    if (notopacity == NULL ||
        m_above_and_opacity == NULL || m_below_and_notopacity == NULL ||
        l_above_and_opacity == NULL || l_below_and_notopacity == NULL) {
        bit_vector_free(&notopacity);
        bit_vector_free(&m_above_and_opacity);
        bit_vector_free(&m_below_and_notopacity);
        bit_vector_free(&l_above_and_opacity);
        bit_vector_free(&l_below_and_notopacity);
        return ERR_MEM;
    }

    output->msb     = bit_vector_or(m_below_and_notopacity, m_above_and_opacity);
    output->lsb     = bit_vector_or(l_below_and_notopacity, l_above_and_opacity);
    output->opacity = bit_vector_or(bit_vector_cpy(iml1.opacity), p_opacity          );

    bit_vector_free(&notopacity);
    bit_vector_free(&m_above_and_opacity);
    bit_vector_free(&l_above_and_opacity);

    return ERR_NONE;
}

// ======================================================================
int image_line_below(image_line_t* output, image_line_t iml1, image_line_t iml2)
{
    M_REQUIRE_NON_NULL(output);
    M_REQUIRE_NON_NULL_IMAGE_LINE(iml1);
    M_REQUIRE_NON_NULL_IMAGE_LINE(iml2);
    M_REQUIRE_MATCHING_IMAGE_LINE_SIZE(iml1, iml2);

    return image_line_below_with_opacity(output, iml1, iml2, iml2.opacity);
}

// ======================================================================
int image_line_join(image_line_t* output, image_line_t iml1, image_line_t iml2, int64_t start)
{
    M_REQUIRE_NON_NULL(output);
    M_REQUIRE_NON_NULL_IMAGE_LINE(iml1);
    M_REQUIRE_NON_NULL_IMAGE_LINE(iml2);
    M_REQUIRE((iml1.msb->size == iml1.lsb->size) &&
              (iml1.lsb->size == iml1.opacity->size), ERR_BAD_PARAMETER,
              "Incorrect sizes in image_line #1 (%zu, %zu, %zu)",
              iml1.lsb->size, iml1.msb->size, iml1.opacity->size);
    M_REQUIRE_MATCHING_IMAGE_LINE_SIZE(iml1, iml2);
    M_REQUIRE(start >= 0, ERR_BAD_PARAMETER, "Incorrect start (%ld < 0)", start);
    M_REQUIRE(start < (int64_t)iml1.msb->size, ERR_BAD_PARAMETER,
              "Incorrect start (%ld >= %zu)", start, iml1.msb->size);

    const int64_t size = (int64_t)iml1.lsb->size;

    if (start % size == 0) {
#define do_imlc(I, X) \
        I->X = bit_vector_cpy(iml2.X)

        do_image_line(output);
#undef do_imlc
    } else {
#define do_imlc(I, X) \
        I->X = bit_vector_join(iml1.X, iml2.X, start)

        do_image_line(output);
#undef do_imlc
    }

    return valid(output);
}

// ======================================================================
void image_line_free(image_line_t* piml)
{
    if (piml == NULL) return;

    bit_vector_free(&piml->msb);
    bit_vector_free(&piml->lsb);
    bit_vector_free(&piml->opacity);
}

// ======================================================================
int image_create(image_t* pim, size_t width, size_t height)
{
    M_REQUIRE_NON_NULL(pim);
    M_REQUIRE(width > 0, ERR_BAD_PARAMETER, "%s", "Parameter width is zero.");
    M_REQUIRE(height > 0, ERR_BAD_PARAMETER, "%s", "Parameter height is zero.");

    pim->content = calloc(height, sizeof(image_line_t));
    if (pim->content == NULL) return ERR_MEM;

    pim->height = height;

    int error = ERR_NONE;
    for (size_t i = 0; i < height && error == ERR_NONE; ++i) {
        error = image_line_create(pim->content + i, width);
    }

    if (error != ERR_NONE) {
        image_free(pim);
        return error;
    }

    return ERR_NONE;
}

// ======================================================================
int image_set_line(image_t* pim, size_t y, image_line_t line)
{
    M_REQUIRE_NON_NULL(pim);
    M_REQUIRE(y < pim->height, ERR_BAD_PARAMETER, "Invalid Y parameter (%zu < %zu)", y, pim->height);
    M_REQUIRE_NON_NULL_IMAGE_LINE(pim->content[y]);
    M_REQUIRE_MATCHING_IMAGE_LINE_SIZE(pim->content[y], line);

#define do_imlc(I, X) \
    memcpy(I->content[y].X->content, line.X->content, size_to_content_size(I->content[y].X->size) * sizeof(uint32_t))

    do_image_line(pim);
#undef do_imlc

    return ERR_NONE;
}

// ======================================================================
int image_get_pixel(uint8_t* output, image_t* pim, size_t x, size_t y)
{
    M_REQUIRE_NON_NULL(output);
    M_REQUIRE_NON_NULL(pim);
    M_REQUIRE(y < pim->height, ERR_BAD_PARAMETER, "Invalid Y parameter (%zu >= %zu)", y, pim->height);
    M_REQUIRE(x < pim->content[y].msb->size, ERR_BAD_PARAMETER, "Invalid X parameter (%zu >= %zu)", x, pim->content[y].msb->size);
    M_REQUIRE(x < pim->content[y].lsb->size, ERR_BAD_PARAMETER, "Invalid X parameter (%zu >= %zu)", x, pim->content[y].lsb->size);

    *output = (uint8_t)((bit_vector_get(pim->content[y].msb, x) ? 0x2 : 0x0) |
                        (bit_vector_get(pim->content[y].lsb, x) ? 0x1 : 0x0));

    return ERR_NONE;
}

// ======================================================================
int image_own_line_content(image_t* pim, size_t y, image_line_t line)
{
    M_REQUIRE_NON_NULL(pim);
    M_REQUIRE(y < pim->height, ERR_BAD_PARAMETER, "Invalid Y parameter (%zu < %zu)", y, pim->height);
    M_REQUIRE_NON_NULL_IMAGE_LINE(pim->content[y]);
    M_REQUIRE_NON_NULL_IMAGE_LINE(line);
    M_REQUIRE_MATCHING_IMAGE_LINE_SIZE(pim->content[y], line);

#define do_imlc(I, X) \
    bit_vector_free(&(I->content[y].X)); \
    I->content[y].X = line.X

    do_image_line(pim);
#undef do_imlc
    return ERR_NONE;
}

// ======================================================================
void image_free(image_t* pim)
{
    if (pim == NULL) return;

    for (size_t i = 0; i < pim->height; ++i) {
        image_line_free(pim->content + i);
    }

    pim->height = 0;
    free(pim->content);
    pim->content = NULL;
}
