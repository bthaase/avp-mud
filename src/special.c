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
               "Special procedure" module
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
    The following special functions are available for mobiles.
*/
DECLARE_SPEC_FUN(   spec_field_medic          );
DECLARE_SPEC_FUN(   spec_janitor              );
DECLARE_SPEC_FUN(   spec_enemy_scan           );
DECLARE_SPEC_FUN(   spec_enemy_grenade        );

/*
    Given a name, return the appropriate spec fun.
*/
SPEC_FUN* spec_lookup( const char* name )
{
    if ( !str_cmp( name, "spec_janitor"       ) )
        return spec_janitor;

    if ( !str_cmp( name, "spec_field_medic"       ) )
        return spec_field_medic;

    if ( !str_cmp( name, "spec_enemy_scan"        ) )
        return spec_enemy_scan;

    if ( !str_cmp( name, "spec_enemy_grenade"     ) )
        return spec_enemy_grenade;

    return 0;
}

/*
    Given a pointer, return the appropriate spec fun text.
*/
char* lookup_spec( SPEC_FUN* special )
{
    if ( special == spec_janitor        )
        return "spec_janitor";

    if ( special == spec_field_medic    )
        return "spec_field_medic";

    if ( special == spec_enemy_scan     )
        return "spec_enemy_scan";

    if ( special == spec_enemy_grenade  )
        return "spec_enemy_grenade";

    return "";
}


bool spec_janitor( CHAR_DATA* ch )
{
    OBJ_DATA* trash;
    OBJ_DATA* trash_next;

    if ( !IS_AWAKE( ch ) )
        return FALSE;

    for ( trash = ch->in_room->first_content; trash; trash = trash_next )
    {
        trash_next = trash->next_content;

        if ( !xIS_SET( trash->wear_flags, ITEM_TAKE )
                ||    IS_OBJ_STAT( trash, ITEM_BURRIED ) )
            continue;

        if ( trash->item_type == ITEM_DRINK_CON
                ||   trash->item_type == ITEM_TRASH
                ||   trash->cost < 10 )
        {
            act( AT_ACTION, "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
            obj_from_room( trash );
            obj_to_char( trash, ch );
            return TRUE;
        }
    }

    return FALSE;
}



bool spec_field_medic( CHAR_DATA* ch )
{
    CHAR_DATA* victim;
    CHAR_DATA* v_next;
    char buf[MAX_STRING_LENGTH];

    if ( !IS_AWAKE( ch ) )
        return FALSE;

    if ( ++ch->busy > 24 )
    {
        if ( number_range( 1, 20 ) == 1 )
        {
            switch ( number_range( 1, 6 ) )
            {
                default:
                    do_emote( ch, "searchs for a wounded victim." );
                    break;

                case 1:
                case 2:
                    do_emote( ch, "looks around, obviously quite bored." );
                    break;

                case 3:
                case 4:
                    do_emote( ch, "grumbles, looking for something to do." );
                    break;

                case 5:
                case 6:
                    do_emote( ch, "fiddles with a syringe." );
                    break;
            }
        }
    }

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
        v_next = victim->next_in_room;

        if ( !can_see( ch, victim ) )
            continue;

        /*
            if ( IS_NPC( victim ) )
            continue;
        */
        if ( victim->hit < 5 )
        {
            sprintf( buf, "%s kneels to tend to %s's wounds.", ch->short_descr, victim->name );
            echo_to_room( AT_GREEN, victim->in_room, buf );
            victim->hit += number_range( ch->top_level * 0.7, ch->top_level * 1.14 );

            if ( victim->hit == 0 )
                victim->hit += number_range( 1, 3 );

            update_pos( victim );
            do_say( ch, "That should be a bit better." );
            ch->busy = 0;
            break;
        }
    }

    return FALSE;
}

bool spec_enemy_scan( CHAR_DATA* ch )
{
    CHAR_DATA*        victim;
    sh_int            dir, dist;
    sh_int            max_dist = 2;
    EXIT_DATA*        pexit;
    ROOM_INDEX_DATA* curr_room;
    ROOM_INDEX_DATA* to_room;

    if ( ch->hunting )
        return;

    max_dist = ( int )( get_curr_per( ch ) / 3 );

    for ( dir = 0 ; dir <= 10 ; dir++ )
    {
        if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
            continue;

        for ( dist = 0; dist <= max_dist; dist++ )
        {
            if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
                break;

            if ( !pexit->to_room )
                break;

            to_room = NULL;

            if ( pexit->distance > 1 )
                to_room = generate_exit( curr_room, &pexit );

            if ( to_room == NULL )
                to_room = pexit->to_room;

            curr_room = to_room;

            for ( victim = curr_room->first_person; victim; victim = victim->next_in_room )
            {
                if ( can_see( ch, victim ) && is_enemy( victim, ch ) && !is_spectator( victim ) )
                {
                    /* Found enemy */
                    start_hating( ch, victim );
                    start_hunting( ch, victim );
                    return TRUE;
                }
            }

            if ( ( pexit = get_exit( curr_room, dir ) ) == NULL )
                break;
        }
    }

    return FALSE;
}

/*
    Three stages:
    1) Check for held grenade. If none, try to find one to hold.
    2) Scan for enemies. None found? Bail.
    3) Found a target? Arm and lob the sucker.
*/
bool spec_enemy_grenade( CHAR_DATA* ch )
{
    char              buf[MAX_INPUT_LENGTH];
    CHAR_DATA*        victim;
    OBJ_DATA*         obj;
    sh_int            dir, dist;
    sh_int            max_dist;
    EXIT_DATA*        pexit;
    ROOM_INDEX_DATA* curr_room;
    ROOM_INDEX_DATA* to_room;
    max_dist = URANGE( 2, ( get_curr_str( ch ) / 5 ), 6 );
    obj = get_eq_char( ch, WEAR_HOLD );

    if ( obj == NULL )
    {
        /* Lookup a grenade in the INV */
        for ( obj = ch->first_carrying; obj; obj = obj->next_content )
        {
            if ( obj == NULL )
                continue;

            if ( obj->item_type != ITEM_GRENADE )
                continue;

            wear_obj( ch, obj, TRUE, -1 ); // Hold the grenade
            return FALSE;
        }

        return FALSE;
    }

    if ( number_range( 1, 3 ) == 1 )
        return FALSE;

    for ( dir = 0 ; dir <= 10 ; dir++ )
    {
        if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
            continue;

        for ( dist = 1; dist <= max_dist; dist++ )
        {
            if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
                break;

            if ( !pexit->to_room )
                break;

            to_room = NULL;

            if ( pexit->distance > 1 )
                to_room = generate_exit( curr_room, &pexit );

            if ( to_room == NULL )
                to_room = pexit->to_room;

            curr_room = to_room;

            if ( dist < 2 )
            {
                if ( ( pexit = get_exit( curr_room, dir ) ) == NULL )
                    break;

                continue;
            }

            for ( victim = curr_room->first_person; victim; victim = victim->next_in_room )
            {
                if ( can_see( ch, victim ) && is_enemy( victim, ch ) && !is_spectator( victim ) )
                {
                    /* Found enemy - Nuke 'em */
                    sprintf( buf, "%s %d", dir_name[dir], dist );
                    do_arm( ch, "5" );
                    do_lob( ch, buf );
                    return TRUE;
                }
            }

            if ( ( pexit = get_exit( curr_room, dir ) ) == NULL )
                break;
        }
    }

    return FALSE;
}

