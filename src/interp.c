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
             Command interpretation module
****************************************************************************/

#include <sys/types.h>
// #include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "mqtt.h"

#ifdef timerclear
    #undef timerclear
#endif

#define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0
#define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)

/*
    Externals
*/

void subtract_times( struct timeval* etime, struct timeval* stime );



bool    check_social    args( ( CHAR_DATA* ch, char* command,
                                char* argument ) );


/*
    Log-all switch.
*/
bool                fLogAll     = FALSE;


CMDTYPE*    command_hash[126];  /* hash table for cmd_table */
SOCIALTYPE* social_index[27];   /* hash table for socials   */

/*
    Character not in position for command?
*/
bool check_pos( CHAR_DATA* ch, sh_int position )
{
    if ( ch->position < position )
    {
        switch ( ch->position )
        {
            case POS_DEAD:
                send_to_char( "A little difficult to do when you are DEAD...\n\r", ch );
                break;

            case POS_MORTAL:
            case POS_INCAP:
                send_to_char( "You are hurt far too bad for that.\n\r", ch );
                break;

            case POS_STUNNED:
                send_to_char( "You are too stunned to do that.\n\r", ch );
                break;

            case POS_PRONE:
                send_to_char( "You can't do that while lying prone.\n\r", ch );
                break;

            case POS_KNEELING:
                send_to_char( "You can't do that while kneeling.\n\r", ch );
                break;

            case POS_SITTING:
                send_to_char( "You can't do that sitting down.\n\r", ch );
                break;
        }

        return FALSE;
    }

    return TRUE;
}

extern char lastplayercmd[MAX_INPUT_LENGTH * 2];

