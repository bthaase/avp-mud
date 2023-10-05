/*
    Matthew Chaplain's Memory Leak Tracer
    Copyright (c) 2000, All Rights Reserved
*/
#include <stdio.h>

#ifndef MC_MEMORY_H_
#define MC_MEMORY_H_

/*
    Initialise the memory using the macro MEMORY_INIT.

    To use:
    Copy mc_mem.c and mc_mem.h into your directory.
    In any file that uses malloc or free, include mc_mem.h.
    Instead of using "malloc", "calloc", "realloc" and "free", use
    "MALLOC", "CALLOC", "REALLOC" and "FREE" respectively.

    Following one of the above macros (MALLOC, CALLOC, etc.) with an
    underscore enables you to specify the file and line number you
    are calling from (hence MALLOC_ and CALLOC_).

    Ensure that mc_mem.o (compiled from mc_mem.c) is linked in with
    your executable.

    The memory leak tracer only traces memory if the symbol DEBUG
    is defined (pass -DDEBUG to the compiler or define it in the code
    for more modular control).  If this is not defined, mc_mem.h
    replaces MALLOC and FREE with the normal malloc and free from
    stdlib.h.

    Also possible is passing a MEMORY_HASH symbol, with a numeric
    value (preferably a prime number) to set the size of the hash
    to work with.  Higher numbers equal more efficiency, at the
    expense of memory.

    Unless a symbol NOREPORTATEXIT is passed, the tracer will register
    a call to mc_memory_done() with the "atexit" standard function.

    Lastly, if a symbol MC_MEM_INTRO is passed (using -DMC_MEM_INTRO
    at the compile line), the a short message will be printed
    with the currently set options.

    All error reporting is done to the standard error file.

    You can see small examples of the memory tracer in use (indeed, the
    tracer was most useful in their development) in my data structure
    snippets at http://snippets.kastagaar.com
*/

#ifndef DEBUG

    #define MEMORY_INIT()
    #define MALLOC( x ) malloc( (x) )
    #define CALLOC( x, y ) calloc( (x), (y) )
    #define REALLOC( x, y ) realloc( (x), (y) )
    #define FREE( x ) free( (x) )

    #define MALLOC_( x, y, z ) malloc( (x) )
    #define CALLOC_( w, x, y, z ) calloc( (w), (x) )
    #define REALLOC_( w, x, y, z ) realloc( (w), (x) )
    #define FREE_( x, y, z ) free( (x) )

#else /* DEBUG is defined */

    #define MEMORY_INIT() mc_memory_init()
    #define MALLOC( x ) mc_memory_malloc( (x), __FILE__, __LINE__ )
    #define CALLOC( x, y ) mc_memory_calloc( (x), (y),  __FILE__, __LINE__ )
    #define REALLOC( x, y ) mc_memory_realloc( (x), (y), __FILE__, __LINE__ )
    #define FREE( x ) mc_memory_free( (x), __FILE__, __LINE__ )

    #define MALLOC_( x, y, z ) mc_memory_malloc( (x), (y), (z) )
    #define CALLOC_( w, x, y, z ) mc_memory_calloc( (w), (x), (y), (z) )
    #define REALLOC_( w, x, y, z ) mc_memory_realloc( (w), (x), (y), (z) )
    #define FREE_( x, y, z ) mc_memory_free( (x), (y), (z) )

#endif

void  mc_memory_init( void );
void* mc_memory_malloc( size_t nSize, char const* szFileName, int nLineNumber );
void* mc_memory_calloc( size_t nElements, size_t nSize,
                        char const* szFileName, int nLineNumber );
void* mc_memory_realloc( void* pvMemory, size_t nSize,
                         char const* szFileName, int nLineNumber );
void  mc_memory_free( void* pvMemory, char const* szFileName, int nLineNumber );
void  mc_memory_done( void );

#endif

