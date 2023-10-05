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
                   Object manipulation module
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "bet.h"

#ifndef WIN32
    #include <dirent.h>
#else
    #include <dir.h>
#endif

#define MAX_NEST        100
static  OBJ_DATA*   rgObjNest   [MAX_NEST];

/*
    External functions
*/
void    show_list_to_char  args( ( OBJ_DATA* list, CHAR_DATA* ch, bool fShort, bool fShowNothing ) );
/*
    Local functions.
*/
void    get_obj         args( ( CHAR_DATA* ch, OBJ_DATA* obj, OBJ_DATA* container ) );
bool    remove_obj  args( ( CHAR_DATA* ch, int iWear, bool fReplace ) );


/*
    how resistant an object is to damage             -Thoric
*/
sh_int get_obj_resistance( OBJ_DATA* obj )
{
    sh_int resist;
    resist = number_fuzzy( MAX_ITEM_IMPACT );

    /* lets make store inventory pretty tough */
    if ( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
        resist += 20;

    /* okay... let's add some bonus/penalty for item level... */
    resist += ( obj->level / 10 );

    /* and lasty... take armor or weapon's condition into consideration */
    if ( obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_WEAPON )
        resist += ( 10 - obj->value[0] );

    return URANGE( 10, resist, 75 );
}

void get_obj( CHAR_DATA* ch, OBJ_DATA* obj, OBJ_DATA* container )
{
    int weight;

    if ( !CAN_WEAR( obj, ITEM_TAKE ) && ( ch->top_level < sysdata.level_getobjnotake )  )
    {
        send_to_char( "You can't take that.\n\r", ch );
        return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !can_take_proto( ch ) )
    {
        send_to_char( "Energy crackles over it, Making it impossible to reach!\n\r", ch );
        return;
    }

    if ( ch->race == RACE_ALIEN && !IS_OBJ_STAT( obj, ITEM_ALIEN )  )
    {
        send_to_char( "Aliens can't carry that sort of thing.\n\r", ch );
        return;
    }

    if ( get_sentry( obj ) != NULL )
    {
        send_to_char( "You can't pick that up, its an active gun emplacement! Turn it off!\n\r", ch );
        return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        act( AT_PLAIN, "$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR );
        return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
        weight = obj->weight;
    else
        weight = get_obj_weight( obj );

    if ( ch->carry_weight + weight > can_carry_w( ch ) )
    {
        act( AT_PLAIN, "$d: you can't carry that much weight.", ch, NULL, obj->name, TO_CHAR );
        return;
    }

    if ( container )
    {
        act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING ) ? "You get $p from beneath $P." : "You get $p from $P", ch, obj, container, TO_CHAR );
        act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING ) ? "$n gets $p from beneath $P." : "$n gets $p from $P", ch, obj, container, TO_ROOM );
        obj_from_obj( obj );
    }
    else
    {
        act( AT_ACTION, "You get $p.", ch, obj, container, TO_CHAR );
        act( AT_ACTION, "$n gets $p.", ch, obj, container, TO_ROOM );
        obj_from_room( obj );
    }

    /* SAVE_EQ room checks */
    if ( if_equip_room( ch->in_room ) && ( !container || container->carried_by == NULL ) )
        save_equip_room( ch, ch->in_room );

    if ( char_died( ch ) )
        return;

    obj = obj_to_char( obj, ch );

    if ( char_died( ch ) || obj_extracted( obj ) )
        return;

    oprog_get_trigger( ch, obj );
    return;
}