/*
    The main entry point for executing commands.
    Can be recursively called from 'at', 'order', 'force'.
*/
/*
    void interpret( CHAR_DATA *ch, char *argument )
*/
void interpret( CHAR_DATA* ch, char* argument, bool is_order )
{
    char command[MIL], logline[MIL], logname[MIL], logperson[SUPER_MIL];
    TIMER* timer = NULL;
    CMDTYPE* cmd = NULL;
    int trust, i = 0;
    int loglvl;
    bool found = FALSE;
    struct timeval time_used;
    long tmptime;

    if ( !ch )
    {
        bug( "interpret: null ch!", 0 );
        return;
    }

    found = FALSE;

    if ( ch->substate == SUB_REPEATCMD )
    {
        DO_FUN* fun;

        if ( ( fun = ch->last_cmd ) == NULL )
        {
            ch->substate = SUB_NONE;
            bug( "interpret: SUB_REPEATCMD with NULL last_cmd", 0 );
            return;
        }
        else
        {
            int x;

            /*
                yes... we lose out on the hashing speediness here...
                but the only REPEATCMDS are wizcommands (currently)
            */
            for ( x = 0; x < 126; x++ )
            {
                for ( cmd = command_hash[x]; cmd; cmd = cmd->next )
                    if ( cmd->do_fun == fun )
                    {
                        found = TRUE;
                        break;
                    }

                if ( found )
                    break;
            }

            if ( !found )
            {
                cmd = NULL;
                bug( "interpret: SUB_REPEATCMD: last_cmd invalid", 0 );
                return;
            }

            snprintf( logline, MIL, "(%s) %s", cmd->name, argument );
        }
    }

    if ( !cmd )
    {
        /* Changed the order of these ifchecks to prevent crashing. */
        if ( !argument || !strcmp( argument, "" ) )
        {
            bug( "interpret: null argument!", 0 );
            return;
        }

        /*
            Strip leading spaces.
        */
        while ( isspace( *argument ) )
            argument++;

        if ( argument[0] == '\0' )
            return;

        timer = get_timerptr( ch, TIMER_DO_FUN );

        /* xREMOVE_BIT( ch->affected_by, AFF_HIDE ); */

        /*
            Implement freeze command.
        */
        if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_FREEZE ) )
        {
            send_to_char( "You're totally frozen!\n\r", ch );
            return;
        }

        /*
            Grab the command word.
            Special parsing so ' can be a command,
             also no spaces needed after punctuation.
        */
        strncpy( logline, argument, MIL );

        if ( !isalpha( argument[0] ) && !isdigit( argument[0] ) )
        {
            command[0] = argument[0];
            command[1] = '\0';
            argument++;

            while ( isspace( *argument ) )
                argument++;
        }
        else
            argument = one_argument( argument, command );

        /*
            Look for command in command table.
            Check for council powers and/or bestowments
        */
        trust = get_trust( ch );

        for ( cmd = command_hash[LOWER( command[0] ) % 126]; cmd; cmd = cmd->next )
            if ( !str_prefix( command, cmd->name )
                    &&   ( cmd->level <= trust
                           ||  ( !IS_NPC( ch ) && ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0'
                                 &&    is_name( cmd->name, ch->pcdata->bestowments )
                                 &&    cmd->level <= ( trust + 5 ) ) ) )
            {
                found = TRUE;
                break;
            }

        /*
            Turn off afk bit when any command performed.
        */
        if ( xIS_SET ( ch->act, PLR_AFK )  && ( str_cmp( command, "AFK" ) ) )
        {
            xREMOVE_BIT( ch->act, PLR_AFK );
            act( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_ROOM );
        }
    }

    /*
        Log and snoop.
    */
    snprintf( lastplayercmd, SUPER_MIL, "** %s: %s", ch->name, logline );

    if ( found && cmd->log == LOG_NEVER )
        strncpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX", MIL );

    loglvl = found ? cmd->log : LOG_NORMAL;

    if ( !IS_NPC( ch ) ) {
        char mqtt_publish_logline[MSL];
        snprintf( mqtt_publish_logline, MSL, "%s,%d,%s", ch->name, get_trust(ch), logline );
        mqtt_publish("out/cmdlog", mqtt_publish_logline);
    }
    
    if ( ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_LOG ) )
            ||   fLogAll
            ||   loglvl == LOG_BUILD
            ||   loglvl == LOG_HIGH
            ||   loglvl == LOG_ALWAYS )
    {
        /*  Added by Narn to show who is switched into a mob that executes
            a logged command.  Check for descriptor in case force is used. */
        if ( ch->desc && ch->desc->original )
            snprintf( log_buf, MSL, "Log %s (%s): %s", ch->name,
                     ch->desc->original->name, logline );
        else
            snprintf( log_buf, MSL, "Log %s: %s", ch->name, logline );

        /*
            Make it so a 'log all' will send most output to the log
            file only, and not spam the log channel to death -Thoric
        */
        if ( fLogAll && loglvl == LOG_NORMAL
                &&  ( IS_NPC( ch ) || !xIS_SET( ch->act, PLR_LOG ) ) )
            loglvl = LOG_ALL;

        /* This is handled in get_trust already */
        /*  if ( ch->desc && ch->desc->original )
              log_string_plus( log_buf, loglvl,
                ch->desc->original->level );
            else*/
        log_string_plus( log_buf, loglvl, get_trust( ch ) );
    }

    /*
        Full Logging system for Immortals
        Generated under LOG Directory, as immortal.log
    */
    if ( !IS_NPC( ch ) )
    {
        char filenameA[MIL];
        char filenameB[MIL];
        char* strtime;
        strtime                    = ctime( &current_time );
        strtime[strlen( strtime ) - 1] = '\0';
        snprintf( filenameA, MIL, "%simmortal.log", LOG_DIR );
        snprintf( filenameB, MIL, "%simmortal.old", LOG_DIR );

        if ( ch->desc && ch->desc->original )
        {
            snprintf( logperson, SUPER_MIL, "%s :: %-12s: %s", strtime, ch->desc->original->name, logline );
        }
        else
        {
            snprintf( logperson, SUPER_MIL, "%s :: %-12s: %s", strtime, ch->name, logline );
        }

        /* Stops logging at 10 megs, instead rename the old one to a new name  */
        if ( file_size( filenameA ) < 10000000 )
        {
            append_to_file( filenameA, logperson );
        }
        else
        {
            log_string( "[File capacity reached, shifting .log to .old]" );
            rename( filenameA, filenameB );
            append_to_file( filenameA, logperson );
        }
    }

    for ( i = 0; i <= 5; i++ )
    {
        if ( ch->desc && ch->desc->snoop_by[i] )
        {
            snprintf( logname, MIL, "%s", ch->name );
            write_to_buffer( ch->desc->snoop_by[i], logname, 0 );
            write_to_buffer( ch->desc->snoop_by[i], "% ",    2 );
            write_to_buffer( ch->desc->snoop_by[i], logline, 0 );
            write_to_buffer( ch->desc->snoop_by[i], "\n\r",  2 );
        }
    }

    if ( cmd && timer && cmd->ooc == 0 )
    {
        int tempsub;
        tempsub = ch->substate;
        ch->substate = SUB_TIMER_DO_ABORT;
        ( timer->do_fun )( ch, "" );

        if ( char_died( ch ) )
            return;

        if ( ch->substate != SUB_TIMER_CANT_ABORT )
        {
            ch->substate = tempsub;
            extract_timer( ch, timer );
        }
        else
        {
            ch->substate = tempsub;
            return;
        }
    }

    /*
        Look for command in skill and socials table.
    */
    if ( !found )
    {
        if ( !check_skill( ch, command, argument )
                &&   !check_social( ch, command, argument )
           )
        {
            EXIT_DATA* pexit;

            /* check for an auto-matic exit command */
            if ( ( pexit = find_door( ch, command, TRUE ) ) != NULL
                    &&   xIS_SET( pexit->exit_info, EX_xAUTO ) )
            {
                if ( xIS_SET( pexit->exit_info, EX_CLOSED )
                        && ( !IS_AFFECTED( ch, AFF_PASS_DOOR )
                             ||   xIS_SET( pexit->exit_info, EX_NOPASSDOOR ) ) )
                {
                    if ( !xIS_SET( pexit->exit_info, EX_SECRET ) )
                        act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
                    else
                        send_to_char( "You cannot do that here.\n\r", ch );

                    return;
                }

                move_char( ch, pexit, pexit->vdir, 0 );
                return;
            }

            send_to_char( "Huh?\n\r", ch );
        }

        return;
    }

    if ( !cmd )
        return;

    /*
        Character not in position for command?
    */
    if ( !check_pos( ch, cmd->position ) )
        return;

    /* PC Charm check might change this to some kind of table at a later point */
    if ( !IS_NPC( ch ) && is_order && IS_AFFECTED( ch, AFF_CHARM ) &&
            str_cmp( cmd->name, "get" ) &&
            str_cmp( cmd->name, "drop" ) &&
            str_cmp( cmd->name, "leave" ) &&
            str_cmp( cmd->name, "board" ) &&
            str_cmp( cmd->name, "sit" ) &&
            str_cmp( cmd->name, "stand" ) &&
            str_cmp( cmd->name, "remove" ) &&
            str_cmp( cmd->name, "north" ) &&
            str_cmp( cmd->name, "south" ) &&
            str_cmp( cmd->name, "west" ) &&
            str_cmp( cmd->name, "east" ) &&
            str_cmp( cmd->name, "nw" ) &&
            str_cmp( cmd->name, "ne" ) &&
            str_cmp( cmd->name, "sw" ) &&
            str_cmp( cmd->name, "se" ) &&
            str_cmp( cmd->name, "up" ) &&
            str_cmp( cmd->name, "down" ) )
    {
        send_to_char( "You cant be ordered to do that...\n\r", ch );
        return;
    }

    /* charmed players can do the following commands */
    if ( !IS_NPC( ch ) && !is_order && IS_AFFECTED( ch, AFF_CHARM ) &&
            str_cmp( cmd->name, "say" ) &&
            str_cmp( cmd->name, "clan" ) &&
            str_cmp( cmd->name, "chat" ) &&
            str_cmp( cmd->name, "emote" ) &&
            str_cmp( cmd->name, "shout" ) &&
            str_cmp( cmd->name, "yell" ) )
    {
        if ( !IS_IMMORTAL( ch ) )
        {
            send_to_char( "You cant do that while apprehended...\n\r", ch );
            return;
        }
        else
            xREMOVE_BIT( ch->affected_by, AFF_CHARM );
    }

    /*
        Dispatch the command.
    */
    ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
    ch->last_cmd = cmd->do_fun;
    start_timer( &time_used );
    ( *cmd->do_fun ) ( ch, argument );
    end_timer( &time_used );
    /*
        Update the record of how many times this command has been used (haus)
    */
    update_userec( &time_used, &cmd->userec );
    tmptime = UMIN( time_used.tv_sec, 19 ) * 1000000 + time_used.tv_usec;

    /* laggy command notice: command took longer than 1.5 seconds */
    if ( tmptime > 1500000 )
    {
        snprintf( log_buf, MSL, "[*****] LAG: %s: %s %s (R:%d S:%d.%06d)", ch->name,
                 cmd->name, ( cmd->log == LOG_NEVER ? "XXX" : argument ),
                 ch->in_room ? ch->in_room->vnum : 0,
                 ( int ) ( time_used.tv_sec ), ( int ) ( time_used.tv_usec ) );
        log_string_plus( log_buf, LOG_NORMAL, get_trust( ch ) );
    }

    tail_chain( );
}

