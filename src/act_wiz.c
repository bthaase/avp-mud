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
               Wizard/god command module
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "mud.h"

#define RESTORE_INTERVAL 21600

char* const save_flag[] =
{
    "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
    "get", "receive", "idle", "backup", "r13", "r14", "r15", "r16",
    "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",
    "r28", "r29", "r30", "r31"
};


/* from comm.c */
bool    write_to_descriptor args( ( int desc, char* txt, int length ) );

/*
    Local functions.
*/
ROOM_INDEX_DATA* find_location args( ( CHAR_DATA* ch, char* arg ) );
void              save_banlist  args( ( void ) );
void              close_area    args( ( AREA_DATA* pArea ) );

int               get_color ( char* argument ); /* function proto */
void              lstat_keys    args( ( CHAR_DATA* ch ) );

/*
    Global variables.
*/
bool pk_allow = TRUE;
char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA* obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA* mob_index_hash[MAX_KEY_HASH];
// extern bool MOBtrigger;

void do_allowpk( CHAR_DATA* ch, char* argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( pk_allow )
    {
        send_to_char( "PKing disallowed.\n\r", ch );
        echo_to_all( AT_RED, "PKing is now disabled.", ECHOTAR_ALL );
        pk_allow = FALSE;
    }
    else
    {
        send_to_char( "PKing allowed.\n\r", ch );
        echo_to_all( AT_RED, "PKing is now enabled.", ECHOTAR_ALL );
        pk_allow = TRUE;
    }
}

int get_saveflag( char* name )
{
    int x;

    for ( x = 0; x < sizeof( save_flag ) / sizeof( save_flag[0] ); x++ )
        if ( !str_cmp( name, save_flag[x] ) )
            return x;

    return -1;
}

/*
    Toggle "Do Not Disturb" flag. Used to prevent lower level imms from
    using commands like "trans" and "goto" on higher level imms.
*/
void do_dnd( CHAR_DATA* ch, char* argument )
{
    if ( !IS_NPC( ch ) && ch->pcdata )
        if ( xIS_SET( ch->pcdata->flags, PCFLAG_DND ) )
        {
            xREMOVE_BIT( ch->pcdata->flags, PCFLAG_DND );
            send_to_char( "Your 'do not disturb' flag is now off.\n\r", ch );
        }
        else
        {
            xSET_BIT( ch->pcdata->flags, PCFLAG_DND );
            send_to_char( "Your 'do not disturb' flag is now on.\n\r", ch );
        }
    else
        send_to_char( "huh?\n\r", ch );
}

void do_wizhelp( CHAR_DATA* ch, char* argument )
{
    CMDTYPE* cmd;
    int col, hash, level, title, slevel;
    slevel = get_trust( ch );

    for ( level = slevel; level > 100 ; level-- )
    {
        /* Skips over the 106-199 Range */
        if ( level > 105 && level < 200 )
            level = 105;

        col = 0;
        title = 0;

        for ( hash = 0; hash < 126; hash++ )
            for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
                if ( cmd->level == level )
                {
                    if ( title == 0 )
                    {
                        ++title;
                        ch_printf( ch, "&z--&CLevel %d&z-----------------------------------------------------\n\r", level );
                    }

                    ch_printf( ch, "&B%-15s", cmd->name );

                    if ( ++col % 5 == 0 )
                        send_to_pager( "\n\r", ch );
                }

        if ( col % 5 != 0 )
            send_to_pager( "\n\r", ch );
    }

    return;
}

void do_restrict( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    sh_int level, hash;
    CMDTYPE* cmd;
    bool found;
    found = FALSE;
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Restrict which command?\n\r", ch );
        return;
    }

    argument = one_argument ( argument, arg2 );

    if ( arg2[0] == '\0' )
        level = get_trust( ch );
    else
        level = atoi( arg2 );

    level = UMAX( UMIN( get_trust( ch ), level ), 0 );
    hash = arg[0] % 126;

    for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
    {
        if ( !str_prefix( arg, cmd->name )
                &&    cmd->level <= get_trust( ch ) )
        {
            found = TRUE;
            break;
        }
    }

    if ( found )
    {
        if ( !str_prefix( arg2, "show" ) )
        {
            sprintf( buf, "%s show", cmd->name );
            do_cedit( ch, buf );
            /*          ch_printf( ch, "%s is at level %d.\n\r", cmd->name, cmd->level );*/
            return;
        }

        cmd->level = level;
        ch_printf( ch, "You restrict %s to level %d\n\r",
                   cmd->name, level );
        sprintf( buf, "%s restricting %s to level %d",
                 ch->name, cmd->name, level );
        log_string( buf );
    }
    else
        send_to_char( "You may not restrict that command.\n\r", ch );

    return;
}

/*
    Check if the name prefix uniquely identifies a char descriptor
*/
CHAR_DATA* get_waiting_desc( CHAR_DATA* ch, char* name )
{
    DESCRIPTOR_DATA* d;
    CHAR_DATA*       ret_char;
    static unsigned int number_of_hits;
    number_of_hits = 0;

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( d->character && ( !str_prefix( name, d->character->name ) ) &&
                IS_WAITING_FOR_AUTH( d->character ) )
        {
            if ( ++number_of_hits > 1 )
            {
                ch_printf( ch, "%s does not uniquely identify a char.\n\r", name );
                return NULL;
            }

            ret_char = d->character;       /* return current char on exit */
        }
    }

    if ( number_of_hits == 1 )
        return ret_char;
    else
    {
        send_to_char( "No one like that waiting for authorization.\n\r", ch );
        return NULL;
    }
}

void do_authorize( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* victim;
    DESCRIPTOR_DATA* d;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Usage:  authorize <player> <yes|name|immsim|mobsim|swear|plain|no/deny>\n\r", ch );
        send_to_char( "Pending authorizations:\n\r", ch );
        send_to_char( " Chosen Character Name\n\r", ch );
        send_to_char( "---------------------------------------------\n\r", ch );

        for ( d = first_descriptor; d; d = d->next )
            if ( ( victim = d->character ) != NULL && IS_WAITING_FOR_AUTH( victim ) )
                ch_printf( ch, " %s@%s new %s...\n\r",
                           victim->name,
                           victim->desc->host,
                           race_table[victim->race].race_name );

        return;
    }

    victim = get_waiting_desc( ch, arg1 );

    if ( victim == NULL )
        return;

    if ( arg2[0] == '\0' || !str_cmp( arg2, "accept" ) || !str_cmp( arg2, "yes" ) )
    {
        victim->pcdata->auth_state = 3;
        xREMOVE_BIT( victim->pcdata->flags, PCFLAG_UNAUTHED );

        if ( victim->pcdata->authed_by )
            STRFREE( victim->pcdata->authed_by );

        victim->pcdata->authed_by = QUICKLINK( ch->name );
        sprintf( buf, "%s authorized %s", ch->name,
                 victim->name );
        to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
        ch_printf( ch, "You have authorized %s.\n\r", victim->name );
        /* Below sends a message to player when name is accepted - Brittany   */
        ch_printf( victim,
                   "The MUD Administrators have accepted the name %s.\n\r"
                   "You are now fully authorized to play Star Wars: Legends of the Jedi.\n\r", victim->name );
        return;
    }
    else if ( !str_cmp( arg2, "immsim" ) || !str_cmp( arg2, "i" ) )
    {
        victim->pcdata->auth_state = 2;
        sprintf( buf, "%s: name denied - similar to Imm name", victim->name );
        to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
        ch_printf( victim,
                   "The name you have chosen is too similar to that of a current immortal.\n\r"
                   "We ask you to please choose another name using the name command.\n\r" );
        ch_printf( ch, "You requested %s change names.\n\r", victim->name );
        return;
    }
    else if ( !str_cmp( arg2, "mobsim" ) || !str_cmp( arg2, "m" ) )
    {
        victim->pcdata->auth_state = 2;
        sprintf( buf, "%s: name denied - similar to mob name", victim->name );
        to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
        ch_printf( victim,
                   "The name you have chosen is too similar to that of certain\n\r"
                   "monsters in the game, in the long run this could cause problems\n\r"
                   "and therefore we are unable to authorize them.  Please choose\n\r"
                   "another name using the name command.\n\r" );
        ch_printf( ch, "You requested %s change names.\n\r", victim->name );
        return;
    }
    else if ( !str_cmp( arg2, "swear" ) || !str_cmp( arg2, "s" ) )
    {
        victim->pcdata->auth_state = 2;
        sprintf( buf, "%s: name denied - swear word", victim->name );
        to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
        ch_printf( victim,
                   "We will not authorize names containing swear words, in any language.\n\r"
                   "Please choose another name using the name command.\n\r" );
        ch_printf( ch, "You requested %s change names.\n\r", victim->name );
        return;
    }
    else if ( !str_cmp( arg2, "plain" ) || !str_cmp( arg2, "p" ) )
    {
        victim->pcdata->auth_state = 2;
        sprintf( buf, "%s: name denied", victim->name );
        to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
        ch_printf( victim,
                   "We would ask you to please attempt to choose a name that is more\n\r"
                   "Star Wars in nature.  Please choose another name using the name\n\r"
                   "command.\n\r" );
        ch_printf( ch, "You requested %s change names.\n\r", victim->name );
        return;
    }
    else if ( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
    {
        send_to_char( "You have been denied access.\n\r", victim );
        sprintf( buf, "%s denied authorization to %s", ch->name,
                 victim->name );
        to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
        ch_printf( ch, "You have denied %s.\n\r", victim->name );
        do_quit( victim, "" );
    }
    else if ( !str_cmp( arg2, "name" ) || !str_cmp( arg2, "n" ) )
    {
        sprintf( buf, "%s has denied %s's name", ch->name,
                 victim->name );
        to_channel( buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
        ch_printf ( victim,
                    "The MUD Administrators have found the name %s "
                    "to be unacceptable.\n\r"
                    "Use 'name' to change it to something more apropriate.\n\r", victim->name );
        ch_printf( ch, "You requested %s change names.\n\r", victim->name );
        victim->pcdata->auth_state = 2;
        return;
    }
    else
    {
        send_to_char( "Invalid argument.\n\r", ch );
        return;
    }
}

void do_bamfin( CHAR_DATA* ch, char* argument )
{
    if ( !IS_NPC( ch ) )
    {
        smash_tilde( argument );
        DISPOSE( ch->pcdata->bamfin );
        ch->pcdata->bamfin = str_dup( argument );
        send_to_char( "Ok.\n\r", ch );
    }

    return;
}



void do_bamfout( CHAR_DATA* ch, char* argument )
{
    if ( !IS_NPC( ch ) )
    {
        smash_tilde( argument );
        DISPOSE( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
        send_to_char( "Ok.\n\r", ch );
    }

    return;
}

/*
         New MUD Statistics ;)
    ... The main interface command is LSTAT ...
             -=# Added by Ghost #=-
*/
void do_lstat( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || arg == NULL )
    {
        send_to_char( "&RSyntax: LSTAT <Statistic>\n\r", ch );
        send_to_char( "&R - Keys, OAverage, MAverage, Files\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "keys" ) )
    {
        lstat_keys( ch );
    }
    else if ( !str_cmp( arg, "oaverage" ) )
    {
    }
    else if ( !str_cmp( arg, "maverage" ) )
    {
    }
    else if ( !str_cmp( arg, "files" ) )
    {
        char buf2[MAX_STRING_LENGTH];
        char buf3[MAX_STRING_LENGTH];
        char buf4[MAX_STRING_LENGTH];
        char buf5[MAX_STRING_LENGTH];
        sprintf( buf, "%s%s", SYSTEM_DIR, BUG_FILE );
        sprintf( buf2, "%s%s", SYSTEM_DIR, LOG_FILE );
        sprintf( buf3, "%s%s", LOG_DIR, "immortal.log" ); /* Reserved */
        sprintf( buf4, "%s%s", LOG_DIR, "connect.log" );
        sprintf( buf5, "%s%s", LOG_DIR, "help.log" );
        ch_printf( ch, "&zFile size tracking:\n\r" );
        ch_printf( ch, "&z-------------------------------------------------------\n\r" );
        ch_printf( ch, "&W Bug list     (bugs.txt)     - &C%d bytes\n\r", file_size( buf ) );
        ch_printf( ch, "&W Speech Log   (log.txt)      - &C%d bytes\n\r", file_size( buf2 ) );
        ch_printf( ch, "&W Immortal Log (immortal.log) - &C%d bytes\n\r", file_size( buf3 ) );
        ch_printf( ch, "&W Connect Log  (connect.log)  - &C%d bytes\n\r", file_size( buf4 ) );
        ch_printf( ch, "&W Help Log     (help.log)     - &C%d bytes\n\r", file_size( buf5 ) );
        return;
    }
    else
    {
        send_to_char( "&RInvalid Statistic, Try again!\n\r", ch );
        do_lstat( ch, "" );
    }

    return;
}

void lstat_keys( CHAR_DATA* ch )
{
    int count = 0;
    OBJ_DATA* obj;
    send_to_char( "\n\r&YName:           Vnum:       Lock ID:  \n\r", ch );
    set_char_color( AT_BLUE, ch );

    for ( obj = first_object; obj; obj = obj->next )
    {
        if ( obj->item_type == ITEM_KEY )
        {
            if ( obj->value[0] > 0 )
            {
                ch_printf( ch, "%-15s %-12ld %-12ld\n\r", obj->short_descr, obj->pIndexData->vnum, obj->value[0] );
                count++;
            }
        }

        if ( count >= 300 )
            break;
    }

    if ( count < 300 )
        ch_printf( ch, "Objects counted: %ld\n\r", count );
    else
        ch_printf( ch, "%ld Object counted. Limit break tripped.\n\r", count );

    return;
}

void do_rank( CHAR_DATA* ch, char* argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage: rank <string>.\n\r", ch );
        send_to_char( "   or: rank none.\n\r", ch );
        return;
    }

    if ( checkclr( argument, "x" ) )
    {
        send_to_char( "&RYou cannot use the BLACK Color token in your rank.\n\r", ch );
        return;
    }

    if ( nifty_is_name( "^", argument ) )
    {
        send_to_char( "&RYou cannot use the background/blinking tokens in your rank.\n\r", ch );
        return;
    }

    smash_tilde( argument );
    strcat( argument, "&w" );
    DISPOSE( ch->pcdata->rank );

    if ( !str_cmp( argument, "none" ) )
        ch->pcdata->rank = str_dup( "" );
    else
        ch->pcdata->rank = str_dup( argument );

    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_retire( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Retire whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( victim->top_level < LEVEL_SAVIOR )
    {
        send_to_char( "The minimum level for retirement is savior.\n\r", ch );
        return;
    }

    if ( IS_RETIRED( victim ) )
    {
        xREMOVE_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
        ch_printf( ch, "%s returns from retirement.\n\r", victim->name );
        ch_printf( victim, "%s brings you back from retirement.\n\r", ch->name );
    }
    else
    {
        xSET_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
        ch_printf( ch, "%s is now a retired immortal.\n\r", victim->name );
        ch_printf( victim, "Courtesy of %s, you are now a retired immortal.\n\r", ch->name );
    }

    return;
}

void do_delay( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;
    char arg[MAX_INPUT_LENGTH];
    int delay;
    set_char_color( AT_IMMORT, ch );
    argument = one_argument( argument, arg );

    if ( !*arg )
    {
        send_to_char( "Syntax:  delay <victim> <# of rounds>\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world_full( ch, arg ) ) )
    {
        send_to_char( "No such character online.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Mobiles are unaffected by lag.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You haven't the power to succeed against them.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( !*arg )
    {
        send_to_char( "For how long do you wish to delay them?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "none" ) )
    {
        send_to_char( "All character delay removed.\n\r", ch );
        victim->wait = 0;
        return;
    }

    delay = atoi( arg );

    if ( delay < 1 )
    {
        send_to_char( "Pointless.  Try a positive number.\n\r", ch );
        return;
    }

    if ( delay > 999 )
    {
        send_to_char( "You cruel bastard.  Just kill them.\n\r", ch );
        return;
    }

    WAIT_STATE( victim, delay * PULSE_VIOLENCE );
    ch_printf( ch, "You've delayed %s for %d rounds.\n\r", victim->name, delay );
    return;
}

void do_deny( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Deny whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    xSET_BIT( victim->act, PLR_DENY );
    send_to_char( "You are denied access!\n\r", victim );
    send_to_char( "OK.\n\r", ch );
    do_quit( victim, "" );
    return;
}



void do_disconnect( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA* d;
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Disconnect whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->desc == NULL )
    {
        act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( get_trust( ch ) <= get_trust( victim ) )
    {
        send_to_char( "They might not like that...\n\r", ch );
        return;
    }

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( d == victim->desc )
        {
            close_socket( d, FALSE );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
    }

    bug( "Do_disconnect: *** desc not found ***.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

/*
    Force a level one player to quit.             Gorog
*/
void do_fquit( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;
    char arg1[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Force whom to quit?\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world_full( ch, arg1 ) ) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->top_level != 1 )
    {
        send_to_char( "They are not level one!\n\r", ch );
        return;
    }

    send_to_char( "The MUD administrators force you to quit\n\r", victim );
    do_quit ( victim, "" );
    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_forceclose( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA* d;
    int desc;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Usage: forceclose <descriptor#>\n\r", ch );
        return;
    }

    desc = atoi( arg );

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( d->descriptor == desc )
        {
            if ( d->character && get_trust( d->character ) >= get_trust( ch ) )
            {
                send_to_char( "They might not like that...\n\r", ch );
                return;
            }

            close_socket( d, FALSE );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
    }

    send_to_char( "Not found!\n\r", ch );
    return;
}



void do_pardon( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: pardon <character> <planet>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    send_to_char( "Syntax: pardon <character> <planet>.... But it doesn't work .... Tell Durga to hurry up and finish it :p\n\r", ch );
    return;
}


void echo_to_all( sh_int AT_COLOR, char* argument, sh_int tar )
{
    DESCRIPTOR_DATA* d;

    if ( !argument || argument[0] == '\0' )
        return;

    for ( d = first_descriptor; d; d = d->next )
    {
        /*  Added showing echoes to players who are editing, so they won't
            miss out on important info like upcoming reboots. --Narn */
        if ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
        {
            /* This one is kinda useless except for switched.. */
            if ( tar == ECHOTAR_PC && IS_NPC( d->character ) )
                continue;
            else if ( tar == ECHOTAR_IMM && !IS_IMMORTAL( d->character ) )
                continue;

            set_char_color( AT_COLOR, d->character );
            send_to_char( argument, d->character );
            send_to_char( "\n\r",   d->character );
        }
    }

    return;
}

void do_echo( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int color;
    int target;
    char* parg;

    if ( xIS_SET( ch->act, PLR_NO_EMOTE ) )
    {
        send_to_char( "You are noemoted and can not echo.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Echo what?\n\r", ch );
        return;
    }

    if ( ( color = get_color( argument ) ) )
        argument = one_argument( argument, arg );

    parg = argument;
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "PC" )
            ||   !str_cmp( arg, "player" ) )
        target = ECHOTAR_PC;
    else if ( !str_cmp( arg, "imm" ) )
        target = ECHOTAR_IMM;
    else
    {
        target = ECHOTAR_ALL;
        argument = parg;
    }

    if ( !color && ( color = get_color( argument ) ) )
        argument = one_argument( argument, arg );

    if ( !color )
        color = AT_IMMORT;

    one_argument( argument, arg );

    if ( !str_cmp( arg, "Merth" )
            ||   !str_cmp( arg, "Durga" ) )
    {
        ch_printf( ch, "I don't think %s would like that!\n\r", arg );
        return;
    }

    echo_to_all ( color, argument, target );
}

void echo_to_room( sh_int AT_COLOR, ROOM_INDEX_DATA* room, char* argument )
{
    CHAR_DATA* vic;

    if ( room == NULL )
        return;

    for ( vic = room->first_person; vic; vic = vic->next_in_room )
    {
        if ( AT_COLOR > -1 )
            set_char_color( AT_COLOR, vic );

        send_to_char( argument, vic );
        send_to_char( "\n\r",   vic );
    }
}

void do_recho( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int color;

    if ( xIS_SET( ch->act, PLR_NO_EMOTE ) )
    {
        send_to_char( "You are noemoted and can not recho.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Recho what?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( ( color = get_color ( argument ) ) )
    {
        argument = one_argument ( argument, arg );
        echo_to_room ( color, ch->in_room, argument );
    }
    else
        echo_to_room ( AT_IMMORT, ch->in_room, argument );
}


ROOM_INDEX_DATA* find_location( CHAR_DATA* ch, char* arg )
{
    CHAR_DATA* victim;
    OBJ_DATA* obj;

    if ( is_number( arg ) )
        return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world_full( ch, arg ) ) != NULL )
        return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
        return obj->in_room;

    return NULL;
}



void do_transfer( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* location;
    DESCRIPTOR_DATA* d;
    CHAR_DATA* victim;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Transfer whom (and where)?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
        for ( d = first_descriptor; d; d = d->next )
        {
            if ( d->connected == CON_PLAYING
                    &&   d->character != ch
                    &&   d->character->in_room
                    &&   d->newstate != 2
                    &&   can_see( ch, d->character ) )
            {
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "%s %s", d->character->name, arg2 );
                do_transfer( ch, buf );
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
            send_to_char( "No such location.\n\r", ch );
            return;
        }
    }

    if ( ( victim = get_char_world_full( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( NOT_AUTHED( victim ) )
    {
        send_to_char( "They are not authorized yet!\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && get_trust( ch ) < 105 )
    {
        ch_printf( ch, "Due to repeated abuse, builders can no longer transfer mortals.\n\r" );
        return;
    }

    if ( !victim->in_room )
    {
        send_to_char( "They are in limbo.\n\r", ch );
        return;
    }

    /* modification to prevent a low level imm from transferring a */
    /* higher level imm with the DND flag on.  - Gorog             */
    if ( !IS_NPC( victim ) && get_trust( ch ) < get_trust( victim )
            &&   victim->desc
            &&   ( victim->desc->connected == CON_PLAYING
                   ||    victim->desc->connected == CON_EDITING )
            &&   xIS_SET( victim->pcdata->flags, PCFLAG_DND ) )
    {
        pager_printf( ch,
                      "Sorry. %s does not wish to be disturbed.\n\r", victim->name );
        pager_printf( victim,
                      "Your DND flag just foiled %s's transfer command.\n\r", ch->name );
        return;
    }

    /* end of modification                                         */
    act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
    victim->retran = victim->in_room->vnum;
    char_from_room( victim );
    char_to_room( victim, location );
    act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );

    if ( ch != victim )
        act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );

    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}

void do_retran( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    DESCRIPTOR_DATA* d;
    char buf[MAX_STRING_LENGTH];
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Retransfer whom?\n\r", ch );
        return;
    }

    if ( !( str_cmp( arg, "all" ) ) )
    {
        for ( d = first_descriptor; d; d = d->next )
        {
            if ( d->connected != CON_PLAYING )
                continue;

            if ( !d->character )
                continue;

            if ( !d->character->in_room )
                continue;

            if ( !d->character->retran )
                continue ;

            if ( get_trust( ch ) < get_trust( d->character ) )
                continue;

            victim = d->character;
            sprintf( buf, "'%s' %d", victim->name, victim->retran );
            do_transfer( ch, buf );
        }
    }
    else if ( !( victim = get_char_world_full( ch, arg ) ) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    sprintf( buf, "'%s' %d", victim->name, victim->retran );
    do_transfer( ch, buf );
    return;
}

void do_regoto( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "%d", ch->regoto );
    do_goto( ch, buf );
    return;
}

/*  Added do_at and do_atobj to reduce lag associated with at
    --Shaddai
*/
void do_at( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* location = NULL;
    ROOM_INDEX_DATA* original;
    CHAR_DATA* wch = NULL;
    set_char_color( AT_IMMORT, ch );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "At where what?\n\r", ch );
        return;
    }

    if ( is_number( arg ) )
        location = get_room_index( atoi( arg ) );
    else if ( ( wch = get_char_world_full( ch, arg ) ) == NULL || wch->in_room == NULL )
    {
        send_to_char( "No such mobile or player in existance.\n\r", ch );
        return;
    }

    if ( !location && wch )
        location = wch->in_room;

    if ( !location )
    {
        send_to_char( "No such location exists.\n\r", ch );
        return;
    }

    /* The following mod is used to prevent players from using the */
    /* at command on a higher level immortal who has a DND flag    */
    if ( wch && !IS_NPC( wch )
            &&   xIS_SET( wch->pcdata->flags, PCFLAG_DND )
            &&   get_trust( ch ) < get_trust( wch ) )
    {
        pager_printf( ch,
                      "Sorry. %s does not wish to be disturbed.\n\r",
                      wch->name );
        pager_printf( wch,
                      "Your DND flag just foiled %s's at command.\n\r", ch->name );
        return;
    }

    /* End of modification  -- Gorog */
    // set_char_color( AT_PLAIN, ch );
    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument, FALSE );

    if ( !char_died( ch ) )
    {
        char_from_room( ch );
        char_to_room( ch, original );
    }

    return;
}

/*  void do_at( CHAR_DATA *ch, char *argument )
    {
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
    send_to_char( "At where what?\n\r", ch );
    return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
    send_to_char( "No such location.\n\r", ch );
    return;
    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument, FALSE );


       See if 'ch' still exists before continuing!
       Handles 'at XXXX quit' case.

    for ( wch = first_char; wch; wch = wch->next )
    {
    if ( wch == ch )
    {
        char_from_room( ch );
        char_to_room( ch, original );
        break;
    }
    }

    return;
    }*/

void do_atobj( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* location;
    ROOM_INDEX_DATA* original;
    OBJ_DATA* obj;
    /*    CHAR_DATA *victim;*/
    set_char_color( AT_IMMORT, ch );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "At where what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL
            || !obj->in_room )
    {
        send_to_char( "No such object in existance.\n\r", ch );
        return;
    }

    location = obj->in_room;
    set_char_color( AT_PLAIN, ch );
    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument, FALSE );

    if ( !char_died( ch ) )
    {
        char_from_room( ch );
        char_to_room( ch, original );
    }

    return;
}

void do_rat( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* location;
    ROOM_INDEX_DATA* original;
    int Start, End, vnum;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Syntax: rat <start> <end> <command>\n\r", ch );
        return;
    }

    Start = atoi( arg1 );
    End = atoi( arg2 );

    if ( Start < 1 || End < Start || Start > End || Start == End || End > MAX_VNUMS )
    {
        send_to_char( "Invalid range.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < 105 )
    {
        AREA_DATA* pArea;

        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            send_to_char( "You must have an assigned area to RAT.\n\r", ch );
            return;
        }

        if ( Start < pArea->low_r_vnum || End > pArea->hi_r_vnum )
        {
            send_to_char( "You can only RAT within your assigned range.\n\r", ch );
            return;
        }
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
    send_to_char( "Done.\n\r", ch );
    return;
}


void do_rstat( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* location;
    OBJ_DATA* obj;
    CHAR_DATA* rch;
    EXIT_DATA* pexit;
    int cnt;
    static char* dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };
    one_argument( argument, arg );

    if ( get_trust( ch ) < LEVEL_IMMORTAL )
    {
        AREA_DATA* pArea;

        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            send_to_char( "You must have an assigned area to goto.\n\r", ch );
            return;
        }

        if ( ch->in_room->vnum < pArea->low_r_vnum
                ||  ch->in_room->vnum > pArea->hi_r_vnum )
        {
            send_to_char( "You can only rstat within your assigned range.\n\r", ch );
            return;
        }
    }

    if ( !str_cmp( arg, "exits" ) )
    {
        location = ch->in_room;
        ch_printf( ch, "Exits for room '%s.' vnum %d\n\r",
                   location->name,
                   location->vnum );

        for ( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
            ch_printf( ch,
                       "%2d) %2s to %-5d.  Key: %d  Flags: %d  Keywords: '%s'.\n\rDescription: %s Exit links back to vnum: %d  Exit's RoomVnum: %d  Distance: %d\n\r",
                       ++cnt,
                       dir_text[pexit->vdir],
                       pexit->to_room ? pexit->to_room->vnum : 0,
                       pexit->key,
                       pexit->exit_info,
                       pexit->keyword,
                       pexit->description[0] != '\0'
                       ? pexit->description : "(none).\n\r",
                       pexit->rexit ? pexit->rexit->vnum : 0,
                       pexit->rvnum,
                       pexit->distance );

        return;
    }

    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );

    if ( !location )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    ch_printf( ch, "&zName: &C%s.\n\r&zArea: &C%s\n\r&zFilename: &C%s.\n\r",
               location->name,
               location->area ? location->area->name : "(None)",
               location->area ? location->area->filename : "(None)" );
    ch_printf( ch,
               "&zVnum: &B%d.  &zCoords: (&W%d/%d/%d&z)\n\r&zSector: &C%d.  &zLight: &C%d.  &zTeleDelay: &C%d.  &zTeleVnum: &C%d  &zTunnel: &C%d.\n\r",
               location->vnum, location->x, location->y, location->z,
               location->sector_type,
               location->light,
               location->tele_delay,
               location->tele_vnum,
               location->tunnel );
    ch_printf( ch, "&zRoom flags: &B%s\n\r", flag_string( &location->room_flags, r_flags, MAX_ROOM_FLAGS ) );
    ch_printf( ch, "&zHived Room flags: &B%s\n\r", flag_string( &location->hroom_flags, r_flags, MAX_ROOM_FLAGS ) );
    send_to_char( "&zCharacters:&B", ch );

    for ( rch = location->first_person; rch; rch = rch->next_in_room )
    {
        if ( can_see( ch, rch ) )
        {
            send_to_char( " ", ch );
            one_argument( rch->name, buf );
            send_to_char( buf, ch );
        }
    }

    send_to_char( "\n\r&zObjects:&B", ch );

    for ( obj = location->first_content; obj; obj = obj->next_content )
    {
        send_to_char( " ", ch );
        one_argument( obj->name, buf );
        send_to_char( buf, ch );
    }

    send_to_char( "\n\r", ch );
    ch_printf( ch, "&zDescription:\n\r&W%s", location->description );
    ch_printf( ch, "&zHived Description:\n\r&W%s", location->hdescription );

    if ( location->first_extradesc )
    {
        EXTRA_DESCR_DATA* ed;
        send_to_char( "&zExtra description keywords: '&C", ch );

        for ( ed = location->first_extradesc; ed; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );

            if ( ed->next )
                send_to_char( " ", ch );
        }

        send_to_char( "&z'.\n\r", ch );
    }

    if ( location->first_exit )
        send_to_char( "&z------------------- EXITS -------------------\n\r", ch );

    for ( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
        ch_printf( ch, "&B%2d&z) &B%2s &zto &C%-5d  &zKey: &C%d  &zKeywords: &C%s\n\r",
                   ++cnt,
                   dir_text[pexit->vdir],
                   pexit->to_room ? pexit->to_room->vnum : 0,
                   pexit->key,
                   pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );

    return;
}



void do_ostat( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA* paf;
    OBJ_DATA* obj;
    char* pdesc;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Ostat what?\n\r", ch );
        return;
    }

    if ( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
        strcpy( arg, argument );

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
        return;
    }

    ch_printf( ch, "&zName: &G%s.&z\n\r", obj->name );
    pdesc = get_extra_descr( arg, obj->first_extradesc );

    if ( !pdesc )
        pdesc = get_extra_descr( arg, obj->pIndexData->first_extradesc );

    if ( !pdesc )
        pdesc = get_extra_descr( obj->name, obj->first_extradesc );

    if ( !pdesc )
        pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );

    if ( pdesc )
        send_to_char( pdesc, ch );

    ch_printf( ch, "&zVnum: &C%d.  &zType: &C%s.  &zCount: &C%d  &zGcount: &C%d\n\r",
               obj->pIndexData->vnum, item_type_name( obj ), obj->pIndexData->count,
               obj->count );
    ch_printf( ch, "&zSerial#: &C%d  &zTopIdxSerial#: &C%d  &zTopSerial#: &C%d\n\r",
               obj->serial, obj->pIndexData->serial, cur_obj_serial );
    ch_printf( ch, "&zShort description: &C%s.\n\r&zLong description: &C%s\n\r",
               obj->short_descr, obj->description );

    if ( obj->action_desc[0] != '\0' )
        ch_printf( ch, "&zAction description: &C%s.\n\r", obj->action_desc );

    ch_printf( ch, "&zWear flags :  &C%s\n\r", flag_string( &obj->wear_flags, w_flags, MAX_WEAR ) );
    ch_printf( ch, "&zExtra flags:  &C%s\n\r", flag_string( &obj->extra_flags, o_flags, MAX_ITEM_FLAG ) );

    if ( obj->killed_by )
        ch_printf( ch, "&zKilled by: &C%s\n\r", obj->killed_by );

    ch_printf( ch, "&zNumber: &C%d&z/&C%d&z.  Weight: &C%d&z/&C%d&z.  Layers: &C%d&z\n\r", 1, get_obj_number( obj ),
               obj->weight, get_obj_weight( obj ), obj->pIndexData->layers );
    ch_printf( ch, "&zCost: &C%d&z.  Rent: &C%d&z.  Timer: &C%d&z.  Level: &C%d&z.\n\r",
               obj->cost, obj->pIndexData->rent, obj->timer, obj->level );
    ch_printf( ch, "&zIn room: &C%d&z.  In object: &C%s&z.  Carried by: &C%s&z.  Wear_loc: &C%d&z.\n\r",
               obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
               obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
               obj->carried_by == NULL    ? "(none)" : obj->carried_by->name,
               obj->wear_loc );
    ch_printf( ch, "&zIndex Values : &C%d %d %d %d %d %d&z.\n\r",
               obj->pIndexData->value[0], obj->pIndexData->value[1],
               obj->pIndexData->value[2], obj->pIndexData->value[3],
               obj->pIndexData->value[4], obj->pIndexData->value[5] );
    ch_printf( ch, "&zObject Values: &C%d %d %d %d %d %d&z.\n\r",
               obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );

    if ( obj->pIndexData->first_extradesc )
    {
        EXTRA_DESCR_DATA* ed;
        send_to_char( "&zPrimary description keywords:   '&C", ch );

        for ( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );

            if ( ed->next )
                send_to_char( " ", ch );
        }

        send_to_char( "&z'.\n\r", ch );
    }

    if ( obj->first_extradesc )
    {
        EXTRA_DESCR_DATA* ed;
        send_to_char( "&zSecondary description keywords: '&C", ch );

        for ( ed = obj->first_extradesc; ed; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );

            if ( ed->next )
                send_to_char( " ", ch );
        }

        send_to_char( "&z'.\n\r", ch );
    }

    for ( paf = obj->first_affect; paf; paf = paf->next )
        ch_printf( ch, "&zAffects &C%s &zby &C%d&z. (extra)\n\r",
                   affect_loc_name( paf->location ), paf->modifier );

    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
        ch_printf( ch, "&zAffects &C%s &zby &C%d&z.\n\r",
                   affect_loc_name( paf->location ), paf->modifier );

    return;
}

