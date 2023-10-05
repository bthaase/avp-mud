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
    The MUDprograms are heavily based on the original MOBprogram code that
    was written by N'Atas-ha.
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"

char*   mprog_type_to_name  args( ( int type ) );
ch_ret  simple_damage( CHAR_DATA* ch, CHAR_DATA* victim, int dam, int dt );
CHAR_DATA* get_char_room_mp  args( ( CHAR_DATA* ch, char* argument ) );

char* mprog_type_to_name( int type )
{
    switch ( type )
    {
        case IN_FILE_PROG:
            return "in_file_prog";

        case ACT_PROG:
            return "act_prog";

        case SPEECH_PROG:
            return "speech_prog";

        case RAND_PROG:
            return "rand_prog";

        case FIGHT_PROG:
            return "fight_prog";

        case HITPRCNT_PROG:
            return "hitprcnt_prog";

        case DEATH_PROG:
            return "death_prog";

        case ENTRY_PROG:
            return "entry_prog";

        case GREET_PROG:
            return "greet_prog";

        case ALL_GREET_PROG:
            return "all_greet_prog";

        case GIVE_PROG:
            return "give_prog";

        case BRIBE_PROG:
            return "bribe_prog";

        case HOUR_PROG:
            return "hour_prog";

        case TIME_PROG:
            return "time_prog";

        case WEAR_PROG:
            return "wear_prog";

        case REMOVE_PROG:
            return "remove_prog";

        case SAC_PROG :
            return "sac_prog";

        case LOOK_PROG:
            return "look_prog";

        case EXA_PROG:
            return "exa_prog";

        case ZAP_PROG:
            return "zap_prog";

        case GET_PROG:
            return "get_prog";

        case DROP_PROG:
            return "drop_prog";

        case REPAIR_PROG:
            return "repair_prog";

        case DAMAGE_PROG:
            return "damage_prog";

        case USEON_PROG:
            return "useon_prog";

        case USEOFF_PROG:
            return "useoff_prog";

        case SCRIPT_PROG:
            return "script_prog";

        case SLEEP_PROG:
            return "sleep_prog";

        case REST_PROG:
            return "rest_prog";

        case LEAVE_PROG:
            return "leave_prog";

        case USE_PROG:
            return "use_prog";

        case PULSE_PROG:
            return "pulse_prog";

        default:
            return "ERROR_PROG";
    }
}

/*  A trivial rehack of do_mstat.  This doesnt show all the data, but just
    enough to identify the mob and give its basic condition.  It does however,
    show the MUDprograms which are set.
*/
void do_mpstat( CHAR_DATA* ch, char* argument )
{
    char        arg[MAX_INPUT_LENGTH];
    MPROG_DATA* mprg;
    CHAR_DATA*  victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "MProg stat whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) )
    {
        send_to_char( "Only Mobiles can have MobPrograms!\n\r", ch );
        return;
    }

    if ( xIS_EMPTY( victim->pIndexData->progtypes ) )
    {
        send_to_char( "That Mobile has no Programs set.\n\r", ch );
        return;
    }

    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
               victim->name, victim->pIndexData->vnum );
    ch_printf( ch, "Short description: %s.\n\rLong  description: %s",
               victim->short_descr,
               victim->long_descr[0] != '\0' ?
               victim->long_descr : "(none).\n\r" );
    ch_printf( ch, "Hp: %d/%d.  Move: %d/%d. \n\r",
               victim->hit,         victim->max_hit,
               victim->move,        victim->max_move );

    for ( mprg = victim->pIndexData->mudprogs; mprg; mprg = mprg->next )
        ch_printf( ch, ">%s %s\n\r%s\n\r",
                   mprog_type_to_name( mprg->type ),
                   mprg->arglist,
                   mprg->comlist );

    return;
}

/* Opstat - Scryn 8/12*/
void do_opstat( CHAR_DATA* ch, char* argument )
{
    char        arg[MAX_INPUT_LENGTH];
    MPROG_DATA* mprg;
    OBJ_DATA*   obj;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "OProg stat what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
        send_to_char( "You cannot find that.\n\r", ch );
        return;
    }

    if ( xIS_EMPTY( obj->pIndexData->progtypes ) )
    {
        send_to_char( "That object has no programs set.\n\r", ch );
        return;
    }

    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
               obj->name, obj->pIndexData->vnum );
    ch_printf( ch, "Short description: %s.\n\r",
               obj->short_descr );

    for ( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
        ch_printf( ch, ">%s %s\n\r%s\n\r",
                   mprog_type_to_name( mprg->type ),
                   mprg->arglist,
                   mprg->comlist );

    return;
}

/* Rpstat - Scryn 8/12 */
void do_rpstat( CHAR_DATA* ch, char* argument )
{
    MPROG_DATA* mprg;

    if ( xIS_EMPTY( ch->in_room->progtypes ) )
    {
        send_to_char( "This room has no programs set.\n\r", ch );
        return;
    }

    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
               ch->in_room->name, ch->in_room->vnum );

    for ( mprg = ch->in_room->mudprogs; mprg; mprg = mprg->next )
        ch_printf( ch, ">%s %s\n\r%s\n\r",
                   mprog_type_to_name( mprg->type ),
                   mprg->arglist,
                   mprg->comlist );

    return;
}

