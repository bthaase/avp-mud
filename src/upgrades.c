#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void do_attach( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    OBJ_DATA* tobj;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( ch->race == RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "&RSyntax: ATTACH (item) (weapon)\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_wear( ch, arg1 ) ) == NULL )
    {
        if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char( "&RYou don't seem to have that item.\n\r", ch );
            return;
        }
    }

    if ( obj->item_type != ITEM_ATTACHMENT )
    {
        send_to_char( "&RYou know, that ain't an attachment.\n\r", ch );
        return;
    }

    if ( ( tobj = get_obj_wear( ch, arg2 ) ) == NULL )
    {
        if ( ( tobj = get_obj_carry( ch, arg2 ) ) == NULL )
        {
            send_to_char( "&RYou can't seem to find that item.\n\r", ch );
            return;
        }
    }

    if ( tobj->attach != NULL )
    {
        send_to_char( "&RThat weapon seems to have an attachment already.\n\r", ch );
        return;
    }

    if ( tobj->item_type != ITEM_WEAPON )
    {
        send_to_char( "&RI'm pretty sure weapon attachments only go on weapons.\n\r", ch );
        return;
    }

    if ( obj->value[1] != tobj->pIndexData->vnum )
    {
        send_to_char( "&RThat attachment can't be used with that weapon.\n\r", ch );
        return;
    }

    separate_obj( obj );
    obj_from_char( obj );
    ch_printf( ch, "You upgrade %s with a %s.\n\r", tobj->short_descr, obj->short_descr );
    act( AT_PLAIN, "$n upgrades a $p.", ch, tobj, NULL, TO_ROOM );
    ch->carry_weight -= get_obj_weight( tobj );
    tobj->attach = obj;
    // ch->ap = 0;
    WAIT_STATE( ch, 8 );
    ch->carry_weight += get_obj_weight( tobj );

    /* Flat Modifiers */
    if ( obj->value[0] == 2 )
        tobj->value[2] += obj->value[4];

    return;
}

void do_detach( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( ch->race == RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "&RSyntax: DETACH (weapon)\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_wear( ch, arg1 ) ) == NULL )
    {
        if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char( "&RYou don't seem to have that item.\n\r", ch );
            return;
        }
    }

    if ( obj->item_type != ITEM_WEAPON )
    {
        send_to_char( "&RYou know, that ain't a weapon.\n\r", ch );
        return;
    }

    if ( !obj->attach )
    {
        send_to_char( "&RThat weapon doesn't have an attachment.\n\r", ch );
        return;
    }

    ch_printf( ch, "You detach a %s from %s.\n\r", obj->attach->short_descr, obj->short_descr );
    act( AT_PLAIN, "$n removes $p from a weapon.", ch, obj->attach, NULL, TO_ROOM );

    /* Flat Modifiers */
    if ( obj->attach->value[0] == 2 )
        obj->value[2] -= obj->attach->value[4];

    ch->carry_weight -= get_obj_weight( obj );
    obj_to_char( obj->attach, ch );
    obj->attach = NULL;
    // ch->ap = 0;
    WAIT_STATE( ch, 8 );
    ch->carry_weight += get_obj_weight( obj );
    return;
}

