//==================================
// WALKHEAP - Matt Pietrek 1995
// FILE: HEAPW32.H
//==================================

typedef struct tagHEAP_ARENA_DEBUG
{
	DWORD	size;	// Size of block, including arena, OR'ed with A0000000
					// If bottom bit is set, block is free
	union
	{
		DWORD	alloc_EIP;	// 0x04	// If in-use block
		DWORD	prev;		// 0x04	// if free block
	}a;

	WORD	threadID;	// 0x08	(Free blocks are 0xFEFE)
	WORD	signature;	// 0x0A 0x4842 = "BH", 0x4846 = "FH"

	union
	{
		DWORD	checksum;	// 0x0C	// If in-use block
		DWORD	next;		// 0x0C	// If free block
	}b;
} HEAP_ARENA_DEBUG, *PHEAP_ARENA_DEBUG;


typedef struct tagHEAP_ARENA_RETAIL
{
	DWORD	size;	// Size of block, including arena, OR'ed with A0000000
					// If bottom bit is set, block is free
} HEAP_ARENA_RETAIL, *PHEAP_ARENA_RETAIL;


typedef struct tagFREE_HEAP_ARENA_DEBUG
{
	HEAP_ARENA_DEBUG arena;
	DWORD	freeBlockChecksum;		// 0x10	- only present if a free block
} FREE_HEAP_ARENA_DEBUG, *PFREE_HEAP_ARENA_DEBUG;


typedef struct tagFREE_HEAP_ARENA_RETAIL
{
	HEAP_ARENA_RETAIL	arena;
	DWORD				prev;
	DWORD				next;
} FREE_HEAP_ARENA_RETAIL, *PFREE_HEAP_ARENA_RETAIL;


typedef struct tagFREE_LIST_HEADER_DEBUG
{
	DWORD					dwMaxBlockSize;
	FREE_HEAP_ARENA_DEBUG	freeArena;
} FREE_LIST_HEADER_DEBUG, *PFREE_LIST_HEADER_DEBUG;

typedef struct tagFREE_LIST_HEADER_RETAIL
{
	DWORD					dwMaxBlockSize;
	FREE_HEAP_ARENA_RETAIL	freeArena;
} FREE_LIST_HEADER_RETAIL, *PFREE_LIST_HEADER_RETAIL;

typedef struct tagHEAP_HEADER_DEBUG
{
	DWORD	dwSize;		// 0x00 total size of heap (defaults to 1MB + 4K)
	DWORD	nextBlock;	// 0x04 next reserved block of memory in this heap

	FREE_LIST_HEADER_DEBUG	freeListArray[4];	// 0x8
							// 0x08 start of array of free list structures
							// 0x18 bytes long.  Array of these.  Sizes
							// are 0x20, 0x80, and 0x200, and < 0xFFFFFFFF

	HANDLE				nextHeap;			// 0x68	 Next heap in process

	PCRITICAL_SECTION	pCriticalSection;	// 0x6C	- for heap synchronization

	CRITICAL_SECTION	criticalSection;	// 0x70

	DWORD	unknown1[14];

	DWORD	creating_EIP;	// 0xC0
	DWORD	checksum;		// 0xC4 checksum
	WORD	creating_thread_ordinal;	// 0xC8
	WORD	unknown2;		// 0xCA
		
	BYTE	flags;			// 0xCC HEAP_xxx flags
	BYTE	unknown3;		// 0xCD filler
	WORD	signature;		// 0xCE
} HEAP_HEADER_DEBUG, *PHEAP_HEADER_DEBUG;

typedef struct tagHEAP_HEADER_RETAIL
{
	DWORD	dwSize;		// 0x00 total size of heap (defaults to 1MB + 4K)
	DWORD	nextBlock;	// 0x04 next reserved block of memory in this heap

	FREE_LIST_HEADER_RETAIL	freeListArray[4];	// 0x8
							// 0x08 start of array of free list structures
							// 0x18 bytes long.  Array of these.  Sizes
							// are 0x20, 0x80, and 0x200, and < 0xFFFFFFFF

	HANDLE				nextHeap;			// 0x48	 Next heap in process

	PCRITICAL_SECTION	pCriticalSection;	// 0x4C	- for heap synchronization

	CRITICAL_SECTION	criticalSection;	// 0x50

	DWORD	unknown1[2];					// 0x68

	BYTE	flags;			// 0x70 HEAP_xxx flags
	BYTE	unknown2;		// 0x71 filler
	WORD	signature;		// 0x72
} HEAP_HEADER_RETAIL, *PHEAP_HEADER_RETAIL;