/* Prints the argument to all the rooms around the mobile */
void do_mpasound( CHAR_DATA* ch, char* argument )
{
    ROOM_INDEX_DATA* was_in_room;
    EXIT_DATA*       pexit;
    EXT_BV           actflags;

    if ( !ch )
    {
        bug( "Nonexistent ch in do_mpasound!", 0 );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        progbug( "Mpasound - No argument", ch );
        return;
    }

    actflags = ch->act;
    xREMOVE_BIT( ch->act, ACT_SECRETIVE );
    was_in_room = ch->in_room;

    for ( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
    {
        if ( pexit->to_room && pexit->to_room != was_in_room )
        {
            ch->in_room = pexit->to_room;
            MOBtrigger  = FALSE;
            act( AT_SAY, argument, ch, NULL, NULL, TO_ROOM );
        }
    }

    ch->act = actflags;
    ch->in_room = was_in_room;
    return;
}

void do_mprmflag( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA* room;
    int value;
    int state;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || atoi( arg1 ) < -1 || arg2[0] == '\0' || atoi( arg2 ) > 2 )
    {
        send_to_char( "Syntax: MPRMFLAG (vnum) (state) (flags)?\n\r", ch );
        progbug( "Mprmflag: invalid argument1 (room vnum)", ch );
        return;
    }

    if ( atoi( arg1 ) == -1 )
    {
        room = ch->in_room;
    }
    else
    {
        if ( ( room = get_room_index( atoi( arg1 ) ) ) == NULL )
        {
            progbug( "Mprmflag: arg1 - Room vnum does not exist", ch );
            return;
        }
    }

    state = URANGE( 0, atoi( arg2 ), 2 );

    while ( argument[0] != '\0' )
    {
        argument = one_argument( argument, arg3 );
        value = get_rflag( arg3 );

        if ( value < 0 || value >= MAX_ROOM_FLAGS )
            ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
        else
        {
            if ( value != ROOM_MAPSTART )
            {
                if ( state == 0 )
                    xSET_BIT( room->room_flags, value );

                if ( state == 1 )
                    xREMOVE_BIT( room->room_flags, value );

                if ( state == 2 )
                    xTOGGLE_BIT( room->room_flags, value );
            }
        }
    }

    return;
}

/* lets the mobile kill any player or mobile without murder*/

void do_mpkill( CHAR_DATA* ch, char* argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA* victim;

    if ( !ch )
    {
        bug( "Nonexistent ch in do_mpkill!", 0 );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        progbug( "MpKill - no argument", ch );
        return;
    }

    if ( ( victim = get_char_room_mp( ch, arg ) ) == NULL )
    {
        progbug( "MpKill - Victim not in room", ch );
        return;
    }

    if ( victim == ch )
    {
        /* progbug( "MpKill - Bad victim to attack", ch ); */
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
        progbug( "MpKill - Charmed mob attacking master", ch );
        return;
    }

    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}


/*  lets the mobile destroy an object in its inventory
    it can also destroy a worn object and it can destroy
    items using all.xxxxx or just plain all of them */

void do_mpjunk( CHAR_DATA* ch, char* argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    OBJ_DATA* obj;
    OBJ_DATA* obj_next;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        progbug( "Mpjunk - No argument", ch );
        return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        if ( ( obj = get_obj_wear( ch, arg ) ) != NULL )
        {
            unequip_char( ch, obj );
            extract_obj( obj );
            return;
        }

        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
            return;

        extract_obj( obj );
    }
    else
        for ( obj = ch->first_carrying; obj; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
            {
                if ( obj->wear_loc != WEAR_NONE )
                    unequip_char( ch, obj );

                extract_obj( obj );
            }
        }

    return;
}

/*
    This function examines a text string to see if the first "word" is a
    color indicator (e.g. _red, _whi_, _blu).  -  Gorog
*/
int get_color( char* argument )  /* get color code from command string */
{
    char color[MAX_INPUT_LENGTH];
    char* cptr;
    static char const* color_list =
        "_bla_red_dgr_bro_dbl_pur_cya_cha_dch_ora_gre_yel_blu_pin_lbl_whi";
    static char const* blink_list =
        "*bla*red*dgr*bro*dbl*pur*cya*cha*dch*ora*gre*yel*blu*pin*lbl*whi";
    one_argument ( argument, color );

    if ( color[0] != '_' && color[0] != '*' )
        return 0;

    if ( ( cptr = strstr( color_list, color ) ) )
        return ( cptr - color_list ) / 4;

    if ( ( cptr = strstr( blink_list, color ) ) )
        return ( cptr - blink_list ) / 4 + AT_BLINK;

    return 0;
}


/* prints the message to everyone in the room other than the mob and victim */

void do_mpechoaround( CHAR_DATA* ch, char* argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA* victim;
    EXT_BV     actflags;
    sh_int     color;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        progbug( "Mpechoaround - No argument", ch );
        return;
    }

    if ( !( victim = get_char_room_mp( ch, arg ) ) )
    {
        progbug( "Mpechoaround - victim does not exist", ch );
        return;
    }

    actflags = ch->act;
    xREMOVE_BIT( ch->act, ACT_SECRETIVE );

    if ( ( color = get_color( argument ) ) )
    {
        argument = one_argument( argument, arg );
        act( color, argument, ch, NULL, victim, TO_NOTVICT );
    }
    else
        act( AT_ACTION, argument, ch, NULL, victim, TO_NOTVICT );

    ch->act = actflags;
}


/* prints message only to victim */

void do_mpechoat( CHAR_DATA* ch, char* argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA* victim;
    EXT_BV     actflags;
    sh_int     color;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        progbug( "Mpechoat - No argument", ch );
        return;
    }

    if ( !( victim = get_char_room_mp( ch, arg ) ) )
    {
        progbug( "Mpechoat - victim does not exist", ch );
        return;
    }

    actflags = ch->act;
    xREMOVE_BIT( ch->act, ACT_SECRETIVE );

    if ( ( color = get_color( argument ) ) )
    {
        argument = one_argument( argument, arg );
        act( color, argument, ch, NULL, victim, TO_VICT );
    }
    else
        act( AT_ACTION, argument, ch, NULL, victim, TO_VICT );

    ch->act = actflags;
}