CMDTYPE* find_command( char* command )
{
    CMDTYPE* cmd;
    int hash;
    hash = LOWER( command[0] ) % 126;

    for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
        if ( !str_prefix( command, cmd->name ) )
            return cmd;

    return NULL;
}

SOCIALTYPE* find_social( char* command )
{
    SOCIALTYPE* social;
    int hash;

    if ( command[0] < 'a' || command[0] > 'z' )
        hash = 0;
    else
        hash = ( command[0] - 'a' ) + 1;

    for ( social = social_index[hash]; social; social = social->next )
        if ( !str_prefix( command, social->name ) )
            return social;

    return NULL;
}

bool check_social( CHAR_DATA* ch, char* command, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA* victim;
    SOCIALTYPE* social;

    if ( is_spectator( ch ) )
        return FALSE;

    if ( ( social = find_social( command ) ) == NULL )
        return FALSE;

    if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_NO_EMOTE ) )
    {
        send_to_char( "You are anti-social!\n\r", ch );
        return TRUE;
    }

    switch ( ch->position )
    {
        case POS_DEAD:
            send_to_char( "Lie still; you are DEAD.\n\r", ch );
            return TRUE;

        case POS_INCAP:
        case POS_MORTAL:
            send_to_char( "You are hurt far too bad for that.\n\r", ch );
            return TRUE;

        case POS_STUNNED:
            send_to_char( "You are too stunned to do that.\n\r", ch );
            return TRUE;
    }

    one_argument( argument, arg );
    victim = NULL;

    if ( arg[0] == '\0' )
    {
        act( AT_SOCIAL, social->others_no_arg, ch, NULL, victim, TO_ROOM    );
        act( AT_SOCIAL, social->char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
        act( AT_SOCIAL, social->others_auto,   ch, NULL, victim, TO_ROOM    );
        act( AT_SOCIAL, social->char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
        act( AT_SOCIAL, social->others_found,  ch, NULL, victim, TO_NOTVICT );
        act( AT_SOCIAL, social->char_found,    ch, NULL, victim, TO_CHAR    );
        act( AT_SOCIAL, social->vict_found,    ch, NULL, victim, TO_VICT    );

        if ( !IS_NPC( ch ) && IS_NPC( victim ) && !IS_AFFECTED( victim, AFF_CHARM )
                && IS_AWAKE( victim ) && !xIS_SET( victim->pIndexData->progtypes, ACT_PROG ) )
        {
            switch ( number_bits( 4 ) )
            {
                case 0:
                    act( AT_ACTION, "$n acts like $N doesn't even exist.",  victim, NULL, ch, TO_NOTVICT );
                    act( AT_ACTION, "You just ignore $N.",  victim, NULL, ch, TO_CHAR    );
                    act( AT_ACTION, "$n appears to be ignoring you.", victim, NULL, ch, TO_VICT    );
                    break;

                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                    act( AT_SOCIAL, social->others_found, victim, NULL, ch, TO_NOTVICT );
                    act( AT_SOCIAL, social->char_found, victim, NULL, ch, TO_CHAR    );
                    act( AT_SOCIAL, social->vict_found, victim, NULL, ch, TO_VICT    );
                    break;

                case 9:
                case 10:
                case 11:
                case 12:
                    act( AT_ACTION, "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
                    act( AT_ACTION, "You slap $N.",  victim, NULL, ch, TO_CHAR    );
                    act( AT_ACTION, "$n slaps you.", victim, NULL, ch, TO_VICT    );
                    break;
            }
        }
    }

    return TRUE;
}



/*
    Return true if an argument is completely numeric.
*/
bool is_number( char* arg )
{
    if ( *arg == '\0' )
        return FALSE;

    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }

    return TRUE;
}

bool is_number_sym( char* arg )
{
    if ( *arg == '\0' )
        return FALSE;

    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) && *arg != '-' && *arg != '+' )
            return FALSE;
    }

    return TRUE;
}


