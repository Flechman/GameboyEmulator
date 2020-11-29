
// for thread-safe randomization
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <check.h>
#include <inttypes.h>

#include "tests.h"
#include "cartridge.h"
#include "cpu.h"
#include "bus.h"

#define FIBONACCI_ROM "tests/data/fibonacci.gb"

START_TEST(cartridge_init_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    cartridge_t ct = {0};
    const char* fpath = "./file_that_doesnt_exist";
    ck_assert_bad_param(cartridge_init(NULL, NULL));
    ck_assert_bad_param(cartridge_init(&ct, NULL));
    ck_assert_bad_param(cartridge_init(NULL, fpath));
    ck_assert_int_eq(cartridge_init(&ct, fpath), ERR_IO);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

#define FIB_BYTES_SIZE 32
#define FIB_BYTES {0x31, 0xff, 0xff, 0x3e, 0x0b, 0xcd, 0x0a, 0x00, 0x76, 0x00, 0xfe, 0x02,0xd8, 0xc5, 0x3d, 0x47,\
           0xcd, 0x0a, 0x00, 0x4f, 0x78, 0x3d, 0xcd, 0x0a, 0x00, 0x81, 0xc1, 0xc9, 0x00, 0x00, 0x00, 0x00 }

START_TEST(cartridge_init_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    cartridge_t ct = {0};
    uint16_t fb[FIB_BYTES_SIZE] = FIB_BYTES;
    ck_assert_err_none(cartridge_init(&ct, FIBONACCI_ROM));

    for (size_t i = 0; i < FIB_BYTES_SIZE; ++i) {
        ck_assert_int_eq(ct.c.mem->memory[i], fb[i]);
    }

    cartridge_free(&ct);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(cartridge_free_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif

    cartridge_t ct = {0};
    ck_assert_err_none(cartridge_init(&ct, FIBONACCI_ROM));
    cartridge_free(&ct);
    ck_assert_ptr_null(ct.c.mem);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


START_TEST(cartridge_plug_err)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    cartridge_t ct = {0};
    bus_t bus = {0};
    ck_assert_err_none(cartridge_init(&ct, FIBONACCI_ROM));
    ck_assert_bad_param(cartridge_plug(NULL, NULL));
    ck_assert_bad_param(cartridge_plug(&ct, NULL));
    ck_assert_bad_param(cartridge_plug(NULL, bus));
    cartridge_free(&ct);
    ck_assert_bad_param(cartridge_plug(&ct, bus));
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST

START_TEST(cartridge_plug_exec)
{
// ------------------------------------------------------------
#ifdef WITH_PRINT
    printf("=== %s:\n", __func__);
#endif
    cartridge_t ct = {0};
    bus_t bus = {0};
    ck_assert_err_none(cartridge_init(&ct, FIBONACCI_ROM));
    ck_assert_ptr_null(bus[0]);
    ck_assert_err_none(cartridge_plug(&ct, bus));
    ck_assert_ptr_nonnull(bus[0]);
    ck_assert_ptr_eq(bus[0], &(ct.c.mem->memory[0]));

    cartridge_free(&ct);
#ifdef WITH_PRINT
    printf("=== END of %s\n", __func__);
#endif

}
END_TEST


Suite* cartridge_test_suite()
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
    srand(time(NULL) ^ getpid() ^ pthread_self());
#pragma GCC diagnostic pop

    Suite* s = suite_create("cartridge.c Tests");

    Add_Case(s, tc1, "Cartridge Tests");
    tcase_add_test(tc1, cartridge_init_err);
    tcase_add_test(tc1, cartridge_init_exec);
    tcase_add_test(tc1, cartridge_free_exec);
    tcase_add_test(tc1, cartridge_plug_err);
    tcase_add_test(tc1, cartridge_plug_exec);

    return s;
}

TEST_SUITE(cartridge_test_suite)
