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
                 Informational module
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

ROOM_INDEX_DATA* generate_exit( ROOM_INDEX_DATA* in_room, EXIT_DATA** pexit );

char        conv_result[MAX_STRING_LENGTH];   /* Color Token Filtering */

char*   const   where_name  [] =
{
    "&B<&Cused as light&B>     ",
    "&B<&Cworn on finger&B>    ",
    "&B<&Cworn on finger&B>    ",
    "&B<&Cworn around neck&B>  ",
    "&B<&Cworn around neck&B>  ",
    "&B<&Cworn on body&B>      ",
    "&B<&Cworn on head&B>      ",
    "&B<&Cworn on legs&B>      ",
    "&B<&Cworn on feet&B>      ",
    "&B<&Cworn on hands&B>     ",
    "&B<&Cworn on arms&B>      ",
    "&B<&Ccombat shield&B>     ",
    "&B<&Cworn about body&B>   ",
    "&B<&Cworn about waist&B>  ",
    "&B<&Cworn around wrist&B> ",
    "&B<&Cworn around wrist&B> ",
    "&B<&Cwielded&B>           ",
    "&B<&Cheld&B>              ",
    "&B<&Cdual wielded&B>      ",
    "&B<&Cworn on ears&B>      ",
    "&B<&Cworn on eyes&B>      ",
    "&B<&Cmissile wielded&B>   ",
    "&B<&Cworn over body&B>    ",
    "&B<&Cworn on back&B>      ",
    //    "&B<&Cworn over face&B>    ",
    //    "&B<&Cworn around ankle&B> ",
    //    "&B<&Cworn around ankle&B> ",
    "&B<&Cworn on shoulder&B>  ",
};


/*
    Local functions.
*/
void    show_char_to_char_0 args( ( CHAR_DATA* victim, CHAR_DATA* ch ) );
void    show_char_to_char_1 args( ( CHAR_DATA* victim, CHAR_DATA* ch ) );
void    show_char_to_char   args( ( CHAR_DATA* list, CHAR_DATA* ch ) );
void    show_char_to_char   args( ( CHAR_DATA* list, CHAR_DATA* ch ) );
bool    check_blind     args( ( CHAR_DATA* ch ) );
void    show_condition          args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
sh_int  str_similarity( const char* astr, const char* bstr );
sh_int  str_prefix_level( const char* astr, const char* bstr );
void    similar_help_files( CHAR_DATA* ch, char* argument );

char* format_obj_to_char( OBJ_DATA* obj, CHAR_DATA* ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];
    SENTRY_DATA* sentry = NULL;
    sentry = get_sentry( obj );
    buf[0] = '\0';

    // Must be at top
    if ( xIS_SET( obj->extra_flags, ITEM_RESPAWN ) )
        strcat( buf, "&w&B" );

    if ( obj->item_type == ITEM_AMMO )
    {
        if ( obj->value[2] <= 0 )
            strcat( buf, "&w&R" );
    }

    if ( obj->item_type == ITEM_SPAWNER )
    {
        if ( obj->value[0] <= 0 )
            strcat( buf, "&w&R" );
    }

    if ( sentry != NULL )
        strcat( buf, "&w&r(&RArmed&r) &G" );

    if ( obj->item_type == ITEM_COVER )
        strcat( buf, "&w&z(&CCover&z) &G" );

    if ( obj->item_type == ITEM_FLARE )
        if ( obj->value[2] != 0 )
            strcat( buf, "(Burning) " );

    if ( xIS_SET( obj->extra_flags, ITEM_INVIS ) )
        strcat( buf, "(Invis) "     );

    if ( xIS_SET( obj->extra_flags, ITEM_ALIENINVIS ) )
        strcat( buf, "(Alien Invis) " );

    if ( xIS_SET( obj->extra_flags, ITEM_GLOW ) )
        strcat( buf, "(Glowing) "   );

    if ( xIS_SET( obj->extra_flags, ITEM_HUM ) )
        strcat( buf, "(Humming) "   );

    if ( xIS_SET( obj->extra_flags, ITEM_HIDDEN ) )
        strcat( buf, "(Hidden) "    );

    if ( xIS_SET( obj->extra_flags, ITEM_BURRIED ) )
        strcat( buf, "(Burried) "   );

    if ( IS_IMMORTAL( ch ) )
    {
        if ( xIS_SET( obj->extra_flags, ITEM_PROTOTYPE ) )
            strcat( buf, "(Prototype)" );
    }

    if ( fShort )
    {
        if ( obj->short_descr )
            strcat( buf, obj->short_descr );
    }
    else
    {
        if ( obj->description )
            strcat( buf, obj->description );
    }

    if ( sentry != NULL )
    {
        strcat( buf, "(" );
        strcat( buf, capitalize( dir_name[sentry->arc] ) );
        strcat( buf, ")" );
    }

    return buf;
}

/*  This is the punct snippet from Desden el Chaman Tibetano - Nov 1998
    Email: jlalbatros@mx2.redestb.es
*/
char* num_punct( int foo )
{
    unsigned int index;
    int index_new, rest;
    char buf[16];
    static char buf_new[16];
    sprintf( buf, "%d", foo );
    rest = strlen( buf ) % 3;

    for ( index = index_new = 0; index < strlen( buf ); index++, index_new++ )
    {
        if ( index != 0 && ( index - rest ) % 3 == 0 )
        {
            buf_new[index_new] = ',';
            index_new++;
            buf_new[index_new] = buf[index];
        }
        else
            buf_new[index_new] = buf[index];
    }

    buf_new[index_new] = '\0';
    return strdup( buf_new );
}


/*
    Show a list to a character.
    Can coalesce duplicated items.
*/
void show_list_to_char( OBJ_DATA* list, CHAR_DATA* ch, bool fShort, bool fShowNothing )
{
    char** prgpstrShow;
    int* prgnShow;
    int* pitShow;
    char* pstrShow;
    OBJ_DATA* obj;
    int nShow;
    int iShow;
    int count, offcount, tmp, ms, cnt;
    bool fCombine;
    offcount = 0;

    if ( !ch->desc )
        return;

    /*
        if there's no list... then don't do all this crap!  -Thoric
    */
    if ( !list )
    {
        if ( fShowNothing )
        {
            if ( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
                send_to_char( "     ", ch );

            set_char_color( AT_OBJECT, ch );
            send_to_char( "Nothing.\n\r", ch );
        }

        return;
    }

    /*
        Alloc space for output lines.
    */
    count = 0;

    for ( obj = list; obj; obj = obj->next_content )
        count++;

    ms  = ( ch->mental_state ? ch->mental_state : 1 );

    if ( count <= 0 )
    {
        if ( fShowNothing )
        {
            if ( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
                send_to_char( "     ", ch );

            set_char_color( AT_OBJECT, ch );
            send_to_char( "Nothing.\n\r", ch );
        }

        return;
    }

    CREATE( prgpstrShow,        char*,  count );
    CREATE( prgnShow,           int,    count );
    CREATE( pitShow,            int,    count );
    nShow   = 0;
    tmp         = 0;
    cnt         = 0;

    /*
        Format the list of objects.
    */
    for ( obj = list; obj; obj = obj->next_content )
    {
        if ( offcount < 0 && ++cnt > count + offcount )
            break;

        if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
        {
            pstrShow = format_obj_to_char( obj, ch, fShort );
            fCombine = FALSE;

            if ( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
            {
                /*
                    Look for duplicates, case sensitive.
                    Matches tend to be near end so run loop backwords.
                */
                for ( iShow = nShow - 1; iShow >= 0; iShow-- )
                {
                    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
                    {
                        prgnShow[iShow] += obj->count;
                        fCombine = TRUE;
                        break;
                    }
                }
            }

            pitShow[nShow] = obj->item_type;

            /*
                Couldn't combine, or didn't want to.
            */
            if ( !fCombine )
            {
                prgpstrShow [nShow] = str_dup( pstrShow );
                prgnShow    [nShow] = obj->count;
                nShow++;
            }
        }
    }

    /*
        Output the formatted list.       -Color support by Thoric
    */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
        switch ( pitShow[iShow] )
        {
            default:
                set_char_color( AT_OBJECT, ch );
                break;

            case ITEM_FOOD:
                set_char_color( AT_HUNGRY, ch );
                break;

            case ITEM_DRINK_CON:
            case ITEM_FOUNTAIN:
                set_char_color( AT_THIRSTY, ch );
                break;

            case ITEM_FIRE:
                set_char_color( AT_FIRE, ch );
                break;
        }

        if ( fShowNothing )
            send_to_char( "     ", ch );

        send_to_char( prgpstrShow[iShow], ch );

        if ( prgnShow[iShow] != 1 )
            ch_printf( ch, " (%d)", prgnShow[iShow] );

        send_to_char( "\n\r", ch );
        DISPOSE( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
        if ( IS_NPC( ch ) || xIS_SET( ch->act, PLR_COMBINE ) )
            send_to_char( "     ", ch );

        send_to_char( "Nothing.\n\r", ch );
    }

    /*
        Clean up.
    */
    DISPOSE( prgpstrShow );
    DISPOSE( prgnShow    );
    DISPOSE( pitShow     );
    return;
}


/*
    Show fancy descriptions for certain spell affects        -Thoric
*/
void show_visible_affects_to_char( CHAR_DATA* victim, CHAR_DATA* ch )
{
    if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
    {
        {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "%s is shrouded in flowing shadow and light.\n\r",
                       IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
        }
    }

    if ( IS_AFFECTED( victim, AFF_FIRESHIELD ) )
    {
        set_char_color( AT_FIRE, ch );
        ch_printf( ch, "%s is engulfed within a blaze of mystical flame.\n\r",
                   IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
    }

    if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD ) )
    {
        set_char_color( AT_BLUE, ch );
        ch_printf( ch, "%s is surrounded by cascading torrents of energy.\n\r",
                   IS_NPC( victim ) ? capitalize( victim->short_descr ) : ( victim->name ) );
    }

    /* Okay, This just got ANNOYING! Removed by Ghost */
    /*
        if ( IS_AFFECTED(victim, AFF_CHARM)       )
        {
        set_char_color( AT_MAGIC, ch );
        ch_printf( ch, "%s looks ahead free of expression.\n\r",
        IS_NPC( victim ) ? capitalize(victim->short_descr) : (victim->name) );
        }
        if ( !IS_NPC(victim) && !victim->desc
        &&    victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS) )
        {
        set_char_color( AT_MAGIC, ch );
        strcpy( buf, PERS( victim, ch ) );
        strcat( buf, " appears to be in a deep trance...\n\r" );
        }
    */
}

void show_char_to_char_0( CHAR_DATA* victim, CHAR_DATA* ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char prefix[MAX_STRING_LENGTH];
    buf[0] = '\0';

    /*
        Renders specified mobs invisible to all players, except Lv. 105+ Imms.
    */
    if ( IS_NPC( victim ) && get_trust( ch ) < 105 )
    {
        if ( victim->pIndexData->vnum == 1 )
            return;  /* Puff */

        if ( victim->pIndexData->vnum == 3 )
            return;  /* Supermob */
    }

    if ( IS_NPC( victim ) )
        strcat( buf, " "  );

    strcpy( prefix, "" );

    if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_HOSTAGE ) )
    {
        strcpy( prefix, "&w&z[&WRescue&z] " );
    }
    else if ( ch->race == victim->race )
    {
        if ( victim->race == RACE_ALIEN )
            strcpy( prefix, "&w&z[&WAlien&z] " );

        if ( victim->race == RACE_MARINE )
            strcpy( prefix, "&w&z[&WMarine&z] " );

        if ( victim->race == RACE_PREDATOR )
            strcpy( prefix, "&w&z[&WPredator&z] " );
    }

    if ( IS_NPC( victim ) )
    {
        if ( victim->pIndexData->vnum == 1 || victim->pIndexData->vnum == 3 )
            strcat( buf, "(IMask) " );
    }

    if ( !IS_NPC( victim ) && !victim->desc )
    {
        if ( !victim->switched )
            strcat( buf, "(Link Dead) "  );
        else if ( !IS_AFFECTED( victim->switched, AFF_POSSESS ) )
            strcat( buf, "(Switched) "   );
    }

    if ( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_AFK ) )
        strcat( buf, "[AFK] " );

    if ( is_spectator( victim ) )
        strcat( buf, "(Spectator) " );

    if ( ( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_WIZINVIS ) )
            || ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_MOBINVIS ) ) )
    {
        if ( !IS_NPC( victim ) )
            sprintf( buf1, "(Invis %d) ", victim->pcdata->wizinvis );
        else
            sprintf( buf1, "(Mobinvis %d) ", victim->mobinvis );

        strcat( buf, buf1 );
    }

    if ( IN_VENT( victim ) && IS_IMMORTAL( ch )   )
        strcat( buf, "(Vent) "       );

    if ( IS_AFFECTED( victim, AFF_INVISIBLE )   )
        strcat( buf, "(Invis) "      );

    if ( IS_AFFECTED( victim, AFF_HIDE )        )
        strcat( buf, "(Hide) "       );

    if ( IS_AFFECTED( victim, AFF_BLIND )       )
        strcat( buf, "(Blinded) "    );

    if ( victim->block != NULL                )
        strcat( buf, "(Blocking) "   );

    if ( get_timerptr( victim, TIMER_DO_FUN ) )
        strcat( buf, "(Working) "   );

    if ( IS_AFFECTED( victim, AFF_CLOAK )       )
    {
        if ( IS_AFFECTED( victim, AFF_SHORTAGE ) )
            strcat( buf, "(Decloaked) "    );

        if ( !IS_AFFECTED( victim, AFF_SHORTAGE ) )
            strcat( buf, "(Cloaked) "    );
    }

    if ( IS_AFFECTED( victim, AFF_PASS_DOOR )   )
        strcat( buf, "(Translucent) " );

    if ( IS_AFFECTED( victim, AFF_FAERIE_FIRE ) )
        strcat( buf, "&P(Pink Aura)&w "  );

    if ( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_LITTERBUG  ) )
        strcat( buf, "(LITTERBUG) "  );

    if ( IS_NPC( victim ) && IS_IMMORTAL( ch ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
        strcat( buf, "(PROTO) " );

    if ( victim->desc && victim->desc->connected == CON_EDITING )
        strcat( buf, "(Writing) " );
    else if ( victim->desc && victim->desc->connected != CON_PLAYING )
        strcat( buf, "(Menu) " );

    if ( IS_NPC( victim ) && victim->position == victim->defposition && victim->long_descr[0] != '\0' )
    {
        send_to_char( prefix, ch );
        strcat( buf, victim->long_descr );
        set_char_color( AT_PERSON, ch );
        send_to_char( buf, ch );
        show_visible_affects_to_char( victim, ch );
        return;
    }

    /*   strcat( buf, PERS( victim, ch ) );       old system of titles
          removed to prevent prepending of name to title     -Kuran

          But added back bellow so that you can see mobs too :P   -Durga
    */

    if ( !IS_NPC( victim ) && !xIS_SET( ch->act, PLR_BRIEF ) )
    {
        if ( knows_player( ch, victim ) )
            strcat( buf, victim->pcdata->title );
        else
            strcat( buf, make_greet_desc( victim, ch ) );
    }
    else
        strcat( buf, PERS( victim, ch ) );

    switch ( victim->position )
    {
        case POS_DEAD:
            strcat( buf, " is DEAD!!" );
            break;

        case POS_MORTAL:
            strcat( buf, " is mortally wounded." );
            break;

        case POS_INCAP:
            strcat( buf, " is incapacitated." );
            break;

        case POS_STUNNED:
            strcat( buf, " is lying here stunned." );
            break;

        case POS_PRONE:
            if ( ch->position == POS_PRONE )
                strcat( buf, " is lying prone nearby." );
            else
                strcat( buf, " is lying prone here." );

            break;

        case POS_KNEELING:
            if ( ch->position == POS_KNEELING )
                strcat ( buf, " is kneeling next to you." );
            else
                strcat ( buf, " is kneeling here." );

            break;

        case POS_SITTING:
            if ( ch->position == POS_SITTING )
                strcat( buf, " sits here with you." );
            else
                strcat( buf, " sits upright here." );

            break;

        case POS_STANDING:
            if ( IS_IMMORTAL( victim ) )
                strcat( buf, " is here before you." );
            else if ( ( victim->in_room->sector_type == SECT_UNDERWATER )
                      && !IS_AFFECTED( victim, AFF_AQUA_BREATH ) && !IS_NPC( victim ) )
                strcat( buf, " is drowning here." );
            else if ( victim->in_room->sector_type == SECT_UNDERWATER )
                strcat( buf, " is here in the water." );
            else if ( ( victim->in_room->sector_type == SECT_OCEANFLOOR )
                      && !IS_AFFECTED( victim, AFF_AQUA_BREATH ) && !IS_NPC( victim ) )
                strcat( buf, " is drowning here." );
            else if ( victim->in_room->sector_type == SECT_OCEANFLOOR )
                strcat( buf, " is standing here in the water." );
            else if ( IS_AFFECTED( victim, AFF_FLOATING )
                      || IS_AFFECTED( victim, AFF_FLYING ) )
                strcat( buf, " is hovering here." );
            else
                strcat( buf, " is standing here." );

            break;

        case POS_SHOVE:
            strcat( buf, " is being shoved around." );
            break;

        case POS_DRAG:
            strcat( buf, " is being dragged around." );
            break;

        case POS_MOUNTED:
            strcat( buf, " is here, upon " );

            if ( !victim->mount )
                strcat( buf, "thin air???" );
            else if ( victim->mount == ch )
                strcat( buf, "your back." );
            else if ( victim->in_room == victim->mount->in_room )
            {
                strcat( buf, PERS( victim->mount, ch ) );
                strcat( buf, "." );
            }
            else
                strcat( buf, "someone who left??" );

            break;
    }

    send_to_char( prefix, ch );
    set_char_color( AT_PERSON, ch );
    strcat( buf, "\n\r" );
    buf[0] = UPPER( buf[0] );
    send_to_char( buf, ch );
    show_visible_affects_to_char( victim, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA* victim, CHAR_DATA* ch )
{
    OBJ_DATA* obj;
    int iWear;
    bool found;

    if ( can_see( victim, ch ) )
    {
        // act( AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT    );
        // act( AT_ACTION, "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }

    if ( victim->description[0] != '\0' )
    {
        send_to_char( victim->description, ch );
    }
    else
    {
        act( AT_PLAIN, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    show_condition( ch, victim );
    found = FALSE;

    if ( ( ( obj = get_eq_char( victim, WEAR_OVER ) ) == NULL ) || obj->value[2] == 0 || !can_see_obj( ch, obj ) )
    {
        for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
        {
            if ( ( obj = get_eq_char( victim, iWear ) ) != NULL && can_see_obj( ch, obj ) )
            {
                if ( !found )
                {
                    send_to_char( "\n\r", ch );
                    act( AT_PLAIN, "$N is wearing:", ch, NULL, victim, TO_CHAR );
                    found = TRUE;
                }

                send_to_char( "&B ", ch );
                send_to_char( where_name[iWear], ch );
                send_to_char( "&G", ch );
                send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
                send_to_char( "&w\n\r", ch );
            }
        }
    }
    else
    {
        send_to_char( "\n\r", ch );
        act( AT_PLAIN, "$N is wearing:", ch, NULL, victim, TO_CHAR );
        found = TRUE;
        send_to_char( "&B ", ch );
        send_to_char( where_name[WEAR_OVER], ch );
        send_to_char( "&G", ch );
        send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
        send_to_char( "&w\n\r", ch );

        for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
        {
            if ( iWear != WEAR_WIELD && iWear != WEAR_DUAL_WIELD && iWear != WEAR_SHIELD )
                continue;

            if ( ( obj = get_eq_char( victim, iWear ) ) != NULL && can_see_obj( ch, obj ) )
            {
                send_to_char( "&B ", ch );
                send_to_char( where_name[iWear], ch );
                send_to_char( "&G", ch );
                send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
                send_to_char( "&w\n\r", ch );
            }
        }
    }

    /*
        Crash fix here by Thoric
    */
    if ( IS_NPC( ch ) || victim == ch )
        return;

    send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
    show_list_to_char( victim->first_carrying, ch, TRUE, TRUE );
    return;
}


void show_char_to_char( CHAR_DATA* list, CHAR_DATA* ch )
{
    CHAR_DATA* rch;

    for ( rch = list; rch; rch = rch->next_in_room )
    {
        if ( rch == ch )
            continue;

        if ( can_see( ch, rch ) )
        {
            show_char_to_char_0( rch, ch );
        }
        else if ( IS_IMMORTAL( rch ) )
        { }
        else if ( room_is_dark( ch, ch->in_room ) && IS_AFFECTED( rch, AFF_INFRARED ) && IN_VENT( ch ) )
        {
            set_char_color( AT_BLOOD, ch );
            send_to_char( "The red form of a living creature is here.\n\r", ch );
        }
    }

    return;
}

bool check_blind( CHAR_DATA* ch )
{
    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_HOLYLIGHT ) )
        return TRUE;

    if ( IS_IMMORTAL( ch ) )
        return TRUE;

    if ( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
        return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    {
        send_to_char( "You can't see a thing!\n\r", ch );
        return FALSE;
    }

    return TRUE;
}

/*
    Returns classical DIKU door direction based on text in arg   -Thoric
*/
int get_door( char* arg )
{
    int door;

    if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "north"     ) )
        door = 0;
    else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east"      ) )
        door = 1;
    else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "south"     ) )
        door = 2;
    else if ( !str_cmp( arg, "w"  ) || !str_cmp( arg, "west"      ) )
        door = 3;
    else if ( !str_cmp( arg, "u"  ) || !str_cmp( arg, "up"    ) )
        door = 4;
    else if ( !str_cmp( arg, "d"  ) || !str_cmp( arg, "down"      ) )
        door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) )
        door = 6;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) )
        door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) )
        door = 8;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) )
        door = 9;
    else
        door = -1;

    return door;
}

