# Block memory allocation library

## Summary

This library provides management of memory blocks allocation/deallocation. Library implements simple linked list algorithm.
You can use it with static/single memory pool or different chunk size pools.
Library can check your memory pool for memory overflow with magic number in header with `-DMEMMGR_CHECK_OVERFLOW` flag and user-provided callback.

## Detailed description

You should provide memory buffer that would be marked as descried above:

* memory pool header:
  * magic (if used memory overflow checking)
  * next free block address to fast allocation
* memory block 1:
  * memory block header:
    * magic (if used memory overflow checking)
    * next free block address
  * aligned user memory block
* memory block N

The library call `vMemMgrOverflowHook` on every magic check assertion when used memory overflow checking.

Actually available memory is differ to user provided buffer size because of using memory block headers and due to alignment.

## Usage

Compilation flags:

`-DMEMMGR_CHECK_OVERFLOW=[0|1]` - library provides checking for memory overflow functionality using magic number and call user provided callback `void vMemMgrOverflowHook(void* pvMemAddr);`

`-DMEMMGR_MACHINE_ALIGNMENT=a` - align allocated block address to `MEMMGR_MACHINE_ALIGNMENT`. Default is ARM-recommended 8 bytes used

`-DMEMMGR_MACHINE_ALIGNMENT_MASK=m` - address mask to check memory alignment. Default is ARM-recommended 8 bytes alignment mask `0x7`.

`-DMEMMGR_USE_STATIC_POOL=[1|0]` - user wants to use only one memory poll. This pool would be created statically after `vMemMgrInit(pxBuf, xSize, xChunkSize)` call and no pool pointer would be required with alloc/dealloc calls.

`-DMEMMGR_THREAD_SAFE=[1|0]` - if you want to use library in multithreaded environment you shold set this flag to 1 and provide two functions `void vMemMgrLock(HeapHeader_t*)` and `void vMemMgrUnlock(HeapHeader_t*)`

## Unit testing

Unit tests requires the following
* libcheck
* llvm

If you have this frameworks you can call `make tests` from `test` subdirectory.

## Known Constraints

* Library uses extension named zero-memory-array. It means if your compiler does not support it your can not use this library or you can modify source code to calculate memory block address manually.
* Unit tests are not covering multithreading. Only simple checking for object is unlocked after thread-sensitive calls. You should test it with your environment due to it's depended on target implementation.
* There is no checks for calling from ISR, be careful with thread-safe functions implementation.
