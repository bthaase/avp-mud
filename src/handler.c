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
                Main structure manipulation module
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>      /* Soundex requirement */
#include <string.h>
#include <time.h>
#include "soundex.h"
#include "mud.h"

#define BFS_MARK         BV01

extern int      top_exit;
extern int      top_ed;
extern int      top_affect;
extern int      cur_qobjs;
extern int      cur_qchars;
extern CHAR_DATA*   gch_prev;
extern OBJ_DATA*    gobj_prev;

CHAR_DATA*   cur_char;
ROOM_INDEX_DATA* cur_room;
bool         cur_char_died;
ch_ret       global_retcode;

int      cur_obj;
int      cur_obj_serial;
bool         cur_obj_extracted;
obj_ret      global_objcode;

bool is_wizvis( CHAR_DATA* ch, CHAR_DATA* victim );
OBJ_DATA* group_object( OBJ_DATA* obj1, OBJ_DATA* obj2 );

void vec_explode_1( ROOM_INDEX_DATA* room, int blast, int range );
void vec_explode_2( ROOM_INDEX_DATA* room, int blast, int range );
void room_explode( OBJ_DATA* obj, CHAR_DATA* xch, ROOM_INDEX_DATA* room );
void room_explode_1( OBJ_DATA* obj, CHAR_DATA* xch, ROOM_INDEX_DATA* room, int blast, int range, int type );
void room_explode_2( ROOM_INDEX_DATA* room, int blast, int range );
bool using_nvg( CHAR_DATA* ch );
char LetterConversion( char chLetter );

/*
    Detonates a object.
*/
void  explode( OBJ_DATA* obj )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA* room = NULL;
    OBJ_DATA* cont = NULL;
    OBJ_DATA* scraps = NULL;
    bool held = FALSE;
    int tmp_tk = 0;

    /* Is it being held? */
    if ( obj->carried_by )
    {
        act( AT_WHITE, "$p EXPLODES in $n's hands!", obj->carried_by, obj, NULL, TO_ROOM );
        act( AT_WHITE, "$p EXPLODES in your hands!", obj->carried_by, obj, NULL, TO_CHAR );

        // Disable this characters Tumble, for being an idiot
        if ( !IS_NPC( obj->carried_by ) && obj->carried_by->pcdata )
            if ( obj->carried_by->pcdata->learned[gsn_tumble] )
                obj->carried_by->pcdata->prepared[gsn_tumble] = skill_table[gsn_tumble]->reset - 1;

        room = obj->carried_by->in_room;
        held = TRUE;
    }
    else if ( obj->in_room )
    {
        room = obj->in_room;
    }
    else if ( obj->in_obj )
    {
        for ( cont = obj->in_obj; cont; cont = cont->in_obj )
        {
            if ( !cont || cont == NULL )
                break;

            if ( cont->in_room )
            {
                room = cont->in_room;
                break;
            }
            else if ( cont->carried_by )
            {
                room = cont->carried_by->in_room;
                break;
            }
        }
    }
    else
    {
        room = NULL;
    }

    /*
        Armed by a character?
    */
    if ( obj->armed_by )
    {
        CHAR_DATA* xch = NULL;
        int vnum = 0;

        if ( ( vnum = atoi( obj->armed_by ) ) > 0 )
        {
            for ( xch = first_char; xch; xch = xch->next )
            {
                if ( IS_NPC( xch ) && xch->pIndexData && vnum == xch->pIndexData->vnum )
                {
                    if ( !held && room->first_person )
                        act( AT_WHITE, "$p EXPLODES!", room->first_person, obj, NULL, TO_ROOM );

                    strcpy( buf, obj->armed_by );
                    tmp_tk = xch->teamkill;
                    room_explode( obj, xch, room );
                    xch->teamkill = UMAX( tmp_tk - 1, xch->teamkill );
                    break;
                }
            }
        }
        else
        {
            for ( xch = first_char; xch; xch = xch->next )
            {
                if ( !IS_NPC( xch ) && nifty_is_name( obj->armed_by, xch->name ) )
                {
                    if ( !held && room->first_person )
                        act( AT_WHITE, "$p EXPLODES!", room->first_person, obj, NULL, TO_ROOM );

                    strcpy( buf, xch->name );
                    tmp_tk = xch->teamkill;
                    room_explode( obj, xch, room );
                    xch->teamkill = UMAX( tmp_tk - 1, xch->teamkill );
                    scraps = make_scraps( obj );
                    scraps->killed_by = STRALLOC( buf );
                    return;
                }
            }

            /* Make sure nobody is holding it, and that someone is in the room */
            if ( !held && room->first_person )
            {
                act( AT_WHITE, "$p EXPLODES!", room->first_person, obj, NULL, TO_ROOM );
            }

            room_explode( obj, NULL, room );
        }
    }
    else if ( room ) /* Do we have a room to explode in? */
    {
        /* Make sure nobody is holding it, and that someone is in the room */
        if ( !held && room->first_person )
        {
            act( AT_WHITE, "$p EXPLODES!", room->first_person, obj, NULL, TO_ROOM );
        }

        room_explode( obj, NULL, room );
    }

    /* Destroy the explosive */
    scraps = make_scraps( obj );
    /* Record the user */
    scraps->killed_by = STRALLOC( buf );
    return;
}

void room_explode( OBJ_DATA* obj, CHAR_DATA* xch, ROOM_INDEX_DATA* room )
{
    int blast, range, type = 0;

    /* Calculate the blast value */
    if ( obj->item_type == ITEM_COVER )
    {
        blast = obj->weight * 10;
        range = 1;
        type = 0; // Fire
    }
    else if ( obj->item_type == ITEM_LANDMINE )
    {
        blast = number_range( obj->value[0], obj->value[1] );
        range = 1;
        type = obj->value[4];
    }
    else
    {
        blast = number_range( obj->value[0], obj->value[1] );
        range = UMAX( 0, obj->value[2] );
        type = obj->value[3];
    }

    // if ( type < 0 || type > MAX_RIS_FLAGS ) return;
    /* Begin the detonation sequence */
    room_explode_1( obj, xch, room, blast, range, type );
    room_explode_2( room, blast, range );
    return;
}

void room_explode_1( OBJ_DATA* obj, CHAR_DATA* xch, ROOM_INDEX_DATA* room, int blast, int range, int type )
{
    AFFECT_DATA af;
    CHAR_DATA* rch;
    CHAR_DATA* rnext;
    OBJ_DATA*  robj;
    OBJ_DATA*  robj_next;
    int tumble;

    /*
        Is this room 'clean' of the explosion?
    */
    if ( xIS_SET( room->room_flags, BFS_MARK ) )
        return;

    /* Mark the room as being hit by the explosion */
    xSET_BIT( room->room_flags, BFS_MARK );

    /* Anybody in the room with the explosion? */
    for ( rch = room->first_person ; rch ;  rch = rnext )
    {
        rnext = rch->next_in_room;

        if ( is_spectator( rch ) || IN_VENT( rch ) )
            continue;

        if ( !IS_NPC( rch ) )
        {
            if ( rch->pcdata && rch->hit > 0 )
            {
                tumble = rch->pcdata->learned[gsn_tumble];

                if ( rch->pcdata->prepared[gsn_tumble] < skill_table[gsn_tumble]->reset )
                    tumble = 0;

                if ( xch == rch )
                    tumble = 0;

                if ( blast > ( rch->max_hit * 3 ) )
                    tumble = 0;

                if ( tumble > 0 )
                    tumble = ( 25 + ( tumble  * 25 ) );

                if ( number_percent() < tumble )
                {
                    ch_printf( rch, "&w&C(Tumble) You successfully dodged a grenade blast.\n\r" );
                    rch->pcdata->prepared[gsn_tumble] = 0;
                    continue;
                }
            }
        }

        if ( xch != NULL )
        {
            if ( !IS_NPC( xch ) )
                match_log( "GRENADE;'PC:%s' hit ':%s' with grenade blast.", xch->name, rch->name );
        }

        switch ( type )
        {
            default:
                break;

            case 0:  // Normal Types
            case 1:
            case 2:
            case 3:
            case 4:
                send_to_char( "&YThe shockwave from a massive explosion rips through your body!\n\r", rch );
                damage( xch, rch, blast, TYPE_GENERIC + type );
                break;

            case 5:  // Immolation Grenade
                send_to_char( "&YThe shockwave from a massive explosion rips through your body!\n\r", rch );
                damage( xch, rch, blast, TYPE_GENERIC + RIS_FIRE );
                ignite_target( xch, rch, ( int )( ( float )( blast ) / ( float )( 10 ) ) );
                break;
        }
    }

    /* Any objects to destroy? */
    for ( robj = room->first_content; robj; robj = robj_next )
    {
        robj_next = robj->next_content;
        set_cur_obj( robj );
        rch = robj->parent;
        robj->parent = xch;
        damage_obj( robj, blast );

        if ( !obj_extracted( robj ) )
            robj->parent = rch;
    }

    /* Spread the explosion to nearby rooms */
    {
        EXIT_DATA* pexit;

        for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        {
            if ( pexit->to_room && pexit->to_room != room )
            {
                if ( xIS_SET( pexit->exit_info, EX_ARMORED ) && xIS_SET( pexit->exit_info, EX_CLOSED ) )
                {
                    /* Blast can't penetrate closed, armored doors */
                    return;
                }
                else if ( range > 0 && blast > 1 )
                {
                    int roomblast;
                    roomblast = break_exit( room, pexit, roomblast );
                    roomblast = blast * 0.5;  /* 1|2 of the remaining blast */
                    room_explode_1( obj, xch, pexit->to_room, roomblast, range - 1, type );
                }
                else
                {
                    if ( !xIS_SET( pexit->to_room->room_flags, BFS_MARK ) )
                        echo_to_room( AT_WHITE, pexit->to_room, "You hear a massive explosion not too far from here!" );
                }
            }
        }
    }
}

int break_exit( ROOM_INDEX_DATA* room, EXIT_DATA* pexit, int roomblast )
{
    if ( xIS_SET( pexit->exit_info, EX_BLASTOPEN ) && !xIS_SET( pexit->exit_info, EX_BLASTED ) )
    {
        roomblast = ( int )( roomblast / 2 );
        echo_to_room( AT_CYAN, room, "The shockwave blasts open a new exit!" );
        echo_to_room( AT_CYAN, pexit->to_room, "The shockwave blasts open a new exit!" );
        set_bexit_flag( pexit, EX_BLASTED );
    }

    return roomblast;
}

void room_explode_2( ROOM_INDEX_DATA* room, int blast, int range )
{
    if ( !xIS_SET( room->room_flags, BFS_MARK ) )
        return;

    xREMOVE_BIT( room->room_flags, BFS_MARK );

    if ( blast > 0 && range > 0 )
    {
        int roomblast;
        EXIT_DATA* pexit;

        for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        {
            if ( pexit->to_room && pexit->to_room != room )
            {
                roomblast = blast * 0.5;  /* 1|2 of the remaining blast */
                room_explode_2( pexit->to_room, roomblast, range - 1 );
            }
        }
    }
}

bool contains_explosive( OBJ_DATA* obj, int antiloop )
{
    OBJ_DATA* pObj;

    if ( antiloop > 99999 )
    {
        bug( "contains_explosive: Reached nesting level of 99999. Aborting." );
        return FALSE;
    }

    if ( !obj->first_content )
        return FALSE;

    for ( pObj = obj->first_content; pObj; pObj = pObj->next_content )
    {
        if ( pObj->item_type == ITEM_CONTAINER )
        {
            if ( contains_explosive( pObj, ( antiloop + 1 ) ) )
                return TRUE;
        }
        else if ( pObj->item_type == ITEM_GRENADE )
        {
            if ( pObj->value[4] > 0 )
                return TRUE;
        }
    }

    return FALSE;
}

/*
    For use with auction. Determines total value of a container
*/
int contents_value( OBJ_DATA* obj, int antiloop )
{
    OBJ_DATA* pObj;
    int worth = 0;

    if ( antiloop > 99999 )
    {
        bug( "contents_value: Reached nesting level of 99999. Aborting." );
        return worth;
    }

    if ( !obj->first_content )
        return obj->cost;

    for ( pObj = obj->first_content; pObj; pObj = pObj->next_content )
    {
        worth += contents_value( pObj, ( antiloop + 1 ) );
    }

    return worth;
}

bool is_wizvis( CHAR_DATA* ch, CHAR_DATA* victim )
{
    if ( !IS_NPC( victim )
            &&   xIS_SET( victim->act, PLR_WIZINVIS )
            &&   get_trust( ch ) < victim->pcdata->wizinvis )
        return FALSE;

    return TRUE;
}

/*
    Calculate roughly how much experience a character is worth
*/
int get_exp_worth( CHAR_DATA* ch )
{
    OBJ_DATA* obj;
    int exp, texp, tcnt;
    exp = 10;

    if ( !IS_NPC( ch ) )
        exp *= 2;

    exp += 10 + ( ch->top_level / 2 );
    exp *= 10;
    return exp;
}

/*
    New combat routine -- Calculates the characters max AP
*/
int get_max_ap( CHAR_DATA* ch )
{
    OBJ_DATA* weapA = NULL;
    OBJ_DATA* weapB = NULL;
    int ap = 0;
    weapA = get_eq_char( ch, WEAR_WIELD );
    weapB = get_eq_char( ch, WEAR_DUAL_WIELD );

    if ( weapA != NULL && weapB != NULL )
    {
        ap = UMAX( weapA->value[5], weapB->value[5] );
        ap += 1; /* Tiny dual-wield penalty */

        if ( weapA->attach )  /* Fire Rate? */
            if ( weapA->attach->value[0] == 8 )
            {
                ap -= weapA->attach->value[2];
            }
            else if ( weapB->attach ) /* Fire Rate? */
                if ( weapB->attach->value[0] == 8 )
                    ap -= weapB->attach->value[2];
    }
    else if ( weapA != NULL )
    {
        ap = weapA->value[5];

        if ( weapA->attach )  /* Fire Rate? */
            if ( weapA->attach->value[0] == 8 )
                ap -= weapA->attach->value[2];
    }
    else
    {
        if ( ch->race == RACE_ALIEN )
            ap = 2;

        if ( ch->race == RACE_MARINE )
            ap = 3;

        if ( ch->race == RACE_PREDATOR )
            ap = 3;
    }

    ap = URANGE( 2, ap, 6 );
    return ap;
}

/*
    New combat routine -- Calculates the characters max MP
*/
int get_max_mp( CHAR_DATA* ch )
{
    int mp = 0;

    if ( ch->race == RACE_ALIEN )
        mp = 4;

    if ( ch->race == RACE_MARINE )
        mp = 3;

    if ( ch->race == RACE_PREDATOR )
        mp = 3;

    return mp;
}

/*
    New combat routine -- Calculates the max Morale for a Marine.
*/
int get_max_morale( CHAR_DATA* ch )
{
    int morale = 0;

    if ( ch->race != RACE_MARINE )
        return 0;

    morale = ( get_curr_bra( ch ) * 3 );
    morale = URANGE( 30, morale, 90 );
    return morale;
}

/*
    New combat routine -- Calculates the max Resin for an alien.
*/
int get_max_resin( CHAR_DATA* ch )
{
    int resin = 0;

    if ( ch == NULL )
        return 0;

    if ( ch->race != RACE_ALIEN )
        return 0;

    resin = 40 + ( get_curr_sta( ch ) * 2 );
    resin = URANGE( 40, resin, 100 );
    return resin;
}


/*
    New combat routine -- Calculates the max team kills allowed for a player.
*/
int get_max_teamkill( CHAR_DATA* ch )
{
    int tk = 3;
    // tk = ( ch->top_level / 4 ) + 2;
    // tk = URANGE( 2, tk, 7 );
    return tk;
}

