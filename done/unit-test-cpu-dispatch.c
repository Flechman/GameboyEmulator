
// for thread-safe randomization
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <check.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>

#include "tests.h"
#include "error.h"
#include "alu.h"
#include "cpu.h"
#include "opcode.h"
#include "bit.h"
#include "util.h"

#include "unit-test-cpu-dispatch.h"

#include "cpu.c" // NOTICE: include cpu.c for testing static functions

// ------------------------------------------------------------

#define RUN_DISPATCH_ALU_A(input_a, input_f, expected_v, expected_f, ...) \
    do{ \
        INIT_RUN();\
        LOOP_ON(input_f) {\
            cpu.A = input_a[i_];\
            cpu.F = input_f[i_];\
            DO_RUN(cpu,__VA_ARGS__);\
        ck_assert_msg(cpu.A == expected_v[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => value 0x%" PRIX8 " (!= 0x%" PRIX8 ")",\
            lu.opcode, i_, input_a[i_],cpu.A, expected_v[i_]);\
        ck_assert_msg(cpu.F == expected_f[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => flag 0x%" PRIX8 " (!= 0x%" PRIX8 ")",\
            lu.opcode, i_, input_a[i_],cpu.F, expected_f[i_]);\
        }\
        END_RUN();\
    } while(0)

#define RUN_DISPATCH_ALU_HL_S(input_a,  input_f, expected_v, expected_f, ...) \
    do{ \
        INIT_RUN();\
        LOOP_ON(input_f) {\
            cpu.F = input_f[i_];\
            cpu_HL_set(&cpu,0);\
            ck_assert_int_eq(cpu_write_at_HL(&cpu, input_a[i_]), ERR_NONE);\
            DO_RUN(cpu,__VA_ARGS__);\
        ck_assert_msg(cpu_read_at_HL(&cpu) == lsb8(expected_v[i_]),"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => value 0x%" PRIX8 " (!= 0x%" PRIX8 ")",\
                lu.opcode, i_, input_a[i_], cpu_read_at_HL(&cpu), lsb8(expected_v[i_]));\
        ck_assert_msg(cpu.F == expected_f[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => flag 0x%" PRIX8 " (!= 0x%" PRIX8 ")",\
                lu.opcode, i_, input_a[i_], cpu.F, expected_f[i_]);\
        }\
        END_RUN();\
    } while(0)


//====================================================
//                  SUB
//====================================================

//SUB DATA
#define SUB_IN_DATA_A    {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F}
#define SUB_IN_DATA_B    {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x80, 0xFF, 0xF1, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x80, 0xFF, 0xF1}
#define SUB_IN_FLAG      {0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0}

#define SUB_OUT_DATA     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0xF2, 0xEE, 0x00, 0x00, 0x1E, 0x00, 0xFF, 0x01, 0xF2, 0xEE, 0x00, 0x00, 0x1E}
#define SUB_OUT_FLAG     {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x70, 0x40, 0x70, 0x40, 0xC0, 0xC0, 0x50, 0xC0, 0x70, 0x40, 0x70, 0x40, 0xC0, 0xC0, 0x50}
#define SUB_OUT_S_DATA   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define SUB_OUT_S_FLAG   {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0}

#define SUB_OUT_C_DATA   {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x01, 0xF2, 0xEE, 0x00, 0x00, 0x1E, 0xFF, 0xFE, 0x00, 0xF1, 0xED, 0xFF, 0xFF, 0x1D}
#define SUB_OUT_C_FLAG   {0xC0, 0x70, 0xC0, 0x70, 0xC0, 0x70, 0xC0, 0xC0, 0x70, 0xC0, 0x70, 0x40, 0x70, 0x40, 0xC0, 0xC0, 0x50, 0x70, 0x70, 0xC0, 0x70, 0x40, 0x70, 0x70, 0x50}
#define SUB_OUT_SC_DATA  {0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
#define SUB_OUT_SC_FLAG  {0xC0, 0x70, 0xC0, 0x70, 0xC0, 0x70, 0xC0, 0xC0, 0x70, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70}

#define DEC_IN_DATA    {0x00, 0x01, 0xFF, 0xFE, 0x10, 0x11, 0xdd, 0xbb, 0x0F, 0x00, 0xFF, 0xF0}
#define DEC_IN_FLAG    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0}
#define DEC_OUT_DATA   {0xFF, 0x00, 0xFE, 0xFD, 0x0F, 0x10, 0xdc, 0xba, 0x0E, 0xFF, 0xFE, 0xEF}
#define DEC_OUT_FLAG   {0x60, 0xC0, 0x40, 0x40, 0x60, 0x40, 0x40, 0x40, 0x40, 0x70, 0x50, 0x70}

#define DEC_IN_DATA16  {0x1000, 0x0000, 0xFFFF, 0xFFF0, 0x0001, 0xF000, 0xdead, 0xB011, 0xB005, 0x0000, 0xFFFF}
#define DEC_IN_FLAG16  {  0x00,   0x00,   0x00,   0x00,   0x80,   0x00,   0x00,   0x00,   0x00,   0xF0,   0xF0}
#define DEC_OUT_DATA16 {0x0FFF, 0xFFFF, 0xFFFE, 0xFFEF, 0x0000, 0xEFFF, 0xdeac, 0xB010, 0xB004, 0xFFFF, 0xFFFE}
#define DEC_OUT_FLAG16 {  0x00,   0x00,   0x00,   0x00,   0x80,   0x00,   0x00,   0x00,   0x00,   0xF0,   0xF0}

START_TEST(test_SUB_A_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = SUB_IN_FLAG;
    const uint8_t input_a[] = SUB_IN_DATA_A;
    const uint8_t input_b[] = SUB_IN_DATA_B;

    const uint8_t expected_v[] = SUB_OUT_DATA;
    const uint8_t expected_f[] = SUB_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_HL(input_a, input_b, input_f, expected_v, expected_f, OP_SUB_A_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_SBC_A_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = SUB_IN_FLAG;
    const uint8_t input_a[] = SUB_IN_DATA_A;
    const uint8_t input_b[] = SUB_IN_DATA_B;

    const uint8_t expected_v[] = SUB_OUT_C_DATA;
    const uint8_t expected_f[] = SUB_OUT_C_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_HL(input_a, input_b, input_f, expected_v, expected_f, OP_SBC_A_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_SUB_A_N8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = SUB_IN_FLAG;
    const uint8_t input_a[] = SUB_IN_DATA_A;
    const uint8_t input_b[] = SUB_IN_DATA_B;

    const uint8_t expected_v[] = SUB_OUT_DATA;
    const uint8_t expected_f[] = SUB_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_N8(input_a, input_b, input_f, expected_v, expected_f, OP_SUB_A_N8);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_SBC_A_N8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = SUB_IN_FLAG;
    const uint8_t input_a[] = SUB_IN_DATA_A;
    const uint8_t input_b[] = SUB_IN_DATA_B;

    const uint8_t expected_v[] = SUB_OUT_C_DATA;
    const uint8_t expected_f[] = SUB_OUT_C_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_N8(input_a, input_b, input_f, expected_v, expected_f, OP_SBC_A_N8);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_SUB_A_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = SUB_IN_FLAG;
    const uint8_t input_a[] = SUB_IN_DATA_A;
    const uint8_t input_b[] = SUB_IN_DATA_B;

    const uint8_t expected_v[] = SUB_OUT_DATA;
    const uint8_t expected_f[] = SUB_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_B_CODE, OP_SUB_A_B);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_C_CODE, OP_SUB_A_C);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_D_CODE, OP_SUB_A_D);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_E_CODE, OP_SUB_A_E);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_H_CODE, OP_SUB_A_H);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_L_CODE, OP_SUB_A_L);

    //Special Case for A+A !
    const uint8_t expected_va[] = SUB_OUT_S_DATA;
    const uint8_t expected_fa[] = SUB_OUT_S_FLAG;
    ASSERT_EQ_NB_EL(input_f, expected_va);
    ASSERT_EQ_NB_EL(input_f, expected_fa);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_va, expected_fa, REG_A_CODE, OP_SUB_A_A);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_SBC_A_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = SUB_IN_FLAG;
    const uint8_t input_a[] = SUB_IN_DATA_A;
    const uint8_t input_b[] = SUB_IN_DATA_B;

    const uint8_t expected_v[] = SUB_OUT_C_DATA;
    const uint8_t expected_f[] = SUB_OUT_C_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_B_CODE, OP_SBC_A_B);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_C_CODE, OP_SBC_A_C);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_D_CODE, OP_SBC_A_D);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_E_CODE, OP_SBC_A_E);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_H_CODE, OP_SBC_A_H);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_L_CODE, OP_SBC_A_L);

    //Special Case for A+A !
    const uint8_t expected_va[] = SUB_OUT_SC_DATA;
    const uint8_t expected_fa[] = SUB_OUT_SC_FLAG;

    ASSERT_EQ_NB_EL(input_f, expected_va);
    ASSERT_EQ_NB_EL(input_f, expected_fa);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_va, expected_fa, REG_A_CODE, OP_SBC_A_A);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_DEC8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f8[] = DEC_IN_FLAG;
    const uint8_t input_a8[] = DEC_IN_DATA;
    const uint8_t expected_v8[] = DEC_OUT_DATA;
    const uint8_t expected_f8[] = DEC_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f8, input_a8);
    ASSERT_EQ_NB_EL(input_f8, expected_v8);
    ASSERT_EQ_NB_EL(input_f8, expected_f8);

    RUN_DISPATCH_ALU_R8_S(input_a8, input_f8, expected_v8, expected_f8, REG_A_CODE, OP_DEC_A);
    RUN_DISPATCH_ALU_R8_S(input_a8, input_f8, expected_v8, expected_f8, REG_B_CODE, OP_DEC_B);
    RUN_DISPATCH_ALU_R8_S(input_a8, input_f8, expected_v8, expected_f8, REG_C_CODE, OP_DEC_C);
    RUN_DISPATCH_ALU_R8_S(input_a8, input_f8, expected_v8, expected_f8, REG_D_CODE, OP_DEC_D);
    RUN_DISPATCH_ALU_R8_S(input_a8, input_f8, expected_v8, expected_f8, REG_E_CODE, OP_DEC_E);
    RUN_DISPATCH_ALU_R8_S(input_a8, input_f8, expected_v8, expected_f8, REG_H_CODE, OP_DEC_H);
    RUN_DISPATCH_ALU_R8_S(input_a8, input_f8, expected_v8, expected_f8, REG_L_CODE, OP_DEC_L);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_DEC16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint16_t input_a16[] = DEC_IN_DATA16;
    const uint8_t input_f16[] = DEC_IN_FLAG16;
    const uint16_t expected_v16[] = DEC_OUT_DATA16;
    const uint8_t expected_f16[] = DEC_OUT_FLAG16;

    ASSERT_EQ_NB_EL(input_f16, input_a16);
    ASSERT_EQ_NB_EL(input_f16, expected_v16);
    ASSERT_EQ_NB_EL(input_f16, expected_f16);

    RUN_DISPATCH_ALU_R16_S(input_a16, input_f16, expected_v16, expected_f16, REG_BC_CODE, OP_DEC_BC);
    RUN_DISPATCH_ALU_R16_S(input_a16, input_f16, expected_v16, expected_f16, REG_DE_CODE, OP_DEC_DE);
    RUN_DISPATCH_ALU_R16_S(input_a16, input_f16, expected_v16, expected_f16, REG_AF_CODE, OP_DEC_SP);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


