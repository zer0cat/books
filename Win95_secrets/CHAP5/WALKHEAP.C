//==================================
// WALKHEAP - Matt Pietrek 1995
// FILE: WALKHEAP.C
//==================================

#include <windows.h>
#include <stdio.h>
#include "heapw32.h"

// Prototypes
HANDLE WalkHeap( HANDLE hHeap );
void DisplayFreeBlock( PFREE_HEAP_ARENA_DEBUG pFreeArena );
void DisplayInUseBlock( PHEAP_ARENA_DEBUG pArena );
void DisplayHeapFlags( BYTE flags );
void MakeSomeAllocationsAndDeletions( HANDLE hHeap );

int main(int argc, char * argv[])
{
    HANDLE heap, anotherHeap;

    if ( (GetVersion() & 0xC0000000) != 0xC0000000 )
    {
        printf("WALKHEAP will only work with Win95\n");
        return 0;       
    }
    
    if ( !GetSystemMetrics(SM_DEBUG) )
    {
        printf("WALKHEAP will only work with the debug version of Win95\n");
        return 0;
    }
    
    if ( argc == 2 )    // If user supplied an hHeap to walk, do it.
    {
        HANDLE hHeap;
        
        if ( 1 == sscanf( argv[1], "%x", &hHeap ) )
        {
            WalkHeap( hHeap );
        }
        else
            printf( "Syntax: \"WALKHEAP <hHeap>\" or just \"WALKHEAP\"\n");

        return 0;
    }

    // No command line was specified.  Do default action.

	heap = GetProcessHeap();

	// First, make the main heap look more interesting
    MakeSomeAllocationsAndDeletions( heap );

	// Create another heap to show heap chaining.
    anotherHeap = HeapCreate( HEAP_GENERATE_EXCEPTIONS | HEAP_NO_SERIALIZE,
                              0x80000, 0x100000 );

	// Walk all the heaps in the process.  New heaps are put at the head
    // of the list.
    heap = anotherHeap;
	while ( heap )
		heap = WalkHeap( heap );

    return 0;
}

#define WIDTH 40

// Returns HANDLE for next heap in process, or 0 on failure
HANDLE WalkHeap( HANDLE hHeap )
{
    PHEAP_HEADER_DEBUG pHeapHeader = (PHEAP_HEADER_DEBUG)hHeap;
    PHEAP_ARENA_DEBUG pArena;
    PFREE_LIST_HEADER_DEBUG pFreeListHdr;
    unsigned i;

    if (   IsBadReadPtr( pHeapHeader, sizeof(HEAP_HEADER_DEBUG))
        || pHeapHeader->signature != 0x4948 )
    {
        printf("%08X is not a valid heap handle\n", hHeap);
        return 0;
    }
    
    printf("Heap at %08X\n", pHeapHeader);
    
    printf("%-*s%08X\n", WIDTH, "size:", pHeapHeader->dwSize);

    printf("%-*s%08X\n", WIDTH, "next block:", pHeapHeader->nextBlock);

    printf("Free lists:\n");
    pFreeListHdr = pHeapHeader->freeListArray;
    
    for ( i=0; i < 4; i++ )
    {
        printf("  Head:%08X  size: %X\n",
                &pFreeListHdr->freeArena,
                pFreeListHdr->dwMaxBlockSize);
        pFreeListHdr++;     // Advance to next free list header
    }

    printf("%-*s%08X\n", WIDTH, "Next heap:", pHeapHeader->nextHeap);

    printf("%-*s%08X\n", WIDTH, "CritSection:", pHeapHeader->pCriticalSection);

    // CRITICAL_SECTION criticalSection;    // 0x7C

    printf("%-*s%08X\n", WIDTH, "Creating EIP:", pHeapHeader->creating_EIP);
    printf("%-*s%08X\n", WIDTH, "checksum:", pHeapHeader->checksum);
    printf("%-*s%04X\n", WIDTH, "Creating Thread:",
            pHeapHeader->creating_thread_ordinal);
    printf("%-*s%02X\n", WIDTH, "Flags:", pHeapHeader->flags);
    DisplayHeapFlags( pHeapHeader->flags );
    printf("%-*s%04X\n", WIDTH, "Signature:", (WORD)pHeapHeader->signature);

    // The first arena starts after the heap header.  (The "+1" is C ptr math)
    pArena = (PHEAP_ARENA_DEBUG)(pHeapHeader+1);

    printf("\nHeap Blocks\n");
    printf("Block     Stat  Size      Checksum  Thrd\n"
           "--------  ----  --------  --------  ----\n");

    pFreeListHdr = pHeapHeader->freeListArray;
    for ( i=0; i < 4; i++ )
    {
        DisplayFreeBlock( &pFreeListHdr->freeArena );
        pFreeListHdr++;     // Point at next free list head
    }

    printf("\n");
    
    while ( 1 )
    {
        DWORD blockSize = pArena->size & ~0xA0000003;

        if ( blockSize == 0 )
            break;

        if (   (pArena->signature != 0x4842)        // 0x4842 = "BH" (in-use)
            && (pArena->signature != 0x4846) )      // 0x4846 = "FH" (free)
            break;


        if ( pArena->size & 1 )
            DisplayFreeBlock( (PFREE_HEAP_ARENA_DEBUG)pArena );
        else
            DisplayInUseBlock( pArena );

        // Advance to next block
        pArena = (PHEAP_ARENA_DEBUG)((DWORD)pArena + blockSize);
    }

	printf("\n\n");

	return pHeapHeader->nextHeap;
}

