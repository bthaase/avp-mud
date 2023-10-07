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
               Player communication module
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "mqtt.h"

#define BFS_ERROR      -1
#define BFS_ALREADY_THERE  -2
#define BFS_NO_PATH    -3
#define BFS_MARK            1

/*
    Externals
*/
void send_obj_page_to_char( CHAR_DATA* ch, OBJ_INDEX_DATA* idx, char page );
void send_room_page_to_char( CHAR_DATA* ch, ROOM_INDEX_DATA* idx, char page );
void send_page_to_char( CHAR_DATA* ch, MOB_INDEX_DATA* idx, char page );
void send_control_page_to_char( CHAR_DATA* ch, char page );

// extern bool MOBtrigger;

/*
    Local functions.
*/
void    talk_channel    args( ( CHAR_DATA* ch, char* argument, int channel, const char* verb ) );
void    random_quote    args( ( CHAR_DATA* ch ) );
char*   scramble        args( ( const char* argument, int modifier ) );
void    yell_radius     args( ( CHAR_DATA* ch, char* buf ) );
void    yell_radius_1   args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room, char* buf, int range, int dir ) );
void    yell_radius_2   args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room, int range ) );

int const lang_array[] = { LANG_MARINE, LANG_ALIEN, LANG_PREDATOR, LANG_UNKNOWN };

char* const lang_names[] = { "marine", "alien", "predator", "" };

/*typedef enum
{
    CHANNEL_CHAT, CHANNEL_QUEST, CHANNEL_IMMTALK,
    CHANNEL_MUSIC, CHANNEL_ASK, CHANNEL_SHOUT, CHANNEL_YELL,
    CHANNEL_MONITOR, CHANNEL_LOG, CHANNEL_104,
    CHANNEL_BUILD, CHANNEL_105, CHANNEL_AVTALK, CHANNEL_PRAY,
    CHANNEL_COUNCIL, CHANNEL_C17, CHANNEL_COMM, CHANNEL_TELLS,
    CHANNEL_C20, CHANNEL_NEWBIE, CHANNEL_WARTALK, CHANNEL_OOC,
    CANNNEL_C24, CHANNEL_C25, CHANNEL_C26, CHANNEL_WHISPER,
    CHANNEL_INFO, MAX_CHANNEL
} channel_type;*/

const char* chan_names[] = {
    "chat", "quest", "immtalk", "music", "ask", "shout", "yell", "monitor", "log", "104",
    "build", "105", "avtalk", "pray", "council", "c17", "comm", "tells", "c20", "newbie",
    "wartalk", "ooc", "c24", "c25", "c26", "whisper", "info", "max"
};

void sound_to_room( ROOM_INDEX_DATA* room, char* argument )
{
    CHAR_DATA* vic;

    if ( room == NULL )
        return;

    for ( vic = room->first_person; vic; vic = vic->next_in_room )
        if ( !IS_NPC( vic ) && xIS_SET( vic->act, PLR_SOUND ) )
            send_to_char( argument, vic );
}

/*
    BEEP Command
    Allows a player to 'beep' another player with a sound and message.
    - Cannot be used in silent rooms
    - Cannot use with 'silenced' or 'no tell' flags
*/
void do_beep( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );
    strcpy( buf, argument );
    xREMOVE_BIT( ch->deaf, CHANNEL_TELLS );

    if ( !IS_NPC( ch ) && ( xIS_SET( ch->act, PLR_SILENCE ) || xIS_SET( ch->act, PLR_NO_TELL ) ) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "Beep who?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL || ( IS_NPC( victim ) && victim->in_room != ch->in_room )
            || ( !NOT_AUTHED( ch ) && NOT_AUTHED( victim ) && !IS_IMMORTAL( ch ) ) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( NOT_AUTHED( ch ) && !NOT_AUTHED( victim ) && !IS_IMMORTAL( victim ) )
    {
        send_to_char( "They can't hear you because you are not authorized.\n\r", ch );
        return;
    }

    if ( is_ignored( victim, ch->name ) )
    {
        send_to_char( "&w&CThat player can't hear you right now.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched )
            && ( get_trust( ch ) > LEVEL_AVATAR ) )
    {
        send_to_char( "That player is switched.\n\r", ch );
        return;
    }
    else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
        send_to_char( "That player is link-dead.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->deaf, CHANNEL_TELLS )
            && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
        act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( !IS_NPC ( victim ) && ( xIS_SET( victim->act, PLR_SILENCE ) ) )
    {
        send_to_char( "That player is silenced.  They will receive your message but can not respond.\n\r", ch );
    }

    if ( victim->desc       /* make sure desc exists first  -Thoric */
            &&   victim->desc->connected == CON_EDITING
            &&   get_trust( ch ) < LEVEL_GOD )
    {
        act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
        return;
    }

    ch_printf( ch, "&R(OOC) You beep %s: %s\n\r\a", victim->name, stripclr( buf ) );
    send_to_char( "\a", victim );
    sprintf( buf, "&R(OOC) %s beeps: '%s'", ch->name, stripclr( argument ) );
    act( AT_RED, buf, ch, argument, victim, TO_VICT );
    return;
}

/* Text scrambler -- Altrag */
char* scramble( const char* argument, int modifier )
{
    static char arg[MAX_INPUT_LENGTH];
    sh_int position;
    sh_int conversion = 0;
    modifier %= number_range( 80, 300 ); /* Bitvectors get way too large #s */

    for ( position = 0; position < MAX_INPUT_LENGTH; position++ )
    {
        if ( argument[position] == '\0' )
        {
            arg[position] = '\0';
            return arg;
        }
        else if ( argument[position] >= 'A' && argument[position] <= 'Z' )
        {
            conversion = -conversion + position - modifier + argument[position] - 'A';
            conversion = number_range( conversion - 5, conversion + 5 );

            while ( conversion > 25 )
                conversion -= 26;

            while ( conversion < 0 )
                conversion += 26;

            arg[position] = conversion + 'A';
        }
        else if ( argument[position] >= 'a' && argument[position] <= 'z' )
        {
            conversion = -conversion + position - modifier + argument[position] - 'a';
            conversion = number_range( conversion - 5, conversion + 5 );

            while ( conversion > 25 )
                conversion -= 26;

            while ( conversion < 0 )
                conversion += 26;

            arg[position] = conversion + 'a';
        }
        else if ( argument[position] >= '0' && argument[position] <= '9' )
        {
            conversion = -conversion + position - modifier + argument[position] - '0';
            conversion = number_range( conversion - 2, conversion + 2 );

            while ( conversion > 9 )
                conversion -= 10;

            while ( conversion < 0 )
                conversion += 10;

            arg[position] = conversion + '0';
        }
        else
            arg[position] = argument[position];
    }

    arg[position] = '\0';
    return arg;
}

/*
    Static Disruption for Marine Radio
*/
char* disrupt( const char* argument, int chance )
{
    static char arg[MAX_INPUT_LENGTH];
    sh_int position;
    int black = 0;

    for ( position = 0; position < MAX_INPUT_LENGTH; position++ )
    {
        if ( argument[position] == '\0' )
        {
            arg[position] = '\0';
            return arg;
        }
        else if (
            ( argument[position] >= '0' && argument[position] <= '9' ) ||
            ( argument[position] >= 'a' && argument[position] <= 'z' ) ||
            ( argument[position] >= 'A' && argument[position] <= 'Z' ) )
        {
            if ( black < 3 && number_range( 1, 50 ) <= chance )
            {
                black += 1;
                arg[position] = '-';
            }
            else
            {
                black = 0;
                arg[position] = argument[position];
            }
        }
        else
            arg[position] = argument[position];
    }

    arg[position] = '\0';
    return arg;
}

/*
    Profanity Filter  -Ghost [with help from LoKust]
*/
void profanity_filter( char* arg, char* out )
{
    char* nbuf, *tbuf, *p;
    bool change = TRUE;
    unsigned int x, v;
    nbuf = malloc( strlen( arg ) + 1 );
    tbuf = malloc( strlen( arg ) + 1 );
    strcpy( nbuf, strlower( arg ) );
    strcpy( tbuf, arg );

    while ( change )
    {
        change = FALSE;

        for ( x = 0; strcmp( ignore_table[x], "$" ); x++ )
        {
            if ( ( p = strstr( nbuf, ignore_table[x] ) ) )
            {
                change = TRUE;

                for ( v = 0; v < strlen( ignore_table[x] ); v++ )
                    *( p + v ) = '.';
            }
        }
    }

    change = TRUE;

    while ( change )
    {
        change = FALSE;

        for ( x = 0; strcmp( curse_table[x], "$" ); x++ )
        {
            if ( ( p = strstr( nbuf, curse_table[x] ) ) )
            {
                change = TRUE;

                for ( v = 0; v < strlen( curse_table[x] ); v++ )
                    *( p + v ) = '*';
            }
        }
    }

    /*
        Clone *'s over to the real string
    */
    for ( x = 0; x < strlen( nbuf ); x++ )
    {
        if ( nbuf[x] == '*' )
            tbuf[x] = '*';
    }

    strcpy( out, tbuf );
    free( nbuf );
    free( tbuf );
    return;
}

bool profanity_check( char* arg )
{
    char* nbuf, *p;
    bool change = TRUE;
    unsigned int x, v;
    nbuf = malloc( strlen( arg ) + 1 );
    strcpy( nbuf, strlower( arg ) );

    while ( change )
    {
        change = FALSE;

        for ( x = 0; strcmp( ignore_table[x], "$" ); x++ )
        {
            if ( ( p = strstr( nbuf, ignore_table[x] ) ) )
            {
                change = TRUE;

                for ( v = 0; v < strlen( ignore_table[x] ); v++ )
                    *( p + v ) = '.';
            }
        }
    }

    for ( x = 0; strcmp( curse_table[x], "$" ); x++ )
    {
        if ( ( p = strstr( nbuf, curse_table[x] ) ) )
        {
            free( nbuf );
            return TRUE;
        }
    }

    free( nbuf );
    return FALSE;
}

bool block_profane( CHAR_DATA* ch )
{
    if ( ch == NULL )
        return FALSE;

    if ( !ch->desc )
        return FALSE;

    if ( !ch->pcdata )
        return FALSE;

    if ( ch->desc->connected != CON_PLAYING )
        return FALSE;

    if ( xIS_SET( ch->act, PLR_CENSOR ) )
        return TRUE;

    return FALSE;
}

/*
    Returns the first radio on a player
*/
OBJ_DATA* get_radio( CHAR_DATA* ch )
{
    OBJ_DATA* obj;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->item_type == ITEM_RADIO )
            return obj;

    return NULL;
}

/*
    Checks the player for a motion detector
*/
bool has_motion_tracker( CHAR_DATA* ch )
{
    OBJ_DATA* obj;

    /*
        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->item_type == ITEM_MOTION )
          return TRUE;
    */

    if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) != NULL )
    {
        if ( obj->item_type == ITEM_MOTION )
            return TRUE;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
    {
        if ( obj->attach )
        {
            if ( obj->attach->value[0] == 5 && obj->attach->value[2] == 1 )
                return TRUE;
        }
    }

    if ( ( obj = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL )
    {
        if ( obj->attach )
        {
            if ( obj->attach->value[0] == 5 && obj->attach->value[2] == 1 )
                return TRUE;
        }
    }

    return FALSE;
}

/*
    Checks the player for a radio
*/
bool has_radio( CHAR_DATA* ch )
{
    OBJ_DATA* obj;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->item_type == ITEM_RADIO )
            return TRUE;

    return FALSE;
}

