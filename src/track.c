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
             Tracking/hunting module
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

#define BFS_ERROR      -1
#define BFS_ALREADY_THERE  -2
#define BFS_NO_PATH    -3
#define BFS_MARK            1

#define TRACK_THROUGH_DOORS

extern int  top_room;

bool mob_snipe( CHAR_DATA* ch, CHAR_DATA* victim );
ch_ret  one_hit             args( ( CHAR_DATA* ch, CHAR_DATA* victim, int dt ) );

/*  You can define or not define TRACK_THOUGH_DOORS, above, depending on
    whether or not you want track to find paths which lead through closed
    or hidden doors.
*/

struct bfs_queue_struct
{
    ROOM_INDEX_DATA* room;
    char   dir;
    struct bfs_queue_struct* next;
};

static struct bfs_queue_struct*  queue_head = NULL,
                                     *queue_tail = NULL,
                                      *room_queue = NULL;

/* Utility macros */
#define MARK(room)  (xSET_BIT(   (room)->room_flags, BFS_MARK) )
#define UNMARK(room)    (xREMOVE_BIT(    (room)->room_flags, BFS_MARK) )
#define IS_MARKED(room) (xIS_SET( (room)->room_flags, BFS_MARK) )

ROOM_INDEX_DATA* toroom( ROOM_INDEX_DATA* room, sh_int door )
{
    return ( get_exit( room, door )->to_room );
}

bool valid_edge( ROOM_INDEX_DATA* room, sh_int door )
{
    EXIT_DATA* pexit;
    ROOM_INDEX_DATA* to_room;
    pexit = get_exit( room, door );

    if ( pexit
            &&  ( to_room = pexit->to_room ) != NULL
#ifndef TRACK_THROUGH_DOORS
            &&  !xIS_SET( pexit->exit_info, EX_CLOSED )
#endif
            &&  !IS_MARKED( to_room ) )
        return TRUE;
    else
        return FALSE;
}

void bfs_enqueue( ROOM_INDEX_DATA* room, char dir )
{
    struct bfs_queue_struct* curr;
    CREATE( curr, struct bfs_queue_struct, 1 );
    curr->room = room;
    curr->dir = dir;
    curr->next = NULL;

    if ( queue_tail )
    {
        queue_tail->next = curr;
        queue_tail = curr;
    }
    else
        queue_head = queue_tail = curr;
}


void bfs_dequeue( void )
{
    struct bfs_queue_struct* curr;
    curr = queue_head;

    if ( !( queue_head = queue_head->next ) )
        queue_tail = NULL;

    DISPOSE( curr );
}


void bfs_clear_queue( void )
{
    while ( queue_head )
        bfs_dequeue();
}

void room_enqueue( ROOM_INDEX_DATA* room )
{
    struct bfs_queue_struct* curr;
    CREATE( curr, struct bfs_queue_struct, 1 );
    curr->room = room;
    curr->next = room_queue;
    room_queue = curr;
}

void clean_room_queue( void )
{
    struct bfs_queue_struct* curr, *curr_next;

    for ( curr = room_queue; curr; curr = curr_next )
    {
        UNMARK( curr->room );
        curr_next = curr->next;
        DISPOSE( curr );
    }

    room_queue = NULL;
}