void do_get( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    OBJ_DATA* obj_next;
    OBJ_DATA* container;
    sh_int number;
    bool found;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );

    if ( is_number( arg1 ) )
    {
        number = atoi( arg1 );

        if ( number < 1 )
        {
            send_to_char( "That was easy...\n\r", ch );
            return;
        }

        if ( ( ch->carry_number + number ) > can_carry_n( ch ) )
        {
            send_to_char( "You can't carry that many.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg1 );
    }
    else
    {
        number = 0;
    }

    argument = one_argument( argument, arg2 );

    /* Munch optional words */
    if ( !str_cmp( arg2, "from" ) && argument[0] != '\0' )
        argument = one_argument( argument, arg2 );

    /* Get type */
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Get what?\n\r", ch );
        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    if ( arg2[0] == '\0' )
    {
        if ( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
        {
            /* 'get obj' */
            obj = get_obj_list( ch, arg1, ch->in_room->first_content );

            if ( !obj )
            {
                act( AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
                return;
            }

            separate_obj( obj );
            get_obj( ch, obj, NULL );

            if ( char_died( ch ) )
                return;

            if ( IS_SET( sysdata.save_flags, SV_GET ) )
                save_char_obj( ch );
        }
        else
        {
            sh_int cnt = 0;
            bool fAll;
            char* chk;

            if ( !str_cmp( arg1, "all" ) )
                fAll = TRUE;
            else
                fAll = FALSE;

            if ( number > 1 )
                chk = arg1;
            else
                chk = &arg1[4];

            /* 'get all' or 'get all.obj' */
            found = FALSE;

            for ( obj = ch->in_room->first_content; obj; obj = obj_next )
            {
                obj_next = obj->next_content;

                if ( ( fAll || nifty_is_name( chk, obj->name ) )
                        &&   can_see_obj( ch, obj ) )
                {
                    found = TRUE;

                    if ( number && ( cnt + obj->count ) > number )
                        split_obj( obj, number - cnt );

                    cnt += obj->count;
                    get_obj( ch, obj, NULL );

                    if ( char_died( ch )
                            ||   ch->carry_number >= can_carry_n( ch )
                            ||   ch->carry_weight >= can_carry_w( ch )
                            ||   ( number && cnt >= number ) )
                    {
                        if ( IS_SET( sysdata.save_flags, SV_GET )
                                &&  !char_died( ch ) )
                            save_char_obj( ch );

                        return;
                    }
                }
            }

            if ( !found )
            {
                if ( fAll )
                    send_to_char( "I see nothing here.\n\r", ch );
                else
                    act( AT_PLAIN, "I see no $T here.", ch, NULL, chk, TO_CHAR );
            }
            else if ( IS_SET( sysdata.save_flags, SV_GET ) )
                save_char_obj( ch );
        }
    }
    else
    {
        /* 'get ... container' */
        if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }

        if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
        {
            act( AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
            return;
        }

        switch ( container->item_type )
        {
            default:
                if ( !IS_OBJ_STAT( container, ITEM_COVERING ) )
                {
                    send_to_char( "That's not a container.\n\r", ch );
                    return;
                }

                if ( ch->carry_weight + container->weight > can_carry_w( ch ) )
                {
                    send_to_char( "It's too heavy for you to lift.\n\r", ch );
                    return;
                }

                break;

            case ITEM_CONTAINER:
            case ITEM_CORPSE_PC:
            case ITEM_CORPSE_NPC:
                break;
        }

        if ( !IS_OBJ_STAT( container, ITEM_COVERING )
                &&    IS_SET( container->value[1], CONT_CLOSED ) )
        {
            act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
            return;
        }

        if ( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
        {
            /* 'get obj container' */
            obj = get_obj_list( ch, arg1, container->first_content );

            if ( !obj )
            {
                act( AT_PLAIN, IS_OBJ_STAT( container, ITEM_COVERING ) ?
                     "I see nothing like that beneath the $T." :
                     "I see nothing like that in the $T.",
                     ch, NULL, arg2, TO_CHAR );
                return;
            }

            separate_obj( obj );
            get_obj( ch, obj, container );

            if ( char_died( ch ) )
                return;

            if ( IS_SET( sysdata.save_flags, SV_GET ) )
                save_char_obj( ch );
        }
        else
        {
            int cnt = 0;
            bool fAll;
            char* chk;

            /* 'get all container' or 'get all.obj container' */
            if ( !str_cmp( arg1, "all" ) )
                fAll = TRUE;
            else
                fAll = FALSE;

            if ( number > 1 )
                chk = arg1;
            else
                chk = &arg1[4];

            found = FALSE;

            for ( obj = container->first_content; obj; obj = obj_next )
            {
                obj_next = obj->next_content;

                if ( ( fAll || nifty_is_name( chk, obj->name ) )
                        &&   can_see_obj( ch, obj ) )
                {
                    found = TRUE;

                    if ( number && ( cnt + obj->count ) > number )
                        split_obj( obj, number - cnt );

                    cnt += obj->count;
                    get_obj( ch, obj, container );

                    if ( char_died( ch )
                            ||   ch->carry_number >= can_carry_n( ch )
                            ||   ch->carry_weight >= can_carry_w( ch )
                            ||   ( number && cnt >= number ) )
                        return;
                }
            }

            if ( !found )
            {
                if ( fAll )
                    act( AT_PLAIN, IS_OBJ_STAT( container, ITEM_COVERING ) ?
                         "I see nothing beneath the $T." :
                         "I see nothing in the $T.",
                         ch, NULL, arg2, TO_CHAR );
                else
                    act( AT_PLAIN, IS_OBJ_STAT( container, ITEM_COVERING ) ?
                         "I see nothing like that beneath the $T." :
                         "I see nothing like that in the $T.",
                         ch, NULL, arg2, TO_CHAR );
            }

            if ( char_died( ch ) )
                return;

            if ( found && IS_SET( sysdata.save_flags, SV_GET ) )
                save_char_obj( ch );
        }
    }

    return;
}



void do_put( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA* container;
    OBJ_DATA* obj;
    OBJ_DATA* obj_next;
    sh_int  count;
    int     number;
    bool    save_char = FALSE;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );

    if ( is_number( arg1 ) )
    {
        number = atoi( arg1 );

        if ( number < 1 )
        {
            send_to_char( "That was easy...\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg1 );
    }
    else
        number = 0;

    argument = one_argument( argument, arg2 );

    /* munch optional words */
    if ( ( !str_cmp( arg2, "into" ) || !str_cmp( arg2, "inside" ) || !str_cmp( arg2, "in" ) )
            &&   argument[0] != '\0' )
        argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Put what in what?\n\r", ch );
        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
        act( AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
        return;
    }

    if ( !container->carried_by && IS_SET( sysdata.save_flags, SV_PUT ) )
        save_char = TRUE;

    if ( IS_OBJ_STAT( container, ITEM_COVERING ) )
    {
        if ( ch->carry_weight + container->weight > can_carry_w( ch ) )
        {
            send_to_char( "It's too heavy for you to lift.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( container->item_type != ITEM_CONTAINER )
        {
            send_to_char( "That's not a container.\n\r", ch );
            return;
        }

        if ( IS_SET( container->value[1], CONT_CLOSED ) )
        {
            act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
            return;
        }
    }

    if ( number <= 1 && str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) )
    {
        /* 'put obj container' */
        if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        if ( obj == container )
        {
            send_to_char( "You can't fold it into itself.\n\r", ch );
            return;
        }

        if ( !can_drop_obj( ch, obj ) )
        {
            send_to_char( "You can't let go of it.\n\r", ch );
            return;
        }

        if ( ( IS_OBJ_STAT( container, ITEM_COVERING )
                &&   ( get_obj_weight( obj ) / obj->count )
                > ( ( get_obj_weight( container ) / container->count )
                    -   container->weight ) ) )
        {
            send_to_char( "It won't fit under there.\n\r", ch );
            return;
        }

        if ( ( get_obj_weight( obj ) / obj->count )
                + ( get_obj_weight( container ) / container->count )
                >  container->value[0] )
        {
            send_to_char( "It won't fit.\n\r", ch );
            return;
        }

        if ( ( get_obj_weight( obj ) / obj->count ) > container->value[4] )
        {
            send_to_char( "Its too heavy for that container!\n\r", ch );
            return;
        }

        separate_obj( obj );
        separate_obj( container );
        obj_from_char( obj );
        obj = obj_to_obj( obj, container );

        if ( char_died( ch ) )
            return;

        count = obj->count;
        obj->count = 1;
        act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING )
             ? "$n hides $p beneath $P." : "$n puts $p in $P.",
             ch, obj, container, TO_ROOM );
        act( AT_ACTION, IS_OBJ_STAT( container, ITEM_COVERING )
             ? "You hide $p beneath $P." : "You put $p in $P.",
             ch, obj, container, TO_CHAR );
        obj->count = count;

        if ( save_char )
            save_char_obj( ch );

        /* SAVE_EQ room checks */
        if ( if_equip_room( ch->in_room ) && ( !container || container->carried_by == NULL ) )
            save_equip_room( ch, ch->in_room );
    }
    else
    {
        bool found = FALSE;
        int cnt = 0;
        bool fAll;
        char* chk;

        if ( !str_cmp( arg1, "all" ) )
            fAll = TRUE;
        else
            fAll = FALSE;

        if ( number > 1 )
            chk = arg1;
        else
            chk = &arg1[4];

        separate_obj( container );

        /* 'put all container' or 'put all.obj container' */
        for ( obj = ch->first_carrying; obj; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( ( fAll || nifty_is_name( chk, obj->name ) )
                    &&   can_see_obj( ch, obj )
                    &&   obj->wear_loc == WEAR_NONE
                    &&   obj != container
                    &&   can_drop_obj( ch, obj )
                    &&   get_obj_weight( obj ) + get_obj_weight( container )
                    <= container->value[0]
                    &&   get_obj_weight( obj ) <= container->value[4] )
            {
                if ( number && ( cnt + obj->count ) > number )
                    split_obj( obj, number - cnt );

                cnt += obj->count;
                obj_from_char( obj );
                act( AT_ACTION, "$n puts $p in $P.", ch, obj, container, TO_ROOM );
                act( AT_ACTION, "You put $p in $P.", ch, obj, container, TO_CHAR );
                obj = obj_to_obj( obj, container );
                found = TRUE;

                if ( char_died( ch ) )
                    return;

                if ( number && cnt >= number )
                    break;
            }
        }

        /*
            Don't bother to save anything if nothing was dropped   -Thoric
        */
        if ( !found )
        {
            if ( fAll )
                act( AT_PLAIN, "You are not carrying anything.",
                     ch, NULL, NULL, TO_CHAR );
            else
                act( AT_PLAIN, "You are not carrying any $T.",
                     ch, NULL, chk, TO_CHAR );

            return;
        }

        if ( save_char )
            save_char_obj( ch );

        /* SAVE_EQ room checks */
        if ( if_equip_room( ch->in_room ) && ( !container || container->carried_by == NULL ) )
            save_equip_room( ch, ch->in_room );
    }

    return;
}


void do_drop( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    OBJ_DATA* obj_next;
    bool found;
    int number;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( IN_VENT( ch ) )
    {
        send_to_char( "&RNot while inside a vent. Sorry.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( is_number( arg ) )
    {
        number = atoi( arg );

        if ( number < 1 )
        {
            send_to_char( "That was easy...\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg );
    }
    else
        number = 0;

    if ( arg[0] == '\0' )
    {
        send_to_char( "Drop what?\n\r", ch );
        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_NODROP )
            ||   ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_LITTERBUG ) ) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "A magical force stops you!\n\r", ch );
        set_char_color( AT_TELL, ch );
        send_to_char( "Someone tells you, 'No littering here!'\n\r", ch );
        return;
    }

    if ( number <= 1 && str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        /* 'drop obj' */
        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        if ( !can_drop_obj( ch, obj ) )
        {
            send_to_char( "You can't let go of it.\n\r", ch );
            return;
        }

        separate_obj( obj );
        act( AT_ACTION, "$n drops $p.", ch, obj, NULL, TO_ROOM );
        act( AT_ACTION, "You drop $p.", ch, obj, NULL, TO_CHAR );
        obj_from_char( obj );
        obj = obj_to_room( obj, ch->in_room );
        oprog_drop_trigger ( ch, obj );   /* mudprogs */

        if ( char_died( ch ) || obj_extracted( obj ) )
            return;

        /* SAVE_EQ room checks */
        if ( if_equip_room( ch->in_room ) )
            save_equip_room( ch, ch->in_room );
    }
    else
    {
        int cnt = 0;
        char* chk;
        bool fAll;

        if ( !str_cmp( arg, "all" ) )
            fAll = TRUE;
        else
            fAll = FALSE;

        if ( number > 1 )
            chk = arg;
        else
            chk = &arg[4];

        /* 'drop all' or 'drop all.obj' */
        if ( xIS_SET( ch->in_room->room_flags, ROOM_NODROPALL ) )
        {
            send_to_char( "You can't seem to do that here...\n\r", ch );
            return;
        }

        found = FALSE;

        for ( obj = ch->first_carrying; obj; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( ( fAll || nifty_is_name( chk, obj->name ) )
                    &&   can_see_obj( ch, obj )
                    &&   obj->wear_loc == WEAR_NONE
                    &&   can_drop_obj( ch, obj ) )
            {
                found = TRUE;

                if ( xIS_SET( obj->pIndexData->progtypes, DROP_PROG ) && obj->count > 1 )
                {
                    ++cnt;
                    separate_obj( obj );
                    obj_from_char( obj );

                    if ( !obj_next )
                        obj_next = ch->first_carrying;
                }
                else
                {
                    if ( number && ( cnt + obj->count ) > number )
                        split_obj( obj, number - cnt );

                    cnt += obj->count;
                    obj_from_char( obj );
                }

                act( AT_ACTION, "$n drops $p.", ch, obj, NULL, TO_ROOM );
                act( AT_ACTION, "You drop $p.", ch, obj, NULL, TO_CHAR );
                obj = obj_to_room( obj, ch->in_room );
                oprog_drop_trigger( ch, obj );      /* mudprogs */

                if ( char_died( ch ) )
                    return;

                if ( number && cnt >= number )
                    break;
            }
        }

        /* SAVE_EQ room checks */
        if ( if_equip_room( ch->in_room ) )
            save_equip_room( ch, ch->in_room );

        if ( !found )
        {
            if ( fAll )
                act( AT_PLAIN, "You are not carrying anything.",
                     ch, NULL, NULL, TO_CHAR );
            else
                act( AT_PLAIN, "You are not carrying any $T.",
                     ch, NULL, chk, TO_CHAR );
        }
    }

    if ( IS_SET( sysdata.save_flags, SV_DROP ) )
        save_char_obj( ch );    /* duping protector */

    return;
}



void do_give( CHAR_DATA* ch, char* argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf  [MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    OBJ_DATA*  obj;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
        argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Give what to whom?\n\r", ch );
        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
        send_to_char( "You must remove it first.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->race != ch->race && !IS_IMMORTAL( ch ) )
    {
        send_to_char( "Your supposed to be killing them. Sharing is wrong.\n\r", ch );
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    if ( victim->carry_number + ( get_obj_number( obj ) / obj->count ) > can_carry_n( victim ) )
    {
        act( AT_PLAIN, "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( victim->carry_weight + ( get_obj_weight( obj ) / obj->count ) > can_carry_w( victim ) )
    {
        act( AT_PLAIN, "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
        act( AT_PLAIN, "$N can't see it.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !can_take_proto( victim ) )
    {
        act( AT_PLAIN, "You cannot give that to $N!", ch, NULL, victim, TO_CHAR );
        return;
    }

    separate_obj( obj );
    obj_from_char( obj );
    act( AT_ACTION, "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( AT_ACTION, "$n gives you $p.",   ch, obj, victim, TO_VICT    );
    act( AT_ACTION, "You give $p to $N.", ch, obj, victim, TO_CHAR    );
    obj = obj_to_char( obj, victim );
    mprog_give_trigger( victim, ch, obj );

    if ( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died( ch ) )
        save_char_obj( ch );

    if ( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died( victim ) )
        save_char_obj( victim );

    return;
}

void do_show( CHAR_DATA* ch, char* argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf  [MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    OBJ_DATA*  obj;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
        argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Show what to whom?\n\r", ch );
        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
        send_to_char( "You must remove it first.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
        act( AT_PLAIN, "$N can't see it.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !can_take_proto( victim ) )
    {
        act( AT_PLAIN, "You cannot show that to $N!", ch, NULL, victim, TO_CHAR );
        return;
    }

    act( AT_YELLOW, "$n shows $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( AT_YELLOW, "$n shows you $p.",   ch, obj, victim, TO_VICT    );
    act( AT_YELLOW, "You show $p to $N.", ch, obj, victim, TO_CHAR    );
    return;
}

/*
    Damage a single object.
    Make object into scraps if necessary.
*/
obj_ret damage_obj( OBJ_DATA* obj, int dam )
{
    ROOM_INDEX_DATA* room;
    CHAR_DATA* rch, * rnext;
    CHAR_DATA* ch;
    obj_ret objcode;
    ch = obj->carried_by;
    objcode = rNONE;

    if ( obj->item_type == ITEM_SCRAPS )
        return objcode;

    if ( obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_CORPSE_PC )
        return objcode;

    if ( IS_OBJ_STAT( obj, ITEM_INDESTRUCTABLE ) )
        return objcode;

    separate_obj( obj );
    /*
        if ( ch )
        act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_CHAR );
        else
        if ( obj->in_room && ( ch = obj->in_room->first_person ) != NULL )
        {
        act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_ROOM );
        act( AT_OBJECT, "($p gets damaged)", ch, obj, NULL, TO_CHAR );
        ch = NULL;
        }
    */
    oprog_damage_trigger( ch, obj );

    if ( obj_extracted( obj ) )
        return global_objcode;

    switch ( obj->item_type )
    {
        default:

            /* Dont ALWAYS Scrap it.. Give it a chance. */
            if ( number_range( 1, 4 ) == 1 )
            {
                OBJ_DATA* scraps;
                scraps = make_scraps( obj );
                objcode = rOBJ_SCRAPPED;
            }

            break;

        case ITEM_NVGOGGLE:
            if ( number_range( 1, 8 ) == 1 )
            {
                OBJ_DATA* scraps;
                scraps = make_scraps( obj );
                objcode = rOBJ_SCRAPPED;
            }

            break;

        case ITEM_COVER:
            obj->value[3] = URANGE( 0, obj->value[3] - dam, obj->value[4] );
            room = obj->in_room;

            if ( obj->carried_by )
                room = obj->carried_by->in_room;

            if ( obj->value[3] <= 0 )
            {
                for ( ch = last_char; ch; ch = ch->prev )
                {
                    remove_cover( ch, obj );
                }

                /* Death actions */
                if ( ( obj->value[5] == 1 || obj->value[5] == 4 ) && room )
                {
                    for ( rch = room->first_person; rch; rch = rnext )
                    {
                        rnext = rch->next_in_room;

                        if ( IN_VENT( rch ) || is_spectator( rch ) )
                            continue;

                        if ( !IS_NPC( rch ) )
                        {
                            ch_printf( rch, "&w&R%s explodes violently! (%d fire damage)\n\r", obj->short_descr, ( obj->weight * 10 ) );
                        }

                        if ( obj->parent )
                        {
                            damage( obj->parent, rch, obj->weight * 10, TYPE_GENERIC + ( ( obj->value[5] == 1 ) ? RIS_FIRE : RIS_PIERCE ) );
                        }
                        else
                        {
                            damage( rch, rch, obj->weight * 10, TYPE_GENERIC + ( ( obj->value[5] == 1 ) ? RIS_FIRE : RIS_PIERCE ) );
                        }

                        if ( obj->value[5] == 1 )
                            ignite_target( rch, rch, obj->weight );
                    }
                }
                else if ( obj->value[5] == 3 )
                {
                    oprog_useon_trigger( NULL, obj );
                }

                extract_obj( obj );
                objcode = rOBJ_SCRAPPED;
            }

            break;

        case ITEM_CONTAINER:
            if ( --obj->value[3] <= 0 )
            {
                OBJ_DATA* scraps;
                scraps = make_scraps( obj );
                objcode = rOBJ_SCRAPPED;
            }

            break;

        case ITEM_GRENADE:
            // explode( obj );
            break;

        case ITEM_REGENERATOR:
            if ( ( obj->value[0] -= dam ) <= 0 )
            {
                OBJ_DATA* scraps;
                scraps = make_scraps( obj );
                objcode = rOBJ_SCRAPPED;
            }

            break;

        case ITEM_SENTRY:
            if ( ( obj->value[3] -= dam ) <= 0 )
            {
                OBJ_DATA* scraps;
                scraps = make_scraps( obj );
                objcode = rOBJ_SCRAPPED;
            }

            break;

        case ITEM_TRAP:
        case ITEM_MOTIONB:
        case ITEM_MSPAWNER:
            extract_obj( obj );
            objcode = rOBJ_SCRAPPED;
            break;

        case ITEM_ARMOR:
            if ( ( obj->value[0] -= dam ) <= 0 )
            {
                OBJ_DATA* scraps;
                scraps = make_scraps( obj );
                objcode = rOBJ_SCRAPPED;
            }

            break;

        case ITEM_WEAPON:
        {
            OBJ_DATA* scraps;
            scraps = make_scraps( obj );
            objcode = rOBJ_SCRAPPED;
        }
        break;
    }

    return objcode;
}


/*
    Remove an object.
*/
bool remove_obj( CHAR_DATA* ch, int iWear, bool fReplace )
{
    OBJ_DATA* obj, *tmpobj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
        return TRUE;

    if ( !fReplace
            &&   ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        act( AT_PLAIN, "$d: you can't carry that many items.",
             ch, NULL, obj->name, TO_CHAR );
        return FALSE;
    }

    if ( !fReplace )
        return FALSE;

    if ( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
    {
        act( AT_PLAIN, "You can't remove $p.", ch, obj, NULL, TO_CHAR );
        return FALSE;
    }

    if ( obj == get_eq_char( ch, WEAR_WIELD ) && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL )
        tmpobj->wear_loc = WEAR_WIELD;

    unequip_lag( ch, obj );
    unequip_char( ch, obj );
    act( AT_ACTION, "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( AT_ACTION, "You stop using $p.", ch, obj, NULL, TO_CHAR );
    oprog_remove_trigger( ch, obj );
    return TRUE;
}

/*
    See if char can dual wield at this time          -Thoric
*/
bool can_dual( CHAR_DATA* ch )
{
    OBJ_DATA* wield = NULL;

    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
    {
        send_to_char( "You are already wielding two weapons!\n\r", ch );
        return FALSE;
    }

    if ( get_eq_char( ch, WEAR_HOLD ) )
    {
        send_to_char( "You cannot dual wield while holding something!\n\r", ch );
        return FALSE;
    }

    if ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
    {
        if ( xIS_SET( wield->extra_flags, ITEM_NODUAL ) )
        {
            send_to_char( "You cannot dual wield with your current weapon!\n\r", ch );
            return FALSE;
        }
    }

    return TRUE;
}


/*
    Check to see if there is room to wear another object on this location
    (Layered clothing support)
*/
bool can_layer( CHAR_DATA* ch, OBJ_DATA* obj, sh_int wear_loc )
{
    OBJ_DATA*   otmp;
    sh_int  bitlayers = 0;
    sh_int  objlayers = obj->pIndexData->layers;

    for ( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
        if ( otmp->wear_loc == wear_loc )
        {
            if ( !otmp->pIndexData->layers )
                return FALSE;
            else
                bitlayers |= otmp->pIndexData->layers;
        }

    if ( ( bitlayers && !objlayers ) || bitlayers > objlayers )
        return FALSE;

    if ( !bitlayers || ( ( bitlayers & ~objlayers ) == bitlayers ) )
        return TRUE;

    return FALSE;
}

/*
    Wear one object.
    Optional replacement of existing objects.
    Big repetitive code, ick.
    Restructured a bit to allow for specifying body location -Thoric
*/
void wear_obj( CHAR_DATA* ch, OBJ_DATA* obj, bool fReplace, sh_int wear_bit )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA* tmpobj;
    sh_int bit, tmp;
    bool check_species;
    separate_obj( obj );

    if ( wear_bit > -1 )
    {
        bit = wear_bit;

        if ( !CAN_WEAR( obj, bit ) )
        {
            if ( fReplace )
            {
                switch ( bit )
                {
                    case ITEM_HOLD:
                        send_to_char( "You cannot hold that.\n\r", ch );
                        break;

                    case ITEM_WIELD:
                        send_to_char( "You cannot wield that.\n\r", ch );
                        break;

                    default:
                        sprintf( buf, "You cannot wear that on your %s.\n\r", w_flags[bit] );
                        send_to_char( buf, ch );
                }
            }

            return;
        }
    }
    else
    {
        for ( bit = -1, tmp = 1; tmp < 31; tmp++ )
        {
            if ( CAN_WEAR( obj, tmp ) )
            {
                bit = tmp;
                break;
            }
        }
    }

    check_species = FALSE;

    if ( ch->race != RACE_MARINE && IS_OBJ_STAT( obj, ITEM_MARINE ) )
        check_species = TRUE;

    if ( ch->race != RACE_PREDATOR && IS_OBJ_STAT( obj, ITEM_PREDATOR ) )
        check_species = TRUE;

    if ( ch->race != RACE_ALIEN && IS_OBJ_STAT( obj, ITEM_ALIEN ) )
        check_species = TRUE;

    if ( check_species )
    {
        act( AT_RED, "You cannot use other species items.", ch, NULL, NULL, TO_CHAR );
        act( AT_ACTION, "$n tries to use $p, but can't.", ch, obj, NULL, TO_ROOM );
        return;
    }

    if ( obj->item_type == ITEM_MEDICOMP )
        obj->value[0] = 0;

    /* currently cannot have a light in non-light position */
    if ( obj->item_type == ITEM_LIGHT )
    {
        if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
            return;

        if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
        {
            if ( !obj->action_desc || obj->action_desc[0] == '\0' )
            {
                act( AT_ACTION, "$n holds $p as a light.", ch, obj, NULL, TO_ROOM );
                act( AT_ACTION, "You hold $p as your light.",  ch, obj, NULL, TO_CHAR );
            }
            else
                actiondesc( ch, obj, NULL );
        }

        equip_char( ch, obj, WEAR_LIGHT );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( bit == -1 )
    {
        if ( fReplace )
            send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

        return;
    }

    switch ( bit )
    {
        default:
            bug( "wear_obj: uknown/unused item_wear bit %d", bit );

            if ( fReplace )
                send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

            return;

        case ITEM_WEAR_FINGER:
            if ( get_eq_char( ch, WEAR_FINGER_L )
                    &&   get_eq_char( ch, WEAR_FINGER_R )
                    &&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
                    &&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
                return;

            if ( !get_eq_char( ch, WEAR_FINGER_L ) )
            {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
                    if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                    {
                        act( AT_ACTION, "$n slips $s left finger into $p.",    ch, obj, NULL, TO_ROOM );
                        act( AT_ACTION, "You slip your left finger into $p.",  ch, obj, NULL, TO_CHAR );
                    }
                    else
                        actiondesc( ch, obj, NULL );
                }

                equip_char( ch, obj, WEAR_FINGER_L );
                equip_lag( ch, obj );
                oprog_wear_trigger( ch, obj );
                return;
            }

            if ( !get_eq_char( ch, WEAR_FINGER_R ) )
            {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
                    if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                    {
                        act( AT_ACTION, "$n slips $s right finger into $p.",   ch, obj, NULL, TO_ROOM );
                        act( AT_ACTION, "You slip your right finger into $p.", ch, obj, NULL, TO_CHAR );
                    }
                    else
                        actiondesc( ch, obj, NULL );
                }

                equip_char( ch, obj, WEAR_FINGER_R );
                equip_lag( ch, obj );
                oprog_wear_trigger( ch, obj );
                return;
            }

            bug( "Wear_obj: no free finger.", 0 );
            send_to_char( "You already wear something on both fingers.\n\r", ch );
            return;

        case ITEM_WEAR_NECK:
            if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
                    &&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
                    &&   !remove_obj( ch, WEAR_NECK_1, fReplace )
                    &&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
                return;

            if ( !get_eq_char( ch, WEAR_NECK_1 ) )
            {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
                    if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                    {
                        act( AT_ACTION, "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
                        act( AT_ACTION, "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
                    }
                    else
                        actiondesc( ch, obj, NULL );
                }

                equip_char( ch, obj, WEAR_NECK_1 );
                equip_lag( ch, obj );
                oprog_wear_trigger( ch, obj );
                return;
            }

            if ( !get_eq_char( ch, WEAR_NECK_2 ) )
            {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
                    if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                    {
                        act( AT_ACTION, "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
                        act( AT_ACTION, "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
                    }
                    else
                        actiondesc( ch, obj, NULL );
                }

                equip_char( ch, obj, WEAR_NECK_2 );
                equip_lag( ch, obj );
                oprog_wear_trigger( ch, obj );
                return;
            }

            bug( "Wear_obj: no free neck.", 0 );
            send_to_char( "You already wear two neck items.\n\r", ch );
            return;

        case ITEM_WEAR_BODY:

            /*
                if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
                  return;
            */
            if ( !can_layer( ch, obj, WEAR_BODY ) )
            {
                send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
                return;
            }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n fits $p on $s body.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You fit $p on your body.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_BODY );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_HEAD:
            if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
                return;

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n dons $p upon $s head.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You don $p upon your head.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_HEAD );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_EYES:
            if ( !remove_obj( ch, WEAR_EYES, fReplace ) )
                return;

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n places $p on $s eyes.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You place $p on your eyes.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_EYES );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_EARS:
            if ( !remove_obj( ch, WEAR_EARS, fReplace ) )
                return;

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n wears $p on $s ears.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You wear $p on your ears.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_EARS );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_LEGS:

            /*
                    if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
                      return;
            */
            if ( !can_layer( ch, obj, WEAR_LEGS ) )
            {
                send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
                return;
            }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n wears $p.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You wear $p.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_LEGS );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_FEET:

            /*
                    if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
                      return;
            */
            if ( !can_layer( ch, obj, WEAR_FEET ) )
            {
                send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
                return;
            }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_FEET );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_HANDS:

            /*
                    if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
                      return;
            */
            if ( !can_layer( ch, obj, WEAR_HANDS ) )
            {
                send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
                return;
            }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_HANDS );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_SHOULDER:

            /*
                    if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
                      return;
            */
            if ( !can_layer( ch, obj, WEAR_SHOULDER ) )
            {
                send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
                return;
            }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n mounts $p on $s shoulder.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You mount $p on your shoulder.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_SHOULDER );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_ARMS:

            /*
                    if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
                      return;
            */
            if ( !can_layer( ch, obj, WEAR_ARMS ) )
            {
                send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
                return;
            }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_ARMS );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_ABOUT:

            /*
                if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
                  return;
            */
            if ( !can_layer( ch, obj, WEAR_ABOUT ) )
            {
                send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
                return;
            }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n wears $p about $s body.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_ABOUT );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_OVER:

            /*
                if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
                  return;
            */
            if ( !can_layer( ch, obj, WEAR_OVER ) )
            {
                send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
                return;
            }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n wears $p over $s body.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You wear $p over your body.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_OVER );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_WAIST:

            /*
                    if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
                      return;
            */
            if ( !can_layer( ch, obj, WEAR_WAIST ) )
            {
                send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
                return;
            }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_WAIST );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WEAR_WRIST:
            if ( get_eq_char( ch, WEAR_WRIST_L )
                    &&   get_eq_char( ch, WEAR_WRIST_R )
                    &&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
                    &&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
                return;

            if ( !get_eq_char( ch, WEAR_WRIST_L ) )
            {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
                    if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                    {
                        act( AT_ACTION, "$n fits $p around $s left wrist.",
                             ch, obj, NULL, TO_ROOM );
                        act( AT_ACTION, "You fit $p around your left wrist.",
                             ch, obj, NULL, TO_CHAR );
                    }
                    else
                        actiondesc( ch, obj, NULL );
                }

                equip_char( ch, obj, WEAR_WRIST_L );
                equip_lag( ch, obj );
                oprog_wear_trigger( ch, obj );
                return;
            }

            if ( !get_eq_char( ch, WEAR_WRIST_R ) )
            {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
                    if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                    {
                        act( AT_ACTION, "$n fits $p around $s right wrist.",
                             ch, obj, NULL, TO_ROOM );
                        act( AT_ACTION, "You fit $p around your right wrist.",
                             ch, obj, NULL, TO_CHAR );
                    }
                    else
                        actiondesc( ch, obj, NULL );
                }

                equip_char( ch, obj, WEAR_WRIST_R );
                equip_lag( ch, obj );
                oprog_wear_trigger( ch, obj );
                return;
            }

            bug( "Wear_obj: no free wrist.", 0 );
            send_to_char( "You already wear two wrist items.\n\r", ch );
            return;

        case ITEM_WEAR_SHIELD:
            if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
                return;

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    act( AT_ACTION, "$n uses $p as an combat shield.", ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "You use $p as an combat shield.", ch, obj, NULL, TO_CHAR );
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_SHIELD );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_WIELD:
            if ( ( tmpobj = get_eq_char( ch, WEAR_HOLD ) ) != NULL )
            {
                if ( xIS_SET( obj->extra_flags, ITEM_NOHOLD ) )
                {
                    // Just remove the held item.
                    remove_obj( ch, tmpobj->wear_loc, TRUE );
                }
            }

            if ( ( tmpobj = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
            {
                if ( can_dual( ch ) && get_eq_char( ch, WEAR_WIELD ) != NULL )
                {
                    if ( xIS_SET( obj->extra_flags, ITEM_NODUAL ) )
                    {
                        send_to_char( "You cannot dual wield that with your current weapon!\n\r", ch );
                        return;
                    }

                    if ( get_obj_weight( obj ) + get_obj_weight( tmpobj ) > get_curr_str( ch ) )
                    {
                        send_to_char( "It is too heavy for you to dual wield.\n\r", ch );
                        return;
                    }

                    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                    {
                        if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                        {
                            if ( obj->ammo && obj->value[0] != WEAPON_FLAMETHROWER )
                            {
                                sprintf( buf, "$n dual-wields $p. &w&z(&W%d rounds loaded&z)", obj->ammo->value[2] );
                                act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );
                                sprintf( buf, "You dual-wield $p. &w&z(&W%d rounds loaded&z)", obj->ammo->value[2] );
                                act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
                            }
                            else
                            {
                                act( AT_ACTION, "$n dual-wields $p.", ch, obj, NULL, TO_ROOM );
                                act( AT_ACTION, "You dual-wield $p.", ch, obj, NULL, TO_CHAR );
                            }
                        }
                        else
                            actiondesc( ch, obj, NULL );
                    }

                    equip_char( ch, obj, WEAR_DUAL_WIELD );
                    equip_lag( ch, obj );
                    oprog_wear_trigger( ch, obj );
                }

                return;
            }

            if ( get_obj_weight( obj ) > get_curr_str( ch ) )
            {
                send_to_char( "It is too heavy for you to wield.\n\r", ch );
                return;
            }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                if ( !obj->action_desc || obj->action_desc[0] == '\0' )
                {
                    if ( obj->ammo && obj->value[0] != WEAPON_FLAMETHROWER )
                    {
                        sprintf( buf, "$n wields $p. &w&z(&W%d rounds loaded&z)", obj->ammo->value[2] );
                        act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );
                        sprintf( buf, "You wield $p. &w&z(&W%d rounds loaded&z)", obj->ammo->value[2] );
                        act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
                    }
                    else
                    {
                        act( AT_ACTION, "$n wields $p.", ch, obj, NULL, TO_ROOM );
                        act( AT_ACTION, "You wield $p.", ch, obj, NULL, TO_CHAR );
                    }
                }
                else
                    actiondesc( ch, obj, NULL );
            }

            equip_char( ch, obj, WEAR_WIELD );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;

        case ITEM_HOLD:
            if ( ( tmpobj = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
            {
                if ( xIS_SET( tmpobj->extra_flags, ITEM_NOHOLD ) )
                {
                    ch_printf( ch, "&RYou cannot hold anything with a weapon that large.\n\r" );
                    return;
                }
            }

            if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
            {
                send_to_char( "You cannot hold something while dual wielding.\n\r", ch );
                return;
            }

            if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
                return;

            if ( obj->item_type == ITEM_GRENADE
                    || obj->item_type == ITEM_FOOD
                    || obj->item_type == ITEM_DRINK_CON
                    || obj->item_type == ITEM_KEY
                    || !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
                act( AT_ACTION, "$n holds $p in $s hands.",   ch, obj, NULL, TO_ROOM );
                act( AT_ACTION, "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
            }

            equip_char( ch, obj, WEAR_HOLD );
            equip_lag( ch, obj );
            oprog_wear_trigger( ch, obj );
            return;
    }
}


void do_wear( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    sh_int wear_bit;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( !str_cmp( arg2, "on" )  || !str_cmp( arg2, "upon" ) || !str_cmp( arg2, "around" ) )
            &&   argument[0] != '\0' )
        argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Wear, wield, or hold what?\n\r", ch );
        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    if ( !str_cmp( arg1, "all" ) )
    {
        OBJ_DATA* obj_next;

        for ( obj = ch->first_carrying; obj; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
                wear_obj( ch, obj, FALSE, -1 );
        }

        return;
    }
    else
    {
        if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        if ( arg2[0] != '\0' )
            wear_bit = get_wflag( arg2 );
        else
            wear_bit = -1;

        wear_obj( ch, obj, TRUE, wear_bit );
    }

    return;
}



void do_remove( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj, *obj_next;
    one_argument( argument, arg );

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove what?\n\r", ch );
        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    if ( !str_cmp( arg, "all" ) )  /* SB Remove all */
    {
        for ( obj = ch->first_carrying; obj != NULL ; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( obj->wear_loc != WEAR_NONE && can_see_obj ( ch, obj ) )
                remove_obj ( ch, obj->wear_loc, TRUE );
        }

        return;
    }

    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
        send_to_char( "You are not using that item.\n\r", ch );
        return;
    }

    if ( ( obj_next = get_eq_char( ch, obj->wear_loc ) ) != obj )
    {
        act( AT_PLAIN, "You must remove $p first.", ch, obj_next, NULL, TO_CHAR );
        return;
    }

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}


void do_bury( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    sh_int move;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "What do you wish to bury?\n\r", ch );
        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );

    if ( !obj )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }

    separate_obj( obj );

    if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    {
        act( AT_PLAIN, "You cannot bury $p.", ch, obj, 0, TO_CHAR );
        return;
    }

    switch ( ch->in_room->sector_type )
    {
        case SECT_CITY:
        case SECT_INSIDE:
            send_to_char( "The floor is too hard to dig through.\n\r", ch );
            return;

        case SECT_WATER_SWIM:
        case SECT_WATER_NOSWIM:
        case SECT_UNDERWATER:
            send_to_char( "You cannot bury something here.\n\r", ch );
            return;

        case SECT_AIR:
            send_to_char( "What?  In the air?!\n\r", ch );
            return;
    }

    move = ( obj->weight * 50 * 3 ) / UMAX( 1, can_carry_w( ch ) );
    move = URANGE( 2, move, 1000 );

    if ( move > ch->move )
    {
        send_to_char( "You don't have the energy to bury something of that size.\n\r", ch );
        return;
    }

    ch->move -= move;
    act( AT_ACTION, "You solemnly bury $p...", ch, obj, NULL, TO_CHAR );
    act( AT_ACTION, "$n solemnly buries $p...", ch, obj, NULL, TO_ROOM );
    xSET_BIT( obj->extra_flags, ITEM_BURRIED );
    WAIT_STATE( ch, URANGE( 10, move / 2, 100 ) );
    return;
}

void do_sacrifice( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    one_argument( argument, arg );

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
        act( AT_ACTION, "$n offers $mself to $s deity, who graciously declines.",
             ch, NULL, NULL, TO_ROOM );
        send_to_char( "Your deity appreciates your offer and may accept it later.", ch );
        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );

    if ( !obj )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }

    separate_obj( obj );

    if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    {
        act( AT_PLAIN, "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
        return;
    }

    oprog_sac_trigger( ch, obj );

    if ( obj_extracted( obj ) )
        return;

    if ( cur_obj == obj->serial )
        global_objcode = rOBJ_SACCED;

    extract_obj( obj );
    return;
}

void do_junk( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj, *obj_next = NULL;
    int cnt = 0;
    one_argument( argument, arg );

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "&RSyntax: JUNK <object | ALL>\n\r", ch );
        send_to_char( "&RMake sure you want to destroy the object first. Junk cannot be undone.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        for ( obj = ch->in_room->first_content; obj; obj = obj_next )
        {
            obj_next = obj->next_content;
            separate_obj( obj );

            if ( !CAN_WEAR( obj, ITEM_TAKE ) )
                continue;

            if ( obj->item_type == ITEM_GRENADE && obj->value[4] > 0 )
                continue;

            if ( obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_CORPSE_PC )
                continue;

            if ( contains_explosive( obj, 0 ) )
                continue;

            if ( obj->item_type == ITEM_SCRAPS )
                continue;

            act( AT_PLAIN, "You junk $p.", ch, obj, 0, TO_CHAR );
            cnt++;
            oprog_sac_trigger( ch, obj );

            if ( obj_extracted( obj ) )
                return;

            if ( cur_obj == obj->serial )
                global_objcode = rOBJ_SACCED;

            extract_obj( obj );
        }

        if ( cnt <= 0 )
        {
            send_to_char( "&RTheres nothing in here you can junk.\n\r", ch );
        }

        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );

    if ( !obj )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }

    separate_obj( obj );

    if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    {
        act( AT_PLAIN, "$p cannot be junked.", ch, obj, 0, TO_CHAR );
        return;
    }

    if ( obj->item_type == ITEM_GRENADE && obj->value[4] > 0 )
    {
        act( AT_PLAIN, "You cannot junk an armed grenade!", ch, obj, 0, TO_CHAR );
        return;
    }

    if ( obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_CORPSE_PC )
    {
        act( AT_PLAIN, "You cannot junk bodily remains!", ch, obj, 0, TO_CHAR );
        return;
    }

    if ( contains_explosive( obj, 0 ) )
    {
        act( AT_PLAIN, "You cannot junk an item containing an armed grenade!", ch, obj, 0, TO_CHAR );
        return;
    }

    if ( obj->item_type == ITEM_SCRAPS )
    {
        act( AT_PLAIN, "You cannot junk any scraps, they decay on their own!", ch, obj, 0, TO_CHAR );
        return;
    }

    act( AT_PLAIN, "You junk $p.", ch, obj, 0, TO_CHAR );
    oprog_sac_trigger( ch, obj );

    if ( obj_extracted( obj ) )
        return;

    if ( cur_obj == obj->serial )
        global_objcode = rOBJ_SACCED;

    extract_obj( obj );

    /* SAVE_EQ room checks */
    if ( if_equip_room( ch->in_room ) )
        save_equip_room( ch, ch->in_room );

    return;
}

void do_smash( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char bufA[MAX_INPUT_LENGTH];
    char bufB[MAX_INPUT_LENGTH];
    OBJ_DATA* obj, *obj_next = NULL;
    int cnt = 0;
    one_argument( argument, arg );

    if ( ch->race == RACE_MARINE && !IS_IMMORTAL( ch ) )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "&RSyntax: SMASH (object)\n\r", ch );
        return;
    }

    if ( ms_find_obj( ch ) )
        return;

    obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );

    if ( !obj )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }

    separate_obj( obj );

    /*
        if ( !CAN_WEAR(obj, ITEM_TAKE) )
        {
        act( AT_PLAIN, "$p cannot be smashed.", ch, obj, 0, TO_CHAR );
        return;
        }
    */

    if ( IS_OBJ_STAT( obj, ITEM_INDESTRUCTABLE ) )
    {
        act( AT_PLAIN, "I really dont think smashing that is going to work.", ch, obj, 0, TO_CHAR );
        return;
    }

    if ( obj->item_type == ITEM_GRENADE && obj->value[4] > 0 )
    {
        act( AT_PLAIN, "You cannot smash an armed grenade!", ch, obj, 0, TO_CHAR );
        return;
    }

    if ( obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_CORPSE_PC )
    {
        act( AT_PLAIN, "You cannot smash bodily remains!", ch, obj, 0, TO_CHAR );
        return;
    }

    if ( contains_explosive( obj, 0 ) )
    {
        act( AT_PLAIN, "You cannot smash an item containing an armed grenade!", ch, obj, 0, TO_CHAR );
        return;
    }

    if ( obj->item_type == ITEM_SCRAPS )
    {
        act( AT_PLAIN, "You cannot smash any scraps, they decay on their own!", ch, obj, 0, TO_CHAR );
        return;
    }

    act( AT_CYAN, "&w&CYou slash at $p, damaging it!", ch, obj, 0, TO_CHAR );
    act( AT_CYAN, "&w&C$n slashes at $p, damaging it!", ch, obj, 0, TO_ROOM );
    sprintf( bufA, "&w&RYou have destroyed %s.\n\r", obj->short_descr );
    sprintf( bufB, "&w&YDamn, you didn't destroy %s.\n\r", obj->short_descr );

    if ( damage_obj( obj, ( get_curr_str( ch ) * 2 ) ) == rOBJ_SCRAPPED )
    {
        ch_printf( ch, bufA );
    }
    else
    {
        ch_printf( ch, bufB );
    }

    WAIT_STATE( ch, PULSE_PER_SECOND );
    return;
}

