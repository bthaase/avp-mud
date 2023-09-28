/*
 * Matthew Chaplain's Memory Leak Tracer
 * Copyright (c) 2000-2001, All Rights Reserved
 */

/* Library includes */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Local includes */

#include "mc_mem.h"

/* Defined constants */

/*
 * Since in most compilers, memory is allocated on a
 * 4-byte boundary, I've chosen a nice prime number for
 * the hash size so that it isn't all clumped into four
 * or so buckets.
 *
 * This can be redefined on the command line if you want
 * it to be bigger.
 *
 * Obviously, the larger the project, the larger you want
 * this number to be, for efficiency.
 */
#ifndef MEMORY_HASH
#define MEMORY_HASH 31
#endif

/* Local type definitions */

typedef struct _mc_memory_t mc_memory_t;

struct _mc_memory_t
{
    mc_memory_t *pNext;
    int          nAmountAllocated;
    char const  *szFileName;
    int          nLineNumber;
    void        *pvMemory;
};

/* Local functions */

static int report_corruption( void *pvMemory );

/* Local variables */

static mc_memory_t *paMemoryHash[MEMORY_HASH];

/*
 * This must be called before any allocation is done, or
 * deallocation becomes undefined.
 */
void mc_memory_init( void )
{
    static short bInitialised = 0;
    int          nIndex;

    if( !bInitialised )
    {
        /*
        printf( "Matthew Chaplain's Memory Tracer\n"
                "Copyright (c) 2000-2001 Matthew Chaplain All Rights "
                "Reserved.\n\nOptions set:\n MEMORY_HASH: %d\n\n",
                MEMORY_HASH );
        */

        for( nIndex = 0; nIndex < MEMORY_HASH; ++nIndex )
        {
            paMemoryHash[nIndex] = NULL;
        }

#ifndef NOREPORTATEXIT
        atexit( mc_memory_done );
#endif

        bInitialised = 1;
    }
}

/*
 * The malloc replacement function.  Allocates memory, stores a structure
 * describing it in the hash table, and returns the memory
 */
void *mc_memory_malloc( size_t nSize, char const *szFileName, int nLineNumber )
{
    mc_memory_t *pNewMemory;
    int          nHash;

    /* allocate a new memory structure */
    pNewMemory                   = malloc( sizeof( *pNewMemory ) );
    pNewMemory->nAmountAllocated = nSize;
    pNewMemory->szFileName       = szFileName;
    pNewMemory->nLineNumber      = nLineNumber;
    pNewMemory->pvMemory         = malloc( nSize );

    /* use its actual memory address as a hashing code */
    nHash = ((int)pNewMemory->pvMemory) % MEMORY_HASH;

    /* insert the memory structure into the lookup */
    pNewMemory->pNext            = paMemoryHash[nHash];
    paMemoryHash[nHash]          = pNewMemory;

    /* return the actual memory address */
    return pNewMemory->pvMemory;
}

/*
 * The calloc replacement function.  Allocates memory, stores a structure
 * describing it in the hash table, and returns the memory
 */
void *mc_memory_calloc( size_t nElements, size_t nSize, char const *szFileName, 
                        int nLineNumber )
{
    mc_memory_t *pNewMemory;
    int          nHash;

    /* allocate a new memory structure */
    pNewMemory                   = malloc( sizeof( *pNewMemory ) );
    pNewMemory->nAmountAllocated = nSize * nElements;
    pNewMemory->szFileName       = szFileName;
    pNewMemory->nLineNumber      = nLineNumber;
    pNewMemory->pvMemory         = calloc( nElements, nSize );

    /* use its actual memory address as a hashing code */
    nHash = ((int)pNewMemory->pvMemory) % MEMORY_HASH;

    /* insert the memory structure into the lookup */
    pNewMemory->pNext            = paMemoryHash[nHash];
    paMemoryHash[nHash]          = pNewMemory;

    /* return the actual memory address */
    return pNewMemory->pvMemory;
}

/*
 * The realloc replacement function.  Allocates memory, stores a structure
 * describing it in the hash table, and returns the memory
 */
