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
                         Low-level communication module
****************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>
#include "mud.h"
#include "mqtt.h"
#include <systemd/sd-daemon.h>

/*
    Socket and TCP/IP stuff.
*/
#ifdef WIN32
    #include <io.h>
    #undef EINTR
    #undef EMFILE
    #define EINTR WSAEINTR
    #define EMFILE WSAEMFILE
    #define EWOULDBLOCK WSAEWOULDBLOCK
    #define MAXHOSTNAMELEN 32

    #define  TELOPT_ECHO        '\x01'
    #define  GA                 '\xF9'
    #define  SB                 '\xFA'
    #define  WILL               '\xFB'
    #define  WONT               '\xFC'
    #define  DO                 '\xFD'
    #define  DONT               '\xFE'
    #define  IAC                '\xFF'
    void bailout( void );
    void shutdown_checkpoint ( void );
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    /*  #include <netinet/in_systm.h> */
    #include <netinet/ip.h>
    #include <arpa/inet.h>
    #include <arpa/telnet.h>
    #include <netdb.h>
    #define closesocket close
#endif

#define MAX_NEST        100

const   char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str    [] = { IAC, GA, '\0' };

void    auth_maxdesc    args( ( int* md, fd_set* ins, fd_set* outs,
                                fd_set* excs ) );
void    auth_check      args( ( fd_set* ins, fd_set* outs, fd_set* excs ) );
void    set_auth        args( ( DESCRIPTOR_DATA* d ) );
void    kill_auth       args( ( DESCRIPTOR_DATA* d ) );

/*  from act_info?  */
void    show_condition( CHAR_DATA* ch, CHAR_DATA* victim );

/*
    Global variables.
*/
int              allowedmp( DESCRIPTOR_DATA* d );
DESCRIPTOR_DATA* first_descriptor;                /* First descriptor      */
DESCRIPTOR_DATA* last_descriptor;                 /* Last descriptor       */
DESCRIPTOR_DATA* d_next;                          /* Next descriptor in loop  */
int              num_descriptors;
FILE*            fpReserve;                       /* Reserved file handle  */
FILE*            fpMatch;                         /* Log for current match */
bool             mud_down;                        /* Shutdown              */
bool             wizlock;                         /* Game is wizlocked     */
time_t           boot_time;
HOUR_MIN_SEC     set_boot_time_struct;
HOUR_MIN_SEC*    set_boot_time;
struct tm*       new_boot_time;
struct tm        new_boot_struct;
char             str_boot_time[MAX_INPUT_LENGTH];
char             lastplayercmd[MAX_INPUT_LENGTH * 2];
time_t           current_time;   /* Time of this pulse       */
int              port;           /* Master MUD Port */
int              control = 0;    /* Controlling descriptor   */
int              control2 = 0;   /* Controlling descriptor #2    */
int              newdesc;        /* New descriptor       */
fd_set           in_set;         /* Set of desc's for reading    */
fd_set           out_set;        /* Set of desc's for writing    */
fd_set           exc_set;        /* Set of desc's with errors    */
int              maxdesc;
bool             emergency_copy; /* emergency Copyover Flag      */
bool             MOBtrigger;
struct xname_data* xnames;
struct allowmp_data* mplist;

/*
    OS-dependent local functions.
*/
void    game_loop               args( ( ) );
int     init_socket             args( ( int port ) );
void    new_descriptor          args( ( int new_desc ) );


/*
    Other local functions (OS-independent).
*/
bool    check_parse_name     args( ( char* name ) );
short   check_reconnect      args( ( DESCRIPTOR_DATA* d, char* name, bool fConn ) );
short   check_playing        args( ( DESCRIPTOR_DATA* d, char* name, bool kick ) );
bool    check_multi          args( ( DESCRIPTOR_DATA* d, char* name ) );
int     main                 args( ( int argc, char** argv ) );
void    nanny                args( ( DESCRIPTOR_DATA* d, char* argument ) );
bool    flush_buffer         args( ( DESCRIPTOR_DATA* d, bool fPrompt ) );
void    read_from_buffer     args( ( DESCRIPTOR_DATA* d ) );
void    stop_idling          args( ( CHAR_DATA* ch ) );
void    free_desc            args( ( DESCRIPTOR_DATA* d ) );
void    display_prompt       args( ( DESCRIPTOR_DATA* d ) );
int     make_color_sequence  args( ( const char* col, char* buf, DESCRIPTOR_DATA* d ) );
bool    pager_output         args( ( DESCRIPTOR_DATA* d ) );
void    set_pager_input      args( ( DESCRIPTOR_DATA* d, char* argument ) );

void    mail_count              args( ( CHAR_DATA* ch ) );
bool    service_shut_down;  /* Shutdown by operator closing down service */
bool    fCopyOver;
bool    systemd_watchdog_enabled;
uint64_t systemd_watchdog_interval;

const char* get_process_name(const char* path) {
    const char* name = strrchr(path, '/');
    if (name) {
        return name + 1;
    }
    return path;
}

#ifdef WIN32
    int mainthread( int argc, char** argv )
#else
    int main( int argc, char** argv )
#endif
{
    struct timeval now_time;
    /*
        Memory debugging if needed.
    */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif
    emergency_copy              = FALSE;
    num_descriptors             = 0;
    first_descriptor            = NULL;
    last_descriptor             = NULL;
    sysdata.NO_NAME_RESOLVING   = TRUE;
    sysdata.ALLOW_OOC           = TRUE;
    sysdata.SET_WIZLOCK         = FALSE;
    sysdata.WAIT_FOR_AUTH       = TRUE;
    /*
        Init time.
    */
    gettimeofday( &now_time, NULL );
    current_time = ( time_t ) now_time.tv_sec;
    boot_time = time( 0 );       /*  <-- I think this is what you wanted */
    strcpy( str_boot_time, ctime( &current_time ) );
    /*
        Init boot time.
    */
    set_boot_time = &set_boot_time_struct;
    /*  set_boot_time->hour   = 6;
        set_boot_time->min    = 0;
        set_boot_time->sec    = 0;*/
    set_boot_time->manual = 0;
    new_boot_time = update_time( localtime( &current_time ) );
    /*  Copies *new_boot_time to new_boot_struct, and then points
        new_boot_time to new_boot_struct again. -- Alty */
    new_boot_struct = *new_boot_time;
    new_boot_time = &new_boot_struct;
    new_boot_time->tm_mday += 1;

    if ( new_boot_time->tm_hour > 12 )
        new_boot_time->tm_mday += 1;

    new_boot_time->tm_sec = 0;
    new_boot_time->tm_min = 0;
    new_boot_time->tm_hour = 6;
    /* Update new_boot_time (due to day increment) */
    new_boot_time = update_time( new_boot_time );
    new_boot_struct = *new_boot_time;
    new_boot_time = &new_boot_struct;
    /* Set reboot time string for do_time */
    get_reboot_string();
    /* Pfile autocleanup initializer - Samson 5-8-99 */
    init_pfile_scan_time();
    systemd_watchdog_enabled = sd_watchdog_enabled( 0, &systemd_watchdog_interval );


    /*
        Reserve two channels for our use.
    */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }

    if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }

    /*
        Get the port number.
    */
    port = 8000;

    if ( argc > 1 )
    {
        if ( !is_number( argv[1] ) )
        {
            fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
            exit( 0 );
        }
        else if ( ( port = atoi( argv[1] ) ) <= 1024 )
        {
            fprintf( stderr, "Port number must be above 1024.\n" );
            exit( 0 );
        }
    }
    else
    {
        fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
        exit( 0 );
    }

    if ( argv[2] && argv[2][0] )
    {
        fCopyOver = TRUE;
        control = atoi( argv[3] );
        control2 = atoi( argv[4] );
    }
    else
    {
        fCopyOver = FALSE;
    }

    /*
        Run the game.
    */
#ifdef WIN32
    {
        /* Initialise Windows sockets library */
        unsigned short wVersionRequested = MAKEWORD( 1, 1 );
        WSADATA wsadata;
        int err;
        /* Need to include library: wsock32.lib for Windows Sockets */
        err = WSAStartup( wVersionRequested, &wsadata );

        if ( err )
        {
            fprintf( stderr, "Error %i on WSAStartup\n", err );
            exit( 1 );
        }

        /* standard termination signals */
        signal( SIGINT, ( void* ) bailout );
        signal( SIGTERM, ( void* ) bailout );
    }
#endif /* WIN32 */
    log_string( "Booting Database" );
    boot_db( fCopyOver );
    if(sysdata.exe_file == NULL)  {
        sysdata.exe_file = STRALLOC(get_process_name(argv[0]));
    }
    log_string( "Booting Monitor.." );
    init_vote( );
    // log_string("Booting Database");
    // boot_db( fCopyOver );
    wizlock = sysdata.SET_WIZLOCK;

    if ( wizlock )
        log_string( "Wizlock automaticly engaged." );

    /*
        Initialize any non-initialized ports
    */
    log_string( "Initializing sockets" );

    log_string( "Initializing MQTT" );
    mqtt_init();

    if ( !control )
        control  = init_socket( port    );

    if ( !control2 )
        control2 = init_socket( port + 1  );

    /* Bootup the web server */
    /* init_web( port + 3 ); */
    sprintf( log_buf, "AvP: Legend ready on port %d.", port );
    log_string( log_buf );
    /*
        if ( !wizlock ) wizlock = !wizlock;
    */
    game_loop( );
    mqtt_cleanup( );
    close_match( );
    close( control  );
    close( control2 );

    /* Shutdown the web server */
    shutdown_web();
    /* Try to cleanup a bit */
    memory_cleanup( );
#ifdef WIN32

    if ( service_shut_down )
    {
        CHAR_DATA* vch;

        /* Save all characters before booting. */
        for ( vch = first_char; vch; vch = vch->next )
            if ( !IS_NPC( vch ) )
            {
                shutdown_checkpoint ();
                save_char_obj( vch );
            }
    }

    /* Shut down Windows sockets */
    WSACleanup();                 /* clean up */
    kill_timer();                 /* stop timer thread */
#endif
    /*
        That's all, folks.
    */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}


int init_socket( int port )
{
    char hostname[64];
    struct sockaddr_in   sa;
    int x = 1;
    int fd;
    gethostname( hostname, sizeof( hostname ) );

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
        perror( "Init_socket: socket" );
        exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( void* ) &x, sizeof( x ) ) < 0 )
    {
        perror( "Init_socket: SO_REUSEADDR" );
        close( fd );
        exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
        struct  linger  ld;
        ld.l_onoff  = 1;
        ld.l_linger = 1000;

        if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER, ( void* ) &ld, sizeof( ld ) ) < 0 )
        {
            perror( "Init_socket: SO_DONTLINGER" );
            close( fd );
            exit( 1 );
        }
    }
#endif
    gethostbyname( hostname );
    getservbyname( "service", "mud" );
    memset( &sa, '\0', sizeof( sa ) );
    sa.sin_family   = AF_INET; /* hp->h_addrtype; */
    sa.sin_port     = htons( port );

    if ( bind( fd, ( struct sockaddr* ) &sa, sizeof( sa ) ) == -1 )
    {
        perror( "Init_socket: bind" );
        close( fd );
        exit( 1 );
    }

    if ( listen( fd, 50 ) < 0 )
    {
        perror( "Init_socket: listen" );
        close( fd );
        exit( 1 );
    }

    return fd;
}

static void SegVio()
{
    char buf[MAX_STRING_LENGTH];
    char bufB[MAX_STRING_LENGTH];
    char* strtime;
    strtime                    = ctime( &current_time );
    strtime[strlen( strtime ) - 1] = '\0';
    log_string( "--- SEGMENTATION VIOLATION ---" );
    capturebacktrace( "SegVio" );
    sprintf( buf, "%slastcmd.log", LOG_DIR );

    /* Stops logging at 5 megs */
    if ( file_size( buf ) < 5000000 )
    {
        sprintf( bufB, "%s :: %s", strtime, lastplayercmd );
        append_to_file( buf, bufB );
    }

    // log_string( lastplayercmd );

    // Are we allowing an emergency copyover?
    if ( emergency_copy == TRUE )
    {
        emergency_copyover( );
        log_string( "Emergency copyover not ready. Shutting down." );
    }
    else
    {
        log_string( "Emergency copyover not ready. Shutting down." );
    }

    exit( 0 );
    // return;
}

void emergency_copyover( void )
{
    FILE* fp;
    DESCRIPTOR_DATA* d;
    char buf[100], buf2[100], buf3[100], buf4[100], buf5[100];
    log_string( "--- Engaging Emergency Copyover! ---" );
    /* Shutdown the web server */
    shutdown_web();
    /* Close the match log */
    match_log( "CONTROL;Match Interrupted by emergency Copyover." );
    close_match( );
    fp = fopen ( COPYOVER_FILE, "w" );

    if ( !fp )
    {
        log_string ( "Could not write to copyover file!" );
        perror ( "emergency_copyover:fopen" );
        return;
    }

    sprintf ( buf, "\n\r [ALERT]: EMERGENCY COPYOVER - Keep calm, we might pull through!\n\r" );

    /* For each playing descriptor, save its state */
    for ( d = first_descriptor; d ; d = d->next )
    {
        CHAR_DATA* och = CH ( d );
        d_next = d->next; /* We delete from the list , so need to save this */

        if ( !och || !d->character || d->connected > CON_PLAYING ) /* drop those logging on */
        {
            write_to_descriptor ( d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0 );
            /* close_socket (d, FALSE); */
        }
        else
        {
            fprintf ( fp, "%d %s %s\n", d->descriptor, och->name, d->host );
            save_char_obj ( och );
            write_to_descriptor ( d->descriptor, buf, 0 );
        }
    }

    fprintf ( fp, "-1\n" );
    fclose ( fp );
    fclose ( fpReserve );
    fclose ( fpLOG );
    sprintf ( buf, "%d", port );
    sprintf ( buf2, "%d", control );
    sprintf ( buf3, "%d", control2 );
    /*
        close( control  );
        close( control2 );
    */
    execl ( EXE_FILE,  "avp", buf, "copyover", buf2, buf3, buf4, buf5, ( char* ) NULL );
    execl ( EXE2_FILE, "avp", buf, "copyover", buf2, buf3, buf4, buf5, ( char* ) NULL );
    perror ( "emergency_copyover: failed to copyover in 'execl'" );

    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }

    if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }
}

/*
    void SegVio(int sig)
    {
    FILE *fp;
    DESCRIPTOR_DATA *d;
    char buf[100], buf2[100], buf3[100], buf4[100], buf5[100];

    if(sig != SIGSEGV)
    {
     sprintf( buf, "Caught unknown signal: %d\n", sig );
     log_string( buf );
     printf( buf );
     return;
    }

    log_string( "ALERT: Segmentation Violation! emergency COPYOVER Attempted!" );

    for (d = first_descriptor; d ; d = d->next)
    {
     CHAR_DATA * och = CH (d);
     d_next = d->next;

     if (!och || !d->character || d->connected > CON_PLAYING)
     {
     write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
     }
     else
     {
     fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
     save_char_obj (och);
     write_to_descriptor (d->descriptor, buf, 0);
     }
    }

    fprintf (fp, "-1\n");
    fclose (fp);

    fclose (fpReserve);
    fclose (fpLOG);

    sprintf (buf, "%d", port);
    sprintf (buf2, "%d", control);
    sprintf (buf3, "%d", control2);

    execl (EXE_FILE,  "avp", buf, "copyover", buf2, buf3, buf4, buf5, (char *) NULL);

    execl (EXE2_FILE, "avp", buf, "copyover", buf2, buf3, buf4, buf5, (char *) NULL);

    perror ("do_copyover: failed to copyover in 'execl'");

    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
      perror( NULL_FILE );
      exit( 1 );
    }
    if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
    {
      perror( NULL_FILE );
      exit( 1 );
    }
    }
*/

/*
    LAG alarm!                                                   -Thoric
*/
static void caught_alarm()
{
    char buf[MAX_STRING_LENGTH];
    bug( "ALARM CLOCK!" );
    strcpy( buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\n\r" );
    echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );

    if ( newdesc )
    {
        FD_CLR( newdesc, &in_set );
        FD_CLR( newdesc, &out_set );
        log_string( "clearing newdesc" );
    }

    game_loop( );
    close( control );
    log_string( "Normal termination of game." );
    exit( 0 );
}

bool check_bad_desc( int desc )
{
    if ( FD_ISSET( desc, &exc_set ) )
    {
        FD_CLR( desc, &in_set );
        FD_CLR( desc, &out_set );
        log_string( "Bad FD caught and disposed." );
        return TRUE;
    }

    return FALSE;
}

