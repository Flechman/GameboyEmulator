
// for thread-safe randomization
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

#include <check.h>
#include <inttypes.h>
#include <assert.h>

#include "tests.h"
#include "bus.h"
#include "memory.h"
#include "component.h"
#include "error.h"

START_TEST(component_create_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    component_t c = {(memory_t *) 0xdeadbeef, 1, 2};

    component_create(&c, 0);
    ck_assert(c.mem == NULL);
    ck_assert(c.start == 0);
    ck_assert(c.end == 0);

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(component_create_free_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    component_t c = {NULL, 0, 0};

    const size_t s = 2;
    ck_assert_int_eq(component_create(&c, s), ERR_NONE);
    ck_assert(c.mem != NULL);
    ck_assert(c.start == 0);
    ck_assert(c.end == 0);
    ck_assert(c.mem->size == s);
    ck_assert(c.mem->memory != NULL);

    c.start = 42;
    c.end = (addr_t)(c.start + s - 1);
    component_free(&c);
    ck_assert(c.mem == NULL);
    ck_assert(c.start == 0);
    ck_assert(c.end == 0);

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

    Suite* s = suite_create("component.c Tests");

    Add_Case(s, tc2, "component tests");

    tcase_add_test(tc2, component_create_err);
    tcase_add_test(tc2, component_create_free_exec);

    return s;
}

TEST_SUITE(bus_test_suite)
