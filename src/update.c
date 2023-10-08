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
                  Regular update module
****************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include "mud.h"
#include "mqtt.h"

/* from swskills.c */
void    add_reinforcements  args( ( CHAR_DATA* ch ) );

/*
    Local functions.
*/
int hit_gain    args( ( CHAR_DATA* ch ) );
int move_gain   args( ( CHAR_DATA* ch ) );
int     field_gain      args( ( CHAR_DATA* ch ) );
void    mobile_update   args( ( void ) );
void    weather_update  args( ( void ) );
void    char_update args( ( void ) );
void    obj_update  args( ( void ) );
void    aggr_update args( ( void ) );
void    room_act_update args( ( void ) );
void    obj_act_update  args( ( void ) );
void    char_check  args( ( void ) );
void    update_object_pulse  args( ( void ) );
void    update_backup   args( ( void ) );
void    update_bacta    args( ( void ) );
void    halucinations   args( ( CHAR_DATA* ch ) );
void    subtract_times  args( ( struct timeval* etime,
                                struct timeval* stime ) );
void    update_xnames( void );
void    update_logalerts args( ( void ) );


/*
    Global Variables
*/

CHAR_DATA*  gch_prev;
OBJ_DATA*   gobj_prev;

CHAR_DATA*  timechar;

char* corpse_descs[] =
{
    "The corpse of %s will soon be gone.",
    "The corpse of %s lies here.",
    "The corpse of %s lies here.",
    "The corpse of %s lies here.",
    "The corpse of %s lies here."
};

char* ashes_descs[] =
{
    "The smoking remains of %s will soon be gone.",
    "The smoking remains of %s lies here.",
    "The smoking remains of %s lies here.",
    "The smoking remains of %s lies here.",
    "The smoking remains of %s lies here.",
};

char* d_corpse_descs[] =
{
    "The shattered remains %s will soon be gone.",
    "The shattered remains %s are here.",
    "The shattered remains %s are here.",
    "The shattered remains %s are here.",
    "The shattered remains %s are here."
};

extern int      top_exit;

void advance_level( CHAR_DATA* ch, bool silent )
{
    if ( ch->top_level < 100 )
    {
        if ( !silent )
            ch_printf( ch, "\n\r&YYou have now reached level %d! Your rank is %s.\n\r", ch->top_level + 1, get_rank( ch->race, ch->top_level + 1 ) );

        ch->top_level++;
    }

    if ( !silent )
        do_save( ch, "-noalert" );

    return;
}

void gain_exp( CHAR_DATA* ch, int gain )
{
    if ( IS_NPC( ch ) )
        return;

    ch->currexp = UMAX( 0, ch->currexp + gain );
    ch->maxexp = UMAX( 0, ch->maxexp + gain );
    return;
}

/*
    Regeneration stuff.
*/
int hit_gain( CHAR_DATA* ch )
{
    int gain;

    if ( IS_NPC( ch ) )
    {
        gain = ch->top_level;
    }
    else
    {
        gain = get_curr_rec( ch );

        switch ( ch->position )
        {
            case POS_DEAD:
                return 0;

            case POS_MORTAL:
                return -10;

            case POS_INCAP:
                return 1;

            case POS_STUNNED:
                return get_curr_rec( ch ) * 2;

            case POS_PRONE:
                gain += get_curr_rec( ch ) * 2;
                break;

            case POS_KNEELING:
                gain += get_curr_rec( ch ) * 1;
                break;
        }

        if ( gain <= 0 )
            return gain;
    }

    if ( IS_AFFECTED( ch, AFF_POISON ) && gain > 0 )
        gain /= 4;
    else if ( IS_AFFECTED( ch, AFF_POISON ) )
        gain *= 2;

    if ( ch->race == RACE_ALIEN && xIS_SET( ch->in_room->room_flags, ROOM_HIVED ) )
        gain *= 2;

    if ( ch->swarm )
        gain = ( int )( ( float )( gain ) * ( float )( 1.1 ) );

    return UMIN( gain, ch->max_hit - ch->hit );
}


int move_gain( CHAR_DATA* ch )
{
    int gain;

    if ( IS_NPC( ch ) )
    {
        gain = URANGE( 1, ( int )( ( float )( ch->top_level ) / ( float )( 2 ) ), 10 );
    }
    else
    {
        gain = URANGE( 1, ( int )( float )( get_curr_rec( ch ) ) / ( float )( 2 ), 10 );

        switch ( ch->position )
        {
            case POS_DEAD:
                return 0;

            case POS_MORTAL:
                return -1;

            case POS_INCAP:
                return -1;

            case POS_STUNNED:
                return 1;

            case POS_PRONE:
                gain *= 2;
                break;

            case POS_KNEELING:
                gain *= 1;
                break;
        }
    }

    if ( IS_AFFECTED( ch, AFF_POISON ) )
        gain /= 2;

    gain = URANGE( 1, gain, 20 );

    if ( ch->swarm )
        gain = ( int )( ( float )( gain ) * ( float )( 1.1 ) );

    return UMIN( gain, ch->max_move - ch->move );
}

int field_gain( CHAR_DATA* ch )
{
    int gain;

    if ( IS_NPC( ch ) )
    {
        gain = ch->top_level;
    }
    else
    {
        gain = URANGE( 1, get_curr_rec( ch ) / 10, 3 );

        switch ( ch->position )
        {
            case POS_DEAD:
                return 0;

            case POS_MORTAL:
                return -3;

            case POS_INCAP:
                return -2;

            case POS_STUNNED:
                return -1;
        }
    }

    return UMIN( gain, ch->max_field - ch->field );
}

/*
    Mob autonomous action.
    This function takes 25% to 35% of ALL Mud cpu time.
*/
void mobile_update( void )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* ch = NULL;
    EXIT_DATA* pexit = NULL;
    int door;
    ch_ret     retcode;
    retcode = rNONE;

    /* Examine all mobs. */
    for ( ch = last_char; ch; ch = gch_prev )
    {
        set_cur_char( ch );

        if ( ch == first_char && ch->prev )
        {
            bug( "mobile_update: first_char->prev != NULL... fixed", 0 );
            ch->prev = NULL;
        }

        gch_prev = ch->prev;

        if ( gch_prev && gch_prev->next != ch )
        {
            sprintf( buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.",
                     ch->name );
            bug( buf, 0 );
            bug( "Short-cutting here", 0 );
            gch_prev = NULL;
            ch->prev = NULL;
        }

        if ( !IS_NPC( ch ) )
        {
            halucinations( ch );
            continue;
        }

        if ( !ch->in_room
                ||   IS_AFFECTED( ch, AFF_CHARM )
                ||   IS_AFFECTED( ch, AFF_PARALYSIS ) )
            continue;

        /* Clean up 'animated corpses' that are not charmed' - Scryn */

        if ( ch->pIndexData->vnum == 5 && !IS_AFFECTED( ch, AFF_CHARM ) )
        {
            if ( ch->in_room->first_person )
                act( AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM );

            if ( IS_NPC( ch ) ) /* Guard against purging switched? */
                extract_char( ch, TRUE, FALSE );

            continue;
        }

        if ( !xIS_SET( ch->act, ACT_RUNNING ) && ch->hunting )
        {
            if ( number_range( 1, 3 ) == 1 )
            {
                hunt_victim( ch );
                continue;
            }
        }
        else if ( !ch->hunting && !xIS_SET( ch->act, ACT_RUNNING ) && ch->was_sentinel && ch->position >= POS_STANDING )
        {
            act( AT_ACTION, "$n leaves.", ch, NULL, NULL, TO_ROOM );
            char_from_room( ch );
            char_to_room( ch, ch->was_sentinel );
            act( AT_ACTION, "$n arrives.", ch, NULL, NULL, TO_ROOM );
            xSET_BIT( ch->act, ACT_SENTINEL );
            ch->was_sentinel = NULL;
        }

        /* Examine call for special procedure */
        if ( !xIS_SET( ch->act, ACT_RUNNING ) && ch->spec_fun )
        {
            if ( ( *ch->spec_fun ) ( ch ) )
                continue;

            if ( char_died( ch ) )
                continue;
        }

        if ( !xIS_SET( ch->act, ACT_RUNNING ) && ch->spec_2 )
        {
            if ( ( *ch->spec_2 ) ( ch ) )
                continue;

            if ( char_died( ch ) )
                continue;
        }

        if ( !xIS_SET( ch->act, ACT_RUNNING ) && ch->spec_3 )
        {
            if ( ( *ch->spec_3 ) ( ch ) )
                continue;

            if ( char_died( ch ) )
                continue;
        }

        if ( !xIS_SET( ch->act, ACT_RUNNING ) && ch->spec_4 )
        {
            if ( ( *ch->spec_4 ) ( ch ) )
                continue;

            if ( char_died( ch ) )
                continue;
        }

        /* Check for mudprogram script on mob */
        if ( xIS_SET( ch->pIndexData->progtypes, SCRIPT_PROG ) )
        {
            mprog_script_trigger( ch );
            continue;
        }

        if ( ch != cur_char )
        {
            bug( "Mobile_update: ch != cur_char after spec_fun", 0 );
            continue;
        }

        /* That's all for sleeping / busy monster */
        // if ( ch->position != POS_STANDING ) continue;

        /*
            if ( xIS_SET(ch->act, ACT_MOUNTED ) )
            {
                   if ( xIS_SET(ch->act, ACT_AGGRESSIVE) )
            do_emote( ch, "snarls and growls." );
            continue;
            }
        */

        /* MOBprogram random trigger */
        if ( ch->in_room->area->nplayer > 0 )
        {
            mprog_random_trigger( ch );

            if ( char_died( ch ) )
                continue;

            if ( ch->position < POS_STANDING )
                continue;
        }

        /* MOBprogram hour trigger: do something for an hour */
        mprog_hour_trigger( ch );

        if ( char_died( ch ) )
            continue;

        rprog_hour_trigger( ch );

        if ( char_died( ch ) )
            continue;

        if ( ch->position < POS_STANDING )
            continue;

        /* Scavenge */
        if ( xIS_SET( ch->act, ACT_SCAVENGER )
                &&   ch->in_room->first_content
                &&   number_bits( 2 ) == 0 )
        {
            OBJ_DATA* obj;
            OBJ_DATA* obj_best;
            int max;
            max         = 1;
            obj_best    = NULL;

            for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
            {
                if ( CAN_WEAR( obj, ITEM_TAKE ) && obj->cost > max
                        && !IS_OBJ_STAT( obj, ITEM_BURRIED ) )
                {
                    obj_best    = obj;
                    max         = obj->cost;
                }
            }

            if ( obj_best )
            {
                obj_from_room( obj_best );
                obj_to_char( obj_best, ch );
                act( AT_ACTION, "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
            }
        }

        /* Wander */
        if ( !xIS_SET( ch->act, ACT_RUNNING )
                &&   !xIS_SET( ch->act, ACT_SENTINEL )
                &&   !xIS_SET( ch->act, ACT_PROTOTYPE )
                && ( door = number_bits( 5 ) ) <= 9
                && ( pexit = get_exit( ch->in_room, door ) ) != NULL
                &&   pexit->to_room
                &&   !xIS_SET( pexit->exit_info, EX_CLOSED )
                &&   !xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
                && ( !xIS_SET( ch->act, ACT_STAY_AREA )
                     ||   pexit->to_room->area == ch->in_room->area ) )
        {
            retcode = move_char( ch, pexit, pexit->vdir, 0 );

            /*  If ch changes position due
                to it's or someother mob's
                movement via MOBProgs,
                continue - Kahn */
            if ( char_died( ch ) )
                continue;

            if ( retcode != rNONE || xIS_SET( ch->act, ACT_SENTINEL )
                    ||    ch->position < POS_STANDING )
                continue;
        }

        /* Flee */
        if ( ch->hit < ch->max_hit / 2
                && ( door = number_bits( 4 ) ) <= 9
                && ( pexit = get_exit( ch->in_room, door ) ) != NULL
                &&   pexit->to_room
                &&   !xIS_SET( pexit->exit_info, EX_CLOSED )
                &&   !xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) )
        {
            CHAR_DATA* rch;
            bool found;
            found = FALSE;

            for ( rch  = ch->in_room->first_person;
                    rch;
                    rch  = rch->next_in_room )
            {
                if ( is_fearing( ch, rch ) )
                {
                    switch ( number_bits( 2 ) )
                    {
                        case 0:
                            sprintf( buf, "Get away from me, %s!", rch->name );
                            break;

                        case 1:
                            sprintf( buf, "Leave me be, %s!", rch->name );
                            break;

                        case 2:
                            sprintf( buf, "%s is trying to kill me!  Help!", rch->name );
                            break;

                        case 3:
                            sprintf( buf, "Someone save me from %s!", rch->name );
                            break;
                    }

                    do_yell( ch, buf );
                    found = TRUE;
                    break;
                }
            }

            if ( found )
                retcode = move_char( ch, pexit, pexit->vdir, 0 );
        }
    }

    return;
}