void accept_new( int ctrl )
{
    static struct timeval null_time;
    DESCRIPTOR_DATA* d;
    /* int maxdesc; Moved up for use with id.c as extern */
    /*
        Poll all active descriptors.
    */
    FD_ZERO( &in_set  );
    FD_ZERO( &out_set );
    FD_ZERO( &exc_set );
    FD_SET( ctrl, &in_set );
    maxdesc = ctrl;
    newdesc = 0;

    for ( d = first_descriptor; d; d = d->next )
    {
        maxdesc = UMAX( maxdesc, d->descriptor );
        FD_SET( d->descriptor, &in_set  );
        FD_SET( d->descriptor, &out_set );
        FD_SET( d->descriptor, &exc_set );

        if ( d->ifd != -1 && d->ipid != -1 )
        {
            maxdesc = UMAX( maxdesc, d->ifd );
            FD_SET( d->ifd, &in_set );
        }

        if ( d->auth_fd != -1 )
        {
            maxdesc = UMAX( maxdesc, d->auth_fd );
            FD_SET( d->auth_fd, &in_set );

            if ( IS_SET( d->auth_state, FLAG_WRAUTH ) )
                FD_SET( d->auth_fd, &out_set );
        }

        if ( d == last_descriptor )
            break;
    }

    auth_maxdesc( &maxdesc, &in_set, &out_set, &exc_set );

    if ( select( maxdesc + 1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
    {
        perror( "accept_new: select: poll" );
        exit( 1 );
    }

    if ( FD_ISSET( ctrl, &exc_set ) )
    {
        bug( "Exception raise on controlling descriptor %d", ctrl );
        FD_CLR( ctrl, &in_set );
        FD_CLR( ctrl, &out_set );
    }
    else if ( FD_ISSET( ctrl, &in_set ) )
    {
        newdesc = ctrl;
        new_descriptor( newdesc );
    }
}

void emergency_arm( )
{
    if ( !emergency_copy )
    {
        log_string( "Notice: emergency hotboot system is ready." );
        emergency_copy = TRUE;
    }

    return;
}

void game_loop( )
{
    struct timeval        last_time;
    char cmdline[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA* d;
    /*  time_t      last_check = 0;  */
    signal( SIGPIPE, SIG_IGN );
    signal( SIGALRM, caught_alarm );
    // emergency Copyover System - PREVENTS COREDUMPS.
    signal( SIGSEGV, SegVio );
    struct timeval last_systemd_watchdog;

    if ( systemd_watchdog_enabled )
    {
        log_string( "Systemd watchdog is enabled." );
        gettimeofday( &last_systemd_watchdog, NULL );
        sd_notify( 0, "WATCHDOG=1" );
    }
    else
    {
        log_string( "Systemd watchdog is not enabled." );
    }

    gettimeofday( &last_time, NULL );
    current_time = ( time_t ) last_time.tv_sec;

    /* Main loop */
    while ( !mud_down )
    {
        accept_new( control  );
        accept_new( control2 );
        handle_web();
        auth_check( &in_set, &out_set, &exc_set );

        /*
            Kick out descriptors with raised exceptions
            or have been idle, then check for input.
        */
        for ( d = first_descriptor; d; d = d_next )
        {
            if ( d == d->next )
            {
                bug( "descriptor_loop: loop found & fixed" );
                d->next = NULL;
            }

            d_next = d->next;
            d->idle++;  /* make it so a descriptor can idle out */

            if ( FD_ISSET( d->descriptor, &exc_set ) )
            {
                FD_CLR( d->descriptor, &in_set  );
                FD_CLR( d->descriptor, &out_set );

                if ( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                    save_char_obj( d->character );

                d->outtop   = 0;
                close_socket( d, TRUE );
                continue;
            }
            else
            {
                d->fcommand = FALSE;

                if ( FD_ISSET( d->descriptor, &in_set ) )
                {
                    d->idle = 0;

                    if ( d->character )
                        d->character->timer = 0;

                    if ( !read_from_descriptor( d ) )
                    {
                        FD_CLR( d->descriptor, &out_set );

                        if ( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
                            save_char_obj( d->character );

                        d->outtop   = 0;
                        close_socket( d, FALSE );
                        continue;
                    }
                }

                /*
                    if ( d->auth_fd != -1)
                    {
                    if ( FD_ISSET( d->auth_fd, &in_set ) )
                    {
                    read_auth( d );
                    }
                    else
                    if ( FD_ISSET( d->auth_fd, &out_set ) && IS_SET( d->auth_state, FLAG_WRAUTH) )
                    {
                    send_auth( d );
                    }
                    }
                */

                if ( ( d->connected == CON_PLAYING || d->character != NULL ) && d->ifd != -1 && FD_ISSET( d->ifd, &in_set ) )
                    process_dns( d );

                if ( d->character && d->character->wait > 0 )
                {
                    --d->character->wait;
                    continue;
                }

                read_from_buffer( d );

                if ( d->incomm[0] != '\0' )
                {
                    d->fcommand     = TRUE;
                    stop_idling( d->character );
                    strcpy( cmdline, d->incomm );
                    d->incomm[0] = '\0';

                    if ( d->character )
                        set_cur_char( d->character );

                    if ( d->pagepoint )
                        set_pager_input( d, cmdline );
                    else
                        switch ( d->connected )
                        {
                            default:
                                nanny( d, cmdline );
                                break;

                            case CON_PLAYING:
                                interpret( d->character, cmdline, FALSE );
                                break;

                            case CON_EDITING:
                                edit_buffer( d->character, cmdline );
                                break;
                        }
                }
            }

            if ( d == last_descriptor )
                break;
        }

        /*
            Autonomous game motion.
        */
        update_handler( );
        /*
            Check REQUESTS pipe
        */
        check_requests( );

        /*
            Output.
        */
        for ( d = first_descriptor; d; d = d_next )
        {
            d_next = d->next;

            if ( ( d->fcommand || d->outtop > 0 )
                    &&   FD_ISSET( d->descriptor, &out_set ) )
            {
                if ( d->pagepoint )
                {
                    if ( !pager_output( d ) )
                    {
                        if ( d->character
                                && ( d->connected == CON_PLAYING
                                     ||   d->connected == CON_EDITING ) )
                            save_char_obj( d->character );

                        d->outtop = 0;
                        close_socket( d, FALSE );
                    }
                }
                else if ( !flush_buffer( d, TRUE ) )
                {
                    if ( d->character
                            && ( d->connected == CON_PLAYING
                                 ||   d->connected == CON_EDITING ) )
                        save_char_obj( d->character );

                    d->outtop   = 0;
                    close_socket( d, FALSE );
                }
            }

            if ( d == last_descriptor )
                break;
        }

        /*
            Synchronize to a clock.
            Sleep( last_time + 1/PULSE_PER_SECOND - now ).
            Careful here of signed versus unsigned arithmetic.
        */
        {
            struct timeval now_time;
            long secDelta;
            long usecDelta;

            if ( systemd_watchdog_enabled &&
                    ( now_time.tv_usec - last_systemd_watchdog.tv_usec +
                      ( now_time.tv_sec - last_systemd_watchdog.tv_sec ) * 1000000 > ( int64_t )systemd_watchdog_interval / 2 ) )
            {
                gettimeofday( &last_systemd_watchdog, NULL );
                sd_notify( 0, "WATCHDOG=1" );
            }

            gettimeofday( &now_time, NULL );
            usecDelta   = ( ( int ) last_time.tv_usec ) - ( ( int ) now_time.tv_usec )
                          + 1000000 / PULSE_PER_SECOND;
            secDelta    = ( ( int ) last_time.tv_sec ) - ( ( int ) now_time.tv_sec );

            while ( usecDelta < 0 )
            {
                usecDelta += 1000000;
                secDelta  -= 1;
            }

            while ( usecDelta >= 1000000 )
            {
                usecDelta -= 1000000;
                secDelta  += 1;
            }

            if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
            {
                struct timeval stall_time;
                stall_time.tv_usec = usecDelta;
                stall_time.tv_sec  = secDelta;
#ifdef WIN32
                Sleep( ( stall_time.tv_sec * 1000L ) + ( stall_time.tv_usec / 1000L ) );
#else

                if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 && errno != EINTR )
                {
                    perror( "game_loop: select: stall" );
                    exit( 1 );
                }

#endif
            }
        }
        gettimeofday( &last_time, NULL );
        current_time = ( time_t ) last_time.tv_sec;
        /*  Check every 5 seconds...  (don't need it right now)
            if ( last_check+5 < current_time )
            {
            CHECK_LINKS(first_descriptor, last_descriptor, next, prev,
              DESCRIPTOR_DATA);
            last_check = current_time;
            }
        */
    }

    return;
}


void init_descriptor( DESCRIPTOR_DATA* dnew, int desc )
{
    dnew->next          = NULL;
    dnew->descriptor    = desc;
    dnew->connected     = CON_GET_NAME;
    dnew->outsize       = 2000;
    dnew->idle          = 0;
    dnew->scrlen        = 24;
    dnew->user          = STRALLOC( "unknown" );
    dnew->newstate      = 0;
    dnew->prevcolor     = 0x07;
    dnew->lines         = 0;
    dnew->auth_fd       = -1;
    dnew->auth_inc      = 0;
    dnew->auth_state    = 0;
    dnew->original      = NULL;
    dnew->character     = NULL;
    dnew->ifd           = -1;
    dnew->ipid          = -1;
    CREATE( dnew->outbuf, char, dnew->outsize );
}

void new_descriptor( int new_desc )
{
    char buf[MAX_STRING_LENGTH];
    char bugbuf[MAX_STRING_LENGTH];
    char* hostname;
    BAN_DATA* pban;
    DESCRIPTOR_DATA* dnew;
    struct sockaddr_in sock;
    struct hostent* from = NULL;
    int desc;
    int size;
    set_alarm( 20 );
    size = sizeof( sock );

    if ( check_bad_desc( new_desc ) )
    {
        set_alarm( 0 );
        return;
    }

    set_alarm( 20 );

    if ( ( desc = accept( new_desc, ( struct sockaddr* ) &sock, ( socklen_t* )&size ) ) < 0 )
    {
        perror( "New_descriptor: accept" );
        sprintf( bugbuf, "[*****] BUG: New_descriptor: accept" );
        log_string_plus( bugbuf, LOG_COMM, sysdata.log_level );
        set_alarm( 0 );
        return;
    }

    if ( check_bad_desc( new_desc ) )
    {
        set_alarm( 0 );
        return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif
    set_alarm( 20 );
#ifdef WIN32

    if ( ioctlsocket( desc, FIONBIO, &arg ) == -1 )
#else
    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
#endif
    {
        perror( "New_descriptor: fcntl: FNDELAY" );
        set_alarm( 0 );
        return;
    }

    if ( check_bad_desc( new_desc ) )
        return;

    CREATE( dnew, DESCRIPTOR_DATA, 1 );
    init_descriptor( dnew, desc );
    strcpy( log_buf, inet_ntoa( sock.sin_addr ) );
    dnew->host = STRALLOC( log_buf );

    if ( !sysdata.NO_NAME_RESOLVING )
    {
        strcpy( buf, in_dns_cache( log_buf ) );

        if ( buf[0] == '\0' )
            resolve_dns( dnew, sock.sin_addr.s_addr );
        else
        {
            STRFREE( dnew->host );
            dnew->host = STRALLOC( buf );
        }
    }

    hostname = STRALLOC( ( char* )( from ? from->h_name : "" ) );
    CREATE( dnew->outbuf, char, dnew->outsize );
    strcpy( buf, inet_ntoa( sock.sin_addr ) );

    /* Check for site banning */
    for ( pban = first_ban; pban; pban = pban->next )
    {
        if ( ( !str_prefix( pban->name, dnew->host ) || !str_prefix( pban->name, hostname ) )
                &&  pban->level >= LEVEL_SUPREME )
        {
            sprintf( buf, "Banning system rejected connection from %s", dnew->host );
            log_string_plus( buf, LOG_NORMAL, 103 );
            write_to_descriptor( desc, "\n\r(Y)our site has been banned from AVP: Legend.\n\r", 0 );
            free_desc( dnew );
            set_alarm( 0 );
            return;
        }
    }

    /*
        Init descriptor data.
    */
    if ( !last_descriptor && first_descriptor )
    {
        DESCRIPTOR_DATA* d;
        bug( "New_descriptor: last_desc is NULL, but first_desc is not! ...fixing" );

        for ( d = first_descriptor; d; d = d->next )
            if ( !d->next )
                last_descriptor = d;
    }

    LINK( dnew, first_descriptor, last_descriptor, next, prev );
    /*
        Send the greeting.
    */
    {
        extern char* help_greeting;

        if ( help_greeting[0] == '.' )
            send_to_buffer( help_greeting + 1, dnew );
        else
            send_to_buffer( help_greeting, dnew );

        send_to_buffer( "&z(&CE&z)nter your name: ", dnew );
    }
    set_auth( dnew );

    if ( ++num_descriptors > sysdata.maxplayers )
        sysdata.maxplayers = num_descriptors;

    if ( sysdata.maxplayers > sysdata.alltimemax )
    {
        if ( sysdata.time_of_max )
            DISPOSE( sysdata.time_of_max );

        sprintf( buf, "%24.24s", ctime( &current_time ) );
        sysdata.time_of_max = str_dup( buf );
        sysdata.alltimemax = sysdata.maxplayers;
        sprintf( log_buf, "Broke all-time maximum player record: %d", sysdata.alltimemax );
        log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
        to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
        save_sysdata( sysdata );
    }

    set_alarm( 0 );
    return;
}

void free_desc( DESCRIPTOR_DATA* d )
{
    kill_auth( d );
    close( d->descriptor );
    STRFREE( d->host );
    DISPOSE( d->outbuf );
    STRFREE( d->user );    /* identd */

    if ( d->pagebuf )
        DISPOSE( d->pagebuf );

    DISPOSE( d );
    --num_descriptors;
    return;
}

void close_socket( DESCRIPTOR_DATA* dclose, bool force )
{
    CHAR_DATA* ch;
    DESCRIPTOR_DATA* d;
    bool DoNotUnlink = FALSE;
    int i = 0;

    if ( dclose->ipid != -1 )
    {
        int status;
        kill( dclose->ipid, SIGKILL );
        waitpid( dclose->ipid, &status, 0 );
    }

    if ( dclose->ifd != -1 )
        close( dclose->ifd );

    /* flush outbuf */
    if ( !force && dclose->outtop > 0 )
        flush_buffer( dclose, FALSE );

    /* Say bye to whoever's snooping this descriptor */
    for ( i = 0; i <= 5; i++ )
    {
        if ( dclose->snoop_by[i] )
            write_to_buffer( dclose->snoop_by[i], "Your victim has left the game.\n\r", 0 );
    }

    /* stop snooping everyone else */
    for ( d = first_descriptor; d; d = d->next )
    {
        for ( i = 0; i <= 5; i++ )
        {
            if ( d->snoop_by[i] == dclose )
                d->snoop_by[i] = NULL;
        }
    }

    /* Check for switched people who go link-dead. -- Altrag */
    if ( dclose->original )
    {
        if ( ( ch = dclose->character ) != NULL )
            do_return( ch, "" );
        else
        {
            bug( "Close_socket: dclose->original without character %s",
                 ( dclose->original->name ? dclose->original->name : "unknown" ) );
            dclose->character = dclose->original;
            dclose->original = NULL;
        }
    }

    ch = dclose->character;

    /* sanity check :( */
    if ( !dclose->prev && dclose != first_descriptor )
    {
        DESCRIPTOR_DATA* dp, *dn;
        bug( "Close_socket: %s desc:%p != first_desc:%p and desc->prev = NULL!",
             ch ? ch->name : d->host, dclose, first_descriptor );
        dp = NULL;

        for ( d = first_descriptor; d; d = dn )
        {
            dn = d->next;

            if ( d == dclose )
            {
                bug( "Close_socket: %s desc:%p found, prev should be:%p, fixing.",
                     ch ? ch->name : d->host, dclose, dp );
                dclose->prev = dp;
                break;
            }

            dp = d;
        }

        if ( !dclose->prev )
        {
            bug( "Close_socket: %s desc:%p could not be found!.",
                 ch ? ch->name : dclose->host, dclose );
            DoNotUnlink = TRUE;
        }
    }

    if ( !dclose->next && dclose != last_descriptor )
    {
        DESCRIPTOR_DATA* dp, *dn;
        bug( "Close_socket: %s desc:%p != last_desc:%p and desc->next = NULL!",
             ch ? ch->name : d->host, dclose, last_descriptor );
        dn = NULL;

        for ( d = last_descriptor; d; d = dp )
        {
            dp = d->prev;

            if ( d == dclose )
            {
                bug( "Close_socket: %s desc:%p found, next should be:%p, fixing.",
                     ch ? ch->name : d->host, dclose, dn );
                dclose->next = dn;
                break;
            }

            dn = d;
        }

        if ( !dclose->next )
        {
            bug( "Close_socket: %s desc:%p could not be found!.",
                 ch ? ch->name : dclose->host, dclose );
            DoNotUnlink = TRUE;
        }
    }

    if ( dclose->character )
    {
        sprintf( log_buf, "Closing link to %s.", ch->name );
        log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->top_level ) );

        /*
            if ( ch->top_level < LEVEL_DEMI )
              to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
        */
        if ( dclose->connected == CON_PLAYING || dclose->connected == CON_EDITING )
        {
            act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
            ch->desc = NULL;
        }
        else if ( dclose->character->last_cmd != NULL )
        {
            dclose->connected = CON_PLAYING;
            act( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
            ch->desc = NULL;
        }
        else
        {
            /* clear descriptor pointer to get rid of bug message in log */
            dclose->character->desc = NULL;
            free_char( dclose->character );
        }
    }

    if ( !DoNotUnlink )
    {
        /* make sure loop doesn't get messed up */
        if ( d_next == dclose )
            d_next = d_next->next;

        UNLINK( dclose, first_descriptor, last_descriptor, next, prev );
    }

    if ( dclose->descriptor == maxdesc )
        --maxdesc;

    if ( dclose->auth_fd != -1 )
        close( dclose->auth_fd );

    free_desc( dclose );
    return;
}


bool read_from_descriptor( DESCRIPTOR_DATA* d )
{
    int iStart, iErr;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
        return TRUE;

    /* Check for overflow. */
    iStart = strlen( d->inbuf );

    if ( iStart >= sizeof( d->inbuf ) - 10 )
    {
        sprintf( log_buf, "%s input overflow!", d->host );
        log_string( log_buf );
        write_to_descriptor( d->descriptor, "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
        return FALSE;
    }

    for ( ; ; )
    {
        int nRead;
        nRead = read( d->descriptor, d->inbuf + iStart, sizeof( d->inbuf ) - 10 - iStart );
#ifdef WIN32
        iErr = WSAGetLastError ();
#else
        iErr = errno;
#endif

        if ( nRead > 0 )
        {
            iStart += nRead;

            if ( d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r' )
                break;
        }
        else if ( nRead == 0 )
        {
            // log_string_plus( "EOF encountered on read.", LOG_COMM, sysdata.log_level );
            return FALSE;
        }
        else if ( iErr == EWOULDBLOCK )
        {
            break;
        }
        else
        {
            perror( "Read_from_descriptor" );
            return FALSE;
        }
    }

    d->inbuf[iStart] = '\0';
    return TRUE;
}


/*
    Transfer one line from input buffer to input line.
*/
void read_from_buffer( DESCRIPTOR_DATA* d )
{
    int i, j, k;

    /*
        Hold horses if pending command already.
    */
    if ( d->incomm[0] != '\0' )
        return;

    /*
        Look for at least one new line.
    */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i < MAX_INBUF_SIZE;
            i++ )
    {
        if ( d->inbuf[i] == '\0' )
            return;
    }

    /*
        Canonical input processing.
    */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
        if ( k >= 254 )
        {
            write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );
            /* skip the rest of the line */
            /*
                for ( ; d->inbuf[i] != '\0' || i>= MAX_INBUF_SIZE ; i++ )
                {
                if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
                break;
                }
            */
            d->inbuf[i]   = '\n';
            d->inbuf[i + 1] = '\0';
            break;
        }

        if ( d->inbuf[i] == '\b' && k > 0 )
            --k;
        else if ( isascii( d->inbuf[i] ) && isprint( d->inbuf[i] ) )
            d->incomm[k++] = d->inbuf[i];
    }

    /*
        Finish off the line.
    */
    if ( k == 0 )
        d->incomm[k++] = ' ';

    d->incomm[k] = '\0';

    /*
        Deal with bozos with #repeat 1000 ...
    */
    if ( k > 1 || d->incomm[0] == '!' )
    {
        if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
        {
            d->repeat = 0;
        }
        else
        {
            if ( ++d->repeat >= 20 )
            {
                /*              sprintf( log_buf, "%s input spamming!", d->host );
                        log_string( log_buf );
                */
                write_to_descriptor( d->descriptor,
                                     "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
            }
        }
    }

    /*
        Do '!' substitution.
    */
    if ( d->incomm[0] == '!' )
        strcpy( d->incomm, d->inlast );
    else
        strcpy( d->inlast, d->incomm );

    /*
        Shift the input buffer.
    */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
        i++;

    for ( j = 0; ( d->inbuf[j] = d->inbuf[i + j] ) != '\0'; j++ )
        ;

    return;
}



/*
    Low level output function.
*/
bool flush_buffer( DESCRIPTOR_DATA* d, bool fPrompt )
{
    char buf[MAX_INPUT_LENGTH];
    extern bool mud_down;
    CHAR_DATA* ch;
    int i = 0;
    ch = d->original ? d->original : d->character;

    /*
        If buffer has more than 4K inside, spit out .5K at a time   -Thoric
    */
    if ( !mud_down && d->outtop > 4096 )
    {
        memcpy( buf, d->outbuf, 512 );
        memmove( d->outbuf, d->outbuf + 512, d->outtop - 512 );
        d->outtop -= 512;

        for ( i = 0; i <= 5; i++ )
        {
            if ( d->snoop_by[i] )
            {
                char snoopbuf[MAX_INPUT_LENGTH];
                buf[512] = '\0';

                if ( d->character && d->character->name )
                {
                    if ( d->original && d->original->name )
                        sprintf( snoopbuf, "%s (%s)", d->character->name, d->original->name );
                    else
                        sprintf( snoopbuf, "%s", d->character->name );

                    write_to_buffer( d->snoop_by[i], snoopbuf, 0 );
                }

                write_to_buffer( d->snoop_by[i], "% ", 2 );
                write_to_buffer( d->snoop_by[i], buf, 0 );
            }
        }

        if ( !write_to_descriptor( d->descriptor, buf, 512 ) )
        {
            d->outtop = 0;
            return FALSE;
        }

        return TRUE;
    }

    /*
        Bust a prompt.
    */
    if ( fPrompt && !mud_down && d->connected == CON_PLAYING )
    {
        ch = d->original ? d->original : d->character;

        if ( xIS_SET( ch->act, PLR_BLANK ) )
            write_to_buffer( d, "\n\r", 2 );

        if ( xIS_SET( ch->act, PLR_PROMPT ) )
            display_prompt( d );

        if ( xIS_SET( ch->act, PLR_TELNET_GA ) )
            write_to_buffer( d, go_ahead_str, 0 );
    }

    /*
        Short-circuit if nothing to write.
    */
    if ( d->outtop == 0 )
        return TRUE;

    /*
        Snoop-o-rama.
    */
    for ( i = 0; i <= 5; i++ )
    {
        if ( d->snoop_by[i] )
        {
            /* without check, 'force mortal quit' while snooped caused crash, -h */
            if ( d->character && d->character->name )
            {
                /* Show original snooped names. -- Altrag */
                if ( d->original && d->original->name )
                    sprintf( buf, "%s (%s)", d->character->name, d->original->name );
                else
                    sprintf( buf, "%s", d->character->name );

                write_to_buffer( d->snoop_by[i], buf, 0 );
            }

            write_to_buffer( d->snoop_by[i], "% ", 2 );
            write_to_buffer( d->snoop_by[i], d->outbuf, d->outtop );
        }
    }

    /*
        OS-dependent output.
    */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) )
    {
        if ( d )
            d->outtop = 0;

        return FALSE;
    }
    else
    {
        d->outtop = 0;
        return TRUE;
    }
}

/*
    Append onto an output buffer.
*/
void write_to_buffer( DESCRIPTOR_DATA* d, const char* buf, int length )
{
    /*
        char *colstr;
        const char *prevstr = txt;
        char colbuf[20];
    */
    char* txt;
    // char txt[MAX_STRING_LENGTH*5];

    if ( !d )
    {
        bug( "Write_to_buffer: NULL descriptor" );
        return;
    }

    /*
        Normally a bug... but can happen if loadup is used.
    */
    if ( !d->outbuf )
        return;

    if ( strlen( buf ) <= 0 )
        return;

    txt = malloc( strlen( buf ) + 1 );
    strcpy( txt, buf );

    if ( d != NULL )
    {
        if ( block_profane( d->character ) )
        {
            profanity_filter( txt, txt );
            // length = strlen(txt);
        }
    }

    /*
        Find length in case caller didn't.
    */
    if ( length <= 0 )
        length = strlen( txt );

    /*  Uncomment if debugging or something
        if ( length != strlen(txt) )
        {
        bug( "Write_to_buffer: length(%d) != strlen(txt)!", length );
        length = strlen(txt);
        }
    */

    /*
        Initial \n\r if needed.
    */
    if ( d->outtop == 0 && !d->fcommand )
    {
        d->outbuf[0]    = '\n';
        d->outbuf[1]    = '\r';
        d->outtop       = 2;
    }

    /*
        Expand the buffer as needed.
    */
    while ( d->outtop + length >= d->outsize )
    {
        if ( d->outsize > 32000 )
        {
            /* empty buffer */
            d->outtop = 0;
            close_socket( d, TRUE );
            bug( "Buffer overflow. Closing (%s).", d->character ? d->character->name : "???" );
            free( txt );
            return;
        }

        d->outsize *= 2;
        RECREATE( d->outbuf, char, d->outsize );
    }

    /*
        Copy.
    */
    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    d->outbuf[d->outtop] = '\0';
    free( txt );
    return;
}

/*
    Write to one char. Commented out in favour of colour

    void send_to_char( const char *txt, CHAR_DATA *ch )
    {
    if ( !ch )
    {
      bug( "Send_to_char: NULL *ch" );
      return;
    }
    if ( !ch->desc )
    {
      bug( "Send_to_char: NULL *ch->desc" );
      return;
    }

    if ( txt )
    write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
    }
*/

/*
    Same as above, but converts &color codes to ANSI sequences..
*/
void send_to_char_color( const char* txt, CHAR_DATA* ch )
{
    DESCRIPTOR_DATA* d;
    char* colstr;
    const char* prevstr = txt;
    char colbuf[20];
    int ln;

    if ( !ch )
    {
        bug( "Send_to_char_color: NULL *ch" );
        return;
    }

    if ( !txt || IS_NPC( ch ) || !ch->desc )
        return;

    // if ( ch->desc->connected != CON_PLAYING && ch->desc->connected != CON_EDITING ) return;
    d = ch->desc;

    /* Clear out old color stuff */
    while ( ( colstr = strpbrk( prevstr, "&^" ) ) != NULL )
    {
        if ( colstr > prevstr )
            write_to_buffer( d, prevstr, ( colstr - prevstr ) );

        ln = make_color_sequence( colstr, colbuf, d );

        if ( ln < 0 )
        {
            prevstr = colstr + 1;
            break;
        }
        else if ( ln > 0 )
            write_to_buffer( d, colbuf, ln );

        prevstr = colstr + 2;
    }

    if ( *prevstr )
        write_to_buffer( d, prevstr, 0 );

    return;
}

/*
    Lowest level output function.
    Write a block of text to the file descriptor.
    If this gives errors on very long blocks (like 'ofind all'),
     try lowering the max block size.
*/
bool write_to_descriptor( int desc, char* txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;

    if ( length <= 0 )
        length = strlen( txt );

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
        nBlock = UMIN( length - iStart, 4096 );

        if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
        {
            perror( "Write_to_descriptor" );
            return FALSE;
        }
    }

    return TRUE;
}



void show_title( DESCRIPTOR_DATA* d )
{
    CHAR_DATA* ch;
    /* Dont know what this is, dont care... Ghost */
    d->connected = CON_PRESS_ENTER;
    return;
    ch = d->character;

    if ( !xIS_SET( ch->pcdata->flags, PCFLAG_NOINTRO ) )
    {
        if ( xIS_SET( ch->act, PLR_ANSI ) )
            send_ansi_title( ch );
        else
            send_ascii_title( ch );
    }
    else
    {
        send_to_buffer( "&zPress enter...\n\r", d );
    }

    d->connected = CON_PRESS_ENTER;
}

/*
    Deal with sockets that haven't logged in yet.
*/
void nanny( DESCRIPTOR_DATA* d, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    CHAR_DATA* ch;
    char* pwdnew;
    char* p;
    int iRace;
    BAN_DATA* pban;
    int c = 0, cost;
    bool fOld;
    short chk;
    int sn;

    while ( isspace( *argument ) )
        argument++;

    ch = d->character;

    switch ( d->connected )
    {
        default:
            bug( "Nanny: bad d->connected %d.", d->connected );
            close_socket( d, TRUE );
            return;

        case CON_GET_NAME:
            if ( argument[0] == '\0' )
            {
                close_socket( d, FALSE );
                return;
            }

            strcpy( argument, capitalize( argument ) );
            argument[0] = UPPER( argument[0] );

            if ( !check_parse_name( argument ) )
            {
                send_to_buffer( "&z(&CI&z)llegal name, try another.\n\rName: ", d );
                return;
            }

            if ( !str_cmp( argument, "New" ) )
            {
                if ( d->newstate == 0 )
                {
                    /* New player */
                    /* Don't allow new players if DENY_NEW_PLAYERS is true */
                    if ( sysdata.DENY_NEW_PLAYERS == TRUE )
                    {
                        sprintf( buf, "&zThe mud is currently preparing for a reboot.\n\r" );
                        send_to_buffer( buf, d );
                        sprintf( buf, "&zNew players are not accepted during this time.\n\r" );
                        send_to_buffer( buf, d );
                        sprintf( buf, "&zPlease try again in a few minutes.\n\r" );
                        send_to_buffer( buf, d );
                        close_socket( d, FALSE );
                    }

                    sprintf( buf, "\n\r&z(&CP&z)lease choose a name for your character: " );
                    send_to_buffer( buf, d );
                    d->newstate++;
                    d->connected = CON_GET_NAME;
                    return;
                }
                else
                {
                    send_to_buffer( "&z(&CI&z)llegal name, try another.\n\r&z(&CN&z)ame: ", d );
                    return;
                }
            }

            if ( check_playing( d, argument, FALSE ) == BERR )
            {
                send_to_buffer( "&z(&CN&z)ame: ", d );
                return;
            }

            fOld = load_char_obj( d, argument, TRUE );

            if ( !d->character )
            {
                sprintf( log_buf, "Bad player file %s@%s.", argument, d->host );
                log_string( log_buf );
                send_to_buffer( "&z(&CY&z)our playerfile is corrupt...&z(&CP&z)lease notify &C...&z.\n\r", d );
                close_socket( d, FALSE );
                return;
            }

            ch = d->character;

            for ( pban = first_ban; pban; pban = pban->next )
            {
                if ( ( !str_prefix( pban->name, d->host ) || !str_suffix( pban->name, d->host ) ) && pban->level >= ch->top_level && pban->level >= 0 )
                {
                    sprintf( buf, "User %s rejected from %s by Banning module.", argument, d->host );
                    send_to_buffer( "\n\r&z(&CY&z)our site has been banned from this Mud.\n\r", d );
                    close_socket( d, FALSE );
                    log_string_plus( buf, LOG_NORMAL, 103 );
                    return;
                }
                else if ( ( !str_prefix( pban->name, d->host ) || !str_suffix( pban->name, d->host ) ) && ch->top_level >= 101 && pban->level == -1 )
                {
                    sprintf( buf, "User %s rejected from %s by Immortal Banning module.", argument, d->host );
                    send_to_buffer( "\n\r&z(&CY&z)our site has been banned from accessing Immortals.\n\r", d );
                    close_socket( d, FALSE );
                    log_string_plus( buf, LOG_NORMAL, 103 );
                    return;
                }

                //         else if ( ( !str_prefix( pban->name, d->host ) || !str_suffix( pban->name, d->host ) ) && is_school_day( ) && pban->level == -2 )
                //   {
                //             sprintf( buf, "User %s rejected from %s by School restrictions.", argument, d->host );
                //             send_to_buffer( "\n\r&z(&CY&z)our site has been banned during school days.\n\r", d );
                // close_socket( d, FALSE );
                // log_string_plus( buf, LOG_NORMAL, 103 );
                // return;
                //   }
            }

            if ( sysdata.TMCBLOCK && ( !str_cmp( d->host, "mudconnect.mudconnect.com" ) || !str_cmp( d->host, "mudconnect.mudconnector.com" ) || !str_cmp( d->host, "mudconnector.com" ) || !str_cmp( d->host, "mudconnect.com" ) ) )
            {
                sprintf( buf, "User %s rejected from %s by security module.", argument, d->host );
                send_to_buffer( "&z(&CD&z)ue to security restrictions, we cannot allow usage of TMC's java client.\n\r", d );
                send_to_buffer( "&z(&CO&z)nce TMC removes the IP mask, You will be able to use their client.\n\r", d );
                close_socket( d, FALSE );
                log_string_plus( buf, LOG_NORMAL, 103 );
                return;
            }

            if ( xIS_SET( ch->act, PLR_DENY ) )
            {
                sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
                log_string_plus( log_buf, LOG_COMM, sysdata.log_level );

                if ( d->newstate != 0 )
                {
                    send_to_buffer( "&z(&CT&z)hat name is already taken.\n\r&z(&CP&z)lease choose another: ", d );
                    d->connected = CON_GET_NAME;
                    d->character->desc = NULL;
                    free_char( d->character ); /* Big Memory Leak before --Shaddai */
                    d->character = NULL;
                    return;
                }

                send_to_buffer( "&z(&CY&z)ou have been denied access.\n\r", d );
                close_socket( d, FALSE );
                return;
            }

            chk = check_reconnect( d, argument, FALSE );

            if ( chk == BERR )
                return;

            if ( chk )
            {
                fOld = TRUE;
            }
            else
            {
                if ( wizlock && !IS_IMMORTAL( ch ) )
                {
                    send_to_buffer( "&z(&CT&z)he game is wizlocked.  Only immortals can connect now.\n\r", d );
                    send_to_buffer( "&z(&CP&z)lease try back later.\n\r", d );
                    close_socket( d, FALSE );
                    return;
                }
            }

            if ( fOld )
            {
                if ( d->newstate != 0 )
                {
                    send_to_buffer( "&z(&CT&z)hat name is already taken.\n\r&z(&CP&z)lease choose another: ", d );
                    d->connected = CON_GET_NAME;
                    d->character->desc = NULL;
                    free_char( d->character ); /* Big Memory Leak before --Shaddai */
                    d->character = NULL;
                    return;
                }

                /* Old player */
                send_to_buffer( "\n\r&z(&CP&z)assword: ", d );
                send_to_buffer( echo_off_str, d );
                d->connected = CON_GET_OLD_PASSWORD;
                return;
            }
            else
            {
                send_to_buffer( "\n\r&cI don't recognize your name, you must be new here.&z\n\r\n\r", d );
                sprintf( buf, "&z(&CD&z)id I get that right, %s&z (&CY&z/&CN&z)? ", argument );
                send_to_buffer( buf, d );
                d->connected = CON_CONFIRM_NEW_NAME;
                return;
            }

            break;

        case CON_GET_OLD_PASSWORD:
            send_to_buffer( "\n\r", d );

            if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
            {
                send_to_buffer( "&z(&CW&z)rong password.\n\r", d );
                sprintf( buf, "WARNING: Invalid password for %s - Password: [%s]", ch->name, argument );
                log_string_plus( buf, LOG_COMM, 200 );
                sprintf( buf, "Address: [%s@%s]", ch->desc->user, ch->desc->host );
                log_string_plus( buf, LOG_COMM, 200 );
                /* clear descriptor pointer to get rid of bug message in log */
                d->character->desc = NULL;
                close_socket( d, FALSE );
                return;
            }

            send_to_buffer( echo_on_str, d );

            if ( check_playing( d, ch->name, TRUE ) )
                return;

            if ( check_multi( d, ch->name  ) )
            {
                close_socket( d, FALSE );
                return;
            }

            chk = check_reconnect( d, ch->name, TRUE );

            if ( chk == BERR )
            {
                if ( d->character && d->character->desc )
                    d->character->desc = NULL;

                close_socket( d, FALSE );
                return;
            }

            if ( chk == TRUE )
                return;

            strncpy( buf, ch->name, MSL );
            d->character->desc = NULL;
            free_char( d->character );
            fOld = load_char_obj( d, buf, FALSE );
            ch = d->character;
            snprintf( log_buf, MSL, "%s@%s(%s) has connected.", ch->name, d->host, d->user );

            if ( ch->top_level < LEVEL_DEMI )
            {
                /*to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->top_level );*/
                // log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
                log_string_plus( log_buf, LOG_COMM, ch->top_level );
            }
            else
                log_string_plus( log_buf, LOG_COMM, ch->top_level );

            show_title( d );
            HTML_Who( );

            if ( IS_IMMORTAL( ch ) )
            {
                d->connected = CON_IMM_INVIS;
                send_to_buffer( "&z(&GW&z)ould you like to enter the game invisible, (&CY&z/&CN&z)?\n\r", d );
            }
            else
            {
                send_to_buffer( "&z(&CP&z)ress [ENTER]\n\r", d );
            }

            break;

        case CON_IMM_INVIS:
            switch ( *argument )
            {
                case 'y':
                case 'Y':
                    ch->pcdata->wizinvis = get_trust( ch );
                    xSET_BIT( ch->act, PLR_WIZINVIS );

                    if ( ch->pcdata->area && !IS_SET( ch->pcdata->area->status, AREA_LOADED ) )
                    {
                        send_to_buffer( "&z(&CW&z)ould you like your area loaded, (&CY&z/&CN&z)?\n\r", d );
                        d->connected = CON_IMM_LOADA;
                    }
                    else
                    {
                        send_to_buffer( "&z(&CP&z)ress [ENTER]\n\r", d );
                        d->connected = CON_PRESS_ENTER;
                    }

                    break;

                case 'n':
                case 'N':
                    xREMOVE_BIT( ch->act, PLR_WIZINVIS );

                    if ( ch->pcdata->area )
                    {
                        send_to_buffer( "&z(&CW&z)ould you like your area loaded?\n\r", d );
                        d->connected = CON_IMM_LOADA;
                    }
                    else
                    {
                        send_to_buffer( "&z(&CP&z)ress [ENTER]\n\r", d );
                        d->connected = CON_PRESS_ENTER;
                    }

                    break;

                default:
                    send_to_buffer( "&z(&CP&z)lease type &CYes &zor &CNo&z. ", d );
                    break;
            }

            break;

        case CON_IMM_LOADA:
            switch ( *argument )
            {
                case 'y':
                case 'Y':
                    do_loadarea ( ch, "" );
                    send_to_buffer( "&z(&CP&z)ress [ENTER]\n\r", d );
                    d->connected = CON_PRESS_ENTER;
                    break;

                case 'n':
                case 'N':
                    send_to_buffer( "&z(&CP&z)ress [ENTER]\n\r", d );
                    d->connected = CON_PRESS_ENTER;
                    break;

                default:
                    send_to_buffer( "&z(&CP&z)lease type &CYes &zor &CNo&z. ", d );
                    break;
            }

            break;

        case CON_CONFIRM_NEW_NAME:
            switch ( *argument )
            {
                case 'y':
                case 'Y':
                    sprintf( buf, "\n\r&z(&CM&z)ake sure to use a password that won't be easily guessed by someone else."
                             "\n\r&z(&CP&z)ick a good password for %s&z: %s", ch->name, echo_off_str );
                    send_to_buffer( buf, d );
                    d->connected = CON_GET_NEW_PASSWORD;
                    break;

                case 'n':
                case 'N':
                    send_to_buffer( "\n\r&zOk, what IS it, then? ", d );
                    /* clear descriptor pointer to get rid of bug message in log */
                    d->character->desc = NULL;
                    free_char( d->character );
                    d->character = NULL;
                    d->connected = CON_GET_NAME;
                    break;

                default:
                    send_to_buffer( "\n\r&z(&CP&z)lease type &CYes &zor &CNo&z. ", d );
                    break;
            }

            break;

        case CON_GET_NEW_PASSWORD:
            send_to_buffer( "\n\r", d );

            if ( strlen( argument ) < 5 )
            {
                send_to_buffer( "&z(&CP&z)assword must be at least five characters long.\n\r&z(&CP&z)assword: ", d );
                return;
            }

            pwdnew = crypt( argument, ch->name );

            for ( p = pwdnew; *p != '\0'; p++ )
            {
                if ( *p == '~' )
                {
                    send_to_buffer( "&z(&CN&z)ew password not acceptable, try again.\n\r&z(&CP&z)assword: ", d );
                    return;
                }
            }

            DISPOSE( ch->pcdata->pwd );
            ch->pcdata->pwd = str_dup( pwdnew );
            send_to_buffer( "&z(&CP&z)lease retype the password to confirm: ", d );
            d->connected = CON_CONFIRM_NEW_PASSWORD;
            break;

        case CON_CONFIRM_NEW_PASSWORD:

            /* send_to_buffer( "\n\r", d ); */
            if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
            {
                send_to_buffer( "&z(&CP&z)asswords don't match.\n\r&z(&CR&z)etype password: ", d );
                d->connected = CON_GET_NEW_PASSWORD;
                return;
            }

            send_to_buffer( echo_on_str, d );
            {
                HELP_DATA* pHelp = NULL;
                pHelp = get_help( NULL, "disclaimer" );

                if ( pHelp != NULL && pHelp->text != NULL && pHelp->text[0] != '\0' )
                {
                    send_to_buffer( "\n\r\n\r&WAliens vs. Predator: Disclaimer &z[&RPlease read carefully&z]\n\r&z", d );

                    if ( pHelp->text[0] == '.' )
                        send_to_buffer( pHelp->text + 1, d );

                    if ( pHelp->text[0] != '.' )
                        send_to_buffer( pHelp->text, d );

                    send_to_buffer( "\n\r&z(&CD&z)o you accept the terms stated before? Type &RI AGREE&z to continue:\n\r", d );
                }
                else
                {
                    send_to_buffer( "\n\r&R[ALERT]: Unable to locate disclaimer text. Continuing. (type \"i agree\" to continue.)\n\r", d );
                }

                d->connected = CON_DISCLAIMER;
            }
            break;

        case CON_GET_FULL_NAME:
            argument = one_argument_sc( argument, arg );
            argument = one_argument_sc( argument, arg2 );

            if ( arg[0] == '\0' || arg2[0] == '\0' || strlen( arg ) < 3 || strlen( arg2 ) < 3 )
            {
                send_to_buffer( "&z(&CB&z)oth names must be atleast 3 letters long.\n\r&zName: ", d );
                return;
            }

            for ( p = arg; *p != '\0'; p++ )
            {
                if ( *p == '~' )
                {
                    send_to_buffer( "&z(&CY&z)ou cannot include ~ in your name. Anywhere. Period.\n\r&zName: ", d );
                    return;
                }
            }

            for ( p = arg2; *p != '\0'; p++ )
            {
                if ( *p == '~' )
                {
                    send_to_buffer( "&z(&CY&z)ou cannot include ~ in your name. Anywhere. Period.\n\r&zName: ", d );
                    return;
                }
            }

            if ( ch->pcdata->fname )
                DISPOSE( ch->pcdata->fname );

            if ( ch->pcdata->lname )
                DISPOSE( ch->pcdata->lname );

            ch->pcdata->fname = str_dup( arg );
            ch->pcdata->lname = str_dup( arg2 );
            sprintf( buf, "\n\r&zYou have choosen the name &B%s '%s' %s&z. Correct? (&CY&z/&CN&z) ", ch->pcdata->fname, ch->name, ch->pcdata->lname );
            send_to_buffer( buf, d );
            d->connected = CON_GET_FULL_NAME_OK;
            break;

        case CON_GET_FULL_NAME_OK:
            switch ( *argument )
            {
                case 'y':
                case 'Y':
                    send_to_buffer( "\n\r\n\r&z(&CU&z)sing &YANSI&z Color by default. You may change this by using the CONFIG Command.\n\r", d );
                    xSET_BIT( ch->act, PLR_ANSI );
                    send_to_buffer( "\n\r&z(&CD&z)o you want to start with the language censor engaged? (&CY&z/&CN&z)?\n\r", d );
                    d->connected = CON_GET_CENSOR;
                    break;

                case 'n':
                case 'N':
                    send_to_buffer( "\n\r&zName: ", d );
                    d->connected = CON_GET_FULL_NAME;
                    break;

                default:
                    send_to_buffer( "\n\r&z(&CP&z)lease type &CYes &zor &CNo&z. ", d );
                    break;
            }

            break;

        case CON_DISCLAIMER:
            if ( strcmp( strlower( argument ), "i agree" ) )
            {
                send_to_buffer( "\n\r&z(&CY&z)ou must accept the terms set forth in the disclaimer to play here. Goodbye.\n\r", d );
                close_socket( d, FALSE );
            }

            send_to_buffer( "\n\r&z(&CY&z)ou may choose from the following races, or type &Chelp [race]&z to learn more:\n\r&z", d );
            buf[0] = '\0';

            for ( iRace = 0; iRace < MAX_RACE; iRace++ )
            {
                if ( race_table[iRace].real_name && race_table[iRace].real_name[0] != '\0' )
                {
                    /*
                        if ( iRace > 0 )
                        {
                        if ( strlen(buf)+strlen(race_table[iRace].real_name) > 80 )
                        {
                         strcat( buf, "\n\r" );
                         send_to_buffer( buf, d );
                         buf[0] = '\0';
                        }
                        else
                         strcat( buf, " " );
                        }
                    */
                    snprintf( buf + strlen(buf), MSL - strlen(buf), "&z[ &B%d - %-10s&z ] &w:: &W%s\n\r", iRace + 1, race_table[iRace].real_name, race_table[iRace].desc );
                }
            }

            strcat( buf, "\n\r&z(&CC&z)hoose your race: " );
            send_to_buffer( buf, d );
            d->connected = CON_GET_NEW_RACE;
            break;

        case CON_GET_NEW_SEX:
            switch ( argument[0] )
            {
                case 'm':
                case 'M':
                    ch->sex = SEX_MALE;
                    break;

                case 'f':
                case 'F':
                    ch->sex = SEX_FEMALE;
                    break;

                case 'n':
                case 'N':
                    ch->sex = SEX_NEUTRAL;
                    break;

                default:
                    send_to_buffer(  "&zThat's not a sex.\n\r&zWhat is your sex? ", d );
                    return;
            }

            if ( ch->sex == SEX_MALE )
                send_to_buffer( "\n\r&z(&CY&z)ou have choosen: [&CMale&z].\n\r", d );
            else if ( ch->sex == SEX_FEMALE )
                send_to_buffer( "\n\r&z(&CY&z)ou have choosen: [&CFemale&z].\n\r", d );
            else if ( ch->sex == SEX_NEUTRAL )
                send_to_buffer( "\n\r&z(&CY&z)ou have choosen: [&CNeutral&z].\n\r", d );

            send_to_buffer( "\n\r&z(&CP&z)lease choose a first and last name, seperated by a space.\n\r&z", d );
            send_to_buffer( "&z[&WExample: John Doe&z]\n\r&z", d );
            send_to_buffer( "&zName: \n\r&z", d );
            d->connected = CON_GET_FULL_NAME;
            break;

        case CON_GET_NEW_RACE:
            argument = one_argument( argument, arg );

            if ( !str_cmp( arg, "help" ) )
            {
                send_to_buffer( "\n\r&c", d );
                do_help( ch, argument );
                send_to_buffer( "&z(&CP&z)lease choose a race: ", d );
                return;
            }

            ch->race = -1;

            for ( iRace = 0; iRace < MAX_RACE; iRace++ )
            {
                if ( atoi( arg ) == ( iRace + 1 ) )
                {
                    ch->race = iRace;
                    break;
                }
            }

            if ( ch->race < -1 )
            {
                for ( iRace = 0; iRace < MAX_RACE; iRace++ )
                {
                    if ( toupper( arg[0] ) == toupper( race_table[iRace].race_name[0] )
                            &&   !str_prefix( arg, race_table[iRace].race_name ) )
                    {
                        ch->race = iRace;
                        break;
                    }
                }
            }

            if ( iRace == MAX_RACE ||  !race_table[iRace].race_name || race_table[iRace].race_name[0] == '\0' )
            {
                send_to_buffer( "\n\r&z(&CT&z)hat's not a race. &z(&CC&z)hoose a race: ", d );
                return;
            }

            sprintf( buf, "\n\r&z(&CY&z)ou have choosen: [&C%s &z-&C %s&z].\n\r", race_table[iRace].real_name, race_table[iRace].race_name );
            send_to_buffer( buf, d );
            {
                ch->perm_str = race_table[ch->race].str_plus;
                ch->perm_sta = race_table[ch->race].sta_plus;
                ch->perm_rec = race_table[ch->race].rec_plus;
                ch->perm_int = race_table[ch->race].int_plus;
                ch->perm_bra = race_table[ch->race].bra_plus;
                ch->perm_per = race_table[ch->race].per_plus;
            }

            if ( ch->race == RACE_MARINE )
            {
                send_to_buffer( "\n\r&z(&CW&z)hat is your sex (&CM&z/&CF&z/&CN&z)? ", d );
                d->connected = CON_GET_NEW_SEX;
                break;
            }
            else
            {
                if ( ch->race == RACE_ALIEN )
                    ch->sex = SEX_NEUTRAL;

                if ( ch->race == RACE_PREDATOR )
                    ch->sex = SEX_MALE;

                send_to_buffer( "\n\r&z(&CU&z)sing &YANSI&z Color by default. You may change this by using the CONFIG Command.\n\r", d );
                xSET_BIT( ch->act, PLR_ANSI );
                send_to_buffer( "\n\r&z(&CD&z)o you want to start with the language censor engaged? (&CY&z/&CN&z)?\n\r", d );
                d->connected = CON_GET_CENSOR;
                break;
            }

        case CON_GET_WANT_RIPANSI:
            switch ( argument[0] )
            {
                case 'a':
                case 'A':
                    xSET_BIT( ch->act, PLR_ANSI );
                    break;

                case 'n':
                case 'N':
                    break;

                default:
                    send_to_buffer( "&z(&CI&z)nvalid selection. [&CA&z]NSI or [&CN&z]ONE? ", d );
                    return;
            }

            send_to_buffer( "&z(&CD&z)oes your mud client support [&CM&z]UD [&CS&z]OUND [&CP&z]ROTOCOL?\n\r", d );
            d->connected = CON_GET_MSP;
            break;

        case CON_GET_CENSOR:
            switch ( argument[0] )
            {
                case 'y':
                case 'Y':
                    xSET_BIT( ch->act, PLR_CENSOR );
                    break;

                case 'n':
                case 'N':
                    break;

                default:
                    send_to_buffer( "&z(&CI&z)nvalid selection. Choose [&CY&z]es or [&CN&z]o? ", d );
                    return;
            }

            if ( xIS_SET( ch->act, PLR_CENSOR ) )
                send_to_buffer( "&z(&CY&z)ou have choosen: [&CYES&z]\n\r", d );
            else
                send_to_buffer( "&z(&CY&z)ou have choosen: [&CNO&z]\n\r", d );

            send_to_buffer( "\n\r&z(&CD&z)oes your mud client support [&CM&z]UD [&CS&z]OUND [&CP&z]ROTOCOL, (&CY&z/&CN&z)?\n\r", d );
            d->connected = CON_GET_MSP;
            break;

        case CON_GET_MSP:
            switch ( argument[0] )
            {
                case 'y':
                case 'Y':
                    xSET_BIT( ch->act, PLR_SOUND );
                    break;

                case 'n':
                case 'N':
                    break;

                default:
                    send_to_buffer( "\n\r&z(&CI&z)nvalid selection. [&CY&z]es or [&CN&z]o? ", d );
                    return;
            }

            if ( xIS_SET( ch->act, PLR_SOUND ) )
                send_to_buffer( "&z(&CY&z)ou have choosen: [&CYES&z]\n\r", d );
            else
                send_to_buffer( "&z(&CY&z)ou have choosen: [&CNO&z]\n\r", d );

            send_to_buffer( "\n\r", d );
            assign_gname( ch );
            sprintf( log_buf, "%s@%s new %s.", ch->name, d->host, race_table[ch->race].race_name );
            log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
            to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
            ch->top_level = 0;
            ch->position = POS_STANDING;
            write_menu_to_desc( d );
            d->connected = CON_MENU_BASE;
            break;

        case CON_MENU_BASE:
            if ( !is_number( argument ) )
            {
                c = -1;
            }
            else
            {
                c = atoi( argument );
            }

            if ( c < 0 || c > 8 )
            {
                send_to_buffer( "\n\r&RInvalid choice. Choose again. ", d );
                return;
            }

            if ( c == 0 && d->character->last_cmd == NULL )
            {
                send_to_buffer( "\n\r\n\r&YNow exiting AVP: Legend... Have a nice day.\n\r\n\r", d );
                close_socket( d, FALSE );
            }
            else if ( c == 1 )
            {
                if ( d->character->last_cmd == NULL )
                {
                    send_to_buffer( "\n\r\n\r&GEntering -{ Aliens vs. Predator: Legend }- ...\n\r", d );

                    if ( curr_arena == NULL )
                    {
                        send_to_buffer( "&zCurrent Arena:  &CNone loaded. Please wait.\n\r", d );
                        send_to_buffer( "&zTime Remaining: &C1 minute.\n\r\n\r", d );
                    }
                    else
                    {
                        sprintf( buf, "&zCurrent Arena:  &C%s.\n\r", curr_arena->name );
                        send_to_buffer( buf, d );
                        sprintf( buf, "&zTime Remaining: &C%d minute(s).\n\r\n\r", curr_arena->ctimer );
                        send_to_buffer( buf, d );
                    }

                    d->connected = CON_ENTER_GAME;
                    nanny( d, "" ); // Force the final step
                }
                else
                {
                    send_to_buffer( "\n\r", d );
                    d->connected = CON_PLAYING;
                    do_look( d->character, "auto" );
                    return;
                }
            }
            else if ( c == 2 )
            {
                if ( ch->top_level <= 0 )
                {
                    send_to_buffer( "\n\r\n\r&RYou cannot view your score from here yet, Sorry.\n\r", d );
                    send_to_buffer( "&z[&WYou must enter the game at least once - &CPRESS ENTER&z]\n\r\n\r", d );
                    d->connected = CON_MENU_ENTER;
                    return;
                }

                send_to_buffer( "\n\r", d );
                do_score( ch, "" );
                send_to_buffer( "\n\r&R[Press enter to continue]\n\r\n\r", d );
                d->connected = CON_MENU_ENTER;
                return;
            }
            else if ( c == 3 )
            {
                char buf[MAX_STRING_LENGTH];

                if ( ch->top_level <= 0 )
                {
                    send_to_buffer( "\n\r\n\rYou must enter the game once before you may level.\n\r\n\r", d );
                    return;
                }

                if ( ch->top_level >= 20 )
                {
                    send_to_buffer( "\n\r\n\r&RYou cannot purchase any more levels, your at max.\n\r", d );
                    send_to_buffer( "&z[&WPress ENTER to return to the Menu&z]\n\r\n\r", d );
                    d->connected = CON_MENU_ENTER;
                    break;
                }

                sprintf( buf, "\n\r\n\r&zProgress: [%s&z] (&WNext Level: %d&z)\n\r\n\r", drawlevel( ch ), ch->top_level + 1 );
                send_to_buffer( buf, d );

                if ( ch->currexp < exp_level( ch->top_level + 1 ) )
                {
                    send_to_buffer( "&RYou cannot afford to purchase another level yet.\n\r", d );
                    send_to_buffer( "&z[&WPress ENTER to return to the Menu&z]\n\r\n\r", d );
                    d->connected = CON_MENU_ENTER;
                    break;
                }
                else
                {
                    send_to_buffer( "&zYou can afford a new level. Proceed? [&CY&z/&CN&z] ", d );
                    d->connected = CON_MENU_LEVEL;
                    break;
                }

                return;
            }
            else if ( c == 4 )
            {
                if ( ch->top_level < 0 )
                {
                    send_to_buffer( "\n\r\n\r&RSkill store has not yet been implemented.\n\r\n\r", d );
                    send_to_buffer( "&z[&WPress ENTER to return to the Menu&z]\n\r\n\r", d );
                    d->connected = CON_MENU_ENTER;
                    break;
                }

                /* Display double-column skill shop list */
                send_skill_store( ch, d );
                d->connected = CON_MENU_SKILL1;
                return;
            }
            else if ( c == 5 )
            {
                send_to_buffer( "\n\r\n\r", d );
                send_to_buffer( "&g---={ &GAliens vs. Predator: Stat Store&g }=---\n\r\n\r", d );
                send_to_buffer( "&C1&z) &WStrength    &C2&z) &WStamina     &C3&z) &WRecovery\n\r", d );
                send_to_buffer( "&C4&z) &WPerception  &C5&z) &WBravery     &C6&z) &WIntelligence\n\r", d );
                send_to_buffer( "\n\r&z[&BChoose an attribute or hit enter to return&z]\n\r\n\r", d );
                d->connected = CON_MENU_STAT1;
                return;
            }
            else if ( c == 6 )
            {
                if ( ch->top_level <= 0 )
                {
                    send_to_buffer( "&G(AVP Monitor) You must enter the game alteast once first.\n\r", d );
                    send_to_buffer( "&z[&WPress ENTER to return to Menu&z]\n\r\n\r", d );
                    d->connected = CON_MENU_ENTER;
                    return;
                }

                send_to_buffer( "\n\r\n\r&z(&CO&z)ld password: ", d );
                send_to_buffer( echo_off_str, d );
                d->connected = CON_MENU_NEWPASS1;
                return;
            }
            else if ( c == 7 && d->character->last_cmd == NULL )
            {
                if ( ch->top_level <= 0 )
                {
                    send_to_buffer( "\n\r\n\r&G(AVP Monitor) Hey, give us a fair shot first, come on!\n\r", d );
                    send_to_buffer( "&G(AVP Monitor) Just quit though if you REALLY want to delete right now.\n\r", d );
                    send_to_buffer( "&z[&WPress ENTER to return to Menu&z]\n\r\n\r", d );
                    d->connected = CON_MENU_ENTER;
                    return;
                }

                send_to_buffer( "\n\r\n\r&z(&CE&z)nter your password to confirm: ", d );
                send_to_buffer( echo_off_str, d );
                d->connected = CON_MENU_DELETE;
                return;
            }
            else
            {
                send_to_buffer( "\n\r&RInvalid choice. Choose again. ", d );
                return;
            }

            break;

        case CON_MENU_LEVEL:
            switch ( argument[0] )
            {
                case 'y':
                case 'Y':
                    ch->currexp -= exp_level( ch->top_level + 1 );
                    send_to_buffer( "\n\r", d );
                    advance_level( ch, FALSE );
                    send_to_buffer( "&z[&WPress ENTER to return to the Menu&z]\n\r\n\r", d );
                    d->connected = CON_MENU_ENTER;
                    return;

                case 'n':
                case 'N':
                    break;

                default:
                    send_to_buffer( "\n\r&z(&CI&z)nvalid selection. [&CY&z]es or [&CN&z]o? ", d );
                    return;
            }

            write_menu_to_desc( d );
            d->connected = CON_MENU_BASE;
            break;

        case CON_MENU_SKILL1:
            if ( argument[0] == '\0' )
            {
                c = 0;
            }
            else if ( !is_number( argument ) )
            {
                c = -1;
            }
            else
            {
                c = atoi( argument );
            }

            if ( c < 0 || c > 19 )
            {
                send_to_buffer( "&RInvalid choice. Choose again.\n\r", d );
                return;
            }

            if ( c == 0 )
            {
                send_to_buffer( "\n\r\n\r", d );
                write_menu_to_desc( d );
                d->connected = CON_MENU_BASE;
                return;
            }

            sn = get_store_skill( ch, c );

            if ( sn == -1 )
            {
                send_to_buffer( "&RInvalid choice. Choose again.\n\r", d );
                return;
            }

            if ( skill_table[sn]->min_level > ch->top_level )
            {
                send_to_buffer( "&RYou cannot learn that skill yet. Choose again.\n\r", d );
                return;
            }

            if ( ch->pcdata->learned[sn] >= 3 )
            {
                send_to_buffer( "&RThat skill is already maxed out. Choose again.\n\r", d );
                return;
            }

            if ( ch->pcdata->learned[sn] <= 0 )
                cost = 500;

            if ( ch->pcdata->learned[sn] == 1 )
                cost = 1000;

            if ( ch->pcdata->learned[sn] == 2 )
                cost = 2000;

            sprintf( buf, "\n\r&zSkill Cost: [%s&z] (&WRequires %d XP&z)\n\r\n\r", drawbar( 10, ch->currexp, cost, "&G", "&g" ), cost );
            send_to_buffer( buf, d );

            if ( ch->currexp < cost )
            {
                send_to_buffer( "&RYou cannot afford to raise this skill yet.\n\r", d );
                send_to_buffer( "&z[&WPress ENTER to return to the Menu&z]\n\r\n\r", d );
                d->connected = CON_MENU_ENTER;
                return;
            }
            else
            {
                send_to_buffer( "&zYou can afford to raise this skill. Proceed? [&CY&z/&CN&z] ", d );
                ch->tempnum = sn;
                d->connected = CON_MENU_SKILL2;
                break;
            }

            break;

        case CON_MENU_SKILL2:
            sn = ch->tempnum;

            if ( ch->pcdata->learned[sn] <= 0 )
                cost = 500;

            if ( ch->pcdata->learned[sn] == 1 )
                cost = 1000;

            if ( ch->pcdata->learned[sn] == 2 )
                cost = 2000;

            switch ( argument[0] )
            {
                case 'y':
                case 'Y':
                    ch->currexp -= cost;
                    ch->pcdata->learned[sn]++;
                    send_to_buffer( "\n\r", d );
                    sprintf( buf, "\n\r&zYou are now %s at &W%s&z.\n\r",
                             ( cost == 500 ) ? "Basic" : ( ( cost == 1000 ) ? "Advanced" : "Expert" ),
                             capitalize( skill_table[sn]->name ) );
                    send_to_buffer( buf, d );
                    send_to_buffer( "&z[&WPress ENTER to return to the Menu&z]\n\r\n\r", d );
                    d->connected = CON_MENU_ENTER;
                    return;

                case 'n':
                case 'N':
                    send_to_buffer( "\n\r\n\r", d );
                    break;

                default:
                    send_to_buffer( "\n\r&z(&CI&z)nvalid selection. [&CY&z]es or [&CN&z]o? ", d );
                    return;
            }

            write_menu_to_desc( d );
            d->connected = CON_MENU_BASE;
            break;

        case CON_MENU_STAT1:
            if ( argument[0] == '\0' )
            {
                c = 0;
            }
            else if ( !is_number( argument ) )
            {
                c = -1;
            }
            else
            {
                c = atoi( argument );
            }

            if ( c < 0 || c > 6 )
            {
                send_to_buffer( "&RInvalid choice. Choose again.\n\r", d );
                return;
            }

            if ( c == 0 )
            {
                send_to_buffer( "\n\r\n\r", d );
                write_menu_to_desc( d );
                d->connected = CON_MENU_BASE;
                return;
            }

            if ( c == 1 )
                sn = ch->perm_str;

            if ( c == 2 )
                sn = ch->perm_sta;

            if ( c == 3 )
                sn = ch->perm_rec;

            if ( c == 4 )
                sn = ch->perm_per;

            if ( c == 5 )
                sn = ch->perm_bra;

            if ( c == 6 )
                sn = ch->perm_int;

            if ( sn >= 30 )
            {
                send_to_buffer( "&RThat attribute is already at maximum. Choose again.\n\r", d );
                return;
            }

            cost = ( sn + 1 ) * 300;
            sprintf( buf, "\n\r&zAttribute Rank: [%s&z] (&W%d of 30 points&z)\n\r", drawbar( 10, sn, 30, "&B", "&B" ), sn );
            send_to_buffer( buf, d );
            sprintf( buf, "&zAttribute Cost: [%s&z] (&WRequires %d XP&z)\n\r\n\r", drawbar( 10, ch->currexp, cost, "&G", "&g" ), cost );
            send_to_buffer( buf, d );

            if ( ch->currexp < cost )
            {
                send_to_buffer( "&RYou cannot afford to raise this attribute yet.\n\r", d );
                send_to_buffer( "&z[&WPress ENTER to return to the Menu&z]\n\r\n\r", d );
                d->connected = CON_MENU_ENTER;
                return;
            }
            else
            {
                send_to_buffer( "&zYou can afford to raise this attribute. Proceed? [&CY&z/&CN&z] ", d );
                ch->tempnum = c;
                d->connected = CON_MENU_STAT2;
                break;
            }

            break;

        case CON_MENU_STAT2:
            sn = ch->tempnum;

            if ( sn == 1 )
                cost = ( ch->perm_str + 1 ) * 300;

            if ( sn == 2 )
                cost = ( ch->perm_sta + 1 ) * 300;

            if ( sn == 3 )
                cost = ( ch->perm_rec + 1 ) * 300;

            if ( sn == 4 )
                cost = ( ch->perm_per + 1 ) * 300;

            if ( sn == 5 )
                cost = ( ch->perm_bra + 1 ) * 300;

            if ( sn == 6 )
                cost = ( ch->perm_int + 1 ) * 300;

            switch ( argument[0] )
            {
                case 'y':
                case 'Y':
                    ch->currexp -= cost;

                    if ( sn == 1 )
                        ch->perm_str++;

                    if ( sn == 2 )
                        ch->perm_sta++;

                    if ( sn == 3 )
                        ch->perm_rec++;

                    if ( sn == 4 )
                        ch->perm_per++;

                    if ( sn == 5 )
                        ch->perm_bra++;

                    if ( sn == 6 )
                        ch->perm_int++;

                    send_to_buffer( "\n\r", d );
                    send_to_buffer( "\n\r&YAttribute has been increased by 1 point. Congratz.\n\r", d );
                    send_to_buffer( "&z[&WPress ENTER to return to the Menu&z]\n\r\n\r", d );
                    d->connected = CON_MENU_ENTER;
                    return;

                case 'n':
                case 'N':
                    send_to_buffer( "\n\r\n\r", d );
                    break;

                default:
                    send_to_buffer( "\n\r&z(&CI&z)nvalid selection. [&CY&z]es or [&CN&z]o? ", d );
                    return;
            }

            write_menu_to_desc( d );
            d->connected = CON_MENU_BASE;
            break;

        case CON_MENU_NEWPASS1:
            send_to_buffer( "\n\r", d );

            if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
            {
                send_to_buffer( echo_on_str, d );
                send_to_buffer( "\n\r&RIncorrect password. &z[&WPress ENTER&z]\n\r\n\r", d );
                d->connected = CON_MENU_ENTER;
                return;
            }

            send_to_buffer( "\n\r&z(&CN&z)ew password: ", d );
            d->connected = CON_MENU_NEWPASS2;
            break;

        case CON_MENU_NEWPASS2:
            send_to_buffer( "\n\r", d );

            if ( strlen( argument ) < 5 )
            {
                send_to_buffer( "&z(&CP&z)assword must be at least five characters long.\n\r&z(&CP&z)assword: ", d );
                return;
            }

            pwdnew = crypt( argument, ch->name );

            for ( p = pwdnew; *p != '\0'; p++ )
            {
                if ( *p == '~' )
                {
                    send_to_buffer( "&z(&CN&z)ew password not acceptable, try again.\n\r&z(&CP&z)assword: ", d );
                    return;
                }
            }

            DISPOSE( ch->pcdata->pwd );
            ch->pcdata->pwd = str_dup( pwdnew );
            send_to_buffer( "&z(&CP&z)lease retype the password to confirm: ", d );
            d->connected = CON_MENU_NEWPASS3;
            break;

        case CON_MENU_NEWPASS3:

            /* send_to_buffer( "\n\r", d ); */
            if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
            {
                send_to_buffer( "&z(&CP&z)asswords don't match.\n\r&z(&CR&z)etype password: ", d );
                d->connected = CON_MENU_NEWPASS2;
                return;
            }

            send_to_buffer( echo_on_str, d );
            send_to_buffer( "\n\r\n\r&YPassword successfully changed. &z[&WPress ENTER&z]\n\r\n\r", d );
            do_save( ch, "-noalert" );
            d->connected = CON_MENU_ENTER;
            break;

        case CON_MENU_DELETE:
            send_to_buffer( echo_on_str, d );

            if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
            {
                send_to_buffer( "\n\r\n\r&RIncorrect password. &z[&WPress ENTER&z]\n\r\n\r", d );
                d->connected = CON_MENU_ENTER;
                return;
            }

            delete_player( ch->name );
            send_to_buffer( "\n\r\n\r&RCharacter deleted. Thanks for playing.\n\r\n\r", d );
            close_socket( d, FALSE );
            break;

        case CON_MENU_ENTER:
            write_menu_to_desc( d );
            d->connected = CON_MENU_BASE;
            break;

        case CON_PRESS_ENTER:
            if ( xIS_SET( ch->act, PLR_ANSI ) )
                send_to_pager( "\033[2J", ch );
            else
                send_to_pager( "\014", ch );

            if ( IS_IMMORTAL( ch ) )
            {
                send_to_pager( "\n\r&z(&CI&z)mmortal Message of the Day&z\n\r", ch );
                do_help( ch, "imotd" );
            }

            if ( ch->top_level <= 100 && ch->top_level > 0 )
            {
                send_to_pager( "\n\r&z(&CM&z)essage of the Day&z\n\r", ch );
                do_help( ch, "motd" );
            }

            if ( ch->top_level == 0 )
                do_help( ch, "nmotd" );

            send_to_pager( "\n\r", ch );
            write_menu_to_desc( d );
            d->connected = CON_MENU_BASE;
            break;

        case CON_ENTER_GAME:
            add_char( ch );
            d->connected    = CON_PLAYING;

            if ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_SOUND ) )
                send_to_char( "!!MUSIC(avp.mid V=100)", ch );

            if ( ch->top_level == 0 )
            {
                /* OBJ_DATA *obj; */
                int iLang;
                set_ident( ch );
                xCLEAR_BITS( ch->affected_by );

                for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
                    if ( race_table[ch->race].language == iLang )
                        break;

                if ( lang_array[iLang] == LANG_UNKNOWN )
                    bug( "Nanny: invalid racial language." );
                else
                {
                    if ( ( iLang = skill_lookup( lang_names[iLang] ) ) < 0 )
                        bug( "Nanny: cannot find racial language." );
                    else
                    {
                        ch->pcdata->learned[iLang] = 100;
                        xCLEAR_BITS( ch->speaking );
                        xSET_BIT( ch->speaking, race_table[ch->race].language );
                    }
                }

                /* Does'nt do anything anymore */
                /* name_stamp_stats( ch ); */
                ch->top_level = 1;
                ch->max_hit = race_table[ch->race].hit;
                ch->max_move = race_table[ch->race].move;
                ch->hit     = ch->max_hit;
                ch->move    = ch->max_move;
                sprintf( buf, "%s", ch->name );

                if ( ch->pcdata->fname && ch->pcdata->lname )
                {
                    if ( ch->pcdata->fname[0] != '\0' )
                    {
                        sprintf( buf, "%s '%s' %s", ch->pcdata->fname, ch->name, ch->pcdata->lname );
                    }
                }

                set_title( ch, buf );
                xSET_BIT( ch->act, PLR_AUTOEXIT );
                xSET_BIT( ch->pcdata->flags, PCFLAG_SHOWRESET );

                /*
                    Starting equipment system. Rewritten by Ghost <->

                        Marine         - 1: Combat Knife +
                                       - 2: M4A3 Pistol +
                                       - 3: Standard M4A3 Clip (x2) +
                                       - 4: Rations (x2) +
                                       - 5: Kevlar Helmet +
                                       - 6: Kevlar Vest +
                                       - 7: Kevlar Boots +
                                       - 25: M40 Hand Grenade (x3) +
                                       - 26: Field Radio +
                                       _ 28: Flashlight +
                                       - 31: Pulse Rifle +
                                       - 32: M309 Clip (x2) +
                                       - 0:
                        Predator       - 20: Predator Mask
                                       - 21: Cloaking Module - REMOVED
                                       - 22: Combi-Spear +
                                       - 23: Side Blade (x2) +
                                       - 24: Energy Grenade (x3) +
                                       - 26: Energy Pistol
                                       - 27: Light Sphere +
                                       - 30: Hunting Pack +
                                   - 0:
                        Alien          - 0: Nothing needed?

                */
                switch ( ch->race )
                {
                    default:
                        bug( "nanny: invalid class for new character! Cannot create items!" );
                        break;

                    case RACE_MARINE:
                        newbie_create( ch, 5 );  /* Kevlar Helmet */
                        newbie_create( ch, 6 );  /* Kevlar Vest */
                        newbie_create( ch, 7 );  /* Kevlar Boots */
                        newbie_create( ch, 1 );  /* Combat Knife */
                        newbie_create( ch, 2 );  /* M4A3 Pistol */
                        newbie_create( ch, 3 );  /* Standard M4A3 Clip */
                        newbie_create( ch, 3 );  /* Standard M4A3 Clip */
                        newbie_create( ch, 4 );  /* Rations */
                        newbie_create( ch, 4 );  /* Rations */
                        newbie_create( ch, 25 ); /* M40 Hand Grenade */
                        newbie_create( ch, 25 ); /* M40 Hand Grenade */
                        newbie_create( ch, 25 ); /* M40 Hand Grenade */
                        newbie_create( ch, 26 ); /* Field Radio */
                        newbie_create( ch, 28 ); /* Flashlight */
                        newbie_create( ch, 31 ); /* Pulse Rifle */
                        newbie_create( ch, 32 ); /* M309 Clip */
                        newbie_create( ch, 32 ); /* M309 Clip */
                        break;

                    case RACE_PREDATOR:
                        newbie_create( ch, 20 );  /* Predator Mask */
                        newbie_create( ch, 22 );  /* Combi-Spear */
                        newbie_create( ch, 23 );  /* Side Blade */
                        newbie_create( ch, 23 );  /* Side Blade */
                        newbie_create( ch, 29 );  /* Energy Pistol */
                        newbie_create( ch, 24 );  /* Energy Grenade */
                        newbie_create( ch, 24 );  /* Energy Grenade */
                        newbie_create( ch, 24 );  /* Energy Grenade */
                        newbie_create( ch, 27 );  /* Light Sphere */
                        newbie_create( ch, 30 );  /* Hunting Pack */
                        break;

                    case RACE_ALIEN: /* Nothing needed */
                        break;
                }

                if ( !sysdata.WAIT_FOR_AUTH )
                {
                    char_to_room( ch, get_room_index( wherehome( ch ) ) );
                    ch->pcdata->auth_state = 3;
                }
                else
                {
                    char_to_room( ch, get_room_index( wherehome( ch ) ) );
                    ch->pcdata->auth_state = 1;
                    xSET_BIT( ch->pcdata->flags, PCFLAG_UNAUTHED );
                }

                /* Display_prompt interprets blank as default */
                // ch->pcdata->prompt = STRALLOC("");
            }
            else if ( !IS_IMMORTAL( ch ) && ch->pcdata->release_date > current_time )
            {
                char_to_room( ch, get_room_index( 6 ) );
            }
            else if ( ch->in_room && !IS_IMMORTAL( ch )
                      && !xIS_SET( ch->in_room->room_flags, ROOM_SPACECRAFT )
                      && ch->in_room != get_room_index( 6 ) )
            {
                char_to_room( ch, ch->in_room );
            }
            else
            {
                char_to_room( ch, get_room_index( wherehome( ch ) ) );
            }

            if ( get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
                remove_timer( ch, TIMER_SHOVEDRAG );

            if ( get_timer( ch, TIMER_PKILLED ) > 0 )
                remove_timer( ch, TIMER_PKILLED );

            /* Automatic IP Logging System */
            {
                char buf[MAX_STRING_LENGTH];
                char filenameA[MAX_STRING_LENGTH];
                char filenameB[MAX_STRING_LENGTH];
                char* strtime;
                strtime                    = ctime( &current_time );
                strtime[strlen( strtime ) - 1] = '\0';
                sprintf( filenameA, "%sconnect.log", LOG_DIR );
                sprintf( filenameB, "%sconnect.old", LOG_DIR );

                if ( ch->desc && ch->desc->original )
                {
                    sprintf( buf, "%s :: %-14s : (%s@%s)", strtime, ch->desc->original->name, ch->desc->user, ch->desc->host );
                }
                else
                {
                    sprintf( buf, "%s :: %-12s: (%s@%s)", strtime, ch->name, ch->desc->user, ch->desc->host );
                }

                /* Stops logging at 10 megs, instead rename the old one to a new name */
                if ( file_size( filenameA ) < 10000000 )
                {
                    append_to_file( filenameA, buf );
                }
                else
                {
                    log_string( "[File capacity reached, shifting .log to .old]" );
                    rename( filenameA, filenameB );
                    append_to_file( filenameA, buf );
                }
            }

            if ( !IS_IMMORTAL( ch ) )
            {
                char tmp[MAX_STRING_LENGTH];
                sprintf( tmp, "&GAvP welcomes %s to the fray.", ch->name );
                send_monitor( ch, tmp );
                act( AT_ACTION, "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
                write_serverstats();
            }

            do_look( ch, "auto" );
            mail_count( ch );
            break;
    }

    return;
}



/*
    Parse a name for acceptability.
*/
bool check_parse_name( char* name )
{
    BOT_DATA* bot;

    if ( name == NULL )
        return FALSE;

    /*
        Reserved words.
    */
    if ( is_name( name, "all auto someone immortal self god supreme demigod dog guard cityguard cat cornholio spock hicaine hithoric death ass fuck shit piss crap public penis rendar rental trainer quit pirate pirates wanker jesus alien marine predator ghostisadick ghostisacock ghostisgay save afk" ) )
        return FALSE;

    /*
        Length restrictions.
    */
    if ( strlen( name ) <  3 )
        return FALSE;

    if ( strlen( name ) > 12 )
        return FALSE;

    if ( profanity_check( name ) )
        return FALSE;

    /*
        Alphanumerics only.
        Lock out IllIll twits.
    */
    {
        char* pc;
        bool fIll;
        fIll = TRUE;

        for ( pc = name; *pc != '\0'; pc++ )
        {
            if ( !isalpha( *pc ) )
                return FALSE;

            if ( LOWER( *pc ) != 'i' && LOWER( *pc ) != 'l' )
                fIll = FALSE;
        }

        if ( fIll )
            return FALSE;
    }

    /*
        Code that followed here used to prevent players from naming
        themselves after mobs... this caused much havoc when new areas
        would go in...
    */

    /*
        Code that followed here used to prevent players from naming
        themselves after bots.
    */
    for ( bot = first_bot; bot; bot = bot->next )
        if ( !str_cmp( bot->name, name ) )
            return FALSE;

    return TRUE;
}



/*
    Look for link-dead player to reconnect.
*/
short check_reconnect( DESCRIPTOR_DATA* d, char* name, bool fConn )
{
    CHAR_DATA* ch;

    for ( ch = first_char; ch; ch = ch->next )
    {
        if ( !IS_NPC( ch )
                && ( !fConn || !ch->desc )
                &&    ch->name
                &&   !str_cmp( name, ch->name ) )
        {
            if ( fConn && ch->switched )
            {
                send_to_buffer( "&z(&CA&z)lready playing.\n\r&z(&CN&z)ame: ", d );
                d->connected = CON_GET_NAME;

                if ( d->character )
                {
                    /* clear descriptor pointer to get rid of bug message in log */
                    d->character->desc = NULL;
                    free_char( d->character );
                    d->character = NULL;
                }

                return BERR;
            }

            if ( fConn == FALSE )
            {
                DISPOSE( d->character->pcdata->pwd );
                d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
            }
            else
            {
                /* clear descriptor pointer to get rid of bug message in log */
                d->character->desc = NULL;
                free_char( d->character );
                d->character = ch;
                ch->desc         = d;
                ch->timer        = 0;
                send_to_buffer( "&z(&RR&z)econnecting.\n\r", d );

                if ( !IS_IMMORTAL( ch ) )
                    act( AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_ROOM );

                sprintf( log_buf, "%s@%s(%s) reconnected.", ch->name, d->host, d->user );
                log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->top_level ) );
                /*
                        if ( ch->top_level < LEVEL_SAVIOR )
                          to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
                */
                d->connected = CON_PLAYING;
            }

            return TRUE;
        }
    }

    return FALSE;
}



