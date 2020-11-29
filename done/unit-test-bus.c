

// for thread-safe randomization
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#ifdef WITH_PRINT
#include <stdio.h>
#endif

#include <check.h>
#include <inttypes.h>
#include <assert.h>

#include "tests.h"
#include "bus.h"
#include "component.h"
#include "error.h"
#include "util.h"

#define INIT \
    bus_t bus; \
    zero_init_var(bus); \
    component_t c; \
    zero_init_var(c)

START_TEST(bus_plug_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    INIT;
    ck_assert_int_eq(bus_plug(bus, NULL, 0, 0), ERR_BAD_PARAMETER);
    const int err = bus_plug(bus, &c, 0, 0);
    ck_assert((err == ERR_BAD_PARAMETER) || (err == ERR_ADDRESS));

    ck_assert_int_eq(component_create(&c, 2), ERR_NONE);

    ck_assert_int_eq(bus_plug(bus, &c, 0, 5), ERR_ADDRESS); // range too large
    ck_assert_int_eq(bus_plug(bus, &c, 0, 1), ERR_NONE);
    ck_assert_int_eq(bus_plug(bus, &c, 0, 1), ERR_ADDRESS); // since we plugged already

    ck_assert_int_eq(bus_unplug(bus, NULL), ERR_BAD_PARAMETER);
    component_free(&c);

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST

START_TEST(bus_plug_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    size_t c_size = 255;
    data_t data = 0xF;
    INIT;
    ck_assert_int_eq(component_create(&c, c_size + 1), ERR_NONE);

    ck_assert_int_eq(bus_plug(bus, &c, 0, (addr_t) c_size), ERR_NONE);
    ck_assert(c.start == 0);
    ck_assert(c.end == c_size);

    for (size_t i = 0; i < c_size; ++i) {
        ck_assert(bus[i] == c.mem->memory + i);
        ck_assert(*bus[i] == 0);
        *bus[i] = data;
        ck_assert(c.mem->memory[i] == data);
    }

    ck_assert_int_eq(bus_unplug(bus, &c), ERR_NONE);
    ck_assert(c.start == 0);
    ck_assert(c.end == 0);

    for (size_t i = 0; i < c_size; ++i) {
        ck_assert(bus[i] == NULL);
    }

    component_free(&c);

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(bus_read_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    size_t c_size = 255;
    data_t data = 0xF;
    INIT;
    ck_assert_int_eq(component_create(&c, c_size + 1), ERR_NONE);

    ck_assert_int_eq(bus_read(bus, 0, &data), ERR_NONE);
    // ck_assert_int_eq(data, 0xFF); // Not necessary (undefined behaviour)

    ck_assert_int_eq(bus_plug(bus, &c, 0, (addr_t)c_size), ERR_NONE);

    ck_assert_int_eq(bus_read(bus, 0, NULL), ERR_BAD_PARAMETER);

    component_free(&c);

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(bus_read_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    size_t c_size = 255;
    data_t data = 0xF;
    INIT;
    ck_assert_int_eq(component_create(&c, c_size + 1), ERR_NONE);

    ck_assert_int_eq(bus_plug(bus, &c, 0, (addr_t)c_size), ERR_NONE);

    for (size_t i = 0; i < c_size; ++i) {
        *bus[i] = (data_t)i;
    }

    for (size_t addr = 0; addr < c_size; ++addr) {
        ck_assert_int_eq(bus_read(bus, (addr_t) addr, &data), ERR_NONE);
        ck_assert(data == (data_t)addr);
    }

    component_free(&c);

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(bus_write_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    size_t c_size = 255;
    data_t data = 0xF;
    INIT;
    ck_assert_int_eq(component_create(&c, c_size + 1), ERR_NONE);

    ck_assert_int_eq(bus_write(bus, 0, data), ERR_BAD_PARAMETER);

    component_free(&c);

#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif
}
END_TEST


START_TEST(bus_write_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    size_t c_size = 255;
    data_t data = 0xE;
    INIT;
    ck_assert_int_eq(component_create(&c, c_size + 1), ERR_NONE);

    ck_assert_int_eq(bus_plug(bus, &c, 0, (addr_t)c_size), ERR_NONE);

    for (size_t i = 0; i < c_size; ++i) {
        *bus[i] = (data_t) i;
    }

    for (size_t addr = 0; addr < c_size; ++addr) {
        ck_assert_int_eq(bus_write(bus, (addr_t) addr, (data_t) addr), ERR_NONE);
        data = 0x4a; // whatever
        ck_assert_int_eq(bus_read(bus, (addr_t) addr, &data), ERR_NONE);
        ck_assert(data == (data_t) addr);
    }

    component_free(&c);

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

    Suite* s = suite_create("bus.c Tests");

    Add_Case(s, tc3, "bus tests");

    tcase_add_test(tc3, bus_plug_err);
    tcase_add_test(tc3, bus_plug_exec);

    tcase_add_test(tc3, bus_read_err);
    tcase_add_test(tc3, bus_read_exec);

    tcase_add_test(tc3, bus_write_err);
    tcase_add_test(tc3, bus_write_exec);

    return s;
}

TEST_SUITE(bus_test_suite)