/*
    Update the weather.
*/
void weather_update( void )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA* d;
    int diff;
    sh_int AT_TEMP = AT_PLAIN;
    buf[0] = '\0';

    switch ( ++time_info.hour )
    {
        case  5:
            weather_info.sunlight = SUN_LIGHT;
            strcat( buf, "The day has begun." );
            AT_TEMP = AT_YELLOW;
            break;

        case  6:
            weather_info.sunlight = SUN_RISE;
            strcat( buf, "The sun rises in the distance." );
            AT_TEMP = AT_ORANGE;
            break;

        case 12:
            weather_info.sunlight = SUN_LIGHT;
            strcat( buf, "It's noon." );
            AT_TEMP = AT_YELLOW;
            break;

        case 19:
            weather_info.sunlight = SUN_SET;
            strcat( buf, "The sun slowly disappears in the distance." );
            AT_TEMP = AT_BLOOD;
            break;

        case 20:
            weather_info.sunlight = SUN_DARK;
            strcat( buf, "The night has begun." );
            AT_TEMP = AT_DGREY;
            break;

        case 24:
            time_info.hour = 0;
            time_info.day++;
            break;
    }

    if ( time_info.day   >= 30 )
    {
        time_info.day = 0;
        time_info.month++;
    }

    if ( time_info.month >= 12 )
    {
        time_info.month = 0;
        time_info.year++;
    }

    if ( buf[0] != '\0' )
    {
        for ( d = first_descriptor; d; d = d->next )
        {
            if ( d->connected == CON_PLAYING
                    &&   IS_OUTSIDE( d->character )
                    &&   IS_AWAKE( d->character )
                    &&   d->character->in_room
                    &&   d->character->in_room->sector_type != SECT_UNDERWATER
                    &&   d->character->in_room->sector_type != SECT_OCEANFLOOR
                    &&   d->character->in_room->sector_type != SECT_UNDERGROUND )
                act( AT_TEMP, buf, d->character, 0, 0, TO_CHAR );
        }

        buf[0] = '\0';
    }

    /*
        Weather change.
    */
    if ( time_info.month >= 9 && time_info.month <= 16 )
        diff = weather_info.mmhg >  985 ? -2 : 2;
    else
        diff = weather_info.mmhg > 1015 ? -2 : 2;

    weather_info.change   += diff * dice( 1, 4 ) + dice( 2, 6 ) - dice( 2, 6 );
    weather_info.change    = UMAX( weather_info.change, -12 );
    weather_info.change    = UMIN( weather_info.change,  12 );
    weather_info.mmhg += weather_info.change;
    weather_info.mmhg  = UMAX( weather_info.mmhg,  960 );
    weather_info.mmhg  = UMIN( weather_info.mmhg, 1040 );
    AT_TEMP = AT_GREY;

    switch ( weather_info.sky )
    {
        default:
            bug( "Weather_update: bad sky %d.", weather_info.sky );
            weather_info.sky = SKY_CLOUDLESS;
            break;

        case SKY_CLOUDLESS:
            if ( weather_info.mmhg <  990
                    || ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) )
            {
                strcat( buf, "The sky is getting cloudy." );
                weather_info.sky = SKY_CLOUDY;
                AT_TEMP = AT_GREY;
            }

            break;

        case SKY_CLOUDY:
            if ( weather_info.mmhg <  970
                    || ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) )
            {
                strcat( buf, "It starts to rain." );
                weather_info.sky = SKY_RAINING;
                AT_TEMP = AT_BLUE;
            }

            if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 )
            {
                strcat( buf, "The clouds disappear." );
                weather_info.sky = SKY_CLOUDLESS;
                AT_TEMP = AT_WHITE;
            }

            break;

        case SKY_RAINING:
            if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 )
            {
                strcat( buf, "Lightning flashes in the sky." );
                weather_info.sky = SKY_LIGHTNING;
                AT_TEMP = AT_YELLOW;
            }

            if ( weather_info.mmhg > 1030
                    || ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) )
            {
                strcat( buf, "The rain stopped." );
                weather_info.sky = SKY_CLOUDY;
                AT_TEMP = AT_WHITE;
            }

            break;

        case SKY_LIGHTNING:
            if ( weather_info.mmhg > 1010
                    || ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) )
            {
                strcat( buf, "The lightning has stopped." );
                weather_info.sky = SKY_RAINING;
                AT_TEMP = AT_GREY;
                break;
            }

            break;
    }

    if ( buf[0] != '\0' )
    {
        for ( d = first_descriptor; d; d = d->next )
        {
            if ( d->connected == CON_PLAYING
                    &&   IS_OUTSIDE( d->character )
                    &&   IS_AWAKE( d->character ) )
                act( AT_TEMP, buf, d->character, 0, 0, TO_CHAR );
        }
    }

    return;
}



