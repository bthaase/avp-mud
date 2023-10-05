/***************************************************************************
                            STAR WARS REALITY 1.0
    --------------------------------------------------------------------------
    Star Wars Reality Code Additions and changes from the Smaug Code
    copyright (c) 1997 by Sean Cooper
    -------------------------------------------------------------------------
    Starwars and Starwars Names copyright(c) Lucas Film Ltd.
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
                          Greeting System routines
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"

bool knows_player( CHAR_DATA* ch, CHAR_DATA* victim )
{
    if ( !ch || !victim )
        return FALSE;

    if ( IS_NPC( ch ) || IS_NPC( victim ) )
        return TRUE;

    if ( ch == victim )
        return TRUE;

    if ( !ch->pcdata || !victim->pcdata )
        return TRUE;

    if ( IS_IMMORTAL( ch ) || IS_IMMORTAL( victim ) )
        return TRUE;

    if ( ch->race == victim->race )
        return TRUE;

    return FALSE;
}

char* make_greet_desc( CHAR_DATA* ch, CHAR_DATA* looker )
{
    static char buf[MAX_STRING_LENGTH];
    strcpy( buf, get_race( ch ) );
    return buf;
}

char* g_name( CHAR_DATA* ch, CHAR_DATA* looker )
{
    static char buf[MAX_STRING_LENGTH];

    if ( !ch )
        return NULL;

    strcpy( buf, ch->name );

    if ( !looker )
        return buf;

    if ( ch == looker )
        return buf;

    if ( IS_NPC( ch ) || IS_NPC( looker ) )
        return buf;

    if ( !ch->pcdata || !looker->pcdata )
        return buf;

    if ( IS_IMMORTAL( ch ) || IS_IMMORTAL( looker ) )
        return buf;

    if ( knows_player( looker, ch ) )
        return buf;

    if ( !ch->pcdata->gname )
        assign_gname( ch );

    strcpy( buf, ch->pcdata->gname );
    return buf;
}


void assign_gname( CHAR_DATA* ch )
{
    char buf[MAX_STRING_LENGTH];

    if ( !ch )
        return;

    if ( !ch->pcdata )
        return;

    strcpy( buf, get_race( ch ) );

    // strcpy( buf, aoran_cap( buf ) );

    if ( ch->pcdata->gname )
        DISPOSE( ch->pcdata->gname );

    ch->pcdata->gname = str_dup( buf );
    // bug( "Assigned name '%s'", ch->pcdata->gname );
    return;
}
