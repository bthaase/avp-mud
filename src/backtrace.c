#include "mud.h"
#include <backtrace.h>
#include <time.h>
#include <string.h>

#define GIT_SHORT_HASH "unknown"


static void error_callback( void* data __attribute__( ( unused ) ), const char* msg, int errnum )
{
    fprintf( stderr, "libbacktrace error: %s (%d)\n", msg, errnum );
}

static int full_callback( void* data, uintptr_t pc, const char* filename, int lineno, const char* function )
{
    char* buf = ( char* )data;
    char line_info[256];

    if ( function && filename )
    {
        snprintf( line_info, sizeof( line_info ), "%s() in %s:%d\n", function, filename, lineno );
    }
    else if ( function )
    {
        snprintf( line_info, sizeof( line_info ), "%s()\n", function );
    }
    else
    {
        snprintf( line_info, sizeof( line_info ), "0x%lx\n", ( long )pc );
    }

    strncat( buf, line_info, MSL - strlen( buf ) - 1 );
    return 0;  // continue
}


char* MudBackTrace( void )
{
    static char mybugbuf[MSL];
    struct backtrace_state* state;
    mybugbuf[0] = '\0';
    state = backtrace_create_state( NULL, 1, error_callback, NULL );

    if ( !state )
    {
        return "Failed to create backtrace state!";
    }

    backtrace_full( state, 0, full_callback, error_callback, mybugbuf );
    return mybugbuf;
}

void capturebacktrace( const char* type )
{
    char buf[MAX_INPUT_LENGTH], buf2[SUB_MSL];

    // // Mud backtrace.
    if ( TRUE )
    {
        int timestamp = ( int )time( 0 );
        const char* backtrace = MudBackTrace();
        snprintf( buf, MAX_INPUT_LENGTH, "%s%d", BACKTRACE_DIR, timestamp );
        snprintf( buf2, SUB_MSL, "#BACKTRACE V0\n\rTIMESTAMP: %d\n\rGIT_SHORT_HASH: %s\n\rTYPE: %s\n\rBACKTRACE:\n\r%s\n\r#END\n\r", timestamp, GIT_SHORT_HASH, type, backtrace );
        FILE* fp = fopen( buf, "w" );

        if ( !fp )
        {
            bug( "Could not open backtrace file!" );
        }
        else
        {
            fprintf( fp, "%s", buf2 );
            fclose( fp );
        }

        fprintf( stderr, "%s", buf2 );
    }
}