/*
    Generic channel function.
*/
void talk_channel( CHAR_DATA* ch, char* argument, int channel, const char* verb )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA* d;
    int position;

    if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        if ( ch->master )
            send_to_char( "I don't think so...\n\r", ch->master );

        return;
    }

    if ( argument[0] == '\0' )
    {
        sprintf( buf, "%s what?\n\r", verb );
        buf[0] = UPPER( buf[0] );
        send_to_char( buf, ch );    /* where'd this line go? */
        return;
    }

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SILENCE ) )
    {
        ch_printf( ch, "You can't %s.\n\r", verb );
        return;
    }

    xREMOVE_BIT( ch->deaf, channel );

    switch ( channel )
    {
        case CHANNEL_YELL:
        case CHANNEL_SHOUT:
            set_char_color( AT_GOSSIP, ch );
            ch_printf( ch, "You %s, '%s'\n\r", verb, argument );
            sprintf( buf, "Someone %ss, '$t'", verb );
            break;

        case CHANNEL_ASK:
            set_char_color( AT_OOC, ch );
            ch_printf( ch, "(OOC) You %s, '%s'\n\r", verb, argument );
            sprintf( buf, "(OOC) %s %ss, '$t'", ch->name, verb );
            break;

        case CHANNEL_NEWBIE:
            set_char_color( AT_OOC, ch );
            ch_printf( ch, "(NEWBIE) %s: %s\n\r", ch->name, argument );
            sprintf( buf, "(NEWBIE) %s: $t", ch->name );
            break;

        case CHANNEL_OOC:
            set_char_color( AT_OOC, ch );
            sprintf( buf, "(OOC) %s: $t", ch->name );
            position        = ch->position;
            ch->position    = POS_STANDING;
            act( AT_OOC, buf, ch, argument, NULL, TO_CHAR );
            ch->position    = position;
            break;

        case CHANNEL_WARTALK:
            set_char_color( AT_WARTALK, ch );
            ch_printf( ch, "You %s '%s'\n\r", verb, argument );
            sprintf( buf, "$n %ss '$t'", verb );
            break;

        case CHANNEL_AVTALK:
        case CHANNEL_IMMTALK:
            if ( channel == CHANNEL_AVTALK )
                sprintf( buf, "$n: $t" );
            else if ( channel == CHANNEL_IMMTALK )
                sprintf( buf, "$n> $t" );

            position    = ch->position;
            ch->position    = POS_STANDING;
            act( channel == CHANNEL_AVTALK ? AT_AVATAR : AT_IMMORT, buf, ch, argument, NULL, TO_CHAR );
            ch->position    = position;
            break;

        case CHANNEL_CHAT:
            ch_printf( ch, "&zPublic Chat Network &B[&W%s&B]&C: %s\n\r", ch->name, argument );
            sprintf( buf, "&zPublic Chat Network &B[&W$n&B]&C: $t" );
            break;

        default:
            set_char_color( AT_GOSSIP, ch );
            ch_printf( ch, "You %s over the public network, '%s'\n\r", verb, argument );
            sprintf( buf, "$n %ss over the public network, '$t'",     verb );
            break;
    }

    if ( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
        sprintf( buf2, "[%d] %s: %s (%s)", ch->in_room->vnum, IS_NPC( ch ) ? ch->short_descr : ch->name, argument, verb );
        append_to_file( LOG_FILE, buf2 );
    }

    for ( d = first_descriptor; d; d = d->next )
    {
        CHAR_DATA* och;
        CHAR_DATA* vch;
        och = d->original ? d->original : d->character;
        vch = d->character;

        if ( d->connected == CON_PLAYING && vch != ch && !xIS_SET( och->deaf, channel ) )
        {
            char* sbuf = argument;

            if ( channel == CHANNEL_IMMTALK && !IS_IMMORTAL( och ) )
                continue;

            if ( channel == CHANNEL_WARTALK && NOT_AUTHED( och ) )
                continue;

            if ( channel == CHANNEL_AVTALK && !IS_HERO( och ) )
                continue;

            if ( is_ignored( och, ch->name ) )
            {
                if ( channel == CHANNEL_OOC )
                    continue;

                if ( channel == CHANNEL_CHAT )
                    continue;
            }

            if ( channel == CHANNEL_YELL || channel == CHANNEL_SHOUT )
            {
                /*
                    Fixed shout... Whichever dork turned it off. -Ghost
                */
                if ( ch->in_room->area != och->in_room->area )
                    continue;
            }

            position        = vch->position;

            if ( channel != CHANNEL_SHOUT && channel != CHANNEL_YELL )
                vch->position   = POS_STANDING;

            if ( ( !knows_language( vch, ch->speaking, ch ) && ( !IS_NPC( ch ) || !xIS_EMPTY( ch->speaking ) ) )
                    && ( channel != CHANNEL_NEWBIE &&  channel != CHANNEL_OOC && channel != CHANNEL_ASK && channel != CHANNEL_AVTALK )  )
                sbuf = scramble( argument, number_range( 1000, 32000 ) );

            MOBtrigger = FALSE;

            if ( channel == CHANNEL_IMMTALK || channel == CHANNEL_AVTALK )
                act( channel == CHANNEL_AVTALK ? AT_AVATAR : AT_IMMORT, buf, ch, sbuf, vch, TO_VICT );
            else if ( channel == CHANNEL_WARTALK )
                act( AT_WARTALK, buf, ch, sbuf, vch, TO_VICT );
            else if ( channel == CHANNEL_OOC || channel == CHANNEL_NEWBIE || channel == CHANNEL_ASK )
                act( AT_OOC, buf, ch, sbuf, vch, TO_VICT );
            else
                act( AT_GOSSIP, buf, ch, sbuf, vch, TO_VICT );

            vch->position   = position;
        }
    }

    snprintf(buf, MAX_STRING_LENGTH, "%s,%s", ch->name, argument);
    snprintf(buf2, MAX_STRING_LENGTH, "out/channel/%s", chan_names[channel]);
    mqtt_publish(buf2, buf);

    /* too much system degradation with 300+ players not to charge 'em a bit */
    /* 600 players now, but waitstate on clantalk is bad for pkillers */
    if ( ( !IS_IMMORTAL( ch ) ) && ( channel != CHANNEL_WARTALK ) )
        WAIT_STATE( ch, 3 );

    return;
}

void to_channel( const char* argument, int channel, const char* verb, sh_int level )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA* d;

    if ( !first_descriptor || argument[0] == '\0' )
        return;

    sprintf( buf, "%s: %s\r\n", verb, argument );

    for ( d = first_descriptor; d; d = d->next )
    {
        CHAR_DATA* och;
        CHAR_DATA* vch;
        och = d->original ? d->original : d->character;
        vch = d->character;

        if ( !och || !vch )
            continue;

        if ( !IS_IMMORTAL( vch )
                || ( get_trust( vch ) < sysdata.build_level && channel == CHANNEL_BUILD )
                || ( get_trust( vch ) < sysdata.log_level
                     && ( channel == CHANNEL_LOG || channel == CHANNEL_COMM ) ) )
            continue;

        if ( d->connected == CON_PLAYING
                &&   !xIS_SET( och->deaf, channel )
                &&   get_trust( vch ) >= level )
        {
            /* set_char_color( AT_LOG, vch ); */
            send_to_char( "&p", vch );
            send_to_char( buf, vch );
        }
    }
    char buf2[1024];
    snprintf(buf2, 1024, "out/channel/%s", verb);

    mqtt_publish( buf2, argument );

    return;
}


void do_chat( CHAR_DATA* ch, char* argument )
{
    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    talk_channel( ch, argument, CHANNEL_CHAT, "chat" );
    return;
}

void do_radio( CHAR_DATA* ch, char* argument )
{
    bool silent = FALSE;

    if ( ch->hit < 0 )
        silent = TRUE;

    if ( ch->race != RACE_MARINE )
    {
        if ( !silent )
            do_nothing( ch, "" );

        return;
    }

    if ( xIS_SET( ch->act, PLR_SILENCE ) )
    {
        if ( !silent )
            send_to_char( "&RYou can't radio while silenced, stupid.\n\r", ch );

        return;
    }

    if ( is_spectator( ch ) )
    {
        if ( !silent )
            send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );

        return;
    }

    if ( !has_radio( ch ) )
    {
        if ( !silent )
            ch_printf( ch, "&w&RYou need a radio to send and recieve radio message.\n\r" );

        return;
    }

    radio_broadcast( ch, argument );
    return;
}

void do_ooc( CHAR_DATA* ch, char* argument )
{
    if ( !sysdata.ALLOW_OOC && !xIS_SET( ch->act, PLR_NCOUNCIL ) )
    {
        send_to_char( "&RThe OOC Channel is closed at this time. Sorry.\n\r", ch );

        if ( IS_IMMORTAL( ch ) )
            send_to_char( "&R[Immortal - Use SETOOC to toggle OOC]\n\r", ch );

        return;
    }

    if ( !IS_NPC( ch ) && xIS_SET( ch->deaf, CHANNEL_OOC ) )
    {
        send_to_char( "&RYou must engage your OOC channel to use it. CHANNEL +OOC.\n\r", ch );
        return;
    }

    if ( !IS_NPC( ch ) && !IS_IMMORTAL( ch ) && !xIS_SET( ch->act, PLR_NCOUNCIL ) && argument[0] != '\0' )
    {
        if ( ch->pcdata->oocbreak )
        {
            send_to_char( "&RYou can't use the OOC just yet.\n\r", ch );
            return;
        }

        if ( ch->pcdata->ooclimit <= 0 )
        {
            ch->pcdata->oocbreak = TRUE;
            send_to_char( "&ROOC Limit Break reached - OOC disabled for a short time.\n\r", ch );
            return;
        }

        ch->pcdata->ooclimit--;
    }

    talk_channel( ch, argument, CHANNEL_OOC, "ooc" );
    return;
}