/*
    Given a string like 14.foo, return 14 and 'foo'
*/
int number_argument( char* argument, char* arg )
{
    char* pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '.' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '.';
            strcpy( arg, pdot + 1 );
            return number;
        }
    }

    strcpy( arg, argument );
    return 1;
}



/*
    Pick off one argument from a string and return the rest.
    Understands quotes.
*/
char* one_argument( const char* argument, char* arg_first )
{
    char cEnd;
    sh_int count;
    count = 0;

    while ( isspace( *argument ) )
        argument++;

    cEnd = ' ';

    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }

        *arg_first = LOWER( *argument );
        arg_first++;
        argument++;
    }

    *arg_first = '\0';

    while ( isspace( *argument ) )
        argument++;

    return ( char* ) argument;
}

/*
    Pick off one argument from a string and return the rest.
    Understands quotes.  Delimiters = { ' ', '-' }
*/
char* one_argument2( const char* argument, char* arg_first )
{
    char cEnd;
    sh_int count;
    count = 0;

    while ( isspace( *argument ) )
        argument++;

    cEnd = ' ';

    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
        if ( *argument == cEnd || *argument == '-' )
        {
            argument++;
            break;
        }

        *arg_first = LOWER( *argument );
        arg_first++;
        argument++;
    }

    *arg_first = '\0';

    while ( isspace( *argument ) )
        argument++;

    return ( char* ) argument;
}