/* prints message to room at large. */

void do_mpecho( CHAR_DATA* ch, char* argument )
{
    char       arg1 [MAX_INPUT_LENGTH];
    sh_int     color;
    EXT_BV     actflags;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        progbug( "Mpecho - called w/o argument", ch );
        return;
    }

    actflags = ch->act;
    xREMOVE_BIT( ch->act, ACT_SECRETIVE );

    if ( ( color = get_color( argument ) ) )
    {
        argument = one_argument ( argument, arg1 );
        act( color, argument, ch, NULL, NULL, TO_ROOM );
    }
    else
        act( AT_ACTION, argument, ch, NULL, NULL, TO_ROOM );

    ch->act = actflags;
}


/*  lets the mobile load an item or mobile.  All items
    are loaded into inventory.  you can specify a level with
    the load object portion as well. */

void do_mpmload( CHAR_DATA* ch, char* argument )
{
    char            arg[ MAX_INPUT_LENGTH ];
    MOB_INDEX_DATA* pMobIndex;
    CHAR_DATA*      victim;
    int             count = 0;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
        progbug( "Mpmload - Bad vnum as arg", ch );
        return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
        progbug( "Mpmload - Bad mob vnum", ch );
        return;
    }

    if ( !ch->in_room )
        return;

    for ( victim = ch->in_room->first_person; victim; victim = victim->next_in_room )
        count++;

    if ( count >= 50 )
    {
        progbug( "Mpmload - Limited to 50 mobs to a room", ch );
        return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    return;
}

void do_mpoload( CHAR_DATA* ch, char* argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    OBJ_INDEX_DATA* pObjIndex;
    OBJ_DATA*       obj;
    int             level;
    int         timer = 0;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        progbug( "Mpoload - Bad syntax", ch );
        return;
    }

    if ( arg2[0] == '\0' )
        level = get_trust( ch );
    else
    {
        /*
            New feature from Alander.
        */
        if ( !is_number( arg2 ) )
        {
            progbug( "Mpoload - Bad level syntax", ch );
            return;
        }

        level = atoi( arg2 );

        if ( level < 0 || level > get_trust( ch ) )
        {
            progbug( "Mpoload - Bad level", ch );
            return;
        }

        /*
            New feature from Thoric.
        */
        timer = atoi( argument );

        if ( timer < 0 )
        {
            progbug( "Mpoload - Bad timer", ch );
            return;
        }
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
        progbug( "Mpoload - Bad vnum arg", ch );
        return;
    }

    obj = create_object( pObjIndex, level );
    obj->timer = timer;

    if ( CAN_WEAR( obj, ITEM_TAKE ) )
        obj_to_char( obj, ch );
    else
        obj_to_room( obj, ch->in_room );

    return;
}

/*  lets the mobile purge all objects and other npcs in the room,
    or purge a specified object or mob in the room.  It can purge
    itself, but this had best be the last command in the MUDprogram
    otherwise ugly stuff will happen */

void do_mppurge( CHAR_DATA* ch, char* argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA* victim;
    OBJ_DATA*  obj;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA* vnext;

        for ( victim = ch->in_room->first_person; victim; victim = vnext )
        {
            vnext = victim->next_in_room;

            if ( IS_NPC( victim ) && !IS_BOT( victim ) && victim != ch )
                extract_char( victim, TRUE, FALSE );
        }

        while ( ch->in_room->first_content )
            extract_obj( ch->in_room->first_content );

        return;
    }

    /*
        Added a MOBILE/OBJECT Option for Purge
        - Ghost [6/5/2002]
    */
    if ( !str_cmp( argument, "mobiles" ) || !str_cmp( argument, "mobs" ) )
    {
        CHAR_DATA* vnext;

        for ( victim = ch->in_room->first_person; victim; victim = vnext )
        {
            vnext = victim->next_in_room;

            if ( IS_NPC( victim ) && !IS_BOT( victim ) && victim != ch )
                extract_char( victim, TRUE, FALSE );
        }

        return;
    }

    if ( !str_cmp( argument, "objects" ) || !str_cmp( argument, "objs" ) )
    {
        while ( ch->in_room->first_content )
            extract_obj( ch->in_room->first_content );

        return;
    }

    if ( ( victim = get_char_room_mp( ch, arg ) ) == NULL )
    {
        if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
            extract_obj( obj );
        else
            progbug( "Mppurge - Bad argument", ch );

        return;
    }

    if ( !IS_NPC( victim ) )
    {
        progbug( "Mppurge - Trying to purge a PC", ch );
        return;
    }

    if ( IS_BOT( victim ) )
    {
        progbug( "Mppurge - Trying to purge a Bot", ch );
        return;
    }

    if ( victim == ch )
    {
        progbug( "Mppurge - Trying to purge oneself", ch );
        return;
    }

    if ( IS_NPC( victim ) && victim->pIndexData->vnum == 3 )
    {
        progbug( "Mppurge: trying to purge supermob", ch );
        return;
    }

    extract_char( victim, TRUE, FALSE );
    return;
}


/* Allow mobiles to go wizinvis with programs -- SB */

void do_mpinvis( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int level;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg && arg[0] != '\0' )
    {
        if ( !is_number( arg ) )
        {
            progbug( "Mpinvis - Non numeric argument ", ch );
            return;
        }

        level = atoi( arg );

        if ( level < 2 || level > 105 )
        {
            progbug( "MPinvis - Invalid level ", ch );
            return;
        }

        ch->mobinvis = level;
        ch_printf( ch, "Mobinvis level set to %d.\n\r", level );
        return;
    }

    if ( ch->mobinvis < 2 )
        ch->mobinvis = ch->top_level;

    if ( xIS_SET( ch->act, ACT_MOBINVIS ) )
    {
        xREMOVE_BIT( ch->act, ACT_MOBINVIS );
        act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly fade back into existence.\n\r", ch );
    }
    else
    {
        xSET_BIT( ch->act, ACT_MOBINVIS );
        act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly vanish into thin air.\n\r", ch );
    }

    return;
}

