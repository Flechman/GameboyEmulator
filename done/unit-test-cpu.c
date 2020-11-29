
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
#include "opcode.h"
#include "bit.h"
#include "util.h"
#include "error.h"

#include "cpu.h"
#include "cpu-registers.h"
#include "cpu-storage.h"
#include "cpu-alu.h"

// ------------------------------------------------------------
#define LOOP_ON(T) const size_t s_ = sizeof(T) / sizeof(*T);  \
  for(size_t i_ = 0; i_ < s_; ++i_)

#define ASSERT_EQ_NB_EL(T1, T2) \
    static_assert(sizeof(T1) / sizeof(*T1) == sizeof(T2) / sizeof(*T2), "Wrong Size in test tables")

#define CPU_BUS_V_AT(cpu,idx) \
        *((*(cpu).bus)[idx])

#define add_bus(cpu,size)\
    bus_t bus = {0}; \
    component_t c = {NULL, 0, 0};\
    do{ \
        ck_assert_int_eq(component_create(&c, size), ERR_NONE);         \
        ck_assert_int_eq(cpu_init(&cpu), ERR_NONE);                     \
        ck_assert_int_eq(cpu_plug(&cpu,&bus), ERR_NONE);                \
        ck_assert_int_eq(bus_forced_plug(bus, &c, 0, (addr_t)(size-1), 0), ERR_NONE); \
    } while(0)

#define finish() \
    do{ \
        component_free(&c); \
        cpu_free(&cpu); \
    } while(0)

#define FOR_EACH_REG(cpu, regp, regk)\
    uint8_t* regp_list[]= {  &(cpu.A),   &(cpu.B),   &(cpu.C),   &(cpu.D),   &(cpu.E),   &(cpu.H),  &(cpu.L)};\
    uint8_t regk_list[]=  {REG_A_CODE, REG_B_CODE, REG_C_CODE, REG_D_CODE, REG_E_CODE, REG_H_CODE, REG_L_CODE};\
    const size_t s2_ = sizeof(regp_list) / sizeof(*regp_list);  \
    for(size_t j_ = 0; j_ < s2_ && (regp = regp_list[j_]) && (regk = regk_list[j_]); ++j_)

#define FILL_REG(cpu,a,b,c,d,e,f,h,l)\
    do{\
        cpu.A = a;\
        cpu.B = b;\
        cpu.C = c;\
        cpu.D = d;\
        cpu.E = e;\
        cpu.F = f;\
        cpu.H = h;\
        cpu.L = l;\
    }while(0)

#define INIT \
    cpu_t cpu; \
    zero_init_var(cpu)


