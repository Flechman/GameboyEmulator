
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
#include "bit.h"
#include "error.h"

// ------------------------------------------------------------
#define LOOP_ON(T) const size_t s_ = sizeof(T) / sizeof(*T);  \
  for(size_t i_ = 0; i_ < s_; ++i_)

#define ASSERT_EQ_NB_EL(T1, T2) \
    static_assert(sizeof(T1) / sizeof(*T1) == sizeof(T2) / sizeof(*T2), "Wrong Size in test tables")

START_TEST(flags_get_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input[]    = {FLAG_Z, FLAG_N, FLAG_H, FLAG_C,
                                0x03, 0x05, 0x07, 0x09, 0x30, 0x50,
                                0x60, 0x70, 0x90,
                                0xA0, 0xB0, 0xC0
                               };
    const uint8_t expected[] = {FLAG_Z, FLAG_N, FLAG_H, FLAG_C,
                                0x00, 0x00, 0x00, 0x00, FLAG_H | FLAG_C, FLAG_N | FLAG_C,
                                FLAG_N | FLAG_H, FLAG_C | FLAG_H | FLAG_N, FLAG_Z | FLAG_C,
                                FLAG_Z | FLAG_H, FLAG_Z | FLAG_H | FLAG_C, FLAG_Z | FLAG_N
                               };
    ASSERT_EQ_NB_EL(input, expected);

    LOOP_ON(input) {
        const uint8_t resultZ = get_Z(input[i_]);
        ck_assert_msg(resultZ == (expected[i_]&FLAG_Z),
                      "get_Z() failed on 0x%" PRIX8 ": got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input[i_], resultZ, expected[i_]);
        const uint8_t resultN = get_N(input[i_]);
        ck_assert_msg(resultN == (expected[i_]&FLAG_N),
                      "get_N() failed on 0x%" PRIX8 ": got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input[i_], resultN, expected[i_]);

        const uint8_t resultH = get_H(input[i_]);
        ck_assert_msg(resultH == (expected[i_]&FLAG_H),
                      "get_H() failed on 0x%" PRIX8 ": got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input[i_], resultH, expected[i_]);

        const uint8_t resultC = get_C(input[i_]);
        ck_assert_msg(resultC == (expected[i_]&FLAG_C),
                      "get_C() failed on 0x%" PRIX8 ": got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input[i_], resultC, expected[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(flags_set_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input[]    = {FLAG_Z, FLAG_N, FLAG_H, FLAG_C,
                                0x03, 0x05, 0x07, 0x09, FLAG_H | FLAG_C, FLAG_N | FLAG_C,
                                FLAG_N | FLAG_H, FLAG_C | FLAG_H | FLAG_N, FLAG_Z | FLAG_C,
                                FLAG_Z | FLAG_H, FLAG_Z | FLAG_H | FLAG_C, FLAG_Z | FLAG_N
                               };
    const uint8_t expected[] = {FLAG_Z, FLAG_N, FLAG_H, FLAG_C,
                                0x00, 0x00, 0x00, 0x00, 0x30, 0x50,
                                0x60, 0x70, 0x90,
                                0xA0, 0xB0, 0xC0
                               };
    ASSERT_EQ_NB_EL(input, expected);

    LOOP_ON(input) {
        flags_t flags = 0x0;

        if (get_Z(input[i_]))
            set_Z(&flags);

        if (get_N(input[i_]))
            set_N(&flags);

        if (get_H(input[i_]))
            set_H(&flags);

        if (get_C(input[i_]))
            set_C(&flags);

        ck_assert_msg(flags == expected[i_],
                      "set_?() failed on 0x%" PRIX8 ": got 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input[i_], flags, expected[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_add8_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_add8(NULL, 0, 0, 0), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_add8_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    const uint8_t input_x[] = {0x10, 0x08, 0x80, 0xF0};
    const uint8_t input_y[] = {0x15, 0x08, 0x7F, 0x10};
    const uint8_t input_c[] = {0x00, 0x00, 0x01, 0x01};

    const uint16_t expected_v[] = {0x0025, 0x0010, 0x000, 0x0001};
    const flags_t  expected_f[] = {0x00, 0x20, 0xB0, 0x10};

    ASSERT_EQ_NB_EL(input_x, input_y);
    ASSERT_EQ_NB_EL(input_y, input_c);
    ASSERT_EQ_NB_EL(input_c, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);

    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_add8(&result, input_x[i_], input_y[i_], input_c[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_add8() failed on 0x%" PRIX8 " + 0x%" PRIX8 " (c=%d):got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], input_c[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_add8() failed on 0x%" PRIX8 " + 0x%" PRIX8 " (c=%d): got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], input_c[i_], result.flags, expected_f[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_add16_low_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_add16_low(NULL, 0, 0), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_add16_low_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    const uint16_t input_x[] = {0x11FF};
    const uint16_t input_y[] = {0x0001};

    const uint16_t expected_v[] = {0x1200};
    const flags_t  expected_f[] = {0x30};

    ASSERT_EQ_NB_EL(input_x, input_y);
    ASSERT_EQ_NB_EL(input_y, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);

    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_add16_low(&result, input_x[i_], input_y[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_add16_low() failed on 0x%" PRIX8 " + 0x%" PRIX8 " :got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_add16_low() failed on 0x%" PRIX8 " + 0x%" PRIX8 " : got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], result.flags, expected_f[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_add16_high_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_add16_low(NULL, 0, 0), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_add16_high_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    const uint16_t input_x[] = {0x11FF, 0xFF11};
    const uint16_t input_y[] = {0x0001, 0x0111};

    const uint16_t expected_v[] = {0x1200, 0x0022};
    const flags_t  expected_f[] = {0x00, 0x30};

    ASSERT_EQ_NB_EL(input_x, input_y);
    ASSERT_EQ_NB_EL(input_y, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);

    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_add16_high(&result, input_x[i_], input_y[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_add16_high() failed on 0x%" PRIX8 " + 0x%" PRIX8 " :got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_add16_high() failed on 0x%" PRIX8 " + 0x%" PRIX8 " : got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], result.flags, expected_f[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_sub8_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_sub8(NULL, 0, 0, 0), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_sub8_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    const uint8_t input_x[] = {0x10, 0x10, 0x01};
    const uint8_t input_y[] = {0x10, 0x80, 0x01};
    const uint8_t input_c[] = {0x00, 0x00, 0x01};

    const uint16_t expected_v[] = {0x00, 0x90, 0xFF};
    const flags_t  expected_f[] = {0xC0, 0x50, 0x70};

    ASSERT_EQ_NB_EL(input_x, input_y);
    ASSERT_EQ_NB_EL(input_y, input_c);
    ASSERT_EQ_NB_EL(input_c, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);

    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_sub8(&result, input_x[i_], input_y[i_], input_c[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_sub8() failed on 0x%" PRIX8 " + 0x%" PRIX8 " (c=%d):got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], input_c[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_sub8() failed on 0x%" PRIX8 " + 0x%" PRIX8 " (c=%d): got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_y[i_], input_c[i_], result.flags, expected_f[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_shift_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_shift(NULL, 0, LEFT), ERR_BAD_PARAMETER);
    alu_output_t result = {0, 0};
    ck_assert_int_eq(alu_shift(&result, 0, (LEFT | RIGHT) + 1), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_shift_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    const uint8_t   input_x[] = {0x80, 0x80 };
    const rot_dir_t input_d[] = {LEFT, RIGHT};

    const uint16_t expected_v[] = {0x00, 0x40};
    const flags_t  expected_f[] = {0x90, 0x00};

    ASSERT_EQ_NB_EL(input_x, input_d);
    ASSERT_EQ_NB_EL(input_d, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);

    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_shift(&result, input_x[i_], input_d[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_shift() failed on 0x%" PRIX8 " (dir = 0x%" PRIX8 ") :got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_d[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_shift() failed on 0x%" PRIX8 " (dir = 0x%" PRIX8 ") : got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_d[i_], result.flags, expected_f[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_shiftRA_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_shiftR_A(NULL, 0), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_shiftRA_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    const uint8_t input_x[] = {0x80, 0x00};

    const uint16_t expected_v[] = {0xC0, 0x00};
    const flags_t  expected_f[] = {0x00, 0x80};

    ASSERT_EQ_NB_EL(input_x, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);

    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_shiftR_A(&result, input_x[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_shift() failed on 0x%" PRIX8 " :got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_shift() failed on 0x%" PRIX8 " : got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], result.flags, expected_f[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_rotate_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_rotate(NULL, 0, LEFT), ERR_BAD_PARAMETER);
    alu_output_t result = {0, 0};
    ck_assert_int_eq(alu_rotate(&result, 0, (LEFT | RIGHT) + 1), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_rotate_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    const uint8_t input_x[] = {0x80, 0x00};
    const uint8_t input_d[] = {LEFT, LEFT};

    const uint16_t expected_v[] = {0x01, 0x00};
    const flags_t  expected_f[] = {0x10, 0x80};

    ASSERT_EQ_NB_EL(input_x, input_d);
    ASSERT_EQ_NB_EL(input_d, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);

    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_rotate(&result, input_x[i_], input_d[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_shift() failed on 0x%" PRIX8 " (dir = 0x%" PRIX8 ") :got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_d[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_shift() failed on 0x%" PRIX8 " (dir = 0x%" PRIX8 ") : got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_d[i_], result.flags, expected_f[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_carryrotate_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_int_eq(alu_carry_rotate(NULL, 0, LEFT, 0), ERR_BAD_PARAMETER);
    alu_output_t result = {0, 0};
    ck_assert_int_eq(alu_carry_rotate(&result, 0, (LEFT | RIGHT) + 1, 0), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(alu_carryrotate_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    const uint8_t input_x[] = {0x80, 0x00, 0x01, 0x00};
    const uint8_t input_d[] = {LEFT, LEFT, RIGHT, LEFT};
    const uint8_t input_f[] = {0, 0x10, 0x00, 0x00};

    const uint16_t expected_v[] = {0x00, 0x01, 0x00, 0x00};
    const flags_t expected_f [] = {0x90, 0x00, 0x90, 0x80};

    ASSERT_EQ_NB_EL(input_x, input_d);
    ASSERT_EQ_NB_EL(input_d, input_f);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(expected_v, expected_f);

    LOOP_ON(input_x) {
        alu_output_t result = {0, 0};

        ck_assert_int_eq(alu_carry_rotate(&result, input_x[i_], input_d[i_], input_f[i_]), ERR_NONE);

        ck_assert_msg(result.value == expected_v[i_],
                      "alu_shift() failed on 0x%" PRIX8 " (dir = 0x%" PRIX8 ", flag = 0x%" PRIX8 ") :got value 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_d[i_], input_f[i_], result.value, expected_v[i_]);

        ck_assert_msg(result.flags == expected_f[i_],
                      "alu_shift() failed on 0x%" PRIX8 " (dir = 0x%" PRIX8 ", flag = 0x%" PRIX8 ") : got flag 0x%"
                      PRIX8 " instead of 0x%" PRIX8,
                      input_x[i_], input_d[i_], input_f[i_], result.flags, expected_f[i_]);
    }

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

// ================================================================================
Suite* bus_test_suite()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
    srand(time(NULL) ^ getpid() ^ pthread_self());
#pragma GCC diagnostic pop

    Suite* s = suite_create("alu.c tests");

    Add_Case(s, tc1, "flags tools tests");
    tcase_add_test(tc1, flags_get_exec);
    tcase_add_test(tc1, flags_set_exec);

    Add_Case(s, tc2, "ALU functions arguments tests");
    tcase_add_test(tc2, alu_add8_err);
    tcase_add_test(tc2, alu_add16_low_err);
    tcase_add_test(tc2, alu_add16_high_err);
    tcase_add_test(tc2, alu_sub8_err);
    tcase_add_test(tc2, alu_shift_err);
    tcase_add_test(tc2, alu_shiftRA_err);
    tcase_add_test(tc2, alu_rotate_err);
    tcase_add_test(tc2, alu_carryrotate_err);

    Add_Case(s, tc3, "ALU functions run tests");
    tcase_add_test(tc3, alu_add8_exec);
    tcase_add_test(tc3, alu_add16_low_exec);
    tcase_add_test(tc3, alu_add16_high_exec);
    tcase_add_test(tc3, alu_sub8_exec);
    tcase_add_test(tc3, alu_shift_exec);
    tcase_add_test(tc3, alu_shiftRA_exec);
    tcase_add_test(tc3, alu_rotate_exec);
    tcase_add_test(tc3, alu_carryrotate_exec);

    return s;
}

TEST_SUITE(bus_test_suite)