void send_attach_stats( CHAR_DATA* ch, OBJ_DATA* obj )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch == NULL )
        return;

    if ( obj == NULL )
        return;

    if ( IS_NPC( ch ) )
        return;

    if ( obj->value[0] == 0 )
        strcpy( buf, "Tactical Light" );

    if ( obj->value[0] == 1 )
        strcpy( buf, "Grenade Launcher" );

    if ( obj->value[0] == 2 )
        strcpy( buf, "Accuracy Modifer" );

    if ( obj->value[0] == 3 )
        strcpy( buf, "Weapon Silencer" );

    if ( obj->value[0] == 4 )
        strcpy( buf, "Damage Modifier" );

    if ( obj->value[0] == 5 )
        strcpy( buf, "Motion Tracker" );

    if ( obj->value[0] == 6 )
        strcpy( buf, "Vision Augment" );

    if ( obj->value[0] == 7 )
        strcpy( buf, "Ammo Modifier" );

    if ( obj->value[0] == 8 )
        strcpy( buf, "Fire Rate Modifier" );

    if ( obj->value[0] == 9 )
        strcpy( buf, "Speed Loader" );

    ch_printf( ch, "&z [&WAttachment Type: %s&z]\n\r", buf );

    if ( obj->value[0] == 0 )
    {
        ch_printf( ch, "&z [&WLight Time: &C%d of %d hours left&z]\n\r", obj->value[2], obj->value[3] );
    }
    else if ( obj->value[0] == 1 )
    {
        // Display Range
        ch_printf( ch, "&z [&WFiring Range: &C%d0 meters&z]\n\r", obj->value[3] );
        ch_printf( ch, "&z [&WFiring Reset: &C%d rounds&z]\n\r", obj->value[4] );
    }

    if ( obj->ammo )
    {
        ch_printf( ch, "&z Loaded with a %s. (&W%d/%d rounds&z)\n\r", obj->ammo->short_descr, obj->ammo->value[2], get_max_rounds( obj->ammo ) );
        strcpy( buf, "Unknown Type" );

        if ( obj->ammo->value[0] == 5 )
        {
            OBJ_INDEX_DATA* pOI;

            if ( ( pOI = get_obj_index( obj->ammo->value[1] ) ) != NULL )
                ch_printf( ch, "&z  [&WAmmo Damage: &R%d - %d&z]\n\r", pOI->value[0], pOI->value[1] );
        }
        else
        {
            if ( obj->ammo->value[0] >= 0 && obj->ammo->value[0] <= 4 )
                sprintf( buf, "%s", ris_flags[obj->ammo->value[0]] );

            ch_printf( ch, "&z  [&WAmmo Damage: &C%d &W/ %s&z]\n\r", obj->ammo->value[1], buf );
        }
    }

    return;
}

void send_attach_note( CHAR_DATA* ch, OBJ_DATA* obj )
{
    if ( ch == NULL )
        return;

    if ( obj == NULL )
        return;

    if ( IS_NPC( ch ) )
        return;

    if ( obj->value[0] == 0 ) /* Tactical Light */
    {
        ch_printf( ch, " &B(&CTime: &Y%s&B)", drawbar( 6, obj->value[2], obj->value[3], "&Y", "&Y" )  );

        if ( obj->value[4] == 0 )
        {
            ch_printf( ch, " &B(&COff&B)" );
        }
        else
        {
            ch_printf( ch, " &B(&COn&B)" );
        }
    }
    else if ( obj->value[0] == 1 ) /* Grenade Launcher */
    {
        if ( obj->ammo )
        {
            ch_printf( ch, " &B(&C%d rounds&B)", obj->ammo->value[2] );
        }
        else
        {
            ch_printf( ch, " &B(&wNot Loaded&B)" );
        }
    }
    else if ( obj->value[0] == 5 ) /* Motion Tracker */
    {
        if ( obj->value[2] == 0 )
        {
            ch_printf( ch, " &B(&COff&B)" );
        }
        else
        {
            ch_printf( ch, " &B(&COn&B)" );
        }
    }

    return;
}


int attach_light_modifier( CHAR_DATA* ch )
{
    OBJ_DATA* obj;
    int mod = 0;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
    {
        /*
            Flare patch
        */
        if ( obj->item_type == ITEM_FLARE && obj->value[2] != 0 )
            mod++;

        if ( obj->wear_loc == -1 )
            continue;

        mod += check_light_modifier( obj );
    }

    return mod;
}

int check_light_modifier( OBJ_DATA* obj )
{
    OBJ_DATA* tmp;

    if ( ( tmp = obj->attach ) != NULL )
    {
        if ( tmp->value[0] == 0 ) /* Attached Light */
        {
            if ( tmp->value[2] > 0 && tmp->value[4] == 1 )
                return 2;
        }
    }

    return 0;
}