/*
    Check if already playing.
*/

bool check_multi( DESCRIPTOR_DATA* d, char* name )
{
    DESCRIPTOR_DATA* dold;
    return FALSE;

    for ( dold = first_descriptor; dold; dold = dold->next )
    {
        if ( dold != d && (  dold->character || dold->original )
                &&   str_cmp( name, dold->original ? dold->original->name : dold->character->name )
                && !str_cmp( dold->host, d->host ) )
        {
            if ( get_trust( d->character ) >= LEVEL_SUPREME || get_trust( dold->original ? dold->original : dold->character ) >= LEVEL_SUPREME )
                return FALSE;

            if ( allowedmp( d ) )
                return FALSE;

            write_to_buffer( d, "Sorry multi-playing is not allowed ... have you other character quit first.\n\r", 0 );
            sprintf( log_buf, "%s attempting to multiplay with %s.", dold->original ? dold->original->name : dold->character->name, d->character->name );
            log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
            d->character = NULL;
            free_char( d->character );
            return TRUE;
        }
    }

    return FALSE;
}

short check_playing( DESCRIPTOR_DATA* d, char* name, bool kick )
{
    CHAR_DATA* ch;
    DESCRIPTOR_DATA* dold;
    int cstate;

    for ( dold = first_descriptor; dold; dold = dold->next )
    {
        if ( dold != d
                && (  dold->character || dold->original )
                &&   !str_cmp( name, dold->original
                               ? dold->original->name : dold->character->name ) )
        {
            cstate = dold->connected;
            ch = dold->original ? dold->original : dold->character;

            if ( !ch->name
                    || ( cstate != CON_PLAYING && cstate != CON_EDITING ) )
            {
                send_to_buffer( "&z(&CA&z)lready connected - try again.\n\r", d );
                sprintf( log_buf, "%s already connected.", ch->name );
                log_string_plus( log_buf, LOG_COMM, sysdata.log_level );
                return BERR;
            }

            if ( !kick )
                return TRUE;

            send_to_buffer( "&z(&CA&z)lready playing... Kicking off old connection.\n\r", d );
            send_to_buffer( "&z(&CK&z)icking off old connection... bye!\n\r", dold );
            close_socket( dold, FALSE );
            /* clear descriptor pointer to get rid of bug message in log */
            d->character->desc = NULL;
            free_char( d->character );
            d->character = ch;
            ch->desc     = d;
            ch->timer    = 0;

            if ( ch->switched )
                do_return( ch->switched, "" );

            ch->switched = NULL;
            send_to_char( "&z(&CR&z)econnecting.\n\r", ch );
            act( AT_ACTION, "$n has reconnected, kicking off old link.",
                 ch, NULL, NULL, TO_ROOM );
            sprintf( log_buf, "%s@%s reconnected, kicking off old link.",
                     ch->name, d->host );
            log_string_plus( log_buf, LOG_COMM, UMAX( sysdata.log_level, ch->top_level ) );
            /*
                    if ( ch->top_level < LEVEL_SAVIOR )
                      to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->top_level );
            */
            d->connected = cstate;
            return TRUE;
        }
    }

    return FALSE;
}



