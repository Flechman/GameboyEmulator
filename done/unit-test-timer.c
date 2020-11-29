
// for thread-safe randomization
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <check.h>
#include <inttypes.h>

#include "util.h"
#include "tests.h"
#include "timer.h"
#include "cpu.h"
#include "bus.h"

#define INIT \
    gbtimer_t timer; \
    cpu_t cpu; \
    zero_init_var(timer); \
    zero_init_var(cpu)

#define register(X) \
    data_t reg_ ## X ## _var = 0; \
    bus[REG_ ## X] = &reg_ ## X ## _var

#define INIT_BUS \
    bus_t bus; \
    zero_init_var(bus); \
    register(DIV); \
    register(TIMA); \
    register(TMA); \
    register(TAC); \
    cpu.bus = &bus

START_TEST(timer_init_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    ck_assert_bad_param(timer_init(NULL, NULL));
    ck_assert_bad_param(timer_init(NULL, &cpu));
    ck_assert_bad_param(timer_init(&timer, NULL));

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(timer_init_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    ck_assert_err_none(timer_init(&timer, &cpu));
    ck_assert_ptr_eq(timer.cpu, &cpu);
    ck_assert_int_eq(timer.counter, 0);

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(timer_cycle_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_bad_param(timer_cycle(NULL));
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

#define CYCLE_COUNT_3FFF 0x3FFF
#define CYCLE_COUNT_3FFF_VALUE 0xFFFC
#define CYCLE_TAC_VALUE 0x4
#define CYCLE_TIMA_VALUE 0x3F
#define CYCLE_TMA_VALUE 0x0
#define CYCLE_DIV_VALUE 0xFF

START_TEST(timer_cycle_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    ck_assert_err_none(timer_init(&timer, &cpu));

    INIT_BUS;
    *bus[REG_TAC] = CYCLE_TAC_VALUE;

    for (size_t i = 0; i < CYCLE_COUNT_3FFF; ++i) { //do many cycles and check values
        timer_cycle(&timer);
    }

    ck_assert_int_eq(timer.counter, CYCLE_COUNT_3FFF_VALUE);
    ck_assert_int_eq(*bus[REG_TAC], CYCLE_TAC_VALUE );
    ck_assert_int_eq(*bus[REG_TIMA], CYCLE_TIMA_VALUE);
    ck_assert_int_eq(*bus[REG_TMA], CYCLE_TMA_VALUE );
    ck_assert_int_eq(*bus[REG_DIV], CYCLE_DIV_VALUE );
    ck_assert_int_eq(cpu.IF, 0);

    for (size_t i = 0; i < 3 * CYCLE_COUNT_3FFF + 4; ++i) { //cycle until interruption occurs
        timer_cycle(&timer);
    }

    ck_assert_int_eq(cpu.IF, 0x4);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(timer_listener_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    ck_assert_bad_param(timer_bus_listener(NULL, 0x00));
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(timer_listener_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    ck_assert_err_none(timer_init(&timer, &cpu));

    INIT_BUS;

    timer.counter = 0xFF;
    ck_assert_err_none(timer_bus_listener(&timer, REG_DIV));
    ck_assert_int_eq(timer.counter, 0);
    ck_assert_int_eq(*bus[REG_DIV], 0);

    ck_assert_err_none(timer_bus_listener(&timer, REG_TAC));

    //TODO: check also timer_incr_if_state_change

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


// ======================================================================
Suite* timer_test_suite()
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
    srand(time(NULL) ^ getpid() ^ pthread_self());
#pragma GCC diagnostic pop

    Suite* s = suite_create("timer.c Tests");

    Add_Case(s, tc1, "Timer Tests");
    tcase_add_test(tc1, timer_init_err);
    tcase_add_test(tc1, timer_init_exec);

    tcase_add_test(tc1, timer_cycle_err);
    tcase_add_test(tc1, timer_cycle_exec);
    tcase_add_test(tc1, timer_listener_err);
    tcase_add_test(tc1, timer_listener_exec);

    return s;
}

TEST_SUITE(timer_test_suite)