void do_look( CHAR_DATA* ch, char* argument )
{
    char arg  [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    EXIT_DATA* pexit;
    CHAR_DATA* victim;
    OBJ_DATA* obj;
    ROOM_INDEX_DATA* original;
    char* pdesc;
    bool doexaprog;
    sh_int door;
    int number, cnt;

    if ( !ch->desc )
        return;

    if ( ch->position <= POS_STUNNED )
    {
        send_to_char( "You can't see anything but stars!\n\r", ch );
        return;
    }

    if ( !check_blind( ch ) )
        return;

    if ( !IS_NPC( ch ) && !xIS_SET( ch->act, PLR_HOLYLIGHT ) && !IS_AFFECTED( ch, AFF_TRUESIGHT ) && ( ch->race != RACE_ALIEN && room_is_dark( ch, ch->in_room ) ) )
    {
        set_char_color( AT_DGREY, ch );
        send_to_char( "It is pitch black ... \n\r", ch );
        show_char_to_char( ch->in_room->first_person, ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    /*  Disabled so it doesn't fuck with Utachas area
        if ( !str_cmp( arg1, "map" ) )
        {
        do_lookmap( ch, (IS_IMMORTAL(ch) ? argument : "5") );
        return;
        }
    */
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    doexaprog = str_cmp( "noprog", arg2 ) && str_cmp( "noprog", arg3 );

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
        /* 'look' or 'look auto' */
        send_to_char( "&W", ch );
        send_to_char( ch->in_room->name, ch );
        send_to_char( " ", ch );

        if ( !ch->desc->original )
        {
            if ( ( get_trust( ch ) >= LEVEL_IMMORTAL ) && ( xIS_SET( ch->pcdata->flags, PCFLAG_ROOM ) ) )
            {
                send_to_char( "&B{", ch );
                ch_printf( ch, "%d", ch->in_room->vnum );
                send_to_char( "}", ch );
                send_to_char( "&c[", ch );
                send_to_char( flag_string( &ch->in_room->room_flags, r_flags, MAX_ROOM_FLAGS ), ch );
                send_to_char( "]", ch );
            }
            else
            {
                if ( xIS_SET( ch->in_room->room_flags, ROOM_DEPLOY_A ) && ch->race == RACE_ALIEN )
                    send_to_char( "&B(Alien Deploy) ", ch );

                if ( xIS_SET( ch->in_room->room_flags, ROOM_DEPLOY_M ) && ch->race == RACE_MARINE )
                    send_to_char( "&B(Marine Deploy) ", ch );

                if ( xIS_SET( ch->in_room->room_flags, ROOM_DEPLOY_P ) && ch->race == RACE_PREDATOR )
                    send_to_char( "&B(Predator Deploy) ", ch );

                if ( xIS_SET( ch->in_room->room_flags, ROOM_DEPLOY_FROM ) )
                    send_to_char( "&B(Deployment Room) ", ch );

                if ( xIS_SET( ch->in_room->room_flags, ROOM_RESCUE ) )
                    send_to_char( "&B(Hostage Rescue Zone) ", ch );

                if ( xIS_SET( ch->in_room->room_flags, ROOM_CP ) )
                    send_to_char( "&B(Control Point) ", ch );

                if ( xIS_SET( ch->in_room->room_flags, ROOM_HIVED ) )
                    send_to_char( "&B(Hived) ", ch );

                if ( xIS_SET( ch->in_room->room_flags, ROOM_VENTED_A ) )
                    send_to_char( "&B(Vent) ", ch );

                if ( xIS_SET( ch->in_room->room_flags, ROOM_VENTED_B ) )
                    send_to_char( "&B(Vent) ", ch );

                if ( xIS_SET( ch->in_room->room_flags, ROOM_VENTED_C ) )
                    send_to_char( "&B(Vent) ", ch );

                if ( xIS_SET( ch->in_room->room_flags, ROOM_VENTED_D ) )
                    send_to_char( "&B(Vent) ", ch );
            }
        }

        send_to_char( "\n\r&w", ch );

        if ( arg1[0] == '\0' || ( !IS_NPC( ch ) && !xIS_SET( ch->act, PLR_BRIEF ) ) )
        {
            if ( IN_VENT( ch ) )
            {
                ROOM_INDEX_DATA* tmp;
                tmp = get_room_index( 50 );

                if ( tmp != NULL )
                    send_to_char( tmp->description, ch );
            }
            else if ( xIS_SET( ch->in_room->room_flags, ROOM_HIVED ) && ch->in_room->hdescription[0] != '\0' )
            {
                send_to_char( ch->in_room->hdescription, ch );
            }
            else
            {
                send_to_char( ch->in_room->description, ch );
            }
        }

        if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_AUTOEXIT ) )
            do_exits( ch, "" );

        show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
        show_char_to_char( ch->in_room->first_person,  ch );
        return;
    }

    if ( !str_cmp( arg1, "under" ) )
    {
        int count;

        /* 'look under' */
        if ( arg2[0] == '\0' )
        {
            send_to_char( "Look beneath what?\n\r", ch );
            return;
        }

        if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You do not see that here.\n\r", ch );
            return;
        }

        if ( ch->carry_weight + obj->weight > can_carry_w( ch ) )
        {
            send_to_char( "It's too heavy for you to look under.\n\r", ch );
            return;
        }

        count = obj->count;
        obj->count = 1;
        act( AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR );
        act( AT_PLAIN, "$n lifts $p and looks beneath it:", ch, obj, NULL, TO_ROOM );
        obj->count = count;

        if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
            show_list_to_char( obj->first_content, ch, TRUE, TRUE );
        else
            send_to_char( "Nothing.\n\r", ch );

        if ( doexaprog )
            oprog_examine_trigger( ch, obj );

        return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
        int count;

        /* 'look in' */
        if ( arg2[0] == '\0' )
        {
            send_to_char( "Look in what?\n\r", ch );
            return;
        }

        if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You do not see that here.\n\r", ch );
            return;
        }

        switch ( obj->item_type )
        {
            default:
                send_to_char( "That is not a container.\n\r", ch );
                break;

            case ITEM_CONTAINER:
            case ITEM_CORPSE_NPC:
            case ITEM_CORPSE_PC:
                if ( IS_SET( obj->value[1], CONT_CLOSED ) )
                {
                    send_to_char( "It is closed.\n\r", ch );
                    break;
                }

                count = obj->count;
                obj->count = 1;
                act( AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR );
                obj->count = count;
                show_list_to_char( obj->first_content, ch, TRUE, TRUE );

                if ( doexaprog )
                    oprog_examine_trigger( ch, obj );

                break;
        }

        return;
    }

    if ( !str_cmp( arg1, "out" ) )
    {
        if ( ch->vent && ch->in_room )
        {
            if ( xIS_SET( ch->in_room->room_flags, ROOM_VENTED_A ) ||
                    xIS_SET( ch->in_room->room_flags, ROOM_VENTED_B ) ||
                    xIS_SET( ch->in_room->room_flags, ROOM_VENTED_C ) ||
                    xIS_SET( ch->in_room->room_flags, ROOM_VENTED_D ) )
            {
                ch->vent = FALSE;
                do_look( ch, "" );

                if ( !is_spectator( ch ) )
                    run_awareness( ch->in_room->first_person, ch );

                ch->vent = TRUE;
            }
            else
            {
                ch_printf( ch, "&RYou can't see out of the vent from here.\n\r" );
            }
        }
        else
        {
            do_nothing( ch, "" );
        }

        return;
    }

    if ( ( pdesc = get_extra_descr( arg1, ch->in_room->first_extradesc ) ) != NULL )
    {
        send_to_char( pdesc, ch );
        return;
    }

    door = get_door( arg1 );

    if ( ( pexit = find_door( ch, arg1, TRUE ) ) != NULL )
    {
        if ( pexit->keyword )
        {
            if ( xIS_SET( pexit->exit_info, EX_CLOSED )
                    &&  !xIS_SET( pexit->exit_info, EX_WINDOW ) )
            {
                if ( xIS_SET( pexit->exit_info, EX_SECRET )
                        &&   door != -1 )
                    send_to_char( "Nothing special there.\n\r", ch );
                else
                    act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );

                return;
            }

            if ( xIS_SET( pexit->exit_info, EX_BASHED ) )
                act( AT_RED, "The $d has been bashed from its hinges!", ch, NULL, pexit->keyword, TO_CHAR );
        }

        if ( pexit->description && pexit->description[0] != '\0' )
            send_to_char( pexit->description, ch );
        else
            send_to_char( "Nothing special there.\n\r", ch );

        /*
            Ability to look into the next room         -Thoric
        */
        if ( pexit->to_room
                && ( IS_AFFECTED( ch, AFF_SCRYING )
                     ||   xIS_SET( pexit->exit_info, EX_xLOOK )
                     ||   get_trust( ch ) >= LEVEL_IMMORTAL ) )
        {
            if ( !xIS_SET( pexit->exit_info, EX_xLOOK )
                    &&    get_trust( ch ) < LEVEL_IMMORTAL )
            {
                set_char_color( AT_MAGIC, ch );
                send_to_char( "You attempt to scry...\n\r", ch );

                /*  Change by Narn, Sept 96 to allow characters who don't have the
                    scry spell to benefit from objects that are affected by scry.
                */
                if ( !IS_NPC( ch ) )
                {
                    int percent = ch->pcdata->learned[ skill_lookup( "scry" ) ];

                    if ( !percent )
                        percent = 99;

                    if (  number_percent( ) > percent )
                    {
                        send_to_char( "You fail.\n\r", ch );
                        return;
                    }
                }
            }

            original = ch->in_room;

            if ( pexit->distance > 1 )
            {
                ROOM_INDEX_DATA* to_room;

                if ( ( to_room = generate_exit( ch->in_room, &pexit ) ) != NULL )
                {
                    char_from_room( ch );
                    char_to_room( ch, to_room );
                }
                else
                {
                    char_from_room( ch );
                    char_to_room( ch, pexit->to_room );
                }
            }
            else
            {
                char_from_room( ch );
                char_to_room( ch, pexit->to_room );
            }

            do_look( ch, "auto" );
            char_from_room( ch );
            char_to_room( ch, original );
        }

        return;
    }
    else if ( door != -1 )
    {
        send_to_char( "Nothing special there.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
        show_char_to_char_1( victim, ch );
        return;
    }

    /* finally fixed the annoying look 2.obj desc bug   -Thoric */
    number = number_argument( arg1, arg );

    for ( cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
        if ( can_see_obj( ch, obj ) )
        {
            if ( ( pdesc = get_extra_descr( arg, obj->first_extradesc ) ) != NULL )
            {
                if ( ( cnt += obj->count ) < number )
                    continue;

                send_to_char( "&w&W", ch );
                send_to_char( pdesc, ch );

                if ( doexaprog )
                    oprog_examine_trigger( ch, obj );

                return;
            }

            if ( ( pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc ) ) != NULL )
            {
                if ( ( cnt += obj->count ) < number )
                    continue;

                send_to_char( pdesc, ch );

                if ( doexaprog )
                    oprog_examine_trigger( ch, obj );

                return;
            }

            if ( nifty_is_name_prefix( arg, obj->name ) )
            {
                if ( ( cnt += obj->count ) < number )
                    continue;

                pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );

                if ( !pdesc )
                    pdesc = get_extra_descr( obj->name, obj->first_extradesc );

                if ( !pdesc )
                    send_to_char( "You can't recall anything about this item.\r\n", ch );
                else
                    send_to_char( pdesc, ch );

                if ( doexaprog )
                    oprog_examine_trigger( ch, obj );

                return;
            }
        }
    }

    for ( obj = ch->in_room->last_content; obj; obj = obj->prev_content )
    {
        if ( can_see_obj( ch, obj ) )
        {
            if ( ( pdesc = get_extra_descr( arg, obj->first_extradesc ) ) != NULL )
            {
                if ( ( cnt += obj->count ) < number )
                    continue;

                send_to_char( "&w&W", ch );
                send_to_char( pdesc, ch );

                if ( doexaprog )
                    oprog_examine_trigger( ch, obj );

                return;
            }

            if ( ( pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc ) ) != NULL )
            {
                if ( ( cnt += obj->count ) < number )
                    continue;

                send_to_char( "&w&W", ch );
                send_to_char( pdesc, ch );

                if ( doexaprog )
                    oprog_examine_trigger( ch, obj );

                return;
            }

            if ( nifty_is_name_prefix( arg, obj->name ) )
            {
                if ( ( cnt += obj->count ) < number )
                    continue;

                pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );

                if ( !pdesc )
                    pdesc = get_extra_descr( obj->name, obj->first_extradesc );

                if ( !pdesc )
                    send_to_char( "You can't recall anything about this item.\r\n", ch );
                else
                    send_to_char( pdesc, ch );

                if ( doexaprog )
                    oprog_examine_trigger( ch, obj );

                return;
            }
        }
    }

    send_to_char( "You do not see that here.\n\r", ch );
    return;
}

void show_obj_desc( CHAR_DATA* ch, OBJ_DATA* obj )
{
    char buf[MAX_INPUT_LENGTH];
    char* pdesc;

    if ( ch == NULL )
        return;

    if ( obj == NULL )
        return;

    sprintf( buf, "%s", obj->name );

    if ( ( pdesc = get_extra_descr( buf, obj->first_extradesc ) ) != NULL )
    {
        send_to_char( "&w&W", ch );
        send_to_char( pdesc, ch );
        return;
    }

    if ( ( pdesc = get_extra_descr( buf, obj->pIndexData->first_extradesc ) ) != NULL )
    {
        send_to_char( pdesc, ch );
        return;
    }

    if ( nifty_is_name_prefix( buf, obj->name ) )
    {
        pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );

        if ( !pdesc )
            pdesc = get_extra_descr( obj->name, obj->first_extradesc );

        if ( !pdesc )
            send_to_char( "You can't recall anything about this item.\r\n", ch );
        else
            send_to_char( pdesc, ch );

        return;
    }

    return;
}

void show_condition( CHAR_DATA* ch, CHAR_DATA* victim )
{
    char buf[MAX_STRING_LENGTH];
    int percent;

    if ( victim->max_hit > 0 )
        percent = ( 100 * victim->hit ) / victim->max_hit;
    else
        percent = -1;

    strcpy( buf, PERS( victim, ch ) );

    if ( IS_NPC ( victim ) && ( xIS_SET( victim->act, ACT_DROID ) ||
                                xIS_SET( victim->act, ACT_PET ) ) )
    {
        if ( percent >= 100 )
            strcat( buf, " is in perfect condition.\n\r"  );
        else if ( percent >=  90 )
            strcat( buf, " is slightly scratched.\n\r" );
        else if ( percent >=  80 )
            strcat( buf, " has a few scrapes.\n\r"     );
        else if ( percent >=  70 )
            strcat( buf, " has some dents.\n\r"         );
        else if ( percent >=  60 )
            strcat( buf, " has a couple holes in its plating.\n\r"    );
        else if ( percent >=  50 )
            strcat( buf, " has many broken pieces.\n\r" );
        else if ( percent >=  40 )
            strcat( buf, " has many exposed circuits.\n\r"    );
        else if ( percent >=  30 )
            strcat( buf, " is leaking oil.\n\r"   );
        else if ( percent >=  20 )
            strcat( buf, " has smoke coming out of it.\n\r"       );
        else if ( percent >=  10 )
            strcat( buf, " is almost completely broken.\n\r"        );
        else
            strcat( buf, " is about to EXPLODE.\n\r"              );
    }
    else
    {
        if ( percent >= 100 )
            strcat( buf, " is in perfect health.\n\r"  );
        else if ( percent >=  90 )
            strcat( buf, " is slightly scratched.\n\r" );
        else if ( percent >=  80 )
            strcat( buf, " has a few bruises.\n\r"     );
        else if ( percent >=  70 )
            strcat( buf, " has some cuts.\n\r"         );
        else if ( percent >=  60 )
            strcat( buf, " has several wounds.\n\r"    );
        else if ( percent >=  50 )
            strcat( buf, " has many nasty wounds.\n\r" );
        else if ( percent >=  40 )
            strcat( buf, " is bleeding freely.\n\r"    );
        else if ( percent >=  30 )
            strcat( buf, " is covered in blood.\n\r"   );
        else if ( percent >=  20 )
            strcat( buf, " is leaking guts.\n\r"       );
        else if ( percent >=  10 )
            strcat( buf, " is almost dead.\n\r"        );
        else
            strcat( buf, " is DYING.\n\r"              );
    }

    buf[0] = UPPER( buf[0] );
    send_to_char( buf, ch );
    return;
}

/*  A much simpler version of look, this function will show you only
    the condition of a mob or pc, or if used without an argument, the
    same you would see if you enter the room and have config +brief.
    -- Narn, winter '96
*/
void do_glance( CHAR_DATA* ch, char* argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    EXT_BV save_act;

    if ( !ch->desc )
        return;

    if ( ch->position <= POS_STUNNED )
    {
        send_to_char( "You can't see anything but stars!\n\r", ch );
        return;
    }

    if ( !check_blind( ch ) )
        return;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        save_act = ch->act;
        xSET_BIT( ch->act, PLR_BRIEF );
        do_look( ch, "auto" );
        ch->act = save_act;
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They're not here.\n\r", ch );
        return;
    }
    else
    {
        if ( can_see( victim, ch ) )
        {
            act( AT_ACTION, "$n glances at you.", ch, NULL, victim, TO_VICT    );
            act( AT_ACTION, "$n glances at $N.",  ch, NULL, victim, TO_NOTVICT );
        }

        show_condition( ch, victim );
        ch_printf( ch, "&zExperience worth: &W%d\n\r", get_exp_worth( victim ) );
        return;
    }

    return;
}

void do_examine( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* keeper = NULL;
    OBJ_DATA* obj = NULL;
    int onum, oref;

    if ( !argument )
    {
        bug( "do_examine: null argument.", 0 );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Examine what?\n\r", ch );
        return;
    }

    if ( arg[0] == '#' )
    {
        /* Store Item */
        if ( ( keeper = find_keeper( ch ) ) == NULL )
        {
            send_to_char( "You can't do that here.\n\r", ch );
            return;
        }

        onum = 0;
        oref = atoi( arg + 1 );

        for ( obj = keeper->last_carrying; obj; obj = obj->prev_content )
        {
            if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
                onum++;

            if ( onum == oref )
            {
                ch_printf( ch, "&w&BViewing Item #%d in this shop: \n\r&z", oref );
                examine_obj( ch, obj );
                return;
            }
            else if ( onum > oref )
                break;
        }

        act( AT_SAY, "$n says 'Sorry, I don't sell that'", keeper, NULL, ch, TO_ROOM );
        return;
    }
    else
    {
        if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
            examine_obj( ch, obj );
    }

    return;
}