void do_newbiechat( CHAR_DATA* ch, char* argument )
{
    if ( ch->top_level > 5 && !IS_IMMORTAL( ch ) && !xIS_SET( ch->act, PLR_NCOUNCIL ) )
    {
        send_to_char( "Aren't you a little old for the newbie channel?\n\r", ch );
        return;
    }

    talk_channel( ch, argument, CHANNEL_NEWBIE, "newbiechat" );
    return;
}

void do_music( CHAR_DATA* ch, char* argument )
{
    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    talk_channel( ch, argument, CHANNEL_MUSIC, "sing" );
    return;
}


void do_ask( CHAR_DATA* ch, char* argument )
{
    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    talk_channel( ch, argument, CHANNEL_ASK, "ask" );
    return;
}



void do_answer( CHAR_DATA* ch, char* argument )
{
    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    talk_channel( ch, argument, CHANNEL_ASK, "answer" );
    return;
}



void do_shout( CHAR_DATA* ch, char* argument )
{
    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    talk_channel( ch, argument, CHANNEL_SHOUT, "shout" );
    WAIT_STATE( ch, 12 );
    return;
}



void do_yell( CHAR_DATA* ch, char* argument )
{
    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Yell what?\n\r", ch );
        return;
    }

    ch_printf( ch, "&CYou yell, '%s'\n\r", argument );
    yell_radius( ch, argument );
    return;
}

void yell_radius( CHAR_DATA* ch, char* buf )
{
    int range = 0;

    if ( !ch->in_room )
        return;

    /*
        Ambience Factors:
        Low:    Range = 6 rooms
        Medium: Range = 4 rooms
        High:   Range = 2 rooms
    */
    if ( ch->in_room->area )
    {
        if ( ch->in_room->area->ambience == 0 )
            range = 4;

        if ( ch->in_room->area->ambience == 1 )
            range = 6;

        if ( ch->in_room->area->ambience == 2 )
            range = 4;

        if ( ch->in_room->area->ambience == 3 )
            range = 2;
    }
    else
    {
        range = 4;  // Default to Medium ambience
    }

    yell_radius_2( ch, ch->in_room, range );
    yell_radius_1( ch, ch->in_room, buf, range, -1 );
    yell_radius_2( ch, ch->in_room, range );
    return;
}

void yell_radius_1( CHAR_DATA* ch, ROOM_INDEX_DATA* room, char* buf, int range, int dir )
{
    char* dtxt;
    CHAR_DATA* rch;
    CHAR_DATA* rnext;
    bool same = FALSE;

    if ( xIS_SET( room->room_flags, BFS_MARK ) )
        return;

    xSET_BIT( room->room_flags, BFS_MARK );

    if ( ch->in_room == room )
    {
        same = TRUE;
    }
    else
    {
        switch ( dir )
        {
            default:
                dtxt = "somewhere";
                break;

            case 0:
                dtxt = "the south";
                break;

            case 1:
                dtxt = "the west";
                break;

            case 2:
                dtxt = "the north";
                break;

            case 3:
                dtxt = "the east";
                break;

            case 4:
                dtxt = "below";
                break;

            case 5:
                dtxt = "above";
                break;

            case 6:
                dtxt = "the south-west";
                break;

            case 7:
                dtxt = "the south-east";
                break;

            case 8:
                dtxt = "the north-west";
                break;

            case 9:
                dtxt = "the north-east";
                break;
        }
    }

    for ( rch = room->first_person ; rch ;  rch = rnext )
    {
        rnext = rch->next_in_room;

        if ( IS_NPC( rch ) || rch == ch )
            continue;

        if ( same )
            ch_printf( rch, "&C%s yells, '%s'\n\r", PERS( ch, rch ), buf );

        if ( !same )
            ch_printf( rch, "&CSomeone yells from %s, '%s'\n\r", dtxt, buf );
    }

    /* Branch out */
    {
        EXIT_DATA* pexit;

        for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        {
            if ( pexit->to_room && pexit->to_room != room )
            {
                if ( range > 0 )
                {
                    yell_radius_1( ch, pexit->to_room, buf, range - 1, pexit->vdir );
                }
                else
                {
                    if ( !xIS_SET( pexit->to_room->room_flags, BFS_MARK ) )
                        echo_to_room( AT_CYAN, pexit->to_room, "You hear someone yelling nearby." );
                }
            }
        }
    }
    return;
}

void yell_radius_2( CHAR_DATA* ch, ROOM_INDEX_DATA* room, int range )
{
    if ( !xIS_SET( room->room_flags, BFS_MARK ) )
        return;

    xREMOVE_BIT( room->room_flags, BFS_MARK );

    if ( range > 0 )
    {
        EXIT_DATA* pexit;

        for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        {
            if ( pexit->to_room && pexit->to_room != room )
                yell_radius_2( ch, pexit->to_room, range - 1 );
        }
    }

    return;
}

void do_immtalk( CHAR_DATA* ch, char* argument )
{
    DESCRIPTOR_DATA* d;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !argument )
    {
        send_to_char( "Why don't you say something!\r\n", ch );
        return;
    }

    sprintf( buf, "&W&BImmNet&W[&C%s&W]: %s\r\n", ch->name, argument );
    sprintf( buf2, "&W&BImmNet&W[&CSomeone&W]: %s\r\n", argument );

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( d->connected != CON_PLAYING )
            continue;

        if ( !d->character )
            continue;

        if ( d->character->top_level < LEVEL_IMMORTAL )
            continue;

        if ( can_see( d->character, ch ) )
            send_to_char( buf, d->character );
        else
            send_to_char( buf2, d->character );
    }
    snprintf(buf, MAX_STRING_LENGTH, "%s,%s", ch->name, argument);
    mqtt_publish("out/channel/immtalk", buf);

    return;
}

void do_i103( CHAR_DATA* ch, char* argument )
{
    DESCRIPTOR_DATA* d;
    char buf[MAX_STRING_LENGTH];

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !argument )
    {
        send_to_char( "Why don't you say something!\r\n", ch );
        return;
    }

    sprintf( buf, "&W&BImmNet&W[&C%s&W]<&R103&W>: %s\r\n", ch->name, argument );

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( d->connected != CON_PLAYING )
            continue;

        if ( !d->character )
            continue;

        if ( d->character->top_level < 103 )
            continue;

        send_to_char( buf, d->character );
    }

    return;
}

void do_i104( CHAR_DATA* ch, char* argument )
{
    DESCRIPTOR_DATA* d;
    char buf[MAX_STRING_LENGTH];

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !argument )
    {
        send_to_char( "Why don't you say something!\r\n", ch );
        return;
    }

    sprintf( buf, "&W&BImmNet&W[&C%s&W]<&R104&W>: %s\r\n", ch->name, argument );

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( d->connected != CON_PLAYING )
            continue;

        if ( !d->character )
            continue;

        if ( d->character->top_level < 104 )
            continue;

        send_to_char( buf, d->character );
    }

    return;
}

void do_i105( CHAR_DATA* ch, char* argument )
{
    DESCRIPTOR_DATA* d;
    char buf[MAX_STRING_LENGTH];

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !argument )
    {
        send_to_char( "Why don't you say something!\r\n", ch );
        return;
    }

    sprintf( buf, "&W&BImmNet&W[&C%s&W]<&R105&W>: %s\r\n", ch->name, argument );

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( d->connected != CON_PLAYING )
            continue;

        if ( !d->character )
            continue;

        if ( d->character->top_level < 105 )
            continue;

        send_to_char( buf, d->character );
    }

    return;
}

void do_i200( CHAR_DATA* ch, char* argument )
{
    DESCRIPTOR_DATA* d;
    char buf[MAX_STRING_LENGTH];

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !argument )
    {
        send_to_char( "Why don't you say something!\r\n", ch );
        return;
    }

    sprintf( buf, "&W&BImmNet&W[&C%s&W]<&R200&W>: %s\r\n", ch->name, argument );

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( d->connected != CON_PLAYING )
            continue;

        if ( !d->character )
            continue;

        if ( d->character->top_level < 200 )
            continue;

        send_to_char( buf, d->character );
    }

    return;
}

void do_avtalk( CHAR_DATA* ch, char* argument )
{
    DESCRIPTOR_DATA* d;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( !argument )
    {
        send_to_char( "Why don't you say something!\r\n", ch );
        return;
    }

    if ( xIS_SET( ch->act, PLR_SILENCE ) )
    {
        send_to_char( "You can't do that!\n\r", ch );
        return;
    }

    sprintf( buf, "&W&GAvNet&W[&C%s&W]: %s\r\n", ch->name, argument );
    sprintf( buf2, "&W&GAvNet&W[&CSomeone&W]: %s\r\n", argument );
    sprintf( buf3, "&W&GAvNet&W[&CImmortal&W]: %s\r\n", argument );

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( d->connected != CON_PLAYING )
            continue;

        if ( !d->character )
            continue;

        if ( d->character->top_level < LEVEL_AVATAR )
            continue;

        if ( can_see( d->character, ch ) )
            send_to_char( buf, d->character );
        else
        {
            if ( !IS_IMMORTAL( ch ) )
                send_to_char( buf2, d->character );

            if ( IS_IMMORTAL( ch ) )
                send_to_char( buf3, d->character );
        }
    }

    return;
}

void do_say( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* vch;
    EXT_BV actflags;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Say what?\n\r", ch );
        return;
    }

    actflags = ch->act;

    if ( IS_NPC( ch ) )
        xREMOVE_BIT( ch->act, ACT_SECRETIVE );

    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
    {
        char* sbuf = argument;

        if ( vch == ch )
            continue;

        if ( !knows_language( vch, ch->speaking, ch ) && ( !IS_NPC( ch ) || !xIS_EMPTY( ch->speaking ) ) )
            sbuf = scramble( argument, number_range( 1000, 32000 ) );

        MOBtrigger = FALSE;

        if ( ch->race == vch->race )
        {
            char tmp[MAX_STRING_LENGTH];
            sprintf( tmp, "&w&z[&W%s&z] &C$n says '$t'", race_table[ch->race].race_name );
            act( AT_SAY, tmp, ch, sbuf, vch, TO_VICT );
        }
        else
        {
            act( AT_SAY, "$n says '$t'", ch, sbuf, vch, TO_VICT );
        }
    }

    ch->act = actflags;
    MOBtrigger = FALSE;
    act( AT_SAY, "You say '$T'", ch, NULL, argument, TO_CHAR );

    if ( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
        sprintf( buf, "[%d] %s: %s", ch->in_room->vnum, IS_NPC( ch ) ? ch->short_descr : ch->name, argument );
        append_to_file( LOG_FILE, buf );
    }

    mprog_speech_trigger( argument, ch );

    if ( char_died( ch ) )
        return;

    oprog_speech_trigger( argument, ch );

    if ( char_died( ch ) )
        return;

    rprog_speech_trigger( argument, ch );
    return;
}