void do_mstat( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA* paf;
    CHAR_DATA* victim;
    SKILLTYPE* skill;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "&YMstat whom?\n\r", ch );
        return;
    }

    if ( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
        strcpy( arg, argument );

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "&YThey aren't here.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
    {
        send_to_char( "&YTheir godly glow prevents you from getting a good look.\n\r", ch );
        ch_printf( victim, "&R%s has attempted to MSTAT you!\n\r", ch->name );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        ch_printf( ch, "&zName: &Y%s\n\r", victim->name );
    }
    else
    {
        ch_printf( ch, "&zName: &G%s\n\r", victim->name );
    }

    /* Allows 105's and up to see mortal passwords. For RP enforcement... */
    if ( get_trust( ch ) >= 105 && !IS_NPC( victim ) && get_trust( victim ) <= get_trust( ch ) )
        ch_printf( ch, "&zPassword: &Y%s\n\r", victim->pcdata->pwd );

    if ( get_trust( ch ) >= LEVEL_GOD && !IS_NPC( victim ) && victim->desc )
        ch_printf( ch, "&zUser: &C%s&z@&C%s   &zDescriptor: &C%d   &zTrust: &Y%d\n\r&zAuthedBy:  &C%s\n\r",
                   victim->desc->user, victim->desc->host, victim->desc->descriptor,
                   victim->trust, victim->pcdata->authed_by[0] != '\0' ? victim->pcdata->authed_by : "(unknown)" );

    if ( !IS_NPC( victim ) && victim->pcdata->release_date != 0 )
        ch_printf( ch, "&RHelled until &C%24.24s &Rby &C%s.\n\r",
                   ctime( &victim->pcdata->release_date ),
                   victim->pcdata->helled_by );

    ch_printf( ch, "&zVnum: &Y%d   &zSex: &C%s   &zRoom: &C%d   &zCount: &C%d &zID: &C%d\n\r",
               IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
               victim->sex == SEX_MALE    ? "male"   : victim->sex == SEX_FEMALE  ? "female" : "neutral",
               victim->in_room == NULL    ?        0 : victim->in_room->vnum,
               IS_NPC( victim ) ? victim->pIndexData->count : 1, IS_NPC( victim ) ? 0 : victim->pcdata->id );
    ch_printf( ch, "&zStr: &C%d  &zSta: &C%d  &zRec: &C%d  &zInt: &C%d  &zBra: &C%d  &zPer: &C%d\n\r",
               get_curr_str( victim ), get_curr_sta( victim ), get_curr_rec( victim ),
               get_curr_int( victim ), get_curr_bra( victim ), get_curr_per( victim ) );
    ch_printf( ch, "&zHps: &C%d&z/&C%d  &zMove: &C%d&z/&C%d  &zXP: &Y%d (%d)\n\r",
               victim->hit, victim->max_hit, victim->move, victim->max_move, victim->currexp, victim->maxexp );
    ch_printf( ch, "&zTop Level: &Y%d     &zRace: &C%d\n\r",
               victim->top_level,  victim->race );

    if (  victim->race  < MAX_NPC_RACE  && victim->race  >= 0 )
        ch_printf( ch, "&zRace: &C%s\n\r", npc_race[victim->race] );

    ch_printf( ch, "&zPosition: &C%d  &zRespawn: &R%d  &zStreak: &C%d\n\r", victim->position, ( victim->pcdata == NULL ) ? 0 : victim->pcdata->respawn, victim->streak );
    ch_printf( ch, "&zMaster: &R%s   &zLeader: &R%s\n\r",
               victim->master      ? victim->master->name   : "(none)",
               victim->leader      ? victim->leader->name   : "(none)" );
    ch_printf( ch, "&zHating: &R%s   &zHunting: &R%s   &zFearing: &R%s\n\r",
               victim->hating      ? victim->hating->name   : "(none)",
               victim->hunting     ? victim->hunting->name  : "(none)",
               victim->fearing     ? victim->fearing->name  : "(none)" );
    ch_printf( ch, "&zCOUNTERS-> &zWarned: &C%d   &zBusy: &C%d  &zAP: &C%d/%d\n\r",
               victim->warned, victim->busy, victim->ap, get_max_ap( victim ) );
    ch_printf( ch, "&zMentalState: &C%d   &zEmotionalState: &C%d\n\r", victim->mental_state, victim->emotional_state );
    ch_printf( ch, "&zCarry figures: items (&C%d&z/&C%d&z)  weight (&C%d&z/&C%d&z)\n\r",
               victim->carry_number, can_carry_n( victim ), victim->carry_weight, can_carry_w( victim ) );
    ch_printf( ch, "&zYears: &C%d   &zSeconds Played: &C%d   &zTimer: &C%d   &zAct: &C%d\n\r",
               get_age( victim ), ( int ) victim->played, victim->timer, victim->act );

    if ( IS_NPC( victim ) )
    {
        ch_printf( ch, "&zAct flags: &C%s\n\r", flag_string( &victim->act, act_flags, MAX_ACT_FLAGS ) );
    }
    else
    {
        ch_printf( ch, "&zPlayer flags: &C%s\n\r", flag_string( &victim->act, plr_flags, MAX_ACT_FLAGS ) );
        ch_printf( ch, "&zPcflags: &C%s\n\r", flag_string( &victim->pcdata->flags, pc_flags, MAX_PC_FLAGS ) );
    }

    ch_printf( ch, "&zAffected by: &C%s\n\r", affect_bit_name( &victim->affected_by ) );
    ch_printf( ch, "&zSpeaks: &C%s\n\r&zSpeaking: &C%s\n\r", flag_string( &victim->speaks, lang_names, LANG_UNKNOWN ), flag_string( &victim->speaking, lang_names, LANG_UNKNOWN ) );

    /*
        send_to_char( "Languages: ", ch );
        for ( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
        {
        xCLEAR_BITS( xbit );
        xSET_BIT( xbit, lang_array[x] );
        if ( knows_language( victim, xbit, victim ) || (IS_NPC(victim) && victim->speaks == 0) )
        {
          if ( xIS_SET(lang_array[x], victim->speaking) || (IS_NPC(victim) && IS_EMPTY(victim->speaking)) )
               set_char_color( AT_RED, ch );
          send_to_char( lang_names[x], ch );
          send_to_char( " ", ch );
          set_char_color( AT_PLAIN, ch );
        }
        else if ( xIS_SET(lang_array[x], victim->speaking) || (IS_NPC(victim) && !victim->speaking) )
        {
          set_char_color( AT_PINK, ch );
          send_to_char( lang_names[x], ch );
          send_to_char( " ", ch );
          set_char_color( AT_PLAIN, ch );
        }
        }
        send_to_char( "\n\r", ch );
    */

    if ( victim->pcdata && victim->pcdata->bestowments && victim->pcdata->bestowments[0] != '\0' )
        ch_printf( ch, "&zBestowments: &C%s\n\r", victim->pcdata->bestowments );

    ch_printf( ch, "&zShort description: &Y%s\n\r&zLong  description: &Y%s",
               victim->short_descr, victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );

    if ( IS_NPC( victim ) )
        ch_printf( ch, "&zMobile is running these specials:\n\r&z &G%-20s &z &G%-20s &z\n\r&z &G%-20s &z &G%-20s &z|\n\r",
                   victim->spec_fun ? lookup_spec( victim->spec_fun ) : "(empty)",
                   victim->spec_2 ? lookup_spec( victim->spec_2 ) : "(empty)",
                   victim->spec_3 ? lookup_spec( victim->spec_3 ) : "(empty)",
                   victim->spec_4 ? lookup_spec( victim->spec_4 ) : "(empty)" );

    for ( paf = victim->first_affect; paf; paf = paf->next )
        if ( ( skill = get_skilltype( paf->type ) ) != NULL )
            ch_printf( ch, "&z%s: '&C%s&z' modifies &C%s &zby &C%d &zfor &C%d &zrounds with bits &C%s.\n\r",
                       skill_tname[skill->type], skill->name, affect_loc_name( paf->location ), paf->modifier, paf->duration, affect_bit_name( &paf->bitvector ) );

    return;
}



