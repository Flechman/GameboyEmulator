

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
#include "gameboy.h" // REGISTERS_START
#include "util.h"

#include "unit-test-cpu-dispatch.h"

#include "cpu.c" // NOTICE: include cpu.c for testing static functions

// ------------------------------------------------------------

#define RUN_DISPATCH_ALU_R16(input_a, input_b, input_f, expected_v, expected_f, reg_k, ...) \
    do{ \
        INIT_RUN();\
        LOOP_ON(input_f) {\
            cpu_HL_set(&cpu,input_a[i_]);\
            cpu.F = input_f[i_];\
            cpu_reg_pair_SP_set(&cpu, reg_k, input_b[i_]);\
            DO_RUN(cpu,__VA_ARGS__);\
        ck_assert_msg(cpu_HL_get(&cpu) == expected_v[i_],"Opcode 0x%" PRIX8 ": (0x%" PRIX16 ", 0x%" PRIX16 ") => value 0x%" PRIX16 " (!= 0x%" PRIX16 ")",\
                lu.opcode,input_a[i_], input_b[i_],cpu_HL_get(&cpu), expected_v[i_]);\
        ck_assert_msg(cpu.F == expected_f[i_],"Opcode 0x%" PRIX8 ":  (0x%" PRIX16 ", 0x%" PRIX16 ") => flag 0x%" PRIX8 "  (!= 0x%" PRIX8 ")",\
                lu.opcode,input_a[i_], input_b[i_],cpu.F, expected_f[i_]);\
        }\
        END_RUN();\
    } while(0)

//====================================================
//                  ERRRORS
//====================================================

START_TEST(dispatch_err)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    cpu_t cpu;
    zero_init_var(cpu);
    instruction_t lu = {0};

    ck_assert_int_eq(cpu_dispatch(NULL, &cpu), ERR_BAD_PARAMETER);
    ck_assert_int_eq(cpu_dispatch(&lu, NULL), ERR_BAD_PARAMETER);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//====================================================
//                  ADD
//====================================================

//ADD DATA
#define ADD_IN_DATA_A    {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F, 0x00, 0x00, 0x01, 0x01, 0xEF, 0x80, 0xFF, 0x0F}
#define ADD_IN_DATA_B    {0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x80, 0xFF, 0xF1, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x80, 0xFF, 0xF1}
#define ADD_IN_FLAG      {0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0}

#define ADD_OUT_DATA     {0x00, 0x00, 0x02, 0x02, 0x04, 0x04, 0x08, 0x10, 0x10, 0x00, 0x01, 0x01, 0x10, 0xF0, 0x00, 0xFE, 0x00, 0x00, 0x01, 0x01, 0x10, 0xF0, 0x00, 0xFE, 0x00}
#define ADD_OUT_FLAG     {0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x80, 0x00, 0x00, 0x20, 0x20, 0x90, 0x30, 0xB0, 0x80, 0x00, 0x00, 0x20, 0x20, 0x90, 0x30, 0xB0}
#define ADD_OUT_S_DATA   {0x00, 0x00, 0x02, 0x02, 0x04, 0x04, 0x08, 0x10, 0x10, 0x00, 0x00, 0x02, 0x02, 0xDE, 0x00, 0xFE, 0x1E, 0x00, 0x00, 0x02, 0x02, 0xDE, 0x00, 0xFE, 0x1E}
#define ADD_OUT_S_FLAG   {0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x80, 0x80, 0x00, 0x00, 0x30, 0x90, 0x30, 0x20, 0x80, 0x80, 0x00, 0x00, 0x30, 0x90, 0x30, 0x20}

#define ADD_OUT_C_DATA   {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x10, 0x11, 0x00, 0x01, 0x01, 0x10, 0xF0, 0x00, 0xFE, 0x00, 0x01, 0x02, 0x02, 0x11, 0xF1, 0x01, 0xFF, 0x01}
#define ADD_OUT_C_FLAG   {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x80, 0x00, 0x00, 0x20, 0x20, 0x90, 0x30, 0xB0, 0x00, 0x00, 0x00, 0x20, 0x20, 0x10, 0x30, 0x30}
#define ADD_OUT_SC_DATA  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x10, 0x11, 0x00, 0x00, 0x02, 0x02, 0xDE, 0x00, 0xFE, 0x1E, 0x01, 0x01, 0x03, 0x03, 0xDF, 0x01, 0xFF, 0x1F}
#define ADD_OUT_SC_FLAG  {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x80, 0x80, 0x00, 0x00, 0x30, 0x90, 0x30, 0x20, 0x00, 0x00, 0x00, 0x00, 0x30, 0x10, 0x30, 0x20}

#define INC_IN_DATA    {0x00, 0x01, 0xFF, 0xFE, 0x10, 0xF0, 0xdd, 0xbb, 0x0F, 0x00, 0xFF}
#define INC_IN_FLAG    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0}
#define INC_OUT_DATA   {0x01, 0x02, 0x00, 0xFF, 0x11, 0xF1, 0xde, 0xbc, 0x10, 0x01, 0x00}
#define INC_OUT_FLAG   {0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x10, 0xB0}

#define INC_IN_DATA16  {0x1000, 0x0000, 0xFFFF, 0xFFF0, 0x0001, 0x0FFF, 0xdead, 0xb011, 0xb005, 0x0000, 0xFFFF}
#define INC_IN_FLAG16  {  0x00,   0x00,   0x00,   0x00,   0x80,   0x00,   0x00,   0x00,   0x00,   0xF0,   0xF0}
#define INC_OUT_DATA16 {0x1001, 0x0001, 0x0000, 0xFFF1, 0x0002, 0x1000, 0xdeae, 0xb012, 0xb006, 0x0001, 0x0000}
#define INC_OUT_FLAG16 {  0x00,   0x00,   0x00,   0x00,   0x80,   0x00,   0x00,   0x00,   0x00,   0xF0,   0xF0}

#define ADD_IN_DATA_A16  {0x0000, 0xFFFF, 0x0001, 0x000F, 0x0FFF, 0x0002, 0x00FF, 0xEEEE, 0x000F, 0x0000, 0xFFFF}
#define ADD_IN_DATA_B16  {0x0000, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0001, 0x2222, 0x0000, 0x0000, 0x0001}
#define ADD_IN_FLAG16    {  0x00,   0x10,   0x00,   0x10,   0x00,   0x10,   0x00,   0x00,   0x10,   0xF0,   0xF0}