void examine_obj( CHAR_DATA* ch, OBJ_DATA* obj )
{
    char buf[MAX_STRING_LENGTH];
    SENTRY_DATA* tmp = NULL;
    BOARD_DATA* board = NULL;
    sh_int dam;

    if ( !ch )
    {
        bug( "examine_obj: null ch.", 0 );
        return;
    }

    if ( !obj )
    {
        bug( "examine_obj: null obj.", 0 );
        return;
    }

    /* Emulated LOOK (Object) */
    show_obj_desc( ch, obj );

    if ( ( board = get_board( obj ) ) != NULL )
    {
        if ( board->num_posts )
            ch_printf( ch, "There are about %d notes posted here.  Type 'note list' to list them.\n\r", board->num_posts );
        else
            send_to_char( "There aren't any notes posted here.\n\r", ch );
    }

    switch ( obj->item_type )
    {
        default:
            break;

        case ITEM_ARMOR:
            ch_printf( ch, "\n\r&zArmor health: &B%d&b/%d.\n\r", obj->value[0], obj->value[1] );

            if ( obj->value[3] == 1 )
                ch_printf( ch, "&zArmor class: &Clight.\n\r" );

            if ( obj->value[3] == 2 )
                ch_printf( ch, "&zArmor class: &Cmedium.\n\r" );

            if ( obj->value[3] == 3 )
                ch_printf( ch, "&zArmor class: &Cheavy.\n\r" );

            break;

        case ITEM_REGENERATOR:
            ch_printf( ch, "\n\r&zHitpoints: &R%d&r/&R%d.\n\r", obj->value[0], obj->value[1] );
            break;

        case ITEM_LIGHT:
            ch_printf( ch, "\n\r&zThis item is a light. &wSee HELP LIGHT.\n\r\n\r" );
            ch_printf( ch, "&w&zCurrent Charge: &R%d/%d.\n\r", obj->value[2], obj->value[4] );
            break;

        case ITEM_GRENADE:
            ch_printf( ch, "\n\r&RThis item is a grenade weapon. &wSee HELP GRENADE.\n\r\n\r" );
            strcpy( buf, "Unknown" );

            if ( obj->value[3] >= 0 && obj->value[3] <= 4 )
                sprintf( buf, "%s", ris_flags[obj->value[3]] );

            ch_printf( ch, "&w&zDamage Type: &C%s.\n\r", buf );
            ch_printf( ch, "&w&zDamage Amount: &R%d - %d.\n\r", obj->value[0], obj->value[1] );
            ch_printf( ch, "&w&zMax Blast Radius: &C%d room%s.\n\r", obj->value[2], ( obj->value[2] == 1 ) ? "" : "s" );
            ch_printf( ch, "&w&zTimer Range: &C%d to %d rounds.\n\r", obj->value[5], obj->value[5] * 5 );
            break;

        case ITEM_COVER:
            ch_printf( ch, "\n\r&zThis item can afford protection from weapons fire.\n\r" );
            ch_printf( ch, "&zCoverage Remaining: &C%d units.\n\r", obj->value[0] );
            ch_printf( ch, "&zArmor Rating: &C%d AC &z(&W-%d damage&z)\n\r", obj->value[2], obj->value[2] * 10 );
            ch_printf( ch, "&zCondition: [%s&z] (&W%d/%d&z)\n\r", drawbar( 10, obj->value[3], obj->value[4], "&G", "&g" ), obj->value[3], obj->value[4] );

            if ( obj->value[5] == 1 )
                ch_printf( ch, "\n\r&zThis item will explode for &w&R%d fire &zdamage when killed.\n\r", obj->weight * 10 );

            if ( obj->value[5] == 2 )
                ch_printf( ch, "\n\r&zThis item will shatter for &w&C%d pierce &zdamage when killed.\n\r", obj->weight );

            break;

        case ITEM_WEAPON:
            send_to_char( "\n\r&z", ch );

            switch ( obj->value[0] )
            {
                default:
                    return;

                case WEAPON_KNIFE:
                    strcpy( buf, "Knife Weapon (Marine-Class)\n\r" );
                    break;

                case WEAPON_PISTOL:
                    strcpy( buf, "Pistol Weapon (Marine-Class)\n\r" );
                    break;

                case WEAPON_RIFLE:
                    strcpy( buf, "Rifle Weapon (Marine-Class)\n\r" );
                    break;

                case WEAPON_SHOTGUN:
                    strcpy( buf, "Shotgun Weapon (Marine-Class)\n\r" );
                    break;

                case WEAPON_AUTOMATIC:
                    strcpy( buf, "Automatic Weapon (Marine-Class)\n\r" );
                    break;

                case WEAPON_FLAMETHROWER:
                    strcpy( buf, "Flamethrower Weapon (Marine-Class)\n\r" );
                    break;

                case WEAPON_ROCKETFIRED:
                    strcpy( buf, "Projectile-based Weapon (Marine-Class)\n\r" );
                    break;

                case WEAPON_BLADE:
                    strcpy( buf, "Blade Weapon (Predator-Class)\n\r" );
                    break;

                case WEAPON_SPEAR:
                    strcpy( buf, "Spear Weapon (Predator-Class)\n\r" );
                    break;

                case WEAPON_ERANGED:
                    strcpy( buf, "Ranged Energy Weapon (Predator-Class)\n\r" );
                    break;

                case WEAPON_RANGED:
                    strcpy( buf, "Ranged Weapon (Predator-Class)\n\r" );
                    break;

                case WEAPON_DISC:
                    strcpy( buf, "Disc Weapon (Predator-Class)\n\r" );
                    break;
            }

            send_to_char( buf, ch );

            if ( xIS_SET( obj->extra_flags, ITEM_BURSTFIRE ) )
                ch_printf( ch, "&zFire rate of &C%d &zrounds per minute via Burstfire.\n\r", obj->value[3] );

            if ( xIS_SET( obj->extra_flags, ITEM_AUTOFIRE ) )
                ch_printf( ch, "&zFire rate of &C%d &zrounds per minute via Automatic.\n\r", obj->value[4] );

            if ( is_ranged( obj->value[0] ) )
            {
                ch_printf( ch, "&zThis weapon can fire up to &C%d &zmeter(s).\n\r", obj->value[2] * 10 );

                if ( obj->weapon_mode == MODE_SINGLE )
                    ch_printf( ch, "This weapon is currently set to &WSINGLE FIRE.\n\r" );

                if ( obj->weapon_mode == MODE_BURST )
                    ch_printf( ch, "This weapon is currently set to &WBURST FIRE.\n\r" );

                if ( obj->weapon_mode == MODE_AUTOMATIC )
                    ch_printf( ch, "This weapon is currently set to &WAUTOMATIC FIRE.\n\r" );

                if ( obj->weapon_mode == MODE_SEMIAUTO )
                    ch_printf( ch, "This weapon is currently set to &WSEMI-AUTOMATIC FIRE.\n\r" );
            }
            else
            {
                strcpy( buf, "" );

                if ( obj->value[4] >= 0 && obj->value[4] <= 4 )
                    sprintf( buf, "%s ", ris_flags[obj->value[4]] );

                ch_printf( ch, "&zThis weapon deals &C%d &zto &C%d &z%sdamage.\n\r", obj->value[1], obj->value[2], buf );
                ch_printf( ch, "&zThis weapon has a reach of &C%d&z. (&WAccuracy Bonus: %d%%&z)\n\r", obj->value[3], obj->value[3] * 5 );
            }

            if ( obj->ammo )
            {
                ch_printf( ch, "&zLoaded with a %s. (&W%d/%d rounds&z)\n\r", obj->ammo->short_descr, obj->ammo->value[2], get_max_rounds( obj->ammo ) );
                strcpy( buf, "Unknown Type" );

                if ( obj->ammo->value[0] == 5 )
                {
                    OBJ_INDEX_DATA* pOI;

                    if ( ( pOI = get_obj_index( obj->ammo->value[1] ) ) != NULL )
                        ch_printf( ch, "&z  [&WAmmo Damage: &C%d - %d&z]\n\r", pOI->value[0], pOI->value[1] );
                }
                else
                {
                    if ( obj->ammo->value[0] >= 0 && obj->ammo->value[0] <= 4 )
                        sprintf( buf, "%s", ris_flags[obj->ammo->value[0]] );

                    ch_printf( ch, "&z [&WAmmo Damage: &C%d &W/ %s&z]\n\r", obj->ammo->value[1], buf );
                }
            }
            else if ( obj->value[0] == WEAPON_ERANGED )
            {
                ch_printf( ch, "&zThis weapon deals &C%d &zenergy damage.\n\r", obj->value[4] );
                ch_printf( ch, "&zEach shot drains &C%d &zfield charge.\n\r", obj->value[1] );
            }

            if ( obj->attach )
            {
                ch_printf( ch, "&zUpgraded with a %s.\n\r", obj->attach->short_descr );
                send_attach_stats( ch, obj->attach );
            }

            break;

        case ITEM_SENTRY:
            tmp = get_sentry( obj );
            send_to_char( "\n\r&z", ch );
            ch_printf( ch, "&zCondition: &C%d&z of &C%d &zhitpoints.\n\r", obj->value[3], obj->value[4] );

            if ( tmp == NULL )
            {
                ch_printf( ch, "&zThis weapon is currently: &CDisarmed.\n\r" );
            }
            else
            {
                ch_printf( ch, "&zThis weapon is currently: &RARMED.\n\r" );
                ch_printf( ch, "&zTempature Gauge: [%s&z]\n\r", drawbar( 10, tmp->temp, UMAX( 2, obj->value[5] ), "&R", "&z" ) );
            }

            ch_printf( ch, "&zThis weapon fires ammo bursts up to &C%d&z meters.\n\r", obj->value[2] * 10 );
            ch_printf( ch, "&zThis weapon fires &C%d&z rounds per attack.\n\r", obj->value[1] );

            if ( obj->ammo )
            {
                ch_printf( ch, "&zLoaded with a %s. (&W%d/%d rounds&z)\n\r", obj->ammo->short_descr, obj->ammo->value[2], get_max_rounds( obj->ammo ) );
                strcpy( buf, "Unknown Type" );

                if ( obj->ammo->value[0] == 5 )
                {
                    OBJ_INDEX_DATA* pOI;

                    if ( ( pOI = get_obj_index( obj->ammo->value[1] ) ) != NULL )
                        ch_printf( ch, "&z  [&WAmmo Damage: &C%d - %d&z]\n\r", pOI->value[0], pOI->value[1] );
                }
                else
                {
                    if ( obj->ammo->value[0] >= 0 && obj->ammo->value[0] <= 4 )
                        sprintf( buf, "%s", ris_flags[obj->ammo->value[0]] );

                    ch_printf( ch, "&z [&WAmmo Damage: &C%d &W/ %s&z]\n\r", obj->ammo->value[1], buf );
                }
            }

            break;

        case ITEM_AMMO:
            strcpy( buf, "Unknown Type" );

            if ( obj->value[0] >= 0 && obj->value[0] <= 4 )
                sprintf( buf, "%s", ris_flags[obj->value[0]] );

            ch_printf( ch, "\n\r&zDeals &C%d %s &zdamage.\n\r", obj->value[1], buf );
            ch_printf( ch, "&zContains &C%d of %d &zrounds.\n\r", obj->value[2], get_max_rounds( obj ) );
            break;

        case ITEM_FOOD:
            ch_printf( ch, "\n\r&zHealth + &C%d&z, Movement + &C%d&z.\n\r", obj->value[0], obj->value[1] );
            break;

        case ITEM_CORPSE_PC:
        case ITEM_CORPSE_NPC:
        {
            sh_int timerfrac = obj->timer;

            if ( obj->item_type == ITEM_CORPSE_PC )
                timerfrac = ( int )obj->timer / 8 + 1;

            switch ( timerfrac )
            {
                default:
                    send_to_char( "This corpse has recently been slain.\n\r", ch );
                    break;

                case 4:
                    send_to_char( "This corpse was slain a little while ago.\n\r", ch );
                    break;

                case 3:
                    send_to_char( "A foul smell rises from the corpse, and it is covered in flies.\n\r", ch );
                    break;

                case 2:
                    send_to_char( "A writhing mass of maggots and decay, you can barely go near this corpse.\n\r", ch );
                    break;

                case 1:
                case 0:
                    send_to_char( "Little more than bones, there isn't much left of this corpse.\n\r", ch );
                    break;
            }

            break;
        }
    }

    /*
        if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
        {
        sprintf( buf, "under %s noprog", arg );
        do_look( ch, buf );
        }
    */
    oprog_examine_trigger( ch, obj );

    if ( char_died( ch ) || obj_extracted( obj ) )
        return;

    return;
}


void do_exits( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA* pexit;
    char sign = '-';
    bool found;
    bool fAuto;
    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
        return;

    if ( !IS_NPC( ch ) && !xIS_SET( ch->act, PLR_HOLYLIGHT ) && !IS_AFFECTED( ch, AFF_TRUESIGHT ) && ( ch->race != RACE_ALIEN && room_is_dark( ch, ch->in_room ) ) )
    {
        send_to_char( "&zIt's too dark to distinguish the exits!\n\r", ch );
        return;
    }

    strcpy( buf, fAuto ? "&z&WExits:" : "&z&WObvious exits:\n\r" );
    found = FALSE;

    for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
    {
        if ( pexit->to_room && !xIS_SET( pexit->exit_info, EX_HIDDEN ) && ( !xIS_SET( pexit->exit_info, EX_BLASTOPEN ) || xIS_SET( pexit->exit_info, EX_BLASTED ) ) && ( !xIS_SET( pexit->exit_info, EX_NOVENT ) || !ch->vent ) )
        {
            found = TRUE;

            if ( !fAuto )
            {
                sign = '-';

                if ( xIS_SET( pexit->exit_info, EX_ISDOOR ) )
                    sign = '+';

                if ( ch->vent )
                {
                    sprintf( buf + strlen( buf ), "&z&W%-5s %c %s\n\r", capitalize( dir_name[pexit->vdir] ), sign, room_is_dark( ch, pexit->to_room ) ?  "Too dark to tell" : pexit->to_room->name );
                }
                else if ( xIS_SET( pexit->exit_info, EX_LOCKED ) )
                {
                    if ( xIS_SET( pexit->exit_info, EX_ARMORED ) )
                    {
                        sprintf( buf + strlen( buf ), "&z&W%-5s %c [locked doorway]\n\r", capitalize( dir_name[pexit->vdir] ), sign );
                    }
                    else
                    {
                        sprintf( buf + strlen( buf ), "&z&W%-5s %c (locked doorway)\n\r", capitalize( dir_name[pexit->vdir] ), sign );
                    }
                }
                else if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
                {
                    if ( xIS_SET( pexit->exit_info, EX_ARMORED ) )
                    {
                        sprintf( buf + strlen( buf ), "&z&W%-5s %c [closed doorway]\n\r", capitalize( dir_name[pexit->vdir] ), sign );
                    }
                    else
                    {
                        sprintf( buf + strlen( buf ), "&z&W%-5s %c (closed doorway)\n\r", capitalize( dir_name[pexit->vdir] ), sign );
                    }
                }
                else if ( xIS_SET( pexit->exit_info, EX_WINDOW ) )
                {
                    if ( xIS_SET( pexit->exit_info, EX_ISDOOR ) )
                    {
                        sprintf( buf + strlen( buf ), "&z&W%-5s %c %s\n\r", capitalize( dir_name[pexit->vdir] ), sign, room_is_dark( ch, pexit->to_room ) ?  "Too dark to tell" : pexit->to_room->name );
                    }
                    else if ( xIS_SET( pexit->exit_info, EX_BARRED ) )
                    {
                        sprintf( buf + strlen( buf ), "&z&W%-5s %c (barred window)\n\r", capitalize( dir_name[pexit->vdir] ), sign );
                    }
                    else
                    {
                        sprintf( buf + strlen( buf ), "&z&W%-5s %c (open window)\n\r", capitalize( dir_name[pexit->vdir] ), sign );
                    }
                }
                else if ( xIS_SET( pexit->exit_info, EX_xAUTO ) )
                {
                    sprintf( buf + strlen( buf ), "&z&W%-5s %c %s\n\r", capitalize( pexit->keyword ), sign, room_is_dark( ch, pexit->to_room ) ? "Too dark to tell" : stripclr( pexit->to_room->name ) );
                }
                else
                {
                    sprintf( buf + strlen( buf ), "&z&W%-5s %c %s\n\r", capitalize( dir_name[pexit->vdir] ), sign, room_is_dark( ch, pexit->to_room ) ?  "Too dark to tell" : pexit->to_room->name );
                }
            }
            else
            {
                sprintf( buf + strlen( buf ), " &z&W%s", capitalize( dir_name[pexit->vdir] ) );
            }
        }
    }

    if ( !found )
        strcat( buf, fAuto ? " &z&Wnone.\n\r" : "&z&WNone.\n\r" );
    else if ( fAuto )
        strcat( buf, ".\n\r" );

    send_to_char( buf, ch );
    return;
}

char*   const   day_name    [] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char*   const   month_name  [] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

/*
    void do_time( CHAR_DATA *ch, char *argument )
    {
    extern char str_boot_time[];
    extern char reboot_time[];
    char *suf;
    int day;

    day     = time_info.day + 1;

     if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    set_char_color( AT_YELLOW, ch );
    ch_printf( ch,
    "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r"
        "The mud started up at:    %s\r"
        "The system time (E.S.T.): %s\r"
        "Next Reboot is set for:   %s\r",

    (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
    time_info.hour >= 12 ? "pm" : "am",
    day_name[day % 7],
    day, suf,
    month_name[time_info.month],
    str_boot_time,
    (char *) ctime( &current_time ),
    reboot_time
    );

    return;
    }
*/

void do_time( CHAR_DATA* ch, char* argument )
{
    extern char str_boot_time[];
    extern char reboot_time[];

    if ( !str_cmp( argument, "system" ) )
    {
        ch_printf( ch, "&zThe mud started up at:     &C%s\r&zThe system time (P.D.T.):  &C%s\r&zNext Reboot is set for:    &C%s\r",
                   str_boot_time, ( char* ) ctime( &current_time ), reboot_time );
    }
    else
    {
        ch_printf( ch, "&zCurrent time: &C%d o' clock %s.\n\r",
                   ( time_info.hour % 12 == 0 ) ? 12 : time_info.hour % 12, time_info.hour >= 12 ? "pm" : "am" );
        ch_printf( ch, "&z[&WYou may use TIME SYSTEM for RL Times&z]\n\r" );
        // act( AT_PLAIN, "$n checks $s chronometer.", ch, NULL, NULL, TO_ROOM );
    }

    return;
}


