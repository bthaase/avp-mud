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
                                 Bots Module
****************************************************************************/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

BOT_DATA* first_bot;
BOT_DATA* last_bot;

void write_bot_list( )
{
    BOT_DATA* tbot;
    FILE* fpout;
    char filename[256];
    sprintf( filename, "%s%s", BOT_DIR, BOT_LIST );
    fpout = fopen( filename, "w" );

    if ( !fpout )
    {
        bug( "FATAL: cannot open bots.lst for writing!\n\r", 0 );
        return;
    }

    for ( tbot = first_bot; tbot; tbot = tbot->next )
    {
        fprintf( fpout, "%s\n", tbot->filename );
    }

    fprintf( fpout, "$\n" );
    fclose( fpout );
}

/*
    Save a bot's data to its data file
*/
void save_bot( BOT_DATA* bot )
{
    FILE* fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !bot )
    {
        bug( "save_bot: null bot pointer!", 0 );
        return;
    }

    if ( !bot->filename || bot->filename[0] == '\0' )
    {
        sprintf( buf, "save_bot: %s has no filename", bot->name );
        bug( buf, 0 );
        return;
    }

    sprintf( filename, "%s%s", BOT_DIR, bot->filename );
    fclose( fpReserve );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "save_bot: fopen", 0 );
        perror( filename );
    }
    else
    {
        fprintf( fp, "#BOT\n" );
        fprintf( fp, "Name         %s~\n",      bot->name );
        fprintf( fp, "Filename     %s~\n",      bot->filename    );
        fprintf( fp, "Level        %d\n",       bot->state );
        fprintf( fp, "Favweap      %d\n",       bot->favweap );
        fprintf( fp, "Fear         %d\n",       bot->fear );
        fprintf( fp, "Attack       %d\n",       bot->attack );
        fprintf( fp, "Camping      %d\n",       bot->camping );
        fprintf( fp, "Blasting     %d\n",       bot->blasting );
        fprintf( fp, "Race         %d\n",       bot->race );
        fprintf( fp, "End\n\n"                               );
        fprintf( fp, "#END\n"                                );
    }

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

#if defined(KEY)
    #undef KEY
#endif

#define KEY( literal, field, value )                    \
    if ( !str_cmp( word, literal ) )    \
    {                   \
        field  = value;         \
        fMatch = TRUE;          \
        break;              \
    }


void fread_bot( BOT_DATA* bot, FILE* fp )
{
    char buf[MAX_STRING_LENGTH];
    char* word;
    bool fMatch;

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER( word[0] ) )
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol( fp );
                break;

            case 'A':
                KEY( "Attack",      bot->attack,            fread_number( fp ) );
                break;

            case 'B':
                KEY( "Blasting",    bot->blasting,            fread_number( fp ) );
                break;

            case 'C':
                KEY( "Camping",     bot->camping,            fread_number( fp ) );
                break;

            case 'E':
                if ( !str_cmp( word, "End" ) )
                {
                    if ( !bot->name )
                        bot->name = STRALLOC( "" );

                    if ( !bot->filename )
                        bot->filename = STRALLOC( "" );

                    if ( bot->prev != NULL )
                        bot->id = bot->prev->id + 1;
                    else
                        bot->id = 1;

                    return;
                }

                break;

            case 'F':
                KEY( "Filename",    bot->filename,         fread_string_nohash( fp ) );
                KEY( "Favweap",     bot->favweap,          fread_number( fp ) );
                KEY( "Fear",        bot->fear,             fread_number( fp ) );
                break;

            case 'L':
                KEY( "Level",       bot->level,            fread_number( fp ) );
                break;

            case 'N':
                KEY( "Name",        bot->name,             fread_string( fp ) );
                break;

            case 'R':
                KEY( "Race",        bot->race,             fread_number( fp ) );
                break;
        }

        if ( !fMatch )
        {
            sprintf( buf, "Fread_bot: no match: %s", word );
            bug( buf, 0 );
        }
    }
}

