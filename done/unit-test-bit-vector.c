

// for thread-safe randomization
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <check.h>
#include <inttypes.h>

#include "tests.h"
#include "bit_vector.h"
#include "image.h"


#define PV1_SIZE 1
#define PV1_1_VALUE {0xFFFFFFFF}
#define PV1_0_VALUE {0x00000000}

#define PV1_5_VALUE {0x0000FFFF}

#define PV2_SIZE 2
#define PV2_1_VALUE {0xFFFFFFFF, 0xFFFFFFFF}
#define PV2_0_VALUE {0x00000000, 0x00000000}

#define PV2_5_VALUE {0xFFFFFFFF, 0x0000FFFF}

#define PV1_AAAAAAAA_VALUE {0xAAAAAAAA}
#define PV1_DEADBOSS_VALUE {0xdeadb055}
#define PV1_DEADBOSS_NOT_VALUE {0x21524FAA}
#define PV1_DEADBOSS_AAAAAAAA_AND_VALUE {0x8AA8A000}
#define PV1_DEADBOSS_AAAAAAAA_OR_VALUE {0xFEAFBAFF}
#define PV1_DEADBOSS_AAAAAAAA_XOR_VALUE {0x74071AFF}
#define PV1_DEADBOSS_EXT_ZERO_P5_VALUE  {0xAEF56D82, 0x6F56D82}
#define PV1_DEADBOSS_EXT_ZERO_M5_VALUE  {0xD5B60AA0, 0xD5B60ABB}
#define PV1_DEADBOSS_EXT_ZERO_P10_VALUE {0x1577AB6C, 0x0037AB6C}
#define PV1_DEADBOSS_EXT_ZERO_M10_VALUE {0xB6C15400, 0xB6C1577A}
#define PV1_DEADBOSS_EXT_WRAP_P5_VALUE  {0xAEF56D82, 0xAEF56D82}
#define PV1_DEADBOSS_EXT_WRAP_M5_VALUE  {0xD5B60ABB, 0xD5B60ABB}
#define PV1_DEADBOSS_EXT_WRAP_P10_VALUE {0x1577AB6C, 0x1577AB6C}
#define PV1_DEADBOSS_EXT_WRAP_M10_VALUE {0xB6C1577A, 0xB6C1577A}


#define fill_vector_with(vec,val,size) \
    do{\
        for(size_t i = 0; i<size; ++i){ vec->content[i] = val; }\
    }while(0)

#define vector_match_vector(vec1, vec2) \
    do{ \
      ck_assert((vec1)->size == (vec2)->size); \
      const size_t bound = (vec1)->size / IMAGE_LINE_WORD_BITS + ((vec1)->size % IMAGE_LINE_WORD_BITS ? 1 :0); \
      for (size_t i = 0; i < bound; ++i) { ck_assert_int_eq((vec1)->content[i], (vec2)->content[i]); } \
    } while(0)

#define vector_match_tab(vec, tab, S) \
    do {\
        const size_t bound1 = (vec)->size / IMAGE_LINE_WORD_BITS; \
        const size_t bound = (bound1 < (S) ? bound1 : (S));       \
        for (size_t i = 0; i < bound; ++i) { ck_assert_int_eq((vec)->content[i], (tab)[i]); } \
        if ((S) > bound) { \
            ck_assert_uint_eq(S, bound1 + 1); \
            const size_t shift = 8*sizeof((vec)->content[0]) - ((vec)->size % IMAGE_LINE_WORD_BITS) ; \
            ck_assert_int_eq((vec)->content[bound1] << shift, (tab)[bound1] << shift); \
        } \
    } while(0)

#define vector_match_val(vec, val, size) \
    do{\
        for(size_t i = 0; i<size; ++i){ ck_assert_int_eq(vec->content[i], val); }\
    }while(0)


START_TEST(bit_vector_create_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    bit_vector_t* pbv = NULL;

    ck_assert_ptr_null(bit_vector_create(0, 0));
    ck_assert_ptr_null(bit_vector_create(0, 1));
    ck_assert_ptr_null(bit_vector_create((size_t) -1, 0));

    const uint32_t pv1_1[] = PV1_1_VALUE;
    const uint32_t pv1_0[] = PV1_0_VALUE;
    const uint32_t pv2_1[] = PV2_1_VALUE;
    const uint32_t pv2_0[] = PV2_0_VALUE;
    const uint32_t pv1_5[] = PV1_5_VALUE;
    const uint32_t pv2_5[] = PV2_5_VALUE;

    pbv = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    ck_assert_ptr_nonnull(pbv);
    ck_assert(pbv->size == PV1_SIZE * IMAGE_LINE_WORD_BITS);
    vector_match_tab(pbv, pv1_1, PV1_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 0);
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv1_0, PV1_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv2_1, PV2_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv2_0, PV2_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS / 2, 1);
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv1_5, PV1_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS / 4 * 3, 1);
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv2_5, PV2_SIZE);
    bit_vector_free(&pbv);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