//====================================================
//                  AND
//====================================================

//AND DATA
#define AND_IN_DATA_A    {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0xFF}
#define AND_IN_DATA_B    {0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x03, 0x08, 0x0F, 0x00, 0xFF}
#define AND_IN_FLAG      {0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x10, 0xF0, 0xF0}

#define AND_OUT_DATA     {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x00, 0x08, 0x08, 0x00, 0xFF}
#define AND_OUT_FLAG     {0xA0, 0xA0, 0x20, 0x20, 0x20, 0x20, 0xA0, 0x20, 0x20, 0xA0, 0x20}
#define AND_OUT_S_DATA   {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0xFF}
#define AND_OUT_S_FLAG   {0xA0, 0xA0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xA0, 0x20}


START_TEST(test_AND_A_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = AND_IN_FLAG;
    const uint8_t input_a[] = AND_IN_DATA_A;
    const uint8_t input_b[] = AND_IN_DATA_B;

    const uint8_t expected_v[] = AND_OUT_DATA;
    const uint8_t expected_f[] = AND_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_HL(input_a, input_b, input_f, expected_v, expected_f, OP_AND_A_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_AND_A_N8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = AND_IN_FLAG;
    const uint8_t input_a[] = AND_IN_DATA_A;
    const uint8_t input_b[] = AND_IN_DATA_B;

    const uint8_t expected_v[] = AND_OUT_DATA;
    const uint8_t expected_f[] = AND_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_N8(input_a, input_b, input_f, expected_v, expected_f, OP_AND_A_N8);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_AND_A_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = AND_IN_FLAG;
    const uint8_t input_a[] = AND_IN_DATA_A;
    const uint8_t input_b[] = AND_IN_DATA_B;

    const uint8_t expected_v[] = AND_OUT_DATA;
    const uint8_t expected_f[] = AND_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_B_CODE, OP_AND_A_B);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_C_CODE, OP_AND_A_C);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_D_CODE, OP_AND_A_D);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_E_CODE, OP_AND_A_E);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_H_CODE, OP_AND_A_H);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_L_CODE, OP_AND_A_L);

    //Special Case for A+A !
    const uint8_t expected_va[] = AND_OUT_S_DATA;
    const uint8_t expected_fa[] = AND_OUT_S_FLAG;
    ASSERT_EQ_NB_EL(input_f, expected_va);
    ASSERT_EQ_NB_EL(input_f, expected_fa);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_va, expected_fa, REG_A_CODE, OP_AND_A_A);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST



//====================================================
//                  OR
//====================================================


//OR DATA
#define OR_IN_DATA_A    {0x00, 0x0F, 0x00, 0x03, 0x00, 0xFF, 0x00}
#define OR_IN_DATA_B    {0x00, 0x00, 0x0F, 0x0C, 0x00, 0xFF, 0xFF}
#define OR_IN_FLAG      {0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0}

#define OR_OUT_DATA     {0x00, 0x0F, 0x0F, 0x0F, 0x00, 0xFF, 0xFF}
#define OR_OUT_FLAG     {0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00}
#define OR_OUT_S_DATA   {0x00, 0x0F, 0x00, 0x03, 0x00, 0xFF, 0x00}
#define OR_OUT_S_FLAG   {0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80}


START_TEST(test_OR_A_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = OR_IN_FLAG;
    const uint8_t input_a[] = OR_IN_DATA_A;
    const uint8_t input_b[] = OR_IN_DATA_B;

    const uint8_t expected_v[] = OR_OUT_DATA;
    const uint8_t expected_f[] = OR_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_HL(input_a, input_b, input_f, expected_v, expected_f, OP_OR_A_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_OR_A_N8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = OR_IN_FLAG;
    const uint8_t input_a[] = OR_IN_DATA_A;
    const uint8_t input_b[] = OR_IN_DATA_B;

    const uint8_t expected_v[] = OR_OUT_DATA;
    const uint8_t expected_f[] = OR_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_N8(input_a, input_b, input_f, expected_v, expected_f, OP_OR_A_N8);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_OR_A_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = OR_IN_FLAG;
    const uint8_t input_a[] = OR_IN_DATA_A;
    const uint8_t input_b[] = OR_IN_DATA_B;

    const uint8_t expected_v[] = OR_OUT_DATA;
    const uint8_t expected_f[] = OR_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_B_CODE, OP_OR_A_B);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_C_CODE, OP_OR_A_C);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_D_CODE, OP_OR_A_D);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_E_CODE, OP_OR_A_E);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_H_CODE, OP_OR_A_H);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_L_CODE, OP_OR_A_L);

    //Special Case for A+A !
    const uint8_t expected_va[] = OR_OUT_S_DATA;
    const uint8_t expected_fa[] = OR_OUT_S_FLAG;
    ASSERT_EQ_NB_EL(input_f, expected_va);
    ASSERT_EQ_NB_EL(input_f, expected_fa);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_va, expected_fa, REG_A_CODE, OP_OR_A_A);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


//====================================================
//                  XOR
//====================================================

//XOR DATA
#define XOR_IN_DATA_A    {0x00, 0x0F, 0x00, 0x03, 0x00, 0xFF, 0x00, 0xFF, 0x00}
#define XOR_IN_DATA_B    {0x00, 0x00, 0x0F, 0x0C, 0xFF, 0xFF, 0x00, 0xFF, 0xFF}
#define XOR_IN_FLAG      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0}

#define XOR_OUT_DATA     {0x00, 0x0F, 0x0F, 0x0F, 0xFF, 0x00, 0x00, 0x00, 0xFF}
#define XOR_OUT_FLAG     {0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x00}
#define XOR_OUT_S_DATA   {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define XOR_OUT_S_FLAG   {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80}


START_TEST(test_XOR_A_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = XOR_IN_DATA_A;
    const uint8_t input_b[] = XOR_IN_DATA_B;
    const uint8_t input_f[] = XOR_IN_FLAG;

    const uint8_t expected_v[] = XOR_OUT_DATA;
    const uint8_t expected_f[] = XOR_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_HL(input_a, input_b, input_f, expected_v, expected_f, OP_XOR_A_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_XOR_A_N8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = XOR_IN_DATA_A;
    const uint8_t input_b[] = XOR_IN_DATA_B;
    const uint8_t input_f[] = XOR_IN_FLAG;

    const uint8_t expected_v[] = XOR_OUT_DATA;
    const uint8_t expected_f[] = XOR_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_N8(input_a, input_b, input_f, expected_v, expected_f, OP_XOR_A_N8);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_XOR_A_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = XOR_IN_DATA_A;
    const uint8_t input_b[] = XOR_IN_DATA_B;
    const uint8_t input_f[] = XOR_IN_FLAG;

    const uint8_t expected_v[] = XOR_OUT_DATA;
    const uint8_t expected_f[] = XOR_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_B_CODE, OP_XOR_A_B);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_C_CODE, OP_XOR_A_C);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_D_CODE, OP_XOR_A_D);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_E_CODE, OP_XOR_A_E);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_H_CODE, OP_XOR_A_H);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_L_CODE, OP_XOR_A_L);

    //Special Case for A+A !
    const uint8_t expected_va[] = XOR_OUT_S_DATA;
    const uint8_t expected_fa[] = XOR_OUT_S_FLAG;
    ASSERT_EQ_NB_EL(input_f, expected_va);
    ASSERT_EQ_NB_EL(input_f, expected_fa);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_va, expected_fa, REG_A_CODE, OP_XOR_A_A);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


//================================================================
//                  ROT
//================================================================

//ROT DATA
#define ROT_IN_DATA      {0x00, 0x00, 0x0F, 0xFF, 0x03, 0x00, 0xFF, 0xFE, 0x01, 0x80, 0x00, 0x01, 0x10, 0x80, 0x08}
#define ROT_IN_FLAG      {0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0}