/*
    Update all chars, including mobs.
    This function is performance sensitive.
*/
void char_update( void )
{
    CHAR_DATA* ch;
    CHAR_DATA* ch_save;
    sh_int save_count = 0;
    write_serverstats();
    ch_save = NULL;

    for ( ch = last_char; ch; ch = gch_prev )
    {
        if ( ch == first_char && ch->prev )
        {
            bug( "char_update: first_char->prev != NULL... fixed", 0 );
            ch->prev = NULL;
        }

        gch_prev = ch->prev;
        set_cur_char( ch );

        if ( gch_prev && gch_prev->next != ch )
        {
            bug( "char_update: ch->prev->next != ch", 0 );
            return;
        }

        /*
            Team-Killing Defense
        */
        if ( ch->teamkill < get_max_teamkill( ch ) )
        {
            ch->teamkill++;

            if ( xIS_SET( ch->act, PLR_FREEZE ) && ch->teamkill >= URANGE( 1, get_max_teamkill( ch ), 3 ) )
            {
                ch_printf( ch, "&R[Notice]: Assuming you wise up, you may now resume playing.\n\r" );
                xREMOVE_BIT( ch->act, PLR_FREEZE );
            }
            else if ( xIS_SET( ch->act, PLR_FREEZE ) )
            {
                ch_printf( ch, "&w&C[Notice]: Still frozen. You got about %d tick(s) left.\n\r", URANGE( 1, get_max_teamkill( ch ), 3 ) - ch->teamkill );
            }
        }

        /*
            Do a room_prog rand check right off the bat
             if ch disappears (rprog might wax npc's), continue
        */
        if ( !IS_NPC( ch ) )
            rprog_random_trigger( ch );

        if ( char_died( ch ) )
            continue;

        if ( IS_NPC( ch ) )
        {
            mprog_time_trigger( ch );
            mob_reload( ch, get_eq_char( ch, WEAR_WIELD ) );
            mob_reload( ch, get_eq_char( ch, WEAR_DUAL_WIELD ) );
        }

        if ( char_died( ch ) )
            continue;

        rprog_time_trigger( ch );

        if ( char_died( ch ) )
            continue;

        /*
            See if player should be auto-saved.
        */
        if ( !IS_NPC( ch )
                &&   !NOT_AUTHED( ch )
                &&   current_time - ch->save_time > ( sysdata.save_frequency * 60 ) )
            ch_save = ch;
        else
            ch_save = NULL;

        if ( ch->position >= POS_STUNNED )
        {
            if ( ch->hit  < ch->max_hit )
                ch->hit  += hit_gain( ch );

            if ( ch->field < ch->max_field )
                ch->field += field_gain( ch );
        }

        /*
            if ( ch->position == POS_STUNNED )
            update_pos( ch );
        */
        update_pos( ch );

        if ( ch->position == POS_DEAD )
        {
            if ( IS_BOT( ch ) )
            {
                match_log( "KILL;'BOT:%s' bled to death.", ch->name );
            }
            else if ( IS_NPC( ch ) )
            {
                match_log( "KILL;'NPC:%s' bled to death.", ch->name );
            }
            else
            {
                match_log( "KILL;'PC:%s' bled to death.", ch->name );
            }

            set_cur_char( ch );
            raw_kill( ch, ch );
        }

        /*
            Equipment Drain
        */
        if ( !IS_NPC( ch ) && !is_spectator( ch ) )
        {
            OBJ_DATA* obj;
            /*
                Equipment Recharge
            */
            equipment_recharge( ch );

            /*
                Cloaking drain
            */
            if ( xIS_SET( ch->affected_by, AFF_CLOAK ) )
            {
                if ( ( ch->field -= 4 ) <= 0 )
                {
                    ch->field = 0;
                    ch_printf( ch, "&R[Alert]: Field charge depleted. Cloaking disabled.\n\r" );
                    xREMOVE_BIT( ch->affected_by, AFF_CLOAK );

                    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
                    {
                        if ( obj->item_type == ITEM_CLOAK )
                            REMOVE_BIT( obj->value[0], BV00 );
                    }
                }
            }
        }

        if ( !char_died( ch ) )
        {
            /*
                Careful with the damages here,
                 MUST NOT refer to ch after damage taken,
                 as it may be lethal damage (on NPC).
            */
            if ( IS_AFFECTED( ch, AFF_POISON ) )
            {
                AFFECT_DATA* paf;
                // int dam = 0;

                for ( paf = ch->first_affect; paf; paf = paf->next )
                    if ( xIS_SET( paf->bitvector, AFF_POISON ) )
                    {
                        act( AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
                        act( AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR );
                        ch->mental_state = URANGE( 20, ch->mental_state + 4, 100 );
                        // dam = number_range( 10, 25 );
                        bug( "Incomplete -> Cannot deal poison damage" );
                        // if ( !IS_NPC( ch ) ) damage( ch, ch, dam, gsn_poison );
                        // if ( IS_NPC( ch ) ) damage( ch, ch, dam*5, gsn_poison );
                    }
            }
            else if ( ch->position == POS_INCAP )
                damage( ch, ch, 25, TYPE_UNDEFINED );
            else if ( ch->position == POS_MORTAL )
                damage( ch, ch, 50, TYPE_UNDEFINED );

            if ( char_died( ch ) )
                continue;

            if ( ch->mental_state >= 30 )
                switch ( ( ch->mental_state + 5 ) / 10 )
                {
                    case  3:
                        send_to_char( "You feel feverish.\n\r", ch );
                        act( AT_ACTION, "$n looks kind of out of it.", ch, NULL, NULL, TO_ROOM );
                        break;

                    case  4:
                        send_to_char( "You do not feel well at all.\n\r", ch );
                        act( AT_ACTION, "$n doesn't look too good.", ch, NULL, NULL, TO_ROOM );
                        break;

                    case  5:
                        send_to_char( "You need help!\n\r", ch );
                        act( AT_ACTION, "$n looks like $e could use your help.", ch, NULL, NULL, TO_ROOM );
                        break;

                    case  6:
                        send_to_char( "Seekest thou a cleric.\n\r", ch );
                        act( AT_ACTION, "Someone should fetch a healer for $n.", ch, NULL, NULL, TO_ROOM );
                        break;

                    case  7:
                        send_to_char( "You feel reality slipping away...\n\r", ch );
                        act( AT_ACTION, "$n doesn't appear to be aware of what's going on.", ch, NULL, NULL, TO_ROOM );
                        break;

                    case  8:
                        send_to_char( "You begin to understand... everything.\n\r", ch );
                        act( AT_ACTION, "$n starts ranting like a madman!", ch, NULL, NULL, TO_ROOM );
                        break;

                    case  9:
                        send_to_char( "You are ONE with the universe.\n\r", ch );
                        act( AT_ACTION, "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...", ch, NULL, NULL, TO_ROOM );
                        break;

                    case 10:
                        send_to_char( "You feel the end is near.\n\r", ch );
                        act( AT_ACTION, "$n is muttering and ranting in tongues...", ch, NULL, NULL, TO_ROOM );
                        break;
                }

            if ( ch->mental_state <= -30 )
                switch ( ( abs( ch->mental_state ) + 5 ) / 10 )
                {
                    case  10:
                        if ( ch->position > POS_STUNNED )
                        {
                            if ( ( ch->position == POS_STANDING || ch->position <= POS_SITTING )
                                    &&    number_percent() + 10 < abs( ch->mental_state ) )
                                do_sit( ch, "" );
                            else
                                send_to_char( "You're barely conscious.\n\r", ch );
                        }

                        break;

                    case   9:
                        if ( ch->position > POS_STUNNED )
                        {
                            if ( ( ch->position == POS_STANDING || ch->position <= POS_SITTING )
                                    &&   ( number_percent() + 20 ) < abs( ch->mental_state ) )
                                do_sit( ch, "" );
                            else
                                send_to_char( "You can barely keep your eyes open.\n\r", ch );
                        }

                        break;

                    case   8:
                        if ( ch->position > POS_STUNNED )
                        {
                            if ( ch->position < POS_SITTING
                                    &&  ( number_percent() + 30 ) < abs( ch->mental_state ) )
                                do_sit( ch, "" );
                            else
                                send_to_char( "You're extremely drowsy.\n\r", ch );
                        }

                        break;

                    case   7:
                        if ( ch->position > POS_STUNNED )
                            send_to_char( "You feel very unmotivated.\n\r", ch );

                        break;

                    case   6:
                        if ( ch->position > POS_STUNNED )
                            send_to_char( "You feel sedated.\n\r", ch );

                        break;

                    case   5:
                        if ( ch->position > POS_STUNNED )
                            send_to_char( "You feel sleepy.\n\r", ch );

                        break;

                    case   4:
                        if ( ch->position > POS_STUNNED )
                            send_to_char( "You feel tired.\n\r", ch );

                        break;

                    case   3:
                        if ( ch->position > POS_STUNNED )
                            send_to_char( "You could use a rest.\n\r", ch );

                        break;
                }

            if ( !IS_NPC ( ch ) )
            {
                if ( ++ch->timer > 15 && !ch->desc )
                {
                    // if ( ch->in_room )
                    //    char_from_room( ch );
                    // char_to_room( ch , get_room_index( ROOM_PLUOGUS_QUIT ) );
                    ch->position = POS_SITTING;
                    ch->hit = UMAX ( 1, ch->hit );
                    save_char_obj( ch );
                    do_quit( ch, "" );
                }
                else if ( ch == ch_save && IS_SET( sysdata.save_flags, SV_AUTO )
                          &&   ++save_count < 10 )   /* save max of 10 per tick */
                    save_char_obj( ch );
            }
        }
    }

    return;
}



/*
    Update all objs.
    This function is performance sensitive.
*/
void obj_update( void )
{
    OBJ_DATA* obj;
    sh_int AT_TEMP;

    for ( obj = last_object; obj; obj = gobj_prev )
    {
        CHAR_DATA* rch;
        char* message;

        if ( obj == first_object && obj->prev )
        {
            bug( "obj_update: first_object->prev != NULL... fixed", 0 );
            obj->prev = NULL;
        }

        gobj_prev = obj->prev;

        if ( gobj_prev && gobj_prev->next != obj )
        {
            bug( "obj_update: obj->prev->next != obj", 0 );
            return;
        }

        set_cur_obj( obj );

        if ( obj->carried_by )
            oprog_random_trigger( obj );
        else if ( obj->in_room && obj->in_room->area->nplayer > 0 )
            oprog_random_trigger( obj );

        if ( obj_extracted( obj ) )
            continue;

        obj_tick( obj );

        /* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */

        if ( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC )
        {
            sh_int timerfrac = UMAX( 1, obj->timer - 1 );

            if ( obj->item_type == ITEM_CORPSE_PC )
                timerfrac = ( int )( obj->timer / 8 + 1 );

            if ( obj->timer > 0 && obj->value[2] > timerfrac )
            {
                char buf[MAX_STRING_LENGTH];
                char name[MAX_STRING_LENGTH];
                char* bufptr;
                bufptr = one_argument( obj->short_descr, name );
                bufptr = one_argument( bufptr, name );
                bufptr = one_argument( bufptr, name );
                separate_obj( obj );
                obj->value[2] = timerfrac;
                sprintf( buf, corpse_descs[ UMIN( timerfrac - 1, 4 ) ], capitalize( bufptr ) );
                STRFREE( obj->description );
                obj->description = STRALLOC( buf );
            }
        }

        /* don't let inventory decay */
        if ( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
            continue;

        if ( obj->timer > 0 && obj->timer < 5 && obj->item_type == ITEM_ARMOR )
        {
            if ( obj->carried_by )
            {
                act( AT_TEMP, "$p is almost dead.", obj->carried_by, obj, NULL, TO_CHAR );
            }
        }

        if ( ( obj->timer <= 0 || --obj->timer > 0 ) )
            continue;

        /* if we get this far, object's timer has expired. */
        AT_TEMP = AT_PLAIN;

        switch ( obj->item_type )
        {
            default:
                message = "$p has depleted itself.";
                AT_TEMP = AT_PLAIN;
                break;

            case ITEM_FLARE:
                message = "$p fizzles, crackles and burns out.";
                AT_TEMP = AT_RED;
                break;

            case ITEM_AMMO:
                message = "";
                AT_TEMP = -1;

            case ITEM_GRENADE:
                // explode( obj );
                return;
                break;

            case ITEM_FOUNTAIN:
                message = "$p dries up.";
                AT_TEMP = AT_BLUE;
                break;

            case ITEM_CORPSE_NPC:
                message = "$p decays into dust and blows away.";
                AT_TEMP = AT_OBJECT;
                break;

            case ITEM_CORPSE_PC:
                message = "$p decays into dust and is blown away...";
                AT_TEMP = AT_MAGIC;
                break;

            case ITEM_FOOD:
                message = "$p is devoured by a swarm of maggots.";
                AT_TEMP = AT_HUNGRY;
                break;

            case ITEM_BLOODSTAIN:
                message = "$p dries up into flakes and blows away.";
                AT_TEMP = AT_BLOOD;
                break;

            case ITEM_SCRAPS:
                message = "$p crumbles and decays into nothing.";
                AT_TEMP = AT_OBJECT;
                break;

            case ITEM_FIRE:
                if ( obj->in_room )
                    --obj->in_room->light;

                message = "$p burns out.";
                AT_TEMP = AT_FIRE;
        }

        if ( obj->carried_by )
        {
            if ( AT_TEMP >= 0 )
                act( AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR );
        }
        else if ( obj->in_room
                  &&      ( rch = obj->in_room->first_person ) != NULL
                  &&    !IS_OBJ_STAT( obj, ITEM_BURRIED ) )
        {
            if ( AT_TEMP >= 0 )
                act( AT_TEMP, message, rch, obj, NULL, TO_ROOM );

            if ( AT_TEMP >= 0 )
                act( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
        }

        if ( obj->serial == cur_obj )
            global_objcode = rOBJ_EXPIRED;

        extract_obj( obj );
    }

    return;
}


/*
    Function to check important stuff happening to a player
    This function should take about 5% of mud cpu time
*/
void char_check( void )
{
    CHAR_DATA* ch, *ch_next;
    EXIT_DATA* pexit;
    static int cnt = 0;
    int door, retcode;
    cnt = ( cnt + 1 ) % 2;

    for ( ch = first_char; ch; ch = ch_next )
    {
        set_cur_char( ch );
        ch_next = ch->next;
        will_fall( ch, 0 );

        if ( char_died( ch ) )
            continue;

        if ( char_died( ch ) )
            continue;

        /* Update the follower routine */
        if ( ch->master != NULL )
            follow_victim( ch );

        if ( ch->fbonus > 0 )    /* Leveling bonus for combat */
            ch->fbonus--;

        update_pos( ch );

        if ( IS_NPC( ch ) )
        {
            if ( cnt != 0 )
                continue;

            /* running mobs     -Thoric */
            if ( xIS_SET( ch->act, ACT_RUNNING ) )
            {
                if ( !xIS_SET( ch->act, ACT_SENTINEL ) && ch->hunting )
                {
                    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
                    hunt_victim( ch );
                    continue;
                }

                if ( ch->spec_fun )
                {
                    if ( ( *ch->spec_fun ) ( ch ) )
                        continue;

                    if ( char_died( ch ) )
                        continue;
                }

                if ( ch->spec_2 )
                {
                    if ( ( *ch->spec_2 ) ( ch ) )
                        continue;

                    if ( char_died( ch ) )
                        continue;
                }

                if ( !xIS_SET( ch->act, ACT_SENTINEL )
                        &&   !xIS_SET( ch->act, ACT_PROTOTYPE )
                        && ( door = number_bits( 4 ) ) <= 9
                        && ( pexit = get_exit( ch->in_room, door ) ) != NULL
                        &&   pexit->to_room
                        &&   !xIS_SET( pexit->exit_info, EX_CLOSED )
                        &&   !xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB )
                        && ( !xIS_SET( ch->act, ACT_STAY_AREA )
                             ||   pexit->to_room->area == ch->in_room->area ) )
                {
                    retcode = move_char( ch, pexit, pexit->vdir, 0 );

                    if ( char_died( ch ) )
                        continue;

                    if ( retcode != rNONE || xIS_SET( ch->act, ACT_SENTINEL )
                            ||    ch->position < POS_STANDING )
                        continue;
                }
            }

            continue;
        }
        else
        {
            if ( ch->mount
                    &&   ch->in_room != ch->mount->in_room )
            {
                xREMOVE_BIT( ch->mount->act, ACT_MOUNTED );
                ch->mount = NULL;
                ch->position = POS_STANDING;
                send_to_char( "No longer upon your mount, you fall to the ground...\n\rOUCH!\n\r", ch );
            }

            if ( ( ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER )
                    || ( ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR ) )
            {
                if ( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) )
                {
                    if ( get_trust( ch ) < LEVEL_IMMORTAL )
                    {
                        int dam;
                        dam = number_range( ch->max_hit / 50, ch->max_hit / 30 );
                        dam = UMAX( 1, dam );

                        if (  ch->hit <= 0 )
                            dam = UMIN( 10, dam );

                        if ( number_bits( 3 ) == 0 )
                            send_to_char( "You cough and choke as you try to breathe water!\n\r", ch );

                        damage( ch, ch, dam, TYPE_UNDEFINED );
                    }
                }
            }

            if ( char_died( ch ) )
                continue;

            if ( ch->in_room
                    && ( ( ch->in_room->sector_type == SECT_WATER_NOSWIM )
                         ||  ( ch->in_room->sector_type == SECT_WATER_SWIM ) ) )
            {
                if ( !IS_AFFECTED( ch, AFF_FLYING )
                        && !IS_AFFECTED( ch, AFF_FLOATING )
                        && !IS_AFFECTED( ch, AFF_AQUA_BREATH )
                        && !ch->mount )
                {
                    if ( get_trust( ch ) < LEVEL_IMMORTAL )
                    {
                        int dam;

                        if ( ch->move > 0 )
                            ch->move--;
                        else
                        {
                            dam = number_range( ch->max_hit / 50, ch->max_hit / 30 );
                            dam = UMAX( 1, dam );

                            if (  ch->hit <= 0 )
                                dam = UMIN( 10, dam );

                            if ( number_bits( 3 ) == 0 )
                                send_to_char( "Struggling with exhaustion, you choke on a mouthful of water.\n\r", ch );

                            damage( ch, ch, dam, TYPE_UNDEFINED );
                        }
                    }
                }
            }
        }
    }
}


/*
    Aggress.

    for each descriptor
       for each mob in room
           aggress on some random PC

    This function should take 5% to 10% of ALL mud cpu time.
    Unfortunately, checking on each PC move is too tricky,
     because we don't the mob to just attack the first PC
     who leads the party into the room.

*/
void aggr_update( void )
{
    DESCRIPTOR_DATA* d, *dnext;
    CHAR_DATA* wch;
    CHAR_DATA* ch;
    CHAR_DATA* ch_next;
    CHAR_DATA* victim;
    struct act_prog_data* apdtmp;
#ifdef UNDEFD

    /*
        GRUNT!  To do

    */
    if ( IS_NPC( wch ) && wch->mpactnum > 0
            && wch->in_room->area->nplayer > 0 )
    {
        MPROG_ACT_LIST* tmp_act, *tmp2_act;

        for ( tmp_act = wch->mpact; tmp_act;
                tmp_act = tmp_act->next )
        {
            oprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch,
                                  tmp_act->obj, tmp_act->vo, ACT_PROG );
            DISPOSE( tmp_act->buf );
        }

        for ( tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act )
        {
            tmp2_act = tmp_act->next;
            DISPOSE( tmp_act );
        }

        wch->mpactnum = 0;
        wch->mpact    = NULL;
    }

#endif

    /* check mobprog act queue */
    while ( ( apdtmp = mob_act_list ) != NULL )
    {
        wch = mob_act_list->vo;

        if ( !char_died( wch ) && wch->mpactnum > 0 )
        {
            MPROG_ACT_LIST* tmp_act;

            while ( ( tmp_act = wch->mpact ) != NULL )
            {
                if ( tmp_act->obj && obj_extracted( tmp_act->obj ) )
                    tmp_act->obj = NULL;

                if ( tmp_act->ch && !char_died( tmp_act->ch ) )
                    mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch,
                                          tmp_act->obj, tmp_act->vo, ACT_PROG );

                wch->mpact = tmp_act->next;
                DISPOSE( tmp_act->buf );
                DISPOSE( tmp_act );
            }

            wch->mpactnum = 0;
            wch->mpact    = NULL;
        }

        mob_act_list = apdtmp->next;
        DISPOSE( apdtmp );
    }

    /*
        Just check descriptors here for victims to aggressive mobs
        We can check for linkdead victims to mobile_update   -Thoric
    */
    for ( d = first_descriptor; d; d = dnext )
    {
        dnext = d->next;

        if ( d->connected != CON_PLAYING || ( wch = d->character ) == NULL )
            continue;

        if ( char_died( wch )
                ||   IS_NPC( wch )
                ||   wch->top_level >= LEVEL_IMMORTAL
                ||  !wch->in_room )
            continue;

        for ( ch = wch->in_room->first_person; ch; ch = ch_next )
        {
            int count;
            ch_next = ch->next_in_room;

            if ( !IS_NPC( ch )
                    ||   IS_AFFECTED( ch, AFF_CHARM )
                    ||   !IS_AWAKE( ch )
                    ||   !can_see( ch, wch ) )
                continue;

            if ( is_hating( ch, wch ) )
            {
                found_prey( ch, wch );
                continue;
            }

            /* Bypass check */
            continue;

            if ( !xIS_SET( ch->act, ACT_AGGRESSIVE ) || xIS_SET( ch->act, ACT_MOUNTED ) )
                continue;

            victim = wch;

            if ( !victim )
            {
                bug( "Aggr_update: null victim.", count );
                continue;
            }

            if ( get_timer( victim, TIMER_RECENTFIGHT ) > 0 )
                continue;

            global_retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
        }
    }

    return;
}