START_TEST(test_reg_get)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    uint8_t vt[] = {0x00, 0x10, 0xFF, 0xdd, 0x55};

    uint8_t* regp = NULL;
    uint8_t regk = 0;

    LOOP_ON(vt) {
        uint8_t v = vt[i_];
        FOR_EACH_REG(cpu, regp, regk) {
            *regp = v;
            ck_assert_int_eq(v, cpu_reg_get(&cpu, regk));
        }
    }
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_reg_set)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    uint8_t vt[] = {0x00, 0x10, 0xFF, 0xdd, 0x55};

    uint8_t* regp = NULL;
    uint8_t regk = 0;

    LOOP_ON(vt) {
        uint8_t v = vt[i_];
        FOR_EACH_REG(cpu, regp, regk) {
            cpu_reg_set(&cpu, regk, v);
            ck_assert_int_eq(v, *regp);
        }
    }
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_reg_pair_get)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    uint8_t vt1[] =  {  0x00,   0x10,   0xFF,   0xde,   0x55,   0x01,   0x00};
    uint8_t vt2[] =  {  0x00,   0x10,   0xFF,   0xad,   0x55,   0x01,   0x01};
    const uint16_t vtm[] = {0x0000, 0x1010, 0xFFFF, 0xdead, 0x5555, 0x0101, 0x0001};

    ASSERT_EQ_NB_EL(vt1, vt2);
    ASSERT_EQ_NB_EL(vt1, vtm);

    LOOP_ON(vt1) {
        uint8_t v1 = vt1[i_];
        uint8_t v2 = vt2[i_];
        cpu.B = v1;
        cpu.C = v2;
        ck_assert_int_eq(vtm[i_], cpu_reg_pair_get(&cpu, REG_BC_CODE));

        cpu.D = v1;
        cpu.E = v2;
        ck_assert_int_eq(vtm[i_], cpu_reg_pair_get(&cpu, REG_DE_CODE));

        cpu.H = v1;
        cpu.L = v2;
        ck_assert_int_eq(vtm[i_], cpu_reg_pair_get(&cpu, REG_HL_CODE));

        cpu.A = v1;
        cpu.F = v2;
        ck_assert_int_eq(vtm[i_], cpu_reg_pair_get(&cpu, REG_AF_CODE));
    }
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_reg_pair_set)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    uint8_t vt1[] =  {  0x00,   0x10,   0xFF,   0xde,   0x55,   0x01,   0x00};
    uint8_t vt2[] =  {  0x00,   0x10,   0xFF,   0xad,   0x55,   0x01,   0x01};
    uint8_t vt2f[] = {  0x00,   0x10,   0xF0,   0xa0,   0x50,   0x00,   0x00};
    const uint16_t vtm[] = {0x0000, 0x1010, 0xFFFF, 0xdead, 0x5555, 0x0101, 0x0001};

    ASSERT_EQ_NB_EL(vt1, vt2);
    ASSERT_EQ_NB_EL(vt1, vt2f);
    ASSERT_EQ_NB_EL(vt1, vtm);

    LOOP_ON(vt1) {
        uint8_t v1 = vt1[i_];
        uint8_t v2 = vt2[i_];

        cpu_reg_pair_set(&cpu, REG_BC_CODE, vtm[i_]);
        ck_assert_int_eq(cpu.B, v1);
        ck_assert_int_eq(cpu.C, v2);

        cpu_reg_pair_set(&cpu, REG_DE_CODE, vtm[i_]);
        ck_assert_int_eq(cpu.D, v1);
        ck_assert_int_eq(cpu.E, v2);

        cpu_reg_pair_set(&cpu, REG_HL_CODE, vtm[i_]);
        ck_assert_int_eq(cpu.H, v1);
        ck_assert_int_eq(cpu.L, v2);

        cpu_reg_pair_set(&cpu, REG_AF_CODE, vtm[i_]);
        ck_assert_int_eq(cpu.A, v1);
        ck_assert_int_eq(cpu.F, vt2f[i_]);
    }
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_cpu_init_err)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    ck_assert_int_eq(cpu_init(NULL), ERR_BAD_PARAMETER);
    ck_assert_int_eq(cpu_init(&cpu), ERR_NONE);
    cpu_free(&cpu);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_cpu_init_exec)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    FILL_REG(cpu, 1, 1, 1, 1, 1, 1, 1, 1);
    cpu.PC = 0x1515;
    cpu.SP = 0xdead;
    cpu.alu.flags = 0x22;
    cpu.alu.value = 0x7777;

    ck_assert_int_eq(cpu_init(&cpu), ERR_NONE);
    ck_assert_int_eq(cpu.A, 0);
    ck_assert_int_eq(cpu.B, 0);
    ck_assert_int_eq(cpu.C, 0);
    ck_assert_int_eq(cpu.D, 0);
    ck_assert_int_eq(cpu.E, 0);
    ck_assert_int_eq(cpu.F, 0);
    ck_assert_int_eq(cpu.H, 0);
    ck_assert_int_eq(cpu.L, 0);

    ck_assert_int_eq(cpu.PC, 0);
    ck_assert_int_eq(cpu.SP, 0);

    ck_assert_int_eq(cpu.alu.flags, 0);
    ck_assert_int_eq(cpu.alu.value, 0);

    cpu_free(&cpu);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_cpu_plug_err)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    INIT;
    cpu_init(&cpu);
    bus_t bus = {0};
    ck_assert_int_eq(cpu_plug(NULL, &bus), ERR_BAD_PARAMETER);
    ck_assert_int_eq(cpu_plug(&cpu, NULL), ERR_BAD_PARAMETER);
    ck_assert_int_eq(cpu_plug(&cpu, &bus), ERR_NONE);
    cpu_free(&cpu);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_cpu_plug_exec)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
    INIT;
    bus_t bus = {0};
    cpu_init(&cpu);
    cpu_plug(&cpu, &bus);
    ck_assert_int_eq(cpu.bus, &bus);
    bus[0] = 0xdd;
    ck_assert_int_eq((*cpu.bus)[0], bus[0]);
