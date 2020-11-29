

// for thread-safe randomization
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <check.h>
#include <inttypes.h>
#include <assert.h>

#include "tests.h"
#include "bit.h"

// ------------------------------------------------------------
#define LOOP_ON(T) const size_t s_ = sizeof(T) / sizeof(*T);  \
  for(size_t i_ = 0; i_ < s_; ++i_)

#define ASSERT_EQ_NB_EL(T1, T2) \
    static_assert(sizeof(T1) / sizeof(*T1) == sizeof(T2) / sizeof(*T2), "Wrong Size in test tables")

START_TEST(handout_examples)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    uint8_t input    = 0xab;
    uint8_t expected = 0x0b;
    uint8_t result = lsb4(input);
    ck_assert_msg(result == expected,
                  "lsb4() failed on 0x%" PRIX8 ": got 0x%"
                  PRIX8 " instead of 0x%" PRIX8,
                  input, result, expected);

    expected = 0x0a;
    result   = msb4(input);
    ck_assert_msg(result == expected,
                  "msb4() failed on 0x%" PRIX8 ": got 0x%"
                  PRIX8 " instead of 0x%" PRIX8,
                  input, result, expected);

    const int index[]          = { 2, 1, -1, 8 };
    const bit_t expected_bit[] = { 0, 1,  1, 1 };
    LOOP_ON(index) {
        result   = bit_get(input, index[i_]);
        ck_assert_msg(result == expected_bit[i_],
                      "bit_get(0x%" PRIX8 ", %d) failed: got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input, index[i_], result, expected_bit[i_]);
    }

    expected = 0xaf;
    result   = input;
    bit_set(&result, index[0]);
    ck_assert_msg(result == expected,
                  "bit_set(0x%" PRIX8 ", %d) failed: got 0x%"
                  PRIX8 " instead of 0x%" PRIX8,
                  input, index[0], result, expected);

    expected = 0xa9;
    result   = input;
    bit_unset(&result, index[1]);
    ck_assert_msg(result == expected,
                  "bit_unset(0x%" PRIX8 ", %d) failed: got 0x%"
                  PRIX8 " instead of 0x%" PRIX8,
                  input, index[1], result, expected);

    uint8_t input2 = 0xed;
    expected = 0xdb;
    result   = merge4(input, input2);
    ck_assert_msg(result == expected,
                  "merge4(0x%" PRIX8 ", 0x%" PRIX8 ") failed: got 0x%"
                  PRIX8 " instead of 0x%" PRIX8,
                  input, input2, result, expected);

    input  = 0xef;
    input2 = 0xbe;
    const uint16_t expected2 = 0xbeef;
    const uint16_t result2   = merge8(input, input2);
    ck_assert_msg(result2 == expected2,
                  "merge8(0x%" PRIX8 ", 0x%" PRIX8 ") failed: got 0x%"
                  PRIX16 " instead of 0x%" PRIX16,
                  input, input2, result2, expected2);

    const uint16_t input3 = 0xbeef;
    expected = 0xef;
    result   = lsb8(input3);
    ck_assert_msg(result == expected,
                  "lsb8() failed on 0x%" PRIX16 ": got 0x%"
                  PRIX8 " instead of 0x%" PRIX8,
                  input3, result, expected);

    expected = 0xbe;
    result   = msb8(input3);
    ck_assert_msg(result == expected,
                  "msb8() failed on 0x%" PRIX16 ": got 0x%"
                  PRIX8 " instead of 0x%" PRIX8,
                  input3, result, expected);

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(lsb4_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input   [] = {0xAB, 0x01, 0xF0, 0x11, 0xA9, 0x10};
    const uint8_t expected[] = {0x0B, 0x01, 0x00, 0x01, 0x09, 0x00};
    ASSERT_EQ_NB_EL(input, expected);

    LOOP_ON(input) {
        const uint8_t result = lsb4(input[i_]);
        ck_assert_msg(result == expected[i_],
                      "lsb4() failed on 0x%" PRIX8 ": got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input[i_], result, expected[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(bit_rotate_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    uint8_t input[] = {0xAB, 0x01, 0xF0, 0x11, 0xA9, 0x10};
    const uint8_t expected[][7] = {
        {0x57, 0xAE, 0x5D, 0xBA, 0x75, 0xEA, 0xD5}, // 0xAB
        {0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}, // 0x01
        {0xE1, 0xC3, 0x87, 0x0F, 0x1E, 0x3C, 0x78}, // 0xF0
        {0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88}, // 0x11
        {0x53, 0xA6, 0x4D, 0x9A, 0x35, 0x6A, 0xD4}, // 0xA9
        {0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08}  // 0x10
    };
    ASSERT_EQ_NB_EL(input, expected);

    const int nb_shifts = sizeof(*expected) / sizeof(**expected);
    LOOP_ON(input) {
        const uint8_t old_input = input[i_];

        for (int d = 1; d <= nb_shifts; ++d) {
            bit_rotate(&input[i_], LEFT, d);

            ck_assert_msg(input[i_] == expected[i_][d - 1],
                          "bit_rotate(LEFT, %d) failed on 0x%" PRIX8 ": got 0x%"
                          PRIX8 " instead of 0x%" PRIX8,
                          d, old_input, input[i_], expected[i_][d - 1]);

            const uint8_t new_input = input[i_];
            bit_rotate(&input[i_], RIGHT, d);
            ck_assert_msg(input[i_] == old_input,
                          "bit_rotate(RIGTH, %d) failed on 0x%" PRIX8 ": got 0x%"
                          PRIX8 " instead of 0x%" PRIX8,
                          d, new_input, input[i_], old_input);
        }

        bit_rotate(&input[i_], LEFT, 8);
        ck_assert_msg(input[i_] == old_input,
                      "bit_rotate(LEFT, 8) failed on 0x%" PRIX8 ": got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      old_input, input[i_], old_input);

        bit_rotate(&input[i_], RIGHT, 8);
        ck_assert_msg(input[i_] == old_input,
                      "bit_rotate(RIGHT, 8) failed on 0x%" PRIX8 ": got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      old_input, input[i_], old_input);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(lsb8_msb8_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    const uint16_t input        [] = {0xBAAB, 0xFF01, 0xCCF0, 0x0011, 0x11A9, 0xDC00};
    const uint8_t  expected_low [] = {  0xAB,   0x01,   0xF0,   0x11,   0xA9,   0x00};
    const uint8_t  expected_high[] = {0xBA, 0xFF, 0xCC, 0x00, 0x11, 0xDC};
    ASSERT_EQ_NB_EL(input, expected_low);
    ASSERT_EQ_NB_EL(input, expected_high);

    LOOP_ON(input) {
        uint8_t result = lsb8(input[i_]);
        ck_assert_msg(result == expected_low[i_],
                      "lsb8() failed on 0x%" PRIX16 ": got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input[i_], result, expected_low[i_]);
        result = msb8(input[i_]);
        ck_assert_msg(msb8(input[i_])  == expected_high[i_],
                      "msb8() failed on 0x%" PRIX16 ": got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input[i_], result, expected_high[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(merge8_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    const uint8_t high     [] = {0xBA,   0xFF,   0xCC,   0x00,   0x11,   0xDC  };
    const uint8_t low      [] = {  0xAB,   0x01,   0xF0,   0x11,   0xA9,   0x00};
    const uint16_t expected[] = {0xBAAB, 0xFF01, 0xCCF0, 0x0011, 0x11A9, 0xDC00};
    ASSERT_EQ_NB_EL(low, expected);
    ASSERT_EQ_NB_EL(high, expected);

    LOOP_ON(low) {
        const uint16_t result = merge8(low[i_], high[i_]);
        ck_assert_msg(result == expected[i_],
                      "merge8(0x%" PRIX8 ", 0x%" PRIX8 ") failed: got 0x%"
                      PRIX16 " instead of 0x%" PRIX16,
                      low[i_], high[i_], result, expected[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

// ======================================================================
Suite* bus_test_suite()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
    srand(time(NULL) ^ getpid() ^ pthread_self());
#pragma GCC diagnostic pop

    Suite* s = suite_create("bit.c tests");

    Add_Case(s, tc1, "various bit tests");
    tcase_add_test(tc1, handout_examples);
    tcase_add_test(tc1, lsb4_exec);
    tcase_add_test(tc1, bit_rotate_exec);
    tcase_add_test(tc1, lsb8_msb8_exec);
    tcase_add_test(tc1, merge8_exec);

    return s;
}

TEST_SUITE(bus_test_suite)
