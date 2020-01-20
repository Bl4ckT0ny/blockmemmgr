#ifndef MEMMGR_USE_STATIC_POOL
#define MEMMGR_USE_STATIC_POOL
#endif
#include "memmgr.h"
#include <check.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
// some internal hack
#define TEST_HEADER_SIZE (sizeof(void*) + sizeof(size_t) * MEMMGR_CHECK_OVERFLOW)

static volatile bool bOverflowAsserted = false;
void vMemMgrOverflowHook(void* pvMem)
{
    (void)pvMem;
    bOverflowAsserted = true;
}

#define SIMPLE_ALLOC_CHUNK_SIZE 3

START_TEST(test_alloc)
{
    void* pxPtr;
    bOverflowAsserted = false;
    vMemMgrStaticPoolInit(SIMPLE_ALLOC_CHUNK_SIZE);
    pxPtr = pvMemMgrAlloc();
    ck_assert_uint_eq(bOverflowAsserted, 0);
    ck_assert_ptr_nonnull(pxPtr);
}
END_TEST

START_TEST(test_free)
{
    void* pxPtr;
    bOverflowAsserted = false;
    vMemMgrStaticPoolInit(SIMPLE_ALLOC_CHUNK_SIZE);
    pxPtr = pvMemMgrAlloc();
    ck_assert_ptr_nonnull(pxPtr);
    ck_assert_uint_eq(bOverflowAsserted, 0);
    vMemMgrFree(pxPtr);
    ck_assert_uint_eq(bOverflowAsserted, 0);
}
END_TEST

Suite* thread_safe_suite()
{
    Suite* s = suite_create("static-pool-suite");

    TCase* tc_alloc = tcase_create("alloc-test");
    tcase_add_test(tc_alloc, test_alloc);
    suite_add_tcase(s, tc_alloc);

    TCase* tc_free = tcase_create("free-test");
    tcase_add_test(tc_free, test_free);
    suite_add_tcase(s, tc_free);

    return s;
}

int main(void)
{
    int number_failed;
    Suite* s = thread_safe_suite();
    SRunner* sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