void do_mpgoto( CHAR_DATA* ch, char* argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA* location;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        progbug( "Mpgoto - No argument", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        progbug( "Mpgoto - No such location", ch );
        return;
    }

    char_from_room( ch );
    char_to_room( ch, location );
    return;
}

/* lets the mobile do a command at another location. Very useful */

void do_mpat( CHAR_DATA* ch, char* argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA* location;
    ROOM_INDEX_DATA* original;
    CHAR_DATA*       wch;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        progbug( "Mpat - Bad argument", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        progbug( "Mpat - No such location", ch );
        return;
    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument, FALSE );

    /*
        See if 'ch' still exists before continuing!
        Handles 'at XXXX quit' case.
    */
    for ( wch = first_char; wch; wch = wch->next )
        if ( wch == ch )
        {
            char_from_room( ch );
            char_to_room( ch, original );
            break;
        }

    return;
}

/* allow a mobile to advance a player's level... very dangerous */
void do_mpadvance( CHAR_DATA* ch, char* argument )
{
    return;
}

/*  lets the mobile transfer people.  the all argument transfers
    everyone in the current room to the specified location */

void do_mptransfer( CHAR_DATA* ch, char* argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA* location;
    CHAR_DATA*       victim;
    CHAR_DATA*       nextinroom;
    int             safty = 0;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        progbug( "Mptransfer - Bad syntax", ch );
        return;
    }

    /* Put in the variable nextinroom to make this work right. -Narn */
    if ( !str_cmp( arg1, "all" ) )
    {
        safty = 0;

        for ( victim = ch->in_room->first_person; victim; victim = nextinroom )
        {
            nextinroom = victim->next_in_room;

            if ( ++safty > 9999 )
            {
                progbug( "Mptransfer - Infinite loop transfer", ch );
                return;
            }

            if ( victim != ch && can_see( ch, victim ) )
            {
                sprintf( buf, "%s %s", victim->name, arg2 );
                do_mptransfer( ch, buf );
            }
        }

        return;
    }

    /*
        Thanks to Grodyn for the optional location parameter.
    */
    if ( arg2[0] == '\0' )
    {
        location = ch->in_room;
    }
    else
    {
        if ( ( location = find_location( ch, arg2 ) ) == NULL )
        {
            progbug( "Mptransfer - No such location", ch );
            return;
        }
    }

    if ( ( victim = get_char_world_full( ch, arg1 ) ) == NULL )
    {
        progbug( "Mptransfer - No such person", ch );
        return;
    }

    if ( !victim->in_room )
    {
        progbug( "Mptransfer - Victim in Limbo", ch );
        return;
    }

    if ( NOT_AUTHED( victim ) && location->area != victim->in_room->area )
    {
        progbug( "Mptransfer - transferring unauthorized player", ch );
        return;
    }

    /* If victim not in area's level range, do not transfer */
    // if ( !in_hard_range( victim, location->area ) && !xIS_SET( location->room_flags, ROOM_PROTOTYPE ) ) return;
    char_from_room( victim );
    char_to_room( victim, location );

    if ( victim->carrying )
    {
        char_from_room( victim->carrying );
        char_to_room( victim->carrying, location );
    }

    return;
}

/*  lets the mobile force someone to do something.  must be mortal level
    and the all argument only affects those in the room with the mobile */

void do_mpforce( CHAR_DATA* ch, char* argument )
{
    char arg[ MAX_INPUT_LENGTH ];

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ch->desc )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        progbug( "Mpforce - Bad syntax", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA* vch;

        for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
            if ( get_trust( vch ) < get_trust( ch ) && can_see( ch, vch ) )
                interpret( vch, argument, FALSE );
    }
    else
    {
        CHAR_DATA* victim;

        if ( ( victim = get_char_room_mp( ch, arg ) ) == NULL )
        {
            progbug( "Mpforce - No such victim", ch );
            return;
        }

        if ( victim == ch )
        {
            progbug( "Mpforce - Forcing oneself", ch );
            return;
        }

        if ( !IS_NPC( victim )
                && ( !victim->desc )
                && IS_IMMORTAL( victim ) )
        {
            progbug( "Mpforce - Attempting to force link dead immortal", ch );
            return;
        }

        /*
            if ( get_trust(ch) < get_trust( victim ) && !IS_NPC( victim ) )
            {
            progbug( "Mpforce - Attempting to force higher-level player", ch );
            return;
            }
        */
        interpret( victim, argument, FALSE );
    }

    return;
}

void do_mp_practice( CHAR_DATA* ch, char* argument )
{
}

/*
    Syntax: mpslay (character)
    Reinstalled by Ghost. <---
    All ACT Messages removed by Ghost. <---
*/
void do_mp_slay( CHAR_DATA* ch, char* argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    CHAR_DATA* victim;

    if ( !IS_NPC( ch ) || IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "mpslay whom?\n\r", ch );
        progbug( "Mpslay: invalid (nonexistent?) argument", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpslay: victim not in room", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You try to slay yourself.  You fail.\n\r", ch );
        progbug( "Mpslay: trying to slay self", ch );
        return;
    }

    if ( IS_NPC( victim ) && victim->pIndexData->vnum == 3 )
    {
        send_to_char( "You cannot slay supermob!\n\r", ch );
        progbug( "Mpslay: trying to slay supermob", ch );
        return;
    }

    if ( victim->top_level < LEVEL_IMMORTAL )
    {
        set_cur_char( victim );
        raw_kill( ch, victim );
        stop_hating( ch );
        stop_fearing( ch );
        stop_hunting( ch );
    }

    return;
}