void do_whisper( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* victim;
    int position;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    xREMOVE_BIT( ch->deaf, CHANNEL_WHISPER );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Whisper to whom what?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char( "You have a nice little chat with yourself.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched )
            && !IS_AFFECTED( victim->switched, AFF_POSSESS ) )
    {
        send_to_char( "That player is switched.\n\r", ch );
        return;
    }
    else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
        send_to_char( "That player is link-dead.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_AFK ) )
    {
        send_to_char( "That player is afk.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->deaf, CHANNEL_WHISPER )
            && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
        act( AT_PLAIN, "$E has $S whispers turned off.", ch, NULL, victim,
             TO_CHAR );
        return;
    }

    if ( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_SILENCE ) )
        send_to_char( "That player is silenced.  They will receive your message but can not respond.\n\r", ch );

    if ( victim->desc       /* make sure desc exists first  -Thoric */
            &&   victim->desc->connected == CON_EDITING
            &&   get_trust( ch ) < LEVEL_GOD )
    {
        act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
        return;
    }

    act( AT_WHISPER, "You whisper to $N '$t'", ch, argument, victim, TO_CHAR );
    position        = victim->position;
    victim->position    = POS_STANDING;
    act( AT_WHISPER, "$n whispers to you '$t'", ch, argument, victim, TO_VICT );
    victim->position    = position;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
        sprintf( buf, "[%d] %s: %s (whisper to) %s.", ch->in_room->vnum, IS_NPC( ch ) ? ch->short_descr : ch->name, argument, IS_NPC( victim ) ? victim->short_descr : victim->name );
        append_to_file( LOG_FILE, buf );
    }

    mprog_speech_trigger( argument, ch );
    return;
}

void do_tell( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    int position;
    CHAR_DATA* switched_victim = NULL;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    xREMOVE_BIT( ch->deaf, CHANNEL_TELLS );

    if ( !IS_NPC( ch ) && ( xIS_SET( ch->act, PLR_SILENCE ) || xIS_SET( ch->act, PLR_NO_TELL ) ) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Tell whom what?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL
            || ( IS_NPC( victim ) && victim->in_room != ch->in_room )
            || ( !NOT_AUTHED( ch ) && NOT_AUTHED( victim ) && !IS_IMMORTAL( ch ) ) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char( "You have a nice little chat with yourself.\n\r", ch );
        return;
    }

    if ( NOT_AUTHED( ch ) && !NOT_AUTHED( victim ) && !IS_IMMORTAL( victim ) )
    {
        send_to_char( "They can't hear you because you are not authorized.\n\r", ch );
        return;
    }

    if ( is_ignored( victim, ch->name ) )
    {
        send_to_char( "&w&CThat player can't hear you right now.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched )
            && ( get_trust( ch ) > LEVEL_AVATAR )
            && !xIS_SET( victim->switched->act, ACT_POLYMORPHED )
            && !IS_AFFECTED( victim->switched, AFF_POSSESS ) )
    {
        send_to_char( "That player is switched.\n\r", ch );
        return;
    }
    else if ( !IS_NPC( victim ) && ( victim->switched )
              && ( xIS_SET( victim->switched->act, ACT_POLYMORPHED )
                   ||  IS_AFFECTED( victim->switched, AFF_POSSESS ) ) )
        switched_victim = victim->switched;
    else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
        send_to_char( "That player is link-dead.\n\r", ch );
        return;
    }

    if ( !IS_NPC ( victim ) && ( xIS_SET ( victim->act, PLR_AFK ) ) )
    {
        send_to_char( "That player is afk.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->deaf, CHANNEL_TELLS )
            && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
        act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( !IS_NPC ( victim ) && ( xIS_SET ( victim->act, PLR_SILENCE ) ) )
    {
        send_to_char( "That player is silenced.  They will receive your message but can not respond.\n\r", ch );
    }

    if ( ( !IS_IMMORTAL( ch ) && !IS_AWAKE( victim ) ) )
    {
        act( AT_PLAIN, "$E is too tired to discuss such matters with you now.",
             ch, 0, victim, TO_CHAR );
        return;
    }

    if ( victim->desc       /* make sure desc exists first  -Thoric */
            &&   victim->desc->connected == CON_EDITING
            &&   get_trust( ch ) < LEVEL_GOD )
    {
        act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
        return;
    }

    if ( switched_victim )
        victim = switched_victim;

    sprintf( buf, "(OOC) You tell %s '$t'", victim->name );
    act( AT_TELL, buf, ch, argument, victim, TO_CHAR );
    position        = victim->position;
    victim->position    = POS_STANDING;
    sprintf( buf, "(OOC) %s tells you '$t'", ch->name );
    act( AT_TELL, buf, ch, argument, victim, TO_VICT );
    victim->position    = position;
    victim->reply   = ch;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
        sprintf( buf, "[%d] %s: %s (tell to) %s.", ch->in_room->vnum,
                 IS_NPC( ch ) ? ch->short_descr : ch->name,
                 argument, IS_NPC( victim ) ? victim->short_descr : victim->name );
        append_to_file( LOG_FILE, buf );
    }

    mprog_speech_trigger( argument, ch );
    return;
}



void do_reply( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* victim;
    int position;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    xREMOVE_BIT( ch->deaf, CHANNEL_TELLS );

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SILENCE ) )
    {
        send_to_char( "Your message didn't get through.\n\r", ch );
        return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched )
            && can_see( ch, victim ) && ( get_trust( ch ) > LEVEL_AVATAR ) )
    {
        send_to_char( "That player is switched.\n\r", ch );
        return;
    }
    else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
        send_to_char( "That player is link-dead.\n\r", ch );
        return;
    }

    if ( !IS_NPC ( victim ) && ( xIS_SET ( victim->act, PLR_AFK ) ) )
    {
        send_to_char( "That player is afk.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->deaf, CHANNEL_TELLS )
            && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
        act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
             TO_CHAR );
        return;
    }

    if ( is_ignored( victim, ch->name ) )
    {
        send_to_char( "&w&CThat player can't hear you right now.\n\r", ch );
        return;
    }

    sprintf( buf, "(OOC) You tell %s '$t'", victim->name );
    act( AT_TELL, buf, ch, argument, victim, TO_CHAR );
    position        = victim->position;
    victim->position    = POS_STANDING;
    sprintf( buf, "(OOC) %s tells you '$t'", ch->name );
    act( AT_TELL, buf, ch, argument, victim, TO_VICT );
    victim->position    = position;
    victim->reply   = ch;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
        sprintf( buf, "[%d] %s: %s (reply to) %s.", ch->in_room->vnum,
                 IS_NPC( ch ) ? ch->short_descr : ch->name,
                 argument,
                 IS_NPC( victim ) ? victim->short_descr : victim->name );
        append_to_file( LOG_FILE, buf );
    }

    return;
}



void do_emote( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char* plast;
    EXT_BV actflags;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_NO_EMOTE ) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }

    actflags = ch->act;

    if ( IS_NPC( ch ) )
        xREMOVE_BIT( ch->act, ACT_SECRETIVE );

    for ( plast = argument; *plast != '\0'; plast++ )
        ;

    strcpy( buf, argument );

    if ( isalpha( plast[-1] ) )
        strcat( buf, "." );

    MOBtrigger = FALSE;
    act( AT_SOCIAL, "$n $T", ch, NULL, buf, TO_ROOM );
    MOBtrigger = FALSE;
    act( AT_SOCIAL, "$n $T", ch, NULL, buf, TO_CHAR );
    ch->act = actflags;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
    {
        sprintf( buf, "[%d] %s %s (emote)", ch->in_room->vnum, IS_NPC( ch ) ? ch->short_descr : ch->name,
                 argument );
        append_to_file( LOG_FILE, buf );
    }

    return;
}


void do_bug( CHAR_DATA* ch, char* argument )
{
    char    buf[MAX_STRING_LENGTH];
    struct  tm* t = localtime( &current_time );
    set_char_color( AT_PLAIN, ch );

    if ( argument[0] == '\0' )
    {
        send_to_char( "\n\rUsage:  'bug <message>'  (your location is automatically recorded)\n\r", ch );

        if ( get_trust( ch ) >= LEVEL_ASCENDANT )
            send_to_char( "Usage:  'bug list' or 'bug clear now'\n\r",
                          ch );

        return;
    }

    sprintf( buf, "(%-2.2d/%-2.2d):  %s",
             t->tm_mon + 1, t->tm_mday, stripclr( argument ) );

    if ( !str_cmp( argument, "clear now" )
            &&    get_trust( ch ) >= LEVEL_ASCENDANT )
    {
        FILE* fp = fopen( PBUG_FILE, "w" );

        if ( fp )
            fclose( fp );

        send_to_char( "Bug file cleared.\n\r", ch );
        return;
    }

    if ( !str_cmp( argument, "list" ) && get_trust( ch ) >= LEVEL_ASCENDANT
       )
    {
        send_to_char( "\n\r VNUM \n\r.......\n\r", ch );
        show_file( ch, PBUG_FILE );
    }
    else
    {
        append_file( ch, PBUG_FILE, buf );
        send_to_char( "Thanks, your bug notice has been recorded.\n\r", ch
                    );
    }

    return;
}

void do_ide( CHAR_DATA* ch, char* argument )
{
    send_to_char( "If you want to send an idea, type 'idea <message>'.\n\r", ch );
    send_to_char( "If you want to identify an object and have the identify spell,\n\r", ch );
    send_to_char( "Type 'feel identify <object>'.\n\r", ch );
    return;
}

void do_idea( CHAR_DATA* ch, char* argument )
{
    set_char_color( AT_PLAIN, ch );

    if ( argument[0] == '\0' )
    {
        send_to_char( "\n\rUsage:  'idea <message>'\n\r", ch );

        if ( get_trust( ch ) >= LEVEL_ASCENDANT )
            send_to_char( "Usage:  'idea list' or 'idea clear now'\n\r", ch );

        return;
    }

    if ( !str_cmp( argument, "clear now" ) && get_trust( ch ) >= LEVEL_ASCENDANT )
    {
        FILE* fp = fopen( IDEA_FILE, "w" );

        if ( fp )
            fclose( fp );

        send_to_char( "Idea file cleared.\n\r", ch );
        return;
    }

    if ( !str_cmp( argument, "list" ) )
    {
        if ( get_trust( ch ) >= LEVEL_ASCENDANT )
        {
            send_to_char( "\n\r VNUM \n\r.......\n\r", ch );
            show_file( ch, IDEA_FILE );
        }
        else
        {
            send_to_char( "&ROnly Immortals may view the IDEA List.\n\r", ch );
            return;
        }
    }
    else
    {
        append_file( ch, IDEA_FILE, stripclr( argument ) );
        send_to_char( "Thanks, your idea has been recorded.\n\r", ch );
    }

    return;
}