/*
    Return RANK Name for a level and race
*/
char* get_rank( int race, int level )
{
    static char buf[MAX_STRING_LENGTH];
    strcpy( buf, "" );

    if ( race == RACE_MARINE )
    {
        if ( level == 1 )
            strcpy( buf, "Recruit" );

        if ( level >= 2 )
            strcpy( buf, "Private" );

        if ( level >= 3 )
            strcpy( buf, "Corporal" );

        if ( level >= 5 )
            strcpy( buf, "Sergeant" );

        if ( level >= 8 )
            strcpy( buf, "Lieutenant" );

        if ( level >= 11 )
            strcpy( buf, "Captain" );

        if ( level >= 14 )
            strcpy( buf, "Major" );

        if ( level >= 17 )
            strcpy( buf, "Colonel" );

        if ( level >= 20 )
            strcpy( buf, "General" );
    }
    else if ( race == RACE_ALIEN )
    {
        if ( level == 1 )
            strcpy( buf, "Broodling" );

        if ( level >= 2 )
            strcpy( buf, "Alien" );

        if ( level >= 3 )
            strcpy( buf, "Scout" );

        if ( level >= 5 )
            strcpy( buf, "Drone" );

        if ( level >= 8 )
            strcpy( buf, "Sentry" );

        if ( level >= 11 )
            strcpy( buf, "Hunter" );

        if ( level >= 14 )
            strcpy( buf, "Warrior" );

        if ( level >= 17 )
            strcpy( buf, "Guardian" );

        if ( level >= 20 )
            strcpy( buf, "Queen" );
    }
    else if ( race == RACE_PREDATOR )
    {
        if ( level == 1 )
            strcpy( buf, "Uninitiated" );

        if ( level >= 2 )
            strcpy( buf, "Initiate" );

        if ( level >= 3 )
            strcpy( buf, "Unblooded" );

        if ( level >= 5 )
            strcpy( buf, "Blooded" );

        if ( level >= 8 )
            strcpy( buf, "Student" );

        if ( level >= 11 )
            strcpy( buf, "Warrior" );

        if ( level >= 14 )
            strcpy( buf, "Champion" );

        if ( level >= 17 )
            strcpy( buf, "Elder" );

        if ( level >= 20 )
            strcpy( buf, "Ancient" );
    }

    return buf;
}

/*
    Return how much experience is required to reach a certain level
*/
int exp_level( sh_int level )
{
    int lvl;
    lvl = UMAX( 0, level - 1 );
    return ( lvl * 1000 );
}

/*
    Get what level ch is based on exp
*/
sh_int level_exp( CHAR_DATA* ch, int exp )
{
    int x, lastx, y, tmp;
    x = LEVEL_SUPREME;
    lastx = x;
    y = 0;

    while ( !y )
    {
        tmp = exp_level( x );
        lastx = x;

        if ( tmp > exp )
            x /= 2;
        else if ( lastx != x )
            x += ( x / 2 );
        else
            y = x;
    }

    if ( y < 1 )
        y = 1;

    if ( y > LEVEL_SUPREME )
        y = LEVEL_SUPREME;

    return y;
}

/*
    Retrieve a character's trusted level for permission checking.
*/
sh_int get_trust( CHAR_DATA* ch )
{
    if ( !ch )
        return 0;

    if ( ch->desc )
        if ( ch->desc->original )
            ch = ch->desc->original;

    if ( ch->trust != 0 )
        return ch->trust;

    if ( IS_NPC( ch ) && ch->top_level >= LEVEL_AVATAR )
        return LEVEL_AVATAR;

    if ( ch->top_level >= LEVEL_NEOPHYTE && IS_RETIRED( ch ) )
        return LEVEL_NEOPHYTE;

    return ch->top_level;
}


/*
    Retrieve a character's age.
*/
sh_int get_age( CHAR_DATA* ch )
{
    return 17 + ( ch->played + ( current_time - ch->logon ) ) / 14400;
}



/*
    Retrieve character's current strength.
*/
sh_int get_curr_str( CHAR_DATA* ch )
{
    sh_int max;
    max = 30;
    return URANGE( 1, ch->perm_str + ch->mod_str, max );
}



/*
    Retrieve character's current intelligence.
*/
sh_int get_curr_int( CHAR_DATA* ch )
{
    sh_int max;
    max = 30;
    return URANGE( 1, ch->perm_int + ch->mod_int, max );
}



/*
    Retrieve character's current stamina.
*/
sh_int get_curr_sta( CHAR_DATA* ch )
{
    sh_int max;
    max = 30;
    return URANGE( 1, ch->perm_sta + ch->mod_sta, max );
}



/*
    Retrieve character's current recovery.
*/
sh_int get_curr_rec( CHAR_DATA* ch )
{
    sh_int max;
    max = 30;
    return URANGE( 1, ch->perm_rec + ch->mod_rec, max );
}



/*
    Retrieve character's current bravery.
*/
sh_int get_curr_bra( CHAR_DATA* ch )
{
    sh_int max;
    max = 30;
    return URANGE( 1, ch->perm_bra + ch->mod_bra, max );
}

/*
    Retrieve character's current perception.
*/
sh_int get_curr_per( CHAR_DATA* ch )
{
    sh_int max;
    sh_int mod;
    max = 30;
    mod = ch->mod_per;

    /*
        Custom Modifier
    */
    if ( ch->in_room && ch->pcdata )
    {
        if ( ch->pcdata->learned[gsn_nightvision] > 0 )
        {
            if ( room_is_dark( NULL, ch->in_room ) )
                mod -= ( ( 3 - ch->pcdata->learned[gsn_nightvision] ) * 5 );
        }
    }

    return URANGE( 1, ch->perm_per + mod, max );
}

/*
    Retrieve character's perception modifier.
*/
sh_int get_mod_per( CHAR_DATA* ch )
{
    sh_int mod;
    mod = ch->mod_per;

    /*
        Custom Modifier
    */
    if ( ch->in_room && ch->pcdata )
    {
        if ( ch->pcdata->learned[gsn_nightvision] > 0 )
        {
            if ( room_is_dark( NULL, ch->in_room ) )
                mod -= ( ( 3 - ch->pcdata->learned[gsn_nightvision] ) * 5 );
        }
    }

    return mod;
}

/*
    Retrieve a character's carry capacity. (Number)
    Vastly reduced (finally) due to containers       -Thoric
*/
int can_carry_n( CHAR_DATA* ch )
{
    int penalty = 0;

    if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_PET ) )
        return 0;

    return URANGE( 5, get_curr_str( ch ), 20 );
}



/*
    Retrieve a character's carry capacity. (Weight)
*/
int can_carry_w( CHAR_DATA* ch )
{
    int weight = 0;

    if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_PET ) )
        return 0;

    weight = 25 + ( get_curr_str( ch ) * 5 );

    if ( ch->race == RACE_PREDATOR )
        weight *= 2;

    if ( ch->race == RACE_ALIEN )
        weight /= 2;

    return weight;
}


/*
    See if a player/mob can take a piece of prototype eq     -Thoric
*/
bool can_take_proto( CHAR_DATA* ch )
{
    if ( IS_IMMORTAL( ch ) )
        return TRUE;
    else if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_PROTOTYPE ) )
        return TRUE;
    else
        return FALSE;
}


/*
    See if a string is one of the names of an object.
*/
bool is_name( const char* str, char* namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        namelist = one_argument( namelist, name );

        if ( name[0] == '\0' )
            return FALSE;

        if ( !str_cmp( str, name ) )
            return TRUE;
    }
}

bool is_name_prefix( const char* str, char* namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        namelist = one_argument( namelist, name );

        if ( name[0] == '\0' )
            return FALSE;

        if ( !str_prefix( str, name ) )
            return TRUE;
    }
}

/*
    See if a string is one of the names of an object.        -Thoric
    Treats a dash as a word delimiter as well as a space
*/
bool is_name2( const char* str, char* namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        namelist = one_argument2( namelist, name );

        if ( name[0] == '\0' )
            return FALSE;

        if ( !str_cmp( str, name ) )
            return TRUE;
    }
}

bool is_name2_prefix( const char* str, char* namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        namelist = one_argument2( namelist, name );

        if ( name[0] == '\0' )
            return FALSE;

        if ( !str_prefix( str, name ) )
            return TRUE;
    }
}

/*                              -Thoric
    Checks if str is a name in namelist supporting multiple keywords
*/
bool nifty_is_name( char* str, char* namelist )
{
    char name[MAX_INPUT_LENGTH];

    if ( !str || str[0] == '\0' )
        return FALSE;

    for ( ; ; )
    {
        str = one_argument2( str, name );

        if ( name[0] == '\0' )
            return TRUE;

        if ( !is_name2( name, namelist ) )
            return FALSE;
    }
}

bool nifty_is_name_prefix( char* str, char* namelist )
{
    char name[MAX_INPUT_LENGTH];

    if ( !str || str[0] == '\0' )
        return FALSE;

    for ( ; ; )
    {
        str = one_argument2( str, name );

        if ( name[0] == '\0' )
            return TRUE;

        if ( !is_name2_prefix( name, namelist ) )
            return FALSE;
    }
}

/*
    Apply or remove an affect to a character.
*/
void affect_modify( CHAR_DATA* ch, AFFECT_DATA* paf, bool fAdd )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA* wield;
    int mod, i;
    struct skill_type* skill;
    ch_ret retcode;
    mod = paf->modifier;

    if ( fAdd )
    {
        for ( i = 0; i < MAX_AFFECTED_BY; i++ )
            if ( xIS_SET( paf->bitvector, i ) )
                xSET_BIT( ch->affected_by, i );
    }
    else
    {
        for ( i = 0; i < MAX_AFFECTED_BY; i++ )
            if ( xIS_SET( paf->bitvector, i ) )
                xREMOVE_BIT( ch->affected_by, i );

        switch ( paf->location )
        {
            case APPLY_AFFECT:
                REMOVE_BIT( ch->affected_by.bits[0], mod );
                return;
        }

        mod = 0 - mod;
    }

    switch ( paf->location )
    {
        default:
            sprintf( buf, "Affect_modify: bad location %d on %s.", paf->location, ch->name );
            bug( buf );
            //bug( "Affect_modify: unknown location %d.", paf->location );
            return;

        case APPLY_NONE:
            break;

        case APPLY_STR:
            ch->mod_str       += mod;
            break;

        case APPLY_STA:
            ch->mod_sta               += mod;
            break;

        case APPLY_REC:
            ch->mod_rec               += mod;
            break;

        case APPLY_INT:
            ch->mod_int               += mod;
            break;

        case APPLY_BRA:
            ch->mod_bra               += mod;
            break;

        case APPLY_PER:
            ch->mod_per               += mod;
            break;

        /*
            Regular apply types
        */
        case APPLY_HIT:
            ch->max_hit       += mod;
            break;

        case APPLY_MOVE:
            ch->max_move      += mod;
            break;

        /*
            Bitvector modifying apply types
        */
        case APPLY_AFFECT:
            SET_BIT( ch->affected_by.bits[0], mod );
            break;

        /*
            Protection Modifiers
        */
        case APPLY_FIRE:
            ch->protect[RIS_FIRE]     += mod;
            break;

        case APPLY_ENERGY:
            ch->protect[RIS_ENERGY]   += mod;
            break;

        case APPLY_IMPACT:
            ch->protect[RIS_IMPACT]   += mod;
            break;

        case APPLY_PIERCE:
            ch->protect[RIS_PIERCE]   += mod;
            break;

        case APPLY_ACID:
            ch->protect[RIS_ACID]     += mod;
            break;
    }

    /*
        Check for weapon wielding.
        Guard against recursion (for weapons with affects).
    */
    if ( !IS_NPC( ch )
            &&   saving_char != ch
            &&  ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
            &&   get_obj_weight( wield ) > get_curr_str( ch ) )
    {
        static int depth;

        if ( depth == 0 )
        {
            depth++;
            act( AT_ACTION, "You are too weak to wield $p any longer.",
                 ch, wield, NULL, TO_CHAR );
            act( AT_ACTION, "$n stops wielding $p.", ch, wield, NULL, TO_ROOM );
            unequip_char( ch, wield );
            depth--;
        }
    }

    return;
}

/*
    Give an affect to a char.
*/
void affect_to_char( CHAR_DATA* ch, AFFECT_DATA* paf )
{
    AFFECT_DATA* paf_new;

    if ( !ch )
    {
        bug( "Affect_to_char(NULL, %d)", paf ? paf->type : 0 );
        return;
    }

    if ( !paf )
    {
        bug( "Affect_to_char(%s, NULL)", ch->name );
        return;
    }

    CREATE( paf_new, AFFECT_DATA, 1 );
    LINK( paf_new, ch->first_affect, ch->last_affect, next, prev );
    paf_new->type   = paf->type;
    paf_new->duration   = paf->duration;
    paf_new->location   = paf->location;
    paf_new->modifier   = paf->modifier;
    paf_new->bitvector  = paf->bitvector;
    affect_modify( ch, paf_new, TRUE );
    return;
}


/*
    Remove an affect from a char.
*/
void affect_remove( CHAR_DATA* ch, AFFECT_DATA* paf )
{
    if ( !ch->first_affect )
    {
        bug( "Affect_remove(%s, %d): no affect.", ch->name,
             paf ? paf->type : 0 );
        return;
    }

    affect_modify( ch, paf, FALSE );
    UNLINK( paf, ch->first_affect, ch->last_affect, next, prev );
    DISPOSE( paf );
    return;
}

/*
    Strip all affects of a given sn.
*/
void affect_strip( CHAR_DATA* ch, int sn )
{
    AFFECT_DATA* paf;
    AFFECT_DATA* paf_next;

    for ( paf = ch->first_affect; paf; paf = paf_next )
    {
        paf_next = paf->next;

        if ( paf->type == sn )
            affect_remove( ch, paf );
    }

    return;
}



/*
    Return true if a char is affected by a spell.
*/
bool is_affected( CHAR_DATA* ch, int sn )
{
    AFFECT_DATA* paf;

    for ( paf = ch->first_affect; paf; paf = paf->next )
        if ( paf->type == sn )
            return TRUE;

    return FALSE;
}


/*
    Add or enhance an affect.
    Limitations put in place by Thoric, they may be high... but at least
    they're there :)
*/
void affect_join( CHAR_DATA* ch, AFFECT_DATA* paf )
{
    AFFECT_DATA* paf_old;

    for ( paf_old = ch->first_affect; paf_old; paf_old = paf_old->next )
        if ( paf_old->type == paf->type )
        {
            paf->duration = UMIN( 1000000, paf->duration + paf_old->duration );

            if ( paf->modifier )
                paf->modifier = UMIN( 5000, paf->modifier + paf_old->modifier );
            else
                paf->modifier = paf_old->modifier;

            affect_remove( ch, paf_old );
            break;
        }

    affect_to_char( ch, paf );
    return;
}


/*
    Move a char out of a room.
*/
void char_from_room( CHAR_DATA* ch )
{
    OBJ_DATA* obj;

    if ( !ch )
    {
        bug( "Char_from_room: NULL char.", 0 );
        return;
    }

    if ( !ch->in_room )
    {
        bug( "Char_from_room: NULL room: %s", ch->name );
        return;
    }

    if ( !IS_NPC( ch ) )
        --ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
            &&   obj->item_type == ITEM_LIGHT
            &&   obj->value[2] != 0
            &&   IS_SET( obj->value[3], 1 << 0 )
            &&   ch->in_room->light > 0 )
        --ch->in_room->light;

    ch->in_room->light -= attach_light_modifier( ch );
    UNLINK( ch, ch->in_room->first_person, ch->in_room->last_person,
            next_in_room, prev_in_room );
    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->prev_in_room = NULL;

    if ( !IS_NPC( ch )
            &&   get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
        remove_timer( ch, TIMER_SHOVEDRAG );

    return;
}


