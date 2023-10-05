/****************************************************************************
    [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//
    -----------------------------------------------------------|   (0...0)
    SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(
    -----------------------------------------------------------|    {o o}
    SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \
    Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~
    Tricops and Fireblade                                      |
    ------------------------------------------------------------------------
    Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael
    Chastain, Michael Quan, and Mitchell Tse.
    Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,
    Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.
    ------------------------------------------------------------------------
               Pfile autocleanup code
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "mud.h"

/* Globals */
time_t        pfile_time;
HOUR_MIN_SEC        set_pfile_time_struct;
HOUR_MIN_SEC*       set_pfile_time;
struct tm*          new_pfile_time;
struct tm           new_pfile_struct;
time_t new_pfile_time_t;
sh_int  num_pfiles; /* Count up number of pfiles */

#ifndef FCLOSE
    #define FCLOSE(fp)  fclose(fp); fp=NULL;
#endif

#ifndef SOLANCODE
void save_timedata( void )
{
    FILE* fp;
    char filename[MAX_INPUT_LENGTH];
    sprintf( filename, "%stime.dat", SYSTEM_DIR );
    FCLOSE( fpReserve );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "save_timedata: fopen" );
        perror( filename );
    }
    else
    {
        fprintf( fp, "#TIME\n" );
        fprintf( fp, "Purgetime %ld\n", new_pfile_time_t   );
        fprintf( fp, "End\n\n"                        );
        fprintf( fp, "#END\n"                     );
    }

    FCLOSE( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

#ifdef KEY
    #undef KEY
#endif
#define KEY( literal, field, value )                                    \
    if ( !str_cmp( word, literal ) )        \
    {                                       \
        field  = value;                     \
        fMatch = TRUE;                      \
        break;                              \
    }

/* Reads the actual time file from disk - Samson 1-21-99 */
void fread_timedata( FILE* fp )
{
    char* word = NULL;
    bool fMatch = FALSE;

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

            case 'E':
                if ( !str_cmp( word, "End" ) )
                    return;

                break;

            case 'M':
                KEY( "Mhour",   time_info.hour,     fread_number( fp ) );
                KEY( "Mday",    time_info.day,      fread_number( fp ) );
                KEY( "Mmonth",  time_info.month,        fread_number( fp ) );
                KEY( "Myear",   time_info.year,     fread_number( fp ) );
                break;

            case 'P':
                KEY( "Purgetime", new_pfile_time_t,     fread_number( fp ) );
                break;
        }

        if ( !fMatch )
        {
            bug( "Fread_timedata: no match: %s", word );
            fread_to_eol( fp );
        }
    }
}

bool load_timedata( void )
{
    char filename[MAX_INPUT_LENGTH];
    FILE* fp;
    bool found;
    found = FALSE;
    sprintf( filename, "%stime.dat", SYSTEM_DIR );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        found = TRUE;

        for ( ; ; )
        {
            char letter = '\0';
            char* word = NULL;
            letter = fread_letter( fp );

            if ( letter == '*' )
            {
                fread_to_eol( fp );
                continue;
            }

            if ( letter != '#' )
            {
                bug( "Load_timedata: # not found." );
                break;
            }

            word = fread_word( fp );

            if ( !str_cmp( word, "TIME" ) )
            {
                fread_timedata( fp );
                break;
            }
            else if ( !str_cmp( word, "END"  ) )
                break;
            else
            {
                bug( "Load_timedata: bad section - %s.", word );
                break;
            }
        }

        FCLOSE( fp );
    }

    return found;
}
#endif