/* From interp.c */
bool check_social  args( ( CHAR_DATA* ch, char* command, char* argument ) );

void halucinations( CHAR_DATA* ch )
{
    if ( ch->mental_state >= 30 && number_bits( 5 - ( ch->mental_state >= 50 ) - ( ch->mental_state >= 75 ) ) == 0 )
    {
        char* t;

        switch ( number_range( 1, UMIN( 20, ( ch->mental_state + 5 ) / 5 ) ) )
        {
            default:
            case  1:
                t = "You feel very restless... you can't sit still.\n\r";
                break;

            case  2:
                t = "You're tingling all over.\n\r";
                break;

            case  3:
                t = "Your skin is crawling.\n\r";
                break;

            case  4:
                t = "You suddenly feel that something is terribly wrong.\n\r";
                break;

            case  5:
                t = "Those damn little fairies keep laughing at you!\n\r";
                break;

            case  6:
                t = "You can hear your mother crying...\n\r";
                break;

            case  7:
                t = "Have you been here before, or not?  You're not sure...\n\r";
                break;

            case  8:
                t = "Painful childhood memories flash through your mind.\n\r";
                break;

            case  9:
                t = "You hear someone call your name in the distance...\n\r";
                break;

            case 10:
                t = "Your head is pulsating... you can't think straight.\n\r";
                break;

            case 11:
                t = "The ground... seems to be squirming...\n\r";
                break;

            case 12:
                t = "You're not quite sure what is real anymore.\n\r";
                break;

            case 13:
                t = "It's all a dream... or is it?\n\r";
                break;

            case 14:
                t = "They're coming to get you... coming to take you away...\n\r";
                break;

            case 15:
                t = "You begin to feel all powerful!\n\r";
                break;

            case 16:
                t = "You're light as air... the heavens are yours for the taking.\n\r";
                break;

            case 17:
                t = "Your whole life flashes by... and your future...\n\r";
                break;

            case 18:
                t = "You are everywhere and everything... you know all and are all!\n\r";
                break;

            case 19:
                t = "You feel immortal!\n\r";
                break;

            case 20:
                t = "Ahh... the power of a Supreme Entity... what to do...\n\r";
                break;
        }

        send_to_char( t, ch );
    }

    return;
}