/*
    Move a char into a room.
*/
void char_to_room( CHAR_DATA* ch, ROOM_INDEX_DATA* pRoomIndex )
{
    OBJ_DATA* obj;

    if ( !ch )
    {
        bug( "Char_to_room: NULL ch!", 0 );
        return;
    }

    if ( !pRoomIndex )
    {
        char buf[MAX_STRING_LENGTH];
        sprintf( buf, "Char_to_room: %s -> NULL room!  Putting char in limbo (%d)",
                 ch->name, ROOM_VNUM_LIMBO );
        bug( buf, 0 );
        /*  This used to just return, but there was a problem with crashing
            and I saw no reason not to just put the char in limbo. -Narn */
        pRoomIndex = get_room_index( ROOM_VNUM_LIMBO );
    }

    ch->in_room     = pRoomIndex;
    LINK( ch, pRoomIndex->first_person, pRoomIndex->last_person, next_in_room, prev_in_room );

    if ( !IS_NPC( ch ) )
        if ( ++ch->in_room->area->nplayer > ch->in_room->area->max_players )
            ch->in_room->area->max_players = ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
            &&   obj->item_type == ITEM_LIGHT
            &&   IS_SET( obj->value[3], 1 << 0 )
            &&   obj->value[2] != 0 )
        ++ch->in_room->light;

    ch->in_room->light += attach_light_modifier( ch );

    if ( !IS_NPC( ch ) && get_timer( ch, TIMER_SHOVEDRAG ) <= 0 )
        add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );  /*-30 Seconds-*/

    if ( ( !IS_NPC( ch ) || IS_BOT( ch ) ) )
    {
        if ( is_home( ch ) )
        {
            if ( !ch->was_home )
            {
                if ( IS_BOT( ch ) )
                {
                    match_log( "EVENT;'BOT:%s' is home.", ch->name );
                }
                else
                {
                    match_log( "EVENT;'PC:%s' is home.", ch->name );
                }

                ch->was_home = TRUE;
            }
        }
        else
        {
            if ( ch->was_home )
            {
                if ( IS_BOT( ch ) )
                {
                    match_log( "EVENT;'BOT:%s' is deployed.", ch->name );
                }
                else
                {
                    match_log( "EVENT;'PC:%s' is deployed.", ch->name );
                }

                ch->was_home = FALSE;
            }
        }
    }

    /* Do we need to update swarm states? */
    if ( ch->race == RACE_ALIEN )
        update_swarm( );

    /*
        Delayed Teleport rooms                   -Thoric
        Should be the last thing checked in this function
    */
    if ( xIS_SET( ch->in_room->room_flags, ROOM_TELEPORT ) && ch->in_room->tele_delay > 0 )
    {
        TELEPORT_DATA* tele;

        for ( tele = first_teleport; tele; tele = tele->next )
            if ( tele->room == pRoomIndex )
                return;

        CREATE( tele, TELEPORT_DATA, 1 );
        LINK( tele, first_teleport, last_teleport, next, prev );
        tele->room      = pRoomIndex;
        tele->timer     = pRoomIndex->tele_delay;
    }

    return;
}

/*
    Give an obj to a char.
*/
OBJ_DATA* obj_to_char( OBJ_DATA* obj, CHAR_DATA* ch )
{
    OBJ_DATA* otmp;
    OBJ_DATA* oret = obj;
    bool skipgroup, grouped;
    int oweight = get_obj_weight( obj );
    int onum = get_obj_number( obj );
    int wear_loc = obj->wear_loc;
    EXT_BV extra_flags = obj->extra_flags;
    bool was_over = FALSE;
    skipgroup = FALSE;
    grouped = FALSE;

    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
    {
        if ( !IS_IMMORTAL( ch )
                && ( IS_NPC( ch ) && !xIS_SET( ch->act, ACT_PROTOTYPE ) ) )
            return obj_to_room( obj, ch->in_room );
    }

    if ( loading_char == ch )
    {
        int x, y;

        for ( x = 0; x < MAX_WEAR; x++ )
            for ( y = 0; y < MAX_LAYERS; y++ )
                if ( save_equipment[x][y] == obj )
                {
                    skipgroup = TRUE;
                    break;
                }
    }
    else
    {
        was_over = ( ch->carry_weight >= ( can_carry_w( ch ) * 0.75 ) ) ? TRUE : FALSE;
    }

    if ( !skipgroup )
        for ( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
            if ( ( oret = group_object( otmp, obj ) ) == otmp )
            {
                grouped = TRUE;
                break;
            }

    if ( !grouped )
    {
        LINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );
        obj->carried_by         = ch;
        obj->in_room            = NULL;
        obj->in_obj         = NULL;
    }

    if ( wear_loc == WEAR_NONE )
    {
        ch->carry_number    += onum;
        ch->carry_weight    += oweight;
    }
    else
        ch->carry_weight        += oweight;

    if ( obj->item_type == ITEM_FLARE && obj->value[2] != 0 )
    {
        if ( ch->in_room )
        {
            ch->in_room->light++;
        }
        else
        {
            obj->timer = 0;
            obj->value[2] = 0;
        }
    }

    if ( loading_char != ch && !was_over )
    {
        if ( ch->carry_weight >= ( can_carry_w( ch ) * 0.75 ) )
        {
            ch_printf( ch, "&w&RYou are now encumbered. Movement will drain faster.\n\r" );
        }
    }

    return ( oret ? oret : obj );
}



/*
    Take an obj from its character.
*/
void obj_from_char( OBJ_DATA* obj )
{
    CHAR_DATA* ch;
    bool was_over;

    if ( ( ch = obj->carried_by ) == NULL )
    {
        bug( "Obj_from_char: null ch.", 0 );
        return;
    }

    was_over = ( ch->carry_weight >= ( can_carry_w( ch ) * 0.75 ) ) ? TRUE : FALSE;

    if ( obj->wear_loc != WEAR_NONE )
        unequip_char( ch, obj );

    /* obj may drop during unequip... */
    if ( !obj->carried_by )
        return;

    UNLINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );

    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
        empty_obj( obj, NULL, NULL );

    obj->in_room     = NULL;
    obj->carried_by  = NULL;
    ch->carry_number    -= get_obj_number( obj );
    ch->carry_weight    -= get_obj_weight( obj );

    if ( obj->item_type == ITEM_FLARE && obj->value[2] != 0 )
    {
        if ( ch->in_room )
        {
            ch->in_room->light--;
        }
        else
        {
            obj->timer = 0;
            obj->value[2] = 0;
        }
    }

    if ( !char_died( ch ) && was_over )
    {
        if ( ch->carry_weight < ( can_carry_w( ch ) * 0.75 ) )
        {
            ch_printf( ch, "&w&RYou are no longer encumbered. Movement cost is normal.\n\r" );
        }
    }

    return;
}


/*
    Find the ac value of an obj, including position effect.
*/
int apply_ac( OBJ_DATA* obj, int iWear )
{
    if ( obj->item_type != ITEM_ARMOR )
        return 0;

    switch ( iWear )
    {
        case WEAR_BODY:
            return 3 * obj->value[0];

        case WEAR_HEAD:
            return 2 * obj->value[0];

        case WEAR_LEGS:
            return 2 * obj->value[0];

        case WEAR_FEET:
            return     obj->value[0];

        case WEAR_HANDS:
            return     obj->value[0];

        case WEAR_ARMS:
            return     obj->value[0];

        case WEAR_SHIELD:
            return     obj->value[0];

        case WEAR_FINGER_L:
            return     obj->value[0];

        case WEAR_FINGER_R:
            return     obj->value[0];

        case WEAR_NECK_1:
            return     obj->value[0];

        case WEAR_NECK_2:
            return     obj->value[0];

        case WEAR_ABOUT:
            return 2 * obj->value[0];

        case WEAR_OVER:
            return 2 * obj->value[0];

        case WEAR_WAIST:
            return     obj->value[0];

        case WEAR_WRIST_L:
            return     obj->value[0];

        case WEAR_WRIST_R:
            return     obj->value[0];

        case WEAR_HOLD:
            return     obj->value[0];

        case WEAR_EYES:
            return     obj->value[0];
    }

    return 0;
}



/*
    Find a piece of eq on a character.
    Will pick the top layer if clothing is layered.      -Thoric
*/
OBJ_DATA* get_eq_char( CHAR_DATA* ch, int iWear )
{
    OBJ_DATA* obj, *maxobj = NULL;

    if ( ch->first_carrying == NULL )
        return NULL;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
        if ( obj->wear_loc == iWear )
        {
            if ( !obj->pIndexData->layers )
                return obj;
            else if ( !maxobj
                      ||    obj->pIndexData->layers > maxobj->pIndexData->layers )
                maxobj = obj;
        }

    return maxobj;
}



/*
    Equip a char with an obj.
*/
void equip_char( CHAR_DATA* ch, OBJ_DATA* obj, int iWear )
{
    AFFECT_DATA* paf;
    OBJ_DATA*    otmp;

    if ( obj->carried_by != ch )
    {
        bug( "equip_char: obj not being carried by ch!" );
        return;
    }

    if ( ( otmp = get_eq_char( ch, iWear ) ) != NULL
            &&   ( !otmp->pIndexData->layers || !obj->pIndexData->layers ) )
    {
        bug( "Equip_char: already equipped (%d).", iWear );
        return;
    }

    separate_obj( obj ); /* just in case */
    obj->wear_loc    = iWear;
    ch->carry_number    -= get_obj_number( obj );

    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
        affect_modify( ch, paf, TRUE );

    for ( paf = obj->first_affect; paf; paf = paf->next )
        affect_modify( ch, paf, TRUE );

    if ( obj->item_type == ITEM_LIGHT
            &&   IS_SET( obj->value[3], 1 << 0 )
            &&   obj->value[2] != 0
            &&   ch->in_room )
        ++ch->in_room->light;

    if ( ch->in_room )
        ch->in_room->light += check_light_modifier( obj );

    return;
}



/*
    Unequip a char with an obj.
*/
void unequip_char( CHAR_DATA* ch, OBJ_DATA* obj )
{
    AFFECT_DATA* paf;

    if ( obj->wear_loc == WEAR_NONE )
    {
        bug( "Unequip_char: already unequipped.", 0 );
        return;
    }

    ch->carry_number    += get_obj_number( obj );
    obj->wear_loc    = -1;

    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
        affect_modify( ch, paf, FALSE );

    if ( obj->carried_by )
        for ( paf = obj->first_affect; paf; paf = paf->next )
            affect_modify( ch, paf, FALSE );

    if ( !obj->carried_by )
        return;

    if ( obj->item_type == ITEM_LIGHT
            &&   IS_SET( obj->value[3], 1 << 0 )
            &&   obj->value[2] != 0
            &&   ch->in_room
            &&   ch->in_room->light > 0 )
        --ch->in_room->light;

    if ( ch->in_room )
        ch->in_room->light -= check_light_modifier( obj );

    return;
}



/*
    Count occurrences of an obj in a list.
*/
int count_obj_list( OBJ_INDEX_DATA* pObjIndex, OBJ_DATA* list )
{
    OBJ_DATA* obj;
    int nMatch;
    nMatch = 0;

    for ( obj = list; obj; obj = obj->next_content )
        if ( obj->pIndexData == pObjIndex )
            nMatch++;

    return nMatch;
}



/*
    Move an obj out of a room.
*/
void    write_corpses   args( ( CHAR_DATA* ch, char* name ) );

int falling;

void obj_from_room( OBJ_DATA* obj )
{
    ROOM_INDEX_DATA* in_room;

    if ( ( in_room = obj->in_room ) == NULL )
    {
        bug( "obj_from_room: NULL.", 0 );
        return;
    }

    UNLINK( obj, in_room->first_content, in_room->last_content,
            next_content, prev_content );

    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
        empty_obj( obj, NULL, obj->in_room );

    if ( obj->item_type == ITEM_FIRE )
        obj->in_room->light -= obj->count;

    if ( obj->item_type == ITEM_FLARE && obj->value[2] != 0 )
        obj->in_room->light -= obj->count;

    obj->carried_by   = NULL;
    obj->in_obj       = NULL;
    obj->in_room      = NULL;

    if ( ( obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC ) && falling == 0 )
        write_corpses( NULL, obj->short_descr + 14 );

    return;
}


/*
    Move an obj into a room.
*/
OBJ_DATA* obj_to_room( OBJ_DATA* obj, ROOM_INDEX_DATA* pRoomIndex )
{
    OBJ_DATA* otmp, *oret;
    sh_int count = obj->count;
    sh_int item_type = obj->item_type;
    sh_int v2 = obj->value[2];

    if ( pRoomIndex->first_content )
    {
        for ( otmp = pRoomIndex->first_content; otmp; otmp = otmp->next_content )
        {
            if ( otmp == NULL )
                continue;

            if ( ( oret = group_object( otmp, obj ) ) == otmp )
            {
                if ( item_type == ITEM_FIRE )
                    pRoomIndex->light += count;

                if ( item_type == ITEM_FLARE && v2 != 0 )
                    pRoomIndex->light += count;

                return oret;
            }
        }
    }

    LINK( obj, pRoomIndex->first_content, pRoomIndex->last_content,
          next_content, prev_content );
    obj->in_room                = pRoomIndex;
    obj->carried_by             = NULL;
    obj->in_obj                 = NULL;

    if ( item_type == ITEM_FIRE )
        pRoomIndex->light += count;

    if ( item_type == ITEM_FLARE && v2 != 0 )
        pRoomIndex->light += count;

    falling++;
    obj_fall( obj, FALSE );
    falling--;

    if ( ( obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC ) && falling == 0 )
        write_corpses( NULL, obj->short_descr + 14 );

    return obj;
}



/*
    Move an object into an object.
*/
OBJ_DATA* obj_to_obj( OBJ_DATA* obj, OBJ_DATA* obj_to )
{
    OBJ_DATA* otmp, *oret;

    if ( obj == obj_to )
    {
        bug( "Obj_to_obj: trying to put object inside itself: vnum %d", obj->pIndexData->vnum );
        return obj;
    }

    /* Big carry_weight bug fix here by Thoric */
    if ( obj->carried_by != obj_to->carried_by )
    {
        if ( obj->carried_by )
            obj->carried_by->carry_weight -= get_obj_weight( obj );

        if ( obj_to->carried_by )
            obj_to->carried_by->carry_weight += get_obj_weight( obj );
    }

    for ( otmp = obj_to->first_content; otmp; otmp = otmp->next_content )
        if ( ( oret = group_object( otmp, obj ) ) == otmp )
            return oret;

    LINK( obj, obj_to->first_content, obj_to->last_content,
          next_content, prev_content );
    obj->in_obj              = obj_to;
    obj->in_room             = NULL;
    obj->carried_by          = NULL;
    return obj;
}


/*
    Move an object out of an object.
*/
void obj_from_obj( OBJ_DATA* obj )
{
    OBJ_DATA* obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
        bug( "Obj_from_obj: null obj_from.", 0 );
        return;
    }

    UNLINK( obj, obj_from->first_content, obj_from->last_content,
            next_content, prev_content );

    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
        empty_obj( obj, obj->in_obj, NULL );

    obj->in_obj       = NULL;
    obj->in_room      = NULL;
    obj->carried_by   = NULL;

    for ( ; obj_from; obj_from = obj_from->in_obj )
        if ( obj_from->carried_by )
            obj_from->carried_by->carry_weight -= get_obj_weight( obj );

    return;
}