#define ADD_OUT_DATA16   {0x0000, 0x0000, 0x0002, 0x0010, 0x1001, 0x0004, 0x0100, 0x1110, 0x000F, 0x0000, 0x0000}
#define ADD_OUT_FLAG16   {  0x00,   0x30,   0x00,   0x00,   0x20,   0x00,   0x00,   0x30,   0x00,   0x80,   0xB0}
#define ADD_OUT_S_DATA16 {0x0000, 0xFFFE, 0x0002, 0x001E, 0x1FFE, 0x0004, 0x01FE, 0xDDDC, 0x001E, 0x0000, 0xFFFE}
#define ADD_OUT_S_FLAG16 {  0x00,   0x30,   0x00,   0x00,   0x20,   0x00,   0x00,   0x30,   0x00,   0x80,   0xB0}


START_TEST(test_ADD_A_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = ADD_IN_FLAG;
    const uint8_t input_a[] = ADD_IN_DATA_A;
    const uint8_t input_b[] = ADD_IN_DATA_B;

    const uint8_t expected_v[] = ADD_OUT_DATA;
    const uint8_t expected_f[] = ADD_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_HL(input_a, input_b, input_f, expected_v, expected_f, OP_ADD_A_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_ADC_A_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = ADD_IN_FLAG;
    const uint8_t input_a[] = ADD_IN_DATA_A;
    const uint8_t input_b[] = ADD_IN_DATA_B;

    const uint8_t expected_v[] = ADD_OUT_C_DATA;
    const uint8_t expected_f[] = ADD_OUT_C_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_HL(input_a, input_b, input_f, expected_v, expected_f, OP_ADC_A_HLR);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_ADD_A_N8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = ADD_IN_FLAG;
    const uint8_t input_a[] = ADD_IN_DATA_A;
    const uint8_t input_b[] = ADD_IN_DATA_B;

    const uint8_t expected_v[] = ADD_OUT_DATA;
    const uint8_t expected_f[] = ADD_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_N8(input_a, input_b, input_f, expected_v, expected_f, OP_ADD_A_N8);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_ADC_A_N8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = ADD_IN_FLAG;
    const uint8_t input_a[] = ADD_IN_DATA_A;
    const uint8_t input_b[] = ADD_IN_DATA_B;

    const uint8_t expected_v[] = ADD_OUT_C_DATA;
    const uint8_t expected_f[] = ADD_OUT_C_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_N8(input_a, input_b, input_f, expected_v, expected_f, OP_ADC_A_N8);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_ADD_A_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = ADD_IN_FLAG;
    const uint8_t input_a[] = ADD_IN_DATA_A;
    const uint8_t input_b[] = ADD_IN_DATA_B;

    const uint8_t expected_v[] = ADD_OUT_DATA;
    const uint8_t expected_f[] = ADD_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_B_CODE, OP_ADD_A_B);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_C_CODE, OP_ADD_A_C);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_D_CODE, OP_ADD_A_D);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_E_CODE, OP_ADD_A_E);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_H_CODE, OP_ADD_A_H);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_L_CODE, OP_ADD_A_L);

    //Special Case for A+A !
    const uint8_t expected_va[] = ADD_OUT_S_DATA;
    const uint8_t expected_fa[] = ADD_OUT_S_FLAG;
    ASSERT_EQ_NB_EL(input_f, expected_va);
    ASSERT_EQ_NB_EL(input_f, expected_fa);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_va, expected_fa, REG_A_CODE, OP_ADD_A_A);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_ADC_A_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = ADD_IN_FLAG;
    const uint8_t input_a[] = ADD_IN_DATA_A;
    const uint8_t input_b[] = ADD_IN_DATA_B;

    const uint8_t expected_v[] = ADD_OUT_C_DATA;
    const uint8_t expected_f[] = ADD_OUT_C_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_B_CODE, OP_ADC_A_B);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_C_CODE, OP_ADC_A_C);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_D_CODE, OP_ADC_A_D);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_E_CODE, OP_ADC_A_E);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_H_CODE, OP_ADC_A_H);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_L_CODE, OP_ADC_A_L);

    //Special Case for A+A !
    const uint8_t expected_va[] = ADD_OUT_SC_DATA;
    const uint8_t expected_fa[] = ADD_OUT_SC_FLAG;

    ASSERT_EQ_NB_EL(input_f, expected_va);
    ASSERT_EQ_NB_EL(input_f, expected_fa);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_va, expected_fa, REG_A_CODE, OP_ADC_A_A);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_INC8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = INC_IN_FLAG;
    const uint8_t input_a[] = INC_IN_DATA;

    const uint8_t expected_v[] = INC_OUT_DATA;
    const uint8_t expected_f[] = INC_OUT_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_A_CODE, OP_INC_A);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_B_CODE, OP_INC_B);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_C_CODE, OP_INC_C);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_D_CODE, OP_INC_D);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_E_CODE, OP_INC_E);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_H_CODE, OP_INC_H);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_f, REG_L_CODE, OP_INC_L);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_INC16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = INC_IN_FLAG16;
    const uint16_t input_a[] = INC_IN_DATA16;

    const uint16_t expected_v[] = INC_OUT_DATA16;
    const uint8_t expected_f[] = INC_OUT_FLAG16;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R16_S(input_a, input_f, expected_v, expected_f, REG_BC_CODE, OP_INC_BC);
    RUN_DISPATCH_ALU_R16_S(input_a, input_f, expected_v, expected_f, REG_DE_CODE, OP_INC_DE);
    RUN_DISPATCH_ALU_R16_S(input_a, input_f, expected_v, expected_f, REG_AF_CODE, OP_INC_SP);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_ADD_HL_R16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = ADD_IN_FLAG16;
    const uint16_t input_a[] = ADD_IN_DATA_A16;
    const uint16_t input_b[] = ADD_IN_DATA_B16;

    const uint16_t expected_v[] = ADD_OUT_DATA16;
    const uint8_t expected_f[] = ADD_OUT_FLAG16;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, input_b);
    ASSERT_EQ_NB_EL(input_f, expected_v);
    ASSERT_EQ_NB_EL(input_f, expected_f);

    RUN_DISPATCH_ALU_R16(input_a, input_b, input_f, expected_v, expected_f, REG_BC_CODE, OP_ADD_HL_BC);
    RUN_DISPATCH_ALU_R16(input_a, input_b, input_f, expected_v, expected_f, REG_DE_CODE, OP_ADD_HL_DE);
    RUN_DISPATCH_ALU_R16(input_a, input_b, input_f, expected_v, expected_f, REG_AF_CODE, OP_ADD_HL_SP);

    //Special Case for HL+HL !
    const uint16_t expected_va[] = ADD_OUT_S_DATA16;
    const uint8_t expected_fa[] = ADD_OUT_S_FLAG16;

    ASSERT_EQ_NB_EL(input_f, expected_va);
    ASSERT_EQ_NB_EL(input_f, expected_fa);

    RUN_DISPATCH_ALU_R16_S(input_a, input_f, expected_va, expected_fa, REG_HL_CODE, OP_ADD_HL_HL);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_ROT_R8)
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

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_A_CODE, OP_RL_A);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_B_CODE, OP_RL_B);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_C_CODE, OP_RL_C);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_D_CODE, OP_RL_D);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_E_CODE, OP_RL_E);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_H_CODE, OP_RL_H);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_L_CODE, OP_RL_L);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_A_CODE, OP_RR_A);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_B_CODE, OP_RR_B);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_C_CODE, OP_RR_C);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_D_CODE, OP_RR_D);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_E_CODE, OP_RR_E);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_H_CODE, OP_RR_H);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vr, expected_fr, REG_L_CODE, OP_RR_L);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_SLA_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_a[] = SHFT_IN_DATA;
    const uint8_t input_f[] = SHFT_IN_FLAG;

    const uint8_t expected_vl[] = SHFT_OUT_L_DATA;
    const uint8_t expected_fl[] = SHFT_OUT_L_FLAG;

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_vl);
    ASSERT_EQ_NB_EL(input_f, expected_fl);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_A_CODE, OP_SLA_A);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_B_CODE, OP_SLA_B);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_C_CODE, OP_SLA_C);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_D_CODE, OP_SLA_D);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_E_CODE, OP_SLA_E);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_H_CODE, OP_SLA_H);
    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_vl, expected_fl, REG_L_CODE, OP_SLA_L);

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//================================================================
//                  CMP
//================================================================