/*
    Load a bot file
*/
bool load_bot( char* botfile )
{
    char filename[256];
    BOT_DATA* bot;
    FILE* fp;
    bool found;
    CREATE( bot, BOT_DATA, 1 );
    found = FALSE;
    sprintf( filename, "%s%s", BOT_DIR, botfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        found = TRUE;
        LINK( bot, first_bot, last_bot, next, prev );

        for ( ; ; )
        {
            char letter;
            char* word;
            letter = fread_letter( fp );

            if ( letter == '*' )
            {
                fread_to_eol( fp );
                continue;
            }

            if ( letter != '#' )
            {
                bug( "Load_bot_file: # not found.", 0 );
                break;
            }

            word = fread_word( fp );

            if ( !str_cmp( word, "BOT"        ) )
            {
                fread_bot( bot, fp );
                break;
            }
            else if ( !str_cmp( word, "END"  ) )
                break;
            else
            {
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "Load_bot_file: bad section: %s.", word );
                bug( buf, 0 );
                break;
            }
        }

        fclose( fp );
    }
    else
        bug( "load_bot: failed to open bot file!" );

    if ( !( found ) )
        DISPOSE( bot );

    return found;
}

void load_bots( )
{
    FILE* fpList;
    char* filename;
    char botlist[256];
    char buf[MAX_STRING_LENGTH];
    first_bot    = NULL;
    last_bot     = NULL;
    sprintf( botlist, "%s%s", BOT_DIR, BOT_LIST );
    fclose( fpReserve );

    if ( ( fpList = fopen( botlist, "r" ) ) == NULL )
    {
        perror( botlist );
        exit( 1 );
    }

    for ( ; ; )
    {
        filename = feof( fpList ) ? "$" : fread_word( fpList );

        if ( filename[0] == '$' )
            break;

        if ( !load_bot( filename ) )
        {
            sprintf( buf, "Cannot load bot file: %s", filename );
            bug( buf, 0 );
        }
    }

    fclose( fpList );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
    Get pointer to an bot from the bot ID.
*/
BOT_DATA* bot_from_id( int id )
{
    BOT_DATA* bot;

    for ( bot = first_bot; bot; bot = bot->next )
        if ( bot->id == id )
            return bot;

    return NULL;
}


/*
    Get pointer to an bot from the bot name.
*/
BOT_DATA* bot_from_name( char* name )
{
    BOT_DATA* bot;

    if ( ( bot = bot_from_id( atoi( name ) ) ) != NULL )
        return bot;

    for ( bot = first_bot; bot; bot = bot->next )
        if ( !str_cmp( name, bot->name ) )
            return bot;

    for ( bot = first_bot; bot; bot = bot->next )
        if ( !str_prefix( name, bot->name ) )
            return bot;

    return NULL;
}

/*
    Makes a new bot structure
*/
void do_makebot( CHAR_DATA* ch, char* argument )
{
    char arg[MIL];
    char filename[MIL];
    BOT_DATA* bot;
    argument = one_argument( argument, arg );

    if ( NULLSTR( argument ) )
    {
        send_to_char( "Usage: makebot <filename> <bot name>\n\r", ch );
        return;
    }

    CREATE( bot, BOT_DATA, 1 );
    LINK( bot, first_bot, last_bot, next, prev );

    if ( bot->prev == NULL )
        bot->id = 1;
    else
        bot->id = bot->prev->id + 1;

    bot->name            = STRALLOC( argument );
    strncpy( filename, arg, MIL );
    bot->filename = str_dup( filename );
    save_bot( bot );
    write_bot_list();
}

void do_allbots( CHAR_DATA* ch, char* argument )
{
    char info[MAX_STRING_LENGTH];
    BOT_DATA* bot;
    int count = 0;
    ch_printf( ch, "&zBots: -----------------------\n\r" );

    for ( bot = first_bot; bot; bot = bot->next )
    {
        sprintf( info, "%2d &z| %s", bot->level, bot->loaded ? "&GLoaded" : "&RUnloaded" );
        ch_printf( ch, "&z[&C%-3d&z]: [ &B%-26s&z]  &C%s\n\r", bot->id, info, bot->name );
        count++;
    }

    if ( !count )
    {
        send_to_char( "There are no bots currently formed.\n\r", ch );
        return;
    }
}

void do_showbot( CHAR_DATA* ch, char* argument )
{
    BOT_DATA* bot;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: showbot <name>\n\r", ch );
        return;
    }

    bot = bot_from_name( argument );

    if ( bot == NULL )
    {
        send_to_char( "&RBot not found.\n\r", ch );
        return;
    }

    ch_printf( ch, "&zName:      &C%s &z[&WID: %d&z]\n\r", bot->name, bot->id );
    ch_printf( ch, "&zFilename:  &C%s\n\r", bot->filename );
    // ch_printf( ch, "&zPeople Inside: &C%d\n\r", count );
    // ch_printf( ch, "&z---&BRooms&z-----------------------------------------------\n\r" );
    // ch_printf( ch, "&zFirstroom: &C%d\n\r", arena->firstroom );
    // ch_printf( ch, "&zLastroom:  &C%d\n\r", arena->lastroom );
    // ch_printf( ch, "&z---&BStats&z-----------------------------------------------\n\r" );
    // ch_printf( ch, "&zMinp:  &C%d           &zMaxp: &C%d\n\r", arena->min_p, arena->max_p );
    // ch_printf( ch, "&zTimer: &C%d\n\r", arena->timer );
    return;
}