void do_mfind( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA* pMobIndex;
    int hash;
    int nMatch;
    bool fAll;
    set_pager_color( AT_PLAIN, ch );
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Mfind whom?\n\r", ch );
        return;
    }

    fAll    = !str_cmp( arg, "all" );
    nMatch  = 0;

    /*
        This goes through all the hash entry points (1024), and is therefore
        much faster, though you won't get your vnums in order... oh well. :)

        Tests show that Furey's method will usually loop 32,000 times, calling
        get_mob_index()... which loops itself, an average of 1-2 times...
        So theoretically, the above routine may loop well over 40,000 times,
        and my routine bellow will loop for as many index_mobiles are on
        your mud... likely under 3000 times.
        -Thoric
    */
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pMobIndex = mob_index_hash[hash];
                pMobIndex;
                pMobIndex = pMobIndex->next )
            if ( fAll || nifty_is_name( arg, pMobIndex->player_name ) )
            {
                nMatch++;
                pager_printf( ch, "[%5d] %s\n\r",
                              pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
            }

    if ( nMatch )
        pager_printf( ch, "Number of matches: %d\n", nMatch );
    else
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA* pObjIndex;
    int hash;
    int nMatch;
    bool fAll;
    set_pager_color( AT_PLAIN, ch );
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Ofind what?\n\r", ch );
        return;
    }

    fAll    = !str_cmp( arg, "all" );
    nMatch  = 0;

    /*
        This goes through all the hash entry points (1024), and is therefore
        much faster, though you won't get your vnums in order... oh well. :)

        Tests show that Furey's method will usually loop 32,000 times, calling
        get_obj_index()... which loops itself, an average of 2-3 times...
        So theoretically, the above routine may loop well over 50,000 times,
        and my routine bellow will loop for as many index_objects are on
        your mud... likely under 3000 times.
        -Thoric
    */
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash];
                pObjIndex;
                pObjIndex = pObjIndex->next )
            if ( fAll || nifty_is_name( arg, pObjIndex->name ) )
            {
                nMatch++;
                pager_printf( ch, "[%5d] %s\n\r",
                              pObjIndex->vnum, capitalize( stripclr( pObjIndex->short_descr ) ) );
            }

    if ( nMatch )
        pager_printf( ch, "Number of matches: %d\n", nMatch );
    else
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}



void do_mwhere( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    bool found;
    set_pager_color( AT_PLAIN, ch );
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Mwhere whom?\n\r", ch );
        return;
    }

    found = FALSE;

    for ( victim = first_char; victim; victim = victim->next )
    {
        if ( IS_NPC( victim )
                &&   victim->in_room
                &&   nifty_is_name( arg, victim->name ) )
        {
            found = TRUE;
            pager_printf( ch, "[%5d] %-28s [%5d] %s\n\r",
                          victim->pIndexData->vnum,
                          victim->short_descr,
                          victim->in_room->vnum,
                          victim->in_room->name );
        }
    }

    if ( !found )
        act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );

    return;
}

void do_gwhere( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA* d;
    bool pmobs = FALSE;
    int low = 1, high = 65, count = 0;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] != '\0' )
    {
        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
            send_to_pager_color( "\n\r&wSyntax:  gwhere | gwhere <low> <high> | gwhere <low> <high> mobs\n\r", ch );
            return;
        }

        low = atoi( arg1 );
        high = atoi( arg2 );
    }

    if ( low < 1 || high < low || low > high || high > 65 )
    {
        send_to_pager_color( "&wInvalid level range.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg3 );

    if ( !str_cmp( arg3, "mobs" ) )
        pmobs = TRUE;

    pager_printf( ch, "\n\rGlobal %s locations:\n\r", pmobs ? "mob" : "player" );

    if ( !pmobs )
    {
        for ( d = first_descriptor; d; d = d->next )
            if ( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
                    && ( victim = d->character ) != NULL && !IS_NPC( victim ) && victim->in_room
                    &&   can_see( ch, victim )
                    &&   victim->top_level >= low && victim->top_level <= high )
            {
                pager_printf( ch, "(%2d) %-12.12s   [%-5d - %-19.19s]   %-25.25s\n\r",
                              victim->top_level, victim->name, victim->in_room->vnum, victim->in_room->area->name, victim->in_room->name );
                count++;
            }
    }
    else
    {
        for ( victim = first_char; victim; victim = victim->next )
            if ( IS_NPC( victim )
                    &&   victim->in_room && can_see( ch, victim )
                    &&   victim->top_level >= low && victim->top_level <= high )
            {
                pager_printf( ch, "(%2d) %-12.12s   [%-5d - %-19.19s]   %-25.25s\n\r",
                              victim->top_level, victim->name, victim->in_room->vnum, victim->in_room->area->name, victim->in_room->name );
                count++;
            }
    }

    pager_printf( ch, "%d %s found.\n\r", count, pmobs ? "mobs" : "characters" );
    return;
}

void do_bodybag( CHAR_DATA* ch, char* argument )
{
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    bool found;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Bodybag whom?\n\r", ch );
        return;
    }

    /* make sure the buf3 is clear? */
    sprintf( buf3, " " );
    /* check to see if vict is playing? */
    sprintf( buf2, "the corpse of %s", arg );
    found = FALSE;

    for ( obj = first_object; obj; obj = obj->next )
    {
        if ( obj->in_room
                && !str_cmp( buf2, obj->short_descr )
                && ( obj->pIndexData->vnum == 11 ) )
        {
            found = TRUE;
            ch_printf( ch, "Bagging body: [%5d] %-28s [%5d] %s\n\r",
                       obj->pIndexData->vnum,
                       obj->short_descr,
                       obj->in_room->vnum,
                       obj->in_room->name );
            obj_from_room( obj );
            obj = obj_to_char( obj, ch );
            obj->timer = -1;
            save_char_obj( ch );
        }
    }

    if ( !found )
        ch_printf( ch, " You couldn't find any %s\n\r", buf2 );

    return;
}


/* New owhere by Altrag, 03/14/96 */
void do_owhere( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    bool found;
    int icnt = 0;
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Owhere what?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    set_pager_color( AT_PLAIN, ch );

    if ( arg1[0] != '\0' && !str_prefix( arg1, "nesthunt" ) )
    {
        if ( !( obj = get_obj_world( ch, arg ) ) )
        {
            send_to_char( "Nesthunt for what object?\n\r", ch );
            return;
        }

        for ( ; obj->in_obj; obj = obj->in_obj )
        {
            pager_printf( ch, "[%5d] %-28s in object [%5d] %s\n\r",
                          obj->pIndexData->vnum, obj_short( obj ),
                          obj->in_obj->pIndexData->vnum, obj->in_obj->short_descr );
            ++icnt;
        }

        sprintf( buf, "[%5d] %-28s in ", obj->pIndexData->vnum,
                 obj_short( obj ) );

        if ( obj->carried_by )
            sprintf( buf + strlen( buf ), "invent [%5d] %s\n\r",
                     ( IS_NPC( obj->carried_by ) ? obj->carried_by->pIndexData->vnum
                       : 0 ), PERS( obj->carried_by, ch ) );
        else if ( obj->in_room )
            sprintf( buf + strlen( buf ), "room   [%5d] %s\n\r",
                     obj->in_room->vnum, obj->in_room->name );
        else if ( obj->in_obj )
        {
            bug( "do_owhere: obj->in_obj after NULL!", 0 );
            strcat( buf, "object??\n\r" );
        }
        else
        {
            bug( "do_owhere: object doesnt have location!", 0 );
            strcat( buf, "nowhere??\n\r" );
        }

        send_to_pager( buf, ch );
        ++icnt;
        pager_printf( ch, "Nested %d levels deep.\n\r", icnt );
        return;
    }

    found = FALSE;

    for ( obj = first_object; obj; obj = obj->next )
    {
        if ( !nifty_is_name( arg, obj->name ) )
            continue;

        found = TRUE;
        sprintf( buf, "(%3d) [%5d] %-28s in ", ++icnt, obj->pIndexData->vnum,
                 obj_short( obj ) );

        if ( obj->carried_by )
            sprintf( buf + strlen( buf ), "invent [%5d] %s\n\r",
                     ( IS_NPC( obj->carried_by ) ? obj->carried_by->pIndexData->vnum
                       : 0 ), PERS( obj->carried_by, ch ) );
        else if ( obj->in_room )
            sprintf( buf + strlen( buf ), "room   [%5d] %s\n\r",
                     obj->in_room->vnum, obj->in_room->name );
        else if ( obj->in_obj )
            sprintf( buf + strlen( buf ), "object [%5d] %s\n\r",
                     obj->in_obj->pIndexData->vnum, obj_short( obj->in_obj ) );
        else
        {
            bug( "do_owhere: object doesnt have location!", 0 );
            strcat( buf, "nowhere??\n\r" );
        }

        send_to_pager( buf, ch );
    }

    if ( !found )
        act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    else
        pager_printf( ch, "%d matches.\n\r", icnt );

    return;
}


void do_reboo( CHAR_DATA* ch, char* argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool mud_down;
    CHAR_DATA* vch;

    if ( str_cmp( argument, "mud now" )
            &&   str_cmp( argument, "nosave" )
            &&   str_cmp( argument, "and sort skill table" ) )
    {
        send_to_char( "Syntax: 'reboot mud now' or 'reboot nosave'\n\r", ch );
        return;
    }

    sprintf( buf, "Reboot by %s.", ch->name );
    do_echo( ch, buf );

    if ( !str_cmp( argument, "and sort skill table" ) )
    {
        sort_skill_table();
        save_skill_table();
    }

    /* Save all characters before booting. */
    if ( str_cmp( argument, "nosave" ) )
        for ( vch = first_char; vch; vch = vch->next )
            if ( !IS_NPC( vch ) )
                save_char_obj( vch );

    mud_down = TRUE;
    return;
}



void do_shutdow( CHAR_DATA* ch, char* argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool mud_down;
    CHAR_DATA* vch;

    if ( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) )
    {
        send_to_char( "Syntax: 'shutdown mud now' or 'shutdown nosave'\n\r", ch );
        return;
    }

    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    do_echo( ch, buf );

    /* Save all characters before booting. */
    if ( str_cmp( argument, "nosave" ) )
        for ( vch = first_char; vch; vch = vch->next )
            if ( !IS_NPC( vch ) )
                save_char_obj( vch );

    mud_down = TRUE;
    return;
}


void do_snoop( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA* d;
    CHAR_DATA* victim;
    int v, i, cnt = 0;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Snoop whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !victim->desc )
    {
        send_to_char( "No descriptor to snoop.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Cancelling all snoops.\n\r", ch );

        for ( d = first_descriptor; d; d = d->next )
        {
            for ( i = 0; i <= 5; i++ )
            {
                if ( d->snoop_by[i] == ch->desc )
                    d->snoop_by[i] = NULL;
            }
        }

        return;
    }

    for ( i = 0; i <= 5; i++ )
    {
        if ( victim->desc->snoop_by[i] != NULL )
            cnt++;
    }

    if ( cnt >= 5 )
    {
        send_to_char( "There are already 5 people snooping that target.\n\r", ch );
        return;
    }

    /*
        Minimum snoop level... a secret mset value
        makes the snooper think that the victim is already being snooped
    */
    if ( get_trust( victim ) >= get_trust( ch )
            ||  ( victim->pcdata && victim->pcdata->min_snoop > get_trust( ch ) ) )
    {
        send_to_char( "&w&RBusy already. Maximum of five pipes already established.\n\r", ch );
        return;
    }

    if ( ch->desc )
    {
        for ( v = 0; v <= 5; v++ )
        {
            for ( d = ch->desc->snoop_by[v]; d; d = d->snoop_by[v] )
            {
                if ( d->character == victim || d->original == victim )
                {
                    send_to_char( "No snoop loops.\n\r", ch );
                    return;
                }
            }
        }
    }

    /*  Snoop notification for higher imms, if desired, uncomment this
        if ( get_trust(victim) > LEVEL_GOD && get_trust(ch) < LEVEL_SUPREME )
          write_to_descriptor( victim->desc->descriptor, "\n\rYou feel like someone is watching your every move...\n\r", 0 );
    */
    for ( i = 0; i <= 5; i++ )
    {
        if ( victim->desc->snoop_by[i] == NULL )
        {
            victim->desc->snoop_by[i] = ch->desc;
            ch_printf( ch, "&w&RYou are now snooping them. You are snoop number %d on this player.\n\r", 1 + i );
            return;
        }
    }

    send_to_char( "&w&RBusy already. Maximum of five pipes already established.\n\r", ch );
    // victim->desc->snoop_by = ch->desc;
    // send_to_char( "Ok.\n\r", ch );
    return;
}



/*
    void do_switch( CHAR_DATA *ch, char *argument )
    {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    send_to_char( "Switch into whom?\n\r", ch );
    return;
    }

    if ( !ch->desc )
    return;

    if ( ch->desc->original )
    {
    send_to_char( "You are already switched.\n\r", ch );
    return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
    send_to_char( "They aren't here.\n\r", ch );
    return;
    }

    if ( victim == ch )
    {
    send_to_char( "Ok.\n\r", ch );
    return;
    }

    if ( victim->desc )
    {
    send_to_char( "Character in use.\n\r", ch );
    return;
    }

    if ( !IS_NPC(victim) && get_trust(ch) < LEVEL_GREATER )
    {
    send_to_char( "You cannot switch into a player!\n\r", ch );
    return;
    }

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->switched    = victim;
    send_to_char( "Ok.\n\r", victim );
    return;
    }
*/


void do_return( CHAR_DATA* ch, char* argument )
{
    if ( !ch->desc )
        return;

    if ( !ch->desc->original )
    {
        send_to_char( "You aren't switched.\n\r", ch );
        return;
    }

    if ( xIS_SET( ch->act, ACT_POLYMORPHED ) )
    {
        send_to_char( "Use revert to return from a polymorphed mob.\n\r", ch );
        return;
    }

    send_to_char( "You return to your original body.\n\r", ch );
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc->character->switched = NULL;
    ch->desc                  = NULL;
    return;
}



void do_minvoke( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA* pMobIndex;
    CHAR_DATA* victim;
    int vnum;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: minvoke <vnum>.\n\r", ch );
        return;
    }

    if ( !is_number( arg ) )
    {
        char arg2[MAX_INPUT_LENGTH];
        int  hash, cnt;
        int  count = number_argument( arg, arg2 );
        vnum = -1;

        for ( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
            for ( pMobIndex = mob_index_hash[hash];
                    pMobIndex;
                    pMobIndex = pMobIndex->next )
                if ( nifty_is_name( arg2, pMobIndex->player_name )
                        &&   ++cnt == count )
                {
                    vnum = pMobIndex->vnum;
                    break;
                }

        if ( vnum == -1 )
        {
            send_to_char( "No such mobile exists.\n\r", ch );
            return;
        }
    }
    else
        vnum = atoi( arg );

    if ( get_trust( ch ) < LEVEL_DEMI )
    {
        AREA_DATA* pArea;

        if ( IS_NPC( ch ) )
        {
            send_to_char( "Huh?\n\r", ch );
            return;
        }

        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            send_to_char( "You must have an assigned area to invoke this mobile.\n\r", ch );
            return;
        }

        if ( vnum < pArea->low_m_vnum
                &&   vnum > pArea->hi_m_vnum )
        {
            send_to_char( "That number is not in your allocated range.\n\r", ch );
            return;
        }
    }

    if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
    {
        send_to_char( "No mobile has that vnum.\n\r", ch );
        return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( AT_IMMORT, "$n has created $N!", ch, NULL, victim, TO_ROOM );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oinvoke( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA* pObjIndex;
    OBJ_DATA* obj;
    int amount;
    int level;
    int vnum;
    int i;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: oinvoke <vnum> <amount>.\n\r", ch );
        return;
    }

    level = get_trust( ch );

    if ( arg2[0] == '\0' )
    {
        amount = 1;
    }
    else
    {
        if ( !is_number( arg2 ) )
        {
            send_to_char( "Syntax: oinvoke <vnum> <amount>.\n\r", ch );
            return;
        }

        amount = atoi( arg2 );

        if ( amount < 1 || amount > 99 )
        {
            send_to_char( "Amount must be between 1 and 99.\n\r", ch );
            return;
        }
    }

    if ( !is_number( arg1 ) )
    {
        char arg[MAX_INPUT_LENGTH];
        int  hash, cnt;
        int  count = number_argument( arg1, arg );
        vnum = -1;

        for ( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
            for ( pObjIndex = obj_index_hash[hash];
                    pObjIndex;
                    pObjIndex = pObjIndex->next )
                if ( nifty_is_name( arg, pObjIndex->name )
                        &&   ++cnt == count )
                {
                    vnum = pObjIndex->vnum;
                    break;
                }

        if ( vnum == -1 )
        {
            send_to_char( "No such object exists.\n\r", ch );
            return;
        }
    }
    else
        vnum = atoi( arg1 );

    if ( get_trust( ch ) < LEVEL_DEMI )
    {
        AREA_DATA* pArea;

        if ( IS_NPC( ch ) )
        {
            send_to_char( "Huh?\n\r", ch );
            return;
        }

        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            send_to_char( "You must have an assigned area to invoke this object.\n\r", ch );
            return;
        }

        if ( vnum < pArea->low_o_vnum
                &&   vnum > pArea->hi_o_vnum )
        {
            send_to_char( "That number is not in your allocated range.\n\r", ch );
            return;
        }
    }

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
        send_to_char( "No object has that vnum.\n\r", ch );
        return;
    }

    /*  Commented out by Narn, it seems outdated
        if ( IS_OBJ_STAT( pObjIndex, ITEM_PROTOTYPE )
        &&   pObjIndex->count > 5 )
        {
        send_to_char( "That object is at its limit.\n\r", ch );
        return;
        }
    */

    for ( i = 0; i < amount; i++ )
    {
        obj = create_object( pObjIndex, level );

        if ( CAN_WEAR( obj, ITEM_TAKE ) )
        {
            obj = obj_to_char( obj, ch );
        }
        else
        {
            obj = obj_to_room( obj, ch->in_room );

            if ( i == 1 )
                act( AT_IMMORT, "$n has created $p!", ch, obj, NULL, TO_ROOM );
        }
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    OBJ_DATA* obj;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA* vnext;
        OBJ_DATA*  obj_next;

        for ( victim = ch->in_room->first_person; victim; victim = vnext )
        {
            vnext = victim->next_in_room;

            if ( IS_NPC( victim ) && !IS_BOT( victim ) && victim != ch && !xIS_SET( victim->act, ACT_POLYMORPHED ) )
                extract_char( victim, TRUE, FALSE );
        }

        for ( obj = ch->in_room->first_content; obj; obj = obj_next )
        {
            obj_next = obj->next_content;
            extract_obj( obj );
        }

        /* SAVE_EQ room checks */
        if ( if_equip_room( ch->in_room ) )
            save_equip_room( ch, ch->in_room );

        act( AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM );
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    victim = NULL;
    obj = NULL;

    /*  fixed to get things in room first -- i.e., purge portal (obj),
        no more purging mobs with that keyword in another room first
        -- Tri */
    if ( ( victim = get_char_room( ch, arg ) ) == NULL
            && ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
        if ( ( victim = get_char_world_full( ch, arg ) ) == NULL
                &&   ( obj = get_obj_world( ch, arg ) ) == NULL )  /* no get_obj_room */
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }
    }

    /* Single object purge in room for high level purge - Scryn 8/12*/
    if ( obj )
    {
        separate_obj( obj );
        act( AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM );
        act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
        extract_obj( obj );

        /* SAVE_EQ room checks */
        if ( if_equip_room( ch->in_room ) )
            save_equip_room( ch, ch->in_room );

        return;
    }

    if ( !IS_NPC( victim ) )
    {
        send_to_char( "Not on PC's.\n\r", ch );
        return;
    }

    if ( IS_BOT( victim ) )
    {
        send_to_char( "Not on Bots.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You cannot purge yourself!\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->act, ACT_POLYMORPHED ) )
    {
        send_to_char( "You cannot purge a polymorphed player.\n\r", ch );
        return;
    }

    act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE, FALSE );
    return;
}


void do_low_purge( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    OBJ_DATA* obj;
    return;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Purge what?\n\r", ch );
        return;
    }

    victim = NULL;
    obj = NULL;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL
            &&   ( obj    = get_obj_here ( ch, arg ) ) == NULL )
    {
        send_to_char( "You can't find that here.\n\r", ch );
        return;
    }

    if ( obj )
    {
        separate_obj( obj );
        act( AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM );
        act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
        extract_obj( obj );
        return;
    }

    if ( !IS_NPC( victim ) )
    {
        send_to_char( "Not on PC's.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You cannot purge yourself!\n\r", ch );
        return;
    }

    act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
    extract_char( victim, TRUE, FALSE );
    return;
}


void do_balzhur( CHAR_DATA* ch, char* argument )
{
    char arg[MIL], buf[MSL], buf2[SUPER_MSL], buf3[SUB_MSL];
    CHAR_DATA* victim;
    AREA_DATA* pArea;
    int sn;
    argument = one_argument( argument, arg );

    if ( NULLSTR( arg ) )
    {
        send_to_char( "Who is deserving of such a fate?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "I wouldn't even think of that if I were you...\n\r", ch );
        return;
    }

    set_char_color( AT_WHITE, ch );
    send_to_char( "You summon the demon Balzhur to wreak your wrath!\n\r", ch );
    send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke.\n\r", ch );
    set_char_color( AT_IMMORT, victim );
    send_to_char( "You hear an ungodly sound in the distance that makes your blood run cold!\n\r", victim );
    snprintf( buf, MSL, "Balzhur screams, 'You are MINE %s!!!'", victim->name );
    echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
    victim->top_level = 1;
    victim->trust    = 0;
    victim->max_hit  = 500;
    victim->max_move = 1000;

    for ( sn = 0; sn < top_sn; sn++ )
        victim->pcdata->learned[sn] = 0;

    victim->hit      = victim->max_hit;
    victim->move     = victim->max_move;
    sprintf( buf, "%s%s", GOD_DIR, capitalize( victim->name ) );

    if ( !remove( buf ) )
        send_to_char( "Player's immortal data destroyed.\n\r", ch );
    else if ( errno != ENOENT )
    {
        ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric\n\r",
                   errno, strerror( errno ) );
        snprintf( buf2, SUPER_MSL, "%s balzhuring %s", ch->name, buf );
        perror( buf2 );
    }

    snprintf( buf3, SUB_MSL, "%s.are", capitalize( arg ) );

    for ( pArea = first_build; pArea; pArea = pArea->next )
        if ( !strcmp( pArea->filename, buf3 ) )
        {
            snprintf( buf, MSL, "%s%s", BUILD_DIR, buf3 );

            if ( IS_SET( pArea->status, AREA_LOADED ) )
                fold_area( pArea, buf, FALSE );

            close_area( pArea );
            snprintf( buf2, SUPER_MSL, "%s.bak", buf );
            set_char_color( AT_RED, ch ); /* Log message changes colors */

            if ( !rename( buf, buf2 ) )
                send_to_char( "Player's area data destroyed.  Area saved as backup.\n\r", ch );
            else if ( errno != ENOENT )
            {
                ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Thoric.\n\r",
                           errno, strerror( errno ) );
                snprintf( buf2, SUPER_MSL, "%s destroying %s", ch->name, buf );
                perror( buf2 );
            }
        }

    make_wizlist();
    do_help( victim, "M_BALZHUR_" );
    set_char_color( AT_WHITE, victim );
    send_to_char( "You awake after a long period of time...\n\r", victim );

    while ( victim->first_carrying )
        extract_obj( victim->first_carrying );

    return;
}