void do_weather( CHAR_DATA* ch, char* argument )
{
    static char* const sky_look[4] =
    {
        "cloudless",
        "cloudy",
        "rainy",
        "lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE( ch ) )
    {
        send_to_char( "You can't see the sky from here.\n\r", ch );
        return;
    }

    set_char_color( AT_BLUE, ch );
    ch_printf( ch, "The sky is %s and %s.\n\r",
               sky_look[weather_info.sky],
               weather_info.change >= 0
               ? "a warm southerly breeze blows"
               : "a cold northern gust blows"
             );
    return;
}

/*
    Produce a description of the weather based on area weather using
    the following sentence format:
        <combo-phrase> and <single-phrase>.
    Where the combo-phrase describes either the precipitation and
    temperature or the wind and temperature. The single-phrase
    describes either the wind or precipitation depending upon the
    combo-phrase.
    Last Modified: July 31, 1997
    Fireblade - Under Construction
*/
/*  void do_weather(CHAR_DATA *ch, char *argument)
    {
    char *combo, *single;
    char buf[MAX_INPUT_LENGTH];
    int temp, precip, wind;

    if ( !IS_OUTSIDE(ch) )
    {
        ch_printf(ch, "You can't see the sky from here.\n\r");
        return;
    }

    temp = (ch->in_room->area->weather->temp + 3*weath_unit - 1)/
        weath_unit;
    precip = (ch->in_room->area->weather->precip + 3*weath_unit - 1)/
        weath_unit;
    wind = (ch->in_room->area->weather->wind + 3*weath_unit - 1)/
        weath_unit;

    if ( precip >= 3 )
    {
        combo = preciptemp_msg[precip][temp];
        single = wind_msg[wind];
    }
    else
    {
        combo = windtemp_msg[wind][temp];
        single = precip_msg[precip];
    }

    sprintf(buf, "%s and %s.\n\r", combo, single);

    set_char_color(AT_BLUE, ch);

    ch_printf(ch, buf);
    }*/


/*
    Moved into a separate function so it can be used for other things
    ie: online help editing              -Thoric
*/
HELP_DATA* get_help( CHAR_DATA* ch, char* argument )
{
    char argall[MAX_INPUT_LENGTH];
    char argone[MAX_INPUT_LENGTH];
    char argnew[MAX_INPUT_LENGTH];
    HELP_DATA* pHelp;
    int lev;

    if ( argument[0] == '\0' )
        argument = "summary";

    if ( isdigit( argument[0] ) )
    {
        lev = number_argument( argument, argnew );
        argument = argnew;
    }
    else
        lev = -2;

    /*
        Tricky argument handling so 'help a b' doesn't match a.
    */
    argall[0] = '\0';

    while ( argument[0] != '\0' )
    {
        argument = one_argument( argument, argone );

        if ( argall[0] != '\0' )
            strcat( argall, " " );

        strcat( argall, argone );
    }

    for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
    {
        if ( ch != NULL )
            if ( pHelp->level > get_trust( ch ) )
                continue;

        if ( lev != -2 && pHelp->level != lev )
            continue;

        if ( is_name( argall, pHelp->keyword ) )
            return pHelp;
    }

    return NULL;
}

/*
    LAWS command
*/
void do_laws( CHAR_DATA* ch, char* argument )
{
    char buf[1024];

    if ( argument == NULL )
        do_help( ch, "laws" );
    else
    {
        sprintf( buf, "law %s", argument );
        do_help( ch, buf );
    }
}

/*
    Now this is cleaner.
    Added SOUNDEX Support --- Ghost (5/22/02)
*/
void do_help( CHAR_DATA* ch, char* argument )
{
    HELP_DATA* pHelp = NULL;

    if ( argument[0] == '\0' )
    {
        if ( ch->race == RACE_ALIEN )
            pHelp = get_help( ch, "Alien Index" );

        if ( ch->race == RACE_MARINE )
            pHelp = get_help( ch, "Marine Index" );

        if ( ch->race == RACE_PREDATOR )
            pHelp = get_help( ch, "Predator Index" );
    }
    else
    {
        pHelp = get_help( ch, argument );
    }

    if ( pHelp == NULL )
    {
        pager_printf( ch, "&w&RNo help on \'%s\' found.", argument );
        similar_help_files( ch, argument );
        /* Log Lookup Attempt */
        {
            char buf[MAX_STRING_LENGTH];
            char filename[MAX_STRING_LENGTH];
            char* strtime;
            strtime                    = ctime( &current_time );
            strtime[strlen( strtime ) - 1] = '\0';
            sprintf( filename, "%shelp.log", LOG_DIR );

            if ( ch->desc && ch->desc->original )
            {
                sprintf( buf, "%s :: %-14s : [%s]", strtime, ch->desc->original->name, argument );
            }
            else
            {
                sprintf( buf, "%s :: %-14s : [%s]", strtime, ch->name, argument );
            }

            /* Stops logging at 5 megs */
            if ( file_size( filename ) > 500000 )
                return;

            append_to_file( filename, buf );
        }
        return;
    }

    send_to_pager_color( "&R", ch );

    if ( pHelp->level >= 0 && str_cmp( argument, "imotd" ) && !nifty_is_name_prefix( argument, "news_" ) )
    {
        send_to_pager( pHelp->keyword, ch );
        send_to_pager( "\n\r", ch );
    }

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SOUND ) )
        send_to_pager( "!!SOUND(help)", ch );

    /*
        Strip leading '.' to allow initial blanks.
    */
    if ( pHelp->text[0] == '.' )
        send_to_pager_color( pHelp->text + 1, ch );
    else
        send_to_pager_color( pHelp->text, ch );

    return;
}

void do_news( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' || atoi( argument ) < 1 || atoi( argument ) > 9 )
    {
        send_to_char( "&zSyntax: NEWS <page 1-9>\n\r", ch );
        return;
    }
    else
    {
        set_pager_color( AT_NOTE, ch );
        sprintf( buf, "news_%d", atoi( argument ) );
        do_help( ch, buf );
    }

    return;
}

void do_changes( CHAR_DATA* ch, char* argument )
{
    set_pager_color( AT_NOTE, ch );
    do_help( ch, "changes" );
}

extern char* help_greeting;     /* so we can edit the greeting online */

/*
    Help editor                          -Thoric
*/
void do_hedit( CHAR_DATA* ch, char* argument )
{
    HELP_DATA* pHelp;

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor.\n\r", ch );
        return;
    }

    switch ( ch->substate )
    {
        default:
            break;

        case SUB_HELP_EDIT:
            if ( ( pHelp = ch->dest_buf ) == NULL )
            {
                bug( "hedit: sub_help_edit: NULL ch->dest_buf", 0 );
                stop_editing( ch );
                return;
            }

            if ( help_greeting == pHelp->text )
                help_greeting = NULL;

            STRFREE( pHelp->text );
            pHelp->text = copy_buffer( ch );

            if ( !help_greeting )
                help_greeting = pHelp->text;

            stop_editing( ch );
            return;
    }

    if ( ( pHelp = get_help( ch, argument ) ) == NULL ) /* new help */
    {
        HELP_DATA* tHelp;
        char argnew[MAX_INPUT_LENGTH];
        int lev;
        bool new_help = TRUE;

        for ( tHelp = first_help; tHelp; tHelp = tHelp->next )
            if ( !str_cmp( argument, tHelp->keyword ) )
            {
                pHelp = tHelp;
                new_help = FALSE;
                break;
            }

        if ( new_help )
        {
            if ( isdigit( argument[0] ) )
            {
                lev = number_argument( argument, argnew );
                argument = argnew;
            }
            else
                lev = get_trust( ch );

            CREATE( pHelp, HELP_DATA, 1 );
            pHelp->keyword = STRALLOC( strupper( argument ) );
            pHelp->text    = STRALLOC( "" );
            pHelp->level   = lev;
            add_help( pHelp );
        }
    }

    ch->substate = SUB_HELP_EDIT;
    ch->dest_buf = pHelp;
    start_editing( ch, pHelp->text );
}

/*
    News editor                                                  -Thoric
*/
void do_newswrite( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    HELP_DATA* pHelp;
    /* Help file to be editing */
    strcpy( buf, "news_1" );

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor. Sucks for you.\n\r", ch );
        return;
    }

    /* Make sure it was bestowed */
    if ( !( ch->pcdata && ch->pcdata->bestowments && is_name( "newswrite", ch->pcdata->bestowments ) ) )
    {
        send_to_char( "Huh?", ch );
        return;
    }

    switch ( ch->substate )
    {
        default:
            break;

        case SUB_HELP_EDIT:
            if ( ( pHelp = ch->dest_buf ) == NULL )
            {
                bug( "newswrite: sub_help_edit: NULL ch->dest_buf", 0 );
                stop_editing( ch );
                return;
            }

            if ( help_greeting == pHelp->text )
                help_greeting = NULL;

            STRFREE( pHelp->text );
            pHelp->text = copy_buffer( ch );

            if ( !help_greeting )
                help_greeting = pHelp->text;

            stop_editing( ch );
            /* Save the file - Ghost */
            strcpy( buf, "save" );
            do_hset( ch, buf );
            return;
    }

    if ( argument[0] == '\0' || atoi( argument ) < 1 || atoi( argument ) > 9 )
    {
        send_to_char( "&zSyntax: NEWSWRITE <page 1-9>\n\r", ch );
        return;
    }

    sprintf( buf, "news_%d", atoi( argument ) );

    if ( ( pHelp = get_help( ch, buf ) ) == NULL ) /* new help */
    {
        HELP_DATA* tHelp;
        char argnew[MAX_INPUT_LENGTH];
        int lev;
        bool new_help = TRUE;

        for ( tHelp = first_help; tHelp; tHelp = tHelp->next )
            if ( !str_cmp( buf, tHelp->keyword ) )
            {
                pHelp = tHelp;
                new_help = FALSE;
                break;
            }

        if ( new_help )
        {
            CREATE( pHelp, HELP_DATA, 1 );
            pHelp->keyword = STRALLOC( strupper( buf ) );
            pHelp->text    = STRALLOC( "" );
            pHelp->level   = 1;
            add_help( pHelp );
        }
    }

    ch->substate = SUB_HELP_EDIT;
    ch->dest_buf = pHelp;
    start_editing( ch, pHelp->text );
}

/*
    Stupid leading space muncher fix             -Thoric
*/
char* help_fix( char* text )
{
    char* fixed;

    if ( !text )
        return "";

    fixed = strip_cr( text );

    if ( fixed[0] == ' ' )
        fixed[0] = '.';

    return fixed;
}