/*
    Save items in a SAVE_EQ room (Code by Ghost)
*/
void save_equip_room( CHAR_DATA* ch, ROOM_INDEX_DATA* room )
{
    FILE* fp;
    char filename[256];
    sh_int templvl;
    OBJ_DATA* contents;

    if ( !room )
    {
        bug( "save_equip_room: Null room pointer!", 0 );
        return;
    }

    if ( !ch )
    {
        bug ( "save_equip_room: Null ch pointer!", 0 );
        return;
    }

    // bug( "Saving equipment room at %d...", room->vnum );
    sprintf( filename, "%s%d", EQ_DIR, room->vnum );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "save_equip_room: fopen", 0 );
        perror( filename );
    }
    else
    {
        if ( room->first_content == NULL )
        {
            fclose( fp );
            remove( filename );
            return;
        }

        templvl = ch->top_level;
        ch->top_level = LEVEL_HERO;     /* make sure EQ doesn't get lost */
        contents = room->last_content;

        if ( contents )
            fwrite_obj( ch, contents, fp, 0, OS_CARRY );

        fprintf( fp, "#END\n" );
        ch->top_level = templvl;
        fclose( fp );
        return;
    }

    return;
}

/*
    Load the EQ-Saving rooms
*/
void load_equip_rooms( void )
{
    char buf[MAX_STRING_LENGTH];
    DIR* dp;
    struct dirent* de;
    extern FILE* fpArea;
    extern char strArea[MAX_INPUT_LENGTH];
    extern int falling;
    int rID = 0, cnt = 0;

    if ( !( dp = opendir( EQ_DIR ) ) )
    {
        bug( "Load_equip_rooms: can't open EQ_DIR", 0 );
        perror( EQ_DIR );
        return;
    }

    falling = 1;

    while ( ( de = readdir( dp ) ) != NULL )
    {
        if ( de->d_name[0] != '.' )
        {
            sprintf( strArea, "%s%s", EQ_DIR, de->d_name );
            rID = atoi( de->d_name );

            if ( rID <= 0 )
                continue;

            if ( ( fpArea = fopen( strArea, "r" ) ) != NULL )
            {
                int iNest;
                bool found;
                OBJ_DATA* tobj, *tobj_next;
                rset_supermob( get_room_index( rID ) );

                for ( iNest = 0; iNest < MAX_NEST; iNest++ )
                    rgObjNest[iNest] = NULL;

                found = TRUE;

                for ( ; ; )
                {
                    char letter;
                    char* word;
                    letter = fread_letter( fpArea );

                    if ( letter == '*' )
                    {
                        fread_to_eol( fpArea );
                        continue;
                    }

                    if ( letter != '#' )
                    {
                        bug( "Load_equip_rooms: # not found.", 0 );
                        break;
                    }

                    word = fread_word( fpArea );

                    if ( !str_cmp( word, "OBJECT" ) )       /* Objects      */
                        fread_obj  ( supermob, fpArea, OS_CARRY );
                    else if ( !str_cmp( word, "END"    ) )  /* Done         */
                        break;
                    else
                    {
                        bug( "Load_equip_rooms: bad section.", 0 );
                        break;
                    }
                }

                fclose( fpArea );

                for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
                {
                    tobj_next = tobj->next_content;
                    obj_from_char( tobj );
                    obj_to_room( tobj, get_room_index( rID ) );
                }

                release_supermob();
                cnt++;
            }
            else
                log_string( "Cannot open equipment saving room." );
        }
    }

    sprintf( buf, "Loaded %d equipment rooms.", cnt );
    log_string( buf );
    fpArea = NULL;
    strcpy( strArea, "$" );
    closedir( dp );
    falling = 0;
    return;
}