void do_advance( CHAR_DATA* ch, char* argument )
{
    send_to_char( "Incomplete code. Contact Ghost.\n\r", ch );
    bug( "act_wiz.c: incomplete code under do_advance." );
    return;
}

/*  void do_immortalize( CHAR_DATA *ch, char *argument )
    {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    send_to_char( "Syntax: immortalize <char>\n\r", ch );
    return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
    send_to_char( "That player is not here.\n\r", ch);
    return;
    }

    if ( IS_NPC(victim) )
    {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
    }

    if ( victim->top_level != LEVEL_AVATAR )
    {
    send_to_char( "This player is not worthy of immortality yet.\n\r", ch );
    return;
    }

    send_to_char( "Immortalizing a player...\n\r", ch );
    set_char_color( AT_IMMORT, victim );
    act( AT_IMMORT, "$n begins to chant softly... then raises $s arms to the sky...",
     ch, NULL, NULL, TO_ROOM );
    set_char_color( AT_WHITE, victim );
    send_to_char( "You suddenly feel very strange...\n\r\n\r", victim );
    set_char_color( AT_LBLUE, victim );

    do_help(victim, "M_GODLVL1_" );
    set_char_color( AT_WHITE, victim );
    send_to_char( "You awake... all your possessions are gone.\n\r", victim );
    while ( victim->first_carrying )
    extract_obj( victim->first_carrying );

    victim->top_level = LEVEL_IMMORTAL;

    advance_level( victim );

    victim->trust = 0;
    return;
    }
*/



void do_trust( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    int level;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch );
        return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
        send_to_char( "Level must be 0 (reset) or 1 to 60.\n\r", ch );
        return;
    }

    if ( level > get_trust( ch ) )
    {
        send_to_char( "Limited to your own trust.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    victim->trust = level;
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_prestore( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "&w&RSyntax: PRestore <character name>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) != NULL )
    {
        if ( !str_cmp( victim->name, arg ) && !IS_NPC( victim ) )
        {
            ch_printf( ch, "&RNot while %s is still online.\n\r", victim->name );
            return;
        }
    }

    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), capitalize( arg ) );
    sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower( arg[0] ), capitalize( arg ) );

    if ( !file_exist( buf2 ) )
    {
        ch_printf( ch, "&RNo backup located at '%s'.\n\r", buf2 );
        return;
    }

    rename( buf2, buf );

    if ( !file_exist( buf ) )
    {
        ch_printf( ch, "&RFailed backup restore from '%s'.\n\r", buf2 );
        return;
    }

    ch_printf( ch, "&GRestored backup from '%s'.\n\r", buf2 );
    return;
}

void do_restore( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Restore whom?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA* vch;
        CHAR_DATA* vch_next;

        if ( !ch->pcdata )
            return;

        if ( get_trust( ch ) < MAX_LEVEL )
        {
            send_to_char( "Only admins can restore all now.\n\r", ch );
            return;
        }

        if ( get_trust( ch ) < LEVEL_SUB_IMPLEM )
        {
            if ( IS_NPC( ch ) )
            {
                send_to_char( "You can't do that.\n\r", ch );
                return;
            }
            else
            {
                /* Check if the player did a restore all within the last 18 hours. */
                if ( current_time - last_restore_all_time < RESTORE_INTERVAL )
                {
                    send_to_char( "Sorry, you can't do a restore all yet.\n\r", ch );
                    do_restoretime( ch, "" );
                    return;
                }
            }
        }

        last_restore_all_time    = current_time;
        ch->pcdata->restore_time = current_time;
        save_char_obj( ch );
        send_to_char( "Ok.\n\r", ch );

        for ( vch = first_char; vch; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC( vch ) && !IS_IMMORTAL( vch ) )
            {
                vch->hit = vch->max_hit;
                vch->move = vch->max_move;
                vch->mental_state = 0;
                vch->emotional_state = 0;
                update_pos ( vch );
                act( AT_IMMORT, "$n has restored you.", ch, NULL, vch, TO_VICT );
            }
        }
    }
    else
    {
        CHAR_DATA* victim;

        if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( get_trust( ch ) < LEVEL_LESSER
                &&  victim != ch
                && !( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) ) )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }

        victim->hit  = victim->max_hit;
        victim->move = victim->max_move;
        victim->mental_state = 0;
        victim->emotional_state = 0;
        update_pos( victim );

        if ( ch != victim )
            act( AT_IMMORT, "$n has restored you.", ch, NULL, victim, TO_VICT );

        send_to_char( "Ok.\n\r", ch );
        return;
    }
}

void do_restoretime( CHAR_DATA* ch, char* argument )
{
    long int time_passed;
    int hour, minute;

    if ( !last_restore_all_time )
        ch_printf( ch, "There has been no restore all since reboot\n\r" );
    else
    {
        time_passed = current_time - last_restore_all_time;
        hour = ( int ) ( time_passed / 3600 );
        minute = ( int ) ( ( time_passed - ( hour * 3600 ) ) / 60 );
        ch_printf( ch, "The  last restore all was %d hours and %d minutes ago.\n\r",
                   hour, minute );
    }

    if ( !ch->pcdata )
        return;

    if ( !ch->pcdata->restore_time )
    {
        send_to_char( "You have never done a restore all.\n\r", ch );
        return;
    }

    time_passed = current_time - ch->pcdata->restore_time;
    hour = ( int ) ( time_passed / 3600 );
    minute = ( int ) ( ( time_passed - ( hour * 3600 ) ) / 60 );
    ch_printf( ch, "Your last restore all was %d hours and %d minutes ago.\n\r",
               hour, minute );
    return;
}

void do_freeze( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Freeze whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->act, PLR_FREEZE ) )
    {
        xREMOVE_BIT( victim->act, PLR_FREEZE );
        send_to_char( "You can play again.\n\r", victim );
        send_to_char( "FREEZE removed.\n\r", ch );
    }
    else
    {
        xSET_BIT( victim->act, PLR_FREEZE );
        send_to_char( "You can't do ANYthing!\n\r", victim );
        send_to_char( "FREEZE set.\n\r", ch );
    }

    save_char_obj( victim );
    return;
}



void do_log( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Log whom?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        if ( fLogAll )
        {
            fLogAll = FALSE;
            send_to_char( "Log ALL off.\n\r", ch );
        }
        else
        {
            fLogAll = TRUE;
            send_to_char( "Log ALL on.\n\r", ch );
        }

        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    /*
        No level check, gods can log anyone.
    */
    if ( xIS_SET( victim->act, PLR_LOG ) )
    {
        xREMOVE_BIT( victim->act, PLR_LOG );
        send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
        xSET_BIT( victim->act, PLR_LOG );
        send_to_char( "LOG set.\n\r", ch );
    }

    return;
}


void do_litterbug( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Set litterbug flag on whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->act, PLR_LITTERBUG ) )
    {
        xREMOVE_BIT( victim->act, PLR_LITTERBUG );
        send_to_char( "You can drop items again.\n\r", victim );
        send_to_char( "LITTERBUG removed.\n\r", ch );
    }
    else
    {
        xSET_BIT( victim->act, PLR_LITTERBUG );
        send_to_char( "You a strange force prevents you from dropping any more items!\n\r", victim );
        send_to_char( "LITTERBUG set.\n\r", ch );
    }

    return;
}


void do_noemote( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Noemote whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->act, PLR_NO_EMOTE ) )
    {
        xREMOVE_BIT( victim->act, PLR_NO_EMOTE );
        send_to_char( "You can emote again.\n\r", victim );
        send_to_char( "NO_EMOTE removed.\n\r", ch );
    }
    else
    {
        xSET_BIT( victim->act, PLR_NO_EMOTE );
        send_to_char( "You can't emote!\n\r", victim );
        send_to_char( "NO_EMOTE set.\n\r", ch );
    }

    return;
}



void do_notell( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Notell whom?", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->act, PLR_NO_TELL ) )
    {
        xREMOVE_BIT( victim->act, PLR_NO_TELL );
        send_to_char( "You can tell again.\n\r", victim );
        send_to_char( "NO_TELL removed.\n\r", ch );
    }
    else
    {
        xSET_BIT( victim->act, PLR_NO_TELL );
        send_to_char( "You can't tell!\n\r", victim );
        send_to_char( "NO_TELL set.\n\r", ch );
    }

    return;
}


void do_notitle( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Notitle whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->pcdata->flags, PCFLAG_NOTITLE ) )
    {
        xREMOVE_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
        send_to_char( "You can set your own title again.\n\r", victim );
        send_to_char( "NOTITLE removed.\n\r", ch );
    }
    else
    {
        xSET_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
        sprintf( buf, "%s", victim->name );
        set_title( victim, buf );
        send_to_char( "You can't set your own title!\n\r", victim );
        send_to_char( "NOTITLE set.\n\r", ch );
    }

    return;
}

void do_silence( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Silence whom?", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->act, PLR_SILENCE ) )
    {
        send_to_char( "Player already silenced, use unsilence to remove.\n\r", ch );
    }
    else
    {
        xSET_BIT( victim->act, PLR_SILENCE );
        send_to_char( "You can't use channels!\n\r", victim );
        send_to_char( "SILENCE set.\n\r", ch );
    }

    return;
}

void do_unsilence( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Unsilence whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( xIS_SET( victim->act, PLR_SILENCE ) )
    {
        xREMOVE_BIT( victim->act, PLR_SILENCE );
        send_to_char( "You can use channels again.\n\r", victim );
        send_to_char( "SILENCE removed.\n\r", ch );
    }
    else
    {
        send_to_char( "That player is not silenced.\n\r", ch );
    }

    return;
}

BAN_DATA*       first_ban;
BAN_DATA*       last_ban;

void save_banlist( void )
{
    BAN_DATA* pban;
    FILE* fp;
    fclose( fpReserve );

    if ( !( fp = fopen( SYSTEM_DIR BAN_LIST, "w" ) ) )
    {
        bug( "Save_banlist: Cannot open " BAN_LIST, 0 );
        perror( BAN_LIST );
        fpReserve = fopen( NULL_FILE, "r" );
        return;
    }

    for ( pban = first_ban; pban; pban = pban->next )
        fprintf( fp, "%d %s~~%s~\n", pban->level, pban->name, pban->ban_time );

    fprintf( fp, "-1\n" );
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}



void do_ban( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA* pban;
    int bnum;

    if ( IS_NPC( ch ) )
        return;

    argument = one_argument( argument, arg );
    set_pager_color( AT_PLAIN, ch );

    if ( arg[0] == '\0' )
    {
        send_to_pager( "Banned sites:\n\r", ch );
        send_to_pager( "[ #] (Lv) Time                     Site\n\r", ch );
        send_to_pager( "---- ---- ------------------------ ---------------\n\r", ch );

        for ( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
            pager_printf( ch, "[%2d] (%2d) %-24s %s\n\r", bnum,
                          pban->level, pban->ban_time, pban->name );

        return;
    }

    /*  People are gonna need .# instead of just # to ban by just last
        number in the site ip.                               -- Altrag */
    if ( is_number( arg ) )
    {
        for ( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
            if ( bnum == atoi( arg ) )
                break;

        if ( !pban )
        {
            do_ban( ch, "" );
            return;
        }

        argument = one_argument( argument, arg );

        if ( arg[0] == '\0' )
        {
            do_ban( ch, "help" );
            return;
        }

        if ( !str_cmp( arg, "level" ) )
        {
            argument = one_argument( argument, arg );

            if ( arg[0] == '\0' || !is_number( arg ) )
            {
                do_ban( ch, "help" );
                return;
            }

            if ( atoi( arg ) < 1 || atoi( arg ) > LEVEL_SUPREME )
            {
                ch_printf( ch, "Level range: 1 - %d.\n\r", LEVEL_SUPREME );
                return;
            }

            pban->level = atoi( arg );
            send_to_char( "Ban level set.\n\r", ch );
        }
        else if ( !str_cmp( arg, "newban" ) )
        {
            pban->level = 1;
            send_to_char( "New characters banned.\n\r", ch );
        }
        else if ( !str_cmp( arg, "mortal" ) )
        {
            pban->level = LEVEL_AVATAR;
            send_to_char( "All mortals banned.\n\r", ch );
        }
        else if ( !str_cmp( arg, "total" ) )
        {
            pban->level = LEVEL_SUPREME;
            send_to_char( "Everyone banned.\n\r", ch );
        }
        else if ( !str_cmp( arg, "imm" ) )
        {
            pban->level = -1;
            send_to_char( "Immortals banned.\n\r", ch );
        }
        else if ( !str_cmp( arg, "school" ) )
        {
            pban->level = -2;
            send_to_char( "School days banned.\n\r", ch );
        }
        else
        {
            do_ban( ch, "help" );
            return;
        }

        save_banlist( );
        return;
    }

    if ( !str_cmp( arg, "help" ) )
    {
        send_to_char( "Syntax: ban <site address>\n\r", ch );
        send_to_char( "Syntax: ban <ban number> <level <lev>|newban|mortal|total|imm>\n\r", ch );
        return;
    }

    for ( pban = first_ban; pban; pban = pban->next )
    {
        if ( !str_cmp( arg, pban->name ) )
        {
            send_to_char( "That site is already banned!\n\r", ch );
            return;
        }
    }

    CREATE( pban, BAN_DATA, 1 );
    LINK( pban, first_ban, last_ban, next, prev );
    pban->name  = str_dup( arg );
    pban->level = LEVEL_AVATAR;
    sprintf( buf, "%24.24s", ctime( &current_time ) );
    pban->ban_time = str_dup( buf );
    save_banlist( );
    send_to_char( "Ban created.  Mortals banned from site.\n\r", ch );
    return;
}

void do_allow( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA* pban;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove which site from the ban list?\n\r", ch );
        return;
    }

    for ( pban = first_ban; pban; pban = pban->next )
    {
        if ( !str_cmp( arg, pban->name ) )
        {
            UNLINK( pban, first_ban, last_ban, next, prev );

            if ( pban->ban_time )
                DISPOSE( pban->ban_time );

            DISPOSE( pban->name );
            DISPOSE( pban );
            save_banlist( );
            send_to_char( "Site no longer banned.\n\r", ch );
            return;
        }
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}

void do_wizlock( CHAR_DATA* ch, char* argument )
{
    extern bool wizlock;
    wizlock = !wizlock;
    sysdata.SET_WIZLOCK = wizlock;
    save_sysdata( sysdata );

    if ( wizlock )
        send_to_char( "Game wizlocked.\n\r", ch );
    else
        send_to_char( "Game un-wizlocked.\n\r", ch );

    return;
}


void do_noresolve( CHAR_DATA* ch, char* argument )
{
    sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

    if ( sysdata.NO_NAME_RESOLVING )
        send_to_char( "Name resolving disabled.\n\r", ch );
    else
        send_to_char( "Name resolving enabled.\n\r", ch );

    return;
}


void do_setooc( CHAR_DATA* ch, char* argument )
{
    if ( !str_cmp( argument, "on" ) )
        sysdata.ALLOW_OOC = TRUE;
    else if ( !str_cmp( argument, "off" ) )
        sysdata.ALLOW_OOC = FALSE;
    else
        sysdata.ALLOW_OOC = !sysdata.ALLOW_OOC;

    save_sysdata( sysdata );

    if ( sysdata.ALLOW_OOC )
        send_to_char( "\n\rOOC usage is now enabled.\n\r", ch );
    else
        send_to_char( "\n\rOOC usage is now disabled.\n\r", ch );

    return;
}

void do_users( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA* d;
    int count;
    char arg[MAX_INPUT_LENGTH];
    bool full = FALSE;
    set_pager_color( AT_PLAIN, ch );
    one_argument ( argument, arg );
    count   = 0;
    buf[0]  = '\0';

    if ( !str_cmp( arg, "full" ) )
        full = TRUE;

    if ( full )
    {
        sprintf( buf, "\n\rDesc|Con|Idle| Port | Player      @HostIP           " );
        strcat( buf, "\n\r" );
        strcat( buf, "----+---+----+------+-------------------------------" );
        strcat( buf, "\n\r" );
    }
    else
    {
        sprintf( buf, "\n\rDesc|Con|Idle| Player      @HostIP           " );
        strcat( buf, "\n\r" );
        strcat( buf, "----+---+----+-------------------------------" );
        strcat( buf, "\n\r" );
    }

    send_to_pager( buf, ch );

    for ( d = first_descriptor; d; d = d->next )
    {
        if ( arg[0] == '\0' )
        {
            if (  get_trust( ch ) >= LEVEL_SUPREME
                    ||   ( d->character && can_see( ch, d->character ) ) )
            {
                count++;

                if ( full )
                {
                    sprintf( buf,
                             " %3d| %2d|%4d|%6d| %-12s@%-16s ",
                             d->descriptor,
                             d->connected,
                             d->idle / 4,
                             d->port,
                             d->original  ? d->original->name  :
                             d->character ? d->character->name : "(none)",
                             d->host );
                }
                else
                {
                    sprintf( buf,
                             " %3d| %2d|%4d| %-12s@%-16s ",
                             d->descriptor,
                             d->connected,
                             d->idle / 4,
                             d->original  ? d->original->name  :
                             d->character ? d->character->name : "(none)",
                             d->host );
                }

                strcat( buf, "\n\r" );
                send_to_pager( buf, ch );
            }
        }
        else
        {
            if ( ( get_trust( ch ) >= LEVEL_SUPREME
                    ||   ( d->character && can_see( ch, d->character ) ) )
                    &&   ( !str_prefix( arg, d->host )
                           ||   ( d->character && !str_prefix( arg, d->character->name ) ) ) )
            {
                count++;

                if ( full )
                {
                    pager_printf( ch,
                                  " %3d| %2d|%4d|%6d| %-12s@%-16s ",
                                  d->descriptor,
                                  d->connected,
                                  d->idle / 4,
                                  d->port,
                                  d->original  ? d->original->name  :
                                  d->character ? d->character->name : "(none)",
                                  d->host
                                );
                }
                else
                {
                    pager_printf( ch,
                                  " %3d| %2d|%4d| %-12s@%-16s ",
                                  d->descriptor,
                                  d->connected,
                                  d->idle / 4,
                                  d->original  ? d->original->name  :
                                  d->character ? d->character->name : "(none)",
                                  d->host
                                );
                }

                buf[0] = '\0';
                strcat( buf, "\n\r" );
                send_to_pager( buf, ch );
            }
        }
    }

    pager_printf( ch, "%d user%s.\n\r", count, count == 1 ? "" : "s" );
    return;
}



/*
    Thanks to Grodyn for pointing out bugs in this function.
*/
void do_force( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    bool mobsonly;
    set_char_color( AT_IMMORT, ch );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Force whom to do what?\n\r", ch );
        return;
    }

    mobsonly = get_trust( ch ) < sysdata.level_forcepc;

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA* vch;
        CHAR_DATA* vch_next;

        if ( mobsonly )
        {
            send_to_char( "Force whom to do what?\n\r", ch );
            return;
        }

        for ( vch = first_char; vch; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC( vch ) && get_trust( vch ) < get_trust( ch ) )
            {
                if ( get_trust( ch ) < 105 )
                    act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );

                interpret( vch, argument, FALSE );
            }
        }
    }
    else
    {
        CHAR_DATA* victim;

        if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( victim == ch )
        {
            send_to_char( "Aye aye, right away!\n\r", ch );
            return;
        }

        if ( ( get_trust( victim ) >= get_trust( ch ) )
                || ( mobsonly && !IS_NPC( victim ) ) )
        {
            send_to_char( "Do it yourself!\n\r", ch );
            return;
        }

        if ( get_trust( ch ) < 105 )
            act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );

        interpret( victim, argument, FALSE );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_invis( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int level;
    /*
        if ( IS_NPC(ch))
        return;
    */
    argument = one_argument( argument, arg );

    if ( !NULLSTR( arg ) )
    {
        if ( !is_number( arg ) )
        {
            send_to_char( "Usage: invis | invis <level>\n\r", ch );
            return;
        }

        level = atoi( arg );

        if ( level < 2 || level > get_trust( ch ) )
        {
            send_to_char( "Invalid level.\n\r", ch );
            return;
        }

        if ( !IS_NPC( ch ) )
        {
            ch->pcdata->wizinvis = level;
            ch_printf( ch, "Wizinvis level set to %d.\n\r", level );
        }

        if ( IS_NPC( ch ) )
        {
            ch->mobinvis = level;
            ch_printf( ch, "Mobinvis level set to %d.\n\r", level );
        }

        return;
    }

    if ( !IS_NPC( ch ) )
    {
        if ( ch->pcdata->wizinvis < 2 )
            ch->pcdata->wizinvis = ch->top_level;
    }

    if ( IS_NPC( ch ) )
    {
        if ( ch->mobinvis < 2 )
            ch->mobinvis = ch->top_level;
    }

    if ( xIS_SET( ch->act, PLR_WIZINVIS ) )
    {
        xREMOVE_BIT( ch->act, PLR_WIZINVIS );
        act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly fade back into existence.\n\r", ch );
    }
    else
    {
        xSET_BIT( ch->act, PLR_WIZINVIS );
        act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly vanish into thin air.\n\r", ch );
    }

    return;
}