/*
    syntax: mpdamage (character) (#hps)
*/
void do_mp_damage( CHAR_DATA* ch, char* argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    CHAR_DATA* victim;
    int dam;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "mpdamage whom?\n\r", ch );
        progbug( "Mpdamage: invalid argument1", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_char( "mpdamage inflict how many hps?\n\r", ch );
        progbug( "Mpdamage: invalid argument2", ch );
        return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
        CHAR_DATA* rch = NULL;
        CHAR_DATA* rnext = NULL;

        for ( rch = ch->in_room->first_person; rch; rch = rnext )
        {
            rnext = rch->next;

            if ( rch == NULL )
                break;

            // MPDAMAGE?
        }
    }
    else if ( ( victim = get_char_room_mp( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpdamage: victim not in room", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You can't mpdamage yourself.\n\r", ch );
        progbug( "Mpdamage: trying to damage self", ch );
        return;
    }

    dam = atoi( arg2 );

    if ( ( dam < 0 ) || ( dam > 32000 ) )
    {
        send_to_char( "Mpdamage how much?\n\r", ch );
        progbug( "Mpdamage: invalid (nonexistent?) argument", ch );
        return;
    }

    /* this is kinda begging for trouble        */
    /*
        Note from Thoric to whoever put this in...
        Wouldn't it be better to call damage(ch, ch, dam, dt)?
        I hate redundant code
    */
    if ( simple_damage( ch, victim, dam, TYPE_UNDEFINED ) == rVICT_DIED )
    {
        stop_hating( ch );
        stop_fearing( ch );
        stop_hunting( ch );
    }

    return;
}


/*
    syntax: mprestore (character) (#hps)                Gorog
*/
void do_mp_restore( CHAR_DATA* ch, char* argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    CHAR_DATA* victim;
    int hp;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "mprestore whom?\n\r", ch );
        progbug( "Mprestore: invalid argument1", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_char( "mprestore how many hps?\n\r", ch );
        progbug( "Mprestore: invalid argument2", ch );
        return;
    }

    if ( ( victim = get_char_room_mp( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mprestore: victim not in room", ch );
        return;
    }

    hp = atoi( arg2 );

    if ( ( hp < 0 ) || ( hp > 32000 ) )
    {
        send_to_char( "Mprestore how much?\n\r", ch );
        progbug( "Mprestore: invalid (nonexistent?) argument", ch );
        return;
    }

    hp += victim->hit;
    victim->hit = ( hp > 32000 || hp < 0 || hp > victim->max_hit ) ?
                  victim->max_hit : hp;
}

void do_mpgain( CHAR_DATA* ch, char* argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char buf[ MAX_STRING_LENGTH ];
    CHAR_DATA* victim;
    long exp;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "mpgain whom?\n\r", ch );
        progbug( "Mpgain: invalid argument1", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_char( "mpgain how much exp?\n\r", ch );
        progbug( "Mpgain: invalid argument3", ch );
        return;
    }

    if ( ( victim = get_char_room_mp( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpgain: victim not in room", ch );
        return;
    }

    exp = atoi( arg2 );

    if ( ( exp < 1 ) )
    {
        send_to_char( "Mpgain how much?\n\r", ch );
        progbug( "Mpgain: experience out of range", ch );
        return;
    }

    exp =  URANGE( 1, exp, 1000 );
    // sprintf( buf, "[ALERT]: MPGAIN by %s in room %d!", ch->name, ch->in_room->vnum );
    // log_string_plus( buf, LOG_NORMAL, 103 );
    // sprintf( buf, "%s earned %ld %s xp points.", victim->name, exp, ability_name[ability] );
    // log_string_plus( buf, LOG_NORMAL, 103 );
    ch_printf( victim, "You gain %ld experience.\n\r", exp );
    gain_exp( victim, exp );
    return;
}

void do_mpwait( CHAR_DATA* ch, char* argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char buf[ MAX_STRING_LENGTH ];
    CHAR_DATA* victim;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        progbug( "Mpwait: invalid argument1", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        progbug( "Mpwait: invalid argument2", ch );
        return;
    }

    if ( ( victim = get_char_room_mp( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpwait: victim not in room", ch );
        return;
    }

    if ( atoi( arg2 ) < 0 || atoi( arg2 ) > 100 )
    {
        send_to_char( "Mpwait how long?\n\r", ch );
        progbug( "Mpwait: rounds out of range", ch );
        return;
    }

    WAIT_STATE( victim, atoi( arg2 ) );
    return;
}

/*
    Syntax mp_open_passage x y z

    opens a 1-way passage from room x to room y in direction z

    won't mess with existing exits
*/
void do_mp_open_passage( CHAR_DATA* ch, char* argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA* targetRoom, *fromRoom;
    int targetRoomVnum, fromRoomVnum, exit_num;
    EXIT_DATA* pexit;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    if ( !is_number( arg1 ) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    fromRoomVnum = atoi( arg1 );

    if (  ( fromRoom = get_room_index( fromRoomVnum ) )  == NULL )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    if ( !is_number( arg2 ) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    targetRoomVnum = atoi( arg2 );

    if (  ( targetRoom = get_room_index( targetRoomVnum ) )  == NULL )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    if ( !is_number( arg3 ) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    exit_num = atoi( arg3 );

    if ( ( exit_num < 0 ) || ( exit_num > MAX_DIR ) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }

    if ( ( pexit = get_exit( fromRoom, exit_num ) ) != NULL )
    {
        if ( !xIS_SET( pexit->exit_info, EX_PASSAGE ) )
            return;

        progbug( "MpOpenPassage - Exit exists", ch );
        return;
    }

    pexit = make_exit( fromRoom, targetRoom, exit_num );
    pexit->keyword      = STRALLOC( "" );
    pexit->description      = STRALLOC( "" );
    pexit->key          = -1;
    xSET_BIT( pexit->exit_info, EX_PASSAGE );
    /* act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR ); */
    /* act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM ); */
    return;
}

/*
    Syntax mp_close_passage x y

    closes a passage in room x leading in direction y

    the exit must have EX_PASSAGE set
*/
void do_mp_close_passage( CHAR_DATA* ch, char* argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA* fromRoom;
    int fromRoomVnum, exit_num;
    EXIT_DATA* pexit;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg2[0] == '\0' )
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }

    if ( !is_number( arg1 ) )
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }

    fromRoomVnum = atoi( arg1 );

    if (  ( fromRoom = get_room_index( fromRoomVnum ) )  == NULL )
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }

    if ( !is_number( arg2 ) )
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }

    exit_num = atoi( arg2 );

    if ( ( exit_num < 0 ) || ( exit_num > MAX_DIR ) )
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }

    if ( ( pexit = get_exit( fromRoom, exit_num ) ) == NULL )
    {
        return;    /* already closed, ignore...  so rand_progs */
        /*                            can close without spam */
    }

    if ( !xIS_SET( pexit->exit_info, EX_PASSAGE ) )
    {
        progbug( "MpClosePassage - Exit not a passage", ch );
        return;
    }

    extract_exit( fromRoom, pexit );
    /* act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_CHAR ); */
    /* act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_ROOM ); */
    return;
}



/*
    Does nothing.  Used for scripts.
*/
void do_mpnothing( CHAR_DATA* ch, char* argument )
{
    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    return;
}


/*
    Forces the current area to reset.
*/
void do_mpreset( CHAR_DATA* ch, char* argument )
{
    AREA_DATA* area = NULL;
    int num = 0;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !ch->in_room )
        return;

    if ( !ch->in_room->area )
        return;

    area = ch->in_room->area;
    num = area->nplayer;
    area->nplayer = 0;
    reset_area( area );
    area->nplayer = num;
    return;
}