void do_setbot( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    BOT_DATA* bot;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "&zSyntax: setbot <name> <field> <value>\n\r\n\r", ch );
        send_to_char( "&zField being one of:\n\r", ch );
        send_to_char( " &zname, filename, level, state, favweap, fear, attack,\n\r", ch );
        send_to_char( " &zcamping, blasting, loaded, race\n\r", ch );
        return;
    }

    bot = bot_from_name( arg1 );

    if ( bot == NULL )
    {
        send_to_char( "&RBot not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        STRFREE( bot->name );
        bot->name = STRALLOC( argument );
        send_to_char( "Bot name set.\n\r", ch );
        save_bot( bot );
    }
    else if ( !str_cmp( arg2, "filename" ) )
    {
        STRFREE( bot->filename );
        bot->filename = STRALLOC( argument );
        send_to_char( "Bot filename set. (Resaving bots)\n\r", ch );
        save_bot( bot );
        write_bot_list();
    }
    else if ( !str_cmp( arg2, "race" ) )
    {
        bot->race = URANGE( 0, atoi( argument ), MAX_RACE );
        send_to_char( "Bot race set.\n\r", ch );
        save_bot( bot );
    }
    else if ( !str_cmp( arg2, "level" ) )
    {
        bot->level = URANGE( 0, atoi( argument ), 200 );
        send_to_char( "Bot level set.\n\r", ch );
        save_bot( bot );
    }
    else if ( !str_cmp( arg2, "state" ) )
    {
        bot->state = URANGE( 0, atoi( argument ), 100 );
        send_to_char( "Bot state set.\n\r", ch );
        save_bot( bot );
    }
    else if ( !str_cmp( arg2, "favweap" ) )
    {
        bot->favweap = URANGE( 0, atoi( argument ), 100 );
        send_to_char( "Bot's favorite weapon set.\n\r", ch );
        save_bot( bot );
    }
    else if ( !str_cmp( arg2, "fear" ) )
    {
        bot->fear = URANGE( 0, atoi( argument ), 100 );
        send_to_char( "Bot fear % set.\n\r", ch );
        save_bot( bot );
    }
    else if ( !str_cmp( arg2, "attack" ) )
    {
        bot->attack = URANGE( 0, atoi( argument ), 100 );
        send_to_char( "Bot attack % set.\n\r", ch );
        save_bot( bot );
    }
    else if ( !str_cmp( arg2, "camping" ) )
    {
        bot->blasting = URANGE( 0, atoi( argument ), 100 );
        send_to_char( "Bot camping % set.\n\r", ch );
        save_bot( bot );
    }
    else if ( !str_cmp( arg2, "blasting" ) )
    {
        bot->blasting = URANGE( 0, atoi( argument ), 100 );
        send_to_char( "Bot blasting % set.\n\r", ch );
        save_bot( bot );
    }
    else if ( !str_cmp( arg2, "loaded" ) )
    {
        if ( bot->loaded )
        {
            bot_unload( bot );
            send_to_char( "Loaded is set to false.\n\r", ch );
        }
        else
        {
            bot_load( bot );
            send_to_char( "Loaded is set to true.\n\r", ch );
        }

        save_bot( bot );
    }
    else
        do_setbot( ch, "" );
}