void stop_idling( CHAR_DATA* ch )
{
    if ( !ch
            ||   !ch->desc
            ||    ch->desc->connected != CON_PLAYING
            ||   !ch->was_in_room
            ||    ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
        return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room     = NULL;
    act( AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}

void write_to_pager( DESCRIPTOR_DATA* d, const char* txt, int length )
{
    if ( length <= 0 )
        length = strlen( txt );

    if ( length == 0 )
        return;

    if ( !d->pagebuf )
    {
        d->pagesize = MAX_STRING_LENGTH;
        CREATE( d->pagebuf, char, d->pagesize );
    }

    if ( !d->pagepoint )
    {
        d->pagepoint = d->pagebuf;
        d->pagetop = 0;
        d->pagecmd = '\0';
    }

    if ( d->pagetop == 0 && !d->fcommand )
    {
        d->pagebuf[0] = '\n';
        d->pagebuf[1] = '\r';
        d->pagetop = 2;
    }

    while ( d->pagetop + length >= d->pagesize )
    {
        if ( d->pagesize > 32000 )
        {
            bug( "Pager overflow.  Ignoring.\n\r" );
            d->pagetop = 0;
            d->pagepoint = NULL;
            DISPOSE( d->pagebuf );
            d->pagesize = MAX_STRING_LENGTH;
            return;
        }

        d->pagesize *= 2;
        RECREATE( d->pagebuf, char, d->pagesize );
    }

    strncpy( d->pagebuf + d->pagetop, txt, length );
    d->pagetop += length;
    d->pagebuf[d->pagetop] = '\0';
    return;
}

/*  commented out in favour of colour routine

    void send_to_pager( const char *txt, CHAR_DATA *ch )
    {
    if ( !ch )
    {
    bug( "Send_to_pager: NULL *ch" );
    return;
    }
    if ( txt && ch->desc )
    {
    DESCRIPTOR_DATA *d = ch->desc;

    ch = d->original ? d->original : d->character;
    if ( IS_NPC(ch) || !xIS_SET(ch->pcdata->flags, PCFLAG_PAGERON) )
    {
    send_to_char(txt, d->character);
    return;
    }
    write_to_pager(d, txt, 0);
    }
    return;
    }

*/

void send_to_pager_color( const char* txt, CHAR_DATA* ch )
{
    DESCRIPTOR_DATA* d;
    char* colstr;
    const char* prevstr = txt;
    char colbuf[20];
    int ln;

    if ( !ch )
    {
        bug( "Send_to_pager_color: NULL *ch" );
        return;
    }

    if ( !txt || !ch->desc )
        return;

    d = ch->desc;
    ch = d->original ? d->original : d->character;

    if ( IS_NPC( ch ) || !xIS_SET( ch->pcdata->flags, PCFLAG_PAGERON ) )
    {
        send_to_char_color( txt, d->character );
        return;
    }

    /* Clear out old color stuff */
    while ( ( colstr = strpbrk( prevstr, "&^" ) ) != NULL )
    {
        if ( colstr > prevstr )
            write_to_pager( d, prevstr, ( colstr - prevstr ) );

        ln = make_color_sequence( colstr, colbuf, d );

        if ( ln < 0 )
        {
            prevstr = colstr + 1;
            break;
        }
        else if ( ln > 0 )
            write_to_pager( d, colbuf, ln );

        prevstr = colstr + 2;
    }

    if ( *prevstr )
        write_to_pager( d, prevstr, 0 );

    return;
}

/*
    Color Routine =)
*/
void send_to_buffer( const char* txt, DESCRIPTOR_DATA* d )
{
    CHAR_DATA* ch;
    char* colstr;
    const char* prevstr = txt;
    char colbuf[20];
    int ln;

    if ( !d )
    {
        bug( "Send_to_buffer: NULL *d" );
        return;
    }

    if ( !txt )
        return;

    ch = d->original ? d->original : d->character;

    if ( ch != NULL )
    {
        if ( IS_NPC( ch ) )
        {
            send_to_char_color( txt, d->character );
            return;
        }

        xSET_BIT( ch->act, PLR_ANSI );
    }

    /* Clear out old color stuff */
    while ( ( colstr = strpbrk( prevstr, "&^" ) ) != NULL )
    {
        if ( colstr > prevstr )
            write_to_buffer( d, prevstr, ( colstr - prevstr ) );

        ln = make_color_sequence( colstr, colbuf, d );

        if ( ln < 0 )
        {
            prevstr = colstr + 1;
            break;
        }
        else if ( ln > 0 )
            write_to_buffer( d, colbuf, ln );

        prevstr = colstr + 2;
    }

    if ( *prevstr )
        write_to_buffer( d, prevstr, 0 );

    return;
}

void set_char_color( sh_int AType, CHAR_DATA* ch )
{
    char buf[16];
    CHAR_DATA* och;

    if ( !ch || !ch->desc )
        return;

    if ( ch->desc )
        och = ( ch->desc->original ? ch->desc->original : ch );

    if ( !ch->desc )
        return;

    if ( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) )
    {
        if ( AType == 7 )
            strcpy( buf, "\033[m" );
        else
            sprintf( buf, "\033[0;%d;%s%dm", ( AType & 8 ) == 8,
                     ( AType > 15 ? "5;" : "" ), ( AType & 7 ) + 30 );

        write_to_buffer( ch->desc, buf, strlen( buf ) );
    }

    return;
}

void set_pager_color( sh_int AType, CHAR_DATA* ch )
{
    char buf[16];
    CHAR_DATA* och;

    if ( !ch || !ch->desc )
        return;

    och = ( ch->desc->original ? ch->desc->original : ch );

    if ( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) )
    {
        if ( AType == 7 )
            strcpy( buf, "\033[m" );
        else
            sprintf( buf, "\033[0;%d;%s%dm", ( AType & 8 ) == 8,
                     ( AType > 15 ? "5;" : "" ), ( AType & 7 ) + 30 );

        send_to_pager( buf, ch );
        ch->desc->pagecolor = AType;
    }

    return;
}


