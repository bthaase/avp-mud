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
                           General Skills
           New Star Wars Skills Unit by Ghost AKA Camedo Natice
****************************************************************************/
#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
    RESEARCH can ONLY teach skills and languages! (Not spells anymore)
    -Research no longer needs books, but requires that you ARE
    -in a library. Plus, the time needed to research is based on the
    -level diffrence between the skill and your current level.
*/
void do_research( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    bool is_lang = FALSE, is_skill = FALSE;
    int ldiff, chance, sn, adept;
    int boost = 0;
    strcpy( arg, argument );

    if ( IS_NPC( ch ) )
    {
        send_to_char( "&RSorry! Mobs cant *RESEARCH!*\n\r", ch );
        return;
    }

    switch ( ch->substate )
    {
        default:
            if ( arg[0] == '\0' )
            {
                send_to_char( "&zUsage: Research <&CSKILL&z/&CLANGUAGE&z>\n\r&w", ch );
                return;
            }

            if ( !xIS_SET( ch->in_room->room_flags, ROOM_LIBRARY ) )
            {
                send_to_char( "&RYou need to be in a library to research.\n\r", ch );
                return;
            }

            sn = skill_lookup( arg );

            if ( sn == -1 )
            {
                send_to_char( "&RYou search and search but cant find that information.\n\r", ch );
                WAIT_STATE( ch, 3 );
                return;
            }

            if ( skill_table[sn]->guild <= ABILITY_NONE || skill_table[sn]->guild > MAX_ABILITY )
            {
                is_lang = TRUE;
            }
            else
            {
                if ( skill_table[sn]->guild != FORCE_ABILITY )
                    is_skill = TRUE;
            }

            if ( skill_table[sn]->guild == FORCE_ABILITY )
            {
                send_to_char( "&RYour not going to learn force spells from a book. Trust me.\n\r", ch );
                return;
            }

            if ( skill_table[sn]->guild == HUNTING_ABILITY )
            {
                send_to_char( "&RYou can't learn bounty hunting skills from a book!\n\r", ch );
                return;
            }

            if ( skill_table[sn]->guild == SMUGGLING_ABILITY )
            {
                send_to_char( "&RYou can't learn smuggling skills from a book!\n\r", ch );
                return;
            }

            if ( ch->skill_level[skill_table[sn]->guild] < skill_table[sn]->min_level )
            {
                send_to_char( "&RYou're not ready to learn that yet, Sorry....\n\r", ch );
                return;
            }

            if ( is_lang )
                adept = 95;

            if ( is_skill )
                adept = URANGE( 20, ( get_curr_int( ch ) * 5 ), 80 );

            if ( ch->pcdata->learned[sn] >= adept )
            {
                send_to_char( "&RYou can't learn any more about that from books!\n\r", ch );
                return;
            }

            send_to_char( "&GYou sit down and begin to study long and hard.\n\r", ch );
            act( AT_PLAIN, "$n sits down and begins to study quite hard.", ch, NULL, argument, TO_ROOM );
            add_timer ( ch, TIMER_DO_FUN, 10, do_research, 1 );
            ch->dest_buf = str_dup( arg );
            return;

        case 1:
            if ( !ch->dest_buf )
                return;

            strcpy( arg, ch->dest_buf );
            DISPOSE( ch->dest_buf );
            break;

        case SUB_TIMER_DO_ABORT:
            DISPOSE( ch->dest_buf );
            ch->substate = SUB_NONE;
            send_to_char( "&RYou are interupted and fail to finish your studies...\n\r", ch );
            return;
    }

    ch->substate = SUB_NONE;
    sn = skill_lookup( arg );

    if ( sn == -1 )
    {
        send_to_char( "&RHmmm... You forgot what you were studying. Tell an Immortal!\n\r", ch );
        return;
    }

    if ( skill_table[sn]->guild == FORCE_ABILITY )
        return;

    if ( skill_table[sn]->guild != FORCE_ABILITY )
        is_skill = TRUE;

    if ( !is_skill && ( skill_table[sn]->guild <= ABILITY_NONE || skill_table[sn]->guild > MAX_ABILITY ) )
        is_lang = TRUE;

    chance = IS_NPC( ch ) ? ch->top_level : ( int ) ( ch->pcdata->learned[gsn_research] ) - 50;

    if ( chance < 10 )
        chance = 10;

    /* At 100%= 50% chance to learn a normal skill    */
    if ( is_lang )
        chance += 25;    /* At 100%= 75% chance to learn a language        */

    if ( is_lang )
        adept = 95;

    if ( is_skill )
        adept = ( get_curr_int( ch ) * 5 );

    /*
        Level diffrence 'affect' chart
        chance = chance + ( ldiff * 5 )
    */
    ldiff = ( ch->skill_level[skill_table[sn]->guild] - skill_table[sn]->min_level );
    chance = URANGE( 30, chance + ( ldiff * 5 ), 90 );

    if ( number_percent( ) > chance )
    {
        send_to_char( "&RYou study for hours on end, but fail to gather any knowledge.\n\r", ch );
        learn_from_failure( ch, gsn_research );
        return;
    }

    send_to_char( "&GYou finish your studies and feel much more skilled.&w\n\r", ch );
    act( AT_PLAIN, "$n finishes studying the books.", ch, NULL, argument, TO_ROOM );
    boost = ( int_app[get_curr_int( ch )].learn / 2 );

    if ( is_lang && boost > 10 )
        boost = 10;

    if ( !is_lang && boost > 15 )
        boost = 15;

    if ( boost < 4 )
        boost = 4;

    ch->pcdata->learned[sn] += boost;
    ch->pcdata->learned[sn] = URANGE( 0, ch->pcdata->learned[sn], 100 );

    if ( !is_lang )
        ch->pcdata->learned[sn] = URANGE( 0, ch->pcdata->learned[sn], adept );

    learn_from_success( ch, gsn_research );
}
