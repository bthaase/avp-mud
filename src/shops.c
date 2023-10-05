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
             Shop and repair shop module
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
    Local functions
*/

#define CD  CHAR_DATA
CD*     find_keeper args( ( CHAR_DATA* ch ) );
#undef CD

/*
    Shopping commands.
*/
CHAR_DATA* find_keeper( CHAR_DATA* ch )
{
    CHAR_DATA* keeper;
    SHOP_DATA* pShop;
    pShop = NULL;

    for ( keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room )
        if ( IS_NPC( keeper ) && ch->race == keeper->race && ( pShop = keeper->pIndexData->pShop ) != NULL )
            break;

    if ( !pShop )
        return NULL;

    return keeper;
}

void do_buy( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* keeper;
    OBJ_DATA* obj;
    int cost;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Buy what?\n\r", ch );
        return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    obj  = get_obj_carry( keeper, arg );

    if ( !obj && arg[0] == '#' )
    {
        int onum, oref;
        bool ofound = FALSE;
        onum = 0;
        oref = atoi( arg + 1 );

        for ( obj = keeper->last_carrying; obj; obj = obj->prev_content )
        {
            if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
                onum++;

            if ( onum == oref )
            {
                ofound = TRUE;
                break;
            }
            else if ( onum > oref )
                break;
        }

        if ( !ofound )
            obj = NULL;
    }

    if ( !obj )
    {
        act( AT_SAY, "$n says 'Sorry, I don't sell that'", keeper, NULL, ch, TO_ROOM );
        return;
    }

    cost = obj->cost;

    if ( cost <= 0 || !can_see_obj( ch, obj ) )
    {
        act( AT_SAY, "$n says 'Sorry, I don't sell that'", keeper, NULL, ch, TO_ROOM );
        return;
    }

    if ( ch->currexp < cost )
    {
        act( AT_TELL, "$n tells you 'You can't afford to buy $p.'", keeper, obj, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if ( xIS_SET( obj->extra_flags, ITEM_PROTOTYPE ) && get_trust( ch ) < LEVEL_IMMORTAL )
    {
        act( AT_TELL, "$n tells you 'This is a only a prototype!  I can't sell you that...'", keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        send_to_char( "&w&R[&rInformation&R] &zYou can't carry that many items.\n\r", ch );
        return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
        send_to_char( "&w&R[&rInformation&R] &zYou can't carry that much weight.\n\r", ch );
        return;
    }

    if ( !IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
        separate_obj( obj );

    act( AT_ACTION, "$n purchases a $p.", ch, obj, NULL, TO_ROOM );
    act( AT_ACTION, "You purchase a $p.", ch, obj, NULL, TO_CHAR );
    ch->currexp -= cost;

    if ( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
    {
        OBJ_DATA* buy_obj, *bag;
        buy_obj = create_object( obj->pIndexData, obj->level );
        xREMOVE_BIT( buy_obj->extra_flags, ITEM_INVENTORY );
        obj_to_char( buy_obj, ch );
    }
    else
    {
        obj_from_char( obj );
        obj_to_char( obj, ch );
    }

    return;
}


void do_list( CHAR_DATA* ch, char* argument )
{
    char rbar[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* keeper;
    OBJ_DATA* obj;
    int cost = 0;
    int oref = 0;
    bool found;
    bool warning = FALSE;
    one_argument( argument, arg );

    if ( ( keeper = find_keeper( ch ) ) == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    found = FALSE;

    for ( obj = keeper->last_carrying; obj; obj = obj->prev_content )
    {
        if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
        {
            oref++;

            if ( ( cost = obj->cost ) > 0 && ( arg[0] == '\0' || nifty_is_name( arg, obj->name ) ) )
            {
                if ( !found )
                {
                    found = TRUE;
                    send_to_char( "&z[&WPrice&z] {&Wref&z}   Recharge   Item\n\r", ch );
                }

                /* Execute a small 'compare' here */
                sprintf( rbar, "[        ]" );

                if ( obj->item_type == ITEM_WEAPON )
                {
                    strcpy( rbar, "[ &R" );

                    if ( obj->value[5] > 0 )
                    {
                        int i, swap = 0;

                        for ( i = 1; i <= 6; i++ )
                        {
                            if ( i <= obj->value[5] )
                            {
                                strcat( rbar, "|" );
                            }
                            else
                            {
                                if ( swap == 0 )
                                {
                                    swap++;
                                    strcat( rbar, "&r" );
                                }

                                strcat( rbar, "-" );
                            }
                        }
                    }
                    else
                    {
                        strcat( rbar, "------" );
                    }

                    strcat( rbar, " &z]" );
                }

                warning = FALSE;

                // Warning flag?
                if ( ch->pcdata )
                {
                    if ( obj->item_type == ITEM_MEDICOMP && ch->pcdata->learned[gsn_medical] < 3 )
                        warning = TRUE;

                    if ( obj->item_type == ITEM_SENTRY && ch->pcdata->learned[gsn_electronics] < 2 )
                        warning = TRUE;
                }

                if ( obj->item_type == ITEM_WEAPON && get_obj_weight( obj ) > get_curr_str( ch ) )
                    warning = TRUE;

                // Display the Item Listing
                ch_printf( ch, "&z[&Y%5d&z] {&C%3d&z}  &z%-10s  %s%s %s\n\r", cost, oref, rbar, warning ? "&R" : ( xIS_SET( obj->extra_flags, ITEM_RESPAWN ) ? "&B" : "&G" ), obj->short_descr, xIS_SET( obj->extra_flags, ITEM_RESPAWN ) ? "+" : "" );
            }
        }
    }

    if ( !found )
    {
        if ( arg[0] == '\0' )
            send_to_char( "You can't buy anything here.\n\r", ch );
        else
            send_to_char( "You can't buy that here.\n\r", ch );
    }

    return;
}


void do_sell( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* keeper;
    OBJ_DATA* obj;
    int cost;
    int cut;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Sell what?\n\r", ch );
        return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
        act( AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it!\n\r", ch );
        return;
    }

    if ( obj->timer > 0 )
    {
        act( AT_TELL, "$n tells you, '$p is depreciating in value too quickly...'", keeper, obj, ch, TO_VICT );
        return;
    }

    if ( obj->first_content )
    {
        act( AT_TELL, "$n tells you, 'You might want to empty $p first.'", keeper, obj, ch, TO_VICT );
        return;
    }

    cut = 75;  /* Default 50% of base value */

    if ( !xIS_SET( obj->extra_flags, ITEM_RESPAWN ) )
        cut = 25;

    if ( ch->race == RACE_MARINE )
    {
        if ( xIS_SET( obj->extra_flags, ITEM_ALIEN ) )
            cut = 125;

        if ( xIS_SET( obj->extra_flags, ITEM_PREDATOR ) )
            cut = 100;
    }
    else if ( ch->race == RACE_ALIEN )
    {
        if ( xIS_SET( obj->extra_flags, ITEM_MARINE ) )
            cut = 50;

        if ( xIS_SET( obj->extra_flags, ITEM_PREDATOR ) )
            cut = 75;
    }
    else if ( ch->race == RACE_PREDATOR )
    {
        if ( xIS_SET( obj->extra_flags, ITEM_ALIEN ) )
            cut = 100;

        if ( xIS_SET( obj->extra_flags, ITEM_MARINE ) )
            cut = 75;
    }

    cost = ( obj->cost * cut ) / 100;  /* % cut of base value */

    if ( cost <= 0 )
    {
        act( AT_ACTION, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
        return;
    }

    separate_obj( obj );
    act( AT_ACTION, "$n sells $p.", ch, obj, NULL, TO_ROOM );
    sprintf( buf, "You sell $p for %d experience.", cost );
    act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
    ch->currexp += cost;
    ch->maxexp += cost;
    extract_obj( obj );
    return;
}

void do_value( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* keeper;
    OBJ_DATA* obj;
    int cost;
    int cut;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Value what?\n\r", ch );
        return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
        act( AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it!\n\r", ch );
        return;
    }

    cut = 75;  /* Default 75% of base value */

    if ( ch->race == RACE_MARINE )
    {
        if ( xIS_SET( obj->extra_flags, ITEM_ALIEN ) )
            cut = 125;

        if ( xIS_SET( obj->extra_flags, ITEM_PREDATOR ) )
            cut = 100;
    }
    else if ( ch->race == RACE_ALIEN )
    {
        if ( xIS_SET( obj->extra_flags, ITEM_MARINE ) )
            cut = 50;

        if ( xIS_SET( obj->extra_flags, ITEM_PREDATOR ) )
            cut = 75;
    }
    else if ( ch->race == RACE_PREDATOR )
    {
        if ( xIS_SET( obj->extra_flags, ITEM_ALIEN ) )
            cut = 100;

        if ( xIS_SET( obj->extra_flags, ITEM_MARINE ) )
            cut = 75;
    }

    cost = ( obj->cost * cut ) / 100;  /* 75% of base value */
    sprintf( buf, "$n tells you 'I'll give you %d credits for $p.'", cost );
    act( AT_TELL, buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;
    return;
}

/* ------------------ Shop Building and Editing Section ----------------- */

void do_makeshop( CHAR_DATA* ch, char* argument )
{
    SHOP_DATA* shop;
    int vnum;
    MOB_INDEX_DATA* mob;

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage: makeshop <mobvnum>\n\r", ch );
        return;
    }

    vnum = atoi( argument );

    if ( ( mob = get_mob_index( vnum ) ) == NULL )
    {
        send_to_char( "Mobile not found.\n\r", ch );
        return;
    }

    if ( !can_medit( ch, mob ) && !IS_NPC( ch ) )
        return;

    if ( mob->pShop )
    {
        send_to_char( "This mobile already has a shop.\n\r", ch );
        return;
    }

    CREATE( shop, SHOP_DATA, 1 );
    LINK( shop, first_shop, last_shop, next, prev );
    shop->keeper    = vnum;
    mob->pShop      = shop;
    send_to_char( "Done.\n\r", ch );
    return;
}


void do_shopset( CHAR_DATA* ch, char* argument )
{
    SHOP_DATA* shop;
    MOB_INDEX_DATA* mob, *mob2;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int vnum;
    int value;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Usage: shopset <mob vnum> <field> value\n\r", ch );
        send_to_char( "\n\rField being one of:\n\r", ch );
        send_to_char( "  keeper\n\r", ch );
        return;
    }

    vnum = atoi( arg1 );

    if ( ( mob = get_mob_index( vnum ) ) == NULL )
    {
        send_to_char( "Mobile not found.\n\r", ch );
        return;
    }

    if ( !can_medit( ch, mob ) )
        return;

    if ( !mob->pShop )
    {
        send_to_char( "This mobile doesn't keep a shop.\n\r", ch );
        return;
    }

    shop = mob->pShop;
    value = atoi( argument );

    if ( !str_cmp( arg2, "keeper" ) )
    {
        if ( ( mob2 = get_mob_index( vnum ) ) == NULL )
        {
            send_to_char( "Mobile not found.\n\r", ch );
            return;
        }

        if ( !can_medit( ch, mob ) )
            return;

        if ( mob2->pShop )
        {
            send_to_char( "That mobile already has a shop.\n\r", ch );
            return;
        }

        mob->pShop  = NULL;
        mob2->pShop = shop;
        shop->keeper = value;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    do_shopset( ch, "" );
    return;
}


void do_shopstat( CHAR_DATA* ch, char* argument )
{
    SHOP_DATA* shop;
    MOB_INDEX_DATA* mob;
    int vnum;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Usage: shopstat <keeper vnum>\n\r", ch );
        return;
    }

    vnum = atoi( argument );

    if ( ( mob = get_mob_index( vnum ) ) == NULL )
    {
        send_to_char( "Mobile not found.\n\r", ch );
        return;
    }

    if ( !mob->pShop )
    {
        send_to_char( "This mobile doesn't keep a shop.\n\r", ch );
        return;
    }

    shop = mob->pShop;
    ch_printf( ch, "Keeper: %d  %s\n\r", shop->keeper, mob->short_descr );
    return;
}


void do_shops( CHAR_DATA* ch, char* argument )
{
    SHOP_DATA* shop;

    if ( !first_shop )
    {
        send_to_char( "There are no shops.\n\r", ch );
        return;
    }

    set_char_color( AT_NOTE, ch );

    for ( shop = first_shop; shop; shop = shop->next )
        ch_printf( ch, "Keeper: %5d\n\r", shop->keeper );

    return;
}