void DisplayInUseBlock( PHEAP_ARENA_DEBUG pArena )
{
    printf("%08X  used  %08X  %08X  %04X  EIP: %08X  \n",
            pArena,
            pArena->size & ~0xA0000003,
            pArena->b.checksum, 
            pArena->threadID,
            pArena->a.alloc_EIP);   
}

void DisplayFreeBlock( PFREE_HEAP_ARENA_DEBUG pFreeArena )
{
    printf("%08X  free  %08X  %08X  %04X  prev:%08X  next:%08X\n",
            pFreeArena,
            pFreeArena->arena.size & ~0xA0000003,
            pFreeArena->freeBlockChecksum,
            pFreeArena->arena.threadID,
            pFreeArena->arena.a.prev,
            pFreeArena->arena.b.next);  
}

void MakeSomeAllocationsAndDeletions( HANDLE hHeap )
{
    LPVOID ptrArray[20];

    ptrArray[0] = HeapAlloc( hHeap, 0, 0x4 );
    ptrArray[1] = HeapAlloc( hHeap, 0, 0x8 );
    ptrArray[2] = HeapAlloc( hHeap, 0, 0xC );
    ptrArray[3] = HeapAlloc( hHeap, 0, 0x10 );
    ptrArray[4] = HeapAlloc( hHeap, 0, 0x14 );

    ptrArray[5] = HeapAlloc( hHeap, 0, 0x24 );
    ptrArray[6] = HeapAlloc( hHeap, 0, 0x28 );
    ptrArray[7] = HeapAlloc( hHeap, 0, 0x2C );
    ptrArray[8] = HeapAlloc( hHeap, 0, 0x30 );
    ptrArray[9] = HeapAlloc( hHeap, 0, 0x34 );

    ptrArray[10] = HeapAlloc( hHeap, 0, 0xC4 );
    ptrArray[11] = HeapAlloc( hHeap, 0, 0xC8 );
    ptrArray[12] = HeapAlloc( hHeap, 0, 0xCC );
    ptrArray[13] = HeapAlloc( hHeap, 0, 0xD0 );
    ptrArray[14] = HeapAlloc( hHeap, 0, 0xD4 );

    ptrArray[15] = HeapAlloc( hHeap, 0, 0x224 );
    ptrArray[16] = HeapAlloc( hHeap, 0, 0x228 );
    ptrArray[17] = HeapAlloc( hHeap, 0, 0x22C );
    ptrArray[18] = HeapAlloc( hHeap, 0, 0x230 );
    ptrArray[19] = HeapAlloc( hHeap, 0, 0x234 );

    // Put some stuff into the < 0x20 free list
    HeapFree( hHeap, 0, ptrArray[3] );
    HeapFree( hHeap, 0, ptrArray[1] );

    // Put some stuff into the < 0x80 free list
    HeapFree( hHeap, 0, ptrArray[8] );
    HeapFree( hHeap, 0, ptrArray[6] );

    // Put some stuff into the < 0x200 free list
    HeapFree( hHeap, 0, ptrArray[13] );
    HeapFree( hHeap, 0, ptrArray[11] );

    // Put some stuff into the < 0xFFFFFFFF free list
    HeapFree( hHeap, 0, ptrArray[18] );
    HeapFree( hHeap, 0, ptrArray[16] );
	
	// allocate some really big blocks ( > 1MB )
	HeapAlloc( hHeap, 0, 0x200000 );
	HeapAlloc( hHeap, 0, 0x180000 );
}

typedef struct
{
    BYTE    flag;
    PSTR    name;
} BYTE_FLAG_DESCRIPTIONS;

BYTE_FLAG_DESCRIPTIONS HeapFlags[] = 
{
{ 0x00000001, "HEAP_NO_SERIALIZE" },
{ 0x00000002, "HEAP_GROWABLE" },
{ 0x00000004, "HEAP_GENERATE_EXCEPTIONS" },
{ 0x00000008, "HEAP_ZERO_MEMORY" },
{ 0x00000010, "HEAP_REALLOC_IN_PLACE_ONLY" },
{ 0x00000020, "HEAP_TAIL_CHECKING_ENABLED" },
{ 0x00000040, "HEAP_FREE_CHECKING_ENABLED" },
{ 0x00000080, "HEAP_DISABLE_COALESCE_ON_FREE" }
};
#define HEAP_FLAGS_COUNT ( sizeof(HeapFlags) / sizeof(HeapFlags[0]) )

void DisplayHeapFlags( BYTE flags )
{
    unsigned i;
    
    for ( i=0; i < HEAP_FLAGS_COUNT; i++ )
        if ( HeapFlags[i].flag & flags )
            printf("%-*s%s\n", WIDTH, "", HeapFlags[i].name);
}