/*
    Extract an obj from the world.
*/
void extract_obj( OBJ_DATA* obj )
{
    OBJ_DATA* obj_content = NULL;

    if ( !obj )
    {
        bug( "extract_obj: !obj" );
        return;
    }

    if ( obj_extracted( obj ) )
    {
        bug( "extract_obj: obj %d already extracted!", obj->pIndexData->vnum );
        return;
    }

    // Disable linked sentry structures
    rem_sentry( obj, NULL );

    if ( obj->item_type == ITEM_COVER )
    {
        CHAR_DATA* ch;

        for ( ch = last_char; ch; ch = ch->prev )
        {
            remove_cover( ch, obj );
        }
    }

    if ( obj->ammo )
    {
        extract_obj( obj->ammo );
        obj->ammo = NULL;
    }

    if ( obj->attach )
    {
        extract_obj( obj->attach );
        obj->attach = NULL;
    }

    if ( obj->carried_by )
        obj_from_char( obj );
    else if ( obj->in_room )
        obj_from_room( obj );
    else if ( obj->in_obj )
        obj_from_obj( obj );

    while ( ( obj_content = obj->last_content ) != NULL )
        extract_obj( obj_content );

    {
        AFFECT_DATA* paf;
        AFFECT_DATA* paf_next;

        for ( paf = obj->first_affect; paf; paf = paf_next )
        {
            paf_next    = paf->next;
            DISPOSE( paf );
        }

        obj->first_affect = obj->last_affect = NULL;
    }
    {
        EXTRA_DESCR_DATA* ed;
        EXTRA_DESCR_DATA* ed_next;

        for ( ed = obj->first_extradesc; ed; ed = ed_next )
        {
            ed_next = ed->next;
            STRFREE( ed->description );
            STRFREE( ed->keyword     );
            DISPOSE( ed );
        }

        obj->first_extradesc = obj->last_extradesc = NULL;
    }

    if ( obj == gobj_prev )
        gobj_prev     = obj->prev;

    UNLINK( obj, first_object, last_object, next, prev );
    /* shove onto extraction queue */
    queue_extracted_obj( obj );
    obj->pIndexData->count -= obj->count;
    numobjsloaded -= obj->count;
    --physicalobjects;

    if ( obj->serial == cur_obj )
    {
        cur_obj_extracted = TRUE;

        if ( global_objcode == rNONE )
            global_objcode = rOBJ_EXTRACTED;
    }

    return;
}



/*
    Extract a char from the world.
*/
void extract_char( CHAR_DATA* ch, bool fPull, bool menu )
{
    CHAR_DATA* wch;
    OBJ_DATA* obj;
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA* location;

    if ( !ch )
    {
        bug( "Extract_char: NULL ch.", 0 );
        return;     /* who removed this line? */
    }

    if ( !ch->in_room && !menu )
    {
        bug( "Extract_char: NULL room.", 0 );
        return;
    }

    if ( ch == supermob )
    {
        bug( "Extract_char: ch == supermob!", 0 );
        return;
    }

    if ( char_died( ch ) )
    {
        sprintf( buf, "extract_char: %s already died!", ch->name );
        bug( buf, 0 );
        return;
    }

    if ( ch == cur_char )
        cur_char_died = TRUE;

    /* shove onto extraction queue */
    queue_extracted_char( ch, fPull );

    if ( gch_prev == ch )
        gch_prev = ch->prev;

    if ( fPull && !xIS_SET( ch->act, ACT_POLYMORPHED ) )
        die_follower( ch );

    if ( ch->mount )
    {
        xREMOVE_BIT( ch->mount->act, ACT_MOUNTED );
        ch->mount = NULL;
        ch->position = POS_STANDING;
    }

    if ( !IS_NPC( ch ) )
        rempc_sentry( ch );

    if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_MOUNTED ) )
        for ( wch = first_char; wch; wch = wch->next )
            if ( wch->mount == ch )
            {
                wch->mount = NULL;
                wch->position = POS_STANDING;
            }

    xREMOVE_BIT( ch->act, ACT_MOUNTED );

    while ( ( obj = ch->last_carrying ) != NULL )
        extract_obj( obj );

    if ( !menu )
        char_from_room( ch );

    if ( !fPull && !menu )
    {
        location = NULL;

        if ( !location )
            location = get_room_index( wherehome( ch ) );

        if ( !location )
            location = get_room_index( 1 );

        char_to_room( ch, location );
        act( AT_MAGIC, "$n appears from some strange swirling mists!", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SITTING;
        return;
    }

    if ( IS_NPC( ch ) )
    {
        --ch->pIndexData->count;
        --nummobsloaded;
    }

    if ( ch->desc && ch->desc->original )
        do_return( ch, "" );

    for ( wch = first_char; wch; wch = wch->next )
        if ( wch->reply == ch )
            wch->reply = NULL;

    if ( !menu )
        UNLINK( ch, first_char, last_char, next, prev );

    if ( ch->desc )
    {
        if ( ch->desc->character != ch )
            bug( "Extract_char: char's descriptor points to another char", 0 );
        else
        {
            ch->desc->character = NULL;
            close_socket( ch->desc, FALSE );
            ch->desc = NULL;
        }
    }

    return;
}