START_TEST(bit_vector_cpy_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_ptr_null(bit_vector_cpy(NULL));

    bit_vector_t* pbvc = NULL;
    bit_vector_t* pbv = NULL;
    const uint32_t deadboss = PV1_DEADBOSS_VALUE;

    pbv = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    pbvc = bit_vector_cpy(pbv);
    ck_assert_ptr_ne(pbv, pbvc);
    vector_match_vector(pbv, pbvc);
    bit_vector_free(&pbv);
    bit_vector_free(&pbvc);

    pbv = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    fill_vector_with(pbv, deadboss, PV2_SIZE);
    pbvc = bit_vector_cpy(pbv);
    ck_assert_ptr_ne(pbv, pbvc);
    ck_assert(pbv->size == pbvc->size);
    vector_match_val(pbvc, deadboss, PV2_SIZE);
    bit_vector_free(&pbv);
    bit_vector_free(&pbvc);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


START_TEST(bit_vector_get_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(bit_vector_get(NULL, 0), 0);
    bit_vector_t* pbv = NULL;
    const uint32_t deadboss = PV1_DEADBOSS_VALUE;

    pbv = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    ck_assert_int_eq(bit_vector_get(pbv, (size_t) -1), 0);
    ck_assert_int_eq(bit_vector_get(pbv, pbv->size + 1), 0);
    fill_vector_with(pbv, deadboss, PV1_SIZE);

    for (size_t i = 0; i < pbv->size; ++i) {
        size_t mask = (size_t)(0x1 << i);
        uint32_t rval = (uint32_t)((deadboss & mask) >> i);

        ck_assert_int_eq(rval, bit_vector_get(pbv, i));
    }

    bit_vector_free(&pbv);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


START_TEST(bit_vector_not_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    bit_vector_t* pbv = NULL;

    ck_assert_ptr_null(bit_vector_not(NULL));

    const uint32_t pv1_1[] = PV1_1_VALUE;
    const uint32_t pv1_0[] = PV1_0_VALUE;
    const uint32_t pv2_0[] = PV2_0_VALUE;
    const uint32_t pv2_1[] = PV2_1_VALUE;
    const uint32_t deadboss = PV1_DEADBOSS_VALUE;
    const uint32_t not_deadboss = PV1_DEADBOSS_NOT_VALUE;

    pbv = bit_vector_not(bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1));
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv1_0, PV1_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_not(bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 0));
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv1_1, PV1_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_not(bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1));
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv2_0, PV2_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_not(bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0));
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv2_1, PV2_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_not(bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS / 2, 1));
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv1_0, PV1_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_not(bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS / 4 * 3, 1));
    ck_assert_ptr_nonnull(pbv);
    vector_match_tab(pbv, pv2_0, PV2_SIZE);
    bit_vector_free(&pbv);

    pbv = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    ck_assert_ptr_nonnull(pbv);
    fill_vector_with(pbv, not_deadboss, PV2_SIZE);
    pbv = bit_vector_not(pbv);
    vector_match_val(pbv, deadboss, PV2_SIZE);
    bit_vector_free(&pbv);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(bit_vector_and_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_ptr_null(bit_vector_and(NULL, NULL));

    const uint32_t pv1_0[] = PV1_0_VALUE;
    const uint32_t pv1_1[] = PV1_1_VALUE;
    const uint32_t pv2_0[] = PV2_0_VALUE;
    const uint32_t pv2_5[] = PV2_5_VALUE;
    const uint32_t deadboss = PV1_DEADBOSS_VALUE;
    const uint32_t aaaaaaaa = PV1_AAAAAAAA_VALUE;
    const uint32_t deadaaaa = PV1_DEADBOSS_AAAAAAAA_AND_VALUE;

    bit_vector_t* ps1 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    bit_vector_t* ps2 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    ck_assert_ptr_null(bit_vector_and(ps1, ps2));
    bit_vector_t* psa = bit_vector_and(ps1, ps1);
    ck_assert_ptr_nonnull(psa);
    vector_match_tab(psa, pv1_1, PV1_SIZE);
    bit_vector_free(&ps1);
    bit_vector_free(&ps2);

    bit_vector_t* pbv1_1 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    bit_vector_t* pbv1_0 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 0);
    bit_vector_t* pbv1_a = bit_vector_and(pbv1_0, pbv1_1);
    ck_assert_ptr_nonnull(pbv1_a);
    vector_match_tab(pbv1_a, pv1_0, PV1_SIZE);
    bit_vector_free(&pbv1_1);
    bit_vector_free(&pbv1_0);

    bit_vector_t* pbv2_1 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    bit_vector_t* pbv2_0 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    bit_vector_t* pbv2_a = bit_vector_and(pbv2_0, pbv2_1);
    ck_assert_ptr_nonnull(pbv2_a);
    vector_match_tab(pbv2_a, pv2_0, PV1_SIZE);
    bit_vector_free(&pbv2_1);
    bit_vector_free(&pbv2_0);

    bit_vector_t* pbv5_1 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS / 4 * 3, 1);
    bit_vector_t* pbv5_a = bit_vector_and(pbv5_1, pbv5_1);
    ck_assert_ptr_nonnull(pbv5_a);
    vector_match_tab(pbv5_a, pv2_5, PV2_SIZE);
    bit_vector_free(&pbv5_1);

    bit_vector_t* pb_AAAAAAAA = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    bit_vector_t* pb_deadboss = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    fill_vector_with(pb_AAAAAAAA, aaaaaaaa, PV2_SIZE);
    fill_vector_with(pb_deadboss, deadboss, PV2_SIZE);
    bit_vector_t* pba = bit_vector_and(pb_AAAAAAAA, pb_deadboss);
    ck_assert_ptr_nonnull(pba);
    vector_match_val(pba, deadaaaa, PV2_SIZE);
    bit_vector_free(&pb_AAAAAAAA);
    bit_vector_free(&pb_deadboss);
    // bit_vector_free(&pba); // already freed (was same as pb_AAAAAAAA)
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(bit_vector_or_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_ptr_null(bit_vector_or(NULL, NULL));

    const uint32_t pv1_1[] = PV1_1_VALUE;
    const uint32_t pv2_1[] = PV2_1_VALUE;
    const uint32_t pv2_5[] = PV2_5_VALUE;
    const uint32_t deadboss = PV1_DEADBOSS_VALUE;
    const uint32_t aaaaaaaa = PV1_AAAAAAAA_VALUE;
    const uint32_t deadaaaa = PV1_DEADBOSS_AAAAAAAA_OR_VALUE;

    bit_vector_t* ps1 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    bit_vector_t* ps2 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    ck_assert_ptr_null(bit_vector_or(ps1, ps2));
    bit_vector_free(&ps1);
    bit_vector_free(&ps2);

    bit_vector_t* pbv1_1 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    bit_vector_t* pbv1_0 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 0);
    bit_vector_t* pbv1_a = bit_vector_or(pbv1_0, pbv1_1);
    ck_assert_ptr_nonnull(pbv1_a);
    vector_match_tab(pbv1_a, pv1_1, PV1_SIZE);
    bit_vector_free(&pbv1_1);
    bit_vector_free(&pbv1_0);

    bit_vector_t* pbv2_1 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    bit_vector_t* pbv2_0 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    bit_vector_t* pbv2_a = bit_vector_or(pbv2_0, pbv2_1);
    ck_assert_ptr_nonnull(pbv2_a);
    vector_match_tab(pbv2_a, pv2_1, PV2_SIZE);
    bit_vector_free(&pbv2_1);
    bit_vector_free(&pbv2_0);

    bit_vector_t* pbv5_1 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS / 4 * 3, 1);
    bit_vector_t* pbv5_0 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS / 4 * 3, 0);
    bit_vector_t* pbv5_a = bit_vector_or(pbv5_1, pbv5_1);
    ck_assert_ptr_nonnull(pbv5_a);
    vector_match_tab(pbv5_a, pv2_5, PV2_SIZE);
    bit_vector_free(&pbv5_1);
    bit_vector_free(&pbv5_0);

    bit_vector_t* pb_AAAAAAAA = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    bit_vector_t* pb_deadboss = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    fill_vector_with(pb_AAAAAAAA, aaaaaaaa, PV2_SIZE);
    fill_vector_with(pb_deadboss, deadboss, PV2_SIZE);
    bit_vector_t* pba = bit_vector_or(pb_AAAAAAAA, pb_deadboss);
    ck_assert_ptr_nonnull(pba);
    vector_match_val(pba, deadaaaa, PV2_SIZE);
    bit_vector_free(&pb_AAAAAAAA);
    bit_vector_free(&pb_deadboss);
    // bit_vector_free(&pba); // already freed (was same as pb_AAAAAAAA)
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