void do_hset( CHAR_DATA* ch, char* argument )
{
    HELP_DATA* pHelp;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: hset <field> [value] [help page]\n\r",   ch );
        send_to_char( "\n\r",                       ch );
        send_to_char( "Field being one of:\n\r",            ch );
        send_to_char( "  level keyword remove save\n\r",        ch );
        return;
    }

    if ( !str_cmp( arg1, "save" ) )
    {
        FILE* fpout;
        log_string_plus( "Saving help.are...", LOG_NORMAL, LEVEL_GREATER );
        rename( "help.are", "help.are.bak" );
        fclose( fpReserve );

        if ( ( fpout = fopen( "help.are", "w" ) ) == NULL )
        {
            bug( "hset save: fopen", 0 );
            perror( "help.are" );
            fpReserve = fopen( NULL_FILE, "r" );
            return;
        }

        fprintf( fpout, "#HELPS\n\n" );

        for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
            fprintf( fpout, "%d %s~\n%s~\n\n",
                     pHelp->level, pHelp->keyword, help_fix( pHelp->text ) );

        fprintf( fpout, "0 $~\n\n\n#$\n" );
        fclose( fpout );
        fpReserve = fopen( NULL_FILE, "r" );
        send_to_char( "Saved.\n\r", ch );
        return;
    }

    if ( str_cmp( arg1, "remove" ) )
        argument = one_argument( argument, arg2 );

    if ( ( pHelp = get_help( ch, argument ) ) == NULL )
    {
        send_to_char( "Cannot find help on that subject.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "remove" ) )
    {
        UNLINK( pHelp, first_help, last_help, next, prev );
        STRFREE( pHelp->text );
        STRFREE( pHelp->keyword );
        DISPOSE( pHelp );
        send_to_char( "Removed.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "level" ) )
    {
        pHelp->level = atoi( arg2 );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "keyword" ) )
    {
        STRFREE( pHelp->keyword );
        pHelp->keyword = STRALLOC( strupper( arg2 ) );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    do_hset( ch, "" );
}

/*
    Show help topics in a level range                -Thoric
    Idea suggested by Gorog
    prefix keyword indexing added by Fireblade
*/
void do_hlist( CHAR_DATA* ch, char* argument )
{
    int min, max, minlimit, maxlimit, cnt;
    char arg[MAX_INPUT_LENGTH];
    HELP_DATA* help;
    bool minfound, maxfound;
    char* idx;
    maxlimit = get_trust( ch );
    minlimit = maxlimit >= LEVEL_GREATER ? -1 : 0;
    min = minlimit;
    max  = maxlimit;
    idx = NULL;
    minfound = FALSE;
    maxfound = FALSE;

    for ( argument = one_argument( argument, arg ); arg[0] != '\0';
            argument = one_argument( argument, arg ) )
    {
        if ( !isdigit( arg[0] ) )
        {
            if ( idx )
            {
                set_char_color( AT_GREEN, ch );
                ch_printf( ch, "You may only use a single keyword to index the list.\n\r" );
                return;
            }

            idx = STRALLOC( arg );
        }
        else
        {
            if ( !minfound )
            {
                min = URANGE( minlimit, atoi( arg ), maxlimit );
                minfound = TRUE;
            }
            else if ( !maxfound )
            {
                max = URANGE( minlimit, atoi( arg ), maxlimit );
                maxfound = TRUE;
            }
            else
            {
                set_char_color( AT_GREEN, ch );
                ch_printf( ch, "You may only use two level limits.\n\r" );
                return;
            }
        }
    }

    if ( min > max )
    {
        int temp = min;
        min = max;
        max = temp;
    }

    set_pager_color( AT_GREEN, ch );
    pager_printf( ch, "Help Topics in level range %d to %d:\n\r\n\r", min, max );

    for ( cnt = 0, help = first_help; help; help = help->next )
        if ( help->level >= min && help->level <= max
                &&  ( !idx || nifty_is_name_prefix( idx, help->keyword ) ) )
        {
            pager_printf( ch, "  %3d %s\n\r", help->level, help->keyword );
            ++cnt;
        }

    if ( cnt )
        pager_printf( ch, "\n\r%d pages found.\n\r", cnt );
    else
        send_to_char( "None found.\n\r", ch );

    if ( idx )
        STRFREE( idx );

    return;
}


/*
    Latest version of do_who eliminates redundant code by using linked lists.
    Shows imms separately, indicates guest and retired immortals.
*/
void do_who( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char stat_str[MAX_INPUT_LENGTH];
    char char_name[MAX_INPUT_LENGTH];
    char race_text[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA* d;
    BOT_DATA* bTmp;
    int iRace;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    int bCnt = 0;
    bool rgfRace[MAX_RACE];
    bool fRaceRestrict;
    FILE* whoout;
    WHO_DATA* cur_who = NULL;
    WHO_DATA* next_who = NULL;
    WHO_DATA* first_a = NULL;
    WHO_DATA* first_p = NULL;
    WHO_DATA* first_m = NULL;

    if ( !ch )
        return;

    /*
        Set default arguments.
    */
    iLevelLower    = 0;
    iLevelUpper    = 200;

    for ( iRace = 0; iRace < MAX_RACE; iRace++ )
        rgfRace[iRace] = FALSE;

    /*
        Parse arguments.
    */
    nNumber = 0;

    for ( ;; )
    {
        char arg[MAX_STRING_LENGTH];
        argument = one_argument( argument, arg );

        if ( arg[0] == '\0' )
            break;

        if ( is_number( arg ) )
        {
            switch ( ++nNumber )
            {
                case 1:
                    iLevelLower = atoi( arg );
                    break;

                case 2:
                    iLevelUpper = atoi( arg );
                    break;

                default:
                    send_to_char( "Only two level numbers allowed.\n\r", ch );
                    return;
            }
        }
    }

    /*
        Now find matching chars.
    */
    nMatch = 0;
    buf[0] = '\0';

    /*
        List the bots first, so the show up on the end.
    */
    for ( bTmp = first_bot; bTmp; bTmp = bTmp->next )
    {
        char const* race;

        if ( !bTmp->loaded )
            continue;

        nMatch++;
        bCnt++;
        strcpy( char_name, "" );
        sprintf( race_text, "&z(&BLevel %-2.2d&z) ", bTmp->level );
        race = race_text;

        if ( bTmp->respawn > 0 )
            sprintf( stat_str, "&r( &RRespawn in %d seconds &r) ", bTmp->respawn );
        else
            sprintf( stat_str, "&z( &WActive &z) " );

        sprintf( buf, "%s%s&C%-15s&z- %s\n\r", race, "&z(&WB&z) ", bTmp->name, stat_str );
        /* First make the structure. */
        CREATE( cur_who, WHO_DATA, 1 );
        cur_who->text = str_dup( buf );

        if ( bTmp->race == RACE_ALIEN )
            cur_who->type = WT_ALIEN;
        else if ( bTmp->race == RACE_MARINE )
            cur_who->type = WT_MARINE;
        else
            cur_who->type = WT_PREDATOR;

        /* Then put it into the appropriate list. */
        switch ( cur_who->type )
        {
            case WT_ALIEN:
                cur_who->next = first_a;
                first_a = cur_who;
                break;

            case WT_MARINE:
                cur_who->next = first_m;
                first_m = cur_who;
                break;

            case WT_PREDATOR:
                cur_who->next = first_p;
                first_p = cur_who;
                break;
        }
    }

    /* start from last to first to get it in the proper order */
    for ( d = last_descriptor; d; d = d->prev )
    {
        CHAR_DATA* wch;
        char const* race;

        /*
            If there not PLAYING or EDITING, skip them.
            If the character can't see the wch(and wch is a Imm), skip them.
            If they dont have a original character, skip them.
        */
        if ( ( !can_see( ch, d->character ) && IS_IMMORTAL( d->character ) )
                || d->original )
            continue;

        wch = d->original ? d->original : d->character;

        if ( wch == NULL )
            continue;

        /*
            Check for the NOWHO flag. -Ghost
        */
        if ( xIS_SET( wch->pcdata->flags, PCFLAG_NOWHO ) && ( wch->top_level > ch->top_level ) )
            continue;

        nMatch++;
        strcpy( char_name, "" );

        if ( IS_IMMORTAL( wch ) )
        {
            sprintf( race_text, "&z(&BImmortal&z) " );
        }
        else
        {
            sprintf( race_text, "&z(&BLevel %-2.2d&z) ", wch->top_level );
        }

        race = race_text;

        if ( xIS_SET( wch->act, PLR_WIZINVIS ) )
            sprintf( stat_str, "&z( &WInvisible - %d&z ) ", wch->pcdata->wizinvis );
        else if ( is_spectator( wch ) )
            sprintf( stat_str, "&r( &RRespawn in %d seconds &r) ", wch->pcdata->respawn );
        else if ( wch->desc && wch->desc->connected == CON_EDITING )
            sprintf( stat_str, "&g( &GBusy Writing &g) " );
        else if ( wch->desc && wch->desc->connected != CON_PLAYING )
            sprintf( stat_str, "&g( &GUsing the Menu &g) " );
        else if ( xIS_SET( wch->act, PLR_FREEZE ) )
            sprintf( stat_str, "&z&c( &CFrozen &c) " );
        else if ( xIS_SET( wch->act, PLR_AFK ) )
            sprintf( stat_str, "&z( &WAway from keyboard &z) " );
        else if ( xIS_SET( wch->act, PLR_SILENCE ) )
            sprintf( stat_str, "&z&c( &CSilenced %s&c) ", is_home( wch ) ? "" : "+ Deployed " );
        else if ( !is_home( wch ) )
            sprintf( stat_str, "&g( &GDeployed &g) " );
        else if ( IS_IDLE( d ) )
            sprintf( stat_str, "&O( &YIdle &O) " );
        else
            sprintf( stat_str, "&z( &WActive &z) " );

        sprintf( buf, "%s%s&C%-15s&z- %s\n\r", race, is_ignored( ch, wch->name ) ? "&r(&R-&r) " : "    ", wch->name, stat_str );
        /* First make the structure. */
        CREATE( cur_who, WHO_DATA, 1 );
        cur_who->text = str_dup( buf );

        if ( wch->race == RACE_ALIEN )
            cur_who->type = WT_ALIEN;
        else if ( wch->race == RACE_MARINE )
            cur_who->type = WT_MARINE;
        else
            cur_who->type = WT_PREDATOR;

        /* Then put it into the appropriate list. */
        switch ( cur_who->type )
        {
            case WT_ALIEN:
                cur_who->next = first_a;
                first_a = cur_who;
                break;

            case WT_MARINE:
                cur_who->next = first_m;
                first_m = cur_who;
                break;

            case WT_PREDATOR:
                cur_who->next = first_p;
                first_p = cur_who;
                break;
        }
    }

    /*
        Ok, now we have three separate linked lists and what remains is to
        display the information and clean up.
    */
    if ( first_p )
    {
        send_to_pager( "&G----------------------------[ Predators ]---------------------------&W\n\r", ch );

        for ( cur_who = first_p; cur_who; cur_who = next_who )
        {
            send_to_pager( cur_who->text, ch );
            next_who = cur_who->next;
            DISPOSE( cur_who->text );
            DISPOSE( cur_who );
        }
    }

    if ( first_m )
    {
        send_to_pager( "&G-----------------------------[ Marines ]----------------------------&W\n\r", ch );

        for ( cur_who = first_m; cur_who; cur_who = next_who )
        {
            send_to_pager( cur_who->text, ch );
            next_who = cur_who->next;
            DISPOSE( cur_who->text );
            DISPOSE( cur_who );
        }
    }

    if ( first_a )
    {
        send_to_pager( "&G-----------------------------[ Aliens ]-----------------------------&W\n\r", ch );

        for ( cur_who = first_a; cur_who; cur_who = next_who )
        {
            send_to_pager( cur_who->text, ch );
            next_who = cur_who->next;
            DISPOSE( cur_who->text );
            DISPOSE( cur_who );
        }
    }

    set_char_color( AT_YELLOW, ch );
    ch_printf( ch, "%d character%s. (%d users - %s)\n\r", nMatch, nMatch == 1 ? "" : "s", nMatch - bCnt, bCnt > 0 ? "Bots online" : "Bots offline" );
    ch_printf( ch, "%d max players this boot. (%d total players ever)\n\r", sysdata.maxplayers, sysdata.alltimemax );
    return;
}

void do_compare( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA* obj1;
    OBJ_DATA* obj2;
    int value1;
    int value2;
    char* msg;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Compare what to what?\n\r", ch );
        return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        for ( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content )
        {
            if ( obj2->wear_loc != WEAR_NONE
                    &&   can_see_obj( ch, obj2 )
                    &&   obj1->item_type == obj2->item_type )
                break;
        }

        if ( !obj2 )
        {
            send_to_char( "You aren't wearing anything comparable.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }
    }

    msg     = NULL;
    value1  = 0;
    value2  = 0;

    if ( obj1 == obj2 )
    {
        msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
        msg = "You can't compare $p and $P.";
    }
    else
    {
        switch ( obj1->item_type )
        {
            default:
                msg = "You can't compare $p and $P.";
                break;

            case ITEM_ARMOR:
                value1 = obj1->value[0];
                value2 = obj2->value[0];
                break;

            case ITEM_WEAPON:
                value1 = obj1->value[1] + obj1->value[2];
                value2 = obj2->value[1] + obj2->value[2];
                break;
        }
    }

    if ( !msg )
    {
        if ( value1 == value2 )
            msg = "$p and $P look about the same.";
        else if ( value1  > value2 )
            msg = "$p looks better than $P.";
        else
            msg = "$p looks worse than $P.";
    }

    act( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );
    return;
}


/*
    void do_where( CHAR_DATA *ch, char *argument )
    {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    if (get_trust(ch) < LEVEL_IMMORTAL)
    {
       send_to_char( "If only life were really that simple...\n\r" , ch);
       return;
    }

    one_argument( argument, arg );

    set_pager_color( AT_PERSON, ch );
    if ( arg[0] == '\0' )
    {
        if (get_trust(ch) >= LEVEL_IMMORTAL)
           send_to_pager( "Players logged in:\n\r", ch );
        else
           pager_printf( ch, "Players near you in %s:\n\r", ch->in_room->area->name );
    found = FALSE;
    for ( d = first_descriptor; d; d = d->next )
        if ( (d->connected == CON_PLAYING || d->connected == CON_EDITING )
        && ( victim = d->character ) != NULL
        &&   !IS_NPC(victim)
        &&   victim->in_room
        &&   (victim->in_room->area == ch->in_room->area || get_trust(ch) >= LEVEL_IMMORTAL )
        &&   can_see( ch, victim ) )
        {
        found = TRUE;
        pager_printf( ch, "%-28s %s\n\r",
            victim->name, victim->in_room->name );
        }
    if ( !found )
        send_to_char( "None\n\r", ch );
    }
    else
    {
    found = FALSE;
    for ( victim = first_char; victim; victim = victim->next )
        if ( victim->in_room
        &&   victim->in_room->area == ch->in_room->area
        &&   !IS_AFFECTED(victim, AFF_HIDE)
        &&   !IS_AFFECTED(victim, AFF_SNEAK)
        &&   can_see( ch, victim )
        &&   is_name( arg, victim->name ) )
        {
        found = TRUE;
        pager_printf( ch, "%-28s %s\n\r",
            PERS(victim, ch), victim->in_room->name );
        break;
        }
    if ( !found )
        act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
    }*/


/* Added from Smaug 1.4a */
void do_where( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* victim;
    DESCRIPTOR_DATA* d;
    bool found;
    bool full = FALSE;
    one_argument( argument, arg );

    if ( !str_cmp( arg, "full" ) && IS_IMMORTAL( ch ) )
        full = TRUE;

    if ( !full )
    {
        if ( arg[0] != '\0'
                &&   ( victim = get_char_world( ch, arg ) ) && !IS_NPC( victim )
                &&   xIS_SET( victim->pcdata->flags, PCFLAG_DND )
                &&   get_trust( ch ) < get_trust( victim ) )
        {
            act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
            return;
        }
    }

    set_pager_color( AT_PERSON, ch );

    if ( arg[0] == '\0' || full )
    {
        if ( full )
            send_to_pager( "Players logged in:\n\r", ch );

        if ( !full )
            pager_printf( ch, "Players near you in %s:\n\r", ch->in_room->area->name );

        found = FALSE;

        for ( d = first_descriptor; d; d = d->next )
            if ( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
                    && ( victim = d->character ) != NULL
                    &&   !IS_NPC( victim )
                    &&   victim->in_room
                    &&   ( victim->in_room->area == ch->in_room->area || full )
                    &&   can_see( ch, victim )
                    && ( get_trust( ch ) >= get_trust( victim )
                         ||   !xIS_SET( victim->pcdata->flags, PCFLAG_DND ) )
               ) /* if victim has the DND flag ch must outrank them */
            {
                found = TRUE;
                /*      if ( CAN_PKILL( victim ) )
                          set_pager_color( AT_PURPLE, ch );
                        else
                          set_pager_color( AT_PERSON, ch );
                */
                sprintf( buf, "&P%-13s  ", victim->name );
                send_to_pager_color( buf, ch );

                if ( IS_IMMORTAL( victim ) && victim->top_level > 100 )
                    send_to_pager_color( "&P(&WImmortal&P)\t", ch );
                else
                    send_to_pager( "\t\t\t", ch );

                sprintf( buf, "&P%s\n\r", victim->in_room->name );
                send_to_pager_color( buf, ch );
            }

        if ( !found )
            send_to_char( "None\n\r", ch );
    }
    else
    {
        found = FALSE;

        for ( victim = first_char; victim; victim = victim->next )
            if ( victim->in_room
                    &&   victim->in_room->area == ch->in_room->area
                    &&   !IS_AFFECTED( victim, AFF_HIDE )
                    &&   !IS_AFFECTED( victim, AFF_SNEAK )
                    &&   can_see( ch, victim )
                    &&   is_name( arg, victim->name ) )
            {
                found = TRUE;
                pager_printf( ch, "%-28s %s\n\r",
                              PERS( victim, ch ), victim->in_room->name );
                break;
            }

        if ( !found )
            act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}


void do_consider( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA* wA = NULL;
    OBJ_DATA* wB = NULL;
    CHAR_DATA* victim;
    int diff, overall = 0;
    bool full = FALSE;
    char* msg;
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Consider killing whom?\n\r", ch );
        return;
    }

    if ( arg2[0] != '\0' )
    {
        if ( !str_cmp( arg2, "full" ) )
        {
            full = TRUE;
        }
        else
        {
            send_to_char( "&RSyntax: Consider <victim> [Optional - FULL]\n\r", ch );
            return;
        }
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They're not here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You decide you're pretty sure you could take yourself in a fight.\n\r", ch );
        return;
    }

    act( AT_ACTION, "$n examines $N closely looking for any weaknesses.", ch, NULL, victim, TO_NOTVICT );
    act( AT_ACTION, "You examine $N closely looking for any weaknesses.", ch, NULL, victim, TO_CHAR );
    act( AT_ACTION, "$n examines you closely looking for weaknesses.", ch, NULL, victim, TO_VICT );
    overall = 0;
    diff = ch->hit * 100 / UMAX( 1, victim->hit );

    if ( diff <=  10 )
    {
        msg = "$E is currently far healthier than you are.";
        overall -= 3;
    }
    else if ( diff <=  50 )
    {
        msg = "$E is currently much healthier than you are.";
        overall -= 2;
    }
    else if ( diff <=  75 )
    {
        msg = "$E is currently slightly healthier than you are.";
        overall -= 1;
    }
    else if ( diff <= 125 )
    {
        msg = "$E is currently about as healthy as you are.";
    }
    else if ( diff <= 200 )
    {
        msg = "You are currently slightly healthier than $M.";
        overall += 1;
    }
    else if ( diff <= 500 )
    {
        msg = "You are currently much healthier than $M.";
        overall += 2;
    }
    else
    {
        msg = "You are currently far healthier than $M.";
        overall += 3;
    }

    if ( full )
        act( AT_LBLUE, msg, ch, NULL, victim, TO_CHAR );

    {
        diff = 0;
        wA = get_eq_char( ch, WEAR_WIELD );
        wB = get_eq_char( victim, WEAR_WIELD );

        if ( wA )
        {
            diff -= ( ( wA->value[1] * 10 ) + ( wA->value[2] * 10 ) ) / 2;

            if ( ( wA = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL )
                diff -= ( ( wA->value[1] * 10 ) + ( wA->value[2] * 10 ) ) / 2;
        }

        if ( wB )
        {
            diff += ( ( wB->value[1] * 10 ) + ( wB->value[2] * 10 ) ) / 2;

            if ( ( wB = get_eq_char( victim, WEAR_DUAL_WIELD ) ) != NULL )
                diff += ( ( wB->value[1] * 10 ) + ( wB->value[2] * 10 ) ) / 2;
        }

        if ( diff <= -25 )
        {
            msg = "You are far better armed than $M.";
            overall += 3;
        }
        else if ( diff <= -12 )
        {
            msg = "$E is not as well armed as you are.";
            overall += 2;
        }
        else if ( diff <=  -5 )
        {
            msg = "$E doesn't seem quite as well armed as you.";
            overall += 1;
        }
        else if ( diff <=   5 )
        {
            msg = "You are about as well armed as $M.";
        }
        else if ( diff <=  12 )
        {
            msg = "$E is slightly better armed than you are.";
            overall -= 1;
        }
        else if ( diff <=  25 )
        {
            msg = "$E seems to be much better armed than you are.";
            overall -= 2;
        }
        else
        {
            msg = "$E is far better armed than you are.";
            overall -= 3;
        }

        if ( full )
            act( AT_LBLUE, msg, ch, NULL, victim, TO_CHAR );
    }
    diff = overall;

    if ( diff <= -10 )
    {
        msg = "Conclusion - You wouldn't last more than a few seconds.";
    }
    else if ( diff <=  -7 )
    {
        msg = "Conclusion - You would need a lot of luck to beat $M.";
    }
    else if ( diff <=  -3 )
    {
        msg = "Conclusion - You would need some luck to beat $M.";
    }
    else if ( diff <=   2 )
    {
        msg = "Conclusion - It would be a very close fight.";
    }
    else if ( diff <=   6 )
    {
        msg = "Conclusion - You shouldn't have a lot of trouble defeating $M.";
    }
    else if ( diff <=   9 )
    {
        msg = "Conclusion - $N is no match for you. You could easily beat $M.";
    }
    else
    {
        msg = "Conclusion - $E wouldn't last more than a few seconds against you.";
    }

    act( AT_ACTION, msg, ch, NULL, victim, TO_CHAR );
    return;
}



/*
    Place any skill types you don't want them to be able to practice
    normally in this list.  Separate each with a space.
    (Uses an is_name check). -- Altrag
*/
#define CANT_PRAC "Tongue"

void do_practice( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char tmp[MAX_STRING_LENGTH];
    int clevel = 0, sn = 0;

    if ( IS_NPC( ch ) )
        return;

    if ( argument[0] == '\0' )
    {
        sh_int  cnt;
        cnt = 0;
        send_to_char( "\n\r", ch );

        for ( sn = 0; ( sn < top_sn || clevel < UMIN( 20, ch->top_level ) ) ; sn++ )
        {
            if ( sn >= top_sn )
            {
                sn = 0;
                clevel++;
                continue;
            }

            if ( !skill_table[sn]->name )
                break;

            if ( skill_table[sn]->min_level != clevel )
                continue;

            if ( skill_table[sn]->race < 0 || skill_table[sn]->race > 2 )
                continue;

            if ( skill_table[sn]->race != ch->race )
                continue;

            if ( ch->pcdata->learned[sn] <= 0 )
                continue;

            ++cnt;

            if ( ch->pcdata->learned[sn] <= 1 )
                strcpy( buf, "Basic" );

            if ( ch->pcdata->learned[sn] == 2 )
                strcpy( buf, "Advanced" );

            if ( ch->pcdata->learned[sn] >= 3 )
                strcpy( buf, "Expert" );

            if ( skill_table[sn]->reset <= 0 )
            {
                sprintf( tmp, "&w&zSkill: &B%-20s &z- ( Level: &W%-8s &z- Ready: [%s&z] )\n\r",
                         capitalize( skill_table[sn]->name ), buf,
                         drawbar( 10, 1, 1, "&G", "&g" ) );
            }
            else
            {
                sprintf( tmp, "&zSkill: &B%-20s &z- ( Level: &W%-8s &z- Ready: [%s&z] )\n\r",
                         capitalize( skill_table[sn]->name ), buf,
                         drawbar( 10, ch->pcdata->prepared[sn], skill_table[sn]->reset, "&G", "&g" ) );
            }

            send_to_char( tmp, ch );
        }
    }

    return;
}

void do_steacher( CHAR_DATA* ch, char* argument )
{
    /*
        CHAR_DATA *victim;
        char * buf[MAX_STRING_LENGTH];
        char arg[MAX_STRING_LENGTH];
        int sn, vnum;
        bool fMob;

        if ( IS_NPC(ch) )
           return;

        set_pager_color( AT_MAGIC, ch );
        send_to_pager("\n\r------------[ Missing Teachers ]-------------\n\r", ch);

        for ( sn = 0; sn < top_sn; sn++ )
        {
           if ( !skill_table[sn]->name )
              break;

           if ( skill_table[sn]->guild < 0 || skill_table[sn]->guild >= MAX_ABILITY )
              continue;

           fMob = FALSE;

           if ( skill_table[sn]->teachers && skill_table[sn]->teachers[0] != '\0' )
           {
              strcpy( buf, skill_table[sn]->teachers );
              for ( ; ; )
              {
                buf = one_argument( buf, arg );
                if ( arg[0] != '\0' )
                {
                   vnum = atoi(arg);
                   if ( vnum > 0 )
                    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
                     fMob = TRUE;
                }
                else
                 if ( buf[0] != '\0' ) { }
                else
                 break;
              }
           }
           else
              continue;

           if ( fMob )
              continue;

           sprintf( buf, "Skill: %-25s    [No Valid Teachers]", skill_table[sn]->name );
           send_to_char( buf, ch );
        }
        return;
    */
}

void do_teach( CHAR_DATA* ch, char* argument )
{
    bug( "act_info.c: incomplete code for do_teach." );
    return;
}


void do_password( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char* pArg;
    char* pwdnew;
    char* p;
    char cEnd;

    if ( IS_NPC( ch ) )
        return;

    /*
        Can't use one_argument here because it smashes case.
        So we just steal all its code.  Bleagh.
    */
    pArg = arg1;

    while ( isspace( *argument ) )
        argument++;

    cEnd = ' ';

    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }

        *pArg++ = *argument++;
    }

    *pArg = '\0';
    pArg = arg2;

    while ( isspace( *argument ) )
        argument++;

    cEnd = ' ';

    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }

        *pArg++ = *argument++;
    }

    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: password <old> <new>.\n\r", ch );
        return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
        WAIT_STATE( ch, 40 );
        send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
        return;
    }

    if ( strlen( arg2 ) < 5 )
    {
        send_to_char(
            "New password must be at least five characters long.\n\r", ch );
        return;
    }

    /*
        No tilde allowed because of player file format.
    */
    pwdnew = crypt( arg2, ch->name );

    for ( p = pwdnew; *p != '\0'; p++ )
    {
        if ( *p == '~' )
        {
            send_to_char(
                "New password not acceptable, try again.\n\r", ch );
            return;
        }
    }

    DISPOSE( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );

    if ( IS_SET( sysdata.save_flags, SV_PASSCHG ) )
        save_char_obj( ch );

    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_socials( CHAR_DATA* ch, char* argument )
{
    int iHash;
    int col = 0;
    SOCIALTYPE* social;
    set_pager_color( AT_PLAIN, ch );

    for ( iHash = 0; iHash < 27; iHash++ )
        for ( social = social_index[iHash]; social; social = social->next )
        {
            pager_printf( ch, "%-12s", social->name );

            if ( ++col % 6 == 0 )
                send_to_pager( "\n\r", ch );
        }

    if ( col % 6 != 0 )
        send_to_pager( "\n\r", ch );

    return;
}


void do_commands( CHAR_DATA* ch, char* argument )
{
    int col, cnt = 0;
    bool found;
    int hash;
    CMDTYPE* command;
    col = 0;

    if ( argument[0] == '\0' )
    {
        for ( hash = 0; hash < 126; hash++ )
            for ( command = command_hash[hash]; command; command = command->next )
                if ( command->level <= get_trust( ch ) && ( command->name[0] != 'm' || command->name[1] != 'p' ) )
                {
                    pager_printf( ch, "&z(&C%3d&z) &B%-16s", command->level, command->name );
                    cnt++;

                    if ( ++col % 3 == 0 )
                        send_to_pager( "\n\r", ch );
                }

        if ( col % 3 != 0 )
            send_to_pager( "\n\r", ch );
    }
    else
    {
        found = FALSE;

        for ( hash = 0; hash < 126; hash++ )
            for ( command = command_hash[hash]; command; command = command->next )
                if ( command->level <  LEVEL_HERO
                        &&   command->level <= get_trust( ch )
                        &&  !str_prefix( argument, command->name )
                        &&  ( command->name[0] != 'm'
                              &&   command->name[1] != 'p' ) )
                {
                    cnt++;
                    pager_printf( ch, "&z(&C%3d&z) &B%-16s", command->level, command->name );
                    found = TRUE;

                    if ( ++col % 3 == 0 )
                        send_to_pager( "\n\r", ch );
                }

        if ( col % 3 != 0 )
            send_to_pager( "\n\r", ch );

        if ( !found )
            ch_printf( ch, "&zNo command found under &C%s&z.\n\r", argument );
    }

    if ( cnt > 0 && found )
        ch_printf( ch, "&zThere are &C%d&z commands listed above.\n\r", cnt );

    return;
}


void do_channels( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SILENCE ) )
        {
            send_to_char( "You are silenced.\n\r", ch );
            return;
        }

        send_to_char( "&z[ Channels ] Setting \n\r", ch );
        send_to_char( !xIS_SET( ch->deaf, CHANNEL_CHAT )
                      ? "&z[ &CCHAT     &z] &GChannel OPEN\n\r"
                      : "&z[ &Cchat     &z] &RChannel CLOSED\n\r",
                      ch );
        send_to_char( !xIS_SET( ch->deaf, CHANNEL_OOC )
                      ? "&z[ &COOC      &z] &GChannel OPEN\n\r"
                      : "&z[ &Cooc      &z] &RChannel CLOSED\n\r",
                      ch );
        send_to_char( !xIS_SET( ch->deaf, CHANNEL_QUEST )
                      ? "&z[ &CQUEST    &z] &GChannel OPEN\n\r"
                      : "&z[ &Cquest    &z] &RChannel CLOSED\n\r",
                      ch );
        send_to_char( !xIS_SET( ch->deaf, CHANNEL_TELLS )
                      ? "&z[ &CTELLS    &z] &GChannel OPEN\n\r"
                      : "&z[ &Ctells    &z] &RChannel CLOSED\n\r",
                      ch );
        send_to_char( !xIS_SET( ch->deaf, CHANNEL_WARTALK )
                      ? "&z[ &CWARTALK  &z] &GChannel OPEN\n\r"
                      : "&z[ &Cwartalk  &z] &RChannel CLOSED\n\r",
                      ch );

        if ( ch->top_level >= 100 )
        {
            send_to_char( !xIS_SET( ch->deaf, CHANNEL_AVTALK )
                          ? "&z[ &CAVTALK   &z] &GChannel OPEN\n\r"
                          : "&z[ &Cavtalk   &z] &RChannel CLOSED\n\r",
                          ch );
        }

        if ( IS_IMMORTAL( ch ) )
        {
            send_to_char( !xIS_SET( ch->deaf, CHANNEL_IMMTALK )
                          ? "&z[ &CIMMTALK  &z] &GChannel OPEN\n\r"
                          : "&z[ &Cimmtalk  &z] &RChannel CLOSED\n\r",
                          ch );
            send_to_char( !xIS_SET( ch->deaf, CHANNEL_PRAY )
                          ? "&z[ &CPRAY     &z] &GChannel OPEN\n\r"
                          : "&z[ &Cpray     &z] &RChannel CLOSED\n\r",
                          ch );
        }

        send_to_char( !xIS_SET( ch->deaf, CHANNEL_MUSIC )
                      ? "&z[ &CMUSIC    &z] &GChannel OPEN\n\r"
                      : "&z[ &Cmusic    &z] &RChannel CLOSED\n\r",
                      ch );
        send_to_char( !xIS_SET( ch->deaf, CHANNEL_ASK )
                      ? "&z[ &CASK      &z] &GChannel OPEN\n\r"
                      : "&z[ &Cask      &z] &RChannel CLOSED\n\r",
                      ch );
        send_to_char( !xIS_SET( ch->deaf, CHANNEL_SHOUT )
                      ? "&z[ &CSHOUT    &z] &GChannel OPEN\n\r"
                      : "&z[ &Cshout    &z] &RChannel CLOSED\n\r",
                      ch );
        send_to_char( !xIS_SET( ch->deaf, CHANNEL_YELL )
                      ? "&z[ &CYELL     &z] &GChannel OPEN\n\r"
                      : "&z[ &Cyell     &z] &RChannel CLOSED\n\r",
                      ch );
        send_to_char( !xIS_SET( ch->deaf, CHANNEL_INFO )
                      ? "&z[ &CINFO     &z] &GChannel OPEN\n\r"
                      : "&z[ &Cinfo     &z] &RChannel CLOSED\n\r",
                      ch );

        if ( IS_IMMORTAL( ch ) )
        {
            send_to_char( !xIS_SET( ch->deaf, CHANNEL_MONITOR )
                          ? "&z[ &CMONITOR  &z] &GChannel OPEN\n\r"
                          : "&z[ &Cmonitor  &z] &RChannel CLOSED\n\r",
                          ch );
        }

        send_to_char( !xIS_SET( ch->deaf, CHANNEL_NEWBIE )
                      ? "&z[ &CNEWBIE   &z] &GChannel OPEN\n\r"
                      : "&z[ &Cnewbie   &z] &RChannel CLOSED\n\r",
                      ch );

        if ( get_trust( ch ) >= sysdata.log_level )
        {
            send_to_char( !xIS_SET( ch->deaf, CHANNEL_LOG )
                          ? "&z[ &CLOG      &z] &GChannel OPEN\n\r"
                          : "&z[ &Clog      &z] &RChannel CLOSED\n\r",
                          ch );
            send_to_char( !xIS_SET( ch->deaf, CHANNEL_BUILD )
                          ? "&z[ &CBUILD    &z] &GChannel OPEN\n\r"
                          : "&z[ &Cbuild    &z] &RChannel CLOSED\n\r",
                          ch );
            send_to_char( !xIS_SET( ch->deaf, CHANNEL_COMM )
                          ? "&z[ &CCOMM     &z] &GChannel OPEN\n\r"
                          : "&z[ &Ccomm     &z] &RChannel CLOSED\n\r",
                          ch );
        }
    }
    else
    {
        bool fClear;
        bool ClearAll;
        int bit;
        bit = 0;
        ClearAll = FALSE;

        if ( arg[0] == '+' )
            fClear = TRUE;
        else if ( arg[0] == '-' )
            fClear = FALSE;
        else
        {
            send_to_char( "Channels -channel or +channel?\n\r", ch );
            return;
        }

        if ( !str_cmp( arg + 1, "chat"     ) )
            bit = CHANNEL_CHAT;
        else if ( !str_cmp( arg + 1, "ooc"      ) )
            bit = CHANNEL_OOC;
        else if ( !str_cmp( arg + 1, "quest"    ) )
            bit = CHANNEL_QUEST;
        else if ( !str_cmp( arg + 1, "tells"    ) )
            bit = CHANNEL_TELLS;
        else if ( !str_cmp( arg + 1, "immtalk"  ) )
            bit = CHANNEL_IMMTALK;
        else if ( !str_cmp( arg + 1, "log"      ) )
            bit = CHANNEL_LOG;
        else if ( !str_cmp( arg + 1, "build"    ) )
            bit = CHANNEL_BUILD;
        else if ( !str_cmp( arg + 1, "pray"     ) )
            bit = CHANNEL_PRAY;
        else if ( !str_cmp( arg + 1, "avtalk"   ) )
            bit = CHANNEL_AVTALK;
        else if ( !str_cmp( arg + 1, "monitor"  ) )
            bit = CHANNEL_MONITOR;
        else if ( !str_cmp( arg + 1, "newbie"   ) )
            bit = CHANNEL_NEWBIE;
        else if ( !str_cmp( arg + 1, "music"    ) )
            bit = CHANNEL_MUSIC;
        else if ( !str_cmp( arg + 1, "ask"      ) )
            bit = CHANNEL_ASK;
        else if ( !str_cmp( arg + 1, "shout"    ) )
            bit = CHANNEL_SHOUT;
        else if ( !str_cmp( arg + 1, "yell"     ) )
            bit = CHANNEL_YELL;
        else if ( !str_cmp( arg + 1, "comm"     ) )
            bit = CHANNEL_COMM;
        else if ( !str_cmp( arg + 1, "wartalk"  ) )
            bit = CHANNEL_WARTALK;
        else if ( !str_cmp( arg + 1, "info"     ) )
            bit = CHANNEL_INFO;
        else if ( !str_cmp( arg + 1, "all"      ) )
            ClearAll = TRUE;
        else
        {
            send_to_char( "Set or clear which channel?\n\r", ch );
            return;
        }

        if ( ( fClear ) && ( ClearAll ) )
        {
            xREMOVE_BIT ( ch->deaf, CHANNEL_CHAT );
            xREMOVE_BIT ( ch->deaf, CHANNEL_QUEST );
            /*     REMOVE_BIT (ch->deaf, CHANNEL_IMMTALK); */
            xREMOVE_BIT ( ch->deaf, CHANNEL_PRAY );
            xREMOVE_BIT ( ch->deaf, CHANNEL_MUSIC );
            xREMOVE_BIT ( ch->deaf, CHANNEL_ASK );
            xREMOVE_BIT ( ch->deaf, CHANNEL_SHOUT );
            xREMOVE_BIT ( ch->deaf, CHANNEL_YELL );
            xREMOVE_BIT ( ch->deaf, CHANNEL_OOC );
            xREMOVE_BIT ( ch->deaf, CHANNEL_TELLS );
            xREMOVE_BIT ( ch->deaf, CHANNEL_WARTALK );
            xREMOVE_BIT ( ch->deaf, CHANNEL_NEWBIE );

            if ( ch->top_level >= 100 )
                xREMOVE_BIT ( ch->deaf, CHANNEL_AVTALK );
        }
        else if ( ( !fClear ) && ( ClearAll ) )
        {
            xSET_BIT ( ch->deaf, CHANNEL_CHAT );
            xSET_BIT ( ch->deaf, CHANNEL_QUEST );
            /*     SET_BIT (ch->deaf, CHANNEL_IMMTALK); */
            xSET_BIT ( ch->deaf, CHANNEL_PRAY );
            xSET_BIT ( ch->deaf, CHANNEL_MUSIC );
            xSET_BIT ( ch->deaf, CHANNEL_ASK );
            xSET_BIT ( ch->deaf, CHANNEL_SHOUT );
            xSET_BIT ( ch->deaf, CHANNEL_YELL );
            xSET_BIT ( ch->deaf, CHANNEL_OOC );
            xSET_BIT ( ch->deaf, CHANNEL_TELLS );
            xSET_BIT ( ch->deaf, CHANNEL_WARTALK );
            xSET_BIT ( ch->deaf, CHANNEL_NEWBIE );

            if ( ch->top_level >= 100 )
                xSET_BIT ( ch->deaf, CHANNEL_AVTALK );
        }
        else if ( fClear )
        {
            xREMOVE_BIT ( ch->deaf, bit );
        }
        else
        {
            xSET_BIT    ( ch->deaf, bit );
        }

        send_to_char( "Ok.\n\r", ch );
    }

    return;
}