void init_pfile_scan_time( void )
{
    /*
        Init pfile scan time.
    */
    set_pfile_time = &set_pfile_time_struct;
    new_pfile_time = update_time( localtime( &current_time ) );
    /*  Copies *new_pfile_time to new_pfile_struct, and then points
        new_pfile_time to new_pfile_struct again. -- Alty */
    new_pfile_struct = *new_pfile_time;
    new_pfile_time = &new_pfile_struct;
    new_pfile_time->tm_mday += 1;

    if ( new_pfile_time->tm_hour > 12 )
        new_pfile_time->tm_mday += 1;

    new_pfile_time->tm_sec = 0;
    new_pfile_time->tm_min = 0;
    new_pfile_time->tm_hour = 3;
    /* Update new_pfile_time (due to day increment) */
    new_pfile_time = update_time( new_pfile_time );
    new_pfile_struct = *new_pfile_time;
    new_pfile_time = &new_pfile_struct;
    /* Bug fix submitted by Gabe Yoder */
    new_pfile_time_t = mktime( new_pfile_time );
    /* check_pfiles(mktime(new_pfile_time)); */
#ifndef SOLANCODE

    if ( !load_timedata() )
    {
        strcpy( log_buf, "Pfile scan time reset to default time of 3am." );
        log_string( log_buf );
    }

#endif
    return;
}

time_t  now_time;
sh_int  deleted = 0;
sh_int   days = 0;

#if defined(KEY)
    #undef KEY
#endif

#define KEY( literal, field, value )                    \
    if ( !strcmp( word, literal ) ) \
    {                   \
        field  = value;         \
        fMatch = TRUE;          \
        break;              \
    }

void fread_pfile( FILE* fp, time_t tdiff, char* fname )
{
    char* word;
    char* name = NULL;
    sh_int level = 0;
    sh_int file_ver = 0;
    bool fMatch;

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );

        if ( word == NULL )
        {
            bug( "fread_word error on character %s", fname );
            return;
        }

        fMatch = FALSE;

        switch ( UPPER( word[0] ) )
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol( fp );
                break;

            case 'E':
                if ( !strcmp( word, "End" ) )
                    goto timecheck;

                break;

            case 'L':
                KEY( "Toplevel",            level,  fread_number( fp ) );
                break;

            case 'N':
                KEY( "Name",        name,   fread_string( fp ) );
                break;

            case 'V':
                KEY( "Version", file_ver,   fread_number( fp ) );
                break;
        }

        if ( !fMatch )
            fread_to_eol( fp );
    }

timecheck:

    if ( level > 0 && ( level < 2 && tdiff > sysdata.newbie_purge ) || ( level < LEVEL_IMMORTAL && tdiff > sysdata.regular_purge ) && level <= 100 )
    {
        if ( level < LEVEL_IMMORTAL )
        {
            if ( unlink( fname ) == -1 )
                perror( "Unlink" );
            else
            {
                char pHome[MAX_STRING_LENGTH];
                /* Try to delete the home, also */
                sprintf( pHome, "%s.home", fname );

                if ( !remove( pHome ) ) { } /* Success! */

                /* Cleanup extra data! */
                // clean_player_ships( name );

                if ( level < 10 )
                    days = sysdata.newbie_purge;
                else
                    days = sysdata.regular_purge;

                sprintf( log_buf, "Player %s was deleted. Exceeded time limit of %d days.", name, days );
                log_string( log_buf );
#ifdef AUTO_AUTH
                remove_from_auth( name );
#endif
                ++deleted;
                return;
            }
        }
    }

    /* PFILE Survived? */
    return;
}

void read_pfile( char* dirname, char* filename )
{
    FILE* fp;
    char fname[MAX_STRING_LENGTH];
    struct stat fst;
    time_t tdiff;
    now_time = time( 0 );
    sprintf( fname, "%s/%s", dirname, filename );

    if ( stat( fname, &fst ) != -1 )
    {
        tdiff = ( now_time - fst.st_mtime ) / 86400;

        if ( ( fp = fopen ( fname, "r" ) ) != NULL )
        {
            for ( ; ; )
            {
                char letter;
                char* word;
                letter = fread_letter( fp );

                if ( letter != '#' )
                    continue;

                word = fread_word( fp );

                if ( !str_cmp( word, "End" ) )
                    break;

                if ( !str_cmp( word, "PLAYER" ) )
                    fread_pfile( fp, tdiff, fname );
                else if ( !str_cmp( word, "END" ) ) /* Done     */
                    break;
            }

            FCLOSE( fp );
        }
    }

    return;
}