void bot_load( BOT_DATA* bot )
{
    MOB_INDEX_DATA*   pIndex;
    CHAR_DATA* ch;

    // Generate the CH structure
    if ( ( pIndex = get_mob_index( MOB_VNUM_BOT ) ) == NULL )
    {
        bug( "bots.c :: bot_load failed to load bot index." );
        return;
    }

    ch = create_mobile( pIndex );
    STRFREE( ch->name );
    STRFREE( ch->short_descr );
    STRFREE( ch->long_descr );
    ch->name                            = STRALLOC( bot->name );
    ch->short_descr                     = STRALLOC( bot->name );
    ch->long_descr                      = STRALLOC( "A Bot\n\r" );
    ch->top_level                       = UMAX( bot->level, 1 );
    ch->max_hit                         = race_table[bot->race].hit;
    ch->max_move                        = race_table[bot->race].move;
    ch->hit                             = ch->max_hit;
    ch->move                            = ch->max_move;
    ch->field                           = 100;
    ch->max_field                       = 100;
    ch->race                            = bot->race;
    ch->sex                             = 1;
    ch->carried                         = NULL;
    ch->carrying                        = NULL;
    ch->perm_str                        = 20;
    ch->perm_sta                        = 20;
    ch->perm_rec                        = 20;
    ch->perm_int                        = 20;
    ch->perm_bra                        = 20;
    ch->perm_per                        = 20;
    ch->mental_state                    = 0;
    ch->mobinvis                        = 0;
    ch->morale                          = get_max_morale( ch );
    ch->position                        = POS_STANDING;
    ch->ap                              = get_max_ap( ch );
    ch->mp                              = get_max_mp( ch );
    char_to_room( ch, get_room_index( wherehome( ch ) ) );
    ch->bot                             = bot;
    bot->ch = ch;
    {
        char tmp[MAX_STRING_LENGTH];
        sprintf( tmp, "&GAvP welcomes %s to the fray.", ch->name );
        send_monitor( ch, tmp );
        act( AT_ACTION, "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
    }
    // Set the rest of the botting values
    bot->arena = FALSE;
    bot->respawn = 0;
    bot->state = 0;
    bot->botspeak = 0;
    bot->loaded = TRUE;
    return;
}

void bot_unload( BOT_DATA* bot )
{
    if ( bot->ch != NULL )
    {
        char tmp[MAX_STRING_LENGTH];
        sprintf( tmp, "&W%s has left the game.", bot->ch->name );
        send_monitor( bot->ch, tmp );
        act( AT_BYE, "$n has left the game.", bot->ch, NULL, NULL, TO_ROOM );
        extract_char( bot->ch, TRUE, FALSE );
        bot->ch = NULL;
    }

    bot->loaded = FALSE;
    return;
}

/*
    Bot Core Update Routine (Called from update.c)
*/
void bot_update( )
{
    BOT_DATA* bot;

    for ( bot = first_bot; bot; bot = bot->next )
    {
        /* Dont update bots that have no shells */
        if ( bot->ch == NULL )
            continue;

        /* Respawn Driver */
        if ( bot->respawn > 0 )
        {
            bot->respawn -= 2;

            if ( bot->respawn <= 0 )
            {
                bot->respawn = 0;
                char_from_room( bot->ch );
                char_to_room( bot->ch, get_room_index( wherehome( bot->ch ) ) );
                act( AT_ACTION, "$n suddenly appears.", bot->ch, NULL, NULL, TO_ROOM );
            }

            continue;
        }

        /*
            More components below...
        */
    }

    return;
}