void *mc_memory_realloc( void *pvMemory, size_t nSize, 
                         char const *szFileName, int nLineNumber )
{
    /* Get hash of memory block */
    int   nHash    = (int)pvMemory % MEMORY_HASH;
    void *pvReallocMemory = NULL;

    if( !pvMemory )
    {
        pvReallocMemory = mc_memory_malloc( nSize, szFileName, nLineNumber );
    }
    else if( !paMemoryHash[nHash] )
    {
        /* A memory corruption has taken place */
    }
    else if( paMemoryHash[nHash]->pvMemory == pvMemory )
    {
        /* Was the first block */
        mc_memory_t *pMemory = paMemoryHash[nHash];

        /* Unlink node */
        paMemoryHash[nHash] = paMemoryHash[nHash]->pNext;

        /* Rewrite the structure to reflect new data */
        pMemory->nAmountAllocated = nSize;
        pMemory->szFileName       = szFileName;
        pMemory->nLineNumber      = nLineNumber;
        pMemory->pvMemory         = realloc( pvMemory, nSize );

        pvReallocMemory = pMemory->pvMemory;
        
        /* Reinsert structure */
        nHash = (int)pMemory->pvMemory % MEMORY_HASH;
        pMemory->pNext      = paMemoryHash[nHash];
        paMemoryHash[nHash] = pMemory;
    }
    else
    {
        /* Search for block */
        mc_memory_t *pPrevMemory;
        mc_memory_t *pCurrentMemory;
        short        bFound = 0;

        pPrevMemory = paMemoryHash[nHash];

        for( pCurrentMemory = pPrevMemory->pNext;
             pCurrentMemory && !bFound;
             pPrevMemory = pPrevMemory->pNext,
             pCurrentMemory = pPrevMemory->pNext )
        {
            if( pCurrentMemory->pvMemory == pvMemory )
            {
                /* Unlink block */
                pPrevMemory->pNext = pCurrentMemory->pNext;

                /* Rewrite the structure to reflect new data */
                pCurrentMemory->nAmountAllocated = nSize;
                pCurrentMemory->szFileName       = szFileName;
                pCurrentMemory->nLineNumber      = nLineNumber;
                pCurrentMemory->pvMemory         = realloc( pvMemory, nSize );
    
                pvReallocMemory = pCurrentMemory->pvMemory;

                /* Reinsert structure */
                nHash = (int)pCurrentMemory->pvMemory % MEMORY_HASH;
                pCurrentMemory->pNext = paMemoryHash[nHash];
                paMemoryHash[nHash]   = pCurrentMemory;

                bFound = 1;
            }
        }
    }

    if( !pvReallocMemory )
    {
        // fprintf( stderr, "Memory corruption : %p at %s:%d\n\r",
        //    pvMemory, szFileName, nLineNumber );

        printf( "Memory corruption : %p at %s:%d\n", pvMemory, szFileName, nLineNumber );

        report_corruption( pvMemory );

        pvReallocMemory = mc_memory_malloc( nSize, szFileName, nLineNumber );
    }

    return pvReallocMemory;
}

/*
 * Frees up memory and takes it out of the hash table.
 */
void mc_memory_free( void *pvMemory, char const *szFileName, int nLineNumber )
{
    mc_memory_t *pCurrentMemory;
    int          nHash;
    int          bFound;

    /* obtain the hash value of the memory */
    nHash = ((int)pvMemory) % MEMORY_HASH;
    
    /* start in that bucket */
    pCurrentMemory = paMemoryHash[nHash];

    if( pCurrentMemory == NULL )
    {
        /* Memory Corruption */
        bFound = 0;
    }
    else if( pCurrentMemory->pvMemory == pvMemory )
    {
        /* The freed memory was at the top of the bucket */
        paMemoryHash[nHash] = pCurrentMemory->pNext;
        free( pCurrentMemory->pvMemory );
        free( pCurrentMemory );
        bFound = 1;
    }
    else
    {
        /* The freed memory is /somewhere/ in this bucket, hopefully
         * iterate through the bucket to find it and remove it
         */
        
        mc_memory_t *pNextMemory;

        bFound = 0;
        
        for( pNextMemory = pCurrentMemory->pNext;
             pNextMemory && !bFound;
             pNextMemory = pNextMemory->pNext,
             pCurrentMemory = pCurrentMemory->pNext )
        {
            if( pNextMemory->pvMemory == pvMemory )
            {
                pCurrentMemory->pNext = pNextMemory->pNext;
                free( pNextMemory->pvMemory );
                free( pNextMemory );
                bFound = 1;
            }
        }
    }

    if( !bFound )
    {
        /* Nothing was freed, so a memory corruption has occurred. */
        
        // fprintf( stderr, "Memory corruption : %p at %s:%d\n\r",
        //     pvMemory, szFileName, nLineNumber );
        printf( "Memory corruption : %p at %s:%d\n", pvMemory, szFileName, nLineNumber );
        report_corruption( pvMemory );
    }
}