void pfile_scan( bool count )
{
    DIR* dp;
    struct dirent* dentry;
    char dir_name[100];
    sh_int alpha_loop;
    sh_int cou = 0;
    now_time = time( 0 );
    nice( 20 );

    for ( alpha_loop = 0; alpha_loop <= 25; alpha_loop++ )
    {
        sprintf( dir_name, "%s%c", PLAYER_DIR, 'a' + alpha_loop );
        dp = opendir( dir_name );
        dentry = readdir( dp );

        while ( dentry )
        {
            if ( dentry->d_name[0] != '.' )
            {
                if ( !count )
                    read_pfile( dir_name, dentry->d_name );

                cou++;
            }

            dentry = readdir( dp );
        }

        closedir( dp );
    }

    if ( !count )
        log_string( "Pfile cleanup completed." );
    else
        log_string( "Pfile count completed." );

    sprintf( log_buf, "Total pfiles scanned: %d", cou );
    log_string( log_buf );

    if ( !count )
    {
        sprintf( log_buf, "Total pfiles deleted: %d", deleted );
        log_string( log_buf );
        sprintf( log_buf, "Total pfiles remaining: %d", cou - deleted );
        num_pfiles = cou - deleted;
        log_string( log_buf );
    }
    else
        num_pfiles = cou;

    return;
}

void do_pfiles( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mobs cannot use this command!\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' || !argument )
    {
        /* Makes a backup copy of existing pfiles just in case - Samson */
        strcpy( buf, "tar -cf " );
        strcat( buf, PLAYER_DIR );
        strcat( buf, "pfiles.tar " );
        strcat( buf, PLAYER_DIR );
        strcat( buf, "*/*" );
        /*  GAH, the shell pipe won't process the command that gets pieced
            together in the preceeding lines! God only knows why. - Samson */
        // system( buf );
        sprintf( log_buf, "Manual pfile cleanup started by %s.", ch->name );
        log_string( log_buf );
        pfile_scan( FALSE );
        return;
    }

    if ( !str_cmp( argument, "settime" ) )
    {
        new_pfile_time_t = current_time + 86400;
        save_timedata( );
        send_to_char( "New cleanup time set for 24 hrs from now.\n\r", ch );
        return;
    }

    if ( !str_cmp( argument, "count" ) )
    {
        sprintf( log_buf, "Pfile count started by %s.", ch->name );
        log_string( log_buf );
        pfile_scan( TRUE );
        return;
    }

    send_to_char( "Invalid argument.\n\r", ch );
    return;
}

void check_pfiles( time_t reset )
{
    /* This only counts them up on reboot if the cleanup isn't needed - Samson 1-2-00 */
    if ( reset == 255 && new_pfile_time_t > current_time )
    {
        reset = 0; /* Call me paranoid, but it might be meaningful later on */
        log_string( "Counting pfiles....." );
        pfile_scan( TRUE );
        return;
    }

    if ( new_pfile_time_t <= current_time )
    {
        if ( sysdata.CLEANPFILES == TRUE )
        {
            char buf[MAX_STRING_LENGTH];
            /* Makes a backup copy of existing pfiles just in case - Samson */
            strcpy( buf, "tar -cf " );
            strcat( buf, PLAYER_DIR );
            strcat( buf, "pfiles.tar " );
            strcat( buf, PLAYER_DIR );
            strcat( buf, "*/*" );
            /*  Would use the shell pipe for this, but alas, it requires a ch in order
                to work, this also gets called during boot_db before the rare item
                checks for the rent code - Samson */
            system( buf );
            new_pfile_time_t = current_time + 86400;
            save_timedata( );
            log_string( "Automated pfile cleanup beginning...." );
            pfile_scan( FALSE );
#ifdef RENTCODE

            if ( reset == 0 )
                rent_update();

#endif
        }
        else
        {
            new_pfile_time_t = current_time + 86400;
            save_timedata( );
            log_string( "Counting pfiles....." );
            pfile_scan( TRUE );
#ifdef RENTCODE

            if ( reset == 0 )
                rent_update();

#endif
        }
    }

    return;
}