/*
    MPCYCLE - Cycles to the next arena.
*/
void do_mpcycle( CHAR_DATA* ch, char* argument )
{
    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( curr_arena )
    {
        curr_arena->ctimer = 1;
        update_arena( );
        return;
    }

    return;
}

/*
    MPBLAST - Slaughters everyone in the current arena.
*/
void do_mpblast( CHAR_DATA* ch, char* argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA* room;
    CHAR_DATA* rnext;
    CHAR_DATA* rch;
    int fr = 0, lr = 0;
    int vnum = 0;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !ch->in_room )
        return;

    if ( !ch->in_room->area )
        return;

    fr = atoi( arg1 );
    lr = atoi( arg2 );

    if ( fr < ch->in_room->area->low_r_vnum )
        fr = ch->in_room->area->low_r_vnum;

    if ( lr > ch->in_room->area->hi_r_vnum )
        lr = ch->in_room->area->hi_r_vnum;

    if ( fr > lr )
        return;

    for ( vnum = fr; vnum <= lr; vnum++ )
    {
        // Everyone in this room -
        if ( ( room = get_room_index( vnum ) ) != NULL )
        {
            for ( rch = room->first_person ; rch ;  rch = rnext )
            {
                rnext = rch->next_in_room;

                if ( is_spectator( rch ) )
                    continue;

                damage( rch, rch, 5000, TYPE_GENERIC + RIS_FIRE );
            }
        }
    }

    return;
}

/*
     Sends a message to sleeping character.  Should be fun
      with room sleep_progs

*/
void do_mpdream( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_STRING_LENGTH];
    CHAR_DATA* vict;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );

    if (  ( vict = get_char_world_full( ch, arg1 ) ) == NULL )
    {
        progbug( "Mpdream: No such character", ch );
        return;
    }

    if ( vict->position <= POS_STUNNED )
    {
        send_to_char( argument, vict );
        send_to_char( "\n\r",   vict );
    }

    return;
}

void do_mpapply( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        progbug( "Mpapply - bad syntax", ch );
        return;
    }

    if ( ( victim = get_char_room_mp( ch, argument ) ) == NULL )
    {
        progbug( "Mpapply - no such player in room.", ch );
        return;
    }

    if ( !victim->desc )
    {
        send_to_char( "Not on linkdeads.\n\r", ch );
        return;
    }

    if ( !NOT_AUTHED( victim ) )
        return;

    if ( victim->pcdata->auth_state >= 1 )
        return;

    sprintf( log_buf, "%s@%s new %s applying for authorization...",
             victim->name, victim->desc->host,
             race_table[victim->race].race_name );
    log_string( log_buf );
    to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
    victim->pcdata->auth_state = 1;
    return;
}