/*
    COMPLAIN Command!
    Syntax: COMPLAIN <player> <message>
    Written by Ghost.
*/
void do_complain( CHAR_DATA* ch, char* argument )
{
    // char arg1[MAX_INPUT_LENGTH];
    if ( argument[0] == '\0' )
    {
        send_to_char( "&RUsage: 'complain <message>' \n\r", ch );
        send_to_char( "&RMake sure to include WHO your complaining about and WHY.\n\r", ch );
        return;
    }

    if ( !str_cmp( argument, "clear now" ) )
        if ( get_trust( ch ) >= 105 )
        {
            FILE* fp = fopen( COMPLAINT_FILE, "w" );

            if ( fp )
                fclose( fp );

            send_to_char( "Complaint file cleared.\n\r", ch );
            return;
        }

    if ( !str_cmp( argument, "list" ) )
    {
        if ( get_trust( ch ) >= 105 )
        {
            send_to_char( "\n\rComplaint File: ------- \n\r \n\r", ch );
            show_file( ch, COMPLAINT_FILE );
        }
    }
    else
    {
        append_file( ch, COMPLAINT_FILE, stripclr( argument ) );
        send_to_char( "Thanks, your complaint has been recorded.\n\r", ch );
    }

    return;
}

void do_typo( CHAR_DATA* ch, char* argument )
{
    set_char_color( AT_PLAIN, ch );

    if ( argument[0] == '\0' )
    {
        send_to_char( "\n\rUsage:  'typo <message>'  (your location is automatically recorded)\n\r", ch );

        if ( get_trust( ch ) >= LEVEL_ASCENDANT )
            send_to_char( "Usage:  'typo list' or 'typo clear now'\n\r", ch );

        return;
    }

    if ( !str_cmp( argument, "clear now" )
            &&    get_trust( ch ) >= LEVEL_SUPREME )
    {
        FILE* fp = fopen( TYPO_FILE, "w" );

        if ( fp )
            fclose( fp );

        send_to_char( "Typo file cleared.\n\r", ch );
        return;
    }

    if ( !str_cmp( argument, "list" ) && get_trust( ch ) >= LEVEL_ASCENDANT )
    {
        send_to_char( "VNUM \n\r.......\n\r", ch );
        show_file( ch, TYPO_FILE );
    }
    else
    {
        append_file( ch, TYPO_FILE, stripclr( argument ) );
        send_to_char( "Thanks, your typo notice has been recorded.\n\r", ch );
    }

    return;
}



void do_rent( CHAR_DATA* ch, char* argument )
{
    set_char_color( AT_WHITE, ch );
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}

void do_qui( CHAR_DATA* ch, char* argument )
{
    set_char_color( AT_RED, ch );
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}

void do_quit( CHAR_DATA* ch, char* argument )
{
    int x, y;
    int level;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&R[Spectator Mode]: You may not quit yet. Please wait.\n\r", ch );
        return;
    }

    if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_POLYMORPHED ) )
    {
        send_to_char( "You can't quit while polymorphed.\n\r", ch );
        return;
    }

    if ( IN_VENT( ch ) )
    {
        send_to_char( "You can't quit while inside a vent.\n\r", ch );
        return;
    }

    if ( IS_NPC( ch ) )
        return;

    if ( ch->position < POS_STUNNED )
    {
        set_char_color( AT_BLOOD, ch );
        send_to_char( "You're not DEAD yet.\n\r", ch );
        return;
    }

    if ( !IS_IMMORTAL( ch ) && ch->in_room && !is_home( ch ) && !NOT_AUTHED( ch ) && ch->desc )
    {
        send_to_char( "You may not quit while deployed.\n\r", ch );
        return;
    }

    set_char_color( AT_WHITE, ch );
    send_to_char( "Your surroundings begin to fade as a mystical swirling vortex of colors\n\renvelops your body... When you come to, things are not as they were.\n\r\n\r", ch );
    // random_quote( ch );
    act( AT_SAY, "\n\rA strange voice says, 'We await your return, $n...'", ch, NULL, NULL, TO_CHAR );

    if ( !IS_IMMORTAL( ch ) )
    {
        char tmp[MAX_STRING_LENGTH];
        sprintf( tmp, "&W%s has left the game.", ch->name );
        send_monitor( ch, tmp );
        act( AT_BYE, "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    }

    set_char_color( AT_GREY, ch );
    sprintf( log_buf, "%s has quit at %d.", ch->name, ch->in_room->vnum );
    quitting_char = ch;
    save_char_obj( ch );
    saving_char = NULL;
    /* Experimental 'drop to menu' */
    /*
        char_from_room( ch );
        remove_char( ch );
        send_to_char( "\n\r&w", ch );
        write_menu_to_desc( ch->desc );
        ch->desc->connected = CON_MENU_BASE;
        ch->desc = NULL;
        return;
    */
    level = get_trust( ch );
    /*
        After extract_char the ch is no longer valid!
    */
    extract_char( ch, TRUE, FALSE );

    for ( x = 0; x < MAX_WEAR; x++ )
        for ( y = 0; y < MAX_LAYERS; y++ )
        {
            save_equipment[0][x][y] = NULL;
            save_equipment[1][x][y] = NULL;
        }

    /* don't show who's logging off to leaving player */
    /*
        to_channel( log_buf, CHANNEL_MONITOR, "Monitor", level );
    */
    log_string_plus( log_buf, LOG_COMM, level );
    write_serverstats();
    return;
}

void random_quote ( CHAR_DATA* ch )
{
    char buf[MAX_STRING_LENGTH];
    int number;
    number = number_range( 0, MAX_QUOTES - 1 );
    sprintf ( buf, "&Y\n\rRandom quote of the minute:\n\r  %s\n\r   - %s\n\r",
              quote_table[number].text, quote_table[number].by );
    send_to_char ( buf, ch );
    return;
}

void send_rip_screen( CHAR_DATA* ch )
{
    FILE* rpfile;
    int num = 0;
    char BUFF[MAX_STRING_LENGTH * 2];

    if ( ( rpfile = fopen( RIPSCREEN_FILE, "r" ) ) != NULL )
    {
        while ( ( BUFF[num] = fgetc( rpfile ) ) != EOF )
            num++;

        fclose( rpfile );
        BUFF[num] = 0;
        write_to_buffer( ch->desc, BUFF, num );
    }
}

void send_rip_title( CHAR_DATA* ch )
{
    FILE* rpfile;
    int num = 0;
    char BUFF[MAX_STRING_LENGTH * 2];

    if ( ( rpfile = fopen( RIPTITLE_FILE, "r" ) ) != NULL )
    {
        while ( ( BUFF[num] = fgetc( rpfile ) ) != EOF )
            num++;

        fclose( rpfile );
        BUFF[num] = 0;
        write_to_buffer( ch->desc, BUFF, num );
    }
}

void send_ansi_title( CHAR_DATA* ch )
{
    FILE* rpfile;
    int num = 0;
    char BUFF[MAX_STRING_LENGTH * 2];

    if ( ( rpfile = fopen( ANSITITLE_FILE, "r" ) ) != NULL )
    {
        while ( ( BUFF[num] = fgetc( rpfile ) ) != EOF )
            num++;

        fclose( rpfile );
        BUFF[num] = 0;
        write_to_buffer( ch->desc, BUFF, num );
    }
}

void send_ascii_title( CHAR_DATA* ch )
{
    FILE* rpfile;
    int num = 0;
    char BUFF[MAX_STRING_LENGTH];

    if ( ( rpfile = fopen( ASCTITLE_FILE, "r" ) ) != NULL )
    {
        while ( ( BUFF[num] = fgetc( rpfile ) ) != EOF )
            num++;

        fclose( rpfile );
        BUFF[num] = 0;
        write_to_buffer( ch->desc, BUFF, num );
    }
}

void do_rip( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Rip ON or OFF?\n\r", ch );
        return;
    }

    if ( ( strcmp( arg, "on" ) == 0 ) || ( strcmp( arg, "ON" ) == 0 ) )
    {
        send_rip_screen( ch );
        xSET_BIT( ch->act, PLR_ANSI );
        return;
    }

    if ( ( strcmp( arg, "off" ) == 0 ) || ( strcmp( arg, "OFF" ) == 0 ) )
    {
        send_to_char( "!|*\n\rRIP now off...\n\r", ch );
        return;
    }
}

void do_ansi( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "ANSI ON or OFF?\n\r", ch );
        return;
    }

    if ( ( strcmp( arg, "on" ) == 0 ) || ( strcmp( arg, "ON" ) == 0 ) )
    {
        xSET_BIT( ch->act, PLR_ANSI );
        set_char_color( AT_WHITE + AT_BLINK, ch );
        send_to_char( "ANSI ON!!!\n\r", ch );
        return;
    }

    if ( ( strcmp( arg, "off" ) == 0 ) || ( strcmp( arg, "OFF" ) == 0 ) )
    {
        xREMOVE_BIT( ch->act, PLR_ANSI );
        send_to_char( "Okay... ANSI support is now off\n\r", ch );
        return;
    }
}

void do_censor( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: CENSOR <on | off>\n\r", ch );
        send_to_char( " Toggles personal profanity filter.\n\r", ch );
        return;
    }

    if ( ( strcmp( arg, "on" ) == 0 ) || ( strcmp( arg, "ON" ) == 0 ) )
    {
        xSET_BIT( ch->act, PLR_CENSOR );
        send_to_char( "&z[&RALERT&z]: &CProfanity filter is now on.\n\r", ch );
        return;
    }

    if ( ( strcmp( arg, "off" ) == 0 ) || ( strcmp( arg, "OFF" ) == 0 ) )
    {
        xREMOVE_BIT( ch->act, PLR_CENSOR );
        send_to_char( "&z[&RALERT&z]: &CProfanity filter is now off.\n\r", ch );
        return;
    }
}

void do_sound( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "SOUND ON or OFF?\n\r", ch );
        return;
    }

    if ( ( strcmp( arg, "on" ) == 0 ) || ( strcmp( arg, "ON" ) == 0 ) )
    {
        xSET_BIT( ch->act, PLR_SOUND );
        set_char_color( AT_WHITE + AT_BLINK, ch );
        send_to_char( "SOUND ON!!!\n\r", ch );
        send_to_char( "!!SOUND(soundon.wav)", ch );
        return;
    }

    if ( ( strcmp( arg, "off" ) == 0 ) || ( strcmp( arg, "OFF" ) == 0 ) )
    {
        xREMOVE_BIT( ch->act, PLR_SOUND );
        send_to_char( "Okay... SOUND support is now off\n\r", ch );
        return;
    }
}