void tele_update( void )
{
    TELEPORT_DATA* tele, *tele_next;

    if ( !first_teleport )
        return;

    for ( tele = first_teleport; tele; tele = tele_next )
    {
        tele_next = tele->next;

        if ( --tele->timer <= 0 )
        {
            if ( tele->room->first_person )
            {
                teleport( tele->room->first_person, tele->room->tele_vnum,
                          TELE_TRANSALL );
            }

            UNLINK( tele, first_teleport, last_teleport, next, prev );
            DISPOSE( tele );
        }
    }
}

#if FALSE
/*
    Write all outstanding authorization requests to Log channel - Gorog
*/
void auth_update( void )
{
    CHAR_DATA* victim;
    DESCRIPTOR_DATA* d;
    char log_buf [MAX_INPUT_LENGTH];
    bool first_time = TRUE;         /* so titles are only done once */

    for ( d = first_descriptor; d; d = d->next )
    {
        victim = d->character;

        if ( victim && IS_WAITING_FOR_AUTH( victim ) )
        {
            if ( first_time )
            {
                first_time = FALSE;
                strcpy ( log_buf, "Pending authorizations:" );
                to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 1 );
            }

            sprintf( log_buf, " %s@%s new %s", victim->name,
                     victim->desc->host, race_table[victim->race].race_name );
            to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 1 );
        }
    }
}
#endif

void auth_update( void )
{
    CHAR_DATA* victim;
    DESCRIPTOR_DATA* d;
    char buf [MAX_INPUT_LENGTH], log_buf [MAX_INPUT_LENGTH];
    bool found_hit = FALSE;         /* was at least one found? */
    strcpy ( log_buf, "Pending authorizations:\n\r" );

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( ( victim = d->character ) && IS_WAITING_FOR_AUTH( victim ) )
        {
            found_hit = TRUE;
            sprintf( buf, " %s@%s new %s\n\r", victim->name,
                     victim->desc->host, race_table[victim->race].race_name );
            strcat ( log_buf, buf );
        }
    }

    if ( found_hit )
    {
        log_string( log_buf );
        to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 1 );
    }
}

/*
    Handle all kinds of updates.
    Called once per pulse from game loop.
    Random times to defeat tick-timing clients and players.
*/
void update_handler( void )
{
    static  int     pulse_bot;
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_single;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int     pulse_second;
    static  int     pulse_recharge;
    static  int     pulse_space;
    static  int     pulse_xnames;
    static  int     pulse_save;
    static  int     pulse_web;
    static  int     pulse_logalert;
    static  int     pulse_emergency;
    static  int     pulse_bacta;
    static  int     pulse_ooc;
    static  int     pulse_order;
    static  int     pulse_pulse;
    struct timeval stime;
    struct timeval etime;

    if ( timechar )
    {
        set_char_color( AT_PLAIN, timechar );
        send_to_char( "Starting update timer.\n\r", timechar );
        gettimeofday( &stime, NULL );
    }

    if ( --pulse_area     <= 0 )
    {
        pulse_area  = number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
        area_update     ( );
    }

    if ( --pulse_logalert  <= 0 )
    {
        pulse_logalert  = PULSE_LOGALERT;
        update_logalerts ( );
    }

    if ( --pulse_web       <= 0 )
    {
        pulse_web   = ( 240 * PULSE_PER_SECOND );
        HTML_Who   ( );
    }

    if ( --pulse_xnames   <= 0 )
    {
        pulse_xnames = PULSE_XNAME;
        update_xnames ( );
    }

    if ( --pulse_save <= 0 )
    {
        pulse_save = PULSE_SAVE;
        autosave();
    }

    if ( --pulse_bot   <= 0 )
    {
        pulse_bot    = PULSE_BOT;
        bot_update  ( );
    }

    if ( --pulse_mobile   <= 0 )
    {
        pulse_mobile    = PULSE_MOBILE;
        mobile_update  ( );
    }

    if ( --pulse_bacta   <= 0 )
    {
        pulse_bacta    = PULSE_BACTA;
        bacta_update  ( );
    }

    if ( --pulse_space   <= 0 )
    {
        pulse_space    = PULSE_SPACE;
        update_movement ( );
    }

    if ( pulse_emergency == 0 )
    {
        pulse_emergency    = PULSE_EMERGENCY;
    }
    else if ( --pulse_emergency <= 0 )
    {
        pulse_emergency = PULSE_EMERGENCY;
        /* Arm emergency Copyover mode */
        emergency_arm( );
    }

    if ( pulse_order == 0 )
    {
        pulse_order   = PULSE_PER_SECOND * 60;
    }
    else if ( --pulse_order <= 0 )
    {
        pulse_order   = PULSE_PER_SECOND * 60;
        update_arena();
    }

    if ( --pulse_recharge <= 0 )
    {
        pulse_recharge = PULSE_SPACE / 3;
    }

    if ( --pulse_single <= 0 )
    {
        pulse_single = PULSE_SINGLE;
        update_respawn ( );
        update_backup ( );
    }

    if ( --pulse_violence <= 0 )
    {
        pulse_violence  = PULSE_VIOLENCE;
        update_object_pulse ( );
        violence_update ( );
    }

    if ( --pulse_ooc <= 0 )
    {
        pulse_ooc = ( PULSE_PER_SECOND * 10 );
        update_ooc( );
    }

    if ( --pulse_pulse <= 0 )
    {
        pulse_pulse = PULSE_PER_SECOND * 15;
        pulse_update    (  );
    }

    if ( --pulse_point    <= 0 )
    {
        pulse_point     = PULSE_TICK;
        auth_update     ( );            /* Gorog */
        weather_update  ( );
        char_update ( );
        obj_update      ( );
        clear_vrooms    ( );                    /* remove virtual rooms */
    }

    if ( --pulse_second   <= 0 )
    {
        pulse_second    = PULSE_PER_SECOND;
        char_check      ( );
        check_dns       ( );
        update_sentry   ( );
        update_votes    ( );
        check_pfiles    ( 0 );
        reboot_check    ( 0 );
    }

    mpsleep_update();  /* MPSLEEP Driver */
    tele_update( );
    aggr_update( );
    obj_act_update ( );
    room_act_update( );
    clean_obj_queue();      /* dispose of extracted objects */
    clean_char_queue();     /* dispose of dead mobs/quitting chars */
    mqtt_update( 100 );     /* limit to 100ms for now */

    if ( timechar )
    {
        gettimeofday( &etime, NULL );
        set_char_color( AT_PLAIN, timechar );
        send_to_char( "Update timing complete.\n\r", timechar );
        subtract_times( &etime, &stime );
        ch_printf( timechar, "Timing took %d.%06d seconds.\n\r",
                   etime.tv_sec, etime.tv_usec );
        timechar = NULL;
    }

    tail_chain( );
    return;
}


