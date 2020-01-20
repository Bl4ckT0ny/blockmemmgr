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

#define SIMPLE_ALLOC_BUFFER_SIZE 256
#define SIMPLE_ALLOC_CHUNK_SIZE 3

START_TEST(test_mem_overflow)
{
    uint8_t puBuf[SIMPLE_ALLOC_BUFFER_SIZE];
    void* pxPtr;
    HeapHeader_t* pxHeader = NULL;
    pxHeader = pxMemMgrInit(puBuf, SIMPLE_ALLOC_BUFFER_SIZE, SIMPLE_ALLOC_CHUNK_SIZE);
    ck_assert_ptr_nonnull(pxHeader);
    pxPtr = pvMemMgrAlloc(pxHeader);
    memset(pxPtr, 0, TEST_HEADER_SIZE + (SIMPLE_ALLOC_CHUNK_SIZE | MEMMGR_MACHINE_ALIGNMENT_MASK) + 1);

    pxPtr = pvMemMgrAlloc(pxHeader);
    ck_assert_uint_ne(bOverflowAsserted, 0);
    bOverflowAsserted = false;
}
END_TEST

START_TEST(test_neg_alloc_init)
{
    uint8_t puBuf[SIMPLE_ALLOC_BUFFER_SIZE];
    HeapHeader_t* pxHeader = NULL;
    pxHeader = pxMemMgrInit(NULL, SIMPLE_ALLOC_BUFFER_SIZE, SIMPLE_ALLOC_CHUNK_SIZE);
    ck_assert_ptr_null(pxHeader);
    pxHeader = pxMemMgrInit(puBuf, 0, SIMPLE_ALLOC_CHUNK_SIZE);
    ck_assert_ptr_null(pxHeader);
    pxHeader = pxMemMgrInit(puBuf, SIMPLE_ALLOC_BUFFER_SIZE, 0);
    ck_assert_ptr_null(pxHeader);

    //set buffer size to chunk size that always less then buffer size
    pxHeader = pxMemMgrInit(puBuf, SIMPLE_ALLOC_CHUNK_SIZE, SIMPLE_ALLOC_BUFFER_SIZE);
    ck_assert_ptr_null(pxHeader);

    //check for memory allocation fails if no header initialized
    void* pvPtr = NULL;
    pvPtr = pvMemMgrAlloc(NULL);
    ck_assert_ptr_null(pvPtr);
}
END_TEST

START_TEST(test_alloc_dealloc)
{
    uint8_t pxBuf[SIMPLE_ALLOC_BUFFER_SIZE] = { 0 };
    bOverflowAsserted = false;

    HeapHeader_t* pxHeader = pxMemMgrInit(pxBuf, SIMPLE_ALLOC_BUFFER_SIZE, SIMPLE_ALLOC_CHUNK_SIZE);
    void* pvPtr1 = pvMemMgrAlloc(pxHeader);
    ck_assert_uint_ne(bOverflowAsserted, 1);
    void* pvPtr2 = pvMemMgrAlloc(pxHeader);
    ck_assert_uint_ne(bOverflowAsserted, 1);
    ck_assert_ptr_nonnull(pvPtr1);
    memset(pvPtr1, 0xFF, SIMPLE_ALLOC_CHUNK_SIZE);
    ck_assert_ptr_nonnull(pvPtr2);
    memset(pvPtr2, 0xFF, SIMPLE_ALLOC_CHUNK_SIZE);
    ck_assert_ptr_ne(pvPtr1, pvPtr2);
    vMemMgrFree(pxHeader, pvPtr2);
    ck_assert_uint_ne(bOverflowAsserted, 1);
    vMemMgrFree(pxHeader, pvPtr1);
    ck_assert_uint_ne(bOverflowAsserted, 1);
}
END_TEST

#define CHECK_ALIGNMENT_BUFFER_SIZE 1024
#define CHECK_ALIGNMENT_MAX_CHUNK (CHECK_ALIGNMENT_BUFFER_SIZE / 2)