START_TEST(bit_vector_xor_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_ptr_null(bit_vector_xor(NULL, NULL));

    const uint32_t pv1_1[] = PV1_1_VALUE;
    const uint32_t pv2_1[] = PV2_1_VALUE;
    const uint32_t pv2_5[] = PV2_5_VALUE;
    const uint32_t pv2_0[] = PV2_0_VALUE;
    const uint32_t deadboss = PV1_DEADBOSS_VALUE;
    const uint32_t aaaaaaaa = PV1_AAAAAAAA_VALUE;
    const uint32_t deadaaaa = PV1_DEADBOSS_AAAAAAAA_XOR_VALUE;

    bit_vector_t* ps1 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    bit_vector_t* ps2 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    ck_assert_ptr_null(bit_vector_xor(ps1, ps2));
    bit_vector_free(&ps1);
    bit_vector_free(&ps2);

    bit_vector_t* pbv1_1 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    bit_vector_t* pbv1_0 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 0);
    bit_vector_t* pbv1_a = bit_vector_xor(pbv1_0, pbv1_1);
    ck_assert_ptr_nonnull(pbv1_a);
    vector_match_tab(pbv1_a, pv1_1, PV1_SIZE);
    bit_vector_free(&pbv1_1);
    bit_vector_free(&pbv1_0);

    bit_vector_t* pbv2_1 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    bit_vector_t* pbv2_0 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    bit_vector_t* pbv2_a = bit_vector_xor(pbv2_0, pbv2_1);
    ck_assert_ptr_nonnull(pbv2_a);
    vector_match_tab(pbv2_a, pv2_1, PV2_SIZE);
    bit_vector_free(&pbv2_1);
    bit_vector_free(&pbv2_0);

    bit_vector_t* pbv5_1 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS / 4 * 3, 1);
    bit_vector_t* pbv5_0 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS / 4 * 3, 0);
    bit_vector_t* pbv5_a = bit_vector_xor(pbv5_1, pbv5_0);
    ck_assert_ptr_nonnull(pbv5_a);
    vector_match_tab(pbv5_a, pv2_5, PV2_SIZE);
    bit_vector_free(&pbv5_1);
    bit_vector_free(&pbv5_0);

    bit_vector_t* pbv5_11 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS / 4 * 3, 1);
    bit_vector_t* pbv5_a1 = bit_vector_xor(pbv5_11, pbv5_11);
    ck_assert_ptr_nonnull(pbv5_a1);
    vector_match_tab(pbv5_a1, pv2_0, PV2_SIZE);
    bit_vector_free(&pbv5_11);

    bit_vector_t* pb_AAAAAAAA = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    bit_vector_t* pb_deadboss = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    fill_vector_with(pb_AAAAAAAA, aaaaaaaa, PV2_SIZE);
    fill_vector_with(pb_deadboss, deadboss, PV2_SIZE);
    bit_vector_t* pba = bit_vector_xor(pb_AAAAAAAA, pb_deadboss);
    ck_assert_ptr_nonnull(pba);
    vector_match_val(pba, deadaaaa, PV2_SIZE);
    bit_vector_free(&pb_AAAAAAAA);
    bit_vector_free(&pb_deadboss);
    // bit_vector_free(&pba); // already freed (was same as pb_AAAAAAAA)
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