/* source: EOD, by John Booth <???> */
void ch_printf( CHAR_DATA* ch, char* fmt, ... )
{
    char buf[MAX_STRING_LENGTH * 2];    /* better safe than sorry */
    va_list args;
    va_start( args, fmt );
    vsprintf( buf, fmt, args );
    va_end( args );
    send_to_char( buf, ch );
}

void pager_printf( CHAR_DATA* ch, char* fmt, ... )
{
    char buf[MAX_STRING_LENGTH * 2];
    va_list args;
    va_start( args, fmt );
    vsprintf( buf, fmt, args );
    va_end( args );
    send_to_pager( buf, ch );
}



char* obj_short( OBJ_DATA* obj )
{
    static char buf[MAX_STRING_LENGTH];

    if ( obj->count > 1 )
    {
        sprintf( buf, "%s (%d)", obj->short_descr, obj->count );
        return buf;
    }

    return obj->short_descr;
}

/*
    The primary output interface for formatted output.
*/
/* Major overhaul. -- Alty */
#define NAME(ch)        (IS_NPC(ch) ? ch->short_descr : ch->name)
char* act_string( const char* format, CHAR_DATA* to, CHAR_DATA* ch,
                  const void* arg1, const void* arg2 )
{
    static char* const he_she  [] = { "it",  "he",  "she" };
    static char* const him_her [] = { "it",  "him", "her" };
    static char* const his_her [] = { "its", "his", "her" };
    static char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    char* point = buf;
    const char* str = format;
    const char* i;
    CHAR_DATA* vch = ( CHAR_DATA* ) arg2;
    OBJ_DATA* obj1 = ( OBJ_DATA* ) arg1;
    OBJ_DATA* obj2 = ( OBJ_DATA* ) arg2;

    if ( !to )
        return "(error)";

    while ( *str != '\0' )
    {
        if ( *str != '$' )
        {
            *point++ = *str++;
            continue;
        }

        ++str;

        if ( !arg2 && *str >= 'A' && *str <= 'Z' )
        {
            bug( "Act: missing arg2 for code %c:", *str );
            bug( format );
            i = " <@@@> ";
        }
        else
        {
            switch ( *str )
            {
                default:
                    bug( "Act: bad code %c.", *str );
                    i = " <@@@> ";
                    break;

                case 't':
                    i = ( char* ) arg1;
                    break;

                case 'T':
                    i = ( char* ) arg2;
                    break;

                case 'n':
                    i = ( to ? PERS( ch, to ) : NAME( ch ) );
                    break;

                case 'N':
                    i = ( to ? PERS( vch, to ) : NAME( vch ) );
                    break;

                case 'e':
                    if ( ch->sex > 2 || ch->sex < 0 )
                    {
                        bug( "act_string: player %s has sex set at %d!", ch->name,
                             ch->sex );
                        i = "it";
                    }
                    else
                        i = he_she [URANGE( 0,  ch->sex, 2 )];

                    break;

                case 'E':
                    if ( vch->sex > 2 || vch->sex < 0 )
                    {
                        bug( "act_string: player %s has sex set at %d!", vch->name,
                             vch->sex );
                        i = "it";
                    }
                    else
                        i = he_she [URANGE( 0, vch->sex, 2 )];

                    break;

                case 'm':
                    if ( ch->sex > 2 || ch->sex < 0 )
                    {
                        bug( "act_string: player %s has sex set at %d!", ch->name,
                             ch->sex );
                        i = "it";
                    }
                    else
                        i = him_her[URANGE( 0,  ch->sex, 2 )];

                    break;

                case 'M':
                    if ( vch->sex > 2 || vch->sex < 0 )
                    {
                        bug( "act_string: player %s has sex set at %d!", vch->name,
                             vch->sex );
                        i = "it";
                    }
                    else
                        i = him_her[URANGE( 0, vch->sex, 2 )];

                    break;

                case 's':
                    if ( ch->sex > 2 || ch->sex < 0 )
                    {
                        bug( "act_string: player %s has sex set at %d!", ch->name,
                             ch->sex );
                        i = "its";
                    }
                    else
                        i = his_her[URANGE( 0,  ch->sex, 2 )];

                    break;

                case 'S':
                    if ( vch->sex > 2 || vch->sex < 0 )
                    {
                        bug( "act_string: player %s has sex set at %d!", vch->name,
                             vch->sex );
                        i = "its";
                    }
                    else
                        i = his_her[URANGE( 0, vch->sex, 2 )];

                    break;

                case 'q':
                    i = ( to == ch ) ? "" : "s";
                    break;

                case 'Q':
                    i = ( to == ch ) ? "your" :
                        his_her[URANGE( 0,  ch->sex, 2 )];
                    break;

                case 'p':
                    i = ( !to || can_see_obj( to, obj1 )
                          ? stripclr( obj_short( obj1 ) ) : "something" );
                    break;

                case 'P':
                    i = ( !to || can_see_obj( to, obj2 )
                          ? stripclr( obj_short( obj2 ) ) : "something" );
                    break;

                case 'd':
                    if ( !arg2 || ( ( char* ) arg2 )[0] == '\0' )
                        i = "door";
                    else
                    {
                        one_argument( ( char* ) arg2, fname );
                        i = fname;
                    }

                    break;
            }
        }

        ++str;

        while ( ( *point = *i ) != '\0' )
            ++point, ++i;
    }

    strcpy( point, "\n\r" );
    buf[0] = UPPER( buf[0] );
    return buf;
}
#undef NAME