void do_mpapplyb( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        progbug( "Mpapplyb - bad syntax", ch );
        return;
    }

    if ( ( victim = get_char_room_mp( ch, argument ) ) == NULL )
    {
        progbug( "Mpapplyb - no such player in room.", ch );
        return;
    }

    if ( !victim->desc )
    {
        send_to_char( "Not on linkdeads.\n\r", ch );
        return;
    }

    if ( !NOT_AUTHED( victim ) )
        return;

    if ( get_timer( victim, TIMER_APPLIED ) >= 1 )
        return;

    switch ( victim->pcdata->auth_state )
    {
        case 0:
        case 1:
        default:
            send_to_char( "You attempt to regain the gods' attention.\n\r", victim );
            sprintf( log_buf, "%s@%s new %s applying for authorization...",
                     victim->name, victim->desc->host,
                     race_table[victim->race].race_name );
            log_string( log_buf );
            to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
            add_timer( victim, TIMER_APPLIED, 10, NULL, 0 );
            victim->pcdata->auth_state = 1;
            break;

        case 2:
            send_to_char( "Your name has been deemed unsuitable by the gods.  Please choose a more apropriate name with the 'name' command.\n\r", victim );
            add_timer( victim, TIMER_APPLIED, 10, NULL, 0 );
            break;

        case 3:
            send_to_char( "The gods permit you to enter Aliens vs. Predator.\n\r", victim );
            xREMOVE_BIT( victim->pcdata->flags, PCFLAG_UNAUTHED );
            char_from_room( victim );
            char_to_room( victim, get_room_index( ROOM_VNUM_SCHOOL ) );
            act( AT_WHITE, "$n enters this world from within a column of blinding light!",
                 victim, NULL, NULL, TO_ROOM );
            do_look( victim, "auto" );
            break;
    }

    return;
}

void do_mprunspec( CHAR_DATA* ch, char* argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    SPEC_FUN* spec;
    bool rtn = FALSE;

    /*
        if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;
    */

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        progbug( "Mprunspec - No argument", ch );
        return;
    }

    /*
        if ( !xIS_SET( ch->act, ACT_RUNNING ) )
        {
        progbug( "Mprunspec - Running flag not set", ch );
        return;
        }
    */

    if ( ( spec = spec_lookup( arg ) ) == 0 )
    {
        progbug( "Mprunspec - No such spec", ch );
        return;
    }

    rtn = ( ( *spec ) ( ch ) );
    return;
}

void do_mppkset( CHAR_DATA* ch, char* argument )
{
    send_to_char( "mppkset has been zapped into the realm of useless old code.\n\r", ch );
    return;
}



/*
    Inflict damage from a mudprogram

    note: should be careful about using victim afterwards
*/
ch_ret simple_damage( CHAR_DATA* ch, CHAR_DATA* victim, int dam, int dt )
{
    sh_int dameq;
    bool npcvict;
    OBJ_DATA* damobj;
    ch_ret retcode;
    retcode = rNONE;

    if ( !ch )
    {
        bug( "Damage: null ch!", 0 );
        return rERROR;
    }

    if ( !victim )
    {
        progbug( "Damage: null victim!", ch );
        return rVICT_DIED;
    }

    if ( victim->position == POS_DEAD )
    {
        return rVICT_DIED;
    }

    npcvict = IS_NPC( victim );

    if ( dam )
    {
        if ( dt >= TYPE_GENERIC )
            dam = ris_damage( victim, dam, dt - TYPE_GENERIC, TRUE );

        if ( dam < 0 )
            dam = 0;
    }

    if ( victim != ch )
    {
        /*
            Damage modifiers.
        */
        if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
            dam /= 2;

        if ( dam < 0 )
            dam = 0;

        /* dam_message( ch, victim, dam, dt ); */
    }

    /*
        Hurt the victim.
        Inform the victim of his new state.
    */
    victim->hit -= dam;

    if ( !IS_NPC( victim )
            &&   get_trust( victim ) >= LEVEL_IMMORTAL
            &&   victim->hit < 1 )
        victim->hit = 1;

    if ( !npcvict
            &&   get_trust( victim ) >= LEVEL_IMMORTAL
            &&   get_trust( ch )     >= LEVEL_IMMORTAL
            &&   victim->hit < 1 )
        victim->hit = 1;

    update_pos( victim );

    switch ( victim->position )
    {
        case POS_MORTAL:
            act( AT_DYING, "$n is mortally wounded, and will die soon, if not aided.",
                 victim, NULL, NULL, TO_ROOM );
            act( AT_DANGER, "You are mortally wounded, and will die soon, if not aided.",
                 victim, NULL, NULL, TO_CHAR );
            break;

        case POS_INCAP:
            act( AT_DYING, "$n is incapacitated and will slowly die, if not aided.",
                 victim, NULL, NULL, TO_ROOM );
            act( AT_DANGER, "You are incapacitated and will slowly die, if not aided.",
                 victim, NULL, NULL, TO_CHAR );
            break;

        case POS_STUNNED:
            if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
            {
                act( AT_ACTION, "$n is stunned, but will probably recover.",
                     victim, NULL, NULL, TO_ROOM );
                act( AT_HURT, "You are stunned, but will probably recover.",
                     victim, NULL, NULL, TO_CHAR );
            }

            break;

        case POS_DEAD:
            act( AT_DEAD, "$n is DEAD!!", victim, 0, 0, TO_ROOM );
            act( AT_DEAD, "You have been KILLED!!\n\r", victim, 0, 0, TO_CHAR );
            break;

        default:
            if ( dam > victim->max_hit / 4 )
                act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );

            if ( victim->hit < victim->max_hit / 4 )
                act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!",
                     victim, 0, 0, TO_CHAR );

            break;
    }

    /*
        Payoff for killing things.
    */
    if ( victim->position == POS_DEAD )
    {
        if ( !npcvict )
        {
            sprintf( log_buf, "%s killed by %s at %d",
                     victim->name,
                     ( IS_NPC( ch ) ? ch->short_descr : ch->name ),
                     victim->in_room->vnum );
            log_string( log_buf );
            to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
        }

        set_cur_char( victim );
        raw_kill( ch, victim );
        victim = NULL;
        return rVICT_DIED;
    }

    if ( victim == ch )
        return rNONE;

    /*
        Take care of link dead people.
    */
    if ( !npcvict && !victim->desc )
    {
        if ( number_range( 0, victim->wait ) == 0 )
        {
            do_recall( victim, "" );
            return rNONE;
        }
    }

    tail_chain( );
    return rNONE;
}