void do_save( CHAR_DATA* ch, char* argument )
{
    if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_POLYMORPHED ) )
    {
        send_to_char( "You can't save while polymorphed.\n\r", ch );
        return;
    }

    if ( IS_NPC( ch ) )
        return;

    /*  Extended Bitvectors damaged this
        if ( !IS_AFFECTED( ch, race_table[ch->race].affected ) )
        xSET_BIT( ch->affected_by, race_table[ch->race].affected );
        if ( !IS_AFFECTED( ch, race_table[ch->race].resist ) )
        xSET_BIT( ch->resistant, race_table[ch->race].resist );
        if ( !IS_AFFECTED( ch, race_table[ch->race].suscept ) )
        xSET_BIT( ch->susceptible, race_table[ch->race].suscept );
    */

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "You can't save until after you've graduated from the acadamey.\n\r", ch );
        return;
    }

    save_char_obj( ch );
    saving_char = NULL;

    if ( strcmp( argument, "-noalert" ) )
        send_to_char( "Character Saved.\n\r", ch );

    return;
}


/*
    Something from original DikuMUD that Merc yanked out.
    Used to prevent following loops, which can cause problems if people
    follow in a loop through an exit leading back into the same room
    (Which exists in many maze areas)            -Thoric
*/
bool circle_follow( CHAR_DATA* ch, CHAR_DATA* victim )
{
    CHAR_DATA* tmp;

    for ( tmp = victim; tmp; tmp = tmp->master )
        if ( tmp == ch )
            return TRUE;

    return FALSE;
}

void do_dismiss( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Dismiss whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ( IS_AFFECTED( victim, AFF_CHARM ) )
            && ( IS_NPC( victim ) )
            && ( victim->master == ch ) )
    {
        stop_follower( victim );
        stop_hating( victim );
        stop_hunting( victim );
        stop_fearing( victim );
        act( AT_ACTION, "$n dismisses $N.", ch, NULL, victim, TO_NOTVICT );
        act( AT_ACTION, "You dismiss $N.", ch, NULL, victim, TO_CHAR );
    }
    else
    {
        send_to_char( "You cannot dismiss them.\n\r", ch );
    }

    return;
}