void act( sh_int AType, const char* format, CHAR_DATA* ch, const void* arg1, const void* arg2, int type )
{
    char* txt;
    CHAR_DATA* to;
    CHAR_DATA* vch = ( CHAR_DATA* )arg2;

    /*
        Discard null and zero-length messages.
    */
    if ( !format || format[0] == '\0' )
        return;

    if ( !ch )
    {
        bug( "Act: null ch. (%s)", format );
        return;
    }

    if ( !ch->in_room )
        to = NULL;
    else if ( type == TO_CHAR )
        to = ch;
    else
        to = ch->in_room->first_person;

    /*
        ACT_SECRETIVE handling
    */
    if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_SECRETIVE ) && type != TO_CHAR )
        return;

    if ( type == TO_VICT )
    {
        if ( !vch )
        {
            bug( "Act: null vch with TO_VICT." );
            bug( "%s (%s)", ch->name, format );
            return;
        }

        if ( !vch->in_room )
        {
            bug( "Act: vch in NULL room!" );
            bug( "%s -> %s (%s)", ch->name, vch->name, format );
            return;
        }

        to = vch;
        /*      to = vch->in_room->first_person;*/
    }

    if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
    {
        OBJ_DATA* to_obj;
        txt = act_string( format, NULL, ch, arg1, arg2 );

        if ( xIS_SET( to->in_room->progtypes, ACT_PROG ) )
            rprog_act_trigger( txt, to->in_room, ch, ( OBJ_DATA* )arg1, ( void* )arg2 );

        for ( to_obj = to->in_room->first_content; to_obj;
                to_obj = to_obj->next_content )
            if ( xIS_SET( to_obj->pIndexData->progtypes, ACT_PROG ) )
                oprog_act_trigger( txt, to_obj, ch, ( OBJ_DATA* )arg1, ( void* )arg2 );
    }

    /*  Anyone feel like telling me the point of looping through the whole
        room when we're only sending to one char anyways..? -- Alty */
    for ( ; to; to = ( type == TO_CHAR || type == TO_VICT ) ? NULL : to->next_in_room )
    {
        if ( ( !to->desc
                && (  IS_NPC( to ) && !xIS_SET( to->pIndexData->progtypes, ACT_PROG ) ) )
                ||   !IS_AWAKE( to ) )
            continue;

        if ( type == TO_CHAR && to != ch )
            continue;

        if ( type == TO_VICT && ( to != vch || to == ch ) )
            continue;

        if ( type == TO_ROOM )
        {
            if ( to == ch )
                continue;

            if ( to->vent != ch->vent )
                continue;
        }

        if ( type == TO_NOTVICT && ( to == ch || to == vch ) )
            continue;

        txt = act_string( format, to, ch, arg1, arg2 );

        if ( to->desc )
        {
            set_char_color( AType, to );
            send_to_char_color( txt, to );
        }

        if ( MOBtrigger )
        {
            /* Note: use original string, not string with ANSI. -- Alty */
            mprog_act_trigger( txt, to, ch, ( OBJ_DATA* )arg1, ( void* )arg2 );
        }
    }

    MOBtrigger = TRUE;
    return;
}

