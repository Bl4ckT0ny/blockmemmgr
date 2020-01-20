/**
 * @file memmgr.c
 * @brief Block memory manager
 * @author Anton Sysoev
 * @version 0.1
 */

#include "memmgr.h"
#include <stddef.h>
#include <stdint.h>

#define MEMMGR_MAGIC ((size_t)0xDEADBEAFDEADBEAFULL)
#define MEMMGR_ALIGN_LOWER(addr) ((addr) & (~(MEMMGR_MACHINE_ALIGNMENT_MASK)))
#define MEMMGR_ALIGN_UPPER(addr) (((addr) & (MEMMGR_MACHINE_ALIGNMENT_MASK)) != 0 ? MEMMGR_ALIGN_LOWER((addr) + (MEMMGR_MACHINE_ALIGNMENT)) : (addr))
#define MEMMGR_NEXT_BLOCK_ADDR(addr, offset)             \
    MEMMGR_ALIGN_UPPER(                                  \
        MEMMGR_ALIGN_UPPER(                              \
            ((uintptr_t)(addr)) + sizeof(BlockHeader_t)) \
        + (offset))

struct BLOCK_HEADER {
#if (MEMMGR_CHECK_OVERFLOW > 0)
    size_t xMagic;
#endif
    BlockHeader_t* pxNextFree;
    uint8_t pvMem[0];
};

#ifdef MEMMGR_USE_STATIC_POOL
BlockHeader_t* pxMemMgrStaticMemPool;
static uint8_t puMemMgrMemoryPool[MEMMGR_STATIC_POOL_SIZE];

static HeapHeader_t* _pxMemMgrInit(void* pvBuf, size_t xSize, size_t xChunkSize);

void vMemMgrStaticPoolInit(size_t xChunkSize)
{
    pxMemMgrStaticMemPool = _pxMemMgrInit(puMemMgrMemoryPool, MEMMGR_STATIC_POOL_SIZE, xChunkSize);
}
#endif

void* _pvMemMgrAlloc(HeapHeader_t* pxHeader)
{

    void* pvResult = NULL;

#if (MEMMGR_THREAD_SAFE > 0)
    vMemMgrLock(pxHeader);
#endif

    do {
        if (pxHeader == NULL)
            break;

#if (MEMMGR_CHECK_OVERFLOW > 0)
        if (pxHeader->xMagic != MEMMGR_MAGIC) {
            vMemMgrOverflowHook(pxHeader);
            break;
        }
#endif

        // No free memory
        if (pxHeader->pxNextFree == NULL)
            break;

        BlockHeader_t* pxPtr = pxHeader->pxNextFree;
        pxHeader->pxNextFree = pxPtr->pxNextFree;

#if (MEMMGR_CHECK_OVERFLOW > 0)
        if (pxPtr->xMagic != MEMMGR_MAGIC) {
            vMemMgrOverflowHook(pxHeader);
            break;
        }
#endif

        // due to struct alignment and struct packing can be differ to required memory alignment
        // we need to align memory block offset
        pvResult = (void*)MEMMGR_ALIGN_UPPER((uintptr_t)pxPtr->pvMem);
    } while (0);

#if (MEMMGR_THREAD_SAFE > 0)
    vMemMgrUnlock(pxHeader);
#endif

    return pvResult;
}

void _vMemMgrFree(HeapHeader_t* pxHeader, void* pvMem)
{
#if (MEMMGR_THREAD_SAFE > 0)
    vMemMgrLock(pxHeader);
#endif

    do {
        if (pxHeader == NULL || pvMem == NULL)
            break;

        BlockHeader_t* pxPtr = (BlockHeader_t*)(void*)MEMMGR_ALIGN_LOWER((uintptr_t)((uint8_t*)pvMem - sizeof(BlockHeader_t)));

#if (MEMMGR_CHECK_OVERFLOW > 0)
        if (pxHeader->xMagic != MEMMGR_MAGIC
            || pxPtr->xMagic != MEMMGR_MAGIC
            || ((pxHeader->pxNextFree != NULL) && (pxHeader->pxNextFree->xMagic != MEMMGR_MAGIC))) {
            vMemMgrOverflowHook(pvMem);
            break;
        }
#endif

        // pointer is out of range
        if ((uintptr_t)pxPtr < MEMMGR_ALIGN_UPPER((uintptr_t)pxHeader->pvMem))
            break;

        pxPtr->pxNextFree = pxHeader->pxNextFree;
        pxHeader->pxNextFree = pxPtr;
    } while (0);

#if (MEMMGR_THREAD_SAFE > 0)
    vMemMgrUnlock(pxHeader);
#endif
    return;
}

HeapHeader_t* _pxMemMgrInit(void* pvBuf, size_t xSize, size_t xChunkSize)
{
#define BUILD_ASSERT(condition) ((void)sizeof(char[1 - 2 * !!(condition)]))
    //assert for compiler accept zero-sized arrays
    BUILD_ASSERT(sizeof(struct BLOCK_HEADER) != (sizeof(uintptr_t) + sizeof(size_t) * MEMMGR_CHECK_OVERFLOW));

    HeapHeader_t* pxResult = NULL;
    do {
        if (pvBuf == NULL || xSize == 0 || xChunkSize == 0)
            break;

        // check if we can allocate 1 block (pool header + first aligned block)
        if (xSize < MEMMGR_ALIGN_UPPER(sizeof(BlockHeader_t) + MEMMGR_NEXT_BLOCK_ADDR(0, xChunkSize)))
            break;

        HeapHeader_t* pxHeader = (HeapHeader_t*)(void*)MEMMGR_ALIGN_UPPER((uintptr_t)pvBuf);
        BlockHeader_t* pxPtr = (BlockHeader_t*)(void*)MEMMGR_ALIGN_UPPER((uintptr_t)pxHeader->pvMem);
        pxHeader->pxNextFree = pxPtr;
        BlockHeader_t* pxPrevPtr = pxPtr;
#if (MEMMGR_CHECK_OVERFLOW > 0)
        pxHeader->xMagic
            = MEMMGR_MAGIC;
#endif

        for (;
             MEMMGR_NEXT_BLOCK_ADDR(pxPtr, xChunkSize) < (uintptr_t)((uint8_t*)pvBuf + xSize);
             pxPtr = pxPtr->pxNextFree) {
            pxPtr->pxNextFree = (BlockHeader_t*)(void*)MEMMGR_NEXT_BLOCK_ADDR(pxPtr, xChunkSize);
            pxPrevPtr = pxPtr;
#if (MEMMGR_CHECK_OVERFLOW > 0)
            pxPtr->xMagic = MEMMGR_MAGIC;
#endif
        }

        //Terminate last block
        pxPrevPtr->pxNextFree = NULL;
        pxResult = pxHeader;
    } while (0);
    return pxResult;
}
