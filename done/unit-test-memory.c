
// for thread-safe randomization
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <check.h>
#include <inttypes.h>

#include "tests.h"
#include "bus.h"
#include "memory.h"
#include "component.h"
#include "error.h"

START_TEST(mem_create_free_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    memory_t mem = {0};

    ck_assert_int_eq(mem_create(NULL, 1), ERR_BAD_PARAMETER);
    ck_assert(mem.memory == NULL);
    ck_assert(mem.size == 0);
    mem_free(&mem);

    ck_assert_int_eq(mem_create(NULL, 1), ERR_BAD_PARAMETER);
    ck_assert(mem.memory == NULL);
    ck_assert(mem.size == 0);
    mem_free(&mem);

    ck_assert_int_eq(mem_create(&mem, (size_t)(-1)), ERR_MEM);
    ck_assert(mem.memory == NULL);
    ck_assert(mem.size == 0);
    mem_free(&mem);

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(mem_create_free_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    memory_t mem = {0};

    ck_assert_int_eq(mem_create(&mem, 1), ERR_NONE);
    ck_assert(mem.size == 1);
    mem_free(&mem);
    ck_assert(mem.memory == NULL);
    ck_assert(mem.size == 0);
    ck_assert_int_eq(mem_create(&mem, 16), ERR_NONE);
    ck_assert(mem.size == 16);
    mem_free(&mem);
    ck_assert(mem.memory == NULL);
    ck_assert(mem.size == 0);

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

    Suite* s = suite_create("memory.c Tests");

    Add_Case(s, tc1, "mem tests");
    tcase_add_test(tc1, mem_create_free_err);
    tcase_add_test(tc1, mem_create_free_exec);

    return s;
}

TEST_SUITE(bus_test_suite)
