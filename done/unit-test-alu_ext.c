
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
#include "alu.h"
#include "alu_ext.h"
#include "bit.h"
#include "error.h"

// ------------------------------------------------------------
#define LOOP_ON(T) const size_t s_ = sizeof(T) / sizeof(*T);  \
  for(size_t i_ = 0; i_ < s_; ++i_)

#define ASSERT_EQ_NB_EL(T1, T2) \
    static_assert(sizeof(T1) / sizeof(*T1) == sizeof(T2) / sizeof(*T2), "Wrong Size in test tables")

START_TEST(alu_bcd_adjust_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_bcd_adjust(NULL), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(alu_bcd_adjust_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    uint8_t input_v    [] = {0x00, 0x01, 0x0A, 0x6D, 0x99, 0x0F};
    uint8_t input_f    [] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x60};

    const uint16_t expected_v[] = {0x00, 0x01, 0x10, 0x73, 0x99, 0x09};
    const flags_t expected_f [] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x40};

    ASSERT_EQ_NB_EL(input_v, input_f);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);

    LOOP_ON(input_v) {
        alu_output_t result = {input_v[i_], input_f[i_]};
        ck_assert_int_eq(alu_bcd_adjust(&result), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_bcd_adjust() failed on 0x%" PRIX8 " (flags = 0x%" PRIX8 ") :got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_v[i_], input_f[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_bcd_adjust() failed on 0x%" PRIX8 " (flags = 0x%" PRIX8 ") : got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_v[i_], input_f[i_], result.flags, expected_f[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


START_TEST(alu_and_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_and(NULL, 0, 0), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(alu_and_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    uint8_t input_x   [] = {0x53, 0x00, 0xFF};
    uint8_t input_y   [] = {0xA7, 0xFF, 0xFF};

    const uint16_t expected_v[] = {0x03, 0x00, 0xFF};
    const flags_t expected_f[]  = {0x20, 0xA0, 0x20};

    ASSERT_EQ_NB_EL(input_x, input_y);
    ASSERT_EQ_NB_EL(input_y, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);


    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_and(&result, input_x[i_], input_y[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_and() failed on 0x%" PRIX8 " & 0x%" PRIX8 " :got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_and() failed on 0x%" PRIX8 " & 0x%" PRIX8 " : got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], result.flags, expected_f[i_]);

    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


START_TEST(alu_or_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_or(NULL, 0, 0), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(alu_or_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    uint8_t input_x   []  = {0x53, 0x00, 0x00, 0xFF};
    uint8_t input_y   []  = {0xA7, 0xFF, 0x00, 0xFF};

    const uint16_t expected_v[] = {0xF7, 0xFF, 0x00, 0xFF};
    const flags_t expected_f[]  = {0x00, 0x00, 0x80, 0x00};

    ASSERT_EQ_NB_EL(input_x, input_y);
    ASSERT_EQ_NB_EL(input_y, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);


    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_or(&result, input_x[i_], input_y[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_or() failed on 0x%" PRIX8 " | 0x%" PRIX8 " :got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_or() failed on 0x%" PRIX8 " | 0x%" PRIX8 " : got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], result.flags, expected_f[i_]);

    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


START_TEST(alu_xor_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_xor(NULL, 0, 0), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(alu_xor_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    uint8_t input_x   []  = {0x53, 0x00, 0x00, 0xFF};
    uint8_t input_y   []  = {0xA7, 0xFF, 0x00, 0xFF};

    const uint16_t expected_v[] = {0xF4, 0xFF, 0x00, 0x00};
    const flags_t expected_f[]  = {0x00, 0x00, 0x80, 0x80};

    ASSERT_EQ_NB_EL(input_x, input_y);
    ASSERT_EQ_NB_EL(input_y, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);


    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_xor(&result, input_x[i_], input_y[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_xor() failed on 0x%" PRIX8 " XOR 0x%" PRIX8 " :got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_xor() failed on 0x%" PRIX8 " XOR 0x%" PRIX8 " : got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], result.flags, expected_f[i_]);

    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


Suite* bus_test_suite()
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
    srand(time(NULL) ^ getpid() ^ pthread_self());
#pragma GCC diagnostic pop

    Suite* s = suite_create("alu.c Tests");

    Add_Case(s, tc1, "various alu tests");
    tcase_add_test(tc1, alu_bcd_adjust_err);
    tcase_add_test(tc1, alu_bcd_adjust_exec);

    tcase_add_test(tc1, alu_and_err);
    tcase_add_test(tc1, alu_and_exec);

    tcase_add_test(tc1, alu_or_err);
    tcase_add_test(tc1, alu_or_exec);

    tcase_add_test(tc1, alu_xor_err);
    tcase_add_test(tc1, alu_xor_exec);

    return s;
}

TEST_SUITE(bus_test_suite)