#pragma GCC diagnostic pop
    cpu_free(&cpu);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_cpu_read_at_idx)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    size_t size = 128;
    add_bus(cpu, size);

    for (size_t i = 0; i < size; ++i) {
        CPU_BUS_V_AT(cpu, i) = (data_t)i;
        ck_assert_int_eq(cpu_read_at_idx(&cpu, (addr_t)i), (data_t)i);
    }

    finish();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_cpu_read16_at_idx)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    size_t size = 255;
    add_bus(cpu, size);

    for (size_t i = 0; i < size - 1; ++i) {
        CPU_BUS_V_AT(cpu, i) = 0xad;
        CPU_BUS_V_AT(cpu, i + 1) = 0xde;
        ck_assert_int_eq(cpu_read16_at_idx(&cpu, (addr_t)i), 0xdead);
    }

    finish();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_cpu_write_at_idx)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    size_t size = 255;
    add_bus(cpu, size);

    for (size_t i = 0; i < size; ++i) {
        ck_assert_int_eq(cpu_write_at_idx(&cpu, (addr_t)i, (data_t)i), ERR_NONE);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, i), (data_t)i);
    }

    finish();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_cpu_write16_at_idx)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    size_t size = 255;
    add_bus(cpu, size);

    for (size_t i = 0; i < size - 1; ++i) {
        ck_assert_int_eq(cpu_write16_at_idx(&cpu, (addr_t)i, 0xdead), ERR_NONE);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, i), 0xad);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, i + 1), 0xde);
    }

    finish();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_cpu_bus_HL_macro)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    size_t size = 255;
    add_bus(cpu, size);

    for (size_t i = 0; i < size - 1; ++i) {
        CPU_BUS_V_AT(cpu, i) = (data_t) i;
        cpu_reg_pair_set(&cpu, REG_HL_CODE, (uint8_t) i);
        ck_assert_int_eq(cpu_read_at_HL(&cpu), (data_t) i);

        ck_assert_int_eq(cpu_write_at_HL(&cpu, (data_t)(i + 1)), ERR_NONE);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, i), (data_t)(i + 1));
    }

    finish();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_cpu_bus_after_op_macro)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    size_t size = 255;
    add_bus(cpu, size);

    for (size_t i = 1; i < size - 1; ++i) {
        CPU_BUS_V_AT(cpu, i) = (data_t) i;
        cpu.PC = (uint16_t)(i - 1);
        ck_assert_int_eq(cpu_read_data_after_opcode(&cpu), (data_t) i);

        CPU_BUS_V_AT(cpu, i) = 0xad;
        CPU_BUS_V_AT(cpu, i + 1) = 0xde;
        ck_assert_int_eq(cpu_read_addr_after_opcode(&cpu), 0xdead);

    }

    finish();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_cpu_sp_exec)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    size_t size = 255;
    add_bus(cpu, size);

    cpu.SP = (uint16_t)(size - 1);

    for (size_t i = size - 1; i >= 1; i -= 2) {
        ck_assert_int_eq(cpu.SP, (uint16_t)(i));
        cpu_SP_push(&cpu, 0xdead);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, i - 1), 0xde);
        ck_assert_int_eq(CPU_BUS_V_AT(cpu, i - 2), 0xad);
        ck_assert_int_eq(cpu.SP, (uint16_t)(i - 2));
    }

    for (size_t i = 0; i < (size - 1); i += 2) {
        ck_assert_int_eq(cpu.SP, (uint16_t)(i));
        ck_assert_int_eq(cpu_SP_pop(&cpu), 0xdead);
        ck_assert_int_eq(cpu.SP, (uint16_t)(i + 2));
    }

    finish();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(test_cpu_cycle_err)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    size_t size = 255;
    add_bus(cpu, size);

    ck_assert_int_eq(cpu_cycle(NULL), ERR_BAD_PARAMETER);
    ck_assert_int_eq(cpu_cycle(&cpu), ERR_NONE);


    finish();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(test_cpu_cycle_exec)
{
    // ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    size_t size = 255;
    add_bus(cpu, size);

    ck_assert_int_eq(cpu_cycle(&cpu), ERR_NONE);

    finish();
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


Suite* cpu_test_suite()
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
    srand(time(NULL) ^ getpid() ^ pthread_self());
#pragma GCC diagnostic pop

    Suite* s = suite_create("cpu.c Tests");

    Add_Case(s, tc1, "Cpu Reg S/G Tests");
    tcase_add_test(tc1, test_reg_get);
    tcase_add_test(tc1, test_reg_set);
    tcase_add_test(tc1, test_reg_pair_get);
    tcase_add_test(tc1, test_reg_pair_set);

    Add_Case(s, tc2, "Cpu Start Tests");
    tcase_add_test(tc2, test_cpu_init_err);
    tcase_add_test(tc2, test_cpu_init_exec);

    Add_Case(s, tc3, "Cpu Bus Tests");
    tcase_add_test(tc3, test_cpu_plug_err);
    tcase_add_test(tc3, test_cpu_plug_exec);

    Add_Case(s, tc4, "Cpu Bus R/W Tests");
    tcase_add_test(tc4, test_cpu_read_at_idx);
    tcase_add_test(tc4, test_cpu_read16_at_idx);
    tcase_add_test(tc4, test_cpu_write_at_idx);
    tcase_add_test(tc4, test_cpu_write16_at_idx);
    tcase_add_test(tc4, test_cpu_bus_HL_macro);
    tcase_add_test(tc4, test_cpu_bus_after_op_macro);
    tcase_add_test(tc4, test_cpu_sp_exec);
    tcase_add_test(tc4, test_cpu_sp_exec);

    Add_Case(s, tc5, "Cpu Cycle Tests");
    tcase_add_test(tc5, test_cpu_cycle_err);
    tcase_add_test(tc5, test_cpu_cycle_exec);

    return s;
}

TEST_SUITE(cpu_test_suite)