void do_holylight( CHAR_DATA* ch, char* argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( xIS_SET( ch->act, PLR_HOLYLIGHT ) )
    {
        xREMOVE_BIT( ch->act, PLR_HOLYLIGHT );
        send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
        xSET_BIT( ch->act, PLR_HOLYLIGHT );
        send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

void do_buildwalk( CHAR_DATA* ch, char* argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( xIS_SET( ch->act, PLR_BUILDWALK ) )
    {
        xREMOVE_BIT( ch->act, PLR_BUILDWALK );
        send_to_char( "Buildwalk mode off.\n\r", ch );
    }
    else
    {
        xSET_BIT( ch->act, PLR_BUILDWALK );
        send_to_char( "Buildwalk mode on.\n\r", ch );
    }

    return;
}

void do_rassign( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  r_lo, r_hi;
    CHAR_DATA* victim;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    r_lo = atoi( arg2 );
    r_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || r_lo < 0 || r_hi < 0 )
    {
        send_to_char( "Syntax: assign <who> <low> <high>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They don't seem to be around.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) || get_trust( victim ) < LEVEL_AVATAR )
    {
        send_to_char( "They wouldn't know what to do with a room range.\n\r", ch );
        return;
    }

    if ( r_lo > r_hi )
    {
        send_to_char( "Unacceptable room range.\n\r", ch );
        return;
    }

    if ( r_lo == 0 )
        r_hi = 0;

    victim->pcdata->r_range_lo = r_lo;
    victim->pcdata->r_range_hi = r_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the room range %d - %d.\n\r",
               ch->name, r_lo, r_hi );
    assign_area( victim );  /* Put back by Thoric on 02/07/96 */

    if ( !victim->pcdata->area )
    {
        bug( "rassign: assign_area failed", 0 );
        return;
    }

    if ( r_lo == 0 )            /* Scryn 8/12/95 */
    {
        REMOVE_BIT ( victim->pcdata->area->status, AREA_LOADED );
        SET_BIT( victim->pcdata->area->status, AREA_DELETED );
    }
    else
    {
        SET_BIT( victim->pcdata->area->status, AREA_LOADED );
        REMOVE_BIT( victim->pcdata->area->status, AREA_DELETED );
    }

    return;
}

void do_vassign( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  r_lo, r_hi;
    CHAR_DATA* victim;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    r_lo = atoi( arg2 );
    r_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || r_lo < 0 || r_hi < 0 )
    {
        send_to_char( "Syntax: vassign <who> <low> <high>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They don't seem to be around.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) || get_trust( victim ) < LEVEL_CREATOR )
    {
        send_to_char( "They wouldn't know what to do with a vnum range.\n\r", ch );
        return;
    }

    if ( r_lo > r_hi )
    {
        send_to_char( "Unacceptable room range.\n\r", ch );
        return;
    }

    if ( r_lo == 0 )
        r_hi = 0;

    victim->pcdata->r_range_lo = r_lo;
    victim->pcdata->r_range_hi = r_hi;
    victim->pcdata->o_range_lo = r_lo;
    victim->pcdata->o_range_hi = r_hi;
    victim->pcdata->m_range_lo = r_lo;
    victim->pcdata->m_range_hi = r_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the vnum range %d - %d.\n\r",
               ch->name, r_lo, r_hi );
    assign_area( victim );  /* Put back by Thoric on 02/07/96 */

    if ( !victim->pcdata->area )
    {
        bug( "rassign: assign_area failed", 0 );
        return;
    }

    if ( r_lo == 0 )            /* Scryn 8/12/95 */
    {
        REMOVE_BIT ( victim->pcdata->area->status, AREA_LOADED );
        SET_BIT( victim->pcdata->area->status, AREA_DELETED );
    }
    else
    {
        SET_BIT( victim->pcdata->area->status, AREA_LOADED );
        REMOVE_BIT( victim->pcdata->area->status, AREA_DELETED );
    }

    return;
}

void do_oassign( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  o_lo, o_hi;
    CHAR_DATA* victim;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    o_lo = atoi( arg2 );
    o_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || o_lo < 0 || o_hi < 0 )
    {
        send_to_char( "Syntax: oassign <who> <low> <high>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They don't seem to be around.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) || get_trust( victim ) < LEVEL_SAVIOR )
    {
        send_to_char( "They wouldn't know what to do with an object range.\n\r", ch );
        return;
    }

    if ( o_lo > o_hi )
    {
        send_to_char( "Unacceptable object range.\n\r", ch );
        return;
    }

    victim->pcdata->o_range_lo = o_lo;
    victim->pcdata->o_range_hi = o_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the object vnum range %d - %d.\n\r",
               ch->name, o_lo, o_hi );
    return;
}

void do_massign( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  m_lo, m_hi;
    CHAR_DATA* victim;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    m_lo = atoi( arg2 );
    m_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || m_lo < 0 || m_hi < 0 )
    {
        send_to_char( "Syntax: massign <who> <low> <high>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They don't seem to be around.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) || get_trust( victim ) < LEVEL_SAVIOR )
    {
        send_to_char( "They wouldn't know what to do with a monster range.\n\r", ch );
        return;
    }

    if ( m_lo > m_hi )
    {
        send_to_char( "Unacceptable monster range.\n\r", ch );
        return;
    }

    victim->pcdata->m_range_lo = m_lo;
    victim->pcdata->m_range_hi = m_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the monster vnum range %d - %d.\n\r",
               ch->name, m_lo, m_hi );
    return;
}

void do_cmdtable( CHAR_DATA* ch, char* argument )
{
    int hash, cnt;
    CMDTYPE* cmd;
    set_pager_color( AT_PLAIN, ch );
    send_to_pager( "Commands and Number of Uses This Run\n\r", ch );

    for ( cnt = hash = 0; hash < 126; hash++ )
        for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
        {
            if ( ( ++cnt ) % 4 )
                pager_printf( ch, "%-6.6s %4d\t", cmd->name, cmd->userec.num_uses );
            else
                pager_printf( ch, "%-6.6s %4d\n\r", cmd->name, cmd->userec.num_uses );
        }

    return;
}

/*
    Load up a player file
*/
void do_loadup( CHAR_DATA* ch, char* argument )
{
    char fname[1024];
    char name[256];
    struct stat fst;
    DESCRIPTOR_DATA* d;
    int old_room_vnum;
    char buf[MAX_STRING_LENGTH];
    one_argument( argument, name );

    if ( name[0] == '\0' )
    {
        send_to_char( "Usage: loadup <playername>\n\r", ch );
        return;
    }

    name[0] = UPPER( name[0] );
    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower( name[0] ),
             capitalize( name ) );

    if ( stat( fname, &fst ) != -1 )
    {
        CREATE( d, DESCRIPTOR_DATA, 1 );
        d->next = NULL;
        d->prev = NULL;
        d->connected = CON_GET_NAME;
        d->outsize = 2000;
        CREATE( d->outbuf, char, d->outsize );
        load_char_obj( d, name, FALSE );
        add_char( d->character );
        old_room_vnum = d->character->in_room->vnum;
        char_to_room( d->character, ch->in_room );

        if ( get_trust( d->character ) >= get_trust( ch ) )
        {
            do_say( d->character, "Do *NOT* disturb me again!" );
            send_to_char( "I think you'd better leave that player alone!\n\r", ch );
            d->character->desc   = NULL;
            do_quit( d->character, "" );
            return;
        }

        d->character->desc  = NULL;
        d->character->retran    = old_room_vnum;
        d->character        = NULL;
        DISPOSE( d->outbuf );
        DISPOSE( d );
        ch_printf( ch, "Player %s loaded from room %d.\n\r", capitalize( name ), old_room_vnum );
        sprintf( buf, "%s appears from nowhere, eyes glazed over.\n\r", capitalize( name ) );
        act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    /* else no player file */
    send_to_char( "No such player.\n\r", ch );
    return;
}

void do_fixchar( CHAR_DATA* ch, char* argument )
{
    char name[MAX_STRING_LENGTH];
    CHAR_DATA* victim;
    one_argument( argument, name );

    if ( name[0] == '\0' )
    {
        send_to_char( "Usage: fixchar <playername>\n\r", ch );
        return;
    }

    victim = get_char_room( ch, name );

    if ( !victim )
    {
        send_to_char( "They're not here.\n\r", ch );
        return;
    }

    fix_char( victim );
    /*  victim->armor   = 100;
        victim->mod_str = 0;
        victim->mod_dex = 0;
        victim->mod_wis = 0;
        victim->mod_int = 0;
        victim->mod_con = 0;
        victim->mod_cha = 0;
        victim->mod_lck = 0;
        victim->damroll = 0;
        victim->hitroll = 0;
        victim->alignment   = URANGE( -1000, victim->alignment, 1000 );
        victim->saving_spell_staff = 0; */
    send_to_char( "Done.\n\r", ch );
}

void do_newbieset( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA* obj;
    CHAR_DATA* victim;
    argument = one_argument( argument, arg1 );
    argument = one_argument ( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "&zSyntax: &Cnewbieset <character> <class set>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( ( victim->top_level < 1 ) || ( victim->top_level > 5 ) )
    {
        send_to_char( "Level of victim must be 1 to 5.\n\r", ch );
        return;
    }

    obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_SHIELD ), 1 );
    obj_to_char( obj, victim );
    obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_DAGGER ), 1 );
    obj_to_char( obj, victim );
    /*  Added by Brittany, on Nov. 24, 1996. The object is the adventurer's
         guide to the realms of despair, part of academy.are. */
    {
        OBJ_INDEX_DATA* obj_ind = get_obj_index( 10333 );

        if ( obj_ind != NULL )
        {
            obj = create_object( obj_ind, 1 );
            obj_to_char( obj, victim );
        }
    }
    /*  Added the burlap sack to the newbieset.  The sack is part of sgate.are
        called Spectral Gate.  Brittany */
    {
        OBJ_INDEX_DATA* obj_ind = get_obj_index( 123 );

        if ( obj_ind != NULL )
        {
            obj = create_object( obj_ind, 1 );
            obj_to_char( obj, victim );
        }
    }
    act( AT_IMMORT, "$n has equipped you with a newbieset.", ch, NULL, victim, TO_VICT );
    ch_printf( ch, "You have re-equipped %s.\n\r", victim->name );
    return;
}

/*
    Extract area names from "input" string and place result in "output" string
    e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
    - Gorog
*/
void extract_area_names ( char* inp, char* out )
{
    char buf[MAX_INPUT_LENGTH], *pbuf = buf;
    int  len;
    *out = '\0';

    while ( inp && *inp )
    {
        inp = one_argument( inp, buf );

        if ( ( len = strlen( buf ) ) >= 5 && !strcmp( ".are", pbuf + len - 4 ) )
        {
            if ( *out )
                strcat ( out, " " );

            strcat ( out, buf );
        }
    }
}

/*
    Remove area names from "input" string and place result in "output" string
    e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
    - Gorog
*/
void remove_area_names ( char* inp, char* out )
{
    char buf[MAX_INPUT_LENGTH], *pbuf = buf;
    int  len;
    *out = '\0';

    while ( inp && *inp )
    {
        inp = one_argument( inp, buf );

        if ( ( len = strlen( buf ) ) < 5 || strcmp( ".are", pbuf + len - 4 ) )
        {
            if ( *out )
                strcat ( out, " " );

            strcat ( out, buf );
        }
    }
}

void do_bestowarea( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* victim;
    int  arg_len;
    argument = one_argument( argument, arg );

    if ( get_trust ( ch ) < LEVEL_SUB_IMPLEM )
    {
        send_to_char( "Sorry...\n\r", ch );
        return;
    }

    if ( !*arg )
    {
        send_to_char(
            "Syntax:\n\r"
            "bestowarea <victim> <filename>.are\n\r"
            "bestowarea <victim> none             removes bestowed areas\n\r"
            "bestowarea <victim> list             lists bestowed areas\n\r"
            "bestowarea <victim>                  lists bestowed areas\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world_full( ch, arg ) ) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "You can't give special abilities to a mob!\n\r", ch );
        return;
    }

    if ( get_trust( victim ) < LEVEL_IMMORTAL )
    {
        send_to_char( "They aren't an immortal.\n\r", ch );
        return;
    }

    if ( !victim->pcdata->bestowments )
        victim->pcdata->bestowments = str_dup( "" );

    if ( !*argument || !str_cmp ( argument, "list" ) )
    {
        extract_area_names ( victim->pcdata->bestowments, buf );
        ch_printf( ch, "Bestowed areas: %s\n\r", buf );
        return;
    }

    if ( !str_cmp ( argument, "none" ) )
    {
        remove_area_names ( victim->pcdata->bestowments, buf );
        DISPOSE( victim->pcdata->bestowments );
        victim->pcdata->bestowments = str_dup( buf );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    arg_len = strlen( argument );

    if ( arg_len < 3  )
    {
        send_to_char( "You can only bestow an area name\n\r", ch );
        send_to_char( "E.G. bestow joe sam.are\n\r", ch );
        return;
    }

    sprintf( buf, "%s %s", victim->pcdata->bestowments, argument );
    DISPOSE( victim->pcdata->bestowments );
    victim->pcdata->bestowments = str_dup( buf );
    ch_printf( victim, "%s has bestowed on you the area: %s\n\r",
               ch->name, argument );
    send_to_char( "Done.\n\r", ch );
}

void do_bestow( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* victim;
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Bestow whom with what?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "You can't give special abilities to a mob!\n\r", ch );
        return;
    }

    if ( get_trust( victim ) > get_trust( ch ) )
    {
        send_to_char( "You aren't powerful enough...\n\r", ch );
        return;
    }

    if ( !victim->pcdata->bestowments )
        victim->pcdata->bestowments = str_dup( "" );

    if ( argument[0] == '\0' || !str_cmp( argument, "list" ) )
    {
        ch_printf( ch, "Current bestowed commands on %s: %s.\n\r",
                   victim->name, victim->pcdata->bestowments );
        return;
    }

    if ( !str_cmp( argument, "none" ) )
    {
        DISPOSE( victim->pcdata->bestowments );
        victim->pcdata->bestowments = str_dup( "" );
        ch_printf( ch, "Bestowments removed from %s.\n\r", victim->name );
        ch_printf( victim, "%s has removed your bestowed commands.\n\r", ch->name );
        return;
    }

    sprintf( buf, "%s %s", victim->pcdata->bestowments, argument );
    DISPOSE( victim->pcdata->bestowments );
    victim->pcdata->bestowments = str_dup( buf );
    ch_printf( victim, "%s has bestowed on you the command(s): %s\n\r",
               ch->name, argument );
    send_to_char( "Done.\n\r", ch );
}

struct tm* update_time ( struct tm* old_time )
{
    time_t time;
    time = mktime( old_time );
    return localtime( &time );
}

void do_set_boot_time( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    bool check;
    check = FALSE;
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: setboot time {hour minute <day> <month> <year>}\n\r", ch );
        send_to_char( "        setboot manual {0/1}\n\r", ch );
        send_to_char( "        setboot default\n\r", ch );
        ch_printf( ch, "Boot time is currently set to %s, manual bit is set to %d\n\r", reboot_time, set_boot_time->manual );
        return;
    }

    if ( !str_cmp( arg, "time" ) )
    {
        struct tm* now_time;
        argument = one_argument( argument, arg );
        argument = one_argument( argument, arg1 );

        if ( !*arg || !*arg1 || !is_number( arg ) || !is_number( arg1 ) )
        {
            send_to_char( "You must input a value for hour and minute.\n\r", ch );
            return;
        }

        now_time = localtime( &current_time );

        if ( ( now_time->tm_hour = atoi( arg ) ) < 0 || now_time->tm_hour > 23 )
        {
            send_to_char( "Valid range for hour is 0 to 23.\n\r", ch );
            return;
        }

        if ( ( now_time->tm_min = atoi( arg1 ) ) < 0 || now_time->tm_min > 59 )
        {
            send_to_char( "Valid range for minute is 0 to 59.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg );

        if ( *arg != '\0' && is_number( arg ) )
        {
            if ( ( now_time->tm_mday = atoi( arg ) ) < 1 || now_time->tm_mday > 31 )
            {
                send_to_char( "Valid range for day is 1 to 31.\n\r", ch );
                return;
            }

            argument = one_argument( argument, arg );

            if ( *arg != '\0' && is_number( arg ) )
            {
                if ( ( now_time->tm_mon = atoi( arg ) ) < 1 || now_time->tm_mon > 12 )
                {
                    send_to_char( "Valid range for month is 1 to 12.\n\r", ch );
                    return;
                }

                now_time->tm_mon--;
                argument = one_argument( argument, arg );

                if ( ( now_time->tm_year = atoi( arg ) - 1900 ) < 0 ||
                        now_time->tm_year > 199 )
                {
                    send_to_char( "Valid range for year is 1900 to 2099.\n\r", ch );
                    return;
                }
            }
        }

        now_time->tm_sec = 0;

        if ( mktime( now_time ) < current_time )
        {
            send_to_char( "You can't set a time previous to today!\n\r", ch );
            return;
        }

        if ( set_boot_time->manual == 0 )
            set_boot_time->manual = 1;

        new_boot_time = update_time( now_time );
        new_boot_struct = *new_boot_time;
        new_boot_time = &new_boot_struct;
        reboot_check( mktime( new_boot_time ) );
        get_reboot_string();
        ch_printf( ch, "Boot time set to %s\n\r", reboot_time );
        check = TRUE;
    }
    else if ( !str_cmp( arg, "manual" ) )
    {
        argument = one_argument( argument, arg1 );

        if ( arg1[0] == '\0' )
        {
            send_to_char( "Please enter a value for manual boot on/off\n\r", ch );
            return;
        }

        if ( !is_number( arg1 ) )
        {
            send_to_char( "Value for manual must be 0 (off) or 1 (on)\n\r", ch );
            return;
        }

        if ( atoi( arg1 ) < 0 || atoi( arg1 ) > 1 )
        {
            send_to_char( "Value for manual must be 0 (off) or 1 (on)\n\r", ch );
            return;
        }

        set_boot_time->manual = atoi( arg1 );
        ch_printf( ch, "Manual bit set to %s\n\r", arg1 );
        check = TRUE;
        get_reboot_string();
        return;
    }
    else if ( !str_cmp( arg, "default" ) )
    {
        set_boot_time->manual = 0;
        /* Reinitialize new_boot_time */
        new_boot_time = localtime( &current_time );
        new_boot_time->tm_mday += 1;

        if ( new_boot_time->tm_hour > 12 )
            new_boot_time->tm_mday += 1;

        new_boot_time->tm_hour = 6;
        new_boot_time->tm_min = 0;
        new_boot_time->tm_sec = 0;
        new_boot_time = update_time( new_boot_time );
        sysdata.DENY_NEW_PLAYERS = FALSE;
        send_to_char( "Reboot time set back to normal.\n\r", ch );
        check = TRUE;
    }

    if ( !check )
    {
        send_to_char( "Invalid argument for setboot.\n\r", ch );
        return;
    }
    else
    {
        get_reboot_string();
        new_boot_time_t = mktime( new_boot_time );
    }
}
/*  Online high level immortal command for displaying what the encryption
    of a name/password would be, taking in 2 arguments - the name and the
    password - can still only change the password if you have access to
    pfiles and the correct password
*/
void do_form_password( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_STRING_LENGTH];
    argument = one_argument( argument, arg );
    ch_printf( ch, "Those two arguments encrypted would result in: %s",
               crypt( arg, argument ) );
    return;
}

/*
    Purge a player file.  No more player.  -- Altrag
*/
void do_destro( CHAR_DATA* ch, char* argument )
{
    set_char_color( AT_RED, ch );
    send_to_char( "If you want to destroy a character, spell it out!\n\r", ch );
    return;
}