void do_follow( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Follow whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master )
    {
        act( AT_PLAIN, "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
        return;
    }

    if ( victim == ch )
    {
        if ( !ch->master )
        {
            send_to_char( "You already follow yourself.\n\r", ch );
            return;
        }

        stop_follower( ch );
        return;
    }

    if ( victim->race != ch->race )
    {
        send_to_char( "You can only follow members of your own race.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "That would be pretty damn pointless.\n\r", ch );
        return;
    }

    if ( circle_follow( ch, victim ) )
    {
        send_to_char( "Following in loops is not allowed... sorry.\n\r", ch );
        return;
    }

    if ( ch->master )
        stop_follower( ch );

    add_follower( ch, victim );
    return;
}



void add_follower( CHAR_DATA* ch, CHAR_DATA* master )
{
    if ( ch->master )
    {
        bug( "Add_follower: non-null master.", 0 );
        return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
        act( AT_ACTION, "$n now follows you.", ch, NULL, master, TO_VICT );

    act( AT_ACTION, "You now follow $N.",  ch, NULL, master, TO_CHAR );
    return;
}



void stop_follower( CHAR_DATA* ch )
{
    if ( !ch->master )
    {
        bug( "Stop_follower: null master.", 0 );
        return;
    }

    if ( can_see( ch->master, ch ) )
        act( AT_ACTION, "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );

    act( AT_ACTION, "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    ch->master = NULL;
    ch->leader = NULL;
    return;
}



void die_follower( CHAR_DATA* ch )
{
    CHAR_DATA* fch;

    if ( ch->master )
        stop_follower( ch );

    ch->leader = NULL;

    for ( fch = first_char; fch; fch = fch->next )
    {
        if ( fch->master == ch )
            stop_follower( fch );

        if ( fch->leader == ch )
            fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char argbuf[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    CHAR_DATA* och;
    CHAR_DATA* och_next;
    bool found;
    bool fAll;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    strcpy( argbuf, argument );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Order whom to do what?\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        fAll   = TRUE;
        victim = NULL;
    }
    else
    {
        fAll   = FALSE;

        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( victim == ch )
        {
            send_to_char( "Aye aye, right away!\n\r", ch );
            return;
        }

        if ( !IS_AFFECTED( victim, AFF_CHARM ) || victim->master != ch )
        {
            send_to_char( "Do it yourself!\n\r", ch );
            return;
        }
    }

    found = FALSE;

    for ( och = ch->in_room->first_person; och; och = och_next )
    {
        och_next = och->next_in_room;

        if ( IS_AFFECTED( och, AFF_CHARM )
                &&   och->master == ch
                && ( fAll || och == victim ) )
        {
            found = TRUE;
            act( AT_ACTION, "$n orders you to '$t'.", ch, argument, och, TO_VICT );
            interpret( och, argument, TRUE );
        }
    }

    if ( found )
    {
        sprintf( log_buf, "%s: order %s.", ch->name, argbuf );
        log_string_plus( log_buf, LOG_NORMAL, ch->top_level );
        send_to_char( "Ok.\n\r", ch );
        WAIT_STATE( ch, 12 );
    }
    else
        send_to_char( "You have no followers here.\n\r", ch );

    return;
}

/*
    Types:
    0 = All
    1 = Mobs only
    2 = Players only
*/
int count_followers( CHAR_DATA* ch, int type, bool inroom )
{
    CHAR_DATA* rch;
    int count = 0;

    if ( inroom && !ch->in_room )
        return 0;

    for ( rch = first_char; rch; rch = rch->next )
    {
        if ( ch == rch )
            continue;

        if ( inroom && ch->in_room != rch->in_room )
            continue;

        if ( rch->master == ch && !ch->master )
        {
            if ( IS_NPC( rch ) && type != 2 )
                count++;

            if ( !IS_NPC( rch ) && type != 1 )
                count++;
        }
    }

    return count;
}

/*
    It is very important that this be an equivalence relation:
    (1) A ~ A
    (2) if A ~ B then B ~ A
    (3) if A ~ B  and B ~ C, then A ~ C
*/
bool is_same_group( CHAR_DATA* ach, CHAR_DATA* bch )
{
    if ( ach->leader )
        ach = ach->leader;

    if ( bch->leader )
        bch = bch->leader;

    return ach == bch;
}

/*
    Language support functions. -- Altrag
    07/01/96
*/
bool knows_language( CHAR_DATA* ch, EXT_BV language, CHAR_DATA* cch )
{
    sh_int sn;

    if ( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
        return TRUE;

    if ( IS_NPC( ch ) && xIS_EMPTY( ch->speaks ) ) /* No langs = knows all for npcs */
        return TRUE;

    if ( IS_NPC( ch ) )
    {
        int lang;

        for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
            if ( xIS_SET( language, lang ) && xIS_SET( ch->speaks, lang ) )
            {
                return TRUE;
            }
    }

    if ( !IS_NPC( ch ) )
    {
        int lang;

        /* Racial languages for PCs */
        if ( xIS_SET( language, race_table[ch->race].language ) )
            return TRUE;

        for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
        {
            sn = skill_lookup( lang_names[lang] );

            if ( sn == -1 )
                continue;

            /* Safty patch - Make sure the flag has been set */
            if ( ch->pcdata->learned[sn] > 0 && !xIS_SET( ch->speaks, lang ) )
            {
                /* bug("Repaired missing language flag on %s. Bug Fixed.", ch->name); */
                xSET_BIT( ch->speaks, lang );
            }

            if ( xIS_SET( language, lang ) && xIS_SET( ch->speaks, lang ) )
            {
                if ( ch->pcdata->learned[sn] >= 60 )
                    return TRUE;
            }
        }
    }

    return FALSE;
}

bool can_learn_lang( CHAR_DATA* ch, int language )
{
    if ( IS_NPC( ch ) || IS_IMMORTAL( ch ) )
        return FALSE;

    if ( race_table[ch->race].language == language )
        return FALSE;

    if ( xIS_SET( ch->speaks, language ) )
    {
        int lang;

        for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
            if ( language == lang )
            {
                int sn;

                if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
                {
                    bug( "Can_learn_lang: valid language without sn: %d", lang );
                    continue;
                }

                if ( ch->pcdata->learned[sn] >= 99 )
                    return FALSE;
            }
    }

    /* if ( VALID_LANGS & language ) */
    return TRUE;
    return FALSE;
}

/* Note: does not count racial language.  This is intentional (for now). */
int countlangs( int languages )
{
    int numlangs = 0;
    int looper;

    for ( looper = 0; lang_array[looper] != LANG_UNKNOWN; looper++ )
    {
        if ( languages & lang_array[looper] )
            numlangs++;
    }

    return numlangs;
}

void do_speak( CHAR_DATA* ch, char* argument )
{
    EXT_BV xchk;
    int langs;
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "&zSyntax: &CSPEAK &z<&Clangauge&z>\n\r", ch );

        if ( IS_IMMORTAL( ch ) || IS_NPC( ch ) )
        {
            ch_printf( ch, "&zYou can speak all languages.\n\r" );
        }
        else
        {
            ch_printf( ch, "&zYou can speak: &B%s&z.\n\r", flag_string( &ch->speaks, lang_names, LANG_UNKNOWN ) );
        }

        return;
    }

    if ( !str_cmp( arg, "all" ) && IS_IMMORTAL( ch ) )
    {
        set_char_color( AT_SAY, ch );
        xCLEAR_BITS( ch->speaking );

        for ( langs = 0; lang_array[langs] != LANG_UNKNOWN; langs++ )
        {
            xSET_BIT( ch->speaking, langs );
        }

        send_to_char( "Now speaking all languages.\n\r", ch );
        return;
    }

    // xSET_BIT( xchk, lang_array[langs] );
    for ( langs = 0; lang_array[langs] != LANG_UNKNOWN; langs++ )
    {
        xCLEAR_BITS( xchk );
        xSET_BIT( xchk, langs );

        if ( !str_prefix( arg, lang_names[langs] ) )
        {
            if ( knows_language( ch, xchk, ch ) )
            {
                xCLEAR_BITS( ch->speaking );
                xSET_BIT( ch->speaking, lang_array[langs] );
                set_char_color( AT_SAY, ch );
                ch_printf( ch, "You now speak %s.\n\r", lang_names[langs] );
            }
            else
            {
                ch_printf( ch, "You don't know how to speak %s.\n\r", lang_names[langs] );
                return;
            }

            return;
        }
    }

    set_char_color( AT_SAY, ch );
    send_to_char( "You do not know that language.\n\r", ch );
    return;
}

void do_languages( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    EXT_BV chk;
    int lang;
    int sn;
    argument = one_argument( argument, arg );

    if ( IS_NPC( ch ) )
        return;

    if ( arg[0] != '\0' && !str_prefix( arg, "learn" ) && !IS_NPC( ch ) )
    {
        CHAR_DATA* sch = NULL;
        char arg2[MAX_INPUT_LENGTH];
        int prct;
        argument = one_argument( argument, arg2 );

        if ( arg2[0] == '\0' )
        {
            send_to_char( "&zLearn which language?\n\r", ch );
            return;
        }

        for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
        {
            if ( !str_prefix( arg2, lang_names[lang] ) )
                break;
        }

        if ( lang_array[lang] == LANG_UNKNOWN )
        {
            send_to_char( "&zThat is not a language.\n\r", ch );
            return;
        }

        if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
        {
            send_to_char( "&zThat is not a language.\n\r", ch );
            return;
        }

        if ( ch->pcdata->learned[sn] >= 100 )
        {
            ch_printf( ch, "&zYou are already fluent in &C%s&z.\n\r", lang_names[lang] );
            return;
        }

        xSET_BIT( chk, lang_array[lang] );

        /*
            if ( knows_language( sch, ch->speaking, ch ) && knows_language( sch, chk, sch ) )
            if ( xIS_EMPTY(sch->speaking) || knows_language( ch, sch->speaking, sch ) )
        */

        for ( sch = ch->in_room->first_person; sch != NULL; sch = sch->next_in_room )
            if ( IS_NPC( sch ) && xIS_SET( sch->act, ACT_SCHOLAR ) )
                break;

        if ( sch == NULL || !IS_NPC( sch ) )
        {
            send_to_char( "&zThere is no one who can teach that language here.\n\r", ch );
            return;
        }

        if ( !knows_language( sch, ch->speaking, ch ) )
        {
            do_say( sch, "Sorry, I dont understand what your saying!" );
            return;
        }

        if ( !knows_language( sch, chk, sch ) )
        {
            do_say( sch, "Sorry, I can't teach you that language. I don't know it." );
            return;
        }

        if ( !xIS_EMPTY( sch->speaking ) ) /* && !knows_language( ch, sch->speaking, sch ) ) */
        {
            do_say( sch, "Sorry, I can't teach you that language." );
            return;
        }

        /* Max 12% (5 + 4 + 3) at 24+ int and 21+ wis. -- Altrag */
        prct = ( 5 + ( get_curr_int( ch ) / 6 ) );
        ch->pcdata->learned[sn] += prct;
        ch->pcdata->learned[sn] = UMIN( ch->pcdata->learned[sn], 100 );
        xSET_BIT( ch->speaks, lang_array[lang] );

        if ( ch->pcdata->learned[sn] == prct )
            ch_printf( ch, "&zYou begin lessons in &C%s&z.\n\r", lang_names[lang] );
        else if ( ch->pcdata->learned[sn] < 60 )
            ch_printf( ch, "&zYou continue lessons in &C%s&z.\n\r", lang_names[lang] );
        else if ( ch->pcdata->learned[sn] < 60 + prct )
            ch_printf( ch, "&zYou feel you can start communicating in &C%s&z.", lang_names[lang] );
        else if ( ch->pcdata->learned[sn] <= 99 )
            ch_printf( ch, "&zYou become more fluent in &C%s&z.\n\r", lang_names[lang] );
        else
            ch_printf( ch, "&zYou now speak perfect &C%s&z.\n\r", lang_names[lang] );

        return;
    }

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
    {
        if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
            send_to_char( " &z(  &C0&z) ", ch );
        else
            ch_printf( ch, " &z(&C%3d&z) ", ch->pcdata->learned[sn] );

        if ( xIS_SET( ch->speaking, lang_array[lang] ) || ( IS_NPC( ch ) && xIS_EMPTY( ch->speaking ) ) )
            sprintf( buf, "&b%-15s&z &z(&BSpeaking&z)\n\r", lang_names[lang] );
        else
            sprintf( buf, "&B%-15s&z\n\r", lang_names[lang] );

        send_to_char( buf, ch );
    }

    /* send_to_char( "\n\r", ch ); */
    return;
}

void do_wartalk( CHAR_DATA* ch, char* argument )
{
    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( NOT_AUTHED( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    talk_channel( ch, argument, CHANNEL_WARTALK, "war" );
    return;
}

void motion_ping( int x, int y, int z, AREA_DATA* area, CHAR_DATA* ignore )
{
    DESCRIPTOR_DATA* d;
    CHAR_DATA* ch;
    int dist;
    int mod = 10;

    /* Only players need to hear the ping, so skim descriptors */
    for ( d = first_descriptor; d; d = d->next )
    {
        ch = d->original ? d->original : d->character;

        if ( ch == NULL )
            continue;

        if ( !ch->in_room )
            continue;

        if ( is_spectator( ch ) )
            continue;

        if ( d->connected == CON_PLAYING && ignore != ch && ch->race == RACE_MARINE )
        {
            if ( !has_motion_tracker( ch ) )
                continue;

            if ( ch->hit <= 0 )
                continue;

            if ( IS_AFFECTED( ch, AFF_BLIND ) )
                continue;

            if ( z != ch->in_room->z )
                continue;

            if ( ch->in_room->area != area )
                continue;

            dist = get_sprox( x * mod, y * mod, z * mod, ch->in_room->x * mod, ch->in_room->y * mod, ch->in_room->z * mod );

            if ( ( ( float )( dist ) / ( float )( mod ) ) <= ( float )( 3 ) )
            {
                char direct[MAX_STRING_LENGTH];
                strcpy( direct, "" );

                if ( z != ch->in_room->z )
                {
                    if ( z  < ch->in_room->z )
                        strcpy( direct, " somewhere below" );

                    if ( z  > ch->in_room->z )
                        strcpy( direct, " somewhere above" );
                }
                else if ( x < ch->in_room->x )
                {
                    if ( y  < ch->in_room->y )
                        strcpy( direct, " southwest" );

                    if ( y  > ch->in_room->y )
                        strcpy( direct, " northwest" );

                    if ( y == ch->in_room->y )
                        strcpy( direct, " west" );
                }
                else if ( x > ch->in_room->x )
                {
                    if ( y  < ch->in_room->y )
                        strcpy( direct, " southeast" );

                    if ( y  > ch->in_room->y )
                        strcpy( direct, " northeast" );

                    if ( y == ch->in_room->y )
                        strcpy( direct, " east" );
                }
                else if ( x == ch->in_room->x )
                {
                    if ( y < ch->in_room->y )
                        strcpy( direct, " south" );

                    if ( y > ch->in_room->y )
                        strcpy( direct, " north" );
                }

                ch_printf( ch, "&R(Alert) Movement detected at %d meters%s.\n\r", dist, direct );
            }
        }
    }

    return;
}

void team_xpgain( int race, int exp )
{
    DESCRIPTOR_DATA* d;
    CHAR_DATA* ch;

    for ( d = first_descriptor; d; d = d->next )
    {
        ch = d->original ? d->original : d->character;

        if ( ch == NULL )
            continue;

        if ( ch->race != race )
            continue;

        if ( d->connected == CON_PLAYING && d->idle < 240 )
        {
            ch_printf( ch, "&w&YYou have been awarded %ld experience.\n\r", exp );
            gain_exp( ch, exp );
        }

        ch = NULL;
    }

    return;
}

void send_monitor( CHAR_DATA* ignore, char* msg )
{
    DESCRIPTOR_DATA* d;
    CHAR_DATA* ch;

    for ( d = first_descriptor; d; d = d->next )
    {
        ch = d->original ? d->original : d->character;

        if ( d->connected == CON_PLAYING && ( ignore != ch || ignore == NULL ) )
        {
            ch_printf( ch, "&B[&CMonitor&B]&w %s\n\r", msg );
        }
    }

    mqtt_publish("out/channel/arena_monitor", msg);
    return;
}

void sound_radius( char* msg, ROOM_INDEX_DATA* room, int radius, CHAR_DATA* ignore )
{
    DESCRIPTOR_DATA* d;
    CHAR_DATA* ch;
    int x, y, z, dist;
    int mod = 10;

    if ( room == NULL )
        return;

    x = room->x;
    y = room->y;
    z = room->z;

    /* Only players need to hear the ping for now, so skim descriptors */
    for ( d = first_descriptor; d; d = d->next )
    {
        ch = d->original ? d->original : d->character;

        if ( is_spectator( ch ) )
            continue;

        if ( d->connected == CON_PLAYING && ignore != ch )
        {
            dist = get_sprox( x * mod, y * mod, z * mod, ch->in_room->x * mod, ch->in_room->y * mod, ch->in_room->z * mod );

            if ( dist == 0 )
                continue;

            if ( ( ( float )( dist ) / ( float )( mod ) ) <= ( float )( radius ) )
            {
                char direct[MAX_STRING_LENGTH];

                if ( z > ch->in_room->z + 1 )
                    continue;

                if ( z < ch->in_room->z - 1 )
                    continue;

                strcpy( direct, "" );

                if ( z != ch->in_room->z )
                {
                    if ( z  < ch->in_room->z )
                        strcpy( direct, "(Below) " );

                    if ( z  > ch->in_room->z )
                        strcpy( direct, "(Above) " );
                }
                else if ( x < ch->in_room->x )
                {
                    if ( y  < ch->in_room->y )
                        strcpy( direct, "Southwest" );

                    if ( y  > ch->in_room->y )
                        strcpy( direct, "Northwest" );

                    if ( y == ch->in_room->y )
                        strcpy( direct, "West" );
                }
                else if ( x > ch->in_room->x )
                {
                    if ( y  < ch->in_room->y )
                        strcpy( direct, "Southeast" );

                    if ( y  > ch->in_room->y )
                        strcpy( direct, "Northeast" );

                    if ( y == ch->in_room->y )
                        strcpy( direct, "East" );
                }
                else if ( x == ch->in_room->x )
                {
                    if ( y < ch->in_room->y )
                        strcpy( direct, "South" );

                    if ( y > ch->in_room->y )
                        strcpy( direct, "North" );
                }

                ch_printf( ch, "&w&z[&WSound&z] &C%s&z: &W%s.\n\r", direct, msg );
            }
        }
    }

    return;
}

void send_sound( char* buf, ROOM_INDEX_DATA* room, int volume, CHAR_DATA* ch )
{
    sound_radius_2( ch, room, volume );
    sound_radius_1( ch, room, buf, volume, -1 );
    sound_radius_2( ch, room, volume );
    return;
}

void sound_radius_1( CHAR_DATA* ch, ROOM_INDEX_DATA* room, char* buf, int range, int dir )
{
    char* dtxt;
    CHAR_DATA* rch;
    CHAR_DATA* rnext;
    bool same = FALSE;

    if ( xIS_SET( room->room_flags, BFS_MARK ) )
        return;

    xSET_BIT( room->room_flags, BFS_MARK );

    if ( ch->in_room == room )
    {
        same = TRUE;
    }
    else
    {
        switch ( dir )
        {
            default:
                dtxt = "somewhere";
                break;

            case 0:
                dtxt = "the south";
                break;

            case 1:
                dtxt = "the west";
                break;

            case 2:
                dtxt = "the north";
                break;

            case 3:
                dtxt = "the east";
                break;

            case 4:
                dtxt = "below";
                break;

            case 5:
                dtxt = "above";
                break;

            case 6:
                dtxt = "the south-west";
                break;

            case 7:
                dtxt = "the south-east";
                break;

            case 8:
                dtxt = "the north-west";
                break;

            case 9:
                dtxt = "the north-east";
                break;
        }
    }

    for ( rch = room->first_person ; rch ;  rch = rnext )
    {
        rnext = rch->next_in_room;

        if ( IS_NPC( rch ) || rch == ch )
            continue;

        if ( rch->hit <= 0 )
            continue;

        if ( !same )
            ch_printf( rch, "&w[Sound] %s from %s!\n\r", buf, dtxt );
    }

    /* Branch out */
    {
        EXIT_DATA* pexit;

        for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        {
            if ( pexit->to_room && pexit->to_room != room )
            {
                if ( range > 0 )
                {
                    sound_radius_1( ch, pexit->to_room, buf, range - 1, pexit->vdir );
                }
            }
        }
    }
    return;
}

void sound_radius_2( CHAR_DATA* ch, ROOM_INDEX_DATA* room, int range )
{
    if ( !xIS_SET( room->room_flags, BFS_MARK ) )
        return;

    xREMOVE_BIT( room->room_flags, BFS_MARK );

    if ( range > 0 )
    {
        EXIT_DATA* pexit;

        for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        {
            if ( pexit->to_room && pexit->to_room != room )
                sound_radius_2( ch, pexit->to_room, range - 1 );
        }
    }

    return;
}

void radio_broadcast( CHAR_DATA* ch, char* in_msg )
{
    char bufA[MAX_STRING_LENGTH];
    char bufB[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA* d;
    CHAR_DATA* rch;
    AREA_DATA* area;
    char* messageN;    // Normal
    char* messageS;    // Static
    int dist, x, y, z;
    int mod = 10;
    int radio_distance = 10;

    if ( !ch )
        return;

    if ( !ch->in_room )
        return;

    messageN = in_msg;
    messageS = in_msg;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_STATIC ) )
        messageN = disrupt( in_msg, 50 );

    sprintf( bufA, "&w&zRadio&W[&C%s&W]: %s\n\r", IS_NPC( ch ) ? ch->short_descr : ch->name, messageN );
    messageS = disrupt( in_msg, 50 );
    sprintf( bufB, "&w&zRadio&W[&C%s&W]: %s\n\r", IS_NPC( ch ) ? ch->short_descr : ch->name, messageS );
    x = ch->in_room->x;
    y = ch->in_room->y;
    z = ch->in_room->z;
    area = ch->in_room->area;

    /* Only players need to hear the radio, so skim descriptors */
    for ( d = first_descriptor; d; d = d->next )
    {
        rch = d->original ? d->original : d->character;

        if ( rch == NULL )
            continue;

        if ( !rch->in_room )
            continue;

        if ( d->connected == CON_PLAYING && rch->race == RACE_MARINE )
        {
            if ( !has_radio( rch ) )
                continue;

            if ( rch->in_room->area != area )
                continue;

            dist = get_sprox( x * mod, y * mod, z * mod, rch->in_room->x * mod, rch->in_room->y * mod, rch->in_room->z * mod );

            if ( ( ( float )( dist ) / ( float )( mod ) ) <= ( float )( radio_distance ) )
            {
                if ( xIS_SET( rch->in_room->room_flags, ROOM_STATIC ) )
                {
                    ch_printf( rch, bufB );
                }
                else
                {
                    ch_printf( rch, bufA );
                }
            }
        }
    }

    return;
}

void quick_radio( CHAR_DATA* ch, int menu, int opt )
{
    int i = 0;

    if ( menu <= 0 || menu >= 5 )
        return;

    if ( ch->race != RACE_MARINE )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( opt <= 0 )
    {
        if ( menu == 1 )
            ch_printf( ch, "\n\r&wRadio Menu 1&Z) &W%s.\n\r", radio_set1[0] );

        if ( menu == 2 )
            ch_printf( ch, "\n\r&wRadio Menu 2&Z) &W%s.\n\r", radio_set2[0] );

        if ( menu == 3 )
            ch_printf( ch, "\n\r&wRadio Menu 3&Z) &W%s.\n\r", radio_set3[0] );

        if ( menu == 4 )
            ch_printf( ch, "\n\r&wRadio Menu 4&Z) &W%s.\n\r", radio_set4[0] );

        for ( i = 1; i < 999; i++ )
        {
            if ( menu == 1 )
            {
                if ( !str_cmp( radio_set1[i], "$" ) )
                    break;
            }

            if ( menu == 2 )
            {
                if ( !str_cmp( radio_set2[i], "$" ) )
                    break;
            }

            if ( menu == 3 )
            {
                if ( !str_cmp( radio_set3[i], "$" ) )
                    break;
            }

            if ( menu == 4 )
            {
                if ( !str_cmp( radio_set4[i], "$" ) )
                    break;
            }

            if ( menu == 1 )
            {
                ch_printf( ch, "&W%d &z:: &C%s\n\r", i, radio_set1[i] );
            }

            if ( menu == 2 )
            {
                ch_printf( ch, "&W%d &z:: &C%s\n\r", i, radio_set2[i] );
            }

            if ( menu == 3 )
            {
                ch_printf( ch, "&W%d &z:: &C%s\n\r", i, radio_set3[i] );
            }

            if ( menu == 4 )
            {
                ch_printf( ch, "&W%d &z:: &C%s\n\r", i, radio_set4[i] );
            }
        }

        return;
    }
    else
    {
        for ( i = 1; i < 999; i++ )
        {
            if ( menu == 1 )
            {
                if ( !str_cmp( radio_set1[i], "$" ) )
                    break;
            }

            if ( menu == 2 )
            {
                if ( !str_cmp( radio_set2[i], "$" ) )
                    break;
            }

            if ( menu == 3 )
            {
                if ( !str_cmp( radio_set3[i], "$" ) )
                    break;
            }

            if ( menu == 4 )
            {
                if ( !str_cmp( radio_set4[i], "$" ) )
                    break;
            }

            if ( i == opt )
            {
                if ( menu == 1 )
                    do_radio( ch, radio_set1[i] );

                if ( menu == 2 )
                    do_radio( ch, radio_set2[i] );

                if ( menu == 3 )
                    do_radio( ch, radio_set3[i] );

                if ( menu == 4 )
                    do_radio( ch, radio_set4[i] );

                return;
            }
        }
    }

    quick_radio( ch, menu, 0 );
    return;
}

void do_radio1( CHAR_DATA* ch, char* argument )
{
    int tmp = atoi( argument );
    quick_radio( ch, 1, tmp );
}

void do_radio2( CHAR_DATA* ch, char* argument )
{
    int tmp = atoi( argument );
    quick_radio( ch, 2, tmp );
}

void do_radio3( CHAR_DATA* ch, char* argument )
{
    int tmp = atoi( argument );
    quick_radio( ch, 3, tmp );
}

void do_radio4( CHAR_DATA* ch, char* argument )
{
    int tmp = atoi( argument );
    quick_radio( ch, 4, tmp );
}

void do_ignore( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char newbuf[MAX_STRING_LENGTH];
    char* tmp;
    bool set = FALSE;

    // argument = one_argument( argument, arg );

    if ( IS_NPC( ch ) )
        return;

    if ( !ch->pcdata )
        return;

    strcpy( newbuf, "" );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: IGNORE (player name)\n\r", ch );
        send_to_char( " Using ignore will toggle the player on your list.\n\r", ch );
        return;
    }

    if ( !ch->pcdata->ignore )
        ch->pcdata->ignore = str_dup( "" );

    /* Check if its already in the list */
    for ( tmp = ch->pcdata->ignore ; ; )
    {
        tmp = one_argument( tmp, arg );

        if ( arg[0] == '\0' )
            break;

        if ( is_name( argument, arg ) )
        {
            /* Found. Remove from list */
            ch_printf( ch, "&w&RYou are no longer ignoring %s.\n\r", argument );
            set = TRUE;
        }
        else
        {
            strcat( newbuf, arg );
            strcat( newbuf, " " );
        }
    }

    if ( set )
    {
        DISPOSE( ch->pcdata->ignore );
        ch->pcdata->ignore = str_dup( newbuf );
    }
    else
    {
        sprintf( buf, "%s %s", ch->pcdata->ignore, argument );
        DISPOSE( ch->pcdata->ignore );
        ch->pcdata->ignore = str_dup( buf );
        ch_printf( ch, "&w&RYou are now ignoring %s.\n\r", argument );
    }

    return;
}

void update_swarm( void )
{
    DESCRIPTOR_DATA* d;
    CHAR_DATA* ch;

    /* Only players need to be updated, so skim descriptors */
    for ( d = first_descriptor; d; d = d->next )
    {
        ch = d->original ? d->original : d->character;

        if ( ch == NULL )
            continue;

        if ( !ch->in_room )
            continue;

        if ( is_spectator( ch ) )
            continue;

        if ( d->connected == CON_PLAYING && ch->race == RACE_ALIEN )
        {
            ch->swarm = count_friends( ch );

            if ( ch->swarm < SWARM_CNT )
            {
                ch->swarm = 0;
            }
            else
            {
                ch->swarm = UMIN( ch->swarm - SWARM_CNT, MAX_SWARM );
            }
        }
    }
}

int count_friends( CHAR_DATA* ch )
{
    EXIT_DATA* pexit = NULL;
    int count = 0;

    if ( !ch )
        return 0;

    if ( !ch->in_room )
        return 0;

    count = count + _count_friends( ch->in_room, ch );

    for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
    {
        if ( pexit->to_room && pexit->to_room != ch->in_room )
        {
            count = count + _count_friends( pexit->to_room, ch );
        }
    }

    return count;
}

int _count_friends( ROOM_INDEX_DATA* room, CHAR_DATA* ch )
{
    CHAR_DATA* rch = NULL;
    int count = 0;
    int race = 0;

    if ( !ch )
        return 0;

    race = ch->race;

    for ( rch = room->first_person; rch; rch = rch->next_in_room )
    {
        if ( rch == ch )
            continue;

        if ( is_spectator( rch ) || IN_VENT( rch ) )
            continue;

        if ( rch->race == race )
            count++;
    }

    return count;
}