void do_name( CHAR_DATA* ch, char* argument )
{
    char fname[1024];
    struct stat fst;
    CHAR_DATA* tmp;

    if ( !NOT_AUTHED( ch ) || ch->pcdata->auth_state != 2 )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument[0] = UPPER( argument[0] );

    if ( !check_parse_name( argument ) )
    {
        send_to_char( "Illegal name, try another.\n\r", ch );
        return;
    }

    if ( !str_cmp( ch->name, argument ) )
    {
        send_to_char( "That's already your name!\n\r", ch );
        return;
    }

    for ( tmp = first_char; tmp; tmp = tmp->next )
    {
        if ( !str_cmp( argument, tmp->name ) )
            break;
    }

    if ( tmp )
    {
        send_to_char( "That name is already taken.  Please choose another.\n\r", ch );
        return;
    }

    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower( argument[0] ),
             capitalize( argument ) );

    if ( stat( fname, &fst ) != -1 )
    {
        send_to_char( "That name is already taken.  Please choose another.\n\r", ch );
        return;
    }

    STRFREE( ch->name );
    ch->name = STRALLOC( argument );
    send_to_char( "Your name has been changed.  Please apply again.\n\r", ch );
    ch->pcdata->auth_state = 2;
    return;
}

char* default_prompt( CHAR_DATA* ch )
{
    static char buf[MAX_STRING_LENGTH];
    strcpy( buf, "" );

    if ( ch->race == RACE_ALIEN )
        strcat( buf, "&R<&BHealth: &G%C &w&BResin: &G%s &w&BMove: &z[%P&w&z] &w&BAttack: &z[%p&w&z] &BState: &z[%S&w&z] &z(&GSwarm: %x&z)&R> &w%_" );

    if ( ch->race == RACE_MARINE )
        strcat( buf, "&R<&BHealth: &G%C &w&BArmor: &G%A &w&BMove: &z[%P&w&z] &w&BAttack: &z[%p&w&z] &BState: &z[%S&w&z]&R> &w%_" );

    if ( ch->race == RACE_PREDATOR )
        strcat( buf, "&R<&BHealth: &G%C &w&BVision: &z[%K&w&z] &w&BMove: &z[%P&w&z] &w&BAttack: &z[%p&w&z] &BState: &z[%S&w&z]&R> &w%_" );

    return buf;
}

int getcolor( char clr )
{
    static const char colors[16] = "xrgObpcwzRGYBPCW";
    int r;

    for ( r = 0; r < 16; r++ )
        if ( clr == colors[r] )
            return r;

    return -1;
}

// bool is_school_day( void )
// {

//   return FALSE;
// }