/* Make objects in rooms that are nofloor fall - Scryn 1/23/96 */

void obj_fall( OBJ_DATA* obj, bool through )
{
    EXIT_DATA* pexit;
    ROOM_INDEX_DATA* to_room;
    static int fall_count;
    char buf[MAX_STRING_LENGTH];
    static bool is_falling; /* Stop loops from the call to obj_to_room()  -- Altrag */

    if ( !obj->in_room || is_falling )
        return;

    if ( fall_count > 30 )
    {
        bug( "object falling in loop more than 30 times", 0 );
        extract_obj( obj );
        fall_count = 0;
        return;
    }

    if ( xIS_SET( obj->in_room->room_flags, ROOM_NOFLOOR ) && CAN_GO( obj, DIR_DOWN ) )
    {
        pexit = get_exit( obj->in_room, DIR_DOWN );
        to_room = pexit->to_room;

        if ( through )
            fall_count++;
        else
            fall_count = 0;

        if ( obj->in_room == to_room )
        {
            sprintf( buf, "Object falling into same room, room %d", to_room->vnum );
            bug( buf, 0 );
            extract_obj( obj );
            return;
        }

        if ( obj->in_room->first_person )
        {
            act( AT_PLAIN, "$p falls far below...",
                 obj->in_room->first_person, obj, NULL, TO_ROOM );
            act( AT_PLAIN, "$p falls far below...",
                 obj->in_room->first_person, obj, NULL, TO_CHAR );
        }

        obj_from_room( obj );
        is_falling = TRUE;
        obj = obj_to_room( obj, to_room );
        is_falling = FALSE;

        if ( obj->in_room->first_person )
        {
            act( AT_PLAIN, "$p falls from above...", obj->in_room->first_person, obj, NULL, TO_ROOM );
            act( AT_PLAIN, "$p falls from above...", obj->in_room->first_person, obj, NULL, TO_CHAR );
        }

        if ( !xIS_SET( obj->in_room->room_flags, ROOM_NOFLOOR ) && through )
        {
            /*      int dam = (int)9.81*sqrt(fall_count*2/9.81)*obj->weight/2;
            */      int dam = fall_count * obj->weight / 2;

            /* Damage players */
            if ( obj->in_room->first_person && number_percent() > 15 )
            {
                CHAR_DATA* rch;
                CHAR_DATA* vch = NULL;
                int chcnt = 0;

                for ( rch = obj->in_room->first_person; rch;
                        rch = rch->next_in_room, chcnt++ )
                    if ( number_range( 0, chcnt ) == 0 )
                        vch = rch;

                act( AT_WHITE, "$p falls on $n!", vch, obj, NULL, TO_ROOM );
                act( AT_WHITE, "$p falls on you!", vch, obj, NULL, TO_CHAR );
                damage( vch, vch, dam * vch->top_level, TYPE_UNDEFINED );
            }

            /* Damage objects */
            switch ( obj->item_type )
            {
                case ITEM_WEAPON:
                case ITEM_ARMOR:
                    if ( ( obj->value[0] - dam ) <= 0 )
                    {
                        OBJ_DATA* scraps;

                        if ( obj->in_room->first_person )
                        {
                            act( AT_PLAIN, "$p is destroyed by the fall!",
                                 obj->in_room->first_person, obj, NULL, TO_ROOM );
                            act( AT_PLAIN, "$p is destroyed by the fall!",
                                 obj->in_room->first_person, obj, NULL, TO_CHAR );
                        }

                        scraps = make_scraps( obj );
                    }
                    else
                        obj->value[0] -= dam;

                    break;

                default:
                    if ( ( dam * 15 ) > get_obj_resistance( obj ) )
                    {
                        OBJ_DATA* scraps;

                        if ( obj->in_room->first_person )
                        {
                            act( AT_PLAIN, "$p is destroyed by the fall!",
                                 obj->in_room->first_person, obj, NULL, TO_ROOM );
                            act( AT_PLAIN, "$p is destroyed by the fall!",
                                 obj->in_room->first_person, obj, NULL, TO_CHAR );
                        }

                        scraps = make_scraps( obj );
                    }

                    break;
            }
        }

        obj_fall( obj, TRUE );
    }

    return;
}

