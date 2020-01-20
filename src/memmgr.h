/**
 * @file memmgr.h
 * @brief Block memory manager
 * @author Anton Sysoev
 * @version 0.1
 */
#ifndef __MEMMGR_H
#define __MEMMGR_H
#include <stddef.h>

/*
 * Define default machine aligment for arm machines
 * ARM recommends 8-byte address alignment http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka4127.html
 */
#ifndef MEMMGR_MACHINE_ALIGNMENT
#define MEMMGR_MACHINE_ALIGNMENT 8
#endif

/*
 * Define default machine address mask alignment for arm machines
 */
#ifndef MEMMGR_MACHINE_ALIGNMENT_MASK
#define MEMMGR_MACHINE_ALIGNMENT_MASK 0x7ULL
#endif

typedef struct BLOCK_HEADER BlockHeader_t;
typedef BlockHeader_t HeapHeader_t;

void* _pvMemMgrAlloc(HeapHeader_t* pxHeader);
void _vMemMgrFree(HeapHeader_t* pxHeader, void* pvMem);
#ifndef MEMMGR_USE_STATIC_POOL
HeapHeader_t* _pxMemMgrInit(void* pxHeader, size_t xSize, size_t xChunkSize);
#endif

#ifndef MEMMGR_CHECK_OVERFLOW
#define MEMMGR_CHECK_OVERFLOW 0
#else
#if (MEMMGR_CHECK_OVERFLOW > 0)
#undef MEMMGR_CHECK_OVERFLOW
#define MEMMGR_CHECK_OVERFLOW 1
#endif
#endif

#if (MEMMGR_CHECK_OVERFLOW > 0)
extern void vMemMgrOverflowHook(void* pvMemAddr);
#endif

#ifndef MEMMGR_THREAD_SAFE
#define MEMMGR_THREAD_SAFE 0
#else
#if (MEMMGR_THREAD_SAFE > 0)
#undef MEMMGR_THREAD_SAFE
#define MEMMGR_THREAD_SAFE 1
#endif
#endif

#if (MEMMGR_THREAD_SAFE > 0)
extern void vMemMgrLock(HeapHeader_t*);
extern void vMemMgrUnlock(HeapHeader_t*);
#endif

#ifdef MEMMGR_USE_STATIC_POOL
extern BlockHeader_t* pxMemMgrStaticMemPool;
#endif

#ifdef MEMMGR_USE_STATIC_POOL
#define pvMemMgrAlloc() _pvMemMgrAlloc(pxMemMgrStaticMemPool)
#else
#define pvMemMgrAlloc(pxStaticMemPool) _pvMemMgrAlloc(pxStaticMemPool)
#endif

#ifdef MEMMGR_USE_STATIC_POOL
#define vMemMgrFree(pvMem) _vMemMgrFree(pxMemMgrStaticMemPool, (pvMem))
#else
#define vMemMgrFree(pxStaticMemPool, pvMem) _vMemMgrFree(pxStaticMemPool, (pvMem))
#endif

#ifdef MEMMGR_USE_STATIC_POOL
void vMemMgrStaticPoolInit(size_t xChunkSize);
#else
#define pxMemMgrInit(pxBuf, xSize, xChunkSize) _pxMemMgrInit((pxBuf), (xSize), (xChunkSize))
#endif

#endif