START_TEST(test_CP_A_R8)
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

    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_B_CODE, OP_CP_A_B);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_C_CODE, OP_CP_A_C);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_D_CODE, OP_CP_A_D);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_E_CODE, OP_CP_A_E);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_H_CODE, OP_CP_A_H);
    RUN_DISPATCH_ALU_R8(input_a, input_b, input_f, expected_v, expected_f, REG_L_CODE, OP_CP_A_L);

    //Special Case for A+A !
    const uint8_t expected_fa[] = CP_OUT_S_FLAG;
    ASSERT_EQ_NB_EL(input_f, expected_fa);

    RUN_DISPATCH_ALU_R8_S(input_a, input_f, expected_v, expected_fa, REG_A_CODE, OP_CP_A_A);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

// ======================================================================
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

START_TEST(test_BIT_U3_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = BIT_IN_FLAG;
    const uint8_t input_a[] = BIT_IN_DATA;

#define BITOP_COUNT 8
    const uint8_t expected_f[BITOP_COUNT][sizeof(input_a) / sizeof(*input_a)] = {
        BIT_OUT_FLAG_0,
        BIT_OUT_FLAG_1,
        BIT_OUT_FLAG_2,
        BIT_OUT_FLAG_3,
        BIT_OUT_FLAG_4,
        BIT_OUT_FLAG_5,
        BIT_OUT_FLAG_6,
        BIT_OUT_FLAG_7
    };

    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_f[0]);
    ASSERT_EQ_NB_EL(input_f, expected_f[1]);
    ASSERT_EQ_NB_EL(input_f, expected_f[2]);
    ASSERT_EQ_NB_EL(input_f, expected_f[3]);
    ASSERT_EQ_NB_EL(input_f, expected_f[4]);
    ASSERT_EQ_NB_EL(input_f, expected_f[5]);
    ASSERT_EQ_NB_EL(input_f, expected_f[6]);
    ASSERT_EQ_NB_EL(input_f, expected_f[7]);


