/***************************************************************************
*                           STAR WARS REALITY 1.0                          *
*--------------------------------------------------------------------------*
* Star Wars Reality Code Additions and changes from the Smaug Code         *
* copyright (c) 1997 by Sean Cooper                                        *
* -------------------------------------------------------------------------*
* Starwars and Starwars Names copyright(c) Lucas Film Ltd.                 *
*--------------------------------------------------------------------------*
* SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider                           *
* SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,                    *
* Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops                *
* ------------------------------------------------------------------------ *
* Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
* Chastain, Michael Quan, and Mitchell Tse.                                *
* Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
* Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
* ------------------------------------------------------------------------ *
*                           Virtual Rooms Module                           *   
****************************************************************************/
 
#include <math.h> 
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

WILD_DATA * first_wild;
WILD_DATA * last_wild;

char *  const   wild_type[] =
{
   "tundra", "jungle", "forest", "ocean", "mountain", "desert", "$"
};

void write_wild_list( )
{
    WILD_DATA *twild;
    FILE *fpout;
    char filename[256];
    
    sprintf( filename, "%s%s", SYSTEM_DIR, WILD_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
         bug( "FATAL: cannot open wild.lst for writing!\n\r", 0 );
         return;
    }
    for ( twild = first_wild; twild; twild = twild->next )
    {
        fprintf( fpout, "%d\n", twild->id );
        fprintf( fpout, "%d\n", twild->type );
        fprintf( fpout, "%d\n", twild->xpos );
        fprintf( fpout, "%d\n", twild->ypos );
    }
    fprintf( fpout, "$\n" );
    fclose( fpout );    
}

