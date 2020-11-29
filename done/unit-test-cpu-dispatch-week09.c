

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

//================================================================
//                  PC JUMP
//================================================================
#define RET_CC_INST 4

START_TEST(test_JP_HL)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint16_t old_pcv = 0xF00D;
    const uint16_t pcvr[] = {0xdead, 0xbeef, 0x0000, 0x00ff};
    const uint8_t pcv1[] = {  0xad,   0xef,   0x00,   0xff};
    const uint8_t pcv2[] = {  0xde,   0xbe,   0x00,   0x00};
    ASSERT_EQ_NB_EL(pcvr, pcv1);
    ASSERT_EQ_NB_EL(pcvr, pcv2);

    INIT_RUN();
    LOOP_ON(pcvr) {
        cpu.PC = old_pcv;
        cpu.L = pcv1[i_];
        cpu.H = pcv2[i_];
        DO_RUN(cpu, OP_JP_HL);
        ck_assert_int_eq(cpu.PC, pcvr[i_]);
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_JP_CC_N16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint16_t old_pcv = 0xdeaa;
    const uint8_t pcv1 = 0xef;
    const uint8_t pcv2 = 0xbe;

    const uint8_t input_f[]    = {  0x00,   0x10,   0x20,   0x30,   0x40,   0x50,   0x60,   0x70,   0x80,   0x90,   0xA0,   0xB0,   0xC0,   0xD0,   0xE0,   0xF0};
    const uint16_t output_c[]  = {0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef};
    const uint16_t output_nc[] = {0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead};
    const uint16_t output_z[]  = {0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef};
    const uint16_t output_nz[] = {0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead};

    ASSERT_EQ_NB_EL(input_f, output_c);
    ASSERT_EQ_NB_EL(input_f, output_nc);
    ASSERT_EQ_NB_EL(input_f, output_z);
    ASSERT_EQ_NB_EL(input_f, output_nz);

    const instruction_t insts[RET_CC_INST] = {OP_JP_C_N16, OP_JP_NC_N16, OP_JP_Z_N16, OP_JP_NZ_N16};
    const uint16_t* outputs[RET_CC_INST] = {output_c, output_nc, output_z, output_nz};

    INIT_RUN();
    cpu.SP = 0xFFFF;

    for (size_t i = 0; i < RET_CC_INST; ++i) {
        LOOP_ON(input_f) {
            cpu.PC = old_pcv;
            cpu.F = input_f[i_];
            CPU_BUS_V_AT(cpu, cpu.PC + 1) = pcv1;
            CPU_BUS_V_AT(cpu, cpu.PC + 2) = pcv2;
            DO_RUN(cpu, insts[i]);
            ck_assert_msg(cpu.PC == outputs[i][i_], "On itr(%u, %u) with (0x%" PRIX8 "), got PC= 0x%" PRIX16" instead of 0x%" PRIX16 " !", i, i_, input_f[i_], cpu.PC, outputs[i][i_]);
        }
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_JP_N16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint16_t old_pcv = 0xF00D;
    const uint16_t pcvr[] = {0xdead, 0xbeef, 0x0000, 0x00ff};
    const uint8_t pcv1[] = {  0xad,   0xef,   0x00,   0xff};
    const uint8_t pcv2[] = {  0xde,   0xbe,   0x00,   0x00};
    ASSERT_EQ_NB_EL(pcvr, pcv1);
    ASSERT_EQ_NB_EL(pcvr, pcv2);

    INIT_RUN();
    LOOP_ON(pcvr) {
        cpu.PC = old_pcv;
        CPU_BUS_V_AT(cpu, cpu.PC + 1) = pcv1[i_];
        CPU_BUS_V_AT(cpu, cpu.PC + 2) = pcv2[i_];
        DO_RUN(cpu, OP_JP_N16);
        ck_assert_int_eq(cpu.PC, pcvr[i_]);
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_JR_CC_E8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint16_t old_pcv = 0xdeab;
    const uint8_t pcv1 = 0x81;

    const uint8_t input_f[]    = {  0x00,   0x10,   0x20,   0x30,   0x40,   0x50,   0x60,   0x70,   0x80,   0x90,   0xA0,   0xB0,   0xC0,   0xD0,   0xE0,   0xF0};
    const uint16_t output_c[]  = {0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e};
    const uint16_t output_nc[] = {0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead, 0xde2e, 0xdead};
    const uint16_t output_z[]  = {0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xde2e, 0xde2e, 0xde2e, 0xde2e, 0xde2e, 0xde2e, 0xde2e, 0xde2e};
    const uint16_t output_nz[] = {0xde2e, 0xde2e, 0xde2e, 0xde2e, 0xde2e, 0xde2e, 0xde2e, 0xde2e, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead};

    ASSERT_EQ_NB_EL(input_f, output_c);
    ASSERT_EQ_NB_EL(input_f, output_nc);
    ASSERT_EQ_NB_EL(input_f, output_z);
    ASSERT_EQ_NB_EL(input_f, output_nz);

    const instruction_t insts[RET_CC_INST] = {OP_JR_C_E8, OP_JR_NC_E8, OP_JR_Z_E8, OP_JR_NZ_E8};
    const uint16_t* outputs[RET_CC_INST] = {output_c, output_nc, output_z, output_nz};

    INIT_RUN();
    cpu.SP = 0xFFFF;

    for (size_t i = 0; i < RET_CC_INST; ++i) {
        LOOP_ON(input_f) {
            cpu.PC = old_pcv;
            cpu.F = input_f[i_];
            CPU_BUS_V_AT(cpu, cpu.PC + 1) = pcv1;

            DO_RUN(cpu, insts[i]);

            ck_assert_msg(cpu.PC == outputs[i][i_], "On itr(%u, %u) with (0x%" PRIX8 "), got PC= 0x%" PRIX16" instead of 0x%" PRIX16 " !", i, i_, input_f[i_], cpu.PC, outputs[i][i_]);
        }
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_JR_E8)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint16_t pcvi[] =   {0x00ff, 0x0000, 0xdead, 0xbeef};
    const uint8_t pcv[] =     {  0xff,   0x01,   0xad,   0xef};
    const uint16_t pcvres[] = {0x0100, 0x0003, 0xde5c, 0xbee0};

    ASSERT_EQ_NB_EL(pcvi, pcv);
    ASSERT_EQ_NB_EL(pcvi, pcvres);

    INIT_RUN();
    LOOP_ON(pcvi) {
        cpu.PC = pcvi[i_];
        CPU_BUS_V_AT(cpu, cpu.PC + 1) = pcv[i_];
        DO_RUN(cpu, OP_JR_E8);
        ck_assert_msg(cpu.PC == pcvres[i_], "On (0x%" PRIX16 " + %d)=> 0x%" PRIX16 " (!= 0x%" PRIX16 ")", pcvi[i_], (int8_t)pcv[i_], (uint16_t)(cpu.PC), pcvres[i_]);
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//================================================================
//                  PC CALL
//================================================================
#define RET_CC_INST 4

START_TEST(test_CALL_CC_N16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint16_t old_pcv = 0xdeaa;
    const uint8_t pcv1 = 0xef;
    const uint8_t pcv2 = 0xbe;

    const uint8_t input_f[]    = {  0x00,   0x10,   0x20,   0x30,   0x40,   0x50,   0x60,   0x70,   0x80,   0x90,   0xA0,   0xB0,   0xC0,   0xD0,   0xE0,   0xF0};
    const uint16_t output_c[]  = {0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef};
    const uint16_t output_nc[] = {0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead};
    const uint16_t output_z[]  = {0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef};
    const uint16_t output_nz[] = {0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead};

    ASSERT_EQ_NB_EL(input_f, output_c);
    ASSERT_EQ_NB_EL(input_f, output_nc);
    ASSERT_EQ_NB_EL(input_f, output_z);
    ASSERT_EQ_NB_EL(input_f, output_nz);

    const instruction_t insts[RET_CC_INST] = {OP_CALL_C_N16, OP_CALL_NC_N16, OP_CALL_Z_N16, OP_CALL_NZ_N16};
    const uint16_t* outputs[RET_CC_INST] = {output_c, output_nc, output_z, output_nz};

    INIT_RUN();
    cpu.SP = 0xFFFF;

    for (size_t i = 0; i < RET_CC_INST; ++i) {
        LOOP_ON(input_f) {
            cpu.PC = old_pcv;
            cpu.F = input_f[i_];
            CPU_BUS_V_AT(cpu, cpu.PC + 1) = pcv1;
            CPU_BUS_V_AT(cpu, cpu.PC + 2) = pcv2;
            DO_RUN(cpu, insts[i]);
            ck_assert_msg(cpu.PC == outputs[i][i_], "On itr(%u, %u) with (0x%" PRIX8 "), got PC= 0x%" PRIX16" instead of 0x%" PRIX16 " !", i, i_, input_f[i_], cpu.PC, outputs[i][i_]);

            if (cpu.PC != old_pcv + 3) {
                ck_assert_int_eq(cpu_SP_pop(&cpu), old_pcv + 3);
            }
        }
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_CALL_N16)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint16_t old_pcv = 0xF00D;
    const uint16_t pcvr[] = {0xdead, 0xbeef, 0x0000, 0x00ff};
    const uint8_t pcv1[] = {  0xad,   0xef,   0x00,   0xff};
    const uint8_t pcv2[] = {  0xde,   0xbe,   0x00,   0x00};
    ASSERT_EQ_NB_EL(pcvr, pcv1);
    ASSERT_EQ_NB_EL(pcvr, pcv2);

    INIT_RUN();
    LOOP_ON(pcvr) {
        cpu.PC = old_pcv;
        CPU_BUS_V_AT(cpu, cpu.PC + 1) = pcv1[i_];
        CPU_BUS_V_AT(cpu, cpu.PC + 2) = pcv2[i_];
        DO_RUN(cpu, OP_CALL_N16);
        ck_assert_int_eq(cpu.PC, pcvr[i_]);
        ck_assert_int_eq(cpu_SP_pop(&cpu), old_pcv + 3);
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


//================================================================
//                  PC RET
//================================================================
#define RET_CC_INST 4

START_TEST(test_RET_CC)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint16_t pcv = 0xdeac;
    const uint16_t pcvs = 0xbeef;

    const uint8_t input_f[]    = {  0x00,   0x10,   0x20,   0x30,   0x40,   0x50,   0x60,   0x70,   0x80,   0x90,   0xA0,   0xB0,   0xC0,   0xD0,   0xE0,   0xF0};
    const uint16_t output_c[]  = {0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef};
    const uint16_t output_nc[] = {0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead, 0xbeef, 0xdead};
    const uint16_t output_z[]  = {0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef};
    const uint16_t output_nz[] = {0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xbeef, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead, 0xdead};

    ASSERT_EQ_NB_EL(input_f, output_c);
    ASSERT_EQ_NB_EL(input_f, output_nc);
    ASSERT_EQ_NB_EL(input_f, output_z);
    ASSERT_EQ_NB_EL(input_f, output_nz);

    const instruction_t insts[4] = {OP_RET_C, OP_RET_NC, OP_RET_Z, OP_RET_NZ};
    const uint16_t* outputs[4] = {output_c, output_nc, output_z, output_nz};

    INIT_RUN();

    cpu.SP = 0xFFFF;

    for (size_t i = 0; i < RET_CC_INST; ++i) {
        LOOP_ON(input_f) {
            cpu.PC = pcv;
            cpu.F = input_f[i_];
            cpu_SP_push(&cpu, pcvs);
            DO_RUN(cpu, insts[i]);
            ck_assert_msg(cpu.PC == outputs[i][i_], "On itr(%u, %u) with (0x%" PRIX8 "), got PC= 0x%" PRIX16" instead of 0x%" PRIX16 " !", i, i_, input_f[i_], cpu.PC, outputs[i][i_]);
        }
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_RET)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();

    const uint16_t pcv[] = {0x000F, 0xdead, 0xbeef, 0xc0fe, 0xd1ce, 0xc001};
    LOOP_ON(pcv) {
        cpu_SP_push(&cpu, pcv[i_]);
        DO_RUN(cpu, OP_RET);
        ck_assert_int_eq(cpu.PC, pcv[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

//================================================================
//                  PC MISC
//================================================================

START_TEST(test_RST_U3)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    const uint16_t pcv = 0x000F;

    const instruction_t insts[] = {OP_RST_0, OP_RST_1, OP_RST_2, OP_RST_3, OP_RST_4, OP_RST_5, OP_RST_6, OP_RST_7};
    const uint16_t rst_addr[] = {0x0000, 0x0008, 0x0010, 0x0018, 0x0020, 0x0028, 0x0030, 0x0038};

    ASSERT_EQ_NB_EL(insts, rst_addr);

    INIT_RUN();
    cpu.SP = 0xFFFF;
    LOOP_ON(insts) {
        cpu.PC = pcv;

        DO_RUN(cpu, insts[i_]);

        ck_assert_int_eq(cpu.PC, rst_addr[i_]);
        ck_assert_int_eq(pcv + 1, cpu_SP_pop(&cpu));
    }
    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_EDI)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();

    const uint8_t order[] = {0, 1, 0, 0, 1, 1, 0};

    LOOP_ON(order) {
        if (order[i_] == 0) {
            DO_RUN(cpu, OP_DI);
        } else {
            DO_RUN(cpu, OP_EI);
        }

        ck_assert_int_eq(cpu.IME, order[i_]);
    }

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_RETI)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    const uint16_t pcv = 0xdead;
    cpu.SP = 0xFFFF;
    cpu_SP_push(&cpu, pcv);

    DO_RUN(cpu, OP_RETI);

    ck_assert_int_eq(cpu.IME, 1);
    ck_assert_int_eq(cpu.PC, pcv);

    END_RUN();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_HALT)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT_RUN();
    DO_RUN(cpu, OP_HALT);
    ck_assert_int_eq(cpu.HALT, 1);
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

    Add_Case(s, tcflow0, "Cpu Dispatch PC JP Tests");
    tcase_add_test(tcflow0, test_JP_HL);
    tcase_add_test(tcflow0, test_JP_CC_N16);
    tcase_add_test(tcflow0, test_JP_N16);
    tcase_add_test(tcflow0, test_JR_CC_E8);
    tcase_add_test(tcflow0, test_JR_E8);

    Add_Case(s, tcflow1, "Cpu Dispatch PC CALL Tests");
    tcase_add_test(tcflow1, test_CALL_N16);
    tcase_add_test(tcflow1, test_CALL_CC_N16);

    Add_Case(s, tcflow2, "Cpu Dispatch PC RET Tests");
    tcase_add_test(tcflow2, test_RET);
    tcase_add_test(tcflow2, test_RET_CC);

    Add_Case(s, tcflow3, "Cpu Dispatch PC MISC Tests");
    tcase_add_test(tcflow3, test_RST_U3);
    tcase_add_test(tcflow3, test_EDI);
    tcase_add_test(tcflow3, test_RETI);
    tcase_add_test(tcflow3, test_HALT);

    return s;
}
TEST_SUITE(cpu_test_suite)