void detonate_traps( CHAR_DATA* ch, ROOM_INDEX_DATA* room )
{
    OBJ_DATA* robj, * robj_next;
    AFFECT_DATA af;
    int i;

    if ( ch == NULL )
        return;

    if ( room == NULL )
        return;

    if ( is_spectator( ch ) )
        return;

    for ( robj = room->first_content; robj; robj = robj_next )
    {
        robj_next = robj->next_content;
        set_cur_obj( robj );

        if ( robj->item_type == ITEM_LANDMINE )
        {
            if ( ( ch->race + 1 ) == robj->value[3] )
                continue;

            if ( ( ch->race + 1 ) != robj->value[2] && robj->value[2] > 0 )
                continue;

            if ( IS_OBJ_STAT( robj, ITEM_BURRIED ) )
                explode( robj );
        }
        else if ( robj->item_type == ITEM_TRAP )
        {
            if ( !IS_OBJ_STAT( robj, ITEM_ALIEN ) && ch->race == RACE_ALIEN )
                continue;

            if ( !IS_OBJ_STAT( robj, ITEM_MARINE ) && ch->race == RACE_MARINE )
                continue;

            if ( !IS_OBJ_STAT( robj, ITEM_PREDATOR ) && ch->race == RACE_PREDATOR )
                continue;

            if ( robj->value[0] == 6 ) // Entangle
            {
                ch_printf( ch, "&w&C(Entangled) Your progress has been slowed by the trap.\n\r" );
                ch->mp = -2;
            }

            if ( robj->value[0] == 7 ) // Damage
            {
                ch_printf( ch, "&w&C(Damage Trap) You have triggered %s.\n\r", robj->short_descr );

                if ( robj->parent )
                {
                    if ( !IS_NPC( robj->parent ) )
                        robj->parent->pcdata->prepared[gsn_battle_method]--;

                    damage( robj->parent, ch, robj->value[2], TYPE_GENERIC + robj->value[1] );

                    if ( !IS_NPC( robj->parent ) )
                        robj->parent->pcdata->prepared[gsn_battle_method]++;
                }
                else
                    damage( ch, ch, robj->value[2], TYPE_GENERIC + robj->value[1] );
            }
            else if ( robj->value[0] == 5 ) // Ambush Traps
            {
                CHAR_DATA* mob;

                for ( i = 0; i < robj->value[2]; ++i )
                {
                    if ( get_mob_index( robj->value[1] ) == NULL )
                        continue;

                    mob = create_mobile( get_mob_index( robj->value[1] ) );
                    char_to_room( mob, ch->in_room );
                    act( AT_IMMORT, "$N has arrived.", mob, NULL, mob, TO_ROOM );
                    // if ( robj->parent ) mob->master = robj->parent;
                    start_hating( mob, ch );
                    start_hunting( mob, ch );
                    motion_ping( room->x, room->y, room->z, room->area, NULL );
                }
            }
            else if ( robj->value[0] == 3 ) // Acid Trap
            {
                ch_printf( ch, "&w&G(Acid Trap) You have triggered an Acid Trap! Your blinded!\n\r" );
                af.type      = gsn_acidspit;
                af.location  = APPLY_PER;
                af.modifier  = -10;
                af.duration  = UMAX( 5, robj->value[1] );
                xCLEAR_BITS( af.bitvector );
                xSET_BIT( af.bitvector, AFF_BLIND );
                affect_join( ch, &af );
            }

            if ( robj->value[5] > 0 )
            {
                if ( --robj->value[5] == 0 )
                    extract_obj( robj );
            }
        }

        if ( is_spectator( ch ) )
            break;
    }

    return;
}