/*
 * This will report on your memory leaks.
 * Unless compiled with "-DNOREPORTATEXIT", this will
 * automatically run upon normal exit of the program.
 *
 * However, it does no cleanup of its own, so you
 * can call it whenever you like to reveal current memory usage.
 */
void mc_memory_done( void )
{
    int          nHash;
    int          nTotal;
    mc_memory_t *pCurrentMemory;

    nTotal = 0;

    for( nHash = 0; nHash < MEMORY_HASH; nHash++ )
    {
        pCurrentMemory = paMemoryHash[nHash];

        while( pCurrentMemory )
        {
            /* Old
            fprintf( stderr, "Memory leak : %p in bucket %d at %s:%d, %d bytes\n\r",
                    pCurrentMemory->pvMemory,
                    nHash,
                    pCurrentMemory->szFileName,
                    pCurrentMemory->nLineNumber,
                    pCurrentMemory->nAmountAllocated );
            */
            /* New
            printf( "Memory leak : %p in bucket %d at %s:%d, %d bytes\n",
                    pCurrentMemory->pvMemory,
                    nHash,
                    pCurrentMemory->szFileName,
                    pCurrentMemory->nLineNumber,
                    pCurrentMemory->nAmountAllocated );
            */
            nTotal += pCurrentMemory->nAmountAllocated;

            pCurrentMemory = pCurrentMemory->pNext;
        }
    }

    if( nTotal > 0 )
    {
        printf( "Total memory leaked: %d bytes\n", nTotal );
    }
}

/*
 * Reports a corruption and the place it occurred.
 * Returns the size of the corruption.
 */
int report_corruption( void *pvMemory )
{
    int          nHash;
    int          nSize = 0;
    mc_memory_t *pCurrentMemory;
    short        bFound = 0;

    /* Cycle through each hash bucket */
    for( nHash = 0; nHash < MEMORY_HASH && !bFound; ++nHash )
    {
        /* Iterate through the bucket's list */
        for( pCurrentMemory = paMemoryHash[nHash];
             pCurrentMemory && !bFound;
             pCurrentMemory = pCurrentMemory->pNext )
        {
            if( ( (unsigned int)pCurrentMemory->pvMemory 
                < (unsigned int)pvMemory )
             && ( (unsigned int)pCurrentMemory->pvMemory
                    + pCurrentMemory->nAmountAllocated 
                >= (unsigned int)pvMemory ) )
            {
                /*
                fprintf( stderr, "   This is in the memory space "
                    "allocated for %p in bucket %d (%d bytes at %s:%d)\n\r",
                    pCurrentMemory->pvMemory,
                    nHash,
                    pCurrentMemory->nAmountAllocated,
                    pCurrentMemory->szFileName,
                    pCurrentMemory->nLineNumber );
                */
                printf( "   This is in the memory space "
                    "allocated for %p in bucket %d (%d bytes at %s:%d)\n",
                    pCurrentMemory->pvMemory,
                    nHash,
                    pCurrentMemory->nAmountAllocated,
                    pCurrentMemory->szFileName,
                    pCurrentMemory->nLineNumber );

                bFound = 1;
                nSize = pCurrentMemory->nAmountAllocated;
            }
        }
    }

    return nSize;
}