/*
    This could have other applications too.. move if needed. -- Altrag
*/
void close_area( AREA_DATA* pArea )
{
    extern ROOM_INDEX_DATA* room_index_hash[MAX_KEY_HASH];
    extern OBJ_INDEX_DATA*   obj_index_hash[MAX_KEY_HASH];
    extern MOB_INDEX_DATA*   mob_index_hash[MAX_KEY_HASH];
    CHAR_DATA* ech;
    CHAR_DATA* ech_next;
    OBJ_DATA* eobj;
    OBJ_DATA* eobj_next;
    int icnt;
    ROOM_INDEX_DATA* rid;
    ROOM_INDEX_DATA* rid_next;
    OBJ_INDEX_DATA* oid;
    OBJ_INDEX_DATA* oid_next;
    MOB_INDEX_DATA* mid;
    MOB_INDEX_DATA* mid_next;
    RESET_DATA* ereset;
    RESET_DATA* ereset_next;
    EXTRA_DESCR_DATA* eed;
    EXTRA_DESCR_DATA* eed_next;
    EXIT_DATA* exit;
    EXIT_DATA* exit_next;
    MPROG_ACT_LIST* mpact;
    MPROG_ACT_LIST* mpact_next;
    MPROG_DATA* mprog;
    MPROG_DATA* mprog_next;
    AFFECT_DATA* paf;
    AFFECT_DATA* paf_next;
    bool tf = FALSE;

    for ( ech = first_char; ech; ech = ech_next )
    {
        ech_next = ech->next;

        if ( IS_NPC( ech ) )
        {
            /* if mob is in area, or part of area. */
            if ( URANGE( pArea->low_m_vnum, ech->pIndexData->vnum,
                         pArea->hi_m_vnum ) == ech->pIndexData->vnum ||
                    ( ech->in_room && ech->in_room->area == pArea ) )
                extract_char( ech, TRUE, FALSE );

            continue;
        }

        if ( ech->in_room && ech->in_room->area == pArea )
            do_recall( ech, "" );
    }

    for ( eobj = first_object; eobj; eobj = eobj_next )
    {
        eobj_next = eobj->next;

        /* if obj is in area, or part of area. */
        if ( URANGE( pArea->low_o_vnum, eobj->pIndexData->vnum,
                     pArea->hi_o_vnum ) == eobj->pIndexData->vnum ||
                ( eobj->in_room && eobj->in_room->area == pArea ) )
            extract_obj( eobj );
    }

    for ( icnt = 0; icnt < MAX_KEY_HASH; icnt++ )
    {
        for ( rid = room_index_hash[icnt]; rid; rid = rid_next )
        {
            rid_next = rid->next;

            for ( exit = rid->first_exit; exit; exit = exit_next )
            {
                exit_next = exit->next;
                tf = TRUE;

                if ( !exit )
                    tf = FALSE;
                else if ( !exit->to_room )
                    tf = FALSE;
                else if ( !exit->to_room->area )
                    tf = FALSE;
                else if ( exit->to_room->area == pArea )
                    tf = TRUE;
                else if ( rid->area == pArea )
                    tf = TRUE;

                if ( tf )
                {
                    STRFREE( exit->keyword );
                    STRFREE( exit->description );
                    UNLINK( exit, rid->first_exit, rid->last_exit, next, prev );
                    DISPOSE( exit );
                }
            }

            if ( rid->area != pArea )
                continue;

            STRFREE( rid->name );
            STRFREE( rid->description );
            STRFREE( rid->hdescription );

            if ( rid->first_person )
            {
                bug( "close_area: room with people #%d", rid->vnum );

                for ( ech = rid->first_person; ech; ech = ech_next )
                {
                    ech_next = ech->next_in_room;

                    if ( IS_NPC( ech ) )
                        extract_char( ech, TRUE, FALSE );
                    else
                        do_recall( ech, "" );
                }
            }

            if ( rid->first_content )
            {
                bug( "close_area: room with contents #%d", rid->vnum );

                for ( eobj = rid->first_content; eobj; eobj = eobj_next )
                {
                    eobj_next = eobj->next_content;
                    extract_obj( eobj );
                }
            }

            for ( eed = rid->first_extradesc; eed; eed = eed_next )
            {
                eed_next = eed->next;
                STRFREE( eed->keyword );
                STRFREE( eed->description );
                DISPOSE( eed );
            }

            for ( mpact = rid->mpact; mpact; mpact = mpact_next )
            {
                mpact_next = mpact->next;
                STRFREE( mpact->buf );
                DISPOSE( mpact );
            }

            for ( mprog = rid->mudprogs; mprog; mprog = mprog_next )
            {
                mprog_next = mprog->next;
                STRFREE( mprog->arglist );
                STRFREE( mprog->comlist );
                DISPOSE( mprog );
            }

            if ( rid == room_index_hash[icnt] )
                room_index_hash[icnt] = rid->next;
            else
            {
                ROOM_INDEX_DATA* trid;

                for ( trid = room_index_hash[icnt]; trid; trid = trid->next )
                    if ( trid->next == rid )
                        break;

                if ( !trid )
                    bug( "Close_area: rid not in hash list %d", rid->vnum );
                else
                    trid->next = rid->next;
            }

            DISPOSE( rid );
        }

        for ( mid = mob_index_hash[icnt]; mid; mid = mid_next )
        {
            mid_next = mid->next;

            if ( mid->vnum < pArea->low_m_vnum || mid->vnum > pArea->hi_m_vnum )
                continue;

            STRFREE( mid->player_name );
            STRFREE( mid->short_descr );
            STRFREE( mid->long_descr  );
            STRFREE( mid->description );

            if ( mid->pShop )
            {
                UNLINK( mid->pShop, first_shop, last_shop, next, prev );
                DISPOSE( mid->pShop );
            }

            for ( mprog = mid->mudprogs; mprog; mprog = mprog_next )
            {
                mprog_next = mprog->next;
                STRFREE( mprog->arglist );
                STRFREE( mprog->comlist );
                DISPOSE( mprog );
            }

            if ( mid == mob_index_hash[icnt] )
                mob_index_hash[icnt] = mid->next;
            else
            {
                MOB_INDEX_DATA* tmid;

                for ( tmid = mob_index_hash[icnt]; tmid; tmid = tmid->next )
                    if ( tmid->next == mid )
                        break;

                if ( !tmid )
                    bug( "Close_area: mid not in hash list %s", mid->vnum );
                else
                    tmid->next = mid->next;
            }

            DISPOSE( mid );
        }

        for ( oid = obj_index_hash[icnt]; oid; oid = oid_next )
        {
            oid_next = oid->next;

            if ( oid->vnum < pArea->low_o_vnum || oid->vnum > pArea->hi_o_vnum )
                continue;

            STRFREE( oid->name );
            STRFREE( oid->short_descr );
            STRFREE( oid->description );
            STRFREE( oid->action_desc );

            for ( eed = oid->first_extradesc; eed; eed = eed_next )
            {
                eed_next = eed->next;
                STRFREE( eed->keyword );
                STRFREE( eed->description );
                DISPOSE( eed );
            }

            for ( paf = oid->first_affect; paf; paf = paf_next )
            {
                paf_next = paf->next;
                DISPOSE( paf );
            }

            for ( mprog = oid->mudprogs; mprog; mprog = mprog_next )
            {
                mprog_next = mprog->next;
                STRFREE( mprog->arglist );
                STRFREE( mprog->comlist );
                DISPOSE( mprog );
            }

            if ( oid == obj_index_hash[icnt] )
                obj_index_hash[icnt] = oid->next;
            else
            {
                OBJ_INDEX_DATA* toid;

                for ( toid = obj_index_hash[icnt]; toid; toid = toid->next )
                    if ( toid->next == oid )
                        break;

                if ( !toid )
                    bug( "Close_area: oid not in hash list %s", oid->vnum );
                else
                    toid->next = oid->next;
            }

            DISPOSE( oid );
        }
    }

    for ( ereset = pArea->first_reset; ereset; ereset = ereset_next )
    {
        ereset_next = ereset->next;
        DISPOSE( ereset );
    }

    DISPOSE( pArea->name );
    DISPOSE( pArea->filename );
    STRFREE( pArea->author );
    UNLINK( pArea, first_build, last_build, next, prev );
    UNLINK( pArea, first_asort, last_asort, next_sort, prev_sort );
    DISPOSE( pArea );
}

void do_destroy( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;
    char buf[MSL], buf2[SUPER_MSL], buf3[SUB_MSL], arg[MIL];
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Destroy what player file?\n\r", ch );
        return;
    }

    for ( victim = first_char; victim; victim = victim->next )
        if ( !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
            break;

    if ( !victim )
    {
        DESCRIPTOR_DATA* d;

        /* Make sure they aren't halfway logged in. */
        for ( d = first_descriptor; d; d = d->next )
            if ( ( victim = d->character ) && !IS_NPC( victim ) &&
                    !str_cmp( victim->name, arg ) )
                break;

        if ( d )
            close_socket( d, TRUE );
    }
    else
    {
        int x, y;
        quitting_char = victim;
        save_char_obj( victim );
        saving_char = NULL;
        extract_char( victim, TRUE, FALSE );

        for ( x = 0; x < MAX_WEAR; x++ )
            for ( y = 0; y < MAX_LAYERS; y++ )
            {
                save_equipment[0][x][y] = NULL;
                save_equipment[1][x][y] = NULL;
            }
    }

    if ( !delete_player( arg ) )
    {
        AREA_DATA* pArea;
        set_char_color( AT_RED, ch );
        send_to_char( "Player destroyed.  Pfile saved in backup directory.\n\r", ch );
        snprintf( buf, MSL, "%s%s", GOD_DIR, capitalize( arg ) );

        if ( !remove( buf ) )
            send_to_char( "Player's immortal data destroyed.\n\r", ch );
        else if ( errno != ENOENT )
        {
            ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric.\n\r",
                       errno, strerror( errno ) );
            snprintf( buf2, SUPER_MSL, "%s destroying %s", ch->name, buf );
            perror( buf2 );
        }

        snprintf( buf3, SUB_MSL, "%s.are", capitalize( arg ) );

        for ( pArea = first_build; pArea; pArea = pArea->next )
            if ( !strcmp( pArea->filename, buf3 ) )
            {
                snprintf( buf, MSL, "%s%s", BUILD_DIR, buf3 );

                if ( IS_SET( pArea->status, AREA_LOADED ) )
                    fold_area( pArea, buf, FALSE );

                close_area( pArea );
                snprintf( buf2, SUPER_MSL, "%s.bak", buf );
                set_char_color( AT_RED, ch ); /* Log message changes colors */

                if ( !rename( buf, buf2 ) )
                    send_to_char( "Player's area data destroyed.  Area saved as backup.\n\r", ch );
                else if ( errno != ENOENT )
                {
                    ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Thoric.\n\r",
                               errno, strerror( errno ) );
                    snprintf( buf2, SUPER_MSL, "%s destroying %s", ch->name, buf );
                    perror( buf2 );
                }
            }
    }
    else if ( errno == ENOENT )
    {
        set_char_color( AT_PLAIN, ch );
        send_to_char( "Player does not exist.\n\r", ch );
    }
    else
    {
        set_char_color( AT_WHITE, ch );
        ch_printf( ch, "Unknown error #%d - %s.  Report to Thoric.\n\r",
                   errno, strerror( errno ) );
        snprintf( buf, MSL, "%s destroying %s", ch->name, arg );
        perror( buf );
    }

    return;
}
extern ROOM_INDEX_DATA*        room_index_hash         [MAX_KEY_HASH]; /* db.c */


/*  Super-AT command:

    FOR ALL <action>
    FOR MORTALS <action>
    FOR GODS <action>
    FOR MOBS <action>
    FOR EVERYWHERE <action>


    Executes action several times, either on ALL players (not including yourself),
    MORTALS (including trusted characters), GODS (characters with level higher than
    L_HERO), MOBS (Not recommended) or every room (not recommended either!)

    If you insert a # in the action, it will be replaced by the name of the target.

    If # is a part of the action, the action will be executed for every target
    in game. If there is no #, the action will be executed for every room containg
    at least one target, but only once per room. # cannot be used with FOR EVERY-
    WHERE. # can be anywhere in the action.

    Example:

    FOR ALL SMILE -> you will only smile once in a room with 2 players.
    FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

    Destroying the characters this command acts upon MAY cause it to fail. Try to
    avoid something like FOR MOBS PURGE (although it actually works at my MUD).

    FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
    though :)

    The command works by transporting the character to each of the rooms with
    target in them. Private rooms are not violated.

*/

/*  Expand the name of a character into a string that identifies THAT
    character within a room. E.g. the second 'guard' -> 2. guard
*/
const char* name_expand ( CHAR_DATA* ch )
{
    int count = 1;
    CHAR_DATA* rch;
    char name[MIL]; /*  HOPEFULLY no mob has a name longer than THAT */
    static char outbuf[MSL];
    strncpy( outbuf, "", MSL );

    if ( !IS_NPC( ch ) )
        return ch->name;

    one_argument ( ch->name, name ); /* copy the first word into name */

    if ( NULLSTR( name ) ) /* weird mob .. no keywords */
    {
        strncpy( outbuf, "", MSL ); /* Do not return NULL, just an empty buffer */
        return outbuf;
    }

    /* ->people changed to ->first_person -- TRI */
    for ( rch = ch->in_room->first_person; rch && ( rch != ch ); rch =
                rch->next_in_room )
        if ( is_name ( name, rch->name ) )
            count++;

    snprintf( outbuf, MSL, "%d.%s", count, name );
    return outbuf;
}


void do_for ( CHAR_DATA* ch, char* argument )
{
    char range[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
    ROOM_INDEX_DATA* room, *old_room;
    CHAR_DATA* p, *p_prev;  /* p_next to p_prev -- TRI */
    int i;
    argument = one_argument ( argument, range );

    if ( !range[0] || !argument[0] ) /* invalid usage? */
    {
        do_help ( ch, "for" );
        return;
    }

    if ( !str_prefix( "quit", argument ) )
    {
        send_to_char ( "Are you trying to crash the MUD or something?\n\r", ch );
        return;
    }

    if ( !str_cmp ( range, "all" ) )
    {
        fMortals = TRUE;
        fGods = TRUE;
    }
    else if ( !str_cmp ( range, "gods" ) )
        fGods = TRUE;
    else if ( !str_cmp ( range, "mortals" ) )
        fMortals = TRUE;
    else if ( !str_cmp ( range, "mobs" ) )
        fMobs = TRUE;
    else if ( !str_cmp ( range, "everywhere" ) )
        fEverywhere = TRUE;
    else
        do_help ( ch, "for" ); /* show syntax */

    /* do not allow # to make it easier */
    if ( fEverywhere && strchr ( argument, '#' ) )
    {
        send_to_char ( "Cannot use FOR EVERYWHERE with the # thingie.\n\r", ch );
        return;
    }

    if ( strchr ( argument, '#' ) ) /* replace # ? */
    {
        /* char_list - last_char, p_next - gch_prev -- TRI */
        for ( p = last_char; p ; p = p_prev )
        {
            p_prev = p->prev;  /* TRI */
            /*  p_next = p->next; */ /* In case someone DOES try to AT MOBS SLAY # */
            found = FALSE;

            if ( !( p->in_room ) || ( p == ch ) )
                continue;

            if ( IS_NPC( p ) && fMobs )
                found = TRUE;
            else if ( !IS_NPC( p ) && get_trust( p ) >= LEVEL_IMMORTAL && fGods )
                found = TRUE;
            else if ( !IS_NPC( p ) && get_trust( p ) < LEVEL_IMMORTAL && fMortals )
                found = TRUE;

            /* It looks ugly to me.. but it works :) */
            if ( found ) /* p is 'appropriate' */
            {
                char* pSource = argument; /* head of buffer to be parsed */
                char* pDest = buf; /* parse into this */

                while ( *pSource )
                {
                    if ( *pSource == '#' ) /* Replace # with name of target */
                    {
                        const char* namebuf = name_expand ( p );

                        if ( namebuf ) /* in case there is no mob name ?? */
                            while ( *namebuf ) /* copy name over */
                                *( pDest++ ) = *( namebuf++ );

                        pSource++;
                    }
                    else
                        *( pDest++ ) = *( pSource++ );
                } /* while */

                *pDest = '\0'; /* Terminate */
                /* Execute */
                old_room = ch->in_room;
                char_from_room ( ch );
                char_to_room ( ch, p->in_room );
                interpret ( ch, buf, FALSE );
                char_from_room ( ch );
                char_to_room ( ch, old_room );
            } /* if found */
        } /* for every char */
    }
    else /* just for every room with the appropriate people in it */
    {
        for ( i = 0; i < MAX_KEY_HASH; i++ ) /* run through all the buckets */
            for ( room = room_index_hash[i] ; room ; room = room->next )
            {
                found = FALSE;

                /* Anyone in here at all? */
                if ( fEverywhere ) /* Everywhere executes always */
                    found = TRUE;
                else if ( !room->first_person ) /* Skip it if room is empty */
                    continue;

                /* ->people changed to first_person -- TRI */

                /* Check if there is anyone here of the requried type */
                /* Stop as soon as a match is found or there are no more ppl in room */
                /* ->people to ->first_person -- TRI */
                for ( p = room->first_person; p && !found; p = p->next_in_room )
                {
                    if ( p == ch ) /* do not execute on oneself */
                        continue;

                    if ( IS_NPC( p ) && fMobs )
                        found = TRUE;
                    else if ( !IS_NPC( p ) && ( get_trust( p ) >= LEVEL_IMMORTAL ) && fGods )
                        found = TRUE;
                    else if ( !IS_NPC( p ) && ( get_trust( p ) <= LEVEL_IMMORTAL ) && fMortals )
                        found = TRUE;
                } /* for everyone inside the room */

                if ( found ) /* Any of the required type here AND room not private? */
                {
                    /*  This may be ineffective. Consider moving character out of old_room
                        once at beginning of command then moving back at the end.
                        This however, is more safe?
                    */
                    old_room = ch->in_room;
                    char_from_room ( ch );
                    char_to_room ( ch, room );
                    interpret ( ch, argument, FALSE );
                    char_from_room ( ch );
                    char_to_room ( ch, old_room );
                } /* if found */
            } /* for every room in a bucket */
    } /* if strchr */
} /* do_for */

void save_sysdata  args( ( SYSTEM_DATA sys ) );

void do_cset( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_STRING_LENGTH];
    sh_int level;
    set_char_color( AT_IMMORT, ch );

    if ( argument[0] == '\0' )
    {
        ch_printf( ch, "Mail:\n\r" );
        ch_printf( ch, "  Read all mail: %d. Read mail for free: %d. Write mail for free: %d.\n\r", sysdata.read_all_mail, sysdata.read_mail_free, sysdata.write_mail_free );
        ch_printf( ch, "  Take all mail: %d\n\r", sysdata.take_others_mail );
        ch_printf( ch, "Pfile autocleanup status: %s  Days before purging newbies: %d\n\r",
                   sysdata.CLEANPFILES ? "On" : "Off",
                   sysdata.newbie_purge );
        ch_printf( ch, "Days before purging regular players: %d\n\r",
                   sysdata.regular_purge );
        ch_printf( ch, "TMC Blocking module: %s\n\r", sysdata.TMCBLOCK ? "On" : "Off" );
        ch_printf( ch, "Channels:\n\r" );
        ch_printf( ch, "  Muse: %d. Think: %d. Log: %d. Build: %d.\n\r", sysdata.muse_level, sysdata.think_level, sysdata.log_level, sysdata.build_level );
        ch_printf( ch, "Building:\n\r" );
        ch_printf( ch, "  Prototype modification: %d.  Player msetting: %d.\n\r", sysdata.level_modify_proto, sysdata.level_mset_player );
        ch_printf( ch, "Guilds:\n\r" );
        ch_printf( ch, "  Overseer: %s.  Advisor: %s.\n\r", sysdata.guild_overseer, sysdata.guild_advisor );
        ch_printf( ch, "Other:\n\r" );
        ch_printf( ch, "  Force on players: %d.  ", sysdata.level_forcepc );
        ch_printf( ch, "Private room override: %d.\n\r", sysdata.level_override_private );
        ch_printf( ch, "  Penalty to regular stun chance: %d.  ", sysdata.stun_regular );
        ch_printf( ch, "Penalty to stun plr vs. plr: %d.\n\r", sysdata.stun_plr_vs_plr );
        ch_printf( ch, "  Percent damage plr vs. plr: %3d.  ", sysdata.dam_plr_vs_plr );
        ch_printf( ch, "Percent damage plr vs. mob: %d.\n\r", sysdata.dam_plr_vs_mob );
        ch_printf( ch, "  Percent damage mob vs. plr: %3d.  ", sysdata.dam_mob_vs_plr );
        ch_printf( ch, "Percent damage mob vs. mob: %d.\n\r", sysdata.dam_mob_vs_mob );
        ch_printf( ch, "  Get object without take flag: %d.  ", sysdata.level_getobjnotake );
        ch_printf( ch, "Autosave frequency (minutes): %d.\n\r", sysdata.save_frequency );
        ch_printf( ch, "  Save flags: %s\n\r", old_flag_string( sysdata.save_flags, save_flag ) );
        return;
    }

    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "remap" ) )
    {
        send_to_char( "&RRemapping active areas...\n\r", ch );
        deploy_map( );
        return;
    }

    if ( !str_cmp( arg, "help" ) )
    {
        do_help( ch, "controls" );
        return;
    }

    if ( !str_cmp( arg, "save" ) )
    {
        save_sysdata( sysdata );
        return;
    }

    if ( !str_cmp( arg, "exe" ) )
    {
        if ( sysdata.exe_file )
            STRFREE( sysdata.exe_file );

        sysdata.exe_file = STRALLOC( argument );
        ch_printf( ch, "EXE changed to %s.\n\r", sysdata.exe_file );
        return;
    }

    if ( !str_cmp( arg, "mqtt_host" ) )
    {
        if ( sysdata.mqtt_host )
            STRFREE( sysdata.mqtt_host );

        sysdata.mqtt_host = STRALLOC( argument );
        ch_printf( ch, "MQTT host changed to %s.\n\r", sysdata.mqtt_host );
        return;
    }

    if ( !str_cmp( arg, "mqtt_port" ) )
    {
        sysdata.mqtt_port = atoi( argument );
        ch_printf( ch, "MQTT port changed to %d.\n\r", sysdata.mqtt_port );
        return;
    }

    if ( !str_cmp( arg, "mqtt_enabled" ) ) {
        sysdata.mqtt_enabled = !sysdata.mqtt_enabled;
        ch_printf( ch, "MQTT enabled changed to %s.\n\r", sysdata.mqtt_enabled ? "true" : "false" );
        return;
    }

    if ( !str_cmp( arg, "pfiles" ) )
    {
        sysdata.CLEANPFILES = !sysdata.CLEANPFILES;

        if ( sysdata.CLEANPFILES )
            send_to_char( "Pfile autocleanup enabled.\n\r", ch );
        else
            send_to_char( "Pfile autocleanup disabled.\n\r", ch );

        return;
    }

    if ( !str_cmp( arg, "tmcblock" ) )
    {
        sysdata.TMCBLOCK = !sysdata.TMCBLOCK;

        if ( sysdata.TMCBLOCK )
            send_to_char( "TMC Blocking enabled.\n\r", ch );
        else
            send_to_char( "TMC Blocking disabled.\n\r", ch );

        return;
    }

    if ( !str_cmp( arg, "saveflag" ) )
    {
        int x = get_saveflag( argument );

        if ( x == -1 )
            send_to_char( "Not a save flag.\n\r", ch );
        else
        {
            TOGGLE_BIT( sysdata.save_flags, 1 << x );
            send_to_char( "Ok.\n\r", ch );
        }

        return;
    }

    if ( !str_prefix( arg, "guild_overseer" ) )
    {
        STRFREE( sysdata.guild_overseer );
        sysdata.guild_overseer = str_dup( argument );
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_prefix( arg, "guild_advisor" ) )
    {
        STRFREE( sysdata.guild_advisor );
        sysdata.guild_advisor = str_dup( argument );
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    level = ( sh_int ) atoi( argument );

    if ( !str_prefix( arg, "savefrequency" ) )
    {
        sysdata.save_frequency = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "currid" ) )
    {
        sysdata.currid = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "newbie_purge" ) )
    {
        if ( level < 0 )
        {
            send_to_char( "You must specify a period of no less than 0 days.\n\r", ch );
            return;
        }

        sysdata.newbie_purge = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "regular_purge" ) )
    {
        if ( level < 0 )
        {
            send_to_char( "You must specify a period of no less than 0 days.\n\r", ch );
            return;
        }

        sysdata.regular_purge = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "stun" ) )
    {
        sysdata.stun_regular = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "stun_pvp" ) )
    {
        sysdata.stun_plr_vs_plr = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "dam_pvp" ) )
    {
        sysdata.dam_plr_vs_plr = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "get_notake" ) )
    {
        sysdata.level_getobjnotake = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "dam_pvm" ) )
    {
        sysdata.dam_plr_vs_mob = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "dam_mvp" ) )
    {
        sysdata.dam_mob_vs_plr = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "dam_mvm" ) )
    {
        sysdata.dam_mob_vs_mob = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( level < 0 || level > MAX_LEVEL )
    {
        send_to_char( "Invalid value for new control.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "read_all" ) )
    {
        sysdata.read_all_mail = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "read_free" ) )
    {
        sysdata.read_mail_free = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "write_free" ) )
    {
        sysdata.write_mail_free = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "take_all" ) )
    {
        sysdata.take_others_mail = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "muse" ) )
    {
        sysdata.muse_level = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "think" ) )
    {
        sysdata.think_level = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "log" ) )
    {
        sysdata.log_level = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "build" ) )
    {
        sysdata.build_level = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "proto_modify" ) )
    {
        sysdata.level_modify_proto = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "override_private" ) )
    {
        sysdata.level_override_private = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "forcepc" ) )
    {
        sysdata.level_forcepc = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "mset_player" ) )
    {
        sysdata.level_mset_player = level;
        send_to_char( "Ok.\n\r", ch );
        return;
    }
    else
    {
        send_to_char( "Invalid argument.\n\r", ch );
        return;
    }
}