#define BITOP_REGS 7
    const instruction_t op_bitt[BITOP_REGS][BITOP_COUNT] = {
        {OP_BIT_0_A, OP_BIT_1_A, OP_BIT_2_A, OP_BIT_3_A, OP_BIT_4_A, OP_BIT_5_A, OP_BIT_6_A, OP_BIT_7_A},
        {OP_BIT_0_B, OP_BIT_1_B, OP_BIT_2_B, OP_BIT_3_B, OP_BIT_4_B, OP_BIT_5_B, OP_BIT_6_B, OP_BIT_7_B},
        {OP_BIT_0_C, OP_BIT_1_C, OP_BIT_2_C, OP_BIT_3_C, OP_BIT_4_C, OP_BIT_5_C, OP_BIT_6_C, OP_BIT_7_C},
        {OP_BIT_0_D, OP_BIT_1_D, OP_BIT_2_D, OP_BIT_3_D, OP_BIT_4_D, OP_BIT_5_D, OP_BIT_6_D, OP_BIT_7_D},
        {OP_BIT_0_E, OP_BIT_1_E, OP_BIT_2_E, OP_BIT_3_E, OP_BIT_4_E, OP_BIT_5_E, OP_BIT_6_E, OP_BIT_7_E},
        {OP_BIT_0_H, OP_BIT_1_H, OP_BIT_2_H, OP_BIT_3_H, OP_BIT_4_H, OP_BIT_5_H, OP_BIT_6_H, OP_BIT_7_H},
        {OP_BIT_0_L, OP_BIT_1_L, OP_BIT_2_L, OP_BIT_3_L, OP_BIT_4_L, OP_BIT_5_L, OP_BIT_6_L, OP_BIT_7_L}
    };
    const reg_kind regkt[BITOP_REGS] = {REG_A_CODE, REG_B_CODE, REG_C_CODE, REG_D_CODE, REG_E_CODE, REG_H_CODE, REG_L_CODE};

    INIT_RUN();

    for (size_t i = 0; i < BITOP_REGS; ++i) {
        for (size_t j = 0; j < BITOP_COUNT; ++j) {
            LOOP_ON(input_a) {
                BITOP_R8_HELPER(cpu, expected_f[j], input_a, regkt[i], op_bitt[i][j]);
            }
        }
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_CHG_U3_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint8_t input_f[] = BIT_IN_FLAG;
    const uint8_t input_a[] = BIT_IN_DATA;

#define BITOP_COUNT 8
    const uint8_t expected_f[BITOP_COUNT][sizeof(input_a) / sizeof(*input_a)] = {
        CHG_OUT_FLAG_0,
        CHG_OUT_FLAG_1,
        CHG_OUT_FLAG_2,
        CHG_OUT_FLAG_3,
        CHG_OUT_FLAG_4,
        CHG_OUT_FLAG_5,
        CHG_OUT_FLAG_6,
        CHG_OUT_FLAG_7
    };

    const uint8_t expected_v[BITOP_COUNT][sizeof(input_a) / sizeof(*input_a)] = {
        CHG_OUT_DATA_0,
        CHG_OUT_DATA_1,
        CHG_OUT_DATA_2,
        CHG_OUT_DATA_3,
        CHG_OUT_DATA_4,
        CHG_OUT_DATA_5,
        CHG_OUT_DATA_6,
        CHG_OUT_DATA_7
    };


    ASSERT_EQ_NB_EL(input_f, input_a);
    ASSERT_EQ_NB_EL(input_f, expected_f[0]);
    ASSERT_EQ_NB_EL(input_f, expected_f[1]);
    ASSERT_EQ_NB_EL(input_f, expected_f[2]);
    ASSERT_EQ_NB_EL(input_f, expected_f[3]);
    ASSERT_EQ_NB_EL(input_f, expected_f[4]);
    ASSERT_EQ_NB_EL(input_f, expected_f[5]);
    ASSERT_EQ_NB_EL(input_f, expected_f[6]);
    ASSERT_EQ_NB_EL(input_f, expected_f[7]);
    ASSERT_EQ_NB_EL(input_f, expected_v[0]);
    ASSERT_EQ_NB_EL(input_f, expected_v[1]);
    ASSERT_EQ_NB_EL(input_f, expected_v[2]);
    ASSERT_EQ_NB_EL(input_f, expected_v[3]);
    ASSERT_EQ_NB_EL(input_f, expected_v[4]);
    ASSERT_EQ_NB_EL(input_f, expected_v[5]);
    ASSERT_EQ_NB_EL(input_f, expected_v[6]);
    ASSERT_EQ_NB_EL(input_f, expected_v[7]);


#define BITOP_REGS 7
    const instruction_t op_bitt[BITOP_REGS][BITOP_COUNT] = {
        {OP_RES_0_A, OP_RES_1_A, OP_RES_2_A, OP_RES_3_A, OP_RES_4_A, OP_RES_5_A, OP_RES_6_A, OP_RES_7_A},
        {OP_RES_0_B, OP_RES_1_B, OP_RES_2_B, OP_RES_3_B, OP_RES_4_B, OP_RES_5_B, OP_RES_6_B, OP_RES_7_B},
        {OP_RES_0_C, OP_RES_1_C, OP_RES_2_C, OP_RES_3_C, OP_RES_4_C, OP_RES_5_C, OP_RES_6_C, OP_RES_7_C},
        {OP_RES_0_D, OP_RES_1_D, OP_RES_2_D, OP_RES_3_D, OP_RES_4_D, OP_RES_5_D, OP_RES_6_D, OP_RES_7_D},
        {OP_RES_0_E, OP_RES_1_E, OP_RES_2_E, OP_RES_3_E, OP_RES_4_E, OP_RES_5_E, OP_RES_6_E, OP_RES_7_E},
        {OP_RES_0_H, OP_RES_1_H, OP_RES_2_H, OP_RES_3_H, OP_RES_4_H, OP_RES_5_H, OP_RES_6_H, OP_RES_7_H},
        {OP_RES_0_L, OP_RES_1_L, OP_RES_2_L, OP_RES_3_L, OP_RES_4_L, OP_RES_5_L, OP_RES_6_L, OP_RES_7_L}
    };
    const reg_kind regkt[BITOP_REGS] = {REG_A_CODE, REG_B_CODE, REG_C_CODE, REG_D_CODE, REG_E_CODE, REG_H_CODE, REG_L_CODE};

    INIT_RUN();

    for (size_t i = 0; i < BITOP_REGS; ++i) {
        for (size_t j = 0; j < BITOP_COUNT; ++j) {
            LOOP_ON(input_a) {
                BITOP_R8_HELPER(cpu, expected_f[j], expected_v[j], regkt[i], op_bitt[i][j]);
            }
        }
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

// ======================================================================
#define LD_PUSH_R16_HELPER(cpu, reg, ...)\
    do{\
        cpu_reg_pair_set(&cpu, reg, nn[i_]);\
        uint16_t spv = cpu.SP;\
        DO_RUN(cpu, __VA_ARGS__);\
        ck_assert_int_eq(cpu.SP, spv-2);\
        if(reg == REG_AF_CODE){\
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, cpu.SP), n1f[i_]);\
        }else{\
            ck_assert_int_eq(CPU_BUS_V_AT(cpu, cpu.SP), n1[i_]);\
        }\
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, cpu.SP+1), n2[i_]);\
    }while(0)

#define LD_POP_R16_HELPER(cpu, reg, ...)\
    do{\
        CPU_BUS_V_AT(cpu, cpu.SP) = n1[i_];\
        CPU_BUS_V_AT(cpu, cpu.SP+1)= n2[i_];\
        uint16_t spv = cpu.SP;\
        DO_RUN(cpu, __VA_ARGS__);\
        ck_assert_int_eq(cpu.SP, spv+2);\
        if(reg == REG_AF_CODE){\
            ck_assert_int_eq(cpu_reg_pair_get(&cpu, reg), nnf[i_]);\
        }else{\
            ck_assert_int_eq(cpu_reg_pair_get(&cpu, reg), nn[i_]);\
        }\
    }while(0)

#define SPPP_N1  {  0x00,   0xad,   0xFF,   0x01,   0x01,   0x00,   0x00,   0x00}
#define SPPP_N2  {  0x00,   0xde,   0x00,   0x01,   0x01,   0x00,   0x00,   0x00}
#define SPPP_NN  {0x0000, 0xdead, 0x00FF, 0x0101, 0x0101, 0x0000, 0x0000, 0x0000}

#define SPPP_N1F {  0x00,   0xa0,   0xF0,   0x00,   0x00,   0x00,   0x00,   0x00}
#define SPPP_NNF {0x0000, 0xdea0, 0x00F0, 0x0100, 0x0100, 0x0000, 0x0000, 0x0000}