void remove_portal( OBJ_DATA* portal )
{
    ROOM_INDEX_DATA* fromRoom, *toRoom;
    CHAR_DATA* ch;
    EXIT_DATA* pexit;
    bool found;

    if ( !portal )
    {
        bug( "remove_portal: portal is NULL", 0 );
        return;
    }

    fromRoom = portal->in_room;
    found = FALSE;

    if ( !fromRoom )
    {
        bug( "remove_portal: portal->in_room is NULL", 0 );
        return;
    }

    for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
        if ( xIS_SET( pexit->exit_info, EX_PORTAL ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
    {
        bug( "remove_portal: portal not found in room %d!", fromRoom->vnum );
        return;
    }

    if ( pexit->vdir != DIR_PORTAL )
        bug( "remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir );

    if ( ( toRoom = pexit->to_room ) == NULL )
        bug( "remove_portal: toRoom is NULL", 0 );

    extract_exit( fromRoom, pexit );
    /* rendunancy */
    /* send a message to fromRoom */
    /* ch = fromRoom->first_person; */
    /* if(ch!=NULL) */
    /* act( AT_PLAIN, "A magical portal below winks from existence.", ch, NULL, NULL, TO_ROOM ); */

    /* send a message to toRoom */
    if ( toRoom && ( ch = toRoom->first_person ) != NULL )
        act( AT_PLAIN, "A magical portal above winks from existence.", ch, NULL, NULL, TO_ROOM );

    /* remove the portal obj: looks better to let update_obj do this */
    /* extract_obj(portal);  */
    return;
}

void update_object_pulse ( void )
{
    ROOM_INDEX_DATA* room;
    OBJ_DATA* obj, *oprev;

    for ( obj = last_object; obj; obj = oprev )
    {
        if ( obj == first_object && obj->prev )
        {
            bug( "update_object_pulse: first_obj->prev != NULL... fixed", 0 );
            obj->prev = NULL;
        }

        oprev = obj->prev;

        if ( oprev && oprev->next != obj )
        {
            bug( "update_object_pulse: obj->prev->next != obj", 0 );
            return;
        }

        separate_obj( obj );

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            continue;

        if ( obj->item_type == ITEM_GRENADE )
        {
            if ( obj->value[4] <= 0 )
                continue;

            if ( --obj->value[4] <= 0 )
                explode( obj );

            continue;
        }
        else if ( obj->item_type == ITEM_SIFT )
        {
            obj->value[0] = UMIN( obj->value[0] + 1, obj->value[1] );
            continue;
        }
        else if ( obj->item_type == ITEM_MEDICOMP )
        {
            obj->value[0] = UMIN( obj->value[0] + 1, obj->value[1] );
            continue;
        }
        else if ( obj->item_type == ITEM_REGENERATOR )
        {
            CHAR_DATA* rch;
            int i;
            obj->value[0] = UMIN( obj->value[1], obj->value[0] + obj->value[5] );

            if ( !obj->in_room )
                continue;

            for ( rch = obj->in_room->first_person; rch; rch = rch->next_in_room )
            {
                if ( is_spectator( rch ) || IN_VENT( rch ) )
                    continue;

                if ( !IS_OBJ_STAT( obj, ITEM_ALIEN ) && rch->race == RACE_ALIEN )
                    continue;

                if ( !IS_OBJ_STAT( obj, ITEM_MARINE ) && rch->race == RACE_MARINE )
                    continue;

                if ( !IS_OBJ_STAT( obj, ITEM_PREDATOR ) && rch->race == RACE_PREDATOR )
                    continue;

                rch->hit = URANGE( 0, rch->hit + obj->value[2], rch->max_hit );

                if ( rch->race == RACE_ALIEN )
                    rch->resin = URANGE( 0, rch->resin + obj->value[3], get_max_resin( rch ) );

                if ( rch->race == RACE_MARINE )
                    rch->morale = URANGE( 0, rch->morale + obj->value[3], get_max_morale( rch ) );

                if ( rch->race == RACE_PREDATOR )
                    rch->field = URANGE( 0, rch->field + obj->value[3], rch->max_field );

                for ( i = 0; i < top_sn; i++ )
                    if ( rch->pcdata )
                        if ( rch->pcdata->learned[i] > 0 )
                            if ( rch->pcdata->prepared[i] < skill_table[i]->reset )
                                rch->pcdata->prepared[i] += obj->value[4];

                update_pos( rch );
            }

            continue;
        }
        else if ( obj->item_type == ITEM_MOTIONB )
        {
            if ( obj->value[0] <= 0 )
                continue;

            if ( --obj->value[1] <= 0 )
            {
                // Generate the Ping
                obj->value[1] = obj->value[0];

                if ( ( room = get_obj_room( obj ) ) == NULL )
                    continue;

                if ( room->area )
                    motion_ping( room->x, room->y, room->z, room->area, NULL );

                if ( obj->value[2] > 0 )
                {
                    if ( --obj->value[2] <= 0 )
                        extract_obj( obj );
                }
            }

            continue;
        }
        else if ( obj->item_type == ITEM_MSPAWNER )
        {
            if ( obj->value[1] <= 0 )
                continue;

            if ( --obj->value[0] <= 0 )
            {
                CHAR_DATA* mob;
                int i = 0;
                obj->value[0] = obj->value[1];

                if ( ( room = get_obj_room( obj ) ) == NULL )
                    continue;

                if ( !room->area )
                    continue;

                for ( i = 0; i < obj->value[3]; ++i )
                {
                    if ( get_mob_index( obj->value[2] ) == NULL )
                        continue;

                    mob = create_mobile( get_mob_index( obj->value[2] ) );
                    char_to_room( mob, room );
                    act( AT_IMMORT, "$N has arrived.", mob, NULL, mob, TO_ROOM );
                    // if ( obj->parent ) mob->master = obj->parent;
                    motion_ping( room->x, room->y, room->z, room->area, NULL );

                    if ( obj->value[4] > 0 )
                    {
                        if ( --obj->value[4] <= 0 )
                        {
                            extract_obj( obj );
                            break;
                        }
                    }
                }
            }

            continue;
        }
    }

    return;
}

void update_backup ( void )
{
    CHAR_DATA* ch;

    for ( ch = last_char; ch; ch = gch_prev )
    {
        if ( ch == first_char && ch->prev )
        {
            bug( "char_update: first_char->prev != NULL... fixed", 0 );
            ch->prev = NULL;
        }

        gch_prev = ch->prev;
        set_cur_char( ch );

        if ( gch_prev && gch_prev->next != ch )
        {
            bug( "update_backup: ch->prev->next != ch", 0 );
            return;
        }

        /*
            Update Carrying
        */
        if ( ch->carrying != NULL )
        {
            if ( ch->carrying->in_room != ch->in_room )
            {
                char_from_room( ch->carrying );
                char_to_room( ch->carrying, ch->in_room );
            }

            if ( IS_AWAKE( ch->carrying ) )
            {
                act( AT_LBLUE, "$N releases you, dropping you on the ground.", ch->carrying, NULL, ch, TO_CHAR );
                act( AT_LBLUE, "You release $N, dropping $M on the ground.", ch, NULL, ch->carrying, TO_CHAR );
                act( AT_LBLUE, "$n releases $N, dropping $M on the ground..", ch, NULL, ch->carrying, TO_NOTVICT );
                ch->carrying->carried = NULL;
                ch->carrying = NULL;
            }
        }

        if ( ch->carried != NULL )
        {
            if ( ch->carried->carrying == NULL )
            {
                bug( "[ERROR]: Fault in carrying module. Dropping player." );
                ch->carried = NULL;
            }
        }
    }

    return;
}

void update_respawn ( void )
{
    CHAR_DATA* ch;

    for ( ch = last_char; ch; ch = ch->prev )
    {
        if ( !ch->pcdata )
            continue;

        if ( xIS_SET( ch->act, PLR_FREEZE ) )
            continue;

        if ( ch->pcdata->respawn > 0 || ch->pcdata->spectator )
        {
            clear_effects( ch );

            if ( --ch->pcdata->respawn <= 0 )
            {
                /*
                    Trigger respawn
                */
                ch->vent = FALSE;
                char_from_room( ch );
                char_to_room( ch, get_room_index( wherehome( ch ) ) );
                ch->pcdata->respawn = 0;
                ch->pcdata->spectator = FALSE;
                do_look( ch, "auto" );
                ch_printf( ch, "\n\r&z(&CMonitor&z) &WRespawning complete. You may resume playing regularly.\n\r" );
                act( AT_ACTION, "$n suddenly appears.", ch, NULL, NULL, TO_ROOM );
            }
            else if ( ch->pcdata->respawn == 10 )
            {
                ch_printf( ch, "&z(&CMonitor&z) &WWarning - 10 seconds till respawn. Get ready.\n\r" );
            }
        }
    }

    return;
}

void reboot_check( time_t reset )
{
    static char* tmsg[] =
    {
        "SYSTEM: Reboot in 1 seconds.",
        "SYSTEM: Reboot in 2 seconds.",
        "SYSTEM: Reboot in 3 seconds.",
        "SYSTEM: Reboot in 4 seconds.",
        "SYSTEM: Reboot in 5 seconds.",
        "SYSTEM: Reboot in 10 seconds.",
        "SYSTEM: Reboot in 30 seconds.",
        "SYSTEM: Reboot in 1 minute.",
        "SYSTEM: Reboot in 2 minutes.",
        "SYSTEM: Reboot in 3 minutes.",
        "SYSTEM: Reboot in 4 minutes.",
        "SYSTEM: Reboot in 5 minutes.",
        "SYSTEM: Reboot in 10 minutes.",
    };
    static const int times[] = { 1, 2, 3, 4, 5, 10, 30, 60,
                                 120, 180, 240, 300, 600
                               };
    static const int timesize =
        UMIN( sizeof( times ) / sizeof( *times ), sizeof( tmsg ) / sizeof( *tmsg ) );
    char buf[MAX_STRING_LENGTH];
    static int trun;
    static bool init;

    if ( !init || reset >= current_time )
    {
        for ( trun = timesize - 1; trun >= 0; trun-- )
            if ( reset >= current_time + times[trun] )
                break;

        init = TRUE;
        return;
    }

    if ( ( current_time % 1800 ) == 0 )
    {
        sprintf( buf, "%.24s: %d players", ctime( &current_time ), num_descriptors );
        append_to_file( USAGE_FILE, buf );
    }

    if ( new_boot_time_t - boot_time < 60 * 60 * 18 &&
            !set_boot_time->manual )
        return;

    if ( new_boot_time_t <= current_time )
    {
        CHAR_DATA* vch;
        extern bool mud_down;
        echo_to_all( AT_RED, "*** Reboot by Diablo ***", ECHOTAR_ALL );

        for ( vch = first_char; vch; vch = vch->next )
            if ( !IS_NPC( vch ) )
                save_char_obj( vch );

        mud_down = TRUE;
        return;
    }

    if ( trun != -1 && new_boot_time_t - current_time <= times[trun] )
    {
        echo_to_all( AT_YELLOW, tmsg[trun], ECHOTAR_ALL );

        if ( trun <= 5 )
            sysdata.DENY_NEW_PLAYERS = TRUE;

        --trun;
        return;
    }

    return;
}

#if 0
void reboot_check( char* arg )
{
    char buf[MAX_STRING_LENGTH];
    extern bool mud_down;
    /*  struct tm *timestruct;
        int timecheck;*/
    CHAR_DATA* vch;
    /*Bools to show which pre-boot echoes we've done. */
    static bool thirty  = FALSE;
    static bool fifteen = FALSE;
    static bool ten     = FALSE;
    static bool five    = FALSE;
    static bool four    = FALSE;
    static bool three   = FALSE;
    static bool two     = FALSE;
    static bool one     = FALSE;

    /*  This function can be called by do_setboot when the reboot time
        is being manually set to reset all the bools. */
    if ( !str_cmp( arg, "reset" ) )
    {
        thirty  = FALSE;
        fifteen = FALSE;
        ten     = FALSE;
        five    = FALSE;
        four    = FALSE;
        three   = FALSE;
        two     = FALSE;
        one     = FALSE;
        return;
    }

    /*  If the mud has been up less than 18 hours and the boot time
        wasn't set manually, forget it. */
    /* Usage monitor */

    if ( ( current_time % 1800 ) == 0 )
    {
        sprintf( buf, "%s: %d players", ctime( &current_time ), num_descriptors );
        append_to_file( USAGE_FILE, buf );
    }

    /*  Change by Scryn - if mud has not been up 18 hours at boot time - still
        allow for warnings even if not up 18 hours
    */
    if ( new_boot_time_t - boot_time < 60 * 60 * 18
            && set_boot_time->manual == 0 )
    {
        return;
    }

    /*
        timestruct = localtime( &current_time);

        if ( timestruct->tm_hour == set_boot_time->hour
             && timestruct->tm_min  == set_boot_time->min )*/
    if ( new_boot_time_t <= current_time )
    {
        sprintf( buf, "You are forced from these realms by a strong magical presence" );
        echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
        sprintf( buf, "as life here is reconstructed." );
        echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );

        /* Save all characters before booting. */
        for ( vch = first_char; vch; vch = vch->next )
        {
            if ( !IS_NPC( vch ) )
                save_char_obj( vch );
        }

        mud_down = TRUE;
    }

    /* How many minutes to the scheduled boot? */
    /*  timecheck = ( set_boot_time->hour * 60 + set_boot_time->min )
                  - ( timestruct->tm_hour * 60 + timestruct->tm_min );

        if ( timecheck > 30  || timecheck < 0 ) return;

        if ( timecheck <= 1 ) */
    if ( new_boot_time_t - current_time <= 60 )
    {
        if ( one == FALSE )
        {
            sprintf( buf, "You feel the ground shake as the end comes near!" );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            one = TRUE;
            sysdata.DENY_NEW_PLAYERS = TRUE;
        }

        return;
    }

    /*  if ( timecheck == 2 )*/
    if ( new_boot_time_t - current_time <= 120 )
    {
        if ( two == FALSE )
        {
            sprintf( buf, "Lightning crackles in the sky above!" );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            two = TRUE;
            sysdata.DENY_NEW_PLAYERS = TRUE;
        }

        return;
    }

    /*  if ( timecheck == 3 )*/
    if ( new_boot_time_t - current_time <= 180 )
    {
        if ( three == FALSE )
        {
            sprintf( buf, "Crashes of thunder sound across the land!" );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            three = TRUE;
            sysdata.DENY_NEW_PLAYERS = TRUE;
        }

        return;
    }

    /*  if ( timecheck == 4 )*/
    if ( new_boot_time_t - current_time <= 240 )
    {
        if ( four == FALSE )
        {
            sprintf( buf, "The sky has suddenly turned midnight black." );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            four = TRUE;
            sysdata.DENY_NEW_PLAYERS = TRUE;
        }

        return;
    }

    /*  if ( timecheck == 5 )*/
    if ( new_boot_time_t - current_time <= 300 )
    {
        if ( five == FALSE )
        {
            sprintf( buf, "You notice the life forms around you slowly dwindling away." );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            five = TRUE;
            sysdata.DENY_NEW_PLAYERS = TRUE;
        }

        return;
    }

    /*  if ( timecheck == 10 )*/
    if ( new_boot_time_t - current_time <= 600 )
    {
        if ( ten == FALSE )
        {
            sprintf( buf, "The seas across the realm have turned frigid." );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            ten = TRUE;
        }

        return;
    }

    /*  if ( timecheck == 15 )*/
    if ( new_boot_time_t - current_time <= 900 )
    {
        if ( fifteen == FALSE )
        {
            sprintf( buf, "The aura of magic which once surrounded the realms seems slightly unstable." );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            fifteen = TRUE;
        }

        return;
    }

    /*  if ( timecheck == 30 )*/
    if ( new_boot_time_t - current_time <= 1800 )
    {
        if ( thirty == FALSE )
        {
            sprintf( buf, "You sense a change in the magical forces surrounding you." );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            thirty = TRUE;
        }

        return;
    }

    return;
}
#endif

void subtract_times( struct timeval* etime, struct timeval* stime )
{
    etime->tv_sec -= stime->tv_sec;
    etime->tv_usec -= stime->tv_usec;

    while ( etime->tv_usec < 0 )
    {
        etime->tv_usec += 1000000;
        etime->tv_sec--;
    }

    return;
}

void update_xnames( void )
{
    struct xname_data* xname, *x2;
    x2 = NULL;

    for ( xname = xnames; xname; xname = xname->next )
    {
        if ( xname->time >= time( 0 ) )
        {
            if ( xname->next )
                if ( x2 )
                    x2->next = xname->next;
                else
                    xname->next = NULL;
            else if ( x2 )
                x2->next = NULL;
        }

        x2 = xname;
    }
}

/*
    Detects if certain 'problem' files are exceeding a set limit.
    Sends a warning to all immortals when this occurs.

    If files exceed their 'failsafe' limit they are either
    taken offline or wiped clean.
*/
void update_logalerts( void )
{
    char buf[MAX_STRING_LENGTH];
    int size = 0;
    /*
        Update for bugs.txt
         Limit: 5 megabytes (5000000 bytes)
         Failsafe: 10 megabytes (Clears bug file)
    */
    sprintf( buf, "%s%s", SYSTEM_DIR, BUG_FILE );

    if ( ( size = file_size( buf ) ) > 10000000 )
    {
        /* Execute failsafe */
        sprintf( buf, "[ALERT]: Bugs.txt exceeds 10 megabyte failsafe! File will be deleted. (%d bytes)", size );
        log_string( buf );

        if ( !remove( BUG_FILE ) )
            log_string( "[ALERT]: Bugs.txt successful cleared. File is now empty." );
        else if ( errno != ENOENT )
            log_string( "[ALERT]: Bugs.txt could not be cleared. Contact Ghost or Raven." );
    }
    else if ( size > 5000000 )
    {
        sprintf( buf, "[WARNING]: Bugs.txt exceeds maximum file limit. (%d bytes)", size );
        log_string( buf );
    }

    /*
        Update for log.txt
         Limit: 2.5 megabytes (25000000 bytes)
         Failsafe: 5 megabytes (Clears log file)
    */
    sprintf( buf, "%s%s", SYSTEM_DIR, LOG_FILE );

    if ( ( size = file_size( buf ) ) > 5000000 )
    {
        /* Execute failsafe */
        sprintf( buf, "[ALERT]: Log.txt exceeds 5 megabyte failsafe! File will be deleted. (%d bytes)", size );
        log_string( buf );

        if ( !remove( LOG_FILE ) )
            log_string( "[ALERT]: Log.txt successful cleared. File is now empty." );
        else if ( errno != ENOENT )
            log_string( "[ALERT]: Log.txt could not be cleared. Contact Ghost or Raven." );
    }
    else if ( size > 2500000 )
    {
        sprintf( buf, "[WARNING]: Log.txt exceeds maximum file limit. (%d bytes)", size );
        log_string( buf );
    }

    return;
}

/*
    Movement Updating (Ghost)
    Used to update a character's movement more often.
*/
void update_movement()
{
    CHAR_DATA* ch;

    for ( ch = last_char; ch; ch = gch_prev )
    {
        if ( ch == first_char && ch->prev )
            ch->prev = NULL;

        gch_prev = ch->prev;
        set_cur_char( ch );

        if ( gch_prev && gch_prev->next != ch )
        {
            bug( "update_movement: ch->prev->next != ch", 0 );
            return;
        }

        if ( ch->morale < get_max_morale( ch ) )
            ch->morale = URANGE( 0, ch->morale + ( get_curr_bra( ch ) / 10 ), get_max_morale( ch ) );

        if ( ch->move < ch->max_move && ch->max_move > 0 )
        {
            if ( IS_NPC( ch ) )
            {
                ch->move += move_gain( ch );
            }
            else
            {
                // a = ( 100 * ch->move ) / ch->max_move;
                ch->move += move_gain( ch );
                // b = ( 100 * ch->move ) / ch->max_move;
                // if ( a != b ) send_to_char( " ", ch );
            }
        }
    }

    return;
}


/*
    Bacta Tank Updates (Coded by Ghost)
    Now also repairs POISON, BLINDING, and PARALYSIS.
*/
void bacta_update()
{
    CHAR_DATA* ch;
    AFFECT_DATA* paf;
    char buf[MAX_STRING_LENGTH];

    for ( ch = last_char; ch; ch = gch_prev )
    {
        if ( ch == first_char && ch->prev )
        {
            bug( "bacta_update: first_char->prev != NULL... fixed", 0 );
            ch->prev = NULL;
        }

        gch_prev = ch->prev;
        set_cur_char( ch );

        if ( gch_prev && gch_prev->next != ch )
        {
            bug( "bacta_update: ch->prev->next != ch", 0 );
            return;
        }

        if ( !IS_NPC( ch ) )
        {
            if ( xIS_SET( ch->in_room->room_flags, ROOM_BACTA ) )
            {
                if ( ch->hit < ch->max_hit )
                {
                    ch->hit = URANGE( 0, ch->hit + 100, ch->max_hit );
                    sprintf( buf, "&GYour wounds heal slightly as the bacta flows over you.\n\r" );
                    send_to_char( buf, ch );
                }

                for ( paf = ch->first_affect; paf; paf = paf->next )
                    if ( xIS_SET( paf->bitvector, AFF_POISON ) || xIS_SET( paf->bitvector, AFF_BLIND ) || xIS_SET( paf->bitvector, AFF_PARALYSIS ) )
                        paf->duration = URANGE( 1, paf->duration - 100, 10000 );
            }
        }
    }
}

/*
    OOC Limit-Break Check
*/
void update_ooc()
{
    CHAR_DATA* ch;
    char buf[MAX_STRING_LENGTH];

    for ( ch = last_char; ch; ch = gch_prev )
    {
        if ( ch == first_char && ch->prev )
        {
            bug( "update_ooc: first_char->prev != NULL... fixed", 0 );
            ch->prev = NULL;
        }

        gch_prev = ch->prev;
        set_cur_char( ch );

        if ( gch_prev && gch_prev->next != ch )
        {
            bug( "update_ooc: ch->prev->next != ch", 0 );
            return;
        }

        if ( !IS_NPC( ch ) )
        {
            if ( ch->pcdata->ooclimit < MUDH_OOC_LIMIT )
            {
                ++ch->pcdata->ooclimit;

                if ( ch->pcdata->ooclimit == MUDH_OOC_LIMIT && ch->pcdata->oocbreak )
                {
                    ch->pcdata->oocbreak = FALSE;
                    sprintf( buf, "&GOOC Limit-Break reset. You may resume using OOC.\n\r" );
                    send_to_char( buf, ch );
                }
            }
        }
    }
}

void write_serverstats( void )
{
    char strsave1[MAX_INPUT_LENGTH];
    char strsave2[MAX_INPUT_LENGTH];
    FILE* fp1;
    FILE* fp2;
    sprintf( strsave1, "../stats/server.stats" );
    sprintf( strsave2, "../stats/server.who" );

    if ( ( fp1 = fopen( strsave1, "w" ) ) == NULL )
    {
        bug( "write_serverstats: fopen 1", 0 );
        perror( strsave1 );
    }
    else if ( ( fp2 = fopen( strsave2, "w" ) ) == NULL )
    {
        bug( "write_serverstats: fopen 2", 0 );
        perror( strsave2 );
    }
    else
    {
        if ( curr_arena != NULL )
        {
            fprintf( fp1, "ArenaName  %s\n", curr_arena->name     );
            fprintf( fp1, "ArenaFName %s\n", curr_arena->filename );
            fprintf( fp1, "ArenaLeft  %d\n", curr_arena->ctimer   );
            fprintf( fp1, "ArenaTime  %d\n", curr_arena->timer    );
            fprintf( fp1, "ArenaMin   %d\n", curr_arena->min_p    );
            fprintf( fp1, "ArenaMax   %d\n", curr_arena->max_p    );
        }

        fprintf( fp1, "Players    %d\n",   num_descriptors       );
        match_log( "CONTROL;Currently %d players on.", num_descriptors );
        fclose( fp1 );
        fclose( fp2 );
    }

    return;
}

void equipment_recharge( CHAR_DATA* ch )
{
    OBJ_DATA* obj;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
    {
        recharge_drill( ch, obj );
        recharge_item( ch, obj );
    }

    return;
}

void recharge_drill( CHAR_DATA* ch, OBJ_DATA* obj )
{
    OBJ_DATA* tmp;

    if ( obj == NULL )
        return;

    if ( obj->first_content == NULL )
        return;

    for ( tmp = obj->first_content; tmp; tmp = tmp->next_content )
    {
        if ( tmp == NULL )
            break;

        recharge_drill( ch, tmp );
        recharge_item( ch, tmp );
    }

    return;
}

void recharge_item( CHAR_DATA* ch, OBJ_DATA* obj )
{
    if ( obj == NULL )
        return;

    recharge_attachment( ch, obj );

    if ( obj->item_type == ITEM_LIGHT )
    {
        /* Update lights */
        if ( !IS_SET( obj->value[3], BV00 ) )
        {
            if ( obj->value[2] < obj->value[4] )
                obj->value[2]++;
        }
        else
        {
            if ( IS_SET( obj->value[3], BV00 ) )
            {
                obj->value[2]--;

                if ( obj->value[2] == 0 )
                {
                    act( AT_ACTION, "&w&R$p has run out of power.", ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "&w&R$p has run out of power.", ch, obj, NULL, TO_CHAR );
                    REMOVE_BIT( obj->value[3], BV00 );
                    ch->in_room->light -= obj->count;
                }
                else if ( obj->value[2] == 1 )
                {
                    act( AT_ACTION, "&w&Y$p flickers and starts to dim.", ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "&w&Y$p flickers and starts to dim.", ch, obj, NULL, TO_CHAR );
                }
            }
        }
    }
    else if ( obj->item_type == ITEM_NVGOGGLE )
    {
        /* Update nightvision goggles */
        if ( !IS_SET( obj->value[3], BV00 ) )
        {
            if ( obj->value[0] < obj->value[1] )
                obj->value[0]++;
        }
        else
        {
            if ( IS_SET( obj->value[3], BV00 ) )
            {
                obj->value[0]--;

                if ( obj->value[0] == 0 )
                {
                    act( AT_ACTION, "&w&R$p has run out of power.", ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "&w&R$p has run out of power.", ch, obj, NULL, TO_CHAR );
                    REMOVE_BIT( obj->value[3], BV00 );
                }
                else if ( obj->value[0] == 1 )
                {
                    act( AT_ACTION, "&w&Y$p flickers and starts to dim.", ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "&w&Y$p flickers and starts to dim.", ch, obj, NULL, TO_CHAR );
                }
            }
        }
    }
    else if ( obj->item_type == ITEM_SCANNON )
    {
        obj->value[4] = UMAX( 0, obj->value[4] - 1 );
    }

    return;
}

void obj_tick( OBJ_DATA* obj )
{
    int i = 0;

    if ( obj == NULL )
        return;

    if ( obj->item_type == ITEM_MEDSTATION )
    {
        i = URANGE( 0, obj->value[3], obj->value[2] - obj->value[1] );
        obj->value[1] += i;
    }
    else if ( obj->item_type == ITEM_TOOLCHEST )
    {
        i = URANGE( 0, obj->value[3], obj->value[2] - obj->value[1] );
        obj->value[1] += i;
    }
    else if ( obj->item_type == ITEM_AMMOBOX )
    {
        if ( obj->value[0] == 0 )
        {
            if ( obj->value[2] < obj->value[3] )
            {
                obj->value[2]++;
            }
            else
            {
                obj->value[2] = 0;
                obj->value[0] = 1;
            }
        }
    }

    return;
}

/*
    Drives the PULSE_PROG Mobprog type.
*/
void pulse_update( void )
{
    CHAR_DATA* ch = NULL;
    CHAR_DATA* nch = NULL;

    for ( ch = last_char; ch; ch = nch )
    {
        nch = ch->prev;

        if ( !ch->in_room )
            continue;

        if ( !ch->in_room->area )
            continue;

        if ( ch->pIndexData )
        {
            if ( xIS_SET( ch->pIndexData->progtypes, PULSE_PROG ) )
                mprog_pulse_trigger( ch );

            if ( char_died( ch ) )
                continue;
        }

        if ( ch->in_room->area->nplayer > 0 )
            rprog_pulse_trigger( ch );

        if ( char_died( ch ) )
            continue;
    }

    return;
}