START_TEST(bit_vector_extract_zero_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    bit_vector_t* pbv = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    ck_assert_ptr_null(bit_vector_extract_zero_ext(pbv, 0, 0));

    const size_t size = 5+2*IMAGE_LINE_WORD_BITS;
    bit_vector_t* result   = bit_vector_extract_zero_ext(NULL, 0, size);
    bit_vector_t* expected = bit_vector_create(size, 0);
    vector_match_vector(result, expected);
    bit_vector_free(&expected);
    bit_vector_free(&result);

    const uint32_t pv2_1[] = PV2_1_VALUE;
    const uint32_t pv2_0[] = PV2_0_VALUE;
    const uint32_t pv1_1[] = PV1_1_VALUE;
    const uint32_t pv1_0[] = PV1_0_VALUE;
    const uint32_t pv2_5[] = PV2_5_VALUE;
    const uint32_t deadboss = PV1_DEADBOSS_VALUE;
    const uint32_t deadboss_p5[] = PV1_DEADBOSS_EXT_ZERO_P5_VALUE;
    const uint32_t deadboss_m5[] = PV1_DEADBOSS_EXT_ZERO_M5_VALUE;
    const uint32_t deadboss_p10[] = PV1_DEADBOSS_EXT_ZERO_P10_VALUE;
    const uint32_t deadboss_m10[] = PV1_DEADBOSS_EXT_ZERO_M10_VALUE;

    bit_vector_t* pbv2_1 = bit_vector_extract_zero_ext(pbv, 0, 2 * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pbv2_0 = bit_vector_extract_zero_ext(pbv, 2 * IMAGE_LINE_WORD_BITS, 2 * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pbv1_1 = bit_vector_extract_zero_ext(pbv, 0, IMAGE_LINE_WORD_BITS);
    bit_vector_t* pbv1_0 = bit_vector_extract_zero_ext(pbv, -IMAGE_LINE_WORD_BITS, IMAGE_LINE_WORD_BITS);
    bit_vector_t* pbv2_5 = bit_vector_extract_zero_ext(pbv, IMAGE_LINE_WORD_BITS / 2, 2 * IMAGE_LINE_WORD_BITS);

    ck_assert_ptr_nonnull(pbv2_1);
    ck_assert_ptr_nonnull(pbv2_0);
    ck_assert_ptr_nonnull(pbv1_1);
    ck_assert_ptr_nonnull(pbv1_0);
    ck_assert_ptr_nonnull(pbv2_5);

    vector_match_tab(pbv1_1, pv1_1, PV1_SIZE);
    vector_match_tab(pbv1_0, pv1_0, PV1_SIZE);
    vector_match_tab(pbv2_1, pv2_1, PV2_SIZE);
    vector_match_tab(pbv2_0, pv2_0, PV2_SIZE);
    vector_match_tab(pbv2_5, pv2_5, PV2_SIZE);

    bit_vector_free(&pbv2_1);
    bit_vector_free(&pbv2_0);
    bit_vector_free(&pbv1_1);
    bit_vector_free(&pbv1_0);
    bit_vector_free(&pbv2_5);

    fill_vector_with(pbv, deadboss, PV2_SIZE);
    bit_vector_t* pba_p5 = bit_vector_extract_zero_ext(pbv, 5, PV2_SIZE * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pba_m5 = bit_vector_extract_zero_ext(pbv, -5, PV2_SIZE * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pba_p10 = bit_vector_extract_zero_ext(pbv, 10, PV2_SIZE * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pba_m10 = bit_vector_extract_zero_ext(pbv, -10, PV2_SIZE * IMAGE_LINE_WORD_BITS);

    ck_assert_ptr_nonnull(pba_p5);
    ck_assert_ptr_nonnull(pba_m5);
    ck_assert_ptr_nonnull(pba_p10);
    ck_assert_ptr_nonnull(pba_m10);

    vector_match_tab(pba_p5, deadboss_p5, PV2_SIZE);
    vector_match_tab(pba_m5, deadboss_m5, PV2_SIZE);
    vector_match_tab(pba_p10, deadboss_p10, PV2_SIZE);
    vector_match_tab(pba_m10, deadboss_m10, PV2_SIZE);

    bit_vector_free(&pba_p5);
    bit_vector_free(&pba_m5);
    bit_vector_free(&pba_p10);
    bit_vector_free(&pba_m10);

    bit_vector_free(&pbv);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(bit_vector_extract_wrap_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    bit_vector_t* pbv = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    ck_assert_ptr_null(bit_vector_extract_wrap_ext(NULL, 0, IMAGE_LINE_WORD_BITS));
    ck_assert_ptr_null(bit_vector_extract_wrap_ext(pbv, 0, 0));

    const uint32_t pv2_1[] = PV2_1_VALUE;
    const uint32_t pv2_0[] = PV2_1_VALUE;
    const uint32_t pv1_1[] = PV1_1_VALUE;
    const uint32_t pv1_0[] = PV1_1_VALUE;
    const uint32_t pv2_5[] = PV2_1_VALUE;
    const uint32_t deadboss = PV1_DEADBOSS_VALUE;
    const uint32_t deadboss_p5[] = PV1_DEADBOSS_EXT_WRAP_P5_VALUE;
    const uint32_t deadboss_m5[] = PV1_DEADBOSS_EXT_WRAP_M5_VALUE;
    const uint32_t deadboss_p10[] = PV1_DEADBOSS_EXT_WRAP_P10_VALUE;
    const uint32_t deadboss_m10[] = PV1_DEADBOSS_EXT_WRAP_M10_VALUE;


    bit_vector_t* pbv2_1 = bit_vector_extract_wrap_ext(pbv, 0, 2 * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pbv2_0 = bit_vector_extract_wrap_ext(pbv, 2 * IMAGE_LINE_WORD_BITS, 2 * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pbv1_1 = bit_vector_extract_wrap_ext(pbv, 0, IMAGE_LINE_WORD_BITS);
    bit_vector_t* pbv1_0 = bit_vector_extract_wrap_ext(pbv, -IMAGE_LINE_WORD_BITS, IMAGE_LINE_WORD_BITS);
    bit_vector_t* pbv2_5 = bit_vector_extract_wrap_ext(pbv, -IMAGE_LINE_WORD_BITS / 2, 2 * IMAGE_LINE_WORD_BITS);

    ck_assert_ptr_nonnull(pbv2_1);
    ck_assert_ptr_nonnull(pbv2_0);
    ck_assert_ptr_nonnull(pbv1_1);
    ck_assert_ptr_nonnull(pbv1_0);
    ck_assert_ptr_nonnull(pbv2_5);

    vector_match_tab(pbv1_1, pv1_1, PV1_SIZE);
    vector_match_tab(pbv1_0, pv1_0, PV1_SIZE);
    vector_match_tab(pbv2_1, pv2_1, PV2_SIZE);
    vector_match_tab(pbv2_0, pv2_0, PV2_SIZE);
    vector_match_tab(pbv2_5, pv2_5, PV2_SIZE);

    bit_vector_free(&pbv2_1);
    bit_vector_free(&pbv2_0);
    bit_vector_free(&pbv1_1);
    bit_vector_free(&pbv1_0);
    bit_vector_free(&pbv2_5);


    fill_vector_with(pbv, deadboss, PV2_SIZE);
    bit_vector_t* pba_p5 = bit_vector_extract_wrap_ext(pbv, 5, PV2_SIZE * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pba_m5 = bit_vector_extract_wrap_ext(pbv, -5, PV2_SIZE * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pba_p10 = bit_vector_extract_wrap_ext(pbv, 10, PV2_SIZE * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pba_m10 = bit_vector_extract_wrap_ext(pbv, -10, PV2_SIZE * IMAGE_LINE_WORD_BITS);

    ck_assert_ptr_nonnull(pba_p5);
    ck_assert_ptr_nonnull(pba_m5);
    ck_assert_ptr_nonnull(pba_p10);
    ck_assert_ptr_nonnull(pba_m10);

    vector_match_tab(pba_p5, deadboss_p5, PV2_SIZE);
    vector_match_tab(pba_m5, deadboss_m5, PV2_SIZE);
    vector_match_tab(pba_p10, deadboss_p10, PV2_SIZE);
    vector_match_tab(pba_m10, deadboss_m10, PV2_SIZE);

    bit_vector_free(&pba_p5);
    bit_vector_free(&pba_m5);
    bit_vector_free(&pba_p10);
    bit_vector_free(&pba_m10);

    bit_vector_free(&pbv);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(bit_vector_shift_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    bit_vector_t* pbv = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    ck_assert_ptr_null(bit_vector_shift(NULL, 0));

    const uint32_t pv2_1[] = PV2_1_VALUE;
    const uint32_t pv2_0[] = PV2_0_VALUE;
    const uint32_t pv2_5[] = PV2_5_VALUE;
    const uint32_t deadboss = PV1_DEADBOSS_VALUE;
    const uint32_t deadboss_p5[] = PV1_DEADBOSS_EXT_ZERO_P5_VALUE;
    const uint32_t deadboss_m5[] = PV1_DEADBOSS_EXT_ZERO_M5_VALUE;
    const uint32_t deadboss_p10[] = PV1_DEADBOSS_EXT_ZERO_P10_VALUE;
    const uint32_t deadboss_m10[] = PV1_DEADBOSS_EXT_ZERO_M10_VALUE;

    bit_vector_t* pbv2_1 = bit_vector_shift(pbv, 0);
    bit_vector_t* pbv2_00 = bit_vector_shift(pbv, 2 * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pbv2_01 = bit_vector_shift(pbv, -2 * IMAGE_LINE_WORD_BITS);
    bit_vector_t* pbv2_5f = bit_vector_shift(pbv, -IMAGE_LINE_WORD_BITS / 2);
    bit_vector_t* pbv2_5 = bit_vector_shift(pbv2_5f, 0);

    ck_assert_ptr_nonnull(pbv2_1);
    ck_assert_ptr_nonnull(pbv2_00);
    ck_assert_ptr_nonnull(pbv2_01);
    ck_assert_ptr_nonnull(pbv2_5);

    vector_match_tab(pbv2_1, pv2_1, PV2_SIZE);
    vector_match_tab(pbv2_00, pv2_0, PV2_SIZE);
    vector_match_tab(pbv2_01, pv2_0, PV2_SIZE);
    vector_match_tab(pbv2_5, pv2_5, PV2_SIZE);

    bit_vector_free(&pbv2_1);
    bit_vector_free(&pbv2_00);
    bit_vector_free(&pbv2_01);
    bit_vector_free(&pbv2_5f);
    bit_vector_free(&pbv2_5);

    fill_vector_with(pbv, deadboss, PV2_SIZE);
    bit_vector_t* pba_p5 = bit_vector_shift(pbv, -5);
    bit_vector_t* pba_m5 = bit_vector_shift(pbv, 5);
    bit_vector_t* pba_p10 = bit_vector_shift(pbv, -10);
    bit_vector_t* pba_m10 = bit_vector_shift(pbv, 10);

    ck_assert_ptr_nonnull(pba_p5);
    ck_assert_ptr_nonnull(pba_m5);
    ck_assert_ptr_nonnull(pba_p10);
    ck_assert_ptr_nonnull(pba_m10);

    vector_match_tab(pba_p5, deadboss_p5, PV2_SIZE);
    vector_match_tab(pba_m5, deadboss_m5, PV2_SIZE);
    vector_match_tab(pba_p10, deadboss_p10, PV2_SIZE);
    vector_match_tab(pba_m10, deadboss_m10, PV2_SIZE);

    bit_vector_free(&pba_p5);
    bit_vector_free(&pba_m5);
    bit_vector_free(&pba_p10);
    bit_vector_free(&pba_m10);

    bit_vector_free(&pbv);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST



START_TEST(bit_vector_join_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    bit_vector_t* pbv1 = NULL;
    bit_vector_t* pbv0 = NULL;
    bit_vector_t* pbvj = NULL;

    ck_assert_ptr_null(bit_vector_join(NULL, pbv1, 0));
    ck_assert_ptr_null(bit_vector_join(pbv0, NULL, 0));
    pbv1 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    pbv0 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    ck_assert_ptr_nonnull(pbv1);
    ck_assert_ptr_nonnull(pbv0);
    ck_assert_ptr_null(bit_vector_join(NULL, pbv1, 0));
    ck_assert_ptr_null(bit_vector_join(pbv1, NULL, 0));
    ck_assert_ptr_null(bit_vector_join(pbv1, pbv0, 0));
    ck_assert_ptr_nonnull(pbvj = bit_vector_join(pbv1, pbv1, IMAGE_LINE_WORD_BITS));
    bit_vector_free(&pbv0);
    bit_vector_free(&pbv1);
    bit_vector_free(&pbvj);

    const uint32_t pv1_5[] = PV1_5_VALUE;
    const uint32_t pv2_5[] = PV2_5_VALUE;
    const uint32_t deadboss[] = PV1_DEADBOSS_VALUE;

    pbv1 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    pbv0 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 0);
    ck_assert_ptr_nonnull(pbv1);
    ck_assert_ptr_nonnull(pbv0);
    pbvj = bit_vector_join(pbv1, pbv0, 16);
    ck_assert_ptr_nonnull(pbvj);
    vector_match_tab(pbvj, pv1_5, PV1_SIZE);
    bit_vector_free(&pbv0);
    bit_vector_free(&pbv1);
    bit_vector_free(&pbvj);

    pbv1 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 1);
    pbv0 = bit_vector_create(PV2_SIZE * IMAGE_LINE_WORD_BITS, 0);
    ck_assert_ptr_nonnull(pbv1);
    ck_assert_ptr_nonnull(pbv0);
    pbvj = bit_vector_join(pbv1, pbv0, 48);
    ck_assert_ptr_nonnull(pbvj);
    vector_match_tab(pbvj, pv2_5, PV2_SIZE);
    bit_vector_free(&pbv0);
    bit_vector_free(&pbv1);
    bit_vector_free(&pbvj);


    pbv1 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 1);
    fill_vector_with(pbv1, 0xB055B055, PV1_SIZE);
    pbv0 = bit_vector_create(PV1_SIZE * IMAGE_LINE_WORD_BITS, 0);
    fill_vector_with(pbv0, 0xDEADDEAD, PV1_SIZE);
    ck_assert_ptr_nonnull(pbv1);
    ck_assert_ptr_nonnull(pbv0);
    pbvj = bit_vector_join(pbv1, pbv0, 16);
    ck_assert_ptr_nonnull(pbvj);
    vector_match_tab(pbvj, deadboss, PV1_SIZE);
    bit_vector_free(&pbv0);
    bit_vector_free(&pbv1);
    bit_vector_free(&pbvj);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


#define PVV2_VALUE {0x0001FFFF}
#define PVV3_VALUE {0xFFE0003F,0xFFE0003F}

START_TEST(bit_vector_various)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    bit_vector_t* pv1 = bit_vector_create(1 * IMAGE_LINE_WORD_BITS, 1);
    ck_assert_ptr_nonnull(pv1);
    uint32_t pv1e[] = PV1_1_VALUE;
    vector_match_tab(pv1, pv1e, 1);

    bit_vector_t* pv2 = bit_vector_not(bit_vector_extract_zero_ext(pv1, -17, 1 * IMAGE_LINE_WORD_BITS));
    ck_assert_ptr_nonnull(pv2);
    uint32_t pv2e[] = PVV2_VALUE;
    vector_match_tab(pv2, pv2e, 1);

    bit_vector_t* pv3 = bit_vector_extract_wrap_ext(pv2, 11, 2 * IMAGE_LINE_WORD_BITS);
    ck_assert_ptr_nonnull(pv3);
    uint32_t pv3e[] = PVV3_VALUE;
    vector_match_tab(pv3, pv3e, 2);


    bit_vector_free(&pv1);
    bit_vector_free(&pv2);
    bit_vector_free(&pv3);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

#define PV_DEADBOSS_AAAA_VALUE {0xdeadb055, 0xaaaa}

START_TEST(bit_vector_deadboss)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    bit_vector_t* pvb_1 = bit_vector_create(1, 1);
    ck_assert_ptr_nonnull(pvb_1);
    bit_vector_t* pv1_8 = bit_vector_extract_zero_ext(pvb_1, -3, 4);
    ck_assert_ptr_nonnull(pv1_8);
    bit_vector_t* pv4_8888 = bit_vector_extract_wrap_ext(pv1_8, 0, 16);
    ck_assert_ptr_nonnull(pv4_8888);
    bit_vector_t* pv4_2222 = bit_vector_shift(pv4_8888, -2);
    ck_assert_ptr_nonnull(pv4_2222);
    bit_vector_t* pv4_AAAA = bit_vector_or(bit_vector_cpy(pv4_2222), pv4_8888);
    ck_assert_ptr_nonnull(pv4_AAAA);
    bit_vector_t* pv8_0000AAAA = bit_vector_extract_zero_ext(pv4_AAAA, 0, IMAGE_LINE_WORD_BITS);
    ck_assert_ptr_nonnull(pv8_0000AAAA);

    bit_vector_t* interm = NULL;
    bit_vector_t* interm2 = NULL;
    bit_vector_t* pv1_D = bit_vector_or(bit_vector_cpy(pv1_8), bit_vector_or(interm  = bit_vector_shift(pv1_8, -1),
                                        interm2 = bit_vector_shift(pv1_8, -3)));
    bit_vector_free(&interm);
    bit_vector_free(&interm2);
    ck_assert_ptr_nonnull(pv1_D);
    bit_vector_t* pv1_E = bit_vector_or(bit_vector_cpy(pv1_8), bit_vector_or(interm  = bit_vector_shift(pv1_8, -1),
                                        interm2 = bit_vector_shift(pv1_8, -2)));
    bit_vector_free(&interm);
    bit_vector_free(&interm2);
    ck_assert_ptr_nonnull(pv1_E);
    bit_vector_t* pv1_A = bit_vector_or(bit_vector_cpy(pv1_8), interm = bit_vector_shift(pv1_8, -2));
    bit_vector_free(&interm);
    ck_assert_ptr_nonnull(pv1_A);
    bit_vector_t* pv1_B = bit_vector_or(bit_vector_cpy(pv1_A), interm = bit_vector_shift(pv1_8, -3));
    bit_vector_free(&interm);
    ck_assert_ptr_nonnull(pv1_B);
    bit_vector_t* pv1_5 = bit_vector_not(bit_vector_cpy(pv1_A));
    ck_assert_ptr_nonnull(pv1_5);

    bit_vector_t* pv4_D0A0 = bit_vector_or(bit_vector_and(bit_vector_extract_zero_ext(pv1_A, -4, 16), pv4_AAAA),
                                           interm = bit_vector_extract_zero_ext(pv1_D, -12, 16)) ;
    bit_vector_free(&interm);
    ck_assert_ptr_nonnull(pv4_D0A0);
    bit_vector_t* pv4_0E0D = bit_vector_or(bit_vector_extract_zero_ext(pv1_E, -8, 16),
                                           interm = bit_vector_extract_zero_ext(pv1_D, 0, 16)) ;
    bit_vector_free(&interm);
    ck_assert_ptr_nonnull(pv4_0E0D);
    bit_vector_t* pv4_DEAD = bit_vector_or(pv4_D0A0, pv4_0E0D);
    ck_assert_ptr_nonnull(pv4_DEAD);

    bit_vector_t* pv4_00SS = bit_vector_extract_zero_ext(interm = bit_vector_extract_wrap_ext(pv1_5, -64, 8), 0, 16);
    bit_vector_free(&interm);
    ck_assert_ptr_nonnull(pv4_00SS);
    bit_vector_t* pv4_BOSS = bit_vector_or(bit_vector_extract_zero_ext(pv1_B, -12, 16), pv4_00SS);
    ck_assert_ptr_nonnull(pv4_BOSS);

    bit_vector_t* interm3 = NULL;
    bit_vector_t* pv8_DEADBOSS = bit_vector_join(interm  = bit_vector_extract_wrap_ext(pv4_BOSS, 0, IMAGE_LINE_WORD_BITS),
                                 interm2 = bit_vector_extract_wrap_ext(pv4_DEAD, 0, IMAGE_LINE_WORD_BITS), 16);
    bit_vector_free(&interm);
    bit_vector_free(&interm2);
    ck_assert_ptr_nonnull(pv8_DEADBOSS);

    bit_vector_t* pv12_AAAADEADBOSS = bit_vector_extract_zero_ext(
                                      interm = bit_vector_join(interm2 = bit_vector_extract_wrap_ext(pv8_DEADBOSS, -IMAGE_LINE_WORD_BITS, 2 * IMAGE_LINE_WORD_BITS),
                                              interm3 = bit_vector_extract_wrap_ext(pv8_0000AAAA, IMAGE_LINE_WORD_BITS, 2 * IMAGE_LINE_WORD_BITS), IMAGE_LINE_WORD_BITS
                                                              ), 0, 48);
    bit_vector_free(&interm);
    bit_vector_free(&interm2);
    bit_vector_free(&interm3);
    ck_assert_ptr_nonnull(pv12_AAAADEADBOSS);


    const uint32_t pve[] = PV_DEADBOSS_AAAA_VALUE;
    const bit_vector_t* pvr = pv12_AAAADEADBOSS;
    vector_match_tab(pvr, pve, 2);

    bit_vector_free(&pv12_AAAADEADBOSS);
    bit_vector_free(&pv8_DEADBOSS);
    bit_vector_free(&pv4_BOSS);
    bit_vector_free(&pv4_00SS);
    bit_vector_free(&pv4_DEAD);
    bit_vector_free(&pv4_0E0D);
    // bit_vector_free(&pv4_D0A0);
    bit_vector_free(&pv1_5);
    bit_vector_free(&pv1_B);
    bit_vector_free(&pv1_A);
    bit_vector_free(&pv1_E);
    bit_vector_free(&pv1_D);
    bit_vector_free(&pv8_0000AAAA);
    bit_vector_free(&pv4_AAAA);
    bit_vector_free(&pv4_2222);
    bit_vector_free(&pv4_8888);
    bit_vector_free(&pv1_8);
    bit_vector_free(&pvb_1);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

Suite* cartridge_test_suite()
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
    srand(time(NULL) ^ getpid() ^ pthread_self());
#pragma GCC diagnostic pop

    Suite* s = suite_create("bit_vector.c Tests");

    Add_Case(s, tc1, "BitVector Tests");
    tcase_add_test(tc1, bit_vector_create_exec);
    tcase_add_test(tc1, bit_vector_cpy_exec);
    tcase_add_test(tc1, bit_vector_get_exec);
    tcase_add_test(tc1, bit_vector_not_exec);
    tcase_add_test(tc1, bit_vector_and_exec);
    tcase_add_test(tc1, bit_vector_or_exec);
    tcase_add_test(tc1, bit_vector_xor_exec);
    tcase_add_test(tc1, bit_vector_extract_zero_exec);
    tcase_add_test(tc1, bit_vector_extract_wrap_exec);
    tcase_add_test(tc1, bit_vector_shift_exec);
    tcase_add_test(tc1, bit_vector_join_exec);
    tcase_add_test(tc1, bit_vector_various);
    tcase_add_test(tc1, bit_vector_deadboss);

    return s;
}

TEST_SUITE(cartridge_test_suite)

