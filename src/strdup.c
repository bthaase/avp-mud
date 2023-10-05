#include <stdio.h>
#include <stdlib.h>

/*  fix strdup bug
    appears some versions of gcc get a core dump when a strdup is free()d
    this, when linked with your program will overide the strdup function
    that is supplied with the standard lib
*/

char* strdup( char* s )
{
    char* d;
    d = malloc( strlen( s ) + 1 );

    if ( !d )
    {
        fprintf( stderr, "strdup err: %s", d );
        fflush( stderr );
        return ( NULL );
    }

    strcpy( d, s );
    return ( d );
}