bool use_attachment( CHAR_DATA* ch, OBJ_DATA* tmp )
{
    OBJ_DATA* obj;

    if ( ( obj = tmp->attach ) == NULL )
        return FALSE;

    switch ( obj->value[0] )
    {
        case 0: // Attachable Lights
            if ( obj->value[4] == 1 )
            {
                act( AT_ACTION, "&w&C$n reachs down and flips $s $p off.",  ch, obj, NULL, TO_ROOM );
                act( AT_ACTION, "&w&CYou reach down and flip a $p off.", ch, obj, NULL, TO_CHAR );
                obj->value[4] = 0;
                ch->in_room->light -= obj->count;
            }
            else if ( obj->value[2] <= 0 )
            {
                ch_printf( ch, "&RIts out of power, you'll have to wait a while.\n\r" );
            }
            else
            {
                act( AT_ACTION, "&w&C$n reachs down and flips $s $p on.",  ch, obj, NULL, TO_ROOM );
                act( AT_ACTION, "&w&CYou reach down and flip a $p on.", ch, obj, NULL, TO_CHAR );
                obj->value[4] = 1;
                ch->in_room->light += obj->count;
            }

            return TRUE;
            break;

        case 5: // Motion Tracker
            if ( obj->value[2] == 1 )
            {
                act( AT_ACTION, "&w&G$n reachs down and flips $s $p off.",  ch, obj, NULL, TO_ROOM );
                act( AT_ACTION, "&w&GYou reach down and flip a $p off.", ch, obj, NULL, TO_CHAR );
                obj->value[2] = 0;
            }
            else
            {
                act( AT_ACTION, "&w&G$n reachs down and flips $s $p on.",  ch, obj, NULL, TO_ROOM );
                act( AT_ACTION, "&w&GYou reach down and flip a $p on.", ch, obj, NULL, TO_CHAR );
                obj->value[2] = 1;
            }

            return TRUE;
            break;
    }

    return FALSE;
}

void recharge_attachment( CHAR_DATA* ch, OBJ_DATA* tmp )
{
    OBJ_DATA* obj;

    if ( ( obj = tmp->attach ) == NULL )
        return;

    switch ( obj->value[0] )
    {
        case 0: // Attachable Lights
            if ( obj->value[4] == 0 )
                obj->value[2] = UMIN( obj->value[3], obj->value[2] + 1 );

            if ( obj->value[4] == 1 )
            {
                if ( --obj->value[2] > 0 )
                    return;

                act( AT_ACTION, "&w&R$p has run out of power.", ch, obj, NULL, TO_ROOM );
                act( AT_ACTION, "&w&R$p has run out of power.", ch, obj, NULL, TO_CHAR );
                ch->in_room->light -= obj->count;
                obj->value[4] = 0;
            }

            break;
    }

    return;
}