CHAR_DATA* get_char_room_mp( CHAR_DATA* ch, char* argument )
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
        if ( ( nifty_is_name( arg, rch->name )
                ||  ( IS_NPC( rch ) && vnum == rch->pIndexData->vnum ) ) )
        {
            if ( number == 0 && !IS_NPC( rch ) )
                return rch;
            else if ( ++count == number )
                return rch;
        }

    if ( vnum != -1 )
        return NULL;

    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( !nifty_is_name_prefix( arg, rch->name ) )
            continue;

        if ( number == 0 && !IS_NPC( rch ) )
            return rch;
        else if ( ++count == number )
            return rch;
    }

    return NULL;
}

void do_mprat( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* location;
    ROOM_INDEX_DATA* original;
    int Start, End, vnum;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Syntax: rat <start> <end> <command>\n\r", ch );
        progbug( "Mprat: invalid arguments. (start, end, command)", ch );
        return;
    }

    Start = atoi( arg1 );
    End = atoi( arg2 );

    if ( Start < 1 || End < Start || Start > End || Start == End || End > MAX_VNUMS )
    {
        progbug( "Mprat: invalid range", ch );
        return;
    }

    if ( !str_cmp( argument, "quit" ) )
    {
        send_to_char( "I don't think so!\n\r", ch );
        return;
    }

    original = ch->in_room;

    for ( vnum = Start; vnum <= End; vnum++ )
    {
        if ( ( location = get_room_index( vnum ) ) == NULL )
            continue;

        char_from_room( ch );
        char_to_room( ch, location );
        interpret( ch, argument, FALSE );
    }

    char_from_room( ch );
    char_to_room( ch, original );
    return;
}

void do_mpset( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int value;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !ch->in_room )
    {
        progbug( "MPSET must be inside a room.", ch );
        return;
    }

    if ( !ch->in_room->area )
    {
        progbug( "MPSET must be inside am area.", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        progbug( "Syntax: MPSET <variable> <value>", ch );
        return;
    }

    value = atoi( arg2 );
    set_variable( ch->in_room->area, arg1, value );
    return;
}

void do_mpadd( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int value;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !ch->in_room )
    {
        progbug( "MPADD must be inside a room.", ch );
        return;
    }

    if ( !ch->in_room->area )
    {
        progbug( "MPADD must be inside am area.", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        progbug( "Syntax: MPADD <variable> <amount>", ch );
        return;
    }

    value = atoi( arg2 );
    value = get_variable( ch->in_room->area, arg1 ) + value;

    if ( value > 9999 || value < -9999 )
    {
        progbug( "MPADD is attempting to go out of bounds.", ch );
        return;
    }

    set_variable( ch->in_room->area, arg1, value );
    return;
}

void do_mpsub( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int value;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !ch->in_room )
    {
        progbug( "MPSUB must be inside a room.", ch );
        return;
    }

    if ( !ch->in_room->area )
    {
        progbug( "MPSUB must be inside am area.", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        progbug( "Syntax: MPSUB <variable> <amount>", ch );
        return;
    }

    value = atoi( arg2 );
    value = get_variable( ch->in_room->area, arg1 ) - value;

    if ( value > 9999 || value < -9999 )
    {
        progbug( "MPSUB is attempting to go out of bounds.", ch );
        return;
    }

    set_variable( ch->in_room->area, arg1, value );
    return;
}

void do_mpmonitor( CHAR_DATA* ch, char* argument )
{
    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        progbug( "Syntax: MPMONITOR (message)", ch );
        return;
    }

    send_monitor( NULL, argument );
    return;
}

void do_mpvar( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int var;
    argument = one_argument( argument, arg );

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        progbug( "Syntax: MPVAR (variable) (message)", ch );
        return;
    }

    var = get_variable( ch->in_room->area, arg );
    sprintf( buf, argument, var );
    send_monitor( NULL, buf );
    return;
}

void do_mpteamgain( CHAR_DATA* ch, char* argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char buf[ MAX_STRING_LENGTH ];
    CHAR_DATA* victim;
    int race;
    int exp;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "mpteamgain what race?\n\r", ch );
        progbug( "Mpgain: invalid argument1 (which race?)", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_char( "mpteamgain how much exp?\n\r", ch );
        progbug( "Mpgain: invalid argument2 (how much xp?)", ch );
        return;
    }

    if ( !str_cmp( arg1, "alien" ) )
    {
        race = RACE_ALIEN;
    }
    else if ( !str_cmp( arg1, "marine" ) )
    {
        race = RACE_MARINE;
    }
    else if ( !str_cmp( arg1, "predator" ) )
    {
        race = RACE_PREDATOR;
    }
    else
    {
        race = atoi( arg1 );
    }

    exp = atoi( arg2 );

    if ( ( exp < 1 || exp > 10000 ) )
    {
        send_to_char( "Mpteamgain how much?\n\r", ch );
        progbug( "Mpteamgain: experience out of range", ch );
        return;
    }

    exp =  URANGE( 1, exp, 10000 );

    if ( race == RACE_ALIEN )
        sprintf( buf, "&GAliens have been awarded %d experience!", exp );

    if ( race == RACE_MARINE )
        sprintf( buf, "&GMarines have been awarded %d experience!", exp );

    if ( race == RACE_PREDATOR )
        sprintf( buf, "&GPredators have been awarded %d experience!", exp );

    send_monitor( NULL, buf );
    team_xpgain( race, exp );
    return;
}