/*
    display WIZLIST file                     -Thoric
*/
void do_wizlist( CHAR_DATA* ch, char* argument )
{
    set_pager_color( AT_IMMORT, ch );
    show_file( ch, WIZLIST_FILE );
}

/*
    Contributed by Grodyn.
    Updated by Ghost. ;)
*/
void do_config( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "&z[ Keyword  ] Option\n\r", ch );
        send_to_char(  xIS_SET( ch->act, PLR_FLEE )
                       ? "&z[&C+FLEE&z     ] You flee if you get attacked.\n\r"
                       : "&z[&C-flee&z     ] You fight back if you get attacked.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->pcdata->flags, PCFLAG_NORECALL )
                       ? "&z[&C+NORECALL&z ] You fight to the death, link-dead or not.\n\r"
                       : "&z[&C-norecall&z ] You try to recall if fighting link-dead.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_AUTOEXIT )
                       ? "&z[&C+AUTOEXIT&z ] You automatically see exits.\n\r"
                       : "&z[&C-autoexit&z ] You don't automatically see exits.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_AUTOLOOT )
                       ? "&z[&C+AUTOLOOT&z ] You automatically loot corpses.\n\r"
                       : "&z[&C-autoloot&z ] You don't automatically loot corpses.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_AUTOSAC )
                       ? "&z[&C+AUTOSAC&z  ] You automatically sacrifice corpses.\n\r"
                       : "&z[&C-autosac&z  ] You don't automatically sacrifice corpses.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->pcdata->flags, PCFLAG_GAG )
                       ? "&z[&C+GAG&z      ] You see only necessary battle text.\n\r"
                       : "&z[&C-gag&z      ] You see full battle text.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->pcdata->flags, PCFLAG_PAGERON )
                       ? "&z[&C+PAGER&z    ] Long output is page-paused.\n\r"
                       : "&z[&C-pager&z    ] Long output scrolls to the end.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_BLANK )
                       ? "&z[&C+BLANK&z    ] You have a blank line before your prompt.\n\r"
                       : "&z[&C-blank&z    ] You have no blank line before your prompt.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_BRIEF )
                       ? "&z[&C+BRIEF&z    ] You see brief descriptions.\n\r"
                       : "&z[&C-brief&z    ] You see long descriptions.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_COMBINE )
                       ? "&z[&C+COMBINE&z  ] You see object lists in combined format.\n\r"
                       : "&z[&C-combine&z  ] You see object lists in single format.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->pcdata->flags, PCFLAG_NOINTRO )
                       ? "&z[&C+NOINTRO&z  ] You don't see the ascii intro screen on login.\n\r"
                       : "&z[&C-nointro&z  ] You see the ascii intro screen on login.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_PROMPT )
                       ? "&z[&C+PROMPT&z   ] You have a prompt.\n\r"
                       : "&z[&C-prompt&z   ] You don't have a prompt.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_TELNET_GA )
                       ? "&z[&C+TELNETGA&z ] You receive a telnet GA sequence.\n\r"
                       : "&z[&C-telnetga&z ] You don't receive a telnet GA sequence.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_ANSI )
                       ? "&z[&C+ANSI&z     ] You receive ANSI color sequences.\n\r"
                       : "&z[&C-ansi&z     ] You don't receive receive ANSI colors.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_SOUND )
                       ? "&z[&C+SOUND&z    ] You have MSP support.\n\r"
                       : "&z[&C-sound&z    ] You don't have MSP support.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_CENSOR )
                       ? "&z[&C+CENSOR&z   ] Profanity filtering is on.\n\r"
                       : "&z[&C-censor&z   ] Profanity filtering is off.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_COMPACT )
                       ? "&z[&C+COMPACT&z  ] Compact score is on.\n\r"
                       : "&z[&C-compact&z  ] Compact score is off.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_WHOIS )
                       ? "&z[&C+WHOIS&z    ] Your WHOIS record is invisible.\n\r"
                       : "&z[&C-whois&z    ] People can view your WHOIS record.\n\r"
                       , ch );
        send_to_char(  xIS_SET( ch->act, PLR_SHOVEDRAG )
                       ? "&z[&C+SHOVEDRAG&z] You allow yourself to be shoved and dragged around.\n\r"
                       : "&z[&C-shovedrag&z] You'd rather not be shoved or dragged around.\n\r"
                       , ch );
        send_to_char( xIS_SET( ch->pcdata->flags, PCFLAG_NOSUMMON )
                      ? "&z[&C+NOSUMMON&z ] You do not allow other players to summon you.\n\r"
                      : "&z[&C-nosummon&z ] You allow other players to summon you.\n\r"
                      , ch );
        send_to_char( xIS_SET( ch->pcdata->flags, PCFLAG_SHOWRESET )
                      ? "&z[&C+SHOWRESET&z] You will see skill reset notifications.\n\r"
                      : "&z[&C-showreset&z] You won't see skill reset notifications.\n\r"
                      , ch );
        send_to_char( xIS_SET( ch->pcdata->flags, PCFLAG_SHOWAMMO )
                      ? "&z[&C+SHOWAMMO &z] You will see ammo remaining notifications.\n\r"
                      : "&z[&C-showammo &z] You won't see ammo remaining notifications.\n\r"
                      , ch );

        if ( IS_IMMORTAL( ch ) )
            send_to_char(  xIS_SET( ch->pcdata->flags, PCFLAG_NOWHO )
                           ? "&z[&G+NOWHO&z    ] You dont show up on the who listing.\n\r"
                           : "&z[&G-nowho&z    ] You show up on the who listing.\n\r"
                           , ch );

        if ( IS_IMMORTAL( ch ) )
            send_to_char(  xIS_SET( ch->act, PLR_ROOMVNUM )
                           ? "&z[&G+VNUM&z     ] You can see the VNUM of a room.\n\r"
                           : "&z[&G-vnum&z     ] You do not see the VNUM of a room.\n\r"
                           , ch );

        if ( IS_IMMORTAL( ch ) )            /* Added 10/16 by Kuran of SWR */
            send_to_char( xIS_SET( ch->pcdata->flags, PCFLAG_ROOM )
                          ? "&z[&G+ROOMFLAGS&z] You will see room flags.\n\r"
                          : "&z[&G-roomflags&z] You will not see room flags.\n\r"
                          , ch );

        send_to_char(  xIS_SET( ch->act, PLR_SILENCE )
                       ? "&z[&R+SILENCE&z  ] You are silenced.\n\r"
                       : ""
                       , ch );
        send_to_char( !xIS_SET( ch->act, PLR_NO_EMOTE )
                      ? ""
                      : "&z[&R+NOEMOTE&z  ] You can't emote.\n\r"
                      , ch );
        send_to_char( !xIS_SET( ch->act, PLR_NO_TELL )
                      ? ""
                      : "&z[&R+NOTELL&z   ] You can't use 'tell'.\n\r"
                      , ch );
        send_to_char( !xIS_SET( ch->act, PLR_LITTERBUG )
                      ? ""
                      : "&z[&R+LITTER&z  ] A convicted litterbug. You cannot drop anything.\n\r"
                      , ch );
    }
    else
    {
        bool fSet;
        int bit = 0;

        if ( arg[0] == '+' )
            fSet = TRUE;
        else if ( arg[0] == '-' )
            fSet = FALSE;
        else
        {
            send_to_char( "Config -option or +option?\n\r", ch );
            return;
        }

        if ( !str_prefix( arg + 1, "autoexit" ) )
            bit = PLR_AUTOEXIT;
        else if ( !str_prefix( arg + 1, "autoloot" ) )
            bit = PLR_AUTOLOOT;
        else if ( !str_prefix( arg + 1, "autosac"  ) )
            bit = PLR_AUTOSAC;
        else if ( !str_prefix( arg + 1, "blank"    ) )
            bit = PLR_BLANK;
        else if ( !str_prefix( arg + 1, "brief"    ) )
            bit = PLR_BRIEF;
        else if ( !str_prefix( arg + 1, "combine"  ) )
            bit = PLR_COMBINE;
        else if ( !str_prefix( arg + 1, "prompt"   ) )
            bit = PLR_PROMPT;
        else if ( !str_prefix( arg + 1, "telnetga" ) )
            bit = PLR_TELNET_GA;
        else if ( !str_prefix( arg + 1, "ansi"     ) )
            bit = PLR_ANSI;
        else if ( !str_prefix( arg + 1, "sound"    ) )
            bit = PLR_SOUND;
        else if ( !str_prefix( arg + 1, "censor"   ) )
            bit = PLR_CENSOR;
        else if ( !str_prefix( arg + 1, "compact"  ) )
            bit = PLR_COMPACT;
        else if ( !str_prefix( arg + 1, "whois"    ) )
            bit = PLR_WHOIS;
        else if ( !str_prefix( arg + 1, "flee"     ) )
            bit = PLR_FLEE;
        else if ( !str_prefix( arg + 1, "nice"     ) )
            bit = PLR_NICE;
        else if ( !str_prefix( arg + 1, "shovedrag" ) )
            bit = PLR_SHOVEDRAG;
        else if ( IS_IMMORTAL( ch )
                  &&   !str_prefix( arg + 1, "vnum"     ) )
            bit = PLR_ROOMVNUM;

        if ( bit )
        {
            if ( fSet )
                xSET_BIT    ( ch->act, bit );
            else
                xREMOVE_BIT ( ch->act, bit );

            send_to_char( "Ok.\n\r", ch );
            return;
        }
        else
        {
            if ( !str_prefix( arg + 1, "norecall" ) )
                bit = PCFLAG_NORECALL;
            else if ( !str_prefix( arg + 1, "nointro"  ) )
                bit = PCFLAG_NOINTRO;
            else if ( !str_prefix( arg + 1, "nosummon" ) )
                bit = PCFLAG_NOSUMMON;
            else if ( !str_prefix( arg + 1, "gag"      ) )
                bit = PCFLAG_GAG;
            else if ( !str_prefix( arg + 1, "pager"    ) )
                bit = PCFLAG_PAGERON;
            else if ( !str_prefix( arg + 1, "showreset" ) )
                bit = PCFLAG_SHOWRESET;
            else if ( !str_prefix( arg + 1, "showammo" ) )
                bit = PCFLAG_SHOWAMMO;
            else if ( !str_prefix( arg + 1, "nowho"    )
                      && ( IS_IMMORTAL( ch ) ) )
                bit = PCFLAG_NOWHO;
            else if ( !str_prefix( arg + 1, "roomflags" )
                      && ( IS_IMMORTAL( ch ) ) )
                bit = PCFLAG_ROOM;
            else
            {
                send_to_char( "Config which option?\n\r", ch );
                return;
            }

            if ( fSet )
                xSET_BIT    ( ch->pcdata->flags, bit );
            else
                xREMOVE_BIT ( ch->pcdata->flags, bit );

            send_to_char( "Ok.\n\r", ch );
            return;
        }
    }

    return;
}


void do_credits( CHAR_DATA* ch, char* argument )
{
    do_help( ch, "credits" );
}


extern int top_area;