void display_prompt( DESCRIPTOR_DATA* d )
{
    CHAR_DATA* ch = d->character;
    CHAR_DATA* och = ( d->original ? d->original : d->character );
    bool ansi = ( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) );
    const char* prompt;
    char buf[MAX_STRING_LENGTH];
    char* pbuf = buf;
    int stat, percent;
    int marmor = 0;

    if ( !ch )
    {
        bug( "display_prompt: NULL ch" );
        return;
    }

    if ( !IS_NPC( ch ) && ch->substate != SUB_NONE && ch->pcdata->subprompt
            &&   ch->pcdata->subprompt[0] != '\0' )
        prompt = ch->pcdata->subprompt;
    else if ( IS_NPC( ch ) || !ch->pcdata->prompt || !*ch->pcdata->prompt )
        prompt = default_prompt ( ch );
    else
        prompt = ch->pcdata->prompt;

    if ( ansi )
    {
        strcpy( pbuf, "\033[m" );
        d->prevcolor = 0x07;
        pbuf += 3;
    }

    /* Clear out old color stuff */
    for ( ; *prompt; prompt++ )
    {
        /*
            '&' = foreground color/intensity bit
            '^' = background color/blink bit
            '%' = prompt commands
            Note: foreground changes will revert background to 0 (black)
        */
        if ( *prompt != '&' && *prompt != '^' && *prompt != '%' )
        {
            *( pbuf++ ) = *prompt;
            continue;
        }

        ++prompt;

        if ( !*prompt )
            break;

        if ( *prompt == *( prompt - 1 ) )
        {
            *( pbuf++ ) = *prompt;
            continue;
        }

        switch ( *( prompt - 1 ) )
        {
            default:
                bug( "Display_prompt: bad command char '%c'.", *( prompt - 1 ) );
                break;

            case '&':
            case '^':
                stat = make_color_sequence( &prompt[-1], pbuf, d );

                if ( stat < 0 )
                    --prompt;
                else if ( stat > 0 )
                    pbuf += stat;

                break;

            case '%':
                *pbuf = '\0';
                stat = 0x80000000;

                switch ( *prompt )
                {
                    case '%':
                        *pbuf++ = '%';
                        *pbuf = '\0';
                        break;

                    case 'c':          // Graphic health prompt
                        percent = -1;

                        if ( ch->max_hit > 0 )
                            percent = ( 100 * ch->hit ) / ch->max_hit;

                        if ( percent >= 100 )
                            sprintf ( pbuf, "%s|||%s|||%s||||%s", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[1;32m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 90 )
                            sprintf ( pbuf, "%s|||%s|||%s|||%s ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[1;32m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 80 )
                            sprintf ( pbuf, "%s|||%s|||%s||%s  ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[1;32m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 70 )
                            sprintf ( pbuf, "%s|||%s|||%s|%s   ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[1;32m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 60 )
                            sprintf ( pbuf, "%s|||%s|||%s    ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 50 )
                            sprintf ( pbuf, "%s|||%s||%s     ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 40 )
                            sprintf ( pbuf, "%s|||%s|%s      ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 30 )
                            sprintf ( pbuf, "%s|||%s       ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 20 )
                            sprintf ( pbuf, "%s||%s        ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 10 )
                            sprintf ( pbuf, "%s|%s         ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else
                            sprintf ( pbuf, "          " );

                        break;

                    case 's':                     // Percentage resin prompt
                        percent = -1;

                        if ( get_max_resin( ch ) > 0 )
                            percent =  URANGE( 0, ( 100 * ch->resin ) / get_max_resin( ch ), 100 );

                        if ( percent >= 60 )
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;32m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 30 && percent < 60 )
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;33m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );
                        else
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;31m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );

                        break;

                    case 'x':
                        stat = ch->swarm;
                        break;

                    case 'S':                     // State Indicator
                        sprintf( pbuf, "%s%s%s%s%s%s%s%s%s",
                                 ANSISAFE( ansi, "\x1b[1;32m" ), ( IS_AFFECTED( ch, AFF_HIDE ) ? "H" : "-" ), ANSISAFE( ansi, "\x1b[0m" ),
                                 ANSISAFE( ansi, "\x1b[1;33m" ), ( IS_AFFECTED( ch, AFF_CLOAK ) ? "C" : "-" ), ANSISAFE( ansi, "\x1b[0m" ),
                                 ANSISAFE( ansi, "\x1b[1;31m" ), ( IS_AFFECTED( ch, AFF_NAPALM ) ? "F" : "-" ),  ANSISAFE( ansi, "\x1b[0m" )
                               );
                        break;

                    case 'C':                     // Percentage health prompt
                        percent = -1;

                        if ( ch->max_hit > 0 )
                            percent =  ( 100 * ch->hit ) / ch->max_hit;

                        if ( percent >= 60 )
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;32m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 30 && percent < 60 )
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;33m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );
                        else
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;31m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );

                        break;

                    case 'z':          // Graphic move prompt
                        percent = -1;

                        if ( ch->max_move > 0 )
                            percent = ( 100 * ch->move ) / ch->max_move;

                        if ( percent >= 100 )
                            sprintf ( pbuf, "%s|||%s|||%s||||%s", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[1;32m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 90 )
                            sprintf ( pbuf, "%s|||%s|||%s|||%s ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[1;32m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 80 )
                            sprintf ( pbuf, "%s|||%s|||%s||%s  ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[1;32m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 70 )
                            sprintf ( pbuf, "%s|||%s|||%s|%s   ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[1;32m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 60 )
                            sprintf ( pbuf, "%s|||%s|||%s    ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 50 )
                            sprintf ( pbuf, "%s|||%s||%s     ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 40 )
                            sprintf ( pbuf, "%s|||%s|%s      ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[1;33m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 30 )
                            sprintf ( pbuf, "%s|||%s       ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 20 )
                            sprintf ( pbuf, "%s||%s        ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 10 )
                            sprintf ( pbuf, "%s|%s         ", ANSISAFE( ansi, "\x1b[1;31m" ), ANSISAFE( ansi, "\x1b[0m" ) );
                        else
                            sprintf ( pbuf, "          " );

                        break;

                    case 'Z':                     // Percentage move prompt
                        percent = -1;

                        if ( ch->max_move > 0 )
                            percent =  ( 100 * ch->move ) / ch->max_move;

                        if ( percent >= 60 )
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;32m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 30 && percent < 60 )
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;33m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );
                        else
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;31m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );

                        break;

                    case 'K':          // Predator Vision Mode
                    {
                        sprintf( pbuf, "%s---", ANSISAFE( ansi, "\x1b[1;30m" ) );

                        if ( ch->vision == RACE_ALIEN )
                            sprintf( pbuf, "%s+++", ANSISAFE( ansi, "\x1b[1;34m" ) );

                        if ( ch->vision == RACE_MARINE )
                            sprintf( pbuf, "%s+++", ANSISAFE( ansi, "\x1b[1;31m" ) );

                        if ( ch->vision == RACE_PREDATOR )
                            sprintf( pbuf, "%s+++", ANSISAFE( ansi, "\x1b[1;32m" ) );
                    }
                    break;

                    case 'p':          // Graphic AP prompt
                    {
                        int max = 0;
                        int cnt = 1;
                        max = get_max_ap( ch );

                        if ( max > 0 )
                            percent = ( 100 * ch->ap ) / max;

                        if ( ( max - ch->ap ) <= 0 )
                            sprintf( pbuf, "%s", ANSISAFE( ansi, "\x1b[1;32m" ) );
                        else if ( ( max - ch->ap ) <= 2 )
                            sprintf( pbuf, "%s", ANSISAFE( ansi, "\x1b[1;33m" ) );
                        else
                            sprintf( pbuf, "%s", ANSISAFE( ansi, "\x1b[1;31m" ) );

                        for ( ; max > 0; max--, cnt++ )
                        {
                            if ( cnt <= ch->ap )
                                strcat( pbuf, "|" );
                            else
                                strcat( pbuf, " " );
                        }
                    }
                    break;

                    case 'P':          // Graphic MP prompt
                    {
                        int max = 0;
                        int cnt = 1;
                        max = get_max_mp( ch );

                        if ( max > 0 )
                            percent = ( 100 * ch->mp ) / max;

                        if ( ( max - ch->mp ) <= 0 )
                            sprintf( pbuf, "%s", ANSISAFE( ansi, "\x1b[1;32m" ) );
                        else if ( ( max - ch->mp ) <= 2 )
                            sprintf( pbuf, "%s", ANSISAFE( ansi, "\x1b[1;33m" ) );
                        else
                            sprintf( pbuf, "%s", ANSISAFE( ansi, "\x1b[1;31m" ) );

                        for ( ; max > 0; max--, cnt++ )
                        {
                            if ( cnt <= ch->mp )
                                strcat( pbuf, "|" );
                            else
                                strcat( pbuf, " " );
                        }
                    }
                    break;

                    case 'A':                     // Percentage armor prompt
                        percent = armor_status( ch, 1 );

                        // marmor = armor_status( ch, 2 );

                        /*
                            if (marmor <= 0) sprintf (pbuf, "%s%d%% (%d)%s", ANSISAFE(ansi, "\x1b[1;32m"), percent, marmor, ANSISAFE(ansi, "\x1b[0m") );
                            else if (percent >= 60) sprintf (pbuf, "%s%d%% (%d)%s", ANSISAFE(ansi, "\x1b[1;32m"), percent, marmor, ANSISAFE(ansi, "\x1b[0m") );
                            else if (percent >= 30 && percent < 60) sprintf (pbuf, "%s%d%% (%d)%s", ANSISAFE(ansi, "\x1b[1;33m"), percent, marmor, ANSISAFE(ansi, "\x1b[0m") );
                            else sprintf (pbuf, "%s%d%% (%d)%s", ANSISAFE(ansi, "\x1b[1;31m"), percent, marmor, ANSISAFE(ansi, "\x1b[0m") );
                        */

                        if ( marmor <= 0 )
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;32m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 60 )
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;32m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );
                        else if ( percent >= 30 && percent < 60 )
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;33m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );
                        else
                            sprintf ( pbuf, "%s%d%%%s", ANSISAFE( ansi, "\x1b[1;31m" ), percent, ANSISAFE( ansi, "\x1b[0m" ) );

                        break;

                    case '_': /* Just add a enter */
                        strcpy( pbuf, "\n\r" );
                        break;

                    case 'h':
                        stat = ch->hit;
                        break;

                    case 'H':
                        stat = ch->max_hit;
                        break;

                    case 'u':
                        stat = num_descriptors;
                        break;

                    case 'U':
                        stat = sysdata.maxplayers;
                        break;

                    case 'v':
                        stat = ch->move;
                        break;

                    case 'V':
                        stat = ch->max_move;
                        break;

                    case 'W':
                        if      ( time_info.hour <  5 )
                            strcpy( pbuf, "night" );
                        else if ( time_info.hour <  6 )
                            strcpy( pbuf, "dawn" );
                        else if ( time_info.hour < 19 )
                            strcpy( pbuf, "day" );
                        else if ( time_info.hour < 20 )
                            strcpy( pbuf, "dusk" );
                        else
                            strcpy( pbuf, "night" );

                        break;

                    case 'J':
                        strcpy( pbuf, "average" );

                        if ( !ch->in_room )
                            break;

                        if ( ch->in_room->area )
                        {
                            if ( ch->in_room->area->ambience == 1 )
                                strcpy( pbuf, "quiet" );

                            if ( ch->in_room->area->ambience == 2 )
                                strcpy( pbuf, "average" );

                            if ( ch->in_room->area->ambience == 3 )
                                strcpy( pbuf, "loud" );
                        }

                        break;

                    case 'r':
                        if ( IS_IMMORTAL( och ) )
                            stat = ch->in_room->vnum;

                        break;

                    case 'R':
                        if ( xIS_SET( och->act, PLR_ROOMVNUM ) )
                            sprintf( pbuf, "<#%d> ", ch->in_room->vnum );

                        break;

                    case 'i':
                        if ( ( !IS_NPC( ch ) && xIS_SET( ch->act, PLR_WIZINVIS ) ) ||
                                ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_MOBINVIS ) ) )
                            sprintf( pbuf, "(Invis %d) ", ( IS_NPC( ch ) ? ch->mobinvis : ch->pcdata->wizinvis ) );
                        else if ( IS_AFFECTED( ch, AFF_INVISIBLE ) )
                            sprintf( pbuf, "(Invis) " );

                        break;

                    case 'b':          // OOC Limit-break
                        if ( ch->pcdata->ooclimit > 6 )
                            ch->pcdata->ooclimit = 6;

                        if ( ch->pcdata->ooclimit <= 6 )
                            sprintf( pbuf, "%s%d", ANSISAFE( ansi, "\x1b[1;32m" ), ch->pcdata->ooclimit );

                        if ( ch->pcdata->ooclimit <= 4 )
                            sprintf( pbuf, "%s%d", ANSISAFE( ansi, "\x1b[1;33m" ), ch->pcdata->ooclimit );

                        if ( ch->pcdata->ooclimit <= 2 )
                            sprintf( pbuf, "%s%d", ANSISAFE( ansi, "\x1b[1;31m" ), ch->pcdata->ooclimit );

                        if ( ch->pcdata->oocbreak )
                            sprintf( pbuf, "%sLB", ANSISAFE( ansi, "\x1b[1;31m" ) );

                        break;

                    case 'B':          // OOC Limit-break
                        if ( ch->pcdata->ooclimit > 6 )
                            ch->pcdata->ooclimit = 6;

                        if ( ch->pcdata->ooclimit == 6 )
                            sprintf( pbuf, "%s||||||", ANSISAFE( ansi, "\x1b[1;32m" ) );

                        if ( ch->pcdata->ooclimit == 5 )
                            sprintf( pbuf, "%s|||||-", ANSISAFE( ansi, "\x1b[1;32m" ) );

                        if ( ch->pcdata->ooclimit == 4 )
                            sprintf( pbuf, "%s||||--", ANSISAFE( ansi, "\x1b[1;33m" ) );

                        if ( ch->pcdata->ooclimit == 3 )
                            sprintf( pbuf, "%s|||---", ANSISAFE( ansi, "\x1b[1;33m" ) );

                        if ( ch->pcdata->ooclimit == 2 )
                            sprintf( pbuf, "%s||----", ANSISAFE( ansi, "\x1b[1;31m" ) );

                        if ( ch->pcdata->ooclimit == 1 )
                            sprintf( pbuf, "%s|-----", ANSISAFE( ansi, "\x1b[1;31m" ) );

                        if ( ch->pcdata->ooclimit == 0 )
                            sprintf( pbuf, "%s------", ANSISAFE( ansi, "\x1b[1;31m" ) );

                        if ( ch->pcdata->oocbreak )
                            sprintf( pbuf, "%s--LB--", ANSISAFE( ansi, "\x1b[1;31m" ) );

                        break;

                    case 'I':
                        stat = ( IS_NPC( ch ) ? ( xIS_SET( ch->act, ACT_MOBINVIS ) ? ch->mobinvis : 0 )
                                 : ( xIS_SET( ch->act, PLR_WIZINVIS ) ? ch->pcdata->wizinvis : 0 ) );
                        break;
                }

                if ( stat != 0x80000000 )
                    sprintf( pbuf, "%d", stat );

                pbuf += strlen( pbuf );
                break;
        }
    }

    *pbuf = '\0';
    write_to_buffer( d, buf, ( pbuf - buf ) );
    return;
}

int make_color_sequence( const char* col, char* buf, DESCRIPTOR_DATA* d )
{
    int ln;
    const char* ctype = col;
    unsigned char cl;
    CHAR_DATA* och;
    bool ansi = TRUE;
    och = ( d->original ? d->original : d->character );

    if ( och != NULL )
        ansi = ( !IS_NPC( och ) && xIS_SET( och->act, PLR_ANSI ) );

    col++;

    if ( !*col )
        ln = -1;
    else if ( *ctype != '&' && *ctype != '^' )
    {
        bug( "Make_color_sequence: command '%c' not '&' or '^'.", *ctype );
        ln = -1;
    }
    else if ( *col == *ctype )
    {
        buf[0] = *col;
        buf[1] = '\0';
        ln = 1;
    }
    else if ( !ansi )
        ln = 0;
    else
    {
        cl = d->prevcolor;

        switch ( *ctype )
        {
            default:
                bug( "Make_color_sequence: bad command char '%c'.", *ctype );
                ln = -1;
                break;

            case '&':
                if ( *col == '-' )
                {
                    buf[0] = '~';
                    buf[1] = '\0';
                    ln = 1;
                    break;
                }

            case '^':
            {
                int newcol;

                if ( ( newcol = getcolor( *col ) ) < 0 )
                {
                    ln = 0;
                    break;
                }
                else if ( *ctype == '&' )
                    cl = ( cl & 0xF0 ) | newcol;
                else
                    cl = ( cl & 0x0F ) | ( newcol << 4 );
            }

            if ( cl == d->prevcolor )
            {
                ln = 0;
                break;
            }

            strcpy( buf, "\033[" );

            if ( ( cl & 0x88 ) != ( d->prevcolor & 0x88 ) )
            {
                strcat( buf, "m\033[" );

                if ( ( cl & 0x08 ) )
                    strcat( buf, "1;" );

                if ( ( cl & 0x80 ) )
                    strcat( buf, "5;" );

                d->prevcolor = 0x07 | ( cl & 0x88 );
                ln = strlen( buf );
            }
            else
                ln = 2;

            if ( ( cl & 0x07 ) != ( d->prevcolor & 0x07 ) )
            {
                sprintf( buf + ln, "3%d;", cl & 0x07 );
                ln += 3;
            }

            if ( ( cl & 0x70 ) != ( d->prevcolor & 0x70 ) )
            {
                sprintf( buf + ln, "4%d;", ( cl & 0x70 ) >> 4 );
                ln += 3;
            }

            if ( buf[ln - 1] == ';' )
                buf[ln - 1] = 'm';
            else
            {
                buf[ln++] = 'm';
                buf[ln] = '\0';
            }

            d->prevcolor = cl;
        }
    }

    if ( ln <= 0 )
        *buf = '\0';

    return ln;
}

void set_pager_input( DESCRIPTOR_DATA* d, char* argument )
{
    while ( isspace( *argument ) )
        argument++;

    d->pagecmd = *argument;
    return;
}

bool pager_output( DESCRIPTOR_DATA* d )
{
    register char* last;
    CHAR_DATA* ch;
    int pclines;
    register int lines;
    bool ret;

    if ( !d || !d->pagepoint || d->pagecmd == -1 )
        return TRUE;

    ch = d->original ? d->original : d->character;
    pclines = UMAX( ch->pcdata->pagerlen, 5 ) - 1;

    switch ( LOWER( d->pagecmd ) )
    {
        default:
            lines = 0;
            break;

        case 'b':
            lines = -1 - ( pclines * 2 );
            break;

        case 'r':
            lines = -1 - pclines;
            break;

        case 'q':
            d->pagetop = 0;
            d->pagepoint = NULL;
            flush_buffer( d, TRUE );
            DISPOSE( d->pagebuf );
            d->pagesize = MAX_STRING_LENGTH;
            return TRUE;
    }

    while ( lines < 0 && d->pagepoint >= d->pagebuf )
        if ( *( --d->pagepoint ) == '\n' )
            ++lines;

    if ( *d->pagepoint == '\n' && *( ++d->pagepoint ) == '\r' )
        ++d->pagepoint;

    if ( d->pagepoint < d->pagebuf )
        d->pagepoint = d->pagebuf;

    for ( lines = 0, last = d->pagepoint; lines < pclines; ++last )
        if ( !*last )
            break;
        else if ( *last == '\n' )
            ++lines;

    if ( *last == '\r' )
        ++last;

    if ( last != d->pagepoint )
    {
        if ( !write_to_descriptor( d->descriptor, d->pagepoint,
                                   ( last - d->pagepoint ) ) )
            return FALSE;

        d->pagepoint = last;
    }

    while ( isspace( *last ) )
        ++last;

    if ( !*last )
    {
        d->pagetop = 0;
        d->pagepoint = NULL;
        flush_buffer( d, TRUE );
        DISPOSE( d->pagebuf );
        d->pagesize = MAX_STRING_LENGTH;
        return TRUE;
    }

    d->pagecmd = -1;

    if ( xIS_SET( ch->act, PLR_ANSI ) )
        if ( write_to_descriptor( d->descriptor, "\033[1;36m", 7 ) == FALSE )
            return FALSE;

    if ( ( ret = write_to_descriptor( d->descriptor,
                                      "(C)ontinue, (R)efresh, (B)ack, (Q)uit: [C] ", 0 ) ) == FALSE )
        return FALSE;

    if ( xIS_SET( ch->act, PLR_ANSI ) )
    {
        char buf[32];

        if ( d->pagecolor == 7 )
            strcpy( buf, "\033[m" );
        else
            sprintf( buf, "\033[0;%d;%s%dm", ( d->pagecolor & 8 ) == 8,
                     ( d->pagecolor > 15 ? "5;" : "" ), ( d->pagecolor & 7 ) + 30 );

        ret = write_to_descriptor( d->descriptor, buf, 0 );
    }

    return ret;
}

void write_menu_to_desc( DESCRIPTOR_DATA* d )
{
    send_to_buffer( "&GWelcome to -{ Aliens vs. Predator: Legend }-\n\r", d );

    if ( !d->character || d->character->last_cmd == NULL )
    {
        send_to_buffer( "&C0&z) &WExit the mud.\n\r", d );
        send_to_buffer( "&C1&z) &WEnter the game.\n\r", d );
    }
    else
    {
        send_to_buffer( "&C1&z) &WReturn to the game.\n\r", d );
    }

    send_to_buffer( "&C2&z) &WView character score.\n\r", d );
    send_to_buffer( "&C3&z) &WPurchase Levels.\n\r", d );
    send_to_buffer( "&C4&z) &WPurchase Skills.\n\r", d );
    send_to_buffer( "&C5&z) &WPurchase Attributes.\n\r", d );
    send_to_buffer( "&C6&z) &WChange your password.\n\r", d );

    if ( !d->character || d->character->last_cmd == NULL )
        send_to_buffer( "&C7&z) &WDelete this character.\n\r", d );

    send_to_buffer( "&zChoice&W) ", d );
    return;
}

void do_menu( CHAR_DATA* ch, char* argument )
{
    send_to_char( "&w\n\r", ch );
    write_menu_to_desc( ch->desc );
    ch->desc->connected = CON_MENU_BASE;
    return;
}

/*
    New COPYOVER System by Ghost
*/
void do_copyover ( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_STRING_LENGTH];
    FILE* fp;
    DESCRIPTOR_DATA* d;
    bool nosave = FALSE;
    bool bypass = FALSE;
    char buf [100], buf2[100], buf3[100], buf4[100], buf5[100];
    argument = one_argument( argument, arg );

    if ( NULLSTR( arg ) )
    {
        send_to_char( "\n\rUSAGE: copyover NOW     - Full Copyover", ch );
        send_to_char( "\n\r       copyover NOSAVE  - Only Stores descriptors", ch );
        send_to_char( "\n\r       copyover BYPASS  - Bypasses PC Safty checks", ch );
        send_to_char( "\n\r    ", ch );
        return;
    }

    if ( !str_cmp( arg, "nosave" ) )
    {
        nosave = TRUE;
    }
    else if ( !str_cmp( arg, "bypass" ) )
    {
        bypass = TRUE;
    }
    else if ( str_cmp( arg, "now" ) )
    {
        do_copyover( ch, "" );
        return;
    }

    fp = fopen ( COPYOVER_FILE, "w" );

    if ( !fp )
    {
        send_to_char ( "Copyover file not writeable, aborted.\n\r", ch );
        log_string ( "Could not write to copyover file!" );
        perror ( "do_copyover:fopen" );
        return;
    }

    /* Shutdown the web server */
    shutdown_web();
    mqtt_cleanup();
    /* Close the match log */
    match_log( "CONTROL;Match Interrupted by Manual Copyover." );
    close_match( );

    /* Safty checks execute here... */
    if ( !bypass )
    {
        send_to_char( "\n\r&RProcessing Safty checks....\n\r", ch );

        for ( d = first_descriptor; d ; d = d->next )
        {
            CHAR_DATA* och = CH ( d );

            if ( d->connected == CON_EDITING )
            {
                send_to_char( "&RSorry, A Player is in a writting buffer. Please Wait for them to finish.\n\r", ch );
                return;
            }

            if ( d->connected > CON_PLAYING )
            {
                send_to_char( "&RA Player is currently connecting, Please wait.\n\r", ch );

                if ( och && och != NULL )
                    ch_printf( ch, "&R  Player: &Y%s   &RState: &Y%d\n\r", och->name, d->connected );

                return;
            }
        }

        send_to_char( "\n\r&RSafty checks cleared... Continuing Copyover.\n\r", ch );
    }

    /* Save Data for retrival after Copyover =) */
    if ( !nosave )
    {
        // save_all_ships( );
    }

    sprintf ( buf, "\n\r *** COPYOVER by %s - please remain seated!\n\r", ch->name );

    /* For each playing descriptor, save its state */
    for ( d = first_descriptor; d ; d = d->next )
    {
        CHAR_DATA* och = CH ( d );

        if ( !och || !d->character || d->connected > CON_PLAYING ) /* drop those logging on */
        {
            write_to_descriptor ( d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0 );
            /* The close_socket seems to be crashing -Ghost */
            /* close_socket (d, FALSE); */
        }
        else
        {
            if ( !nosave )
            {
                do_savearea( ch, "" );   /* Auto-save their area =) */
            }

            fprintf ( fp, "%d %s %s\n", d->descriptor, och->name, d->host );
            save_char_obj ( och );
            write_to_descriptor ( d->descriptor, buf, 0 );
        }
    }

    fprintf ( fp, "-1\n" );
    fclose ( fp );
    /* Close reserve and other always-open files and release other resources */
    fclose ( fpReserve );
    fclose ( fpLOG );
    /* exec - descriptors are inherited */
    sprintf ( buf, "%d", port );
    sprintf ( buf2, "%d", control );
    sprintf ( buf3, "%d", control2 );
    /*
        close( control  );
        close( control2 );
    */
    execl ( EXE_FILE,  "avp", buf, "copyover", buf2, buf3, buf4, buf5, ( char* ) NULL );
    /* Failed - sucessful exec will not return */
    execl ( EXE2_FILE, "avp", buf, "copyover", buf2, buf3, buf4, buf5, ( char* ) NULL );
    /* Failed - sucessful exec will not return */
    perror ( "do_copyover: execl" );
    send_to_char ( "Copyover FAILED!\n\r", ch );

    /* Here you might want to reopen fpReserve */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }

    if ( ( fpLOG = fopen( NULL_FILE, "r" ) ) == NULL )
    {
        perror( NULL_FILE );
        exit( 1 );
    }
}

/* Recover from a copyover - load players */
void copyover_recover ()
{
    DESCRIPTOR_DATA* d;
    FILE* fp;
    char name [100];
    char host[MAX_STRING_LENGTH];
    int desc;
    bool fOld;
    log_string ( "----*> Warmboot recovery initiated <*----" );
    fp = fopen ( COPYOVER_FILE, "r" );

    if ( !fp ) /* there are some descriptors open which will hang forever then ? */
    {
        perror ( "copyover_recover:fopen" );
        log_string( "Copyover file not found. Exitting.\n\r" );
        exit ( 1 );
    }

    unlink ( COPYOVER_FILE ); /* In case something crashes - doesn't prevent reading  */

    for ( ;; )
    {
        fscanf ( fp, "%d %s %s\n", &desc, name, host );

        if ( desc == -1 )
            break;

        /* Write something, and check if it goes error-free */
        if ( !write_to_descriptor ( desc, "Restoring from copyover...\n\r", 0 ) )
        {
            close ( desc ); /* nope */
            continue;
        }

        CREATE( d, DESCRIPTOR_DATA, 1 );
        init_descriptor ( d, desc );
        d->host = STRALLOC( host );
        LINK( d, first_descriptor, last_descriptor, next, prev );
        d->connected = CON_COPYOVER_RECOVER;
        /* Now, find the pfile */
        fOld = load_char_obj ( d, name, FALSE );

        if ( !fOld ) /* Player file not found?! */
        {
            write_to_descriptor ( desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0 );
            close_socket ( d, TRUE );
        }
        else /* ok! */
        {
            write_to_descriptor ( desc, "\n\rCopyover recovery complete.\n\r", 0 );

            /* Just In Case */
            if ( !d->character->in_room )
                d->character->in_room = get_room_index ( ROOM_VNUM_TEMPLE );

            /* Insert in the char_list */
            LINK( d->character, first_char, last_char, next, prev );
            num_descriptors++;
            sysdata.maxplayers++;
            char_to_room ( d->character, d->character->in_room );

            /* Auto-Restore Player areas */
            if ( d->character->pcdata->area )
                do_loadarea ( d->character, "" );

            // do_look (d->character, "auto noprog");
            // act( AT_BLOOD, "$n appears from a swirling cloud of mist!", d->character, NULL, NULL, TO_ROOM );
            d->connected = CON_PLAYING;
        }
    }

    log_string( "<>---> Warmboot complete <---<>" );
    fclose ( fp );
    /* Trip the arena load */
    update_arena();
}

#ifdef WIN32

void shutdown_mud( char* reason );

void bailout( void )
{
    echo_to_all( AT_IMMORT, "MUD shutting down by system operator NOW!!", ECHOTAR_ALL );
    shutdown_mud( "MUD shutdown by system operator" );
    log_string ( "MUD shutdown by system operator" );
    Sleep ( 5000 );             /* give "echo_to_all" time to display */
    mud_down = TRUE;            /* This will cause game_loop to exit */
    service_shut_down = TRUE;   /* This will cause characters to be saved */
    fflush( stderr );
    return;
}

#endif

bool close_match( void )
{
    if ( fpMatch == NULL )
        return FALSE;

    match_log( "CONTROL;Match Log is now closed." );
    fclose( fpMatch );
    fpMatch = NULL;
    return TRUE;
}

bool open_match( void )
{
    char tmp[MIL];
    char filename[MSL];
    time_t t = time( 0 );

    if ( fpMatch != NULL )
        return FALSE;

    // Create Filename MMDDYY-HHMMSS
    strftime( tmp, MIL, "%m%d%Y%H%M%S", localtime( &t ) );
    snprintf( filename, MSL, "../report/%s.log", tmp );

    if ( ( fpMatch = fopen( filename, "w" ) ) == NULL )
        return FALSE;

    match_log( "CONTROL;New match has started." );
    return TRUE;
}

bool match_log( const char* str, ... )
{
    char mlog[MSL];
    char* strtime;
    char* buf;

    if ( fpMatch == NULL )
        return FALSE;

    strtime  = ctime( &current_time );
    strtime[strlen( strtime ) - 1] = '\0';
    buf = ( char* )calloc( sizeof( char ), 256 );
    {
        va_list param;
        va_start( param, str );
        vsprintf( buf + strlen( buf ), str, param );
        va_end( param );
    }
    snprintf( mlog, MSL, "%s;%s\n", strtime, buf );
    fprintf( fpMatch, "%s", mlog );
    free( buf );
    return TRUE;
}