START_TEST(test_PUSH_R16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const uint8_t n1[] =  SPPP_N1;
    const uint8_t n2[] =  SPPP_N2;
    const uint16_t nn[] = SPPP_NN;
    const uint8_t n1f[] = SPPP_N1F;

    ASSERT_EQ_NB_EL(nn, n1);
    ASSERT_EQ_NB_EL(nn, n2);
    ASSERT_EQ_NB_EL(nn, n1f);

    cpu.SP = 0xFF;
    LOOP_ON(nn) {
        LD_PUSH_R16_HELPER(cpu, REG_AF_CODE, OP_PUSH_AF);
    }
    LOOP_ON(nn) {
        LD_PUSH_R16_HELPER(cpu, REG_BC_CODE, OP_PUSH_BC);
    }
    LOOP_ON(nn) {
        LD_PUSH_R16_HELPER(cpu, REG_DE_CODE, OP_PUSH_DE);
    }
    LOOP_ON(nn) {
        LD_PUSH_R16_HELPER(cpu, REG_HL_CODE, OP_PUSH_HL);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_POP_R16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();

    const uint8_t n1[] =  SPPP_N1;
    const uint8_t n2[] =  SPPP_N2;
    const uint16_t nn[] = SPPP_NN;
    const uint16_t nnf[] = SPPP_NNF;

    ASSERT_EQ_NB_EL(nn, n1);
    ASSERT_EQ_NB_EL(nn, n2);
    ASSERT_EQ_NB_EL(nn, nnf);

    cpu.SP = 0xFF;
    LOOP_ON(nn) {
        LD_POP_R16_HELPER(cpu, REG_AF_CODE, OP_POP_AF);
    }
    LOOP_ON(nn) {
        LD_POP_R16_HELPER(cpu, REG_BC_CODE, OP_POP_BC);
    }
    LOOP_ON(nn) {
        LD_POP_R16_HELPER(cpu, REG_DE_CODE, OP_POP_DE);
    }
    LOOP_ON(nn) {
        LD_POP_R16_HELPER(cpu, REG_HL_CODE, OP_POP_HL);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_PUSH_POP_R16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const uint16_t spvi = 0xFF;

    const uint8_t n1[] =  SPPP_N1;
    const uint8_t n2[] =  SPPP_N2;
    const uint16_t nn[] = SPPP_NN;
    const uint8_t n1f[] = SPPP_N1F;
    const uint16_t nnf[] = SPPP_NNF;

    ASSERT_EQ_NB_EL(nn, n1);
    ASSERT_EQ_NB_EL(nn, n2);
    ASSERT_EQ_NB_EL(nn, n1f);
    ASSERT_EQ_NB_EL(nn, nnf);

    cpu.SP = spvi;
    LOOP_ON(nn) {
        LD_PUSH_R16_HELPER(cpu, REG_AF_CODE, OP_PUSH_AF);
        LD_POP_R16_HELPER(cpu, REG_AF_CODE, OP_POP_AF);
    }
    ck_assert_int_eq(cpu.SP, spvi);
    LOOP_ON(nn) {
        LD_PUSH_R16_HELPER(cpu, REG_BC_CODE, OP_PUSH_BC);
        LD_POP_R16_HELPER(cpu, REG_BC_CODE, OP_POP_BC);
    }
    ck_assert_int_eq(cpu.SP, spvi);
    LOOP_ON(nn) {
        LD_PUSH_R16_HELPER(cpu, REG_DE_CODE, OP_PUSH_DE);
        LD_POP_R16_HELPER(cpu, REG_DE_CODE, OP_POP_DE);
    }
    ck_assert_int_eq(cpu.SP, spvi);
    LOOP_ON(nn) {
        LD_PUSH_R16_HELPER(cpu, REG_HL_CODE, OP_PUSH_HL);
        LD_POP_R16_HELPER(cpu, REG_HL_CODE, OP_POP_HL);
    }
    ck_assert_int_eq(cpu.SP, spvi);

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//================================================================
//                  LD-LOAD
//================================================================

START_TEST(test_LD_A_BCR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const data_t dt[] =   {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint8_t bcv[] = {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    ASSERT_EQ_NB_EL(dt, bcv);

    LOOP_ON(dt) {
        CPU_BUS_V_AT(cpu, bcv[i_]) = dt[i_];
        cpu_BC_set(&cpu, bcv[i_]);
        DO_RUN(cpu, OP_LD_A_BCR);
        ck_assert_int_eq(cpu.A, dt[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_A_CR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const data_t dt[] =  {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint8_t cv[] = {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    ASSERT_EQ_NB_EL(dt, cv);

    LOOP_ON(dt) {
        CPU_BUS_V_AT(cpu, REGISTERS_START + cv[i_]) = dt[i_];
        cpu.C = cv[i_];
        DO_RUN(cpu, OP_LD_A_CR);
        ck_assert_int_eq(cpu.A, dt[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_A_DER)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const data_t dt[] =  {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint8_t dev[] = {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    ASSERT_EQ_NB_EL(dt, dev);

    LOOP_ON(dt) {
        CPU_BUS_V_AT(cpu, dev[i_]) = dt[i_];
        cpu_DE_set(&cpu, dev[i_]);
        DO_RUN(cpu, OP_LD_A_DER);
        ck_assert_int_eq(cpu.A, dt[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_A_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();

    const data_t dt[] =    {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint8_t hlv[] =  {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    ASSERT_EQ_NB_EL(dt, hlv);

    LOOP_ON(dt) {
        CPU_BUS_V_AT(cpu, hlv[i_]) = dt[i_];
        cpu_HL_set(&cpu, hlv[i_]);
        DO_RUN(cpu, OP_LD_A_HLR);
        ck_assert_int_eq(cpu.A, dt[i_]);
        ck_assert_int_eq(cpu_HL_get(&cpu), hlv[i_]);
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_A_HLRU)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();

    const data_t dt[] =    {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint8_t hlv[] =  {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    const uint16_t hlevd[] = {0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF};
    const uint8_t hlevi[] = {0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0001, 0x0001, 0x0001};

    ASSERT_EQ_NB_EL(dt, hlv);
    ASSERT_EQ_NB_EL(dt, hlevd);
    ASSERT_EQ_NB_EL(dt, hlevi);

    LOOP_ON(dt) {
        CPU_BUS_V_AT(cpu, hlv[i_]) = dt[i_];
        cpu_HL_set(&cpu, hlv[i_]);
        DO_RUN(cpu, OP_LD_A_HLRD);
        ck_assert_int_eq(cpu.A, dt[i_]);
        ck_assert_int_eq(cpu_HL_get(&cpu), hlevd[i_]);
    }

    LOOP_ON(dt) {
        CPU_BUS_V_AT(cpu, hlv[i_]) = dt[i_];
        cpu_HL_set(&cpu, hlv[i_]);
        DO_RUN(cpu, OP_LD_A_HLRI);
        ck_assert_int_eq(cpu.A, dt[i_]);
        ck_assert_int_eq(cpu_HL_get(&cpu), hlevi[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_A_N16R)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const data_t dt[] =  {  0x00,   0x10,   0x11,   0xFF,   0x00,   0xdd,   0x0F,   0xF0};
    const uint8_t n1[] = {  0x00,   0xad,   0xFF,   0x01,   0x01,   0x00,   0x00,   0x00};
    const uint8_t n2[] = {  0x00,   0xde,   0x00,   0x01,   0x01,   0x00,   0x00,   0x00};
    const uint16_t nn[] = {0x0000, 0xdead, 0x00FF, 0x0101, 0x0101, 0x0000, 0x0000, 0x0000};

    ASSERT_EQ_NB_EL(dt, n1);
    ASSERT_EQ_NB_EL(dt, n2);
    ASSERT_EQ_NB_EL(dt, nn);

    LOOP_ON(dt) {
        CPU_BUS_V_AT(cpu, nn[i_]) = dt[i_];
        CPU_BUS_V_AT(cpu, (cpu.PC + 1)) = n1[i_];
        CPU_BUS_V_AT(cpu, (cpu.PC + 2)) = n2[i_];
        DO_RUN(cpu, OP_LD_A_N16R);
        ck_assert_int_eq(cpu.A, dt[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_A_N8R)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const data_t dt[] =  {  0x00,   0x10,   0x11,   0xFF,   0x00,   0xdd,   0x0F,   0xF0};
    const uint8_t n[] = {  0x00,   0xad,   0xFF,   0x01,   0x01,   0x00,   0x00,   0x00};

    ASSERT_EQ_NB_EL(dt, n);

    LOOP_ON(dt) {
        CPU_BUS_V_AT(cpu, REGISTERS_START + n[i_]) = dt[i_];
        CPU_BUS_V_AT(cpu, (cpu.PC + 1)) = n[i_];
        DO_RUN(cpu, OP_LD_A_N8R);
        ck_assert_int_eq(cpu.A, dt[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

#define LD_R16SP_N16_HELPER(cpu, reg, ...)\
    do{\
        CPU_BUS_V_AT(cpu, (cpu.PC + 1)) = n1[i_];\
        CPU_BUS_V_AT(cpu, (cpu.PC + 2)) = n2[i_];\
        DO_RUN(cpu, __VA_ARGS__);\
        ck_assert_int_eq(cpu_reg_pair_SP_get(&cpu,reg), dt[i_]);\
    }while(0)

START_TEST(test_LD_R16SP_N16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const uint16_t dt[] = {0x0000, 0xdead, 0x00FF, 0x0101, 0x0101, 0x0000, 0x0000, 0x0000};
    const uint8_t n1[] =  {  0x00,   0xad,   0xFF,   0x01,   0x01,   0x00,   0x00,   0x00};
    const uint8_t n2[] =  {  0x00,   0xde,   0x00,   0x01,   0x01,   0x00,   0x00,   0x00};

    ASSERT_EQ_NB_EL(dt, n1);
    ASSERT_EQ_NB_EL(dt, n2);

    LOOP_ON(dt) {
        LD_R16SP_N16_HELPER(cpu,  REG_BC_CODE, OP_LD_BC_N16);
        LD_R16SP_N16_HELPER(cpu,  REG_DE_CODE, OP_LD_DE_N16);
        LD_R16SP_N16_HELPER(cpu,  REG_HL_CODE, OP_LD_HL_N16);
        LD_R16SP_N16_HELPER(cpu,  REG_AF_CODE, OP_LD_SP_N16);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

#define LD_R8_HLR_HELPER(cpu, reg, ...)\
    do{\
        cpu_HL_set(&cpu,hlv[i_]);\
        CPU_BUS_V_AT(cpu,hlv[i_]) = dt[i_];\
        DO_RUN(cpu, __VA_ARGS__);\
        ck_assert_int_eq(cpu_reg_get(&cpu,reg),dt[i_]);\
    }while(0)

START_TEST(test_LD_R8_HLR)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();

    const data_t dt[] =  {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint16_t hlv[] = {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    ASSERT_EQ_NB_EL(dt, hlv);

    LOOP_ON(dt) {
        LD_R8_HLR_HELPER(cpu,  REG_A_CODE, OP_LD_A_HLR);
        LD_R8_HLR_HELPER(cpu,  REG_B_CODE, OP_LD_B_HLR);
        LD_R8_HLR_HELPER(cpu,  REG_C_CODE, OP_LD_C_HLR);
        LD_R8_HLR_HELPER(cpu,  REG_D_CODE, OP_LD_D_HLR);
        LD_R8_HLR_HELPER(cpu,  REG_E_CODE, OP_LD_E_HLR);
        LD_R8_HLR_HELPER(cpu,  REG_H_CODE, OP_LD_H_HLR);
        LD_R8_HLR_HELPER(cpu,  REG_L_CODE, OP_LD_L_HLR);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

#define LD_R8_N8_HELPER(cpu, reg,...)\
    do{\
        CPU_BUS_V_AT(cpu, (cpu.PC + 1)) = dt[i_];\
        DO_RUN(cpu, __VA_ARGS__);\
        ck_assert_int_eq(cpu_reg_get(&cpu,reg),dt[i_]);\
    }while(0)

START_TEST(test_LD_R8_N8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();

    const data_t dt[] =  {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};

    LOOP_ON(dt) {
        LD_R8_N8_HELPER(cpu,  REG_A_CODE, OP_LD_A_N8);
        LD_R8_N8_HELPER(cpu,  REG_B_CODE, OP_LD_B_N8);
        LD_R8_N8_HELPER(cpu,  REG_C_CODE, OP_LD_C_N8);
        LD_R8_N8_HELPER(cpu,  REG_D_CODE, OP_LD_D_N8);
        LD_R8_N8_HELPER(cpu,  REG_E_CODE, OP_LD_E_N8);
        LD_R8_N8_HELPER(cpu,  REG_H_CODE, OP_LD_H_N8);
        LD_R8_N8_HELPER(cpu,  REG_L_CODE, OP_LD_L_N8);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST



//================================================================
//                  LD-STORE
//================================================================

START_TEST(test_LD_BCR_A)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const data_t dt[] =   {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint8_t bcv[] = {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    ASSERT_EQ_NB_EL(dt, bcv);

    LOOP_ON(dt) {
        cpu.A = dt[i_];
        cpu_BC_set(&cpu, bcv[i_]);
        DO_RUN(cpu, OP_LD_BCR_A);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, bcv[i_]), dt[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_CR_A)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const data_t dt[] =  {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint8_t cv[] = {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    ASSERT_EQ_NB_EL(dt, cv);

    LOOP_ON(dt) {
        cpu.A = dt[i_];
        cpu.C = cv[i_];
        DO_RUN(cpu, OP_LD_CR_A);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, REGISTERS_START + cv[i_]), dt[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_DER_A)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const data_t dt[] =  {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint8_t dev[] = {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    ASSERT_EQ_NB_EL(dt, dev);

    LOOP_ON(dt) {
        cpu.A = dt[i_];
        cpu_DE_set(&cpu, dev[i_]);
        DO_RUN(cpu, OP_LD_DER_A);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, dev[i_]), dt[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_HLRU_A)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();

    const data_t dt[] =    {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint8_t hlv[] =  {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    const uint16_t hlevd[] = {0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF};
    const uint8_t hlevi[] = {0x0001, 0x0001, 0x0002, 0x0002, 0x0002, 0x0001, 0x0001, 0x0001};

    ASSERT_EQ_NB_EL(dt, hlv);
    ASSERT_EQ_NB_EL(dt, hlevd);
    ASSERT_EQ_NB_EL(dt, hlevi);

    LOOP_ON(dt) {
        cpu.A = dt[i_];
        cpu_HL_set(&cpu, hlv[i_]);
        DO_RUN(cpu, OP_LD_HLRD_A);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, hlv[i_]), dt[i_]);
        ck_assert_int_eq(cpu_HL_get(&cpu), hlevd[i_]);
    }

    LOOP_ON(dt) {
        cpu.A = dt[i_];
        cpu_HL_set(&cpu, hlv[i_]);
        DO_RUN(cpu, OP_LD_HLRI_A);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, hlv[i_]), dt[i_]);
        ck_assert_int_eq(cpu_HL_get(&cpu), hlevi[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_HLR_N8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const data_t dt[] =  {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint16_t hlv[] = {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};
    const uint16_t pci[] = {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20};

    ASSERT_EQ_NB_EL(dt, hlv);
    ASSERT_EQ_NB_EL(dt, pci);

    LOOP_ON(dt) {
        cpu_HL_set(&cpu, hlv[i_]);
        cpu.PC = pci[i_];
        CPU_BUS_V_AT(cpu, (cpu.PC + 1)) = dt[i_];
        DO_RUN(cpu, OP_LD_HLR_N8);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, hlv[i_]), dt[i_]);
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

#define LD_HLR_R8_HELPER(cpu, reg, ...)\
    do{\
        cpu_HL_set(&cpu,hlv[i_]);\
        RUN_FOR_REG(cpu, reg, __VA_ARGS__);\
        ck_assert_int_eq(CPU_BUS_V_AT(cpu,hlv[i_]),dt[i_]);\
    }while(0)

START_TEST(test_LD_HLR_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();

    const data_t dt[] =  {0x00, 0x10, 0x11, 0xFF, 0x00, 0xdd, 0x0F, 0xF0};
    const uint16_t hlv[] = {0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00};

    ASSERT_EQ_NB_EL(dt, hlv);

    LOOP_ON(dt) {
        LD_HLR_R8_HELPER(cpu, REG_A_CODE, OP_LD_HLR_A);
        LD_HLR_R8_HELPER(cpu, REG_B_CODE, OP_LD_HLR_B);
        LD_HLR_R8_HELPER(cpu, REG_C_CODE, OP_LD_HLR_C);
        LD_HLR_R8_HELPER(cpu, REG_D_CODE, OP_LD_HLR_D);
        LD_HLR_R8_HELPER(cpu, REG_E_CODE, OP_LD_HLR_E);
        LD_HLR_R8_HELPER(cpu, REG_H_CODE, OP_LD_HLR_H);
        LD_HLR_R8_HELPER(cpu, REG_L_CODE, OP_LD_HLR_L);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_N16R_A)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const data_t dt[] =  {  0x00,   0x10,   0x11,   0xFF,   0x00,   0xdd,   0x0F,   0xF0};
    const uint8_t n1[] = {  0x00,   0xad,   0xFF,   0x01,   0x01,   0x00,   0x00,   0x00};
    const uint8_t n2[] = {  0x00,   0xde,   0x00,   0x01,   0x01,   0x00,   0x00,   0x00};
    const uint16_t nn[] = {0x0000, 0xdead, 0x00FF, 0x0101, 0x0101, 0x0000, 0x0000, 0x0000};

    ASSERT_EQ_NB_EL(dt, n1);
    ASSERT_EQ_NB_EL(dt, n2);
    ASSERT_EQ_NB_EL(dt, nn);

    LOOP_ON(dt) {
        cpu.A = dt[i_];
        CPU_BUS_V_AT(cpu, (cpu.PC + 1)) = n1[i_];
        CPU_BUS_V_AT(cpu, (cpu.PC + 2)) = n2[i_];
        DO_RUN(cpu, OP_LD_N16R_A);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, nn[i_]), dt[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_N16R_SP)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const uint16_t dt[] = {0xdead, 0x1010, 0x1111, 0xFFFF, 0x0000, 0xdead, 0x0F0F, 0xF0F0};
    const uint8_t n1[] =  {  0x00,   0xad,   0xFF,   0x01,   0x01,   0x00,   0x00,   0x00};
    const uint8_t n2[] =  {  0x00,   0xde,   0x00,   0x01,   0x01,   0x00,   0x00,   0x00};
    const uint16_t nn[] = {0x0000, 0xdead, 0x00FF, 0x0101, 0x0101, 0x0000, 0x0000, 0x0000};

    ASSERT_EQ_NB_EL(dt, n1);
    ASSERT_EQ_NB_EL(dt, n2);
    ASSERT_EQ_NB_EL(dt, nn);

    LOOP_ON(dt) {
        cpu.SP = dt[i_];
        CPU_BUS_V_AT(cpu, (cpu.PC + 1)) = n1[i_];
        CPU_BUS_V_AT(cpu, (cpu.PC + 2)) = n2[i_];
        DO_RUN(cpu, OP_LD_N16R_SP);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, nn[i_]), lsb8(dt[i_]));
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, nn[i_] + 1), msb8(dt[i_]));
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


//================================================================
//                  LD MISC
//================================================================

START_TEST(test_LD_SP_HL)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const uint16_t nn[] = {0x0000, 0xdead, 0x00FF, 0x1010, 0x0101, 0x0000, 0xdead, 0xbeef, 0x0000};

    LOOP_ON(nn) {
        cpu_HL_set(&cpu, nn[i_]);
        DO_RUN(cpu, OP_LD_SP_HL);
        ck_assert_int_eq(cpu.SP, nn[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_LD_R8_R8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const uint8_t n[] =  {0x00, 0xad, 0xFF, 0x01, 0x10, 0x00, 0xdd, 0xbb, 0x00, 0xc0, 0xfe, 0xca, 0xda, 0x00, 0xbe, 0xef, 0x00};
    const instruction_t insts[7][7] = {{OP_LD_A_A, OP_LD_A_B, OP_LD_A_C, OP_LD_A_D, OP_LD_A_E, OP_LD_A_H, OP_LD_A_L},
        {OP_LD_B_A, OP_LD_B_B, OP_LD_B_C, OP_LD_B_D, OP_LD_B_E, OP_LD_B_H, OP_LD_B_L},
        {OP_LD_C_A, OP_LD_C_B, OP_LD_C_C, OP_LD_C_D, OP_LD_C_E, OP_LD_C_H, OP_LD_C_L},
        {OP_LD_D_A, OP_LD_D_B, OP_LD_D_C, OP_LD_D_D, OP_LD_D_E, OP_LD_D_H, OP_LD_D_L},
        {OP_LD_E_A, OP_LD_E_B, OP_LD_E_C, OP_LD_E_D, OP_LD_E_E, OP_LD_E_H, OP_LD_E_L},
        {OP_LD_H_A, OP_LD_H_B, OP_LD_H_C, OP_LD_H_D, OP_LD_H_E, OP_LD_H_H, OP_LD_H_L},
        {OP_LD_L_A, OP_LD_L_B, OP_LD_L_C, OP_LD_L_D, OP_LD_L_E, OP_LD_L_H, OP_LD_L_L},
    };
    const reg_kind regs_idx[7] = {REG_A_CODE, REG_B_CODE, REG_C_CODE, REG_D_CODE, REG_E_CODE, REG_H_CODE, REG_L_CODE};
    const size_t is = sizeof(insts) / sizeof(*insts);

    for (size_t i = 0; i < is; ++i) {
        for (size_t j = 0; j < is; ++j) {
            const instruction_t cur_inst = insts[i][j];
            const reg_kind reg_from = regs_idx[j];
            const reg_kind reg_to = regs_idx[i];

            LOOP_ON(n) {
                cpu_reg_set(&cpu, reg_from, n[i_]);
                DO_RUN(cpu, cur_inst);
                ck_assert_int_eq(cpu_reg_get(&cpu, reg_to), n[i_]);
            }
        }
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//================================================================
//                  MISC
//================================================================

START_TEST(test_STOP)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    instruction_t lu = OP_STOP;
    ck_assert_int_eq(cpu_dispatch(&lu, &cpu), ERR_NONE);
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_NOP)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    instruction_t lu = OP_NOP;
    ck_assert_int_eq(cpu_dispatch(&lu, &cpu), ERR_NONE);
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

    Add_Case(s, tc1, "Cpu Basic Dispatch Tests");
    tcase_add_test(tc1, dispatch_err);

    Add_Case(s, tcalu0, "Cpu Dispatch ALU ADD Tests");
    tcase_add_test(tcalu0, test_ADD_A_HLR);
    tcase_add_test(tcalu0, test_ADC_A_HLR);
    tcase_add_test(tcalu0, test_ADD_A_N8);
    tcase_add_test(tcalu0, test_ADC_A_N8);
    tcase_add_test(tcalu0, test_ADD_A_R8);
    tcase_add_test(tcalu0, test_ADC_A_R8);
    tcase_add_test(tcalu0, test_ADD_HL_R16);
    tcase_add_test(tcalu0, test_INC8);
    tcase_add_test(tcalu0, test_INC16);

    Add_Case(s, tcalu5, "Cpu Dispatch ALU ROT Tests");
    tcase_add_test(tcalu5, test_ROT_R8);

    Add_Case(s, tcalu6, "Cpu Dispatch ALU SHIFT Tests");
    tcase_add_test(tcalu6, test_SLA_R8);

    Add_Case(s, tcalu7, "Cpu Dispatch ALU CP Tests");
    tcase_add_test(tcalu7, test_CP_A_R8);

    Add_Case(s, tcalu9, "Cpu Dispatch ALU BIT/CHG Tests");
    tcase_add_test(tcalu9, test_BIT_U3_R8);
    tcase_add_test(tcalu9, test_CHG_U3_R8);

    Add_Case(s, tcld0, "Cpu Dispatch LD LOAD Tests");
    tcase_add_test(tcld0, test_LD_A_BCR);
    tcase_add_test(tcld0, test_LD_A_CR);
    tcase_add_test(tcld0, test_LD_A_DER);
    tcase_add_test(tcld0, test_LD_A_HLRU);
    tcase_add_test(tcld0, test_LD_A_HLR);
    tcase_add_test(tcld0, test_LD_A_N16R);
    tcase_add_test(tcld0, test_LD_A_N8R);
    tcase_add_test(tcld0, test_LD_R16SP_N16);
    tcase_add_test(tcld0, test_LD_R8_N8);
    tcase_add_test(tcld0, test_LD_R8_HLR);

    Add_Case(s, tcld1, "Cpu Dispatch LD STORE Tests");
    tcase_add_test(tcld1, test_LD_BCR_A);
    tcase_add_test(tcld1, test_LD_CR_A);
    tcase_add_test(tcld1, test_LD_DER_A);
    tcase_add_test(tcld1, test_LD_HLRU_A);
    tcase_add_test(tcld1, test_LD_HLR_N8);
    tcase_add_test(tcld1, test_LD_HLR_R8);
    tcase_add_test(tcld1, test_LD_N16R_A);
    tcase_add_test(tcld1, test_LD_N16R_SP);

    Add_Case(s, tcld2, "Cpu Dispatch LD SP-PP Tests");
    tcase_add_test(tcld2, test_PUSH_R16);
    tcase_add_test(tcld2, test_POP_R16);
    tcase_add_test(tcld2, test_PUSH_POP_R16);

    Add_Case(s, tcld3, "Cpu Dispatch LD MISC Tests");
    tcase_add_test(tcld3, test_LD_SP_HL);
    tcase_add_test(tcld3, test_LD_R8_R8);

    Add_Case(s, tc99, "Cpu Dispatch OTHER Tests");
    tcase_add_test(tc99, test_STOP); // Doing Nothing
    tcase_add_test(tc99, test_NOP);  // Doing Nothing

    return s;
}
TEST_SUITE(cpu_test_suite)