/*
    New do_areas, written by Fireblade, last modified - 4/27/97

     Syntax: area            ->      lists areas in alphanumeric order
             area <a>        ->      lists areas with soft max less than
                                                      parameter a
             area <a> <b>    ->      lists areas with soft max bewteen
                                                      numbers a and b
             area old        ->      list areas in order loaded

*/
void do_areas( CHAR_DATA* ch, char* argument )
{
    char* header_string1 = "\n\r   &WAuthor    &z|             &WArea"
                           "                     &z| "
                           "&WRecommended &z|  &WEnforced&z\n\r";
    char* header_string2 = "&z-------------+-----------------"
                           "---------------------+----"
                           "---------+-----------\n\r";
    char* print_string = "&B%-12s &z| &B%-36s &z| &C%4d &W- &C%-4d &z| &C%3d &W- "
                         "&C%-3d&z \n\r";
    AREA_DATA* pArea;
    int lower_bound = 0;
    int upper_bound = MAX_LEVEL + 1;
    /* make sure is to init. > max area level */
    char arg[MAX_STRING_LENGTH];
    argument = one_argument( argument, arg );

    if ( arg[0] != '\0' )
    {
        if ( !is_number( arg ) )
        {
            if ( !strcmp( arg, "old" ) )
            {
                // set_pager_color( AT_PLAIN, ch );
                send_to_pager( header_string1, ch );
                send_to_pager( header_string2, ch );

                for ( pArea = first_area; pArea; pArea = pArea->next )
                {
                    pager_printf( ch, print_string,
                                  pArea->author, pArea->name,
                                  pArea->low_soft_range,
                                  pArea->hi_soft_range,
                                  pArea->low_hard_range,
                                  pArea->hi_hard_range );
                }

                return;
            }
            else
            {
                send_to_char( "Area may only be followed by numbers, or 'old'.\n\r", ch );
                return;
            }
        }

        upper_bound = atoi( arg );
        lower_bound = upper_bound;
        argument = one_argument( argument, arg );

        if ( arg[0] != '\0' )
        {
            if ( !is_number( arg ) )
            {
                send_to_char( "Area may only be followed by numbers.\n\r", ch );
                return;
            }

            upper_bound = atoi( arg );
            argument = one_argument( argument, arg );

            if ( arg[0] != '\0' )
            {
                send_to_char( "Only two level numbers allowed.\n\r", ch );
                return;
            }
        }
    }

    if ( lower_bound > upper_bound )
    {
        int swap = lower_bound;
        lower_bound = upper_bound;
        upper_bound = swap;
    }

    // set_pager_color( AT_PLAIN, ch );
    send_to_pager( header_string1, ch );
    send_to_pager( header_string2, ch );

    for ( pArea = first_area; pArea; pArea = pArea->next )
    {
        if ( pArea->hi_soft_range >= lower_bound
                &&  pArea->low_soft_range <= upper_bound )
        {
            pager_printf( ch, print_string,
                          pArea->author, pArea->name,
                          pArea->low_soft_range,
                          pArea->hi_soft_range,
                          pArea->low_hard_range,
                          pArea->hi_hard_range );
        }
    }

    return;
}

void do_afk( CHAR_DATA* ch, char* argument )
{
    if ( IS_NPC( ch ) )
        return;

    if xIS_SET( ch->act, PLR_AFK )
    {
        xREMOVE_BIT( ch->act, PLR_AFK );
        send_to_char( "You are no longer afk.\n\r", ch );
        act( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_ROOM );
    }
    else
    {
        xSET_BIT( ch->act, PLR_AFK );
        send_to_char( "You are now afk.\n\r", ch );
        act( AT_GREY, "$n is now afk.", ch, NULL, NULL, TO_ROOM );
        return;
    }
}

void do_slist( CHAR_DATA* ch, char* argument )
{
    bug( "act_info.c: incomplete code for do_slist." );
    return;
}

void do_whois( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA* victim;
    buf[0] = '\0';

    if ( IS_NPC( ch ) )
        return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "You must input the name of a player online.\n\r", ch );
        return;
    }

    strcat( buf, "0." );
    strcat( buf, argument );

    if ( ( ( victim = get_char_world_full( ch, buf ) ) == NULL ) )
    {
        ch_printf( ch, "&zCould not locate %s in the database.\n\r", argument );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "That's not a player!\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->act, PLR_WHOIS ) && !IS_IMMORTAL( ch ) && ch != victim )
    {
        ch_printf( ch, "&zCould not locate %s in the database.\n\r", argument );
        return;
    }

    /*
        Ghost is 42 year old male human.
        Ghost is level 200.
        Ghost is a member of the Empire.
        Ghost is a member of the Roleplay Council.
        Ghost is currently in room 100.
        Ghost's homepage can be found at http://www.archsysinc.com/finalstands/
        Ghost's personal bio:
        [Bio]
        Info for immortals:
        Ghost was authorized by Raven.
        ...
    */
    ch_printf( ch, "&z%s is &C%d &zyear old &C%s %s.\n\r", victim->name, get_age( victim ),
               victim->sex == SEX_MALE ? "male" : victim->sex == SEX_FEMALE ? "female" : "neutral",
               get_race( victim ) );
    ch_printf( ch, "&z%s is level &C%d&z. (&zExperience worth: &W%d&z)\n\r", victim->name, victim->top_level, get_exp_worth( victim ) );
    ch_printf( ch, "&z%s has earned &C%d&z experience points to date.\n\r", victim->name, victim->maxexp );

    if ( xIS_SET( victim->act, PLR_NCOUNCIL ) )
        ch_printf( ch, "&z%s is a member of &Bthe Roleplay Council.\n\r", victim->name );

    if ( IS_IMMORTAL( ch ) )
        ch_printf( ch, "&z%s is currently in room &C%d.\n\r", victim->name, victim->in_room->vnum );

    if ( victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0' )
    {
        ch_printf( ch, "&z%s's homepage can be found at &C%s.\n\r", victim->name, victim->pcdata->homepage );
    }
    else
    {
        ch_printf( ch, "&z%s has not set a personal homepage.&W\n\r", victim->name );
    }

    if ( victim->pcdata->bio && victim->pcdata->bio[0] != '\0' )
    {
        ch_printf( ch, "&z%s's personal bio: &W\n\r%s", victim->name, victim->pcdata->bio );
    }
    else
    {
        ch_printf( ch, "&z%s has not written a personal bio.&W\n\r", victim->name );
    }

    if ( IS_IMMORTAL( ch ) )
    {
        send_to_char( "&z----------------------------------------------------\n\r", ch );
        send_to_char( "&BInfo for immortals:\n\r", ch );

        if ( victim->pcdata->authed_by && victim->pcdata->authed_by[0] != '\0' )
            ch_printf( ch, "&z%s was authorized by &C%s.\n\r",
                       victim->name, victim->pcdata->authed_by );

        ch_printf( ch, "&z%s is %shelled at the moment.\n\r",
                   victim->name,
                   ( victim->pcdata->release_date == 0 ) ? "not " : "" );

        if ( victim->pcdata->release_date != 0 )
            ch_printf( ch, "&z%s was helled by &C%s&z, and will be released on &C%24.24s.\n\r",
                       victim->sex == SEX_MALE ? "He" :
                       victim->sex == SEX_FEMALE ? "She" : "It",
                       victim->pcdata->helled_by,
                       ctime( &victim->pcdata->release_date ) );

        if ( get_trust( victim ) < get_trust( ch ) )
        {
            sprintf( buf2, "list %s", buf );
            do_comment( ch, buf2 );
        }

        if ( xIS_SET( victim->act, PLR_SILENCE ) || xIS_SET( victim->act, PLR_NO_EMOTE )
                || xIS_SET( victim->act, PLR_NO_TELL ) )
        {
            sprintf( buf2, "&zThis player has the following flags set:" );

            if ( xIS_SET( victim->act, PLR_SILENCE ) )
                strcat( buf2, " &Csilence" );

            if ( xIS_SET( victim->act, PLR_NO_EMOTE ) )
                strcat( buf2, " &Cnoemote" );

            if ( xIS_SET( victim->act, PLR_NO_TELL ) )
                strcat( buf2, " &Cnotell" );

            strcat( buf2, ".\n\r" );
            send_to_char( buf2, ch );
        }

        if ( victim->desc && victim->desc->host[0] != '\0' ) /* added by Gorog */
        {
            sprintf ( buf2, "&z%s's IP info: &C%s ", victim->name, victim->desc->hostip );

            if ( get_trust( ch ) >= LEVEL_GOD )
            {
                strcat ( buf2, victim->desc->user );
                strcat ( buf2, "@" );
                strcat ( buf2, victim->desc->host );
            }

            strcat ( buf2, "\n\r" );
            send_to_char( buf2, ch );
        }

        if ( get_trust( ch ) >= LEVEL_GOD && get_trust( ch ) >= get_trust( victim ) && victim->pcdata )
        {
            sprintf ( buf2, "&zEmail: &C%s\n\r", victim->pcdata->email );
            send_to_char( buf2, ch );
        }
    }
}

void do_pager( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    set_char_color( AT_NOTE, ch );
    argument = one_argument( argument, arg );

    if ( !*arg )
    {
        if ( xIS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) )
            do_config( ch, "-pager" );
        else
            do_config( ch, "+pager" );

        return;
    }

    if ( !is_number( arg ) )
    {
        send_to_char( "Set page pausing to how many lines?\n\r", ch );
        return;
    }

    ch->pcdata->pagerlen = atoi( arg );

    if ( ch->pcdata->pagerlen < 5 )
        ch->pcdata->pagerlen = 5;

    ch_printf( ch, "Page pausing set to %d lines.\n\r", ch->pcdata->pagerlen );
    return;
}

/*
    Version command for the users
*/
void do_version( CHAR_DATA* ch, char* argument )
{
    if ( IS_NPC( ch ) )
        return;

    ch_printf( ch, "\n\r&zVersion Information: ---\n\r" );
    ch_printf( ch, "&zCodebase: &WSWR_AvP &z(Version: &Y%s.%s.%s&z)\n\r", SWR_VERSION_MAJOR, SWR_VERSION_MINOR, SWR_VERSION_REVISION );
    ch_printf( ch, "&zLast Compiled on &C%s &zat &C%s&z.\n\r", __DATE__, __TIME__ );
    return;
}