int find_first_step( ROOM_INDEX_DATA* src, ROOM_INDEX_DATA* target, int maxdist )
{
    int curr_dir, count;

    if ( !src || !target )
    {
        bug( "Illegal value passed to find_first_step (track.c)", 0 );
        return BFS_ERROR;
    }

    if ( src == target )
        return BFS_ALREADY_THERE;

    if ( src->area != target->area )
        return BFS_NO_PATH;

    room_enqueue( src );
    MARK( src );

    /* first, enqueue the first steps, saving which direction we're going. */
    for ( curr_dir = 0; curr_dir < 10; curr_dir++ )
        if ( valid_edge( src, curr_dir ) )
        {
            MARK( toroom( src, curr_dir ) );
            room_enqueue( toroom( src, curr_dir ) );
            bfs_enqueue( toroom( src, curr_dir ), curr_dir );
        }

    count = 0;

    while ( queue_head )
    {
        if ( ++count > maxdist )
        {
            bfs_clear_queue();
            clean_room_queue();
            return BFS_NO_PATH;
        }

        if ( queue_head->room == target )
        {
            curr_dir = queue_head->dir;
            bfs_clear_queue();
            clean_room_queue();
            return curr_dir;
        }
        else
        {
            for ( curr_dir = 0; curr_dir < 10; curr_dir++ )
                if ( valid_edge( queue_head->room, curr_dir ) )
                {
                    MARK( toroom( queue_head->room, curr_dir ) );
                    room_enqueue( toroom( queue_head->room, curr_dir ) );
                    bfs_enqueue( toroom( queue_head->room, curr_dir ), queue_head->dir );
                }

            bfs_dequeue();
        }
    }

    clean_room_queue();
    return BFS_NO_PATH;
}


void do_track( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* vict;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int dir, maxdist;
    // maxdist = 30 + ( get_curr_per(ch) * 2 );
    maxdist = 0;

    if ( ch->pcdata )
    {
        if ( ch->race == RACE_PREDATOR )
        {
            maxdist = 5 + ( ch->pcdata->learned[gsn_track] * 5 );

            if ( maxdist <= 5 )
                maxdist = 0;
        }
        else if ( ch->race == RACE_ALIEN )
        {
            maxdist = ( ch->pcdata->learned[gsn_pursuit] * 10 );
        }
    }

    if ( IS_NPC( ch ) || maxdist <= 0 )
    {
        do_nothing( ch, "" );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "&RSyntax: TRACK (target)\n\r", ch );
        ch_printf( ch, "You can track up to %d rooms right now.\n\r", maxdist );
        return;
    }

    /*
        if ( room_is_dark( ch->in_room ) )
        {
        send_to_char( "Its too dark to make out the tracks of anything!\n\r", ch );
        return;
        }
    */

    if ( !( vict = get_char_world( ch, arg ) ) )
    {
        send_to_char( "You can't find a trail of anyone like that.\n\r", ch );
        return;
    }

    /* Night time */
    // if ( time_info.hour <  6 || time_info.hour > 19 ) maxdist /= 2;
    dir = find_first_step( ch->in_room, vict->in_room, maxdist );

    switch ( dir )
    {
        case BFS_ERROR:
            send_to_char( "Hmm... something seems to be wrong.\n\r", ch );
            break;

        case BFS_ALREADY_THERE:
            send_to_char( "You're already in the same room!\n\r", ch );
            break;

        case BFS_NO_PATH:
            sprintf( buf, "You can't sense a trail from here.\n\r" );
            send_to_char( buf, ch );
            break;

        default:
            ch_printf( ch, "You sense a trail %s from here...\n\r", dir_name[dir] );
            break;
    }
}

/*
    Returns TRUE if the path is cleared to use
*/
bool hail_route_check( CHAR_DATA* ch, ROOM_INDEX_DATA* toroom )
{
    int dir = 0;
    dir = find_first_step( ch->in_room, toroom, 3000 );

    switch ( dir )
    {
        case BFS_ERROR:
            return FALSE;

        case BFS_ALREADY_THERE:
            return FALSE;

        case BFS_NO_PATH:
            return FALSE;

        default:
            return TRUE;
    }

    return FALSE;
}

void found_prey( CHAR_DATA* ch, CHAR_DATA* victim )
{
    char buf[MAX_STRING_LENGTH];
    char victname[MAX_STRING_LENGTH];

    if ( victim == NULL )
    {
        bug( "Found_prey: null victim", 0 );
        return;
    }

    if ( ch == NULL )
    {
        bug( "Found_prey: null ch", 0 );
        return;
    }

    if ( victim->in_room == NULL )
    {
        bug( "Found_prey: null victim->in_room", 0 );
        return;
    }

    sprintf( victname, IS_NPC( victim ) ? victim->short_descr : victim->name );

    if ( !can_see( ch, victim ) )
    {
        return;
    }

    // multi_hit(ch, victim, TYPE_UNDEFINED);
    return;
}