START_TEST(test_check_alignment)
{
    const size_t chunk_size = _i;
    uint8_t* puMem = alloca(CHECK_ALIGNMENT_BUFFER_SIZE);
    HeapHeader_t* pxHeader = pxMemMgrInit(puMem, CHECK_ALIGNMENT_BUFFER_SIZE, chunk_size);
    ck_assert_ptr_nonnull(pxHeader);
    uint8_t* puBuf;
    bOverflowAsserted = false;
    while ((puBuf = pvMemMgrAlloc(pxHeader)) != NULL) {
        ck_assert_uint_eq((uintptr_t)puBuf & MEMMGR_MACHINE_ALIGNMENT_MASK, 0);
        ck_assert_uint_ne(bOverflowAsserted, 1);
    }
    ck_assert_uint_ne(bOverflowAsserted, 1);
}
END_TEST

#define CHECK_FREE_BUFFER_SIZE 256
#define CHECK_FREE_MAX_CHUNK (CHECK_FREE_BUFFER_SIZE / 2)
START_TEST(test_check_free)
{
    void* pvMem = alloca(CHECK_FREE_BUFFER_SIZE);
    const size_t chunk_size = _i;
    HeapHeader_t* const pxHeader = pxMemMgrInit(pvMem, CHECK_FREE_BUFFER_SIZE, chunk_size);
    ck_assert_ptr_nonnull(pxHeader);
    bOverflowAsserted = false;
    size_t xBuffCount = (CHECK_FREE_BUFFER_SIZE / (TEST_HEADER_SIZE + ((chunk_size | MEMMGR_MACHINE_ALIGNMENT_MASK) + 1)));
    void** pvBuffs = alloca(sizeof(void*) * xBuffCount);
    for (size_t nBuff = 0; nBuff < xBuffCount; ++nBuff) {
        for (size_t j = 0; j < nBuff; ++j) {
            void* ptr = pvMemMgrAlloc(pxHeader);
            ck_assert_uint_ne(bOverflowAsserted, 1);
            ck_assert_ptr_nonnull(ptr);
            pvBuffs[j] = ptr;
        }
        for (size_t j = 0; j < nBuff; ++j) {
            memset(pvBuffs[j], 0, chunk_size);
            vMemMgrFree(pxHeader, pvBuffs[j]);
            ck_assert_uint_ne(bOverflowAsserted, 1);
        }
    }
}
END_TEST

Suite* dynamic_pool_suite()
{
    Suite* s = suite_create("dynamic-pool-suite");

    TCase* tc_neg_alloc_init = tcase_create("negative-alloc-init");
    tcase_add_test(tc_neg_alloc_init, test_neg_alloc_init);
    suite_add_tcase(s, tc_neg_alloc_init);

    TCase* tc_alloc_dealloc = tcase_create("alloc-dealloc");
    tcase_add_test(tc_alloc_dealloc, test_alloc_dealloc);
    suite_add_tcase(s, tc_alloc_dealloc);

    TCase* tc_mem_overflow = tcase_create("mem-overflow");
    tcase_add_test(tc_mem_overflow, test_mem_overflow);
    suite_add_tcase(s, tc_mem_overflow);

    TCase* tc_check_free = tcase_create("check-free");
    tcase_add_loop_test(tc_check_free, test_check_free, 1, CHECK_FREE_MAX_CHUNK);
    tcase_set_timeout(tc_check_free, 0);
    suite_add_tcase(s, tc_check_free);

    TCase* tc_check_alignment = tcase_create("check-alignment");
    tcase_add_loop_test(tc_check_alignment, test_check_alignment, 1, CHECK_ALIGNMENT_MAX_CHUNK);
    suite_add_tcase(s, tc_check_alignment);

    return s;
}

int main(void)
{
    int number_failed;
    Suite* s = dynamic_pool_suite();
    SRunner* sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