int fire_attachment( CHAR_DATA* ch, OBJ_DATA* obj, int dir, int range )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA* to_room;
    EXIT_DATA* pexit;
    OBJ_DATA* tmp;
    int ammocode;
    int cnt;

    if ( ch == NULL )
        return 0;

    if ( obj == NULL )
        return 0;

    if ( obj->value[0] != 1 )
        return 0;

    to_room = ch->in_room;

    if ( range < 0 )
        range = obj->value[3];

    if ( range > obj->value[3] )
        range = obj->value[3];

    if ( !obj->ammo )
    {
        ch_printf( ch, "How about you load the weapon first?\n\r" );
        return 0;
    }

    if ( obj->ammo->value[0] != 5 )
        return 0;

    ammocode = obj->ammo->value[1];

    if ( get_obj_index( ammocode ) == NULL )
    {
        bug( "Error finding index at fire_weapon in fight.c" );
        return 0;
    }

    tmp = create_object( get_obj_index( ammocode ), 1 );

    if ( tmp == NULL )
    {
        bug( "Error invoking object at fire_weapon in fight.c" );
        return 0;
    }

    // Firing Message
    ch_printf( ch, "&RYou fire a %s to %s.\n\r", obj->short_descr, main_exit( dir ) );
    sprintf( buf, "$n fires a %s %s.", obj->short_descr, dir_name[dir] );
    act( AT_BLOOD, buf, ch, NULL, NULL, TO_ROOM );

    if ( IS_AFFECTED( ch, AFF_HIDE ) )
        xREMOVE_BIT( ch->affected_by, AFF_HIDE );

    weapon_echo( ch, obj );
    // unequip_char( ch, obj );
    // separate_obj( tmp );
    // obj_from_char( tmp );
    ch->ap = get_max_ap( ch ) - obj->value[4];
    obj->ammo->value[2]--;
    auto_eject( ch, obj );
    showammo_option( ch, obj ); // ShowAmmo Notification
    pexit = get_exit( ch->in_room, dir );

    for ( cnt = 1; ; cnt++ )
    {
        if ( !pexit )
        {
            sprintf( buf, "&r%s sails from %s and hits a wall.", tmp->short_descr, rev_exit( dir ) );
            echo_to_room( -1, to_room, buf );
            break;
        }

        if ( !pexit->to_room )
        {
            sprintf( buf, "&r%s sails from %s and hits a wall.", tmp->short_descr, rev_exit( dir ) );
            echo_to_room( -1, to_room, buf );
            break;
        }

        if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
        {
            sprintf( buf, "&r%s sails from %s and hits a door.", tmp->short_descr, rev_exit( dir ) );
            echo_to_room( -1, to_room, buf );
            break;
        }

        to_room = NULL;

        if ( pexit->distance > 1 )
            to_room = generate_exit( ch->in_room, &pexit );

        if ( to_room == NULL )
            to_room = pexit->to_room;

        if ( !to_room )
            break;

        if ( cnt >= range )
        {
            sprintf( buf, "&r%s sails from %s and lands on the floor.", tmp->short_descr, rev_exit( dir ) );
            echo_to_room( -1, to_room, buf );
            break;
        }
        else if ( ( pexit = get_exit( to_room, dir ) ) == NULL )
        {
            sprintf( buf, "&r%s sails from %s and hits a wall.", tmp->short_descr, rev_exit( dir ) );
            echo_to_room( -1, to_room, buf );
            break;
        }
        else
        {
            sprintf( buf, "&r%s sails through the room. (%s to %s).", tmp->short_descr, dir_name[flip_dir( dir )], dir_name[dir] );
            echo_to_room( -1, to_room, buf );
        }
    }

    if ( !to_room )
        to_room = ch->in_room;

    tmp = obj_to_room( tmp, to_room );

    // Arm this puppy.
    // tmp->value[4] = 1;

    if ( tmp->armed_by )
        STRFREE( tmp->armed_by );

    tmp->armed_by = STRALLOC( ch->name );

    if ( !xIS_SET( to_room->room_flags, ROOM_PROTOTYPE ) )
    {
        if ( to_room->area )
        {
            motion_ping( to_room->x, to_room->y, to_room->z, to_room->area, NULL );
        }
    }

    if ( tmp->item_type == ITEM_GRENADE )
    {
        explode( tmp );
    }
    else if ( tmp->item_type == ITEM_FLARE )
    {
        tmp->timer = tmp->value[0];
        tmp->value[2] = 1;
        tmp->cost = 0;
        to_room->light++;
    }

    return 1;
}

bool has_thermal( CHAR_DATA* ch )
{
    OBJ_DATA* obj;
    bool thermal = FALSE;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
    {
        if ( obj->wear_loc == -1 )
            continue;

        if ( !obj->attach )
            continue;

        if ( obj->attach->value[0] == 6 )
        {
            if ( obj->attach->value[3] != 0 )
                thermal = TRUE;
        }
    }

    return thermal;
}