void hunt_victim( CHAR_DATA* ch )
{
    bool found;
    CHAR_DATA* tmp;
    sh_int ret;

    if ( !ch || !ch->hunting || !ch->hunting->who )
        return;

    /* make sure the char still exists */
    for ( found = FALSE, tmp = first_char; tmp && !found; tmp = tmp->next )
        if ( !is_spectator( tmp ) && ch->hunting->who == tmp )
            found = TRUE;

    if ( !found )
    {
        if ( ch->race == RACE_MARINE )
            do_emote( ch, "curses loudly." );

        if ( ch->race == RACE_ALIEN )
            do_emote( ch, "screechs loudly!" );

        if ( ch->race == RACE_PREDATOR )
            do_emote( ch, "clicks in anger." );

        stop_hunting( ch );
        return;
    }

    if ( ch->in_room == ch->hunting->who->in_room )
    {
        found_prey( ch, ch->hunting->who );
        return;
    }

    /* Hunting with ranged weapons */
    if ( ch->race != RACE_ALIEN )
    {
        if ( mob_snipe( ch, ch->hunting->who ) == TRUE )
            return;
    }

    if ( xIS_SET( ch->act, ACT_SENTINEL ) )
        return;

    ret = find_first_step( ch->in_room, ch->hunting->who->in_room, 100 );

    if ( ret == BFS_NO_PATH )
    {
        EXIT_DATA* pexit;
        int attempt;

        for ( attempt = 0; attempt < 25; attempt++ )
        {
            ret = number_door( );

            if ( ( pexit = get_exit( ch->in_room, ret ) ) == NULL
                    ||   !pexit->to_room
                    || xIS_SET( pexit->exit_info, EX_CLOSED )
                    || xIS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) )
                continue;
        }
    }

    if ( ret < 0 )
    {
        if ( ch->race == RACE_MARINE )
            do_emote( ch, "curses loudly." );

        if ( ch->race == RACE_ALIEN )
            do_emote( ch, "screechs loudly!" );

        if ( ch->race == RACE_PREDATOR )
            do_emote( ch, "clicks in anger." );

        stop_hunting( ch );
        return;
    }
    else
    {
        move_char( ch, get_exit( ch->in_room, ret ), ret, FALSE );

        if ( char_died( ch ) )
            return;

        if ( !ch->hunting )
        {
            if ( !ch->in_room )
            {
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "Hunt_victim: no ch->in_room!  Mob #%d, name: %s.  Placing mob in limbo.",
                         ch->pIndexData->vnum, ch->name );
                bug( buf, 0 );
                char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
                return;
            }

            if ( ch->race == RACE_MARINE )
                do_emote( ch, "curses loudly." );

            if ( ch->race == RACE_ALIEN )
                do_emote( ch, "screechs loudly!" );

            if ( ch->race == RACE_PREDATOR )
                do_emote( ch, "clicks in anger." );

            return;
        }

        if ( ch->in_room == ch->hunting->who->in_room )
            found_prey( ch, ch->hunting->who );

        return;
    }
}