/*
    HTML Output subroutines by Ghost.
    -The WHO list auto-runs every 5 minutes.
    -All webpages update upon reboot.
*/
void do_html( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument(  argument, arg   );

    if ( !IS_IMMORTAL( ch ) )
    {
        send_to_char( "&RThis command may ONLY be used by Immortals!", ch );
        return;
    }

    if ( !arg || arg[0] == '\0' )
    {
        send_to_char( "&R\n\rSyntax: HTML <[Page]|ALL>\n\r", ch );
        send_to_char( "&RCurrent Pages:\n\r", ch );
        send_to_char( "&RAMMOCODE, WHO, WIZLIST, ALLROOMS, OBJSTATS\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "who" ) )
    {
        HTML_Who();
    }
    else if ( !str_cmp( arg, "allrooms" ) )
    {
        if ( str_cmp( ch->name, "ghost" ) )
            send_to_char( "&YSorry! This command is being debugged, and is restricted to Ghost-only use!\n\r", ch );
        else
            HTML_Allrooms();
    }
    else if ( !str_cmp( arg, "ammocode" ) )
    {
        HTML_Ammostats();
    }
    else if ( !str_cmp( arg, "objstats" ) )
    {
        if ( str_cmp( ch->name, "ghost" ) )
            send_to_char( "&YSorry! This command is being debugged, and is restricted to Ghost-only use!\n\r", ch );
        else
            HTML_Objstats();
    }
    else if ( !str_cmp( arg, "wizlist" ) )
    {
        // HTML_Wizlist();
    }
    else
    {
        do_html( ch, "" );
    }

    return;
}

void HTML_Who( void )
{
    FILE* fp;
    DESCRIPTOR_DATA* d;
    char fname[MAX_INPUT_LENGTH];
    char buf[2 * MAX_INPUT_LENGTH];
    char buf2[2 * MAX_INPUT_LENGTH];
    int ppl = 0;
    buf[0] = '\0';
    buf2[0] = '\0';
    fclose( fpReserve );
    /* IMPORTANT: This file needs to exist before you attempt to run this. */
    sprintf( fname, "../webpage/avp_who.htm" );

    if ( ( fp = fopen( fname, "w" ) ) == NULL )
    {
        bug( "avp_who.htm: fopen", 0 );
        perror( "avp_who.htm" );
    }
    else
    {
        fprintf( fp, "<html>\n" );
        fprintf( fp, "<head>\n" );
        fprintf( fp, "<title>AvP: Legend - Who list</title>\n" );
        fprintf( fp, "</head>\n" );
        fprintf( fp, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" LINK=""#00FFFF""\n" );
        fprintf( fp, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n" );
        fprintf( fp, "<h1><center>Who's on AvP?</center></h1>\n" );
        fprintf( fp, "<b><center><font size=""2"">\n" );

        for ( d = first_descriptor; d; d = d->next )
        {
            if ( !d )
                continue;

            if ( !d->character )
                continue;

            if ( !d->character->pcdata )
                continue;

            if ( !IS_IMMORTAL( d->character ) || !xIS_SET( d->character->pcdata->flags, PCFLAG_NOWHO ) )
                ppl++;
        }

        if ( ppl > 0 )
        {
            fprintf( fp, "-There are [ %d ] people currently on AvP-<br>\n", ppl );
            fprintf( fp, "<br><hr color=""#FFFFFF""><br>\n" );
        }

        for ( d = first_descriptor; d; d = d->next )
        {
            CHAR_DATA* wch;

            if ( d->connected != CON_PLAYING )
                continue;

            wch   = ( d->original != NULL ) ? d->original : d->character;

            if ( !IS_IMMORTAL( wch ) )
                fprintf( fp, "--==|    %s    |==--<br>\n", conv_hcolor( wch->pcdata->title ) );
        }

        fprintf( fp, "<br><br><hr color=""#FFFFFF""><br>\n" );

        for ( d = first_descriptor; d; d = d->next )
        {
            CHAR_DATA* wch;

            if ( d->connected != CON_PLAYING )
                continue;

            wch   = ( d->original != NULL ) ? d->original : d->character;

            if ( !IS_IMMORTAL( wch ) || !xIS_SET( wch->pcdata->flags, PCFLAG_NOWHO ) )
            {
                fprintf( fp, "PLAYER: %s <font color= #C0C0C0><br>\n", conv_hcolor( wch->pcdata->title ) );
                fprintf( fp, "RACE: [%s%s<font color= #C0C0C0>]<br>\n", conv_hcolor( "&B" ), get_race( wch ) );
                fprintf( fp, "LEVEL: [%s%d<font color= #C0C0C0>]<br>\n", conv_hcolor( "&B" ), wch->top_level );

                if ( wch->pcdata->bio[0] != '\0' )
                    fprintf( fp, "BIO: %s<font color= #C0C0C0>\n", conv_hcolor( wch->pcdata->bio ) );

                fprintf( fp, "<br><br><hr color=""#FFFFFF""><br>\n" );
            }
        }

        fprintf( fp, "<font face=""Times New Roman"">\n" );

        if ( ppl > 0 )
            fprintf( fp, "-There are [ %d ] people currently on AvP-<br>\n", ppl );
        else
            fprintf( fp, "-There is nobody currently connected to AvP-<br>\n" );

        sprintf( buf, "<br>This file last updated at %s Eastern Time.\n", ( ( char* ) ctime( &current_time ) ) );
        fprintf( fp, buf );
        fprintf( fp, "</center></font>\n" );
        fprintf( fp, "</body>\n" );
        fprintf( fp, "</html>\n" );
        fclose( fp );
    }

    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }

    return;
}

void HTML_Allrooms( void )
{
    FILE* fp;
    AREA_DATA* area;
    ROOM_INDEX_DATA* room;
    EXIT_DATA* pexit;
    char buf[2 * MAX_INPUT_LENGTH];
    char buf2[2 * MAX_INPUT_LENGTH];
    int areaz = 0, roomz = 0, vnum = 0;
    static char* dir_text[] = { "north", "east", "south", "west", "up", "down", "northeast", "northwest", "southeast", "southwest", "somewhere" };
    buf[0] = '\0';
    buf2[0] = '\0';
    fclose( fpReserve );

    /* IMPORTANT: This file needs to exist before you attempt to run this. */
    if ( ( fp = fopen( "/../webpage/avp_allrooms.htm", "w" ) ) == NULL )
    {
        bug( "avp_allrooms.htm: fopen", 0 );
        perror( "avp_allrooms.htm" );
    }
    else
    {
        fprintf( fp, "<html>\n" );
        fprintf( fp, "<head>\n" );
        fprintf( fp, "<title>AvP: Legend - Areas and Rooms</title>\n" );
        fprintf( fp, "</head>\n" );
        fprintf( fp, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" LINK=""#00FFFF""\n" );
        fprintf( fp, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n" );
        fprintf( fp, "<h1><center>Areas and Rooms of AvP!</center></h1>\n" );
        fprintf( fp, "<br><h3><center>Navigatable Index of Rooms</center></h3>\n" );
        fprintf( fp, "<b><center><font size=""2"">\n" );
        fprintf( fp, "<br><hr color=""#FFFFFF""><br>\n" );
        fprintf( fp, "<br><h3>Areas</h3>\n" );
        fprintf( fp, "<br><hr color=""#FFFFFF""><br>\n" );

        for ( area = first_area ; area ; area = area->next )
        {
            if ( area == NULL || !area )
                continue;

            fprintf( fp, "%s<br>\n", area->name );
            areaz++;
        }

        fprintf( fp, "<br>-There are [ %d ] areas installed in AvP-<br><br>\n", areaz );

        for ( area = first_area ; area ; area = area->next )
        {
            if ( !area || area == NULL )
                continue;

            fprintf( fp, "<br><hr color=""#FFFFFF""><br>\n" );
            fprintf( fp, "---------------------=== %s ===---------------------<br>\n", area->name );

            for ( vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++ )
            {
                if ( ( room = get_room_index( vnum ) ) != NULL )
                {
                    fprintf( fp, "<p><a name=""%d""></a></p>", vnum );
                    fprintf( fp, "VNUM: %-5d    Room Name: %-30s<font color= #C0C0C0><br>\n", vnum, conv_hcolor( room->name ) );
                    fprintf( fp, "Flags: %s %s %s %s %s %s<br>\n",
                             xIS_SET( room->room_flags, ROOM_INDOORS )    ? "INDOORS"   : "",
                             xIS_SET( room->room_flags, ROOM_CAN_LAND )   ? "CANLAND"   : "",
                             xIS_SET( room->room_flags, ROOM_CAN_FLY )    ? "CANFLY"    : "",
                             xIS_SET( room->room_flags, ROOM_HOTEL )      ? "HOTEL"     : "",
                             xIS_SET( room->room_flags, ROOM_NOFLOOR )    ? "NOFLOOR"   : "",
                             xIS_SET( room->room_flags, ROOM_INDOORS )    ? "SPACECRAFT" : "" );

                    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
                    {
                        if ( pexit->to_room )
                        {
                            if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
                            {
                                fprintf( fp, "Exit: <a href=""?#%d"">%s to %d [Closed]</a><br>\n", pexit->to_room->vnum, capitalize( dir_text[pexit->vdir] ), pexit->to_room->vnum );
                            }
                            else if ( xIS_SET( pexit->exit_info, EX_WINDOW ) )
                            {
                                fprintf( fp, "Exit: <a href=""?#%d"">%s to %d [Window]</a><br>\n", pexit->to_room->vnum, capitalize( dir_text[pexit->vdir] ), pexit->to_room->vnum );
                            }
                            else if ( xIS_SET( pexit->exit_info, EX_xAUTO ) )
                            {
                                fprintf( fp, "Exit: <a href=""?#%d"">%s to %d [Autoexit]</a><br>\n", pexit->to_room->vnum, capitalize( pexit->keyword ), pexit->to_room->vnum );
                            }
                            else
                            {
                                fprintf( fp, "Exit: <a href=""?#%d"">%s to %d</a><br>\n", pexit->to_room->vnum, capitalize( dir_text[pexit->vdir] ), pexit->to_room->vnum );
                            }
                        }
                    }

                    roomz++;
                }
            }
        }

        fprintf( fp, "<br><hr color=""#FFFFFF""><br>\n" );
        fprintf( fp, "-There are [ %d ] rooms on AvP-<br><br>\n", roomz );
        sprintf( buf, "<br>This file last updated at %s Eastern Time.\n", ( ( char* ) ctime( &current_time ) ) );
        fprintf( fp, buf );
        fprintf( fp, "</center></font>\n" );
        fprintf( fp, "</body>\n" );
        fprintf( fp, "</html>\n" );
        fclose( fp );
    }

    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }

    return;
}

void HTML_Objstats( void )
{
    FILE* fp;
    AREA_DATA* area;
    OBJ_INDEX_DATA* pObjIndex;
    char buf[2 * MAX_INPUT_LENGTH];
    char buf2[2 * MAX_INPUT_LENGTH];
    int vnum = 0, objz = 0;
    buf[0] = '\0';
    buf2[0] = '\0';
    fclose( fpReserve );

    /* IMPORTANT: This file needs to exist before you attempt to run this. */
    if ( ( fp = fopen( "/home/bhaase/webpage/avp_objstats.htm", "w" ) ) == NULL )
    {
        bug( "objstats.htm: fopen", 0 );
        perror( "objstats.htm" );
    }
    else
    {
        fprintf( fp, "<html>\n" );
        fprintf( fp, "<head>\n" );
        fprintf( fp, "<title>AvP: Legend - Object Stats</title>\n" );
        fprintf( fp, "</head>\n" );
        fprintf( fp, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" LINK=""#00FFFF""\n" );
        fprintf( fp, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n" );
        fprintf( fp, "<h1><center>Object Statistics of AvP!</center></h1>\n" );
        fprintf( fp, "<b><center><font size=""2"">\n" );
        fprintf( fp, "<table><tr><td>Name</td><td>Vnum</td><td>Type</td><td>" );
        fprintf( fp, "Value0</td><td>Value1</td><td>Value2</td><td>Value3</td>" );
        fprintf( fp, "<td>Value4</td><td>Value5</td></tr>\n" );

        for ( area = first_area ; area ; area = area->next )
        {
            if ( !area || area == NULL )
                continue;

            for ( vnum = area->low_o_vnum; vnum <= area->hi_o_vnum; vnum++ )
            {
                if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
                    continue;

                fprintf( fp, "<tr><td>\n" );
                fprintf( fp, "%s", conv_hcolor( pObjIndex->short_descr ) );
                fprintf( fp, "</td><td>\n" );
                fprintf( fp, "%d", pObjIndex->vnum );
                fprintf( fp, "</td><td>\n" );
                fprintf( fp, "%s", o_types[pObjIndex->item_type] );
                fprintf( fp, "</td><td>\n" );
                sprintf( buf, "%d%s%d%s%d%s%d%s%d%s%d%s",
                         pObjIndex->value[0], "</td><td>\n",
                         pObjIndex->value[1], "</td><td>\n",
                         pObjIndex->value[2], "</td><td>\n",
                         pObjIndex->value[3], "</td><td>\n",
                         pObjIndex->value[4], "</td><td>\n",
                         pObjIndex->value[5], "</td></tr>\n" );
                fprintf( fp, buf );
                objz++;
            }
        }

        fprintf( fp, "</table><br><hr color=""#FFFFFF""><br>\n" );
        fprintf( fp, "-There are [ %d ] objects on AvP-<br><br>\n", objz );
        sprintf( buf, "<br>This file last updated at %s Eastern Time.\n", ( ( char* ) ctime( &current_time ) ) );
        fprintf( fp, buf );
        fprintf( fp, "</center></font>\n" );
        fprintf( fp, "</body>\n" );
        fprintf( fp, "</html>\n" );
        fclose( fp );
    }

    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }

    return;
}

void HTML_Ammostats( void )
{
    FILE* fp;
    AREA_DATA* area;
    OBJ_INDEX_DATA* pObjIndex;
    char buf[2 * MAX_INPUT_LENGTH];
    char buf2[2 * MAX_INPUT_LENGTH];
    int vnum = 0, objy = 0, objz = 0;
    buf[0] = '\0';
    buf2[0] = '\0';
    fclose( fpReserve );

    /* IMPORTANT: This file needs to exist before you attempt to run this. */
    if ( ( fp = fopen( "/home/bhaase/webpage/avp_ammocode.htm", "w" ) ) == NULL )
    {
        bug( "ammocode.htm: fopen", 0 );
        perror( "objstats.htm" );
    }
    else
    {
        fprintf( fp, "<html>\n" );
        fprintf( fp, "<head>\n" );
        fprintf( fp, "<title>AvP: Legend - Ammo Code Breakdown</title>\n" );
        fprintf( fp, "</head>\n" );
        fprintf( fp, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" LINK=""#00FFFF""\n" );
        fprintf( fp, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n" );
        fprintf( fp, "<h2><center>Ammunition Statistics of AvP!</center></h2>\n" );
        fprintf( fp, "<b><center><font size=""2"">\n" );
        fprintf( fp, "<table border=""1"" width=""90%""><tr><td width=""20%"">Name</td><td width=""20%"">Vnum</td>" );
        fprintf( fp, "<td width=""20%"">Ammo Code A</td><td width=""20%"">Ammo Code B</td><td width=""20%"">Ammo Code C</td></tr>" );

        for ( area = first_asort; area; area = area->next_sort )
        {
            if ( !area || area == NULL )
                continue;

            for ( vnum = area->low_o_vnum; vnum <= area->hi_o_vnum; vnum++ )
            {
                if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
                    continue;

                if ( pObjIndex->item_type != ITEM_AMMO )
                    continue;

                fprintf( fp, "<tr><td>\n" );
                fprintf( fp, "%s", conv_hcolor( pObjIndex->short_descr ) );
                fprintf( fp, "</td><td>\n" );
                fprintf( fp, "%d", pObjIndex->vnum );
                fprintf( fp, "</td><td>\n" );
                sprintf( buf, "%d%s%d%s%d%s",
                         pObjIndex->value[3], "</td><td>\n",
                         pObjIndex->value[4], "</td><td>\n",
                         pObjIndex->value[5], "</td></tr>\n" );
                fprintf( fp, buf );
                objy++;
            }
        }

        for ( area = first_bsort; area; area = area->next_sort )
        {
            if ( !area || area == NULL )
                continue;

            for ( vnum = area->low_o_vnum; vnum <= area->hi_o_vnum; vnum++ )
            {
                if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
                    continue;

                if ( pObjIndex->item_type != ITEM_AMMO )
                    continue;

                fprintf( fp, "<tr><td>\n" );
                fprintf( fp, "* %s", conv_hcolor( pObjIndex->short_descr ) );
                fprintf( fp, "</td><td>\n" );
                fprintf( fp, "%d", pObjIndex->vnum );
                fprintf( fp, "</td><td>\n" );
                sprintf( buf, "%d%s%d%s%d%s",
                         pObjIndex->value[3], "</td><td>\n",
                         pObjIndex->value[4], "</td><td>\n",
                         pObjIndex->value[5], "</td></tr>\n" );
                fprintf( fp, buf );
                objy++;
            }
        }

        fprintf( fp, "</table><br><hr color=""#FFFFFF""><br>\n" );
        fprintf( fp, "<h2><center>Weapon Statistics of AvP!</center></h2>\n" );
        fprintf( fp, "<b><center><font size=""2"">\n" );
        fprintf( fp, "<table border=""1"" width=""90%""><tr><td width=""25%"">Name</td><td width=""25%"">Vnum</td><td width=""25%"">Weapon Code</td>" );
        fprintf( fp, "<td width=""25%"">Ammo Code</td></tr>" );

        for ( area = first_asort; area; area = area->next_sort )
        {
            if ( !area || area == NULL )
                continue;

            for ( vnum = area->low_o_vnum; vnum <= area->hi_o_vnum; vnum++ )
            {
                if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
                    continue;

                if ( pObjIndex->item_type != ITEM_WEAPON )
                    continue;

                if ( pObjIndex->value[0] == WEAPON_KNIFE || pObjIndex->value[0] == WEAPON_SPEAR
                        || pObjIndex->value[0] == WEAPON_BLADE || pObjIndex->value[0] == WEAPON_ERANGED
                        || pObjIndex->value[0] == WEAPON_DISC || pObjIndex->value[0] == WEAPON_NATURAL )
                    continue;

                fprintf( fp, "<tr><td>\n" );
                fprintf( fp, "%s", conv_hcolor( pObjIndex->short_descr ) );
                fprintf( fp, "</td><td>\n" );
                fprintf( fp, "%d", pObjIndex->vnum );
                fprintf( fp, "</td><td>\n" );
                sprintf( buf, "%d%s%d%s",
                         pObjIndex->value[0], "</td><td>\n",
                         pObjIndex->value[1], "</td></tr>\n" );
                fprintf( fp, buf );
                objz++;
            }
        }

        for ( area = first_bsort; area; area = area->next_sort )
        {
            if ( !area || area == NULL )
                continue;

            for ( vnum = area->low_o_vnum; vnum <= area->hi_o_vnum; vnum++ )
            {
                if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
                    continue;

                if ( pObjIndex->item_type != ITEM_WEAPON )
                    continue;

                if ( pObjIndex->value[0] == WEAPON_KNIFE || pObjIndex->value[0] == WEAPON_SPEAR
                        || pObjIndex->value[0] == WEAPON_BLADE || pObjIndex->value[0] == WEAPON_ERANGED
                        || pObjIndex->value[0] == WEAPON_DISC || pObjIndex->value[0] == WEAPON_NATURAL )
                    continue;

                fprintf( fp, "<tr><td>\n" );
                fprintf( fp, "* %s", conv_hcolor( pObjIndex->short_descr ) );
                fprintf( fp, "</td><td>\n" );
                fprintf( fp, "%d", pObjIndex->vnum );
                fprintf( fp, "</td><td>\n" );
                sprintf( buf, "%d%s%d%s",
                         pObjIndex->value[0], "</td><td>\n",
                         pObjIndex->value[1], "</td></tr>\n" );
                fprintf( fp, buf );
                objz++;
            }
        }

        fprintf( fp, "</table><br><hr color=""#FFFFFF""><br>\n" );
        fprintf( fp, "-There are [ %d ] ammo clips on AvP-<br>\n", objy );
        fprintf( fp, "-There are [ %d ] weapons on AvP-<br><br>\n", objz );
        sprintf( buf, "<br>This file last updated at %s Eastern Time.\n", ( ( char* ) ctime( &current_time ) ) );
        fprintf( fp, buf );
        fprintf( fp, "</center></font>\n" );
        fprintf( fp, "</body>\n" );
        fprintf( fp, "</html>\n" );
        fclose( fp );
    }

    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }

    return;
}

/*
    Converts color tokens into HTML Color Codes.
    -Ghost
*/
char* conv_hcolor( char* msg )
{
    char wrkstr[2];
    conv_result[0] = 0;
    wrkstr[1] = 0;

    while ( *msg != 0 )
    {
        if ( *msg == '^' )
        {
            // Ignore the background tags
            msg++;
        }
        else if ( *msg == '&' )
        {
            msg++;

            /*
                The Following Lines allow the sub to replace the & with
                a color in the HTML Document. Removed for convinence.

            */
            switch ( *msg )
            {
                case 'x':
                    strcat( conv_result, "<font color=#000000>" );
                    break;  /* Black */

                case 'g':
                    strcat( conv_result, "<font color=#008000>" );
                    break;  /* Green */

                case 'b':
                    strcat( conv_result, "<font color=#000080>" );
                    break;  /* Dark Blue */

                case 'c':
                    strcat( conv_result, "<font color=#408080>" );
                    break;  /* Cyan */

                case 'z':
                    strcat( conv_result, "<font color=#669999>" );
                    break;  /* Dark Grey */

                case 'G':
                    strcat( conv_result, "<font color=#00FF00>" );
                    break;  /* Light Green */

                case 'B':
                    strcat( conv_result, "<font color=#0000FF>" );
                    break;  /* Blue */

                case 'C':
                    strcat( conv_result, "<font color=#00FFFF>" );
                    break;  /* Light Blue */

                case 'r':
                    strcat( conv_result, "<font color=#800000>" );
                    break;  /* Blood Red */

                case 'O':
                    strcat( conv_result, "<font color=#804000>" );
                    break;  /* Orange(Brown) */

                case 'p':
                    strcat( conv_result, "<font color=#FF00FF>" );
                    break;  /* Purple */

                case 'w':
                    strcat( conv_result, "<font color=#FFFFFF>" );
                    break;  /* Gray(White?) */

                case 'R':
                    strcat( conv_result, "<font color=#FF0000>" );
                    break;  /* Light Red */

                case 'Y':
                    strcat( conv_result, "<font color=#FFFF00>" );
                    break;  /* Yellow */

                case 'P':
                    strcat( conv_result, "<font color=#FF00FF>" );
                    break;  /* Pink */

                case 'W':
                    strcat( conv_result, "<font color=#C0C0C0>" );
                    break;  /* White(Grey?) */
            }
        }
        else
        {
            wrkstr[0] = *msg;
            strcat( conv_result, wrkstr );
        }

        msg++;
    }

    return conv_result;
}

/*
    Ranks by number of matches between two whole words.
    + Code by Senir, for the Similar Help snippet. Thanks, Senir!
*/
sh_int str_similarity( const char* astr, const char* bstr )
{
    sh_int matches = 0;

    if ( !astr || !bstr )
        return matches;

    for ( ; *astr; astr++ )
    {
        if ( LOWER( *astr ) == LOWER( *bstr ) )
            matches++;

        if ( ++bstr == '\0' )
            return matches;
    }

    return matches;
}

/*
    Ranks by number of matches until there's a non-matching character between two words.
    + Code by Senir, for the Similar Help snippet. Thanks, Senir!
*/
sh_int str_prefix_level( const char* astr, const char* bstr )
{
    sh_int matches = 0;

    if ( !astr || !bstr )
        return matches;

    for ( ; *astr; astr++ )
    {
        if ( LOWER( *astr ) == LOWER( *bstr ) )
            matches++;
        else
            return matches;

        if ( ++bstr == '\0' )
            return matches;
    }

    return matches;
}

/*
    Main function of Similar Helpfiles Snippet by Senir. It loops through all of the
    helpfiles, using the string matching function defined to find the closest matching
    helpfiles to the argument. It then checks for singles. Then, if matching helpfiles
    are found at all, it loops through and prints out the closest matching helpfiles.
    If its a single(there's only one), it opens the helpfile.
*/
void similar_help_files( CHAR_DATA* ch, char* argument )
{
    HELP_DATA* pHelp = NULL;
    char buf[MAX_STRING_LENGTH];
    char* extension;
    sh_int lvl = 0;
    bool single = FALSE;
    bool banner = FALSE;

    for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
    {
        buf[0] = '\0';
        extension = pHelp->keyword;

        if ( pHelp->level > get_trust( ch ) )
            continue;

        while ( extension[0] != '\0' )
        {
            extension = one_argument( extension, buf );

            if ( str_similarity( argument, buf ) > lvl )
            {
                lvl = str_similarity( argument, buf );
                single = TRUE;
            }
            else if ( str_similarity( argument, buf ) == lvl && lvl > 0 )
            {
                single = FALSE;
            }
        }
    }

    if ( lvl == 0 )
    {
        send_to_pager_color( " &RNo similar help files.\n\r", ch );
        return;
    }

    for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
    {
        buf[0] = '\0';
        extension = pHelp->keyword;

        while ( extension[0] != '\0' )
        {
            extension = one_argument( extension, buf );

            if ( str_similarity( argument, buf ) >= lvl
                    && pHelp->level <= get_trust( ch ) )
            {
                if ( single )
                {
                    send_to_pager_color( " &ROpening only similar helpfile.&G\n\r", ch );
                    // do_help( ch, buf );
                    send_to_pager_color( "&R", ch );
                    send_to_pager( pHelp->keyword, ch );
                    send_to_pager( "\n\r", ch );

                    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SOUND ) )
                        send_to_pager( "!!SOUND(help)", ch );

                    if ( pHelp->text[0] == '.' )
                        send_to_pager_color( pHelp->text + 1, ch );
                    else
                        send_to_pager_color( pHelp->text, ch );

                    return;
                }

                if ( !banner )
                {
                    send_to_pager_color( " Similar Help Files:\n\r", ch );
                    banner = TRUE;
                }

                pager_printf( ch, "&w&G   %s\n\r", pHelp->keyword );
                break;
            }
        }
    }

    return;
}


void send_skill_store( CHAR_DATA* ch, DESCRIPTOR_DATA* d )
{
    char tmp[MAX_STRING_LENGTH];
    char cA[MAX_STRING_LENGTH];
    char cB[MAX_STRING_LENGTH];
    int cnt = 0, col = 0;
    int clevel = 0, sn = 0;

    if ( !ch || !d )
        return;

    if ( ch->race == RACE_ALIEN )
    {
        strcpy( cA, "&B" );
        strcpy( cB, "&b" );
    }

    if ( ch->race == RACE_MARINE )
    {
        strcpy( cA, "&R" );
        strcpy( cB, "&r" );
    }

    if ( ch->race == RACE_PREDATOR )
    {
        strcpy( cA, "&G" );
        strcpy( cB, "&g" );
    }

    send_to_buffer( "\n\r\n\r", d );
    send_to_buffer( "&g[--------------{ &GAliens vs. Predator: Skill Store&g }--------------]\n\r", d );

    for ( sn = 0; ( sn < top_sn || clevel < 20 ) ; sn++ )
    {
        if ( sn >= top_sn )
        {
            sn = 0;
            clevel++;
            continue;
        }

        if ( !skill_table[sn]->name )
            break;

        if ( skill_table[sn]->min_level != clevel )
            continue;

        if ( skill_table[sn]->race < 0 || skill_table[sn]->race > 2 )
            continue;

        if ( skill_table[sn]->race != ch->race )
            continue;

        // if ( ch->pcdata->learned[sn] <= 0 ) continue;
        ++cnt;
        sprintf( tmp, "&C%-2.2d&z) %s%-20s &z[%s&z]",
                 cnt, ( clevel > ch->top_level ) ? "&z" : "&W", capitalize( skill_table[sn]->name ), ( char* )( drawbar( 3, ch->pcdata->learned[sn], 3, cA, cB ) ) );
        send_to_buffer( tmp, d );

        if ( ++col >= 2 )
        {
            col = 0;
            send_to_buffer( "\n\r", d );
        }
        else
        {
            send_to_buffer( "      ", d );
        }
    }

    if ( ++col >= 2 )
    {
        ++cnt;
        sprintf( tmp, "&C%-2.2d&z)                      &z[%s---&z]\n\r", cnt, cB );
        send_to_buffer( tmp, d );
    }

    send_to_buffer( "\n\r&z[&BPlease choose a skill number or hit enter to return&z]\n\r\n\r", d );
    return;
}

int get_store_skill( CHAR_DATA* ch, int num )
{
    int cnt = 0, clevel = 0, sn = 0;

    if ( !ch )
        return -1;

    for ( sn = 0; ( sn < top_sn || clevel < 20 ) ; sn++ )
    {
        if ( sn >= top_sn )
        {
            sn = 0;
            clevel++;
            continue;
        }

        if ( !skill_table[sn]->name )
            break;

        if ( skill_table[sn]->min_level != clevel )
            continue;

        if ( skill_table[sn]->race < 0 || skill_table[sn]->race > 2 )
            continue;

        if ( skill_table[sn]->race != ch->race )
            continue;

        // if ( ch->pcdata->learned[sn] <= 0 ) continue;
        ++cnt;

        if ( num == cnt )
            return sn;
    }

    return -1;
}


void map_area( AREA_DATA* area, CHAR_DATA* ch, char* filename )
{
    char map[100][100][40];
    ROOM_INDEX_DATA* room;
    int tx, ty, tz;
    int vnum;
    return;

    for ( tx = 0; tx < 100; tx++ )
        for ( ty = 0; ty < 100; ty++ )
            for ( tz = 0; tz < 40; tz++ )
                map[tx][ty][tz] = ' ';

    for ( vnum = area->low_r_vnum; vnum <= area->hi_r_vnum; vnum++ )
    {
        if ( ( room = get_room_index( vnum ) ) == NULL )
            continue;

        if ( xIS_SET( room->room_flags, ROOM_NOCOORD ) )
            continue;

        tx = room->x + 50;
        ty = room->y + 50;
        tz = room->z + 20;

        if ( tx > 0 && tx < 100 && ty > 0 && ty < 100 && tz > 0 && tz < 40 )
        {
            map[tx][ty][tz] = '*';
        }
    }

    return;
}