#define ROT_OUT_L_DATA   {0x00, 0x01, 0x1E, 0xFE, 0x06, 0x00, 0xFF, 0xFC, 0x02, 0x00, 0x01, 0x03, 0x21, 0x01, 0x11}
#define ROT_OUT_L_FLAG   {0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00}
#define ROT_OUT_LZ_FLAG  {0x80, 0x00, 0x00, 0x10, 0x00, 0x80, 0x10, 0x10, 0x00, 0x90, 0x00, 0x00, 0x00, 0x10, 0x00}

#define ROT_OUT_R_DATA   {0x00, 0x80, 0x07, 0x7F, 0x01, 0x00, 0xFF, 0x7F, 0x00, 0x40, 0x80, 0x80, 0x88, 0xC0, 0x84}
#define ROT_OUT_R_FLAG   {0x00, 0x00, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00}
#define ROT_OUT_RZ_FLAG  {0x80, 0x00, 0x10, 0x10, 0x10, 0x80, 0x10, 0x00, 0x90, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00}

#define ROT_OUT_LC_DATA  {0x00, 0x00, 0x1E, 0xFF, 0x06, 0x00, 0xFF, 0xFD, 0x02, 0x01, 0x00, 0x02, 0x20, 0x01, 0x10}
#define ROT_OUT_LC_FLAG  {0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00}
#define ROT_OUT_LCZ_FLAG {0x80, 0x80, 0x00, 0x10, 0x00, 0x80, 0x10, 0x10, 0x00, 0x10, 0x80, 0x00, 0x00, 0x10, 0x00}

#define ROT_OUT_RC_DATA  {0x00, 0x00, 0x87, 0xFF, 0x81, 0x00, 0xFF, 0x7F, 0x80, 0x40, 0x00, 0x80, 0x08, 0x40, 0x04}
#define ROT_OUT_RC_FLAG  {0x00, 0x00, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00}
#define ROT_OUT_RCZ_FLAG {0x80, 0x80, 0x10, 0x10, 0x10, 0x80, 0x10, 0x00, 0x10, 0x00, 0x80, 0x10, 0x00, 0x00, 0x00}



START_TEST(test_ROTA)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = ROT_IN_DATA;
    const uint8_t input_f[] = ROT_IN_FLAG;

    const uint8_t expected_vl[] = ROT_OUT_L_DATA;
    const uint8_t expected_fl[] = ROT_OUT_L_FLAG;

    const uint8_t expected_vr[] = ROT_OUT_R_DATA;
    const uint8_t expected_fr[] = ROT_OUT_R_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_vl);
    ASSERT_EQ_NB_EL(input_f, expected_vr);
    ASSERT_EQ_NB_EL(input_f, expected_fl);
    ASSERT_EQ_NB_EL(input_f, expected_fr);

    RUN_DISPATCH_ALU_A(input_a, input_f, expected_vl, expected_fl, OP_RLA);
    RUN_DISPATCH_ALU_A(input_a, input_f, expected_vr, expected_fr, OP_RRA);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_ROTCA)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = ROT_IN_DATA;
    const uint8_t input_f[] = ROT_IN_FLAG;

    const uint8_t expected_vl[] = ROT_OUT_LC_DATA;
    const uint8_t expected_fl[] = ROT_OUT_LC_FLAG;

    const uint8_t expected_vr[] = ROT_OUT_RC_DATA;
    const uint8_t expected_fr[] = ROT_OUT_RC_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_vl);
    ASSERT_EQ_NB_EL(input_f, expected_fl);
    ASSERT_EQ_NB_EL(input_f, expected_vr);
    ASSERT_EQ_NB_EL(input_f, expected_fr);

    RUN_DISPATCH_ALU_A(input_a, input_f, expected_vl, expected_fl, OP_RLCA);
    RUN_DISPATCH_ALU_A(input_a, input_f, expected_vr, expected_fr, OP_RRCA);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_ROT_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = ROT_IN_DATA;
    const uint8_t input_f[] = ROT_IN_FLAG;

    const uint8_t expected_vl[] = ROT_OUT_L_DATA;
    const uint8_t expected_fl[] = ROT_OUT_LZ_FLAG;

    const uint8_t expected_vr[] = ROT_OUT_R_DATA;
    const uint8_t expected_fr[] = ROT_OUT_RZ_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_vl);
    ASSERT_EQ_NB_EL(input_f, expected_fl);
    ASSERT_EQ_NB_EL(input_f, expected_vr);
    ASSERT_EQ_NB_EL(input_f, expected_fr);

    RUN_DISPATCH_ALU_HL_S(input_a, input_f, expected_vl, expected_fl, OP_RL_HLR);
    RUN_DISPATCH_ALU_HL_S(input_a, input_f, expected_vr, expected_fr, OP_RR_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_ROTC_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = ROT_IN_DATA;
    const uint8_t input_f[] = ROT_IN_FLAG;

    const uint8_t expected_vl[] = ROT_OUT_LC_DATA;
    const uint8_t expected_fl[] = ROT_OUT_LCZ_FLAG;

    const uint8_t expected_vr[] = ROT_OUT_RC_DATA;
    const uint8_t expected_fr[] = ROT_OUT_RCZ_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_vl);
    ASSERT_EQ_NB_EL(input_f, expected_fl);
    ASSERT_EQ_NB_EL(input_f, expected_vr);
    ASSERT_EQ_NB_EL(input_f, expected_fr);

    RUN_DISPATCH_ALU_HL_S(input_a, input_f, expected_vl, expected_fl, OP_RLC_HLR);
    RUN_DISPATCH_ALU_HL_S(input_a, input_f, expected_vr, expected_fr, OP_RRC_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_ROTC_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = ROT_IN_DATA;
    const uint8_t input_f[] = ROT_IN_FLAG;

    const uint8_t expected_vl[] = ROT_OUT_LC_DATA;
    const uint8_t expected_fl[] = ROT_OUT_LCZ_FLAG;

    const uint8_t expected_vr[] = ROT_OUT_RC_DATA;
    const uint8_t expected_fr[] = ROT_OUT_RCZ_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_vl);
    ASSERT_EQ_NB_EL(input_f, expected_fl);
    ASSERT_EQ_NB_EL(input_f, expected_vr);
    ASSERT_EQ_NB_EL(input_f, expected_fr);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_A_CODE, OP_RLC_A);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_B_CODE, OP_RLC_B);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_C_CODE, OP_RLC_C);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_D_CODE, OP_RLC_D);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_E_CODE, OP_RLC_E);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_H_CODE, OP_RLC_H);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_L_CODE, OP_RLC_L);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_A_CODE, OP_RRC_A);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_B_CODE, OP_RRC_B);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_C_CODE, OP_RRC_C);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_D_CODE, OP_RRC_D);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_E_CODE, OP_RRC_E);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_H_CODE, OP_RRC_H);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_L_CODE, OP_RRC_L);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


