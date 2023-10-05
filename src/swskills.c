/***************************************************************************
                            STAR WARS REALITY 1.0
    --------------------------------------------------------------------------
    Star Wars Reality Code Additions and changes from the Smaug Code
    copyright (c) 1997 by Sean Cooper
    -------------------------------------------------------------------------
    Starwars and Starwars Names copyright(c) Lucasfilm Ltd.
    --------------------------------------------------------------------------
    SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider
    SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,
    Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops
    ------------------------------------------------------------------------
    Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael
    Chastain, Michael Quan, and Mitchell Tse.
    Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,
    Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.
    ------------------------------------------------------------------------
           New Star Wars Skills Unit
****************************************************************************/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void              add_reinforcements  args( ( CHAR_DATA* ch ) );
ch_ret            one_hit             args( ( CHAR_DATA* ch, CHAR_DATA* victim, int dt ) );
ROOM_INDEX_DATA* generate_exit       args( ( ROOM_INDEX_DATA* in_room, EXIT_DATA** pexit ) );
CHAR_DATA*        get_char_room_mp    args( ( CHAR_DATA* ch, char* argument ) );

extern int        top_affect;

void do_call( CHAR_DATA* ch, char* argument )
{
    send_to_char( "Sorry, this command is incomplete.\n\r", ch );
    bug( "swskills.c: incomplete code under do_call." );
    return;
}

/* syntax throw <obj> [direction] [target] */