/*
    Find a char in the room.
*/
CHAR_DATA* get_char_far_room( CHAR_DATA* ch, ROOM_INDEX_DATA* room, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* rch;
    int number, count, vnum;
    number = number_argument( argument, arg );

    if ( !str_cmp( arg, "self" ) )
        return ch;

    if ( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    /*
        New Greet code first --
        Look for unknown players by RACE or SEX, THEN go for known players
    */
    count  = 0;

    for ( rch = room->first_person; rch; rch = rch->next_in_room )
    {
        if ( IS_NPC( rch ) )
            continue;

        if ( is_spectator( rch ) )
            continue;

        if ( knows_player( ch, rch ) )
            continue;

        if ( can_see( ch, rch ) && ( nifty_is_name( arg, get_race( rch ) ) || nifty_is_name( arg, get_sex( rch ) ) ) )
        {
            if ( number == 0 )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    count  = 0;

    for ( rch = room->first_person; rch; rch = rch->next_in_room )
    {
        if ( IS_NPC( rch ) )
            continue;

        if ( is_spectator( rch ) )
            continue;

        if ( knows_player( ch, rch ) )
            continue;

        if ( can_see( ch, rch ) && ( nifty_is_name_prefix( arg, get_race( rch ) ) || nifty_is_name_prefix( arg, get_sex( rch ) ) ) )
        {
            if ( number == 0 )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    count  = 0;

    for ( rch = room->first_person; rch; rch = rch->next_in_room )
    {
        if ( is_spectator( rch ) )
            continue;

        if ( can_see( ch, rch ) && ( IS_NPC( rch ) || knows_player( ch, rch ) ) && ( nifty_is_name( arg, rch->name ) || ( IS_NPC( rch ) && vnum == rch->pIndexData->vnum ) ) )
        {
            if ( number == 0 && !IS_NPC( rch ) )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    if ( vnum != -1 )
        return NULL;

    /*  If we didn't find an exact match, run through the list of characters
        again looking for prefix matching, ie gu == guard.
        Added by Narn, Sept/96
    */
    count  = 0;

    for ( rch = room->first_person; rch; rch = rch->next_in_room )
    {
        if ( is_spectator( rch ) )
            continue;

        if ( !can_see( ch, rch ) || !nifty_is_name_prefix( arg, rch->name ) || ( !IS_NPC( rch ) && !knows_player( ch, rch ) ) )
            continue;

        if ( number == 0 && !IS_NPC( rch ) )
            return rch;
        else if ( ++count == number )
            return rch;
    }

    return NULL;
}

/*
    Find a char in the room.
*/
CHAR_DATA* get_char_room( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* rch;
    int number, count, vnum;
    number = number_argument( argument, arg );

    if ( !str_cmp( arg, "self" ) )
        return ch;

    if ( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    /*
        New Greet code first --
        Look for unknown players by RACE or SEX, THEN go for known players
    */
    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( IS_NPC( rch ) )
            continue;

        if ( is_spectator( rch ) )
            continue;

        if ( knows_player( ch, rch ) )
            continue;

        if ( can_see( ch, rch ) && ( nifty_is_name( arg, get_race( rch ) ) ) )
        {
            if ( number == 0 )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( IS_NPC( rch ) )
            continue;

        if ( is_spectator( rch ) )
            continue;

        if ( knows_player( ch, rch ) )
            continue;

        if ( can_see( ch, rch ) && ( nifty_is_name_prefix( arg, get_race( rch ) ) ) )
        {
            if ( number == 0 )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( is_spectator( rch ) )
            continue;

        if ( can_see( ch, rch ) && ( IS_NPC( rch ) || knows_player( ch, rch ) ) && ( nifty_is_name( arg, rch->name ) || ( IS_NPC( rch ) && vnum == rch->pIndexData->vnum ) ) )
        {
            if ( number == 0 && !IS_NPC( rch ) )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    if ( vnum != -1 )
        return NULL;

    /*  If we didn't find an exact match, run through the list of characters
        again looking for prefix matching, ie gu == guard.
        Added by Narn, Sept/96
    */
    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( is_spectator( rch ) )
            continue;

        if ( !can_see( ch, rch ) || !nifty_is_name_prefix( arg, rch->name ) || ( !IS_NPC( rch ) && !knows_player( ch, rch ) ) )
            continue;

        if ( number == 0 && !IS_NPC( rch ) )
            return rch;
        else if ( ++count == number )
            return rch;
    }

    return NULL;
}

/*
    Find a char in the room.
*/
CHAR_DATA* get_char_room_full( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* rch;
    int number, count, vnum;
    number = number_argument( argument, arg );

    if ( !str_cmp( arg, "self" ) )
        return ch;

    if ( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
        if ( can_see( ch, rch )
                &&  !is_spectator( rch )
                &&  ( nifty_is_name( arg, rch->name )
                      ||  ( IS_NPC( rch ) && vnum == rch->pIndexData->vnum ) ) )
        {
            if ( number == 0 && !IS_NPC( rch ) )
                return rch;
            else if ( ++count == number )
                return rch;
        }

    if ( vnum != -1 )
        return NULL;

    /*  If we didn't find an exact match, run through the list of characters
        again looking for prefix matching, ie gu == guard.
        Added by Narn, Sept/96
    */
    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( is_spectator( rch ) )
            continue;

        if ( !can_see( ch, rch ) || !nifty_is_name_prefix( arg, rch->name ) )
            continue;

        if ( number == 0 && !IS_NPC( rch ) )
            return rch;
        else if ( ++count == number )
            return rch;
    }

    return NULL;
}

/*
    Find a char in the world.
*/
CHAR_DATA* get_char_world( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* rch;
    int number, count, vnum;
    number = number_argument( argument, arg );

    if ( !str_cmp( arg, "self" ) )
        return ch;

    if ( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    /*
        New Greet code first --
        Look for unknown players by RACE or SEX, THEN go for known players
    */
    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( IS_NPC( rch ) )
            continue;

        if ( knows_player( ch, rch ) )
            continue;

        if ( can_see( ch, rch ) && ( nifty_is_name( arg, get_race( rch ) ) || nifty_is_name( arg, get_sex( rch ) ) ) )
        {
            if ( number == 0 )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( IS_NPC( rch ) )
            continue;

        if ( knows_player( ch, rch ) )
            continue;

        if ( can_see( ch, rch ) && ( nifty_is_name_prefix( arg, get_race( rch ) ) || nifty_is_name_prefix( arg, get_sex( rch ) ) ) )
        {
            if ( number == 0 )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( can_see( ch, rch ) && ( IS_NPC( rch ) || knows_player( ch, rch ) ) && ( nifty_is_name( arg, rch->name ) || ( IS_NPC( rch ) && vnum == rch->pIndexData->vnum ) ) )
        {
            if ( number == 0 && !IS_NPC( rch ) )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    /*
        Same as above, but examine the entire world
    */
    count  = 0;

    for ( rch = first_char; rch; rch = rch->next )
    {
        if ( IS_NPC( rch ) )
            continue;

        if ( knows_player( ch, rch ) )
            continue;

        if ( can_see( ch, rch ) && ( nifty_is_name( arg, get_race( rch ) ) || nifty_is_name( arg, get_sex( rch ) ) ) )
        {
            if ( number == 0 )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    count  = 0;

    for ( rch = first_char; rch; rch = rch->next )
    {
        if ( IS_NPC( rch ) )
            continue;

        if ( knows_player( ch, rch ) )
            continue;

        if ( can_see( ch, rch ) && ( nifty_is_name_prefix( arg, get_race( rch ) ) || nifty_is_name_prefix( arg, get_sex( rch ) ) ) )
        {
            if ( number == 0 )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    count  = 0;

    for ( rch = first_char; rch; rch = rch->next )
    {
        if ( can_see( ch, rch ) && ( IS_NPC( rch ) || knows_player( ch, rch ) ) && ( nifty_is_name( arg, rch->name ) || ( IS_NPC( rch ) && vnum == rch->pIndexData->vnum ) ) )
        {
            if ( number == 0 && !IS_NPC( rch ) )
                return rch;
            else if ( ++count == number )
                return rch;
        }
    }

    return NULL;
}

/*
    Find a char in the world.
*/
CHAR_DATA* get_char_world_full( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* wch;
    int number, count, vnum;
    number = number_argument( argument, arg );
    count  = 0;

    if ( !str_cmp( arg, "self" ) )
        return ch;

    /*
        Allow reference by vnum for saints+          -Thoric
    */
    if ( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    /* check the room for an exact match */
    for ( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
        if ( ( nifty_is_name( arg, wch->name )
                ||  ( IS_NPC( wch ) && vnum == wch->pIndexData->vnum ) ) && is_wizvis( ch, wch ) )
        {
            if ( number == 0 && !IS_NPC( wch ) )
                return wch;
            else if ( ++count == number )
                return wch;
        }

    count = 0;

    /* check the world for an exact match */
    for ( wch = first_char; wch; wch = wch->next )
        if ( ( nifty_is_name( arg, wch->name )
                ||  ( IS_NPC( wch ) && vnum == wch->pIndexData->vnum ) ) && is_wizvis( ch, wch ) )
        {
            if ( number == 0 && !IS_NPC( wch ) )
                return wch;
            else if ( ++count == number  )
                return wch;
        }

    /* bail out if looking for a vnum match */
    if ( vnum != -1 )
        return NULL;

    /*
        If we didn't find an exact match, check the room for
        for a prefix match, ie gu == guard.
        Added by Narn, Sept/96
    */
    count  = 0;

    for ( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
    {
        if ( !nifty_is_name_prefix( arg, wch->name ) )
            continue;

        if ( number == 0 && !IS_NPC( wch ) && is_wizvis( ch, wch ) )
            return wch;
        else if ( ++count == number  && is_wizvis( ch, wch ) )
            return wch;
    }

    /*
        If we didn't find a prefix match in the room, run through the full list
        of characters looking for prefix matching, ie gu == guard.
        Added by Narn, Sept/96
    */
    count  = 0;

    for ( wch = first_char; wch; wch = wch->next )
    {
        if ( !nifty_is_name_prefix( arg, wch->name ) )
            continue;

        if ( number == 0 && !IS_NPC( wch ) && is_wizvis( ch, wch ) )
            return wch;
        else if ( ++count == number  && is_wizvis( ch, wch ) )
            return wch;
    }

    return NULL;
}

/*
    Find some object with a given index data.
    Used by area-reset 'P', 'T' and 'H' commands.
*/
OBJ_DATA* get_obj_type( OBJ_INDEX_DATA* pObjIndex )
{
    OBJ_DATA* obj;

    for ( obj = last_object; obj; obj = obj->prev )
        if ( obj->pIndexData == pObjIndex )
            return obj;

    return NULL;
}


/*
    Find an obj in a list.
*/
OBJ_DATA* get_obj_list( CHAR_DATA* ch, char* argument, OBJ_DATA* list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    int number;
    int count;
    number = number_argument( argument, arg );
    count  = 0;

    for ( obj = list; obj; obj = obj->next_content )
        if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
            if ( ( count += obj->count ) >= number )
                return obj;

    /*  If we didn't find an exact match, run through the list of objects
        again looking for prefix matching, ie swo == sword.
        Added by Narn, Sept/96
    */
    count = 0;

    for ( obj = list; obj; obj = obj->next_content )
        if ( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
            if ( ( count += obj->count ) >= number )
                return obj;

    return NULL;
}

/*
    Find an obj in a list...going the other way          -Thoric
*/
OBJ_DATA* get_obj_list_rev( CHAR_DATA* ch, char* argument, OBJ_DATA* list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    int number;
    int count;
    number = number_argument( argument, arg );
    count  = 0;

    for ( obj = list; obj; obj = obj->prev_content )
        if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
            if ( ( count += obj->count ) >= number )
                return obj;

    /*  If we didn't find an exact match, run through the list of objects
        again looking for prefix matching, ie swo == sword.
        Added by Narn, Sept/96
    */
    count = 0;

    for ( obj = list; obj; obj = obj->prev_content )
        if ( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
            if ( ( count += obj->count ) >= number )
                return obj;

    return NULL;
}



/*
    Find an obj in player's inventory.
*/
OBJ_DATA* get_obj_carry( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    int number, count, vnum;
    number = number_argument( argument, arg );

    if ( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    count  = 0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->wear_loc == WEAR_NONE
                &&   can_see_obj( ch, obj )
                &&  ( nifty_is_name( arg, obj->name ) || obj->pIndexData->vnum == vnum ) )
            if ( ( count += obj->count ) >= number )
                return obj;

    if ( vnum != -1 )
        return NULL;

    /*  If we didn't find an exact match, run through the list of objects
        again looking for prefix matching, ie swo == sword.
        Added by Narn, Sept/96
    */
    count = 0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->wear_loc == WEAR_NONE
                &&   can_see_obj( ch, obj )
                &&   nifty_is_name_prefix( arg, obj->name ) )
            if ( ( count += obj->count ) >= number )
                return obj;

    return NULL;
}



/*
    Find an obj in player's equipment.
*/
OBJ_DATA* get_obj_wear( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    int number, count, vnum;

    if ( !ch )
    {
        bug( "get_obj_wear: null ch" );
    }

    number = number_argument( argument, arg );

    if ( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    count  = 0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->wear_loc != WEAR_NONE
                &&   can_see_obj( ch, obj )
                &&  ( nifty_is_name( arg, obj->name ) || obj->pIndexData->vnum == vnum ) )
            if ( ++count == number )
                return obj;

    if ( vnum != -1 )
        return NULL;

    /*  If we didn't find an exact match, run through the list of objects
        again looking for prefix matching, ie swo == sword.
        Added by Narn, Sept/96
    */
    count = 0;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->wear_loc != WEAR_NONE
                &&   can_see_obj( ch, obj )
                &&   nifty_is_name_prefix( arg, obj->name ) )
            if ( ++count == number )
                return obj;

    return NULL;
}



/*
    Find an obj in the room or in inventory.
*/
OBJ_DATA* get_obj_here( CHAR_DATA* ch, char* argument )
{
    OBJ_DATA* obj;

    if ( !ch || !ch->in_room )
        return NULL;

    obj = get_obj_list_rev( ch, argument, ch->in_room->last_content );

    if ( obj )
        return obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
        return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
        return obj;

    return NULL;
}



/*
    Find an obj in the world.
*/
OBJ_DATA* get_obj_world( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    int number, count, vnum;

    if ( !ch )
        return NULL;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
        return obj;

    number = number_argument( argument, arg );

    /*
        Allow reference by vnum for saints+          -Thoric
    */
    if ( get_trust( ch ) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    count  = 0;

    for ( obj = first_object; obj; obj = obj->next )
        if ( can_see_obj( ch, obj ) && ( nifty_is_name( arg, obj->name )
                                         ||   vnum == obj->pIndexData->vnum ) )
            if ( ( count += obj->count ) >= number )
                return obj;

    /* bail out if looking for a vnum */
    if ( vnum != -1 )
        return NULL;

    /*  If we didn't find an exact match, run through the list of objects
        again looking for prefix matching, ie swo == sword.
        Added by Narn, Sept/96
    */
    count  = 0;

    for ( obj = first_object; obj; obj = obj->next )
        if ( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
            if ( ( count += obj->count ) >= number )
                return obj;

    return NULL;
}


/*
    How mental state could affect finding an object      -Thoric
    Used by get/drop/put/quaff/recite/etc
    Increasingly freaky based on mental state and drunkeness
*/
bool ms_find_obj( CHAR_DATA* ch )
{
    int ms = ch->mental_state;
    char* t;

    /*
        we're going to be nice and let nothing weird happen unless
        you're a tad messed up
    */
    if ( abs( ms ) < 30 )
        return FALSE;

    if ( ( number_percent() + ( ms < 0 ? 15 : 5 ) ) > abs( ms ) / 2 )
        return FALSE;

    if ( ms > 15 )  /* range 1 to 20 */
        switch ( number_range( UMAX( 1, ( ms / 5 - 15 ) ), ( ms + 4 ) / 5 ) )
        {
            default:
            case  1:
                t = "As you reach for it, you forgot what it was...\n\r";
                break;

            case  2:
                t = "As you reach for it, something inside stops you...\n\r";
                break;

            case  3:
                t = "As you reach for it, it seems to move out of the way...\n\r";
                break;

            case  4:
                t = "You grab frantically for it, but can't seem to get a hold of it...\n\r";
                break;

            case  5:
                t = "It disappears as soon as you touch it!\n\r";
                break;

            case  6:
                t = "You would if it would stay still!\n\r";
                break;

            case  7:
                t = "Whoa!  It's covered in blood!  Ack!  Ick!\n\r";
                break;

            case  8:
                t = "Wow... trails!\n\r";
                break;

            case  9:
                t = "You reach for it, then notice the back of your hand is growing something!\n\r";
                break;

            case 10:
                t = "As you grasp it, it shatters into tiny shards which bite into your flesh!\n\r";
                break;

            case 11:
                t = "What about that huge dragon flying over your head?!?!?\n\r";
                break;

            case 12:
                t = "You stratch yourself instead...\n\r";
                break;

            case 13:
                t = "You hold the universe in the palm of your hand!\n\r";
                break;

            case 14:
                t = "You're too scared.\n\r";
                break;

            case 15:
                t = "Your mother smacks your hand... 'NO!'\n\r";
                break;

            case 16:
                t = "Your hand grasps the worse pile of revoltingness than you could ever imagine!\n\r";
                break;

            case 17:
                t = "You stop reaching for it as it screams out at you in pain!\n\r";
                break;

            case 18:
                t = "What about the millions of burrow-maggots feasting on your arm?!?!\n\r";
                break;

            case 19:
                t = "That doesn't matter anymore... you've found the true answer to everything!\n\r";
                break;

            case 20:
                t = "A supreme entity has no need for that.\n\r";
                break;
        }
    else
    {
        int sub = URANGE( 1, abs( ms ) / 2, 60 );

        switch ( number_range( 1, sub / 10 ) )
        {
            default:
            case  1:
                t = "In just a second...\n\r";
                break;

            case  2:
                t = "You can't find that...\n\r";
                break;

            case  3:
                t = "It's just beyond your grasp...\n\r";
                break;

            case  4:
                t = "...but it's under a pile of other stuff...\n\r";
                break;

            case  5:
                t = "You go to reach for it, but pick your nose instead.\n\r";
                break;

            case  6:
                t = "Which one?!?  I see two... no three...\n\r";
                break;
        }
    }

    send_to_char( t, ch );
    return TRUE;
}


/*
    Generic get obj function that supports optional containers.  -Thoric
    currently only used for "eat" and "quaff".
*/
OBJ_DATA* find_obj( CHAR_DATA* ch, char* argument, bool carryonly )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( !str_cmp( arg2, "from" )
            &&   argument[0] != '\0' )
        argument = one_argument( argument, arg2 );

    if ( arg2[0] == '\0' )
    {
        if ( carryonly && ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return NULL;
        }
        else if ( !carryonly && ( obj = get_obj_here( ch, arg1 ) ) == NULL )
        {
            act( AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
            return NULL;
        }

        return obj;
    }
    else
    {
        OBJ_DATA* container;

        if ( carryonly
                && ( container = get_obj_carry( ch, arg2 ) ) == NULL
                && ( container = get_obj_wear( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return NULL;
        }

        if ( !carryonly && ( container = get_obj_here( ch, arg2 ) ) == NULL )
        {
            act( AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
            return NULL;
        }

        if ( !IS_OBJ_STAT( container, ITEM_COVERING )
                &&    IS_SET( container->value[1], CONT_CLOSED ) )
        {
            act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
            return NULL;
        }

        obj = get_obj_list( ch, arg1, container->first_content );

        if ( !obj )
            act( AT_PLAIN, IS_OBJ_STAT( container, ITEM_COVERING ) ?
                 "I see nothing like that beneath $p." :
                 "I see nothing like that in $p.",
                 ch, container, NULL, TO_CHAR );

        return obj;
    }

    return NULL;
}

int get_obj_number( OBJ_DATA* obj )
{
    return obj->count;
}


/*
    Return weight of an object, including weight of contents.
*/
int get_obj_weight( OBJ_DATA* obj )
{
    int weight;

    if ( obj == NULL )
        return 0;

    weight = obj->count * obj->weight;

    if ( obj->attach )
    {
        weight += get_obj_weight( obj->attach );

        if ( obj->attach->value[0] == 2 )
            weight -= obj->attach->value[5];

        if ( obj->attach->value[0] == 4 )
            weight -= obj->attach->value[4];

        if ( obj->attach->value[0] == 7 )
            weight -= obj->attach->value[3];

        if ( obj->attach->value[0] == 8 )
            weight -= obj->attach->value[3];
    }

    for ( obj = obj->first_content; obj; obj = obj->next_content )
        weight += get_obj_weight( obj );

    return weight;
}



/*
    True if room is dark.
*/
bool room_is_dark( CHAR_DATA* ch, ROOM_INDEX_DATA* pRoomIndex )
{
    if ( !pRoomIndex )
    {
        bug( "room_is_dark: NULL pRoomIndex", 0 );
        return TRUE;
    }

    if ( pRoomIndex->light > 0 )
        return FALSE;

    if ( ch != NULL )
    {
        if ( ch->race == RACE_ALIEN )
            return FALSE;

        if ( ch->pcdata )
        {
            if ( ch->pcdata->learned[gsn_nightvision] > 0 )
                return FALSE;
        }

        if ( using_nvg( ch ) )
            return FALSE;
    }

    if ( xIS_SET( pRoomIndex->room_flags, ROOM_DARK ) )
        return TRUE;

    if ( !xIS_SET( pRoomIndex->room_flags, ROOM_INDOORS )
            && !xIS_SET( pRoomIndex->room_flags, ROOM_SPACECRAFT )
            && !xIS_SET( pRoomIndex->room_flags, ROOM_UNDERGROUND )
            && !xIS_SET( pRoomIndex->room_flags, ROOM_LIT ) )
    {
        /* Night time :-D */
        if ( time_info.hour <  6 || time_info.hour > 19 )
            return TRUE;
    }

    if ( pRoomIndex->sector_type == SECT_INSIDE
            ||   pRoomIndex->sector_type == SECT_CITY )
        return FALSE;

    if ( weather_info.sunlight == SUN_SET
            ||   weather_info.sunlight == SUN_DARK )
        return TRUE;

    return FALSE;
}


/*
    If room is "do not disturb" return the pointer to the imm with dnd flag
    NULL if room is not "do not disturb".
*/
/*  CHAR_DATA *room_is_dnd( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
    {
    CHAR_DATA *rch;

    if ( !pRoomIndex )
    {
        bug( "room_is_dnd: NULL pRoomIndex", 0 );
        return NULL;
    }

    if ( !IS_SET(pRoomIndex->room_flags, ROOM_DND) )
      return NULL;

    for ( rch = pRoomIndex->first_person; rch; rch = rch->next_in_room )
    {
      if ( !IS_NPC(rch) && rch->pcdata && IS_IMMORTAL(rch)
      &&   IS_SET(rch->pcdata->flags, PCFLAG_DND)
      &&   get_trust(ch) < get_trust(rch)
      &&   can_see(ch, rch) )
           return rch;
    }
    return NULL;
    }*/


/*
    Returns TRUE if this room saves equipment  (Coded by Ghost)
*/
bool if_equip_room( ROOM_INDEX_DATA* room )
{
    if ( xIS_SET( room->room_flags, ROOM_PROTOTYPE ) )
        return FALSE;

    if ( xIS_SET( room->room_flags, ROOM_SAVEEQ ) )
        return TRUE;

    return FALSE;
}

/*
    True if char can see victim.
*/
bool can_see( CHAR_DATA* ch, CHAR_DATA* victim )
{
    if ( !victim )
        return FALSE;

    if ( ch != NULL )
    {
        if ( ch->vent != victim->vent && !IS_IMMORTAL( ch ) )
            return FALSE;
    }

    if ( victim->position <= POS_STUNNED )
        return TRUE;

    if ( !ch )
    {
        if ( IS_AFFECTED( victim, AFF_INVISIBLE ) || IS_AFFECTED( victim, AFF_CLOAK ) || IS_AFFECTED( victim, AFF_HIDE ) || xIS_SET( victim->act, PLR_WIZINVIS ) )
            return FALSE;
        else
            return TRUE;
    }

    if ( ch == victim )
        return TRUE;

    if ( is_spectator( victim ) && !IS_IMMORTAL( ch ) )
        return FALSE;

    if ( !IS_NPC( victim )
            &&   xIS_SET( victim->act, PLR_WIZINVIS )
            &&   get_trust( ch ) < victim->pcdata->wizinvis )
        return FALSE;

    /* SB */
    if ( IS_NPC( victim )
            &&   xIS_SET( victim->act, ACT_MOBINVIS )
            &&   get_trust( ch ) < victim->mobinvis )
        return FALSE;

    if ( !IS_IMMORTAL( ch ) && !IS_NPC( victim ) && !victim->desc
            &&    get_timer( victim, TIMER_RECENTFIGHT ) > 0
            &&  ( !victim->switched || !IS_AFFECTED( victim->switched, AFF_POSSESS ) ) )
        return FALSE;

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
        return TRUE;

    /* The miracle cure for blindness? -- Altrag */
    if ( !IS_AFFECTED( ch, AFF_TRUESIGHT ) )
    {
        if ( IS_AFFECTED( ch, AFF_BLIND ) )
            return FALSE;

        if ( IS_AFFECTED( ch, AFF_NAPALM ) )
            return FALSE;

        if ( ch->race == RACE_PREDATOR && ch->vision >= 0 )
        {
            if ( ch->vision == victim->race )
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        if ( ch->race != RACE_ALIEN )
        {
            if ( room_is_dark( ch, ch->in_room ) && get_dark_range( ch ) < 1 && !IS_AFFECTED( ch, AFF_INFRARED ) )
                return FALSE;
        }

        if ( !IS_AFFECTED( victim, AFF_NAPALM ) )
        {
            if ( ( IS_AFFECTED( victim, AFF_CLOAK ) && !IS_AFFECTED( victim, AFF_SHORTAGE ) ) && ( ch->race != RACE_ALIEN ) )
            {
                if ( ch->in_room == victim->in_room )
                    return FALSE;

                if ( !has_thermal( ch ) )
                    return FALSE;
            }

            if ( IS_AFFECTED( victim, AFF_HIDE ) )
            {
                bool hide = TRUE;

                if ( IS_AFFECTED( ch, AFF_DETECT_HIDDEN ) )
                    hide = FALSE;

                if ( !is_enemy( ch, victim ) )
                    hide = FALSE;

                if ( ch->race == RACE_ALIEN )
                    hide = FALSE;

                if ( victim->race == RACE_PREDATOR && ch->in_room != victim->in_room && has_thermal( ch ) )
                    hide = FALSE;

                if ( hide == TRUE )
                    return FALSE;
            }

            if ( IS_AFFECTED( victim, AFF_INVISIBLE ) && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
                return FALSE;
        }
    }

    return TRUE;
}



/*
    True if char can see obj.
*/
bool can_see_obj( CHAR_DATA* ch, OBJ_DATA* obj )
{
    if ( ch == NULL || obj == NULL )
        return FALSE;

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
        return TRUE;

    if ( IN_VENT( ch ) && !IS_IMMORTAL( ch ) )
        return FALSE;

    if ( IS_OBJ_STAT( obj, ITEM_BURRIED ) )
        return FALSE;

    if ( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
        return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
        return FALSE;

    if ( IS_AFFECTED( ch, AFF_NAPALM ) )
        return FALSE;

    /*
        if ( IS_OBJ_STAT(obj, ITEM_HIDDEN) )
        {
        if ( number_percent() > ch->pcdata->learned[gsn_sharp_eye] - 60 )
          return FALSE;
        learn_from_success( ch, gsn_sharp_eye );
        }
    */

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && IS_SET( obj->value[3], 1 << 0 ) )
        return TRUE;

    if ( ch->in_room && ch->race != RACE_ALIEN )
    {
        if ( room_is_dark( ch, ch->in_room ) && !IS_AFFECTED( ch, AFF_INFRARED ) )
        {
            if ( number_range( 1, 6 ) != 1 )
                return FALSE;
        }
    }

    if ( IS_OBJ_STAT( obj, ITEM_ALIENINVIS ) && ch->race != RACE_ALIEN )
        return FALSE;

    if ( IS_OBJ_STAT( obj, ITEM_INVIS ) && !IS_AFFECTED( ch, AFF_DETECT_INVIS ) )
        return FALSE;

    return TRUE;
}



/*
    True if char can drop obj.
*/
bool can_drop_obj( CHAR_DATA* ch, OBJ_DATA* obj )
{
    /* Seems logical */
    if ( !can_see_obj( ch, obj ) )
        return FALSE;

    if ( !IS_OBJ_STAT( obj, ITEM_NODROP ) )
        return TRUE;

    if ( !IS_NPC( ch ) && get_trust( ch ) >= LEVEL_IMMORTAL )
        return TRUE;

    if ( IS_NPC( ch ) && ch->pIndexData->vnum == 3 )
        return TRUE;

    return FALSE;
}


/*
    Return ascii name of an item type.
*/
char* item_type_name( OBJ_DATA* obj )
{
    if ( obj->item_type < 1 || obj->item_type > MAX_ITEM_TYPE )
    {
        bug( "Item_type_name: unknown type %d.", obj->item_type );
        return "(unknown)";
    }

    return o_types[obj->item_type];
}



/*
    Return ascii name of an affect location.
*/
char* affect_loc_name( int location )
{
    switch ( location )
    {
        case APPLY_NONE:
            return "none";

        case APPLY_STR:
            return "strength";

        case APPLY_STA:
            return "stamina";

        case APPLY_REC:
            return "recovery";

        case APPLY_INT:
            return "intelligence";

        case APPLY_BRA:
            return "bravery";

        case APPLY_PER:
            return "perception";

        case APPLY_HIT:
            return "hp";

        case APPLY_MOVE:
            return "moves";

        case APPLY_AFFECT:
            return "affected_by";

        case APPLY_FIRE:
            return "fire";

        case APPLY_ENERGY:
            return "energy";

        case APPLY_IMPACT:
            return "impact";

        case APPLY_PIERCE:
            return "pierce";

        case APPLY_ACID:
            return "acid";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}



/*
    Return ascii name of an affect bit vector.
*/
char* affect_bit_name( EXT_BV* vector )
{
    static char buf[512];
    buf[0] = '\0';

    if ( xIS_SET( *vector, AFF_BLIND        ) )
        strcat( buf, " blind"         );

    if ( xIS_SET( *vector, AFF_INVISIBLE    ) )
        strcat( buf, " invisible"     );

    if ( xIS_SET( *vector, AFF_DETECT_EVIL  ) )
        strcat( buf, " detect_evil"   );

    if ( xIS_SET( *vector, AFF_DETECT_INVIS ) )
        strcat( buf, " detect_invis"  );

    if ( xIS_SET( *vector, AFF_DETECT_MAGIC ) )
        strcat( buf, " detect_magic"  );

    if ( xIS_SET( *vector, AFF_DETECT_HIDDEN ) )
        strcat( buf, " detect_hidden" );

    if ( xIS_SET( *vector, AFF_SANCTUARY    ) )
        strcat( buf, " sanctuary"     );

    if ( xIS_SET( *vector, AFF_FAERIE_FIRE  ) )
        strcat( buf, " faerie_fire"   );

    if ( xIS_SET( *vector, AFF_INFRARED     ) )
        strcat( buf, " infrared"      );

    if ( xIS_SET( *vector, AFF_CURSE        ) )
        strcat( buf, " curse"         );

    if ( xIS_SET( *vector, AFF_FLAMING      ) )
        strcat( buf, " flaming"       );

    if ( xIS_SET( *vector, AFF_POISON       ) )
        strcat( buf, " poison"        );

    if ( xIS_SET( *vector, AFF_NAPALM       ) )
        strcat( buf, " napalm"        );

    if ( xIS_SET( *vector, AFF_PROTECT      ) )
        strcat( buf, " protect"       );

    if ( xIS_SET( *vector, AFF_PARALYSIS    ) )
        strcat( buf, " paralysis"     );

    if ( xIS_SET( *vector, AFF_SLEEP        ) )
        strcat( buf, " sleep"         );

    if ( xIS_SET( *vector, AFF_SNEAK        ) )
        strcat( buf, " sneak"         );

    if ( xIS_SET( *vector, AFF_HIDE         ) )
        strcat( buf, " hide"          );

    if ( xIS_SET( *vector, AFF_CHARM        ) )
        strcat( buf, " charm"         );

    if ( xIS_SET( *vector, AFF_POSSESS      ) )
        strcat( buf, " possess"       );

    if ( xIS_SET( *vector, AFF_FLYING       ) )
        strcat( buf, " flying"        );

    if ( xIS_SET( *vector, AFF_PASS_DOOR    ) )
        strcat( buf, " pass_door"     );

    if ( xIS_SET( *vector, AFF_FLOATING     ) )
        strcat( buf, " floating"      );

    if ( xIS_SET( *vector, AFF_TRUESIGHT    ) )
        strcat( buf, " true_sight"    );

    if ( xIS_SET( *vector, AFF_DETECTTRAPS  ) )
        strcat( buf, " detect_traps"  );

    if ( xIS_SET( *vector, AFF_SCRYING      ) )
        strcat( buf, " scrying"       );

    if ( xIS_SET( *vector, AFF_FIRESHIELD   ) )
        strcat( buf, " fireshield"    );

    if ( xIS_SET( *vector, AFF_SHOCKSHIELD  ) )
        strcat( buf, " shockshield"   );

    if ( xIS_SET( *vector, AFF_SHORTAGE     ) )
        strcat( buf, " shortage"      );

    if ( xIS_SET( *vector, AFF_CLOAK        ) )
        strcat( buf, " cloak"         );

    if ( xIS_SET( *vector, AFF_AQUA_BREATH  ) )
        strcat( buf, " aqua_breath"   );

    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}



/*
    Return ascii name of extra flags vector.
*/
char* extra_bit_name( EXT_BV* extra_flags )
{
    static char buf[512];
    buf[0] = '\0';

    if ( xIS_SET( *extra_flags, ITEM_GLOW )              )
        strcat( buf, " glow"         );

    if ( xIS_SET( *extra_flags, ITEM_HUM )               )
        strcat( buf, " hum"         );

    if ( xIS_SET( *extra_flags, ITEM_INVIS )             )
        strcat( buf, " invis"        );

    if ( xIS_SET( *extra_flags, ITEM_NODROP )            )
        strcat( buf, " nodrop"       );

    if ( xIS_SET( *extra_flags, ITEM_NOREMOVE )          )
        strcat( buf, " noremove"     );

    if ( xIS_SET( *extra_flags, ITEM_INVENTORY )         )
        strcat( buf, " inventory"    );

    if ( xIS_SET( *extra_flags, ITEM_HIDDEN )            )
        strcat( buf, " hidden"    );

    if ( xIS_SET( *extra_flags, ITEM_COVERING )          )
        strcat( buf, " covering"    );

    if ( xIS_SET( *extra_flags, ITEM_DEATHROT )          )
        strcat( buf, " deathrot"     );

    if ( xIS_SET( *extra_flags, ITEM_BURRIED )           )
        strcat( buf, " burried"    );

    if ( xIS_SET( *extra_flags, ITEM_PROTOTYPE )         )
        strcat( buf, " prototype"    );

    if ( xIS_SET( *extra_flags, ITEM_NODUAL )            )
        strcat( buf, " nodual"    );

    if ( xIS_SET( *extra_flags, ITEM_MARINE )            )
        strcat( buf, " marine"    );

    if ( xIS_SET( *extra_flags, ITEM_PREDATOR )          )
        strcat( buf, " predator"    );

    if ( xIS_SET( *extra_flags, ITEM_ALIEN )             )
        strcat( buf, " alien"    );

    return ( buf[0] != '\0' ) ? buf + 1 : "none";
}

/*
    Remove an exit from a room                   -Thoric
*/
void extract_exit( ROOM_INDEX_DATA* room, EXIT_DATA* pexit )
{
    UNLINK( pexit, room->first_exit, room->last_exit, next, prev );

    if ( pexit->rexit )
        pexit->rexit->rexit = NULL;

    STRFREE( pexit->keyword );
    STRFREE( pexit->description );
    DISPOSE( pexit );
}

/*
    Remove a room
*/
void extract_room( ROOM_INDEX_DATA* room )
{
    bug( "extract_room: not implemented", 0 );
    /*
        (remove room from hash table)
        clean_room( room )
        DISPOSE( room );
    */
    return;
}

/*
    clean out a room (leave list pointers intact )       -Thoric
*/
void clean_room( ROOM_INDEX_DATA* room )
{
    EXTRA_DESCR_DATA* ed, *ed_next;
    EXIT_DATA*        pexit, *pexit_next;
    STRFREE( room->description );
    STRFREE( room->name );

    for ( ed = room->first_extradesc; ed; ed = ed_next )
    {
        ed_next = ed->next;
        STRFREE( ed->description );
        STRFREE( ed->keyword );
        DISPOSE( ed );
        top_ed--;
    }

    room->first_extradesc    = NULL;
    room->last_extradesc     = NULL;

    for ( pexit = room->first_exit; pexit; pexit = pexit_next )
    {
        pexit_next = pexit->next;
        STRFREE( pexit->keyword );
        STRFREE( pexit->description );
        DISPOSE( pexit );
        top_exit--;
    }

    room->first_exit = NULL;
    room->last_exit = NULL;
    xCLEAR_BITS( room->room_flags );
    room->sector_type = 0;
    room->light = 0;
}

/*
    clean out an object (index) (leave list pointers intact )    -Thoric
*/
void clean_obj( OBJ_INDEX_DATA* obj )
{
    AFFECT_DATA* paf;
    AFFECT_DATA* paf_next;
    EXTRA_DESCR_DATA* ed;
    EXTRA_DESCR_DATA* ed_next;
    STRFREE( obj->name );
    STRFREE( obj->short_descr );
    STRFREE( obj->description );
    STRFREE( obj->action_desc );
    obj->item_type      = 0;
    xCLEAR_BITS( obj->extra_flags );
    xCLEAR_BITS( obj->wear_flags );
    obj->count      = 0;
    obj->weight     = 0;
    obj->cost       = 0;
    obj->value[0]       = 0;
    obj->value[1]       = 0;
    obj->value[2]       = 0;
    obj->value[3]       = 0;
    obj->value[4]           = 0;
    obj->value[5]           = 0;

    for ( paf = obj->first_affect; paf; paf = paf_next )
    {
        paf_next    = paf->next;
        DISPOSE( paf );
        top_affect--;
    }

    obj->first_affect   = NULL;
    obj->last_affect    = NULL;

    for ( ed = obj->first_extradesc; ed; ed = ed_next )
    {
        ed_next     = ed->next;
        STRFREE( ed->description );
        STRFREE( ed->keyword     );
        DISPOSE( ed );
        top_ed--;
    }

    obj->first_extradesc    = NULL;
    obj->last_extradesc = NULL;
}

/*
    clean out a mobile (index) (leave list pointers intact ) -Thoric
*/
void clean_mob( MOB_INDEX_DATA* mob )
{
    MPROG_DATA* mprog, *mprog_next;
    STRFREE( mob->player_name );
    STRFREE( mob->short_descr );
    STRFREE( mob->long_descr  );
    STRFREE( mob->description );
    mob->spec_fun   = NULL;
    mob->spec_2 = NULL;
    mob->pShop  = NULL;
    xCLEAR_BITS( mob->progtypes );

    for ( mprog = mob->mudprogs; mprog; mprog = mprog_next )
    {
        mprog_next = mprog->next;
        STRFREE( mprog->arglist );
        STRFREE( mprog->comlist );
        DISPOSE( mprog );
    }

    mob->count   = 0;
    mob->killed      = 0;
    mob->sex     = 0;
    mob->level           = 0;
    xCLEAR_BITS( mob->act );
    xCLEAR_BITS( mob->affected_by );
    mob->position    = 0;
    mob->defposition = 0;
    mob->height      = 0;
    mob->weight  = 0;
}

extern int top_reset;

/*
    Remove all resets from an area               -Thoric
*/
void clean_resets( AREA_DATA* tarea )
{
    RESET_DATA* pReset, *pReset_next;

    for ( pReset = tarea->first_reset; pReset; pReset = pReset_next )
    {
        pReset_next = pReset->next;
        DISPOSE( pReset );
        --top_reset;
    }

    tarea->first_reset  = NULL;
    tarea->last_reset   = NULL;
}

/* no more silliness */
void name_stamp_stats( CHAR_DATA* ch )
{
}

/*
    "Fix" a character's stats                    -Thoric
*/
void fix_char( CHAR_DATA* ch )
{
    AFFECT_DATA* aff;
    OBJ_DATA* carry[MAX_LEVEL * 200];
    OBJ_DATA* obj;
    int x, ncarry;
    de_equip_char( ch );
    ncarry = 0;

    while ( ( obj = ch->first_carrying ) != NULL )
    {
        carry[ncarry++]  = obj;
        obj_from_char( obj );
    }

    for ( aff = ch->first_affect; aff; aff = aff->next )
        affect_modify( ch, aff, FALSE );

    xCLEAR_BITS( ch->affected_by );
    ch->mental_state    = 0;
    ch->hit             = UMAX( 1, ch->hit   );
    ch->move            = UMAX( 1, ch->move  );
    ch->mod_str     = 0;
    ch->mod_sta         = 0;
    ch->mod_rec         = 0;
    ch->mod_int         = 0;
    ch->mod_bra         = 0;
    ch->mod_per         = 0;
    ch->carry_weight    = 0;
    ch->carry_number    = 0;

    for ( aff = ch->first_affect; aff; aff = aff->next )
        affect_modify( ch, aff, TRUE );

    for ( x = 0; x < ncarry; x++ )
        obj_to_char( carry[x], ch );

    re_equip_char( ch );
}


/*
    Show an affect verbosely to a character          -Thoric
*/
void showaffect( CHAR_DATA* ch, AFFECT_DATA* paf )
{
    char buf[MAX_STRING_LENGTH];
    int x;

    if ( !paf )
    {
        bug( "showaffect: NULL paf", 0 );
        return;
    }

    if ( paf->location != APPLY_NONE && paf->modifier != 0 )
    {
        switch ( paf->location )
        {
            default:
                sprintf( buf, "Affects %s by %d.\n\r",
                         affect_loc_name( paf->location ), paf->modifier );
                break;

            case APPLY_AFFECT:
                sprintf( buf, "Affects %s by",
                         affect_loc_name( paf->location ) );

                for ( x = 0; x < 32 ; x++ )
                    if ( IS_SET( paf->modifier, 1 << x ) )
                    {
                        strcat( buf, " " );
                        strcat( buf, a_flags[x] );
                    }

                strcat( buf, "\n\r" );
                break;
        }

        send_to_char( buf, ch );
    }
}

/*
    Set the current global object to obj             -Thoric
*/
void set_cur_obj( OBJ_DATA* obj )
{
    cur_obj = obj->serial;
    cur_obj_extracted = FALSE;
    global_objcode = rNONE;
}

/*
    Check the recently extracted object queue for obj        -Thoric
*/
bool obj_extracted( OBJ_DATA* obj )
{
    OBJ_DATA* cod;

    if ( !obj )
        return TRUE;

    if ( obj->serial == cur_obj
            &&   cur_obj_extracted )
        return TRUE;

    for ( cod = extracted_obj_queue; cod; cod = cod->next )
        if ( obj == cod )
            return TRUE;

    return FALSE;
}

/*
    Stick obj onto extraction queue
*/
void queue_extracted_obj( OBJ_DATA* obj )
{
    ++cur_qobjs;
    obj->next = extracted_obj_queue;
    extracted_obj_queue = obj;
}

/*
    Clean out the extracted object queue
*/
void clean_obj_queue()
{
    OBJ_DATA* obj;

    while ( extracted_obj_queue )
    {
        obj = extracted_obj_queue;
        extracted_obj_queue = extracted_obj_queue->next;
        STRFREE( obj->name        );
        STRFREE( obj->description );
        STRFREE( obj->short_descr );
        DISPOSE( obj );
        --cur_qobjs;
    }
}

/*
    Set the current global character to ch           -Thoric
*/
void set_cur_char( CHAR_DATA* ch )
{
    cur_char       = ch;
    cur_char_died  = FALSE;
    cur_room       = ch->in_room;
    global_retcode = rNONE;
}

/*
    Check to see if ch died recently             -Thoric
*/
bool char_died( CHAR_DATA* ch )
{
    EXTRACT_CHAR_DATA* ccd;

    if ( ch == cur_char && cur_char_died )
        return TRUE;

    for ( ccd = extracted_char_queue; ccd; ccd = ccd->next )
        if ( ccd->ch == ch )
            return TRUE;

    /* Hack job */
    if ( is_spectator( ch ) )
        return TRUE;

    return FALSE;
}

/*
    Add ch to the queue of recently extracted characters     -Thoric
*/
void queue_extracted_char( CHAR_DATA* ch, bool extract )
{
    EXTRACT_CHAR_DATA* ccd;

    if ( !ch )
    {
        bug( "queue_extracted char: ch = NULL", 0 );
        return;
    }

    CREATE( ccd, EXTRACT_CHAR_DATA, 1 );
    ccd->ch         = ch;
    ccd->room           = ch->in_room;
    ccd->extract        = extract;

    if ( ch == cur_char )
        ccd->retcode      = global_retcode;
    else
        ccd->retcode      = rCHAR_DIED;

    ccd->next           = extracted_char_queue;
    extracted_char_queue    = ccd;
    cur_qchars++;
}

/*
    clean out the extracted character queue
*/
void clean_char_queue()
{
    EXTRACT_CHAR_DATA* ccd = NULL;

    for ( ccd = extracted_char_queue; ccd; ccd = extracted_char_queue )
    {
        extracted_char_queue = ccd->next;

        if ( ccd->extract )
            free_char( ccd->ch );

        DISPOSE( ccd );
        --cur_qchars;
    }
}

/*
    Add a timer to ch                        -Thoric
    Support for "call back" time delayed commands
*/
void add_timer( CHAR_DATA* ch, sh_int type, sh_int count, DO_FUN* fun, int value )
{
    TIMER* timer;

    for ( timer = ch->first_timer; timer; timer = timer->next )
        if ( timer->type == type )
        {
            timer->count  = count;
            timer->do_fun = fun;
            timer->value  = value;
            break;
        }

    if ( !timer )
    {
        CREATE( timer, TIMER, 1 );
        timer->count    = count;
        timer->type = type;
        timer->do_fun   = fun;
        timer->value    = value;
        LINK( timer, ch->first_timer, ch->last_timer, next, prev );
    }
}

TIMER* get_timerptr( CHAR_DATA* ch, sh_int type )
{
    TIMER* timer;

    for ( timer = ch->first_timer; timer; timer = timer->next )
        if ( timer->type == type )
            return timer;

    return NULL;
}

sh_int get_timer( CHAR_DATA* ch, sh_int type )
{
    TIMER* timer;

    if ( ( timer = get_timerptr( ch, type ) ) != NULL )
        return timer->count;
    else
        return 0;
}

void extract_timer( CHAR_DATA* ch, TIMER* timer )
{
    if ( !timer )
    {
        bug( "extract_timer: NULL timer", 0 );
        return;
    }

    UNLINK( timer, ch->first_timer, ch->last_timer, next, prev );
    DISPOSE( timer );
    return;
}

void remove_timer( CHAR_DATA* ch, sh_int type )
{
    TIMER* timer;

    for ( timer = ch->first_timer; timer; timer = timer->next )
        if ( timer->type == type )
            break;

    if ( timer )
        extract_timer( ch, timer );
}

bool in_soft_range( CHAR_DATA* ch, AREA_DATA* tarea )
{
    return TRUE;

    if ( IS_IMMORTAL( ch ) )
        return TRUE;
    else if ( IS_NPC( ch ) )
        return TRUE;
    else if ( ch->top_level >= tarea->low_soft_range || ch->top_level <= tarea->hi_soft_range )
        return TRUE;
    else
        return FALSE;
}

bool in_hard_range( CHAR_DATA* ch, AREA_DATA* tarea )
{
    return TRUE;

    if ( IS_IMMORTAL( ch ) )
        return TRUE;
    else if ( IS_NPC( ch ) )
        return TRUE;
    else if ( ch->top_level >= tarea->low_hard_range && ch->top_level <= tarea->hi_hard_range )
        return TRUE;
    else
        return FALSE;
}


/*
    Scryn, standard luck check 2/2/96
*/
bool chance( CHAR_DATA* ch, sh_int percent )
{
    sh_int ms;

    if ( !ch )
    {
        bug( "Chance: null ch!", 0 );
        return FALSE;
    }

    ms = 10 - abs( ch->mental_state );

    if ( ( number_percent() - get_curr_bra( ch ) + 12 - ms ) <= percent )
        return TRUE;

    return FALSE;
}

bool chance_attrib( CHAR_DATA* ch, sh_int percent, sh_int attrib )
{
    if ( !ch )
    {
        bug( "Chance: null ch!", 0 );
        return FALSE;
    }

    if ( number_percent() - get_curr_bra( ch ) + 13 - attrib + 12 <= percent )
        return TRUE;

    return FALSE;
}


/*
    Make a simple clone of an object (no extras...yet)       -Thoric
*/
OBJ_DATA* clone_object( OBJ_DATA* obj )
{
    OBJ_DATA* clone;
    CREATE( clone, OBJ_DATA, 1 );
    clone->pIndexData   = obj->pIndexData;
    clone->name     = QUICKLINK( obj->name );
    clone->short_descr  = QUICKLINK( obj->short_descr );
    clone->description  = QUICKLINK( obj->description );
    clone->action_desc  = QUICKLINK( obj->action_desc );
    clone->item_type    = obj->item_type;
    clone->extra_flags  = obj->extra_flags;
    clone->wear_flags   = obj->wear_flags;
    clone->wear_loc = obj->wear_loc;
    clone->weight   = obj->weight;
    clone->cost     = obj->cost;
    clone->level    = obj->level;
    clone->timer    = obj->timer;
    clone->value[0] = obj->value[0];
    clone->value[1] = obj->value[1];
    clone->value[2] = obj->value[2];
    clone->value[3] = obj->value[3];
    clone->value[4] = obj->value[4];
    clone->value[5] = obj->value[5];
    clone->count    = 1;
    ++obj->pIndexData->count;
    ++numobjsloaded;
    ++physicalobjects;
    cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
    clone->serial = clone->pIndexData->serial = cur_obj_serial;
    LINK( clone, first_object, last_object, next, prev );
    return clone;
}

/*
    If possible group obj2 into obj1             -Thoric
    This code, along with clone_object, obj->count, and special support
    for it implemented throughout handler.c and save.c should show improved
    performance on MUDs with players that hoard tons of potions and scrolls
    as this will allow them to be grouped together both in memory, and in
    the player files.
*/
OBJ_DATA* group_object( OBJ_DATA* obj1, OBJ_DATA* obj2 )
{
    if ( !obj1 || !obj2 )
        return NULL;

    if ( obj1 == obj2 )
        return obj1;

    /* Explicit Ignores */
    if ( obj2->item_type == ITEM_TRAP )
        return obj2;

    if ( obj2->item_type == ITEM_MSPAWNER )
        return obj2;

    if ( obj2->item_type == ITEM_SENTRY )
        return obj2;

    if ( obj1->pIndexData == obj2->pIndexData
            /*
                &&  !obj1->pIndexData->mudprogs
                &&  !obj2->pIndexData->mudprogs
            */
            &&   QUICKMATCH( obj1->name,    obj2->name )
            &&   QUICKMATCH( obj1->short_descr, obj2->short_descr )
            &&   QUICKMATCH( obj1->description, obj2->description )
            &&   QUICKMATCH( obj1->action_desc, obj2->action_desc )
            &&   obj1->item_type    == obj2->item_type
            &&   xSAME_BITS( obj1->extra_flags, obj2->extra_flags )
            &&   xSAME_BITS( obj1->wear_flags, obj2->wear_flags )
            &&   obj1->item_type        != ITEM_COVER
            &&   obj1->wear_loc     == obj2->wear_loc
            &&   obj1->weight       == obj2->weight
            &&   obj1->ammo             == obj2->ammo
            &&   obj1->cost     == obj2->cost
            &&   obj1->level        == obj2->level
            &&   obj1->timer        == obj2->timer
            &&   obj1->value[0]     == obj2->value[0]
            &&   obj1->value[1]     == obj2->value[1]
            &&   obj1->value[2]     == obj2->value[2]
            &&   obj1->value[3]     == obj2->value[3]
            &&   obj1->value[4]     == obj2->value[4]
            &&   obj1->value[5]     == obj2->value[5]
            &&  !obj1->first_extradesc  && !obj2->first_extradesc
            &&  !obj1->first_affect && !obj2->first_affect
            &&  !obj1->first_content    && !obj2->first_content )
    {
        obj1->count += obj2->count;
        obj1->pIndexData->count += obj2->count; /* to be decremented in */
        numobjsloaded += obj2->count;       /* extract_obj */
        extract_obj( obj2 );
        return obj1;
    }

    return obj2;
}

/*
    Split off a grouped object                   -Thoric
    decreased obj's count to num, and creates a new object containing the rest
*/
void split_obj( OBJ_DATA* obj, int num )
{
    int count;
    OBJ_DATA* rest;

    if ( !obj )
        return;

    count = obj->count;

    if ( count <= num || num == 0 )
        return;

    rest = clone_object( obj );
    --obj->pIndexData->count;   /* since clone_object() ups this value */
    --numobjsloaded;
    rest->count = obj->count - num;
    obj->count = num;

    if ( obj->carried_by )
    {
        LINK( rest, obj->carried_by->first_carrying,
              obj->carried_by->last_carrying,
              next_content, prev_content );
        rest->carried_by        = obj->carried_by;
        rest->in_room           = NULL;
        rest->in_obj            = NULL;
    }
    else if ( obj->in_room )
    {
        LINK( rest, obj->in_room->first_content, obj->in_room->last_content,
              next_content, prev_content );
        rest->carried_by        = NULL;
        rest->in_room           = obj->in_room;
        rest->in_obj            = NULL;
    }
    else if ( obj->in_obj )
    {
        LINK( rest, obj->in_obj->first_content, obj->in_obj->last_content,
              next_content, prev_content );
        rest->in_obj             = obj->in_obj;
        rest->in_room            = NULL;
        rest->carried_by         = NULL;
    }
}

void separate_obj( OBJ_DATA* obj )
{
    split_obj( obj, 1 );
}

/*
    Empty an obj's contents... optionally into another obj, or a room
*/
bool empty_obj( OBJ_DATA* obj, OBJ_DATA* destobj, ROOM_INDEX_DATA* destroom )
{
    OBJ_DATA* otmp, *otmp_next;
    CHAR_DATA* ch = obj->carried_by;
    bool movedsome = FALSE;

    if ( !obj )
    {
        bug( "empty_obj: NULL obj", 0 );
        return FALSE;
    }

    if ( destobj || ( !destroom && !ch && ( destobj = obj->in_obj ) != NULL ) )
    {
        for ( otmp = obj->first_content; otmp; otmp = otmp_next )
        {
            otmp_next = otmp->next_content;

            if ( destobj->item_type == ITEM_CONTAINER
                    &&   get_obj_weight( otmp ) + get_obj_weight( destobj )
                    > destobj->value[0] )
                continue;

            obj_from_obj( otmp );
            obj_to_obj( otmp, destobj );
            movedsome = TRUE;
        }

        return movedsome;
    }

    if ( destroom || ( !ch && ( destroom = obj->in_room ) != NULL ) )
    {
        for ( otmp = obj->first_content; otmp; otmp = otmp_next )
        {
            otmp_next = otmp->next_content;

            if ( ch && xIS_SET( otmp->pIndexData->progtypes, DROP_PROG ) && otmp->count > 1 )
            {
                separate_obj( otmp );
                obj_from_obj( otmp );

                if ( !otmp_next )
                    otmp_next = obj->first_content;
            }
            else
                obj_from_obj( otmp );

            otmp = obj_to_room( otmp, destroom );

            if ( ch )
            {
                oprog_drop_trigger( ch, otmp );     /* mudprogs */

                if ( char_died( ch ) )
                    ch = NULL;
            }

            movedsome = TRUE;
        }

        return movedsome;
    }

    if ( ch )
    {
        for ( otmp = obj->first_content; otmp; otmp = otmp_next )
        {
            otmp_next = otmp->next_content;
            obj_from_obj( otmp );
            obj_to_char( otmp, ch );
            movedsome = TRUE;
        }

        return movedsome;
    }

    bug( "empty_obj: could not determine a destination for vnum %d",
         obj->pIndexData->vnum );
    return FALSE;
}

/*
    Improve mental state                     -Thoric
*/
void better_mental_state( CHAR_DATA* ch, int mod )
{
    int c = URANGE( 0, abs( mod ), 20 );
    int rec = get_curr_rec( ch );
    c += number_percent() < rec ? 1 : 0;

    if ( ch->mental_state < 0 )
        ch->mental_state = URANGE( -100, ch->mental_state + c, 0 );
    else if ( ch->mental_state > 0 )
        ch->mental_state = URANGE( 0, ch->mental_state - c, 100 );
}


/*
    Deteriorate mental state                 -Thoric
*/
void worsen_mental_state( CHAR_DATA* ch, int mod )
{
    int c   = URANGE( 0, abs( mod ), 20 );
    int rec = get_curr_rec( ch );
    c -= number_percent() < rec ? 1 : 0;

    if ( c < 1 )
        return;

    if ( ch->mental_state < 0 )
        ch->mental_state = URANGE( -100, ch->mental_state - c, 100 );
    else if ( ch->mental_state > 0 )
        ch->mental_state = URANGE( -100, ch->mental_state + c, 100 );
    else
        ch->mental_state -= c;
}


/*
    stripclr - Removes the color codes from a string.

    This function should properly remove the color codes from a string.
*/
char* stripclr( char* text )
{
    int i = 0, j = 0;

    if ( !text || text[0] == '\0' )
    {
        return NULL;
    }
    else
    {
        char* buf;
        static char done[MAX_INPUT_LENGTH];
        done[0] = '\0';

        if ( ( buf = ( char* )malloc( strlen( text ) * sizeof( text ) ) ) == NULL )
            return text;

        /* Loop through until you've hit your terminating 0 */
        while ( text[i] != '\0' )
        {
            while ( text[i] == '&' )
            {
                i += 2;
            }

            if ( text[i] != '\0' )
            {
                if ( isspace( text[i] ) )
                {
                    buf[j] = ' ';
                    i++;
                    j++;
                }
                else
                {
                    buf[j] = text[i];
                    i++;
                    j++;
                }
            }
            else
                buf[j] = '\0';
        }

        buf[j] = '\0';
        sprintf( done, "%s", buf );
        buf = realloc( buf, j * sizeof( char ) );
        free( buf );
        return done;
    }
}

/*
    checkclr - Checks if a certain color code occurs in a string
*/
bool checkclr( char* text, char* token )
{
    int i = 0;

    if ( !text || text[0] == '\0' || !token || token[0] == '\0' )
    {
        return FALSE;
    }
    else
    {
        /* Loop through until you've hit your terminating 0 */
        while ( text[i] != '\0' )
        {
            if ( text[i] == '&' && text[i + 1] == token[0] )
                return TRUE;

            i++;
        }

        return FALSE;
    }
}

/*  Function: GetSoundexKey

    This function determines a soundex key from the string argument and returns
    the address of the key (which is stored in a static variable).  Because the
    most common use of the soundex key is to compare it with _another_ soundex
    key, this function uses two internal storage buffers, which are alternated
    between every time the function is called.

    The function takes one parameter, as follows:

    szTxt: A pointer to the string from which the soundex key is calculated.

    The return value is a pointer to the soundex key string.
*/
char* GetSoundexKey( const char* szTxt )
{
    int iOldIndex = 0; /* Loop index for the old (szTxt) string */
    int iNewIndex = 0; /* Loop index for the new (s_a_chSoundex) string */
    static char s_a_chSoundex[2][KEY_SIZE + 1]; /* Stores the new string */
    static unsigned iSoundex; /* Determines which s_a_chSoundex is used */
    iSoundex++; /* Switch to the other s_a_chSoundex array */
    s_a_chSoundex[iSoundex % 2][0] = '\0'; /* Clear any previous data */

    /* Copy the first character without conversion */
    if ( ( s_a_chSoundex[iSoundex % 2][iNewIndex++] = tolower( szTxt[iOldIndex++] ) ) )
    {
        do /* Loop through szTxt */
        {
            char chLetter; /* Stores the soundex value of a letter */

            /* Double/triple/etc letters are treated as single letters */
            while ( tolower( szTxt[iOldIndex] ) == tolower( szTxt[iOldIndex + 1] ) )
            {
                iOldIndex++;
                continue;
            }

            /* Convert the letter into its soundex value and store it */
            chLetter = LetterConversion( ( char )tolower( szTxt[iOldIndex] ) );

            /* Ignore NUL and 0 characters and only store KEY_SIZE characters */
            if ( chLetter != '\0' && chLetter != '0' && iNewIndex < KEY_SIZE )
            {
                /* Store the soundex value */
                s_a_chSoundex[iSoundex % 2][iNewIndex++] = chLetter;
            }
        } while ( szTxt[iOldIndex++] != '\0' );

        /* If less than KEY_SIZE characters were copied, buffer with zeros */
        while ( iNewIndex < KEY_SIZE )
        {
            s_a_chSoundex[iSoundex % 2][iNewIndex++] = '0';
        }

        /* Add the NUL terminator to the end of the soundex string */
        s_a_chSoundex[iSoundex % 2][iNewIndex] = '\0';
    }

    /* Return the address of the soundex string */
    return ( s_a_chSoundex[iSoundex % 2] );
}

/*  Function: SoundexMatch

    This function compares two soundex keys and returns a percentage match.

    The function takes two parameters, as follows:

    szFirst:  A pointer to the first soundex key.
    szSecond: A pointer to the second soundex key.

    The return value is an integer which contains the percentage match.
*/
int SoundexMatch( char* szFirst, char* szSecond )
{
    int iMatch = 0; /* Number of matching characters found */
    int iMax   = 0; /* Total number of characters compared */

    /* Make sure that both strings are of the correct size */
    if ( strlen( szFirst ) == KEY_SIZE && strlen( szSecond ) == KEY_SIZE )
    {
        int i; /* Loop counter */

        /* Loop through both strings */
        for ( i = 0; i < KEY_SIZE; i++ )
        {
            /* If either of the values are not NUL */
            if ( szFirst[i] != '0' || szSecond[i] != '0' )
            {
                iMax++; /* Increment the maximum */
            }

            /* If BOTH values are not NUL */
            if ( szFirst[i] != '0' && szSecond[i] != '0' )
            {
                /* Check for a match */
                if ( szFirst[i] == szSecond[i] )
                {
                    iMatch++; /* A match was found */
                }
            }
        }
    }

    /* Return the percentage match */
    return ( iMatch * 100 / iMax );
}

/*  Function: LetterConversion

    This function converts a single letter into it's appropriate soundex value.

    The function takes one parameter, as follows:

    chLetter: The letter to be converted.

    The return value is a single character which contains the converted value.
*/
/*static*/ char LetterConversion( char chLetter )
{
    const char* kszSoundexData = "01230120022455012623010202";
    char chResult; /* Store the soundex value, or NUL */

    if ( islower( chLetter ) )
    {
        /* Determine the soundex value associated with the letter */
        chResult = kszSoundexData[ ( chLetter - 'a' ) ];
    }
    else /* it's not a lowercase letter */
    {
        /* NUL means there is no associated soundex value */
        chResult = '\0';
    }

    /* Return the soundex value, or NUL if there isn't one */
    return ( chResult );
}

/*
    This system was added by Ghost =)
    It was written to check for the existance of players,
    even offline. This way, Players can place bounties
    on people offline. Fair?
*/
bool player_exist( char* name )
{
    char strsave[MAX_INPUT_LENGTH];
    // char buf[MAX_STRING_LENGTH];
    // struct stat fst;
    FILE* fp;
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );

    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
        fclose( fp );
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return FALSE;
}


bool is_spectator( CHAR_DATA* ch )
{
    if ( ch == NULL )
        return FALSE;

    if ( IS_NPC( ch ) )
        return FALSE;

    if ( !ch->pcdata )
        return FALSE;

    if ( !ch->pcdata->spectator )
        return FALSE;

    return TRUE;
}

int get_max_rounds( OBJ_DATA* obj )
{
    if ( !obj )
        return 0;

    if ( obj->pIndexData )
        return obj->pIndexData->value[2];

    if ( obj->value[2] > 0 )
        return obj->value[2];

    return 0;
}

void drop_morale( CHAR_DATA* ch, int amount )
{
    bool nofreeze = FALSE;

    if ( ch == NULL )
        return;

    if ( ch->race != RACE_MARINE )
        return;

    if ( IS_IMMORTAL( ch ) )
        return;

    if ( ch->morale <= 0 )
        nofreeze = TRUE;

    ch->morale = URANGE( 0, ch->morale - amount, get_max_morale( ch ) );

    if ( nofreeze )
        return;

    if ( ch->morale <= 0 )
        ch_printf( ch, "&Y(Alarm) Morale depleted. Good thing this isn't done, you lucky shmuck.\n\r" );

    return;
}

void set_variable( AREA_DATA* area, char* name, int value )
{
    VAR_DATA* var;

    if ( area == NULL )
        return;

    for ( var = area->first_var; var; var = var->next )
    {
        if ( var == NULL )
            break;

        if ( !str_cmp( var->name, name ) )
        {
            var->value = value;
            return;
        }
    }

    CREATE( var, VAR_DATA, 1 );
    var->next = NULL;
    var->prev = NULL;
    LINK( var, area->first_var, area->last_var, next, prev );
    var->name = STRALLOC( name );
    var->value = value;
    return;
}

int get_variable( AREA_DATA* area, char* name )
{
    VAR_DATA* var;

    if ( area == NULL )
        return -1;

    for ( var = area->first_var; var; var = var->next )
    {
        if ( var == NULL )
            break;

        if ( !str_cmp( var->name, name ) )
            return var->value;
    }

    return 0;
}

void clear_variables( AREA_DATA* area )
{
    VAR_DATA* var = NULL;
    VAR_DATA* vnext = NULL;

    if ( area == NULL )
        return;

    if ( area->first_var == NULL )
        return;

    for ( var = area->first_var; var; var = vnext )
    {
        if ( var == NULL )
            break;

        vnext = var->next;
        UNLINK( var, area->first_var, area->last_var, next, prev );
        var->next = NULL;
        var->prev = NULL;
        STRFREE( var->name );
        DISPOSE( var );
    }

    return;
}

void do_stack( CHAR_DATA* ch, char* argument )
{
    AREA_DATA* area;
    VAR_DATA* var;

    if ( !ch->in_room )
        return;

    area = ch->in_room->area;

    if ( !str_cmp( argument, "clear" ) )
    {
        clear_variables( area );
        ch_printf( ch, "&w&zCleared the variable stack for this area.\n\r" );
        return;
    }

    ch_printf( ch, "&w&BActive Variable Stack for this area:\n\r" );

    for ( var = area->first_var; var; var = var->next )
    {
        ch_printf( ch, "&w&z%-14s &G= &W%d\n\r", var->name, var->value );
    }

    return;
}

bool is_home( CHAR_DATA* ch )
{
    AREA_DATA* area = NULL;

    if ( ch == NULL )
        return TRUE;

    if ( ch->in_room == NULL )
        return TRUE;

    if ( ch->in_room->area == NULL )
        return TRUE;

    area = ch->in_room->area;

    if ( ch->race == RACE_MARINE )
    {
        if ( !str_cmp( area->filename, "uscm_cc.are" ) )
            return TRUE;
    }

    if ( ch->race == RACE_PREDATOR )
    {
        if ( !str_cmp( area->filename, "predator.are" ) )
            return TRUE;
    }

    if ( ch->race == RACE_ALIEN )
    {
        if ( !str_cmp( area->filename, "hive.are" ) )
            return TRUE;
    }

    if ( !str_cmp( area->filename, "limbo.are" ) )
        return TRUE;

    return FALSE;
}

int get_sprox( int ax, int ay, int az, int bx, int by, int bz )
{
    int prox;
    int x, y, z;
    x = ( ax - bx );

    if ( x < 0 )
        x = x - ( 2 * x );

    y = ( ay - by );

    if ( y < 0 )
        y = y - ( 2 * y );

    z = ( az - bz );

    if ( z < 0 )
        z = z - ( 2 * z );

    prox = sqrt( pow( x, 2 ) + pow( y, 2 ) );
    prox = sqrt( pow( z, 2 ) + pow( prox, 2 ) );
    return prox;
}

void echo_to_room_dnr ( int ecolor, ROOM_INDEX_DATA* room,  char* argument )
{
    CHAR_DATA* vic;

    if ( room == NULL )
        return;

    for ( vic = room->first_person; vic; vic = vic->next_in_room )
    {
        if ( !IS_AWAKE( vic ) )
            continue;

        set_char_color( ecolor, vic );
        send_to_char( argument, vic );
    }
}


bool file_exist( char* name )
{
    FILE* fp;

    if ( ( fp = fopen( name, "r" ) ) != NULL )
    {
        fclose( fp );
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return FALSE;
}

/*
    Compares two players IPs. Coded by Ghost.
*/
bool matching_ip( CHAR_DATA* ch, CHAR_DATA* gch )
{
    DESCRIPTOR_DATA* dA;
    DESCRIPTOR_DATA* dB;

    if ( ( dA = ch->desc ) != NULL )
    {
        if ( ( dB = gch->desc ) != NULL )
        {
            if ( !str_cmp( dA->host, dB->host ) )
                return TRUE;
        }
    }

    return FALSE;
}

bool is_ignored( CHAR_DATA* ch, char* name )
{
    if ( name == NULL )
        return FALSE;

    if ( IS_NPC( ch ) )
        return FALSE;

    if ( !ch->pcdata )
        return FALSE;

    return is_name( name, ch->pcdata->ignore );
}

/*
    WARNING - CH Can be NULL in this function!
*/
bool can_hive_room( CHAR_DATA* ch, ROOM_INDEX_DATA* room )
{
    if ( !xIS_SET( room->room_flags, ROOM_INDOORS ) && !xIS_SET( room->room_flags, ROOM_UNDERGROUND ) )
        return FALSE;

    if ( xIS_SET( room->room_flags, ROOM_NOHIVE ) )
        return FALSE;

    return TRUE;
}

void clear_timers( CHAR_DATA* ch )
{
    TIMER* timer = NULL;
    timer = get_timerptr( ch, TIMER_DO_FUN );

    if ( timer )
    {
        int tempsub;
        tempsub = ch->substate;
        ch->substate = SUB_TIMER_DO_ABORT;
        ( timer->do_fun )( ch, "" );

        if ( char_died( ch ) )
            return;

        if ( ch->substate != SUB_TIMER_CANT_ABORT )
        {
            ch->substate = tempsub;
            extract_timer( ch, timer );
        }
        else
        {
            ch->substate = tempsub;
            return;
        }
    }

    return;
}

int get_dark_range( CHAR_DATA* ch )
{
    OBJ_DATA* obj;
    int range = 0;
    int override = 0;

    if ( ch == NULL )
        return range;

    if ( ch->race == RACE_ALIEN )
        return 999;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
    {
        if ( obj->wear_loc == -1 )
            continue;

        if ( !obj->attach )
            continue;

        if ( obj->attach->value[0] == 6 )
            override = obj->attach->value[2];

        if ( check_light_modifier( obj ) > 0 )
            range = UMAX( 1, range );
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL )
    {
        if ( IS_SET( obj->value[3], BV00 ) )
            range = UMAX( 1, range );
    }

    if ( override > 0 )
        range = override;

    return range;
}

ROOM_INDEX_DATA* get_obj_room( OBJ_DATA* obj )
{
    OBJ_DATA* cont;

    if ( obj == NULL )
        return NULL;

    if ( obj->carried_by )
    {
        return obj->carried_by->in_room;
    }
    else if ( obj->in_room )
    {
        return obj->in_room;
    }
    else if ( obj->in_obj )
    {
        for ( cont = obj->in_obj; cont; cont = cont->in_obj )
        {
            if ( !cont || cont == NULL )
                break;

            if ( cont->in_room )
                return cont->in_room;
            else if ( cont->carried_by )
                return cont->carried_by->in_room;
        }
    }

    return NULL;
}

void skill_power( CHAR_DATA* ch, int gsn, int amount )
{
    if ( ch == NULL )
        return;

    if ( IS_NPC( ch ) )
        return;

    if ( ch->pcdata == NULL )
        return;

    ch->pcdata->prepared[gsn] += amount;
    ch->pcdata->prepared[gsn] = URANGE( 0, ch->pcdata->prepared[gsn], skill_table[gsn]->reset );
    return;
}

int count_players( void )
{
    DESCRIPTOR_DATA* d;
    int cnt = 0;

    for ( d = last_descriptor; d; d = d->prev )
    {
        CHAR_DATA* ch;

        if ( d->original )
            continue;

        ch = d->original ? d->original : d->character;

        if ( ch == NULL )
            continue;

        if ( IS_IDLE( d ) )
            continue;

        if ( xIS_SET( ch->act, PLR_AFK ) )
            continue;

        if ( IS_IMMORTAL( ch ) )
            continue;

        cnt++;
    }

    return cnt;
}

bool using_nvg( CHAR_DATA* ch )
{
    OBJ_DATA* obj = NULL;

    if ( ch == NULL )
        return FALSE;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
    {
        if ( obj->wear_loc == -1 )
            continue;

        if ( obj->item_type == ITEM_NVGOGGLE && IS_SET( obj->value[3], BV00 ) )
            return TRUE;
    }

    return FALSE;
}