/*
    Pick off one argument from a string and return the rest.
    Understands quotes. Saves the case.
*/
char* one_argument_sc( const char* argument, char* arg_first )
{
    char cEnd;
    sh_int count;
    count = 0;

    while ( isspace( *argument ) )
        argument++;

    cEnd = ' ';

    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }

        *arg_first = *argument;
        arg_first++;
        argument++;
    }

    *arg_first = '\0';

    while ( isspace( *argument ) )
        argument++;

    return ( char* ) argument;
}

bool is_plain_text( char* text )
{
    char* pc;
    bool fIll;
    fIll = TRUE;

    for ( pc = text; *pc != '\0'; pc++ )
    {
        if ( !isalpha( *pc ) && !isdigit( *pc ) && *pc != ' ' )
            return FALSE;

        if ( LOWER( *pc ) != 'i' && LOWER( *pc ) != 'l' )
            fIll = FALSE;
    }

    if ( fIll )
        return FALSE;

    return TRUE;
}

bool contains_space( char* text )
{
    char* pc;

    for ( pc = text; *pc != '\0'; pc++ )
    {
        if ( *pc == ' ' )
            return TRUE;
    }

    return FALSE;
}

void do_timecmd( CHAR_DATA* ch, char* argument )
{
    struct timeval stime;
    struct timeval etime;
    static bool timing;
    extern CHAR_DATA* timechar;
    char arg[MAX_INPUT_LENGTH];
    send_to_char( "Timing\n\r", ch );

    if ( timing )
        return;

    one_argument( argument, arg );

    if ( !*arg )
    {
        send_to_char( "No command to time.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "update" ) )
    {
        if ( timechar )
            send_to_char( "Another person is already timing updates.\n\r", ch );
        else
        {
            timechar = ch;
            send_to_char( "Setting up to record next update loop.\n\r", ch );
        }

        return;
    }

    send_to_char( "&zStarting timer.\n\r", ch );
    timing = TRUE;
    gettimeofday( &stime, NULL );
    interpret( ch, argument, FALSE );
    gettimeofday( &etime, NULL );
    timing = FALSE;
    send_to_char( "&zTiming complete.\n\r", ch );
    subtract_times( &etime, &stime );
    ch_printf( ch, "&zTiming took &C%d.%06d &zseconds.\n\r",
               etime.tv_sec, etime.tv_usec );
    return;
}

void start_timer( struct timeval* stime )
{
    if ( !stime )
    {
        bug( "Start_timer: NULL stime.", 0 );
        return;
    }

    gettimeofday( stime, NULL );
    return;
}

time_t end_timer( struct timeval* stime )
{
    struct timeval etime;
    /* Mark etime before checking stime, so that we get a better reading.. */
    gettimeofday( &etime, NULL );

    if ( !stime || ( !stime->tv_sec && !stime->tv_usec ) )
    {
        bug( "End_timer: bad stime.", 0 );
        return 0;
    }

    subtract_times( &etime, stime );
    /* stime becomes time used */
    *stime = etime;
    return ( etime.tv_sec * 1000000 ) + etime.tv_usec;
}

void send_timer( struct timerset* vtime, CHAR_DATA* ch )
{
    struct timeval ntime;
    int carry;

    if ( vtime->num_uses == 0 )
        return;

    ntime.tv_sec  = vtime->total_time.tv_sec / vtime->num_uses;
    carry = ( vtime->total_time.tv_sec % vtime->num_uses ) * 1000000;
    ntime.tv_usec = ( vtime->total_time.tv_usec + carry ) / vtime->num_uses;
    ch_printf( ch, "Has been used %d times this boot.\n\r", vtime->num_uses );
    ch_printf( ch, "Time (in secs): min %d.%0.6d; avg: %d.%0.6d; max %d.%0.6d"
               "\n\r", vtime->min_time.tv_sec, vtime->min_time.tv_usec, ntime.tv_sec,
               ntime.tv_usec, vtime->max_time.tv_sec, vtime->max_time.tv_usec );
    return;
}

void update_userec( struct timeval* time_used, struct timerset* userec )
{
    userec->num_uses++;

    if ( !timerisset( &userec->min_time )
            ||    timercmp( time_used, &userec->min_time, < ) )
    {
        userec->min_time.tv_sec  = time_used->tv_sec;
        userec->min_time.tv_usec = time_used->tv_usec;
    }

    if ( !timerisset( &userec->max_time )
            ||    timercmp( time_used, &userec->max_time, > ) )
    {
        userec->max_time.tv_sec  = time_used->tv_sec;
        userec->max_time.tv_usec = time_used->tv_usec;
    }

    userec->total_time.tv_sec  += time_used->tv_sec;
    userec->total_time.tv_usec += time_used->tv_usec;

    while ( userec->total_time.tv_usec >= 1000000 )
    {
        userec->total_time.tv_sec++;
        userec->total_time.tv_usec -= 1000000;
    }

    return;
}

void get_last_arg( char* arg, char* str )
{
    char name[MAX_INPUT_LENGTH];
    int lp = 0, cnt = 0, tt = 0;

    if ( !arg || arg[0] == '\0' )
        return;

    tt = count_args( arg );

    for ( ; ; )
    {
        if ( ++lp > 999 )
        {
            bug( "get_last_arg: trapped in infinate loop, aborting." );
            return;
        }

        arg = one_argument( arg, name );

        if ( ++cnt == tt )
        {
            strcpy( str, name );
            return;
        }
    }
}

int count_args( char* argument )
{
    char* arg;
    arg = argument;
    return cnt_arg( arg, 0 );
}

int cnt_arg( char* argument, int cnt )
{
    char buf[MAX_STRING_LENGTH];
    argument = one_argument( argument, buf );

    if ( buf[0] == '\0' )
        return cnt;

    return cnt_arg( argument, cnt + 1 );
}