int count_traps( ROOM_INDEX_DATA* room )
{
    OBJ_DATA* robj = NULL;
    int cnt = 0;

    if ( room == NULL )
        return 0;

    for ( robj = room->first_content; robj; robj = robj->next_content )
    {
        if ( robj->item_type == ITEM_LANDMINE )
        {
            if ( IS_OBJ_STAT( robj, ITEM_BURRIED ) )
                cnt++;
        }
        else if ( robj->item_type == ITEM_TRAP )
        {
            cnt++;
        }
    }

    return cnt;
}

void clear_traps( ROOM_INDEX_DATA* room )
{
    OBJ_DATA* robj = NULL;
    OBJ_DATA* robj_next = NULL;

    if ( room == NULL )
        return;

    for ( robj = room->first_content; robj; robj = robj_next )
    {
        robj_next = robj->next_content;

        if ( robj->item_type == ITEM_LANDMINE )
        {
            if ( IS_OBJ_STAT( robj, ITEM_BURRIED ) )
                extract_obj( robj );
        }
        else if ( robj->item_type == ITEM_TRAP )
        {
            extract_obj( robj );
        }
    }

    return;
}


void equip_lag( CHAR_DATA* ch, OBJ_DATA* obj )
{
    int lag = 0;

    if ( ch == NULL )
        return;

    if ( obj == NULL )
        return;

    if ( obj->wear_loc == WEAR_WIELD || obj->wear_loc == WEAR_DUAL_WIELD )
    {
        /* Wielded weapons have the largest affect */
        lag = URANGE( 0, ( int )( ( float )( get_obj_weight( obj ) ) / ( float )( 5 ) ), 5 );
    }
    else
    {
        if ( get_obj_weight( obj ) >= 8 )
            lag = URANGE( 0, ( int )( ( float )( get_obj_weight( obj ) ) / ( float )( 8 ) ), 5 );
    }

    if ( obj->wear_loc == WEAR_SHOULDER )
        lag = 3;

    if ( lag > 0 )
    {
        ch_printf( ch, "&C(Equipment Lag) It will take %d round%s to equip that.\n\r", lag, lag == 1 ? "" : "s" );
        WAIT_STATE( ch, lag * 4 );
    }

    return;
}

void unequip_lag( CHAR_DATA* ch, OBJ_DATA* obj )
{
    return;
}