//================================================================
//                  SHIFT
//================================================================

//SHFT DATA
#define SHFT_IN_DATA      {0x00, 0x00, 0x0F, 0xFF, 0x03, 0x00, 0xFF, 0xFE, 0x01, 0x02, 0x10, 0x80, 0x00, 0x01, 0x10, 0x08, 0x80}
#define SHFT_IN_FLAG      {0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0}

#define SHFT_OUT_L_DATA   {0x00, 0x00, 0x1E, 0xFE, 0x06, 0x00, 0xFE, 0xFC, 0x02, 0x04, 0x20, 0x00, 0x00, 0x02, 0x20, 0x10, 0x00}
#define SHFT_OUT_L_FLAG   {0x80, 0x80, 0x00, 0x10, 0x00, 0x80, 0x10, 0x10, 0x00, 0x00, 0x00, 0x90, 0x80, 0x00, 0x00, 0x00, 0x90}

#define SHFT_OUT_R_DATA   {0x00, 0x00, 0x07, 0xFF, 0x01, 0x00, 0xFF, 0xFF, 0x00, 0x01, 0x08, 0xC0, 0x00, 0x00, 0x08, 0x04, 0xC0}
#define SHFT_OUT_R_FLAG   {0x80, 0x80, 0x10, 0x10, 0x10, 0x80, 0x10, 0x00, 0x90, 0x00, 0x00, 0x00, 0x80, 0x90, 0x00, 0x00, 0x00}

#define SHFT_OUT_RL_DATA  {0x00, 0x00, 0x07, 0x7F, 0x01, 0x00, 0x7F, 0x7F, 0x00, 0x01, 0x08, 0x40, 0x00, 0x00, 0x08, 0x04, 0x40}
#define SHFT_OUT_RL_FLAG  {0x80, 0x80, 0x10, 0x10, 0x10, 0x80, 0x10, 0x00, 0x90, 0x00, 0x00, 0x00, 0x80, 0x90, 0x00, 0x00, 0x00}



START_TEST(test_SXA_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = SHFT_IN_DATA;
    const uint8_t input_f[] = SHFT_IN_FLAG;

    const uint8_t expected_vl[] = SHFT_OUT_L_DATA;
    const uint8_t expected_fl[] = SHFT_OUT_L_FLAG;

    const uint8_t expected_vr[] = SHFT_OUT_R_DATA;
    const uint8_t expected_fr[] = SHFT_OUT_R_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_vl);
    ASSERT_EQ_NB_EL(input_f, expected_vr);
    ASSERT_EQ_NB_EL(input_f, expected_fl);
    ASSERT_EQ_NB_EL(input_f, expected_fr);

    RUN_DISPATCH_ALU_HL_S(input_a, input_f, expected_vl, expected_fl, OP_SLA_HLR);
    RUN_DISPATCH_ALU_HL_S(input_a, input_f, expected_vr, expected_fr, OP_SRA_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_SXA_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = SHFT_IN_DATA;
    const uint8_t input_f[] = SHFT_IN_FLAG;

    const uint8_t expected_vl[] = SHFT_OUT_L_DATA;
    const uint8_t expected_fl[] = SHFT_OUT_L_FLAG;

    const uint8_t expected_vr[] = SHFT_OUT_R_DATA;
    const uint8_t expected_fr[] = SHFT_OUT_R_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_vl);
    ASSERT_EQ_NB_EL(input_f, expected_vr);
    ASSERT_EQ_NB_EL(input_f, expected_fl);
    ASSERT_EQ_NB_EL(input_f, expected_fr);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_A_CODE, OP_SLA_A);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_B_CODE, OP_SLA_B);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_C_CODE, OP_SLA_C);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_D_CODE, OP_SLA_D);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_E_CODE, OP_SLA_E);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_H_CODE, OP_SLA_H);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_L_CODE, OP_SLA_L);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_A_CODE, OP_SRA_A);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_B_CODE, OP_SRA_B);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_C_CODE, OP_SRA_C);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_D_CODE, OP_SRA_D);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_E_CODE, OP_SRA_E);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_H_CODE, OP_SRA_H);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_L_CODE, OP_SRA_L);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_SRL_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = SHFT_IN_DATA;
    const uint8_t input_f[] = SHFT_IN_FLAG;

    const uint8_t expected_v[] = SHFT_OUT_RL_DATA;
    const uint8_t expected_f[] = SHFT_OUT_RL_FLAG;


    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_HL_S(input_a, input_f, expected_v, expected_f, OP_SRL_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_SRL_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = SHFT_IN_DATA;
    const uint8_t input_f[] = SHFT_IN_FLAG;

    const uint8_t expected_v[] = SHFT_OUT_RL_DATA;
    const uint8_t expected_f[] = SHFT_OUT_RL_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_A_CODE, OP_SRL_A);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_B_CODE, OP_SRL_B);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_C_CODE, OP_SRL_C);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_D_CODE, OP_SRL_D);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_E_CODE, OP_SRL_E);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_H_CODE, OP_SRL_H);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_L_CODE, OP_SRL_L);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//================================================================
//                  CMP
//================================================================

#define CP_IN_DATA_A    {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F}
#define CP_IN_DATA_B    {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x80, 0xFF, 0xF1, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x80, 0xFF, 0xF1}
#define CP_IN_FLAG      {0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0}

#define CP_OUT_DATA     {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F}
#define CP_OUT_FLAG     {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x70, 0x40, 0x70, 0x40, 0xC0, 0xC0, 0x50, 0xC0, 0x70, 0x40, 0x70, 0x40, 0xC0, 0xC0, 0x50}
#define CP_OUT_S_FLAG   {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0}