void get_reboot_string( void )
{
    sprintf( reboot_time, "%s", asctime( new_boot_time ) );
}


void do_orange( CHAR_DATA* ch, char* argument )
{
    send_to_char( "Function under construction.\n\r", ch );
    return;
}

void do_mrange( CHAR_DATA* ch, char* argument )
{
    send_to_char( "Function under construction.\n\r", ch );
    return;
}

void do_hell( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;
    char arg[MAX_INPUT_LENGTH];
    sh_int time;
    bool h_d = FALSE;
    struct tm* tms;
    argument = one_argument( argument, arg );

    if ( !*arg )
    {
        send_to_char( "Hell who, and for how long?\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world_full( ch, arg ) ) || IS_NPC( victim ) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_IMMORTAL( victim ) )
    {
        send_to_char( "There is no point in helling an immortal.\n\r", ch );
        return;
    }

    if ( victim->pcdata->release_date != 0 )
    {
        ch_printf( ch, "They are already in hell until %24.24s, by %s.\n\r",
                   ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );
        return;
    }

    argument = one_argument( argument, arg );

    if ( !*arg || !is_number( arg ) )
    {
        send_to_char( "Hell them for how long?\n\r", ch );
        return;
    }

    time = atoi( arg );

    if ( time <= 0 )
    {
        send_to_char( "You cannot hell for zero or negative time.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( !*arg || !str_prefix( arg, "hours" ) )
        h_d = TRUE;
    else if ( str_prefix( arg, "days" ) )
    {
        send_to_char( "Is that value in hours or days?\n\r", ch );
        return;
    }
    else if ( time > 30 )
    {
        send_to_char( "You may not hell a person for more than 30 days at a time.\n\r", ch );
        return;
    }

    tms = localtime( &current_time );

    if ( h_d )
        tms->tm_hour += time;
    else
        tms->tm_mday += time;

    victim->pcdata->release_date = mktime( tms );
    victim->pcdata->helled_by = STRALLOC( ch->name );
    ch_printf( ch, "%s will be released from hell at %24.24s.\n\r", victim->name,
               ctime( &victim->pcdata->release_date ) );
    act( AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT );
    char_from_room( victim );
    char_to_room( victim, get_room_index( 6 ) );
    act( AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOTVICT );
    do_look( victim, "auto" );
    ch_printf( victim, "The immortals are not pleased with your actions.\n\r"
               "You shall remain in hell for %d %s%s.\n\r", time,
               ( h_d ? "hour" : "day" ), ( time == 1 ? "" : "s" ) );
    save_char_obj( victim );  /* used to save ch, fixed by Thoric 09/17/96 */
    return;
}

void do_unhell( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* location;
    argument = one_argument( argument, arg );

    if ( !*arg )
    {
        send_to_char( "Unhell whom..?\n\r", ch );
        return;
    }

    location = ch->in_room;
    ch->in_room = get_room_index( 6 );
    victim = get_char_room( ch, arg );
    ch->in_room = location;            /* The case of unhell self, etc. */

    if ( !victim || IS_NPC( victim ) || victim->in_room->vnum != 6 )
    {
        send_to_char( "No one like that is in hell.\n\r", ch );
        return;
    }

    location = get_room_index( 50000 );

    if ( !location )
        location = ch->in_room;

    MOBtrigger = FALSE;
    act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
    char_from_room( victim );
    char_to_room( victim, location );
    send_to_char( "The gods have smiled on you and released you from hell early!\n\r", victim );
    do_look( victim, "auto" );
    send_to_char( "They have been released.\n\r", ch );

    if ( victim->pcdata->helled_by )
    {
        if ( str_cmp( ch->name, victim->pcdata->helled_by ) )
            ch_printf( ch, "(You should probably write a note to %s, explaining the early release.)\n\r",
                       victim->pcdata->helled_by );

        STRFREE( victim->pcdata->helled_by );
        victim->pcdata->helled_by = NULL;
    }

    MOBtrigger = FALSE;
    act( AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
    victim->pcdata->release_date = 0;
    save_char_obj( victim );
    return;
}

/* Vnum search command by Swordbearer */
void do_vsearch( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE;
    OBJ_DATA* obj;
    OBJ_DATA* in_obj;
    int obj_counter = 1;
    int argi;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax:  vsearch <vnum>.\n\r", ch );
        return;
    }

    set_pager_color( AT_PLAIN, ch );
    argi = atoi( arg );

    if ( argi < 0 && argi > 20000 )
    {
        send_to_char( "Vnum out of range.\n\r", ch );
        return;
    }

    for ( obj = first_object; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !( argi == obj->pIndexData->vnum ) )
            continue;

        found = TRUE;

        for ( in_obj = obj; in_obj->in_obj != NULL;
                in_obj = in_obj->in_obj );

        if ( in_obj->carried_by != NULL )
            pager_printf( ch, "[%2d] Level %d %s carried by %s.\n\r",
                          obj_counter,
                          obj->level, obj_short( obj ),
                          PERS( in_obj->carried_by, ch ) );
        else
            pager_printf( ch, "[%2d] [%-5d] %s in %s.\n\r", obj_counter,
                          ( ( in_obj->in_room ) ? in_obj->in_room->vnum : 0 ),
                          obj_short( obj ), ( in_obj->in_room == NULL ) ?
                          "somewhere" : in_obj->in_room->name );

        obj_counter++;
    }

    if ( !found )
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}

/*
    Simple function to let any imm make any player instantly sober.
    Saw no need for level restrictions on this.
    Written by Narn, Apr/96
*/
void do_sober( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;
    char arg1 [MAX_INPUT_LENGTH];
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on mobs.\n\r", ch );
        return;
    }

    send_to_char( "Ok.\n\r", ch );
    send_to_char( "You feel sober again.\n\r", victim );
    return;
}


/*
    Free a social structure                  -Thoric
*/
void free_social( SOCIALTYPE* social )
{
    if ( social->name )
        DISPOSE( social->name );

    if ( social->char_no_arg )
        DISPOSE( social->char_no_arg );

    if ( social->others_no_arg )
        DISPOSE( social->others_no_arg );

    if ( social->char_found )
        DISPOSE( social->char_found );

    if ( social->others_found )
        DISPOSE( social->others_found );

    if ( social->vict_found )
        DISPOSE( social->vict_found );

    if ( social->char_auto )
        DISPOSE( social->char_auto );

    if ( social->others_auto )
        DISPOSE( social->others_auto );

    DISPOSE( social );
}

/*
    Remove a social from it's hash index             -Thoric
*/
void unlink_social( SOCIALTYPE* social )
{
    SOCIALTYPE* tmp, *tmp_next;
    int hash;

    if ( !social )
    {
        bug( "Unlink_social: NULL social", 0 );
        return;
    }

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
        hash = 0;
    else
        hash = ( social->name[0] - 'a' ) + 1;

    if ( social == ( tmp = social_index[hash] ) )
    {
        social_index[hash] = tmp->next;
        return;
    }

    for ( ; tmp; tmp = tmp_next )
    {
        tmp_next = tmp->next;

        if ( social == tmp_next )
        {
            tmp->next = tmp_next->next;
            return;
        }
    }
}

/*
    Add a social to the social index table           -Thoric
    Hashed and insert sorted
*/
void add_social( SOCIALTYPE* social )
{
    int hash, x;
    SOCIALTYPE* tmp, *prev;

    if ( !social )
    {
        bug( "Add_social: NULL social", 0 );
        return;
    }

    if ( !social->name )
    {
        bug( "Add_social: NULL social->name", 0 );
        return;
    }

    if ( !social->char_no_arg )
    {
        bug( "Add_social: NULL social->char_no_arg", 0 );
        return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; social->name[x] != '\0'; x++ )
        social->name[x] = LOWER( social->name[x] );

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
        hash = 0;
    else
        hash = ( social->name[0] - 'a' ) + 1;

    if ( ( prev = tmp = social_index[hash] ) == NULL )
    {
        social->next = social_index[hash];
        social_index[hash] = social;
        return;
    }

    for ( ; tmp; tmp = tmp->next )
    {
        if ( ( x = strcmp( social->name, tmp->name ) ) == 0 )
        {
            bug( "Add_social: trying to add duplicate name to bucket %d", hash );
            free_social( social );
            return;
        }
        else if ( x < 0 )
        {
            if ( tmp == social_index[hash] )
            {
                social->next = social_index[hash];
                social_index[hash] = social;
                return;
            }

            prev->next = social;
            social->next = tmp;
            return;
        }

        prev = tmp;
    }

    /* add to end */
    prev->next = social;
    social->next = NULL;
    return;
}

/*
    Social editor/displayer/save/delete              -Thoric
*/
void do_sedit( CHAR_DATA* ch, char* argument )
{
    SOCIALTYPE* social;
    char arg1[MIL], arg2[SUPER_MIL];
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    set_char_color( AT_SOCIAL, ch );

    if ( NULLSTR( arg1 ) )
    {
        send_to_char( "Syntax: sedit <social> [field]\n\r", ch );
        send_to_char( "Syntax: sedit <social> create\n\r", ch );

        if ( get_trust( ch ) > LEVEL_GOD )
            send_to_char( "Syntax: sedit <social> delete\n\r", ch );

        if ( get_trust( ch ) > LEVEL_LESSER )
            send_to_char( "Syntax: sedit <save>\n\r", ch );

        send_to_char( "\n\rField being one of:\n\r", ch );
        send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto\n\r", ch );
        return;
    }

    if ( get_trust( ch ) > LEVEL_LESSER && !str_cmp( arg1, "save" ) )
    {
        save_socials();
        send_to_char( "Saved.\n\r", ch );
        return;
    }

    social = find_social( arg1 );

    if ( !str_cmp( arg2, "create" ) )
    {
        if ( social )
        {
            send_to_char( "That social already exists!\n\r", ch );
            return;
        }

        CREATE( social, SOCIALTYPE, 1 );
        social->name = str_dup( arg1 );
        snprintf( arg2, SUPER_MIL, "You %s.", arg1 );
        social->char_no_arg = str_dup( arg2 );
        add_social( social );
        send_to_char( "Social added.\n\r", ch );
        return;
    }

    if ( !social )
    {
        send_to_char( "Social not found.\n\r", ch );
        return;
    }

    if ( NULLSTR( arg2 ) || !str_cmp( arg2, "show" ) )
    {
        ch_printf( ch, "Social: %s\n\r\n\rCNoArg: %s\n\r",
                   social->name,   social->char_no_arg );
        ch_printf( ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\n\r",
                   social->others_no_arg   ? social->others_no_arg : "(not set)",
                   social->char_found      ? social->char_found    : "(not set)",
                   social->others_found    ? social->others_found  : "(not set)" );
        ch_printf( ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\n\r",
                   social->vict_found  ? social->vict_found    : "(not set)",
                   social->char_auto   ? social->char_auto : "(not set)",
                   social->others_auto ? social->others_auto   : "(not set)" );
        return;
    }

    if ( get_trust( ch ) > LEVEL_GOD && !str_cmp( arg2, "delete" ) )
    {
        unlink_social( social );
        free_social( social );
        send_to_char( "Deleted.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "cnoarg" ) )
    {
        if ( argument[0] == '\0' || !str_cmp( argument, "clear" ) )
        {
            send_to_char( "You cannot clear this field.  It must have a message.\n\r", ch );
            return;
        }

        if ( social->char_no_arg )
            DISPOSE( social->char_no_arg );

        social->char_no_arg = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "onoarg" ) )
    {
        if ( social->others_no_arg )
            DISPOSE( social->others_no_arg );

        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->others_no_arg = str_dup( argument );

        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "cfound" ) )
    {
        if ( social->char_found )
            DISPOSE( social->char_found );

        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->char_found = str_dup( argument );

        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "ofound" ) )
    {
        if ( social->others_found )
            DISPOSE( social->others_found );

        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->others_found = str_dup( argument );

        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "vfound" ) )
    {
        if ( social->vict_found )
            DISPOSE( social->vict_found );

        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->vict_found = str_dup( argument );

        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "cauto" ) )
    {
        if ( social->char_auto )
            DISPOSE( social->char_auto );

        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->char_auto = str_dup( argument );

        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "oauto" ) )
    {
        if ( social->others_auto )
            DISPOSE( social->others_auto );

        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->others_auto = str_dup( argument );

        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) > LEVEL_GREATER && !str_cmp( arg2, "name" ) )
    {
        bool relocate;
        one_argument( argument, arg1 );

        if ( arg1[0] == '\0' )
        {
            send_to_char( "Cannot clear name field!\n\r", ch );
            return;
        }

        if ( arg1[0] != social->name[0] )
        {
            unlink_social( social );
            relocate = TRUE;
        }
        else
            relocate = FALSE;

        if ( social->name )
            DISPOSE( social->name );

        social->name = str_dup( arg1 );

        if ( relocate )
            add_social( social );

        send_to_char( "Done.\n\r", ch );
        return;
    }

    /* display usage message */
    do_sedit( ch, "" );
}

/*
    Free a command structure                 -Thoric
*/
void free_command( CMDTYPE* command )
{
    if ( command->name )
        DISPOSE( command->name );

    DISPOSE( command );
}

/*
    Remove a command from it's hash index            -Thoric
*/
void unlink_command( CMDTYPE* command )
{
    CMDTYPE* tmp, *tmp_next;
    int hash;

    if ( !command )
    {
        bug( "Unlink_command NULL command", 0 );
        return;
    }

    hash = command->name[0] % 126;

    if ( command == ( tmp = command_hash[hash] ) )
    {
        command_hash[hash] = tmp->next;
        return;
    }

    for ( ; tmp; tmp = tmp_next )
    {
        tmp_next = tmp->next;

        if ( command == tmp_next )
        {
            tmp->next = tmp_next->next;
            return;
        }
    }
}

/*
    Add a command to the command hash table          -Thoric
*/
void add_command( CMDTYPE* command )
{
    int hash, x;
    CMDTYPE* tmp, *prev;

    if ( !command )
    {
        bug( "Add_command: NULL command", 0 );
        return;
    }

    if ( !command->name )
    {
        bug( "Add_command: NULL command->name", 0 );
        return;
    }

    if ( !command->do_fun )
    {
        bug( "Add_command: NULL command->do_fun", 0 );
        return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; command->name[x] != '\0'; x++ )
        command->name[x] = LOWER( command->name[x] );

    hash = command->name[0] % 126;

    if ( ( prev = tmp = command_hash[hash] ) == NULL )
    {
        command->next = command_hash[hash];
        command_hash[hash] = command;
        return;
    }

    /* add to the END of the list */
    for ( ; tmp; tmp = tmp->next )
        if ( !tmp->next )
        {
            tmp->next = command;
            command->next = NULL;
        }

    return;
}