bool mob_snipe( CHAR_DATA* ch, CHAR_DATA* victim )
{
    sh_int            dir, dist, ret;
    sh_int            max_dist = 3;
    EXIT_DATA*        pexit;
    ROOM_INDEX_DATA* to_room;
    char              buf[MAX_STRING_LENGTH];
    OBJ_DATA*         obj;

    if ( !ch->in_room || !victim->in_room )
        return FALSE;

    if ( is_spectator( victim ) )
        return FALSE;

    if ( ch->ap < get_max_ap( ch ) )
        return FALSE;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
    {
        if ( is_ranged( obj->value[0] ) )
        {
            if ( obj->value[2] <= 0 )
                return FALSE;

            max_dist = URANGE( 1, obj->value[2], 8 );
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    for ( dir = 0 ; dir <= 10 ; dir++ )
    {
        if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
            continue;

        if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
            continue;

        for ( dist = 0; dist <= max_dist; dist++ )
        {
            if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
                break;

            if ( !pexit->to_room )
                break;

            to_room = NULL;

            if ( pexit->distance > 1 )
                to_room = generate_exit( to_room, &pexit );

            if ( to_room == NULL )
                to_room = pexit->to_room;

            if ( to_room == victim->in_room )
            {
                // Target located. Snipe and bail
                ret = snipe_direction( ch, victim, "", obj, dir, 1 );

                if ( ret <= 0 )
                    return FALSE;

                if ( ret == 1 )
                    ch->ap = 0;

                if ( ret == 2 )
                    ch->ap -= 1;

                return TRUE;
            }

            if ( ( pexit = get_exit( to_room, dir ) ) == NULL )
                break;
        }
    }

    return FALSE;
}

void follow_victim( CHAR_DATA* ch )
{
    ROOM_INDEX_DATA* tmproom;
    CHAR_DATA* tmp;
    sh_int ret;
    bool found;

    if ( !ch || !ch->master )
        return;

    if ( ch->mp <= 0 )
        return;

    if ( ch->position < 7 )
        return;

    if ( is_spectator( ch ) )
    {
        ch->master = NULL;
        ch->leader = NULL;
        return;
    }

    /* make sure the char still exists */
    for ( found = FALSE, tmp = first_char; tmp && !found; tmp = tmp->next )
        if ( !is_spectator( tmp ) && ch->master == tmp )
            found = TRUE;

    if ( !found )
    {
        ch_printf( ch, "&RYou've lost contact with your group. Damnit.\n\r" );
        stop_follower( ch );
        return;
    }

    if ( ch->in_room == ch->master->in_room )
    {
        /* Minor Mobile AI */
        if ( IS_NPC( ch ) )
        {
            if ( IS_AFFECTED( ch->master, AFF_HIDE ) )
                if ( ch->mp >= 2 && !IS_AFFECTED( ch, AFF_HIDE ) )
                    do_hide( ch, "" );
        }

        return;
    }

    tmproom = ch->in_room;
    ret = find_first_step( ch->in_room, ch->master->in_room, 100 );

    if ( ret == BFS_NO_PATH )
    {
        /*
            EXIT_DATA *pexit = NULL;
            int attempt;

            for ( attempt = 0; attempt < 25; attempt++ )
            {
            ret = number_door( );

            pexit = get_exit(ch->in_room, ret);

            if ( pexit == NULL || !pexit->to_room || xIS_SET(pexit->exit_info, EX_CLOSED) ) continue;
            }
        */
        ret = UMIN( ret, -1 );
    }

    if ( ret < 0 )
    {
        ch_printf( ch, "&RYou've lost contact with your group. Damnit.\n\r" );
        stop_follower( ch );
        return;
    }
    else
    {
        ch_printf( ch, "... You are automatically following %s ...\n\r", ch->master->name );
        move_char( ch, get_exit( ch->in_room, ret ), ret, FALSE );

        if ( ch->in_room == tmproom )
        {
            ch_printf( ch, "\n\r&w&RYou've lost contact with your group. Damnit.\n\r" );
            stop_follower( ch );
            return;
        }

        if ( char_died( ch ) )
            return;

        if ( !ch->master )
        {
            if ( !ch->in_room )
            {
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "Follow_victim: no ch->in_room! ch name: %s.  Placing in limbo.", ch->name );
                bug( buf, 0 );
                char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
                return;
            }

            ch_printf( ch, "\n\r&w&RYou've lost contact with your group. Damnit.\n\r" );
            return;
        }

        return;
    }
}