START_TEST(test_CP_A_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = CP_IN_FLAG;
    const uint8_t input_a[] = CP_IN_DATA_A;
    const uint8_t input_b[] = CP_IN_DATA_B;

    const uint8_t expected_v[] = CP_OUT_DATA;
    const uint8_t expected_f[] = CP_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_HL(input_a, input_b, input_f, expected_v, expected_f, OP_CP_A_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_CP_A_N8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = CP_IN_FLAG;
    const uint8_t input_a[] = CP_IN_DATA_A;
    const uint8_t input_b[] = CP_IN_DATA_B;

    const uint8_t expected_v[] = CP_OUT_DATA;
    const uint8_t expected_f[] = CP_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_N8(input_a, input_b, input_f, expected_v, expected_f, OP_CP_A_N8);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//================================================================
//                  ALU-SWAP
//================================================================

#define SWAP_IN_DATA_A    {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0x01, 0x05, 0xEF, 0x80, 0xFF, 0x0F, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F}
#define SWAP_IN_FLAG      {0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0}

#define SWAP_OUT_DATA     {0x00, 0x00, 0x10, 0x10, 0x20, 0x20, 0x40, 0x80, 0x80, 0x00, 0x10, 0x50, 0xFE, 0x08, 0xFF, 0xF0, 0x00, 0x00, 0x10, 0x10, 0xFE, 0x08, 0xFF, 0xF0}
#define SWAP_OUT_FLAG     {0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
#define SWAP_OUT_S_FLAG   {0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}


START_TEST(test_SWAP_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = SWAP_IN_FLAG;
    const uint8_t input_a[] = SWAP_IN_DATA_A;

    const uint8_t expected_v[] = SWAP_OUT_DATA;
    const uint8_t expected_f[] = SWAP_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_HL_S(input_a,  input_f, expected_v, expected_f, OP_SWAP_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_SWAP_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = SWAP_IN_FLAG;
    const uint8_t input_a[] = SWAP_IN_DATA_A;

    const uint8_t expected_v[] = SWAP_OUT_DATA;
    const uint8_t expected_f[] = SWAP_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R8_S(input_a,  input_f, expected_v, expected_f, REG_A_CODE, OP_SWAP_A);
    RUN_DISPATCH_ALU_R8_S(input_a,  input_f, expected_v, expected_f, REG_B_CODE, OP_SWAP_B);
    RUN_DISPATCH_ALU_R8_S(input_a,  input_f, expected_v, expected_f, REG_C_CODE, OP_SWAP_C);
    RUN_DISPATCH_ALU_R8_S(input_a,  input_f, expected_v, expected_f, REG_D_CODE, OP_SWAP_D);
    RUN_DISPATCH_ALU_R8_S(input_a,  input_f, expected_v, expected_f, REG_E_CODE, OP_SWAP_E);
    RUN_DISPATCH_ALU_R8_S(input_a,  input_f, expected_v, expected_f, REG_H_CODE, OP_SWAP_H);
    RUN_DISPATCH_ALU_R8_S(input_a,  input_f, expected_v, expected_f, REG_L_CODE, OP_SWAP_L);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//================================================================
//                  ALU-BIT-/CHG
//================================================================
#define BITOP_HL_HELPER(cpu, expected_f, expected_v,...)\
    do{\
        cpu_write_at_HL(&cpu,input_a[i_]);\
        cpu.F = input_f[i_];\
        DO_RUN(cpu, __VA_ARGS__);\
        ck_assert_msg(cpu_read_at_HL(&cpu) == expected_v[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => value 0x%" PRIX8 " (!= 0x%" PRIX8 ")",\
                lu.opcode, i_, input_a[i_], cpu_read_at_HL(&cpu), expected_v[i_]);\
        ck_assert_msg(cpu.F == expected_f[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => flag 0x%" PRIX8 " (!= 0x%" PRIX8 ")",\
                lu.opcode, i_, input_a[i_], cpu.F, expected_f[i_]);\
    }while(0)

#define BITOP_R8_HELPER(cpu, expected_f, expected_v, reg,...)\
    do{\
        cpu_reg_set(&cpu, reg, input_a[i_]);\
        cpu.F = input_f[i_];\
        DO_RUN(cpu, __VA_ARGS__);\
        ck_assert_msg(cpu_reg_get(&cpu, reg) == expected_v[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => value 0x%" PRIX8 " (!= 0x%" PRIX8 ")",\
                lu.opcode, i_, input_a[i_], cpu_reg_get(&cpu, reg), input_a[i_]);\
        ck_assert_msg(cpu.F == expected_f[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => flag 0x%" PRIX8 " (!= 0x%" PRIX8 ")",\
                lu.opcode, i_, input_a[i_], cpu.F, expected_f[i_]);\
    }while(0)


START_TEST(test_BIT_U3_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = BIT_IN_FLAG;
    const uint8_t input_a[] = BIT_IN_DATA;
    const uint8_t expected_f_0[] = BIT_OUT_FLAG_0;
    const uint8_t expected_f_1[] = BIT_OUT_FLAG_1;
    const uint8_t expected_f_2[] = BIT_OUT_FLAG_2;
    const uint8_t expected_f_3[] = BIT_OUT_FLAG_3;
    const uint8_t expected_f_4[] = BIT_OUT_FLAG_4;
    const uint8_t expected_f_5[] = BIT_OUT_FLAG_5;
    const uint8_t expected_f_6[] = BIT_OUT_FLAG_6;
    const uint8_t expected_f_7[] = BIT_OUT_FLAG_7;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_f_0);
    ASSERT_EQ_NB_EL(input_f, expected_f_1);
    ASSERT_EQ_NB_EL(input_f, expected_f_2);
    ASSERT_EQ_NB_EL(input_f, expected_f_3);
    ASSERT_EQ_NB_EL(input_f, expected_f_4);
    ASSERT_EQ_NB_EL(input_f, expected_f_5);
    ASSERT_EQ_NB_EL(input_f, expected_f_6);
    ASSERT_EQ_NB_EL(input_f, expected_f_7);

    INIT_RUN();
    LOOP_ON(input_a) {
        BITOP_HL_HELPER(cpu,  expected_f_0, input_a, OP_BIT_0_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_1, input_a, OP_BIT_1_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_2, input_a, OP_BIT_2_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_3, input_a, OP_BIT_3_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_4, input_a, OP_BIT_4_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_5, input_a, OP_BIT_5_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_6, input_a, OP_BIT_6_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_7, input_a, OP_BIT_7_HLR);
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_CHG_U3_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = BIT_IN_FLAG;
    const uint8_t input_a[] = BIT_IN_DATA;
    const uint8_t expected_f_0[] = CHG_OUT_FLAG_0;
    const uint8_t expected_v_0[] = CHG_OUT_DATA_0;
    const uint8_t expected_f_1[] = CHG_OUT_FLAG_1;
    const uint8_t expected_v_1[] = CHG_OUT_DATA_1;
    const uint8_t expected_f_2[] = CHG_OUT_FLAG_2;
    const uint8_t expected_v_2[] = CHG_OUT_DATA_2;
    const uint8_t expected_f_3[] = CHG_OUT_FLAG_3;
    const uint8_t expected_v_3[] = CHG_OUT_DATA_3;
    const uint8_t expected_f_4[] = CHG_OUT_FLAG_4;
    const uint8_t expected_v_4[] = CHG_OUT_DATA_4;
    const uint8_t expected_f_5[] = CHG_OUT_FLAG_5;
    const uint8_t expected_v_5[] = CHG_OUT_DATA_5;
    const uint8_t expected_f_6[] = CHG_OUT_FLAG_6;
    const uint8_t expected_v_6[] = CHG_OUT_DATA_6;
    const uint8_t expected_f_7[] = CHG_OUT_FLAG_7;
    const uint8_t expected_v_7[] = CHG_OUT_DATA_7;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_f_0);
    ASSERT_EQ_NB_EL(input_f, expected_v_0);
    ASSERT_EQ_NB_EL(input_f, expected_f_1);
    ASSERT_EQ_NB_EL(input_f, expected_v_1);
    ASSERT_EQ_NB_EL(input_f, expected_f_2);
    ASSERT_EQ_NB_EL(input_f, expected_v_2);
    ASSERT_EQ_NB_EL(input_f, expected_f_3);
    ASSERT_EQ_NB_EL(input_f, expected_v_3);
    ASSERT_EQ_NB_EL(input_f, expected_f_4);
    ASSERT_EQ_NB_EL(input_f, expected_v_4);
    ASSERT_EQ_NB_EL(input_f, expected_f_5);
    ASSERT_EQ_NB_EL(input_f, expected_v_5);
    ASSERT_EQ_NB_EL(input_f, expected_f_6);
    ASSERT_EQ_NB_EL(input_f, expected_v_6);
    ASSERT_EQ_NB_EL(input_f, expected_f_7);
    ASSERT_EQ_NB_EL(input_f, expected_v_7);

    INIT_RUN();
    LOOP_ON(input_a) {
        BITOP_HL_HELPER(cpu,  expected_f_0, expected_v_0, OP_RES_0_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_1, expected_v_1, OP_RES_1_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_2, expected_v_2, OP_RES_2_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_3, expected_v_3, OP_RES_3_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_4, expected_v_4, OP_RES_4_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_5, expected_v_5, OP_RES_5_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_6, expected_v_6, OP_RES_6_HLR);
        BITOP_HL_HELPER(cpu,  expected_f_7, expected_v_7, OP_RES_7_HLR);
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//================================================================
//                  ALU-MISC
//================================================================

#define LD_HLSP_INPUT_DATA1    {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F}
#define LD_HLSP_INPUT_DATA2    {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x80, 0xFF, 0xF1, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x80, 0xFF, 0xF1}
#define LD_HLSP_INPUT_FLAG     {0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0}
#define LD_HLSP_OUTPUT_DATA1   {0x00, 0x00, 0x02, 0x02, 0x04, 0x04, 0x08, 0x10, 0x10, 0x00, 0x01, 0x01, 0x10, 0xF0, 0x00, 0xFE, 0x00, 0x00, 0x01, 0x01, 0x10, 0xF0, 0x00, 0xFE, 0x00}
#define LD_HLSP_OUTPUT_DATA2   {0x00, 0x00, 0x02, 0x02, 0x04, 0x04, 0x08, 0x10, 0x10, 0x00, 0x01, 0x01, 0x10, 0xF0, 0x00, 0xFE, 0x00, 0x00, 0x01, 0x01, 0x10, 0xF0, 0x00, 0xFE, 0x00}
#define LD_HLSP_OUTPUT_FLAG    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x00, 0x00, 0x00, 0x20, 0x20, 0x10, 0x30, 0x30, 0x00, 0x00, 0x00, 0x20, 0x20, 0x10, 0x30, 0x30}

#define DAA_INPUT_DATA  {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0xFF, 0x0F, 0xF0, 0xdd, 0x11, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0xFF, 0x0F, 0xF0, 0xdd, 0x11}
#define DAA_INPUT_FLAG  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0}
#define DAA_OUTPUT_DATA {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x65, 0x15, 0x50, 0x43, 0x11, 0x9A, 0x9B, 0x9C, 0x9E, 0xA2, 0xAA, 0xBA, 0xDA, 0x1A, 0x99, 0xA9, 0x8A, 0x77, 0xAB}
#define DAA_OUTPUT_FLAG {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x10, 0x10, 0x00, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50}

#define SCCF_INPUT_FLAG {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0}
#define CCF_OUTPUT_FLAG {0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x90, 0x80, 0x90, 0x80, 0x90, 0x80, 0x90, 0x80}
#define SCF_OUTPUT_FLAG {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90}


START_TEST(test_LD_HLSP_S8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = LD_HLSP_INPUT_DATA1;
    const uint8_t input_b[] = LD_HLSP_INPUT_DATA2;
    const uint8_t input_f[] = LD_HLSP_INPUT_FLAG;

    const uint8_t expected_v1[] = LD_HLSP_OUTPUT_DATA1;
    const uint8_t expected_v2[] = LD_HLSP_OUTPUT_DATA2;
    const uint8_t expected_f[] = LD_HLSP_OUTPUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v1);
    ASSERT_EQ_NB_EL(input_f, expected_v2);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    INIT_RUN();
    LOOP_ON(input_f) {
        cpu.SP = input_a[i_];
        cpu.F = input_f[i_];
        cpu.PC = 0;
        ck_assert_int_eq(cpu_write_at_idx(&cpu, (addr_t) (cpu.PC+1), input_b[i_]), ERR_NONE);
        DO_RUN(cpu,OP_LD_HL_SP_N8);
        ck_assert_msg(cpu.HL == expected_v1[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ", 0x%" PRIX8 ") => value 0x%" PRIX8 " (!= 0x%" PRIX8 ")",
                      lu.opcode, i_, input_a[i_], input_b[i_],cpu.HL, expected_v1[i_]);
        ck_assert_msg(cpu.F == expected_f[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ", 0x%" PRIX8 ") => flag 0x%" PRIX8 " (!= 0x%" PRIX8 ")",
                      lu.opcode, i_, input_a[i_], input_b[i_],cpu.F, expected_f[i_]);
    }

    LOOP_ON(input_f) {
        cpu.SP = input_a[i_];
        cpu.F = input_f[i_];
        cpu.PC = 0;
        ck_assert_int_eq(cpu_write_at_idx(&cpu, (addr_t) (cpu.PC+1), input_b[i_]), ERR_NONE);
        DO_RUN(cpu,OP_ADD_SP_N);
        ck_assert_msg(cpu.SP == expected_v2[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ", 0x%" PRIX8 ") => value 0x%" PRIX8 " (!= 0x%" PRIX8 ")",
                      lu.opcode, i_, input_a[i_], input_b[i_],cpu.SP, expected_v2[i_]);
        ck_assert_msg(cpu.F == expected_f[i_],"Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ", 0x%" PRIX8 ") => flag 0x%" PRIX8 " (!= 0x%" PRIX8 ")",
                      lu.opcode, i_, input_a[i_], input_b[i_],cpu.F, expected_f[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_DAA)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = DAA_INPUT_DATA;
    const uint8_t input_f[] = DAA_INPUT_FLAG;

    const uint8_t expected_v[] = DAA_OUTPUT_DATA;
    const uint8_t expected_f[] = DAA_OUTPUT_FLAG;


    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    INIT_RUN();
    LOOP_ON(input_a) {
        cpu.A = input_a[i_];
        cpu.F = input_f[i_];
        DO_RUN(cpu, OP_DAA);
        ck_assert_msg(cpu.A == expected_v[i_], "Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => value 0x%" PRIX8 " (!= 0x%" PRIX8 ")", \
                      lu.opcode, i_, input_a[i_], cpu.A, expected_v[i_]);
        ck_assert_msg(cpu.F == expected_f[i_], "Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => flag 0x%" PRIX8 " (!= 0x%" PRIX8 ")", \
                      lu.opcode, i_, input_a[i_], cpu.F, expected_f[i_]);
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_SCCF)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = SCCF_INPUT_FLAG;
    const uint8_t expected_sf[] = SCF_OUTPUT_FLAG;
    const uint8_t expected_cf[] = CCF_OUTPUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, expected_sf);
    ASSERT_EQ_NB_EL(input_f, expected_cf);

    INIT_RUN();
    LOOP_ON(input_f) {
        cpu.F = input_f[i_];
        DO_RUN(cpu, OP_SCF);
        ck_assert_msg(cpu.F == expected_sf[i_], "Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => flag 0x%" PRIX8 " (!= 0x%" PRIX8 ")", \
                      lu.opcode, i_, input_f[i_], cpu.F, expected_sf[i_]);
    }
    LOOP_ON(input_f) {
        cpu.F = input_f[i_];
        DO_RUN(cpu, OP_CCF);
        ck_assert_msg(cpu.F == expected_cf[i_], "Opcode 0x%" PRIX8 ", i=%d: (0x%" PRIX8 ") => flag 0x%" PRIX8 " (!= 0x%" PRIX8 ")", \
                      lu.opcode, i_, input_f[i_], cpu.F, expected_cf[i_]);
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//================================================================
//                  END END END
//================================================================

Suite* cpu_test_suite()
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
    srand(time(NULL) ^ getpid() ^ pthread_self());
#pragma GCC diagnostic pop

    Suite* s = suite_create("cpu.c Dispatch Tests");

    Add_Case(s, tcalu1, "Cpu Dispatch ALU SUB Tests");
    tcase_add_test(tcalu1, test_SUB_A_HLR);
    tcase_add_test(tcalu1, test_SBC_A_HLR);
    tcase_add_test(tcalu1, test_SUB_A_N8);
    tcase_add_test(tcalu1, test_SBC_A_N8);
    tcase_add_test(tcalu1, test_SUB_A_R8);
    tcase_add_test(tcalu1, test_SBC_A_R8);
    tcase_add_test(tcalu1, test_DEC8);
    tcase_add_test(tcalu1, test_DEC16);

    Add_Case(s, tcalu2, "Cpu Dispatch ALU AND Tests");
    tcase_add_test(tcalu2, test_AND_A_N8);
    tcase_add_test(tcalu2, test_AND_A_R8);
    tcase_add_test(tcalu2, test_AND_A_HLR);

    Add_Case(s, tcalu3, "Cpu Dispatch ALU OR Tests");
    tcase_add_test(tcalu3, test_OR_A_N8);
    tcase_add_test(tcalu3, test_OR_A_R8);
    tcase_add_test(tcalu3, test_OR_A_HLR);

    Add_Case(s, tcalu4, "Cpu Dispatch ALU XOR Tests");
    tcase_add_test(tcalu4, test_XOR_A_N8);
    tcase_add_test(tcalu4, test_XOR_A_R8);
    tcase_add_test(tcalu4, test_XOR_A_HLR);

    Add_Case(s, tcalu5, "Cpu Dispatch ALU ROT Tests");
    tcase_add_test(tcalu5, test_ROTA);
    tcase_add_test(tcalu5, test_ROTCA);
    tcase_add_test(tcalu5, test_ROT_HLR);
    tcase_add_test(tcalu5, test_ROTC_HLR);
    tcase_add_test(tcalu5, test_ROTC_R8);

    Add_Case(s, tcalu6, "Cpu Dispatch ALU SHIFT Tests");
    tcase_add_test(tcalu6, test_SXA_HLR);
    tcase_add_test(tcalu6, test_SXA_R8);
    tcase_add_test(tcalu6, test_SRL_HLR);
    tcase_add_test(tcalu6, test_SRL_R8);

    Add_Case(s, tcalu7, "Cpu Dispatch ALU CP Tests");
    tcase_add_test(tcalu7, test_CP_A_HLR);
    tcase_add_test(tcalu7, test_CP_A_N8);

    Add_Case(s, tcalu8, "Cpu Dispatch ALU SWAP Tests");
    tcase_add_test(tcalu8, test_SWAP_HLR);
    tcase_add_test(tcalu8, test_SWAP_R8);

    Add_Case(s, tcalu9, "Cpu Dispatch ALU BIT/CHG Tests");
    tcase_add_test(tcalu9, test_BIT_U3_HLR);
    tcase_add_test(tcalu9, test_CHG_U3_HLR);

    Add_Case(s, tcalu99, "Cpu Dispatch ALU MISC Tests");
    tcase_add_test(tcalu99, test_LD_HLSP_S8);
    tcase_add_test(tcalu99, test_DAA);
    tcase_add_test(tcalu99, test_SCCF);

    return s;
}
TEST_SUITE(cpu_test_suite)