/*
    Command editor/displayer/save/delete             -Thoric
*/
void do_cedit( CHAR_DATA* ch, char* argument )
{
    CMDTYPE* command;
    char arg1[MIL], arg2[SUPER_MIL];
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    set_char_color( AT_IMMORT, ch );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: cedit save\n\r", ch );

        if ( get_trust( ch ) > LEVEL_SUB_IMPLEM )
        {
            send_to_char( "Syntax: cedit <command> create [code]\n\r", ch );
            send_to_char( "Syntax: cedit <command> delete\n\r", ch );
            send_to_char( "Syntax: cedit <command> show\n\r", ch );
            send_to_char( "Syntax: cedit <command> raise\n\r", ch );
            send_to_char( "Syntax: cedit <command> lower\n\r", ch );
            send_to_char( "Syntax: cedit <command> list\n\r", ch );
            send_to_char( "Syntax: cedit <command> [field]\n\r", ch );
            send_to_char( "\n\rField being one of:\n\r", ch );
            send_to_char( "  level position log code ooc\n\r", ch );
        }

        return;
    }

    if ( get_trust( ch ) > LEVEL_GREATER && !str_cmp( arg1, "save" ) )
    {
        save_commands();
        send_to_char( "Saved.\n\r", ch );
        return;
    }

    command = find_command( arg1 );

    if ( get_trust( ch ) > LEVEL_SUB_IMPLEM && !str_cmp( arg2, "create" ) )
    {
        if ( command )
        {
            send_to_char( "That command already exists!\n\r", ch );
            return;
        }

        CREATE( command, CMDTYPE, 1 );
        command->name = str_dup( arg1 );
        command->level = get_trust( ch );

        if ( *argument )
            one_argument( argument, arg2 );
        else
            snprintf( arg2, SUPER_MIL, "do_%s", arg1 );

        command->do_fun = skill_function( arg2 );
        add_command( command );
        send_to_char( "Command added.\n\r", ch );

        if ( command->do_fun == skill_notfound )
            ch_printf( ch, "Code %s not found.  Set to no code.\n\r", arg2 );

        return;
    }

    if ( !command )
    {
        send_to_char( "Command not found.\n\r", ch );
        return;
    }
    else if ( command->level > get_trust( ch ) )
    {
        send_to_char( "You cannot touch this command.\n\r", ch );
        return;
    }

    if ( NULLSTR( arg2 ) || !str_cmp( arg2, "show" ) )
    {
        ch_printf( ch, "Command:  %s\n\rLevel:    %d\n\rPosition: %d\n\rLog:      %d\n\rOOC:      %d\n\rCode:     %s\n\r",
                   command->name, command->level, command->position, command->log, command->ooc,
                   skill_name( command->do_fun ) );

        if ( command->userec.num_uses )
            send_timer( &command->userec, ch );

        return;
    }

    if ( get_trust( ch ) <= LEVEL_SUB_IMPLEM )
    {
        do_cedit( ch, "" );
        return;
    }

    if ( !str_cmp( arg2, "raise" ) )
    {
        CMDTYPE* tmp, *tmp_next;
        int hash = command->name[0] % 126;

        if ( ( tmp = command_hash[hash] ) == command )
        {
            send_to_char( "That command is already at the top.\n\r", ch );
            return;
        }

        if ( tmp->next == command )
        {
            command_hash[hash] = command;
            tmp_next = tmp->next;
            tmp->next = command->next;
            command->next = tmp;
            ch_printf( ch, "Moved %s above %s.\n\r", command->name, command->next->name );
            return;
        }

        for ( ; tmp; tmp = tmp->next )
        {
            tmp_next = tmp->next;

            if ( tmp_next->next == command )
            {
                tmp->next = command;
                tmp_next->next = command->next;
                command->next = tmp_next;
                ch_printf( ch, "Moved %s above %s.\n\r", command->name, command->next->name );
                return;
            }
        }

        send_to_char( "ERROR -- Not Found!\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "lower" ) )
    {
        CMDTYPE* tmp, *tmp_next;
        int hash = command->name[0] % 126;

        if ( command->next == NULL )
        {
            send_to_char( "That command is already at the bottom.\n\r", ch );
            return;
        }

        tmp = command_hash[hash];

        if ( tmp == command )
        {
            tmp_next = tmp->next;
            command_hash[hash] = command->next;
            command->next = tmp_next->next;
            tmp_next->next = command;
            ch_printf( ch, "Moved %s below %s.\n\r", command->name, tmp_next->name );
            return;
        }

        for ( ; tmp; tmp = tmp->next )
        {
            if ( tmp->next == command )
            {
                tmp_next = command->next;
                tmp->next = tmp_next;
                command->next = tmp_next->next;
                tmp_next->next = command;
                ch_printf( ch, "Moved %s below %s.\n\r", command->name, tmp_next->name );
                return;
            }
        }

        send_to_char( "ERROR -- Not Found!\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "list" ) )
    {
        CMDTYPE* tmp;
        int hash = command->name[0] % 126;
        pager_printf( ch, "Priority placement for [%s]:\n\r", command->name );

        for ( tmp = command_hash[hash]; tmp; tmp = tmp->next )
        {
            if ( tmp == command )
                set_pager_color( AT_GREEN, ch );
            else
                set_pager_color( AT_PLAIN, ch );

            pager_printf( ch, "  %s\n\r", tmp->name );
        }

        return;
    }

    if ( !str_cmp( arg2, "delete" ) )
    {
        unlink_command( command );
        free_command( command );
        send_to_char( "Deleted.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "code" ) )
    {
        DO_FUN* fun = skill_function( argument );

        if ( fun == skill_notfound )
        {
            send_to_char( "Code not found.\n\r", ch );
            return;
        }

        command->do_fun = fun;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
        int level = atoi( argument );

        if ( level < 0 || level > get_trust( ch ) )
        {
            send_to_char( "Level out of range.\n\r", ch );
            return;
        }

        command->level = level;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "log" ) )
    {
        int log = atoi( argument );

        if ( log < 0 || log > LOG_COMM )
        {
            send_to_char( "Log out of range.\n\r", ch );
            return;
        }

        command->log = log;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "ooc" ) )
    {
        int ooc = atoi( argument );

        if ( ooc < 0 || ooc > 1 )
        {
            send_to_char( "OOC out of range. 1 for OOC, 0 for IC.\n\r", ch );
            return;
        }

        command->ooc = ooc;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "position" ) )
    {
        int position = atoi( argument );

        if ( position < 0 || position > POS_DRAG )
        {
            send_to_char( "Position out of range.\n\r", ch );
            return;
        }

        command->position = position;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        bool relocate;
        one_argument( argument, arg1 );

        if ( arg1[0] == '\0' )
        {
            send_to_char( "Cannot clear name field!\n\r", ch );
            return;
        }

        if ( arg1[0] != command->name[0] )
        {
            unlink_command( command );
            relocate = TRUE;
        }
        else
            relocate = FALSE;

        if ( command->name )
            DISPOSE( command->name );

        command->name = str_dup( arg1 );

        if ( relocate )
            add_command( command );

        send_to_char( "Done.\n\r", ch );
        return;
    }

    /* display usage message */
    do_cedit( ch, "" );
}

/*
    quest point set - TRI
    syntax is: qpset char give/take amount
*/

void do_qpset( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    int amount;
    bool give = TRUE;
    set_char_color( AT_IMMORT, ch );

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Cannot qpset as an NPC.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    amount = atoi( arg3 );

    if ( arg[0] == '\0' || arg2[0] == '\0' || amount <= 0 )
    {
        send_to_char( "Syntax: qpset <character> <give/take> <amount>\n\r", ch );
        send_to_char( "Amount must be a positive number greater than 0.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "There is no such player currently playing.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Glory cannot be given to or taken from a mob.\n\r", ch );
        return;
    }

    set_char_color( AT_IMMORT, victim );

    if ( nifty_is_name_prefix( arg2, "give" ) )
    {
        give = TRUE;

        if ( str_cmp( ch->pcdata->council_name, "Quest Council" )
                && ( get_trust( ch ) < LEVEL_DEMI ) )
        {
            send_to_char( "You must be a member of the Quest Council to give qp to a character.\n\r", ch );
            return;
        }
    }
    else if ( nifty_is_name_prefix( arg2, "take" ) )
        give = FALSE;
    else
    {
        do_qpset( ch, "" );
        return;
    }

    if ( give )
    {
        victim->pcdata->quest_curr += amount;
        victim->pcdata->quest_accum += amount;
        ch_printf( victim, "Your glory has been increased by %d.\n\r", amount );
        ch_printf( ch, "You have increased the glory of %s by %d.\n\r",
                   victim->name, amount );
    }
    else
    {
        if ( victim->pcdata->quest_curr - amount < 0 )
        {
            ch_printf( ch, "%s does not have %d glory to take.\n\r",
                       victim->name, amount );
            return;
        }
        else
        {
            victim->pcdata->quest_curr -= amount;
            ch_printf( victim, "Your glory has been decreased by %d.\n\r", amount );
            ch_printf( ch, "You have decreased the glory of %s by %d.\n\r",
                       victim->name, amount );
        }
    }

    return;
}

/* Easy way to check a player's glory -- Blodkai, June 97 */
void do_qpstat( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    set_char_color( AT_IMMORT, ch );

    if ( IS_NPC( ch ) )
        return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax:  qpstat <character>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg ) ) == NULL )
    {
        send_to_char( "No one by that name currently in the Realms.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Mobs don't have glory.\n\r", ch );
        return;
    }

    ch_printf( ch, "%s has %d glory, out of a lifetime total of %d.\n\r",
               victim->name,
               victim->pcdata->quest_curr,
               victim->pcdata->quest_accum );
    return;
}

/*  void do_reserve(CHAR_DATA *ch, char *argument)
    {
    char arg[MAX_INPUT_LENGTH];
    RESERVE_DATA *res;

    set_char_color( AT_PLAIN, ch );

    argument = one_argument(argument, arg);
    if (!*arg)
    {
    int wid = 0;

    send_to_char("-- Reserved Names --\n\r", ch);
    for (res = first_reserved; res; res = res->next)
    {
      ch_printf(ch, "%c%-17s ", (*res->name == '*' ? '*' : ' '),
          (*res->name == '*' ? res->name+1 : res->name));
      if (++wid % 4 == 0)
        send_to_char("\n\r", ch);
    }
    if (wid % 4 != 0)
      send_to_char("\n\r", ch);
    return;
    }
    for (res = first_reserved; res; res = res->next)
    if (!str_cmp(arg, res->name))
    {
      UNLINK(res, first_reserved, last_reserved, next, prev);
      DISPOSE(res->name);
      DISPOSE(res);
      save_reserved();
      send_to_char("Name no longer reserved.\n\r", ch);
      return;
    }
    CREATE(res, RESERVE_DATA, 1);
    res->name = str_dup(arg);
    sort_reserved(res);
    save_reserved();
    send_to_char("Name reserved.\n\r", ch);
    return;
    }*/

/*
    Command to display the weather status of all the areas
    Last Modified: July 21, 1997
    Fireblade
*/
/*  void do_showweather(CHAR_DATA *ch, char *argument)
    {
    AREA_DATA *pArea;
    char arg[MAX_INPUT_LENGTH];

    if(!ch)
    {
        bug("do_showweather: NULL char data");
        return;
    }

    argument = one_argument(argument, arg);

    set_char_color(AT_BLUE, ch);
    ch_printf(ch, "%-40s%-8s %-8s %-8s\n\r",
        "Area Name:", "Temp:", "Precip:", "Wind:");

    for(pArea = first_area; pArea; pArea = pArea->next)
    {
        if(arg[0] == '\0' ||
            nifty_is_name_prefix(arg, pArea->name))
        {
            set_char_color(AT_BLUE, ch);
            ch_printf(ch, "%-40s", pArea->name);
            set_char_color(AT_WHITE, ch);
            ch_printf(ch, "%3d", pArea->weather->temp);
            set_char_color(AT_BLUE, ch);
            ch_printf(ch, "(");
            set_char_color(AT_LBLUE, ch);
            ch_printf(ch, "%3d", pArea->weather->temp_vector);
            set_char_color(AT_BLUE,ch);
            ch_printf(ch, ") ");
            set_char_color(AT_WHITE,ch);
            ch_printf(ch, "%3d", pArea->weather->precip);
            set_char_color(AT_BLUE, ch);
            ch_printf(ch, "(");
            set_char_color(AT_LBLUE, ch);
            ch_printf(ch, "%3d", pArea->weather->precip_vector);
            set_char_color(AT_BLUE, ch);
            ch_printf(ch, ") ");
            set_char_color(AT_WHITE, ch);
            ch_printf(ch, "%3d", pArea->weather->wind);
            set_char_color(AT_BLUE, ch);
            ch_printf(ch, "(");
            set_char_color(AT_LBLUE, ch);
            ch_printf(ch, "%3d", pArea->weather->wind_vector);
            set_char_color(AT_BLUE, ch);
            ch_printf(ch, ")\n\r");
        }
    }

    return;
    }*/

/*
    Command to control global weather variables and to reset weather
    Last Modified: July 23, 1997
    Fireblade
*/
/*
    void do_setweather(CHAR_DATA *ch, char *argument)
    {
    char arg[MAX_INPUT_LENGTH];

    set_char_color(AT_BLUE, ch);

    argument = one_argument(argument, arg);

    if(arg[0] == '\0')
    {
        ch_printf(ch, "%-15s%-6s\n\r",
            "Parameters:", "Value:");
        ch_printf(ch, "%-15s%-6d\n\r",
            "random", rand_factor);
        ch_printf(ch, "%-15s%-6d\n\r",
            "climate", climate_factor);
        ch_printf(ch, "%-15s%-6d\n\r",
            "neighbor", neigh_factor);
        ch_printf(ch, "%-15s%-6d\n\r",
            "unit", weath_unit);
        ch_printf(ch, "%-15s%-6d\n\r",
            "maxvector", max_vector);

        ch_printf(ch, "\n\rResulting values:\n\r");
        ch_printf(ch, "Weather variables range from "
            "%d to %d.\n\r", -3*weath_unit,
            3*weath_unit);
        ch_printf(ch, "Weather vectors range from "
            "%d to %d.\n\r", -1*max_vector,
            max_vector);
        ch_printf(ch, "The maximum a vector can "
            "change in one update is %d.\n\r",
            rand_factor + 2*climate_factor +
            (6*weath_unit/neigh_factor));
    }
    else if(!str_cmp(arg, "random"))
    {
        if(!is_number(argument))
        {
            ch_printf(ch, "Set maximum random "
                "change in vectors to what?\n\r");
        }
        else
        {
            rand_factor = atoi(argument);
            ch_printf(ch, "Maximum random "
                "change in vectors now "
                "equals %d.\n\r", rand_factor);
            save_weatherdata();
        }
    }
    else if(!str_cmp(arg, "climate"))
    {
        if(!is_number(argument))
        {
            ch_printf(ch, "Set climate effect "
                "coefficient to what?\n\r");
        }
        else
        {
            climate_factor = atoi(argument);
            ch_printf(ch, "Climate effect "
                "coefficient now equals "
                "%d.\n\r", climate_factor);
            save_weatherdata();
        }
    }
    else if(!str_cmp(arg, "neighbor"))
    {
        if(!is_number(argument))
        {
            ch_printf(ch, "Set neighbor effect "
                "divisor to what?\n\r");
        }
        else
        {
            neigh_factor = atoi(argument);

            if(neigh_factor <= 0)
                neigh_factor = 1;

            ch_printf(ch, "Neighbor effect "
                "coefficient now equals "
                "1/%d.\n\r", neigh_factor);
            save_weatherdata();
        }
    }
    else if(!str_cmp(arg, "unit"))
    {
        if(!is_number(argument))
        {
            ch_printf(ch, "Set weather unit "
                "size to what?\n\r");
        }
        else
        {
            weath_unit = atoi(argument);
            ch_printf(ch, "Weather unit size "
                "now equals %d.\n\r",
                weath_unit);
            save_weatherdata();
        }
    }
    else if(!str_cmp(arg, "maxvector"))
    {
        if(!is_number(argument))
        {
            ch_printf(ch, "Set maximum vector "
                "size to what?\n\r");
        }
        else
        {
            max_vector = atoi(argument);
            ch_printf(ch, "Maximum vector size "
                "now equals %d.\n\r",
                max_vector);
            save_weatherdata();
        }
    }
    else if(!str_cmp(arg, "reset"))
    {
        init_area_weather();
        ch_printf(ch, "Weather system reinitialized.\n\r");
    }
    else if(!str_cmp(arg, "update"))
    {
        int i, number;

        number = atoi(argument);

        if(number < 1)
            number = 1;

        for(i = 0; i < number; i++)
            weather_update();

        ch_printf(ch, "Weather system updated.\n\r");
    }
    else
    {
        ch_printf(ch, "You may only use one of the "
            "following fields:\n\r");
        ch_printf(ch, "\trandom\n\r\tclimate\n\r"
            "\tneighbor\n\r\tunit\n\r\tmaxvector\n\r");
        ch_printf(ch, "You may also reset or update "
            "the system using the fields 'reset' "
            "and 'update' respectively.\n\r");
    }

    return;
    }*/

/*  void do_pcrename( CHAR_DATA *ch, char *argument )
    {
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char newname[MAX_STRING_LENGTH];
    char oldname[MAX_STRING_LENGTH];
    char backname[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );
    smash_tilde( arg2 );


    if ( IS_NPC(ch) )
    return;

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
    send_to_char("Syntax: rename <victim> <new name>\n\r", ch );
    return;
    }

    if  (!check_parse_name( arg2, 1) )
    {
    send_to_char("Illegal name.\n\r", ch );
    return;
    }
      Just a security precaution so you don't rename someone you don't mean
       too --Shaddai

    if ( ( victim = get_char_room ( ch, arg1 ) ) == NULL )
    {
    send_to_char("That person is not in the room.\n\r", ch );
    return;
    }
    if ( IS_NPC(victim ) )
    {
    send_to_char("You can't rename NPC's.\n\r", ch );
    return;
    }

    if ( get_trust(ch) < get_trust(victim) )
    {
    send_to_char("I don't think they would like that!\n\r", ch );
    return;
    }
    sprintf( newname, "%s%c/%s", PLAYER_DIR, tolower(arg2[0]),
                                 capitalize( arg2 ) );
    sprintf( oldname, "%s%c/%s",PLAYER_DIR,tolower(victim->pcdata->filename[0]),
                                 capitalize( victim->pcdata->filename ) );
    sprintf( backname,"%s%c/%s",BACKUP_DIR,tolower(victim->pcdata->filename[0]),
                                 capitalize( victim->pcdata->filename ) );
    if ( access( newname, F_OK ) == 0 )
    {
    send_to_char("That name already exists.\n\r", ch );
    return;
    }

     Have to remove the old god entry in the directories
    if ( IS_IMMORTAL( victim ) )
    {
    char godname[MAX_STRING_LENGTH];
    sprintf(godname, "%s%s", GOD_DIR, capitalize(victim->pcdata->filename));
    remove( godname );
    }

     Remember to change the names of the areas
    if ( ch->pcdata->area )
    {
       char filename[MAX_STRING_LENGTH];
       char newfilename[MAX_STRING_LENGTH];

       sprintf( filename, "%s%s.are", BUILD_DIR, victim->name);
       sprintf( newfilename, "%s%s.are", BUILD_DIR, capitalize(arg2));
       rename(filename, newfilename);
       sprintf( filename, "%s%s.are.bak", BUILD_DIR, victim->name);
       sprintf( newfilename, "%s%s.are.bak", BUILD_DIR, capitalize(arg2));
       rename(filename, newfilename);
    }

    STRFREE( victim->name );
    victim->name = STRALLOC( capitalize(arg2) );
    STRFREE( victim->pcdata->filename );
    victim->pcdata->filename = STRALLOC( capitalize(arg2) );
    remove( backname );
    if ( remove( oldname ) )
    {
    sprintf(buf, "Error: Couldn't delete file %s in do_rename.", oldname);
    send_to_char("Couldn't delete the old file!\n\r", ch );
    log_string( oldname );
    }
     Time to save to force the affects to take place
    save_char_obj( victim );

     Now lets update the wizlist
    if ( IS_IMMORTAL( victim ) )
    make_wizlist();
    send_to_char("Character was renamed.\n\r", ch );
    return;
    }*/

void do_saveall( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* vch;
    CHAR_DATA* vch_next;

    for ( vch = first_char; vch; vch = vch_next )
    {
        vch_next = vch->next;

        if ( !IS_NPC( vch ) )
        {
            do_save( vch, "" );
        }
    }

    return;
}

void autosave ( void )
{
    CHAR_DATA* vch;
    CHAR_DATA* vch_next;

    for ( vch = first_char; vch; vch = vch_next )
    {
        if ( vch == NULL )
            break;

        vch_next = vch->next;

        if ( !IS_NPC( vch ) )
        {
            do_save( vch, "-noalert" );
        }
    }

    return;
}

void do_immortalize( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* victim;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int oldlevel, level;
    char buf[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
        send_to_char( "You need to pick a person to imortalize.\r\n", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "That person doesn't exist.\r\n", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Advance a mob.. yea right.\r\n", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char( "Nice try\r\n", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_char( "Syntax: Imortalize <charname> <immlevel>", ch );
        return;
    }

    level = atoi( arg2 );

    if ( level < 101 || level > 105 || level > get_trust( ch ) )
    {
        send_to_char( "Level out of range.\r\n", ch );
        return;
    }

    if ( get_trust( victim ) == level )
    {
        send_to_char( "Victim is already at that level\r\n", ch );
        return;
    }

    if ( get_trust( ch ) <= get_trust( victim ) )
    {
        send_to_char( "You are not authorized to do that\r\n", ch );
        return;
    }

    oldlevel = get_trust( victim );
    victim->top_level = level;
    sprintf( buf, "%s advanced %s from level %d to %d", ch->name, victim->name, oldlevel, level );
    log_string( buf );
    to_channel( buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
    return;
}

void do_freevnums( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    AREA_DATA* pArea;
    bool a_conflict;
    int low_v = 0, high_v = 0, count = 0;
    int l = 0, h = 0, curr = 0, total_c = 0, total_r = 0;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: FREEVNUMS [low vnum] [high vnum]\n\r", ch );
        return;
    }
    else
    {
        low_v = atoi( arg1 );
        high_v = atoi( arg2 );
    }

    if ( low_v < 1 || low_v > MAX_VNUMS )
    {
        send_to_char( "Invalid argument for bottom of range.\n\r", ch );
        return;
    }

    if ( high_v < 1 || high_v > MAX_VNUMS )
    {
        send_to_char( "Invalid argument for top of range.\n\r", ch );
        return;
    }

    if ( high_v <= low_v )
    {
        send_to_char( "Bottom of range must be below top of range.\n\r", ch );
        return;
    }

    if ( ( high_v - low_v ) > 1000000 )
    {
        send_to_char( "Sorry, You can only check up to 1000000 rooms at a time!\n\r", ch );
        return;
    }

    sprintf( buf, "&zFree vnum ranges in the &C%d &zto &C%d &zRANGE: &C------ \n\r\n\r", low_v, high_v );
    send_to_char( buf, ch );
    /* Check the range room-by-room. This is going to SUCK processer time... */
    l = low_v;
    h = low_v;

    for ( curr = low_v; curr <= high_v; curr++ )
    {
        a_conflict = FALSE;

        for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
        {
            if ( IS_SET( pArea->status, AREA_DELETED ) )
                continue;

            if ( ( curr >= pArea->low_r_vnum && curr <= pArea->hi_r_vnum ) ||
                    ( curr >= pArea->low_m_vnum && curr <= pArea->hi_m_vnum ) ||
                    ( curr >= pArea->low_o_vnum && curr <= pArea->hi_o_vnum ) )
            {
                a_conflict = TRUE;
            }
        }

        if ( a_conflict != TRUE )
        {
            for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
            {
                if ( IS_SET( pArea->status, AREA_DELETED ) )
                    continue;

                if ( ( curr >= pArea->low_r_vnum && curr <= pArea->hi_r_vnum ) ||
                        ( curr >= pArea->low_m_vnum && curr <= pArea->hi_m_vnum ) ||
                        ( curr >= pArea->low_o_vnum && curr <= pArea->hi_o_vnum ) )
                {
                    a_conflict = TRUE;
                }
            }
        }

        if ( a_conflict || curr == high_v )
        {
            count = curr - l - 1;
            h = count + l;

            if ( curr == high_v )
                h++;

            if ( count > 1 )
            {
                sprintf( buf, "&BVNums: &C%7d &B- &C%7d   &B|   Count: &C%8d&B\n\r", l, h, count );
                send_to_char( buf, ch );
                total_c++;
                total_r += count;
            }

            l = curr + 1;
            h = curr + 1;
        }
    }

    sprintf( buf, "\n\r&zThere are &C%d &zfree rooms in &C%d &zset(s) of vnums.\n\r", total_r, total_c );
    send_to_char( buf, ch );
    return;
}

void do_compute( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    float a = 0, b = 0, c = 0;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "Syntax: COMPUTE [first number] [expression] [second number]\n\r", ch );
        send_to_char( "Valid Expressions: + - * \\ :\n\r", ch );
        return;
    }
    else
    {
        a = atoi( arg1 );
        b = atoi( arg3 );
    }

    if ( !str_cmp( arg2, "+" ) || !str_cmp( arg2, "-" ) || !str_cmp( arg2, "\\" ) || !str_cmp( arg2, "/" ) || !str_cmp( arg2, "*" ) || !str_cmp( arg2, ":" ) )
    {
        if ( a < -999999 || a > 999999 || b < -999999 || b > 999999 )
        {
            send_to_char( "Sorry, One of your numbers is too large.\n\r", ch );
            return;
        }
    }
    else
    {
        send_to_char( "&RInvalid expression for formula!\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "+" ) )
        c = a + b;

    if ( !str_cmp( arg2, "-" ) )
        c = a - b;

    if ( !str_cmp( arg2, "*" ) )
        c = a * b;

    if ( !str_cmp( arg2, "\\" ) )
        c = a / b;

    if ( !str_cmp( arg2, "/" ) )
        c = a / b;

    if ( !str_cmp( arg2, ":" ) )
    {
        ch_printf( ch, "&zComputed Anwser: &C%s\n\r", reduce_ratio( ( int )( a ), ( int )( b ) ) );
    }
    else
    {
        ch_printf( ch, "&zComputed Answer: &C%-0.2f\n\r", c );
    }

    return;
}

