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
                              Web Server Module
****************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

#define MAXDATA  1024

typedef struct web_descriptor WEB_DESCRIPTOR;

struct web_descriptor
{
    int fd;
    char request[MAXDATA * 2];
    struct sockaddr_in their_addr;
    int sin_size;
    WEB_DESCRIPTOR* next;
    bool valid;
};

WEB_DESCRIPTOR* web_desc_free;

/* FUNCTION DEFS */
int send_buf( int fd, const char* buf );
void handle_web_request( WEB_DESCRIPTOR* wdesc );
void handle_web_who_request( WEB_DESCRIPTOR* wdesc );
void handle_web_help_request( WEB_DESCRIPTOR* wdesc, int hmin, int hmax );
bool check_help_net args( ( WEB_DESCRIPTOR* wdesc, int hmin, int hmax ) );
WEB_DESCRIPTOR* new_web_desc( void );
void free_web_desc( WEB_DESCRIPTOR* desc );

bool  webserve = FALSE;

/* The mark of the end of a HTTP/1.x request */
const char ENDREQUEST[5] = { 13, 10, 13, 10, 0 }; /* (CRLFCRLF) */

/* Externs */
// extern int top_web_desc;

/* Locals */
WEB_DESCRIPTOR* web_descs;
int portid = 8000;
int sockfd;

/*
    Initilize the Web Server
*/
void init_web( int wport )
{
    char log_buf[MAX_STRING_LENGTH];
    struct sockaddr_in my_addr;
    web_descs = NULL;
    sprintf( log_buf, "Web features starting on port: %d", wport );
    log_string( log_buf );

    if ( ( sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
    {
        perror( "web-socket" );
        // exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons( wport );
    my_addr.sin_addr.s_addr = htons( INADDR_ANY );
    bzero( my_addr.sin_zero, 8 );

    if ( ( bind( sockfd, ( struct sockaddr* )&my_addr, sizeof( struct sockaddr ) ) ) == -1 )
    {
        perror( "web-bind" );
        // exit(1);
    }

    /* Set the active flags */
    webserve = TRUE;
    portid = wport;
    /* Only listen for 5 connections at a time. */
    listen( sockfd, 5 );
}

struct timeval ZERO_TIME = { 0, 0 };

/*
    Handle web connections
*/
void handle_web( void )
{
    int max_fd;
    WEB_DESCRIPTOR* current, *prev, *next;
    fd_set readfds;

    if ( !webserve )
        return;

    FD_ZERO( &readfds );
    FD_SET( sockfd, &readfds );
    /* it *will* be atleast sockfd */
    max_fd = sockfd;

    /* add in all the current web descriptors */
    for ( current = web_descs; current != NULL; current = current->next )
    {
        FD_SET( current->fd, &readfds );

        if ( max_fd < current->fd )
            max_fd = current->fd;
    }

    /* Wait for ONE descriptor to have activity */
    select( max_fd + 1, &readfds, NULL, NULL, &ZERO_TIME );

    if ( FD_ISSET( sockfd, &readfds ) )
    {
        /* NEW CONNECTION -- INIT & ADD TO LIST */
        current = new_web_desc();
        current->sin_size = sizeof( struct sockaddr_in );
        current->request[0] = '\0';

        if ( ( current->fd = accept( sockfd, ( struct sockaddr* ) & ( current->their_addr ), &( current->sin_size ) ) ) == -1 )
        {
            perror( "web-accept" );
            // exit(1);
        }

        current->next = web_descs;
        web_descs = current;
    }

    /* DATA IN! */
    for ( current = web_descs; current != NULL; current = current->next )
    {
        if ( FD_ISSET( current->fd, &readfds ) ) /* We Got Data! */
        {
            char buf[MAXDATA];
            int numbytes;

            if ( ( numbytes = read( current->fd, buf, sizeof( buf ) ) ) == -1 )
            {
                perror( "web-read" );
                // exit(1);
            }

            buf[numbytes] = '\0';
            strcat( current->request, buf );
        }
    } /* DONE WITH DATA IN */

    /* DATA OUT */
    for ( current = web_descs; current != NULL; current = next )
    {
        next = current->next;

        if ( strstr( current->request, "HTTP/1." ) /* 1.x request (vernum on FIRST LINE) */
                && strstr( current->request, ENDREQUEST ) )
            handle_web_request( current );
        else if ( !strstr( current->request, "HTTP/1." )
                  &&  strchr( current->request, '\n' ) ) /* HTTP/0.9 (no ver number) */
            handle_web_request( current );
        else
        {
            continue; /* Don't have full request yet! */
        }

        close( current->fd );

        if ( web_descs == current )
        {
            web_descs = current->next;
        }
        else
        {
            /* Just ititerate through the list */
            for ( prev = web_descs; prev->next != current; prev = prev->next );

            prev->next = current->next;
        }

        free_web_desc( current );
    }
}

/* Generic Utility Function */
int send_buf( int fd, const char* buf )
{
    return send( fd, buf, strlen( buf ), 0 );
}

/* These are memory management... they should move to recycle.c soon */
WEB_DESCRIPTOR* new_web_desc( void )
{
    WEB_DESCRIPTOR* desc;

    if ( web_desc_free == NULL )
    {
        // desc = alloc_perm(sizeof(*desc));
        CREATE( desc, WEB_DESCRIPTOR, 1 );
    }
    else
    {
        desc = web_desc_free;
        web_desc_free = web_desc_free->next;
    }

    // VALIDATE(desc);
    desc->valid = TRUE;
    return desc;
}

void free_web_desc( WEB_DESCRIPTOR* desc )
{
    if ( desc->valid )
        return;

    desc->valid = FALSE;
    // INVALIDATE(desc);
    DISPOSE( desc );
    desc->next = web_desc_free;
    web_desc_free = desc;
}


/*
    Disables the Web service
*/
void shutdown_web ( void )
{
    WEB_DESCRIPTOR* current, *next;

    if ( !webserve )
    {
        log_string( "Invalid Procedure [shutdown_web]: Web service not active." );
        return;
    }

    log_string( "Web features shutting down." );

    /* Close All Current Connections */
    for ( current = web_descs; current != NULL; current = next )
    {
        next = current->next;
        close( current->fd );
        free_web_desc( current );
    }

    /* Stop Listening */
    close( sockfd );
    /* Clear running flag */
    webserve = FALSE;
}

/*
    Reports information on the Web service.
*/
void do_webserve( CHAR_DATA* ch, char* argument )
{
    char buf[MSL], arg1[MIL], arg2[MIL];
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        ch_printf( ch, "&zWebserver Information &C----------&z\n\r" );
        ch_printf( ch, "&zStatus: %s&z\n\r", webserve ? "&ROnline" : "&COffline" );
        ch_printf( ch, "&zSocket ID: &C%d  &z[Port: &C%d&z]\n\r", sockfd ? sockfd : 0, portid ? portid : 0 );
        ch_printf( ch, "\n\r" );
        ch_printf( ch, "&zSyntax: WEBSERVE [START] <port>\n\r" );
        ch_printf( ch, "&z        WEBSERVE [STOP]\n\r" );
        return;
    }

    if ( !str_cmp( arg1, "start" ) )
    {
        if ( atoi( arg2 ) < 1 || atoi( arg2 ) > 9999 )
        {
            ch_printf( ch, "Invalid port number: %d\n\r", atoi( arg2 ) );
            return;
        }

        if ( webserve )
        {
            ch_printf( ch, "Already Running the Web service.\n\r" );
            return;
        }

        log_string( "PROCEDURE LOGGED: Booting internal web service." );
        init_web( atoi( arg2 ) );
    }
    else if ( !str_cmp( arg1, "stop" ) )
    {
        if ( !webserve )
        {
            ch_printf( ch, "The web service is not currently running.\n\r" );
            return;
        }

        log_string( "PROCEDURE LOGGED: Disabling internal web service." );
        shutdown_web();
    }
    else
        do_webserve( ch, "" );
}

/*
    Handles web requests
*/
void handle_web_request( WEB_DESCRIPTOR* wdesc )
{
    /* +-= PROCESS REQUEST =-+ */
    /* are we using HTTP/1.x? If so, write out header stuff.. */
    if ( !strstr( wdesc->request, "GET" ) )
    {
        send_buf( wdesc->fd, "HTTP/1.0 501 Not Implemented" );
        return;
    }
    else if ( strstr( wdesc->request, "HTTP/1." ) )
    {
        send_buf( wdesc->fd, "HTTP/1.0 200 OK\n" );
        send_buf( wdesc->fd, "Content-type: text/html\n\n" );
    }

    /* Handle the actual request */
    if ( strstr( wdesc->request, "/default.htm " ) )
    {
        // handle_web_homepage(wdesc);
        send_buf( wdesc->fd, "Welcome to the AVP Dynamic Homepage!" );
        return;
    }

    if ( strstr( wdesc->request, "/wholist.htm " ) )
    {
        handle_web_who_request( wdesc );
        return;
    }

    if ( strstr( wdesc->request, "/help.htm " ) )
    {
        handle_web_help_request( wdesc, -1, 100 );
        return;
    }

    if ( strstr( wdesc->request, "/immhelp.htm " ) )
    {
        handle_web_help_request( wdesc, -2, 200 );
        return;
    }

    if ( strstr( wdesc->request, "/help/" ) )
    {
        if ( check_help_net( wdesc, -1, 100 ) )
            return;

        send_buf( wdesc->fd, "AVP Web help: INVALID HELP FILE" );
        return;
    }

    if ( strstr( wdesc->request, "/~immhelp/" ) )
    {
        if ( check_help_net( wdesc, -2, 200 ) )
            return;

        send_buf( wdesc->fd, "AVP Web help: ILLEGAL ACCESS ATTEMPT" );
        return;
    }

    if ( strstr( wdesc->request, "/~delete_help/" ) )
    {
        if ( check_help_net( wdesc, -2, 200 ) )
            return;

        send_buf( wdesc->fd, "AVP Web help: ILLEGAL ACCESS ATTEMPT" );
        return;
    }

    // log_string("DYNAMIC WEB FAULT: 404 file not found");
    send_buf( wdesc->fd, "ALERT: Error 404<br><br><br><br>" );
    send_buf( wdesc->fd, wdesc->request );
    // handle_web_404(wdesc);
    return;
}

/*
    ####################################################
     Web HELP Request (Or not)
    ####################################################
*/
bool check_help_net( WEB_DESCRIPTOR* wdesc, int hmin, int hmax )
{
    HELP_DATA* help;
    char buf[MSL], buf2[SUB_MSL], buf3[MIL], buf4[MSL], buf5[MSL];

    for ( help = first_help; help; help = help->next )
    {
        if ( help->level >= hmax || help->level <= hmin )
            continue;

        snprintf( buf4, MSL, "/~delete_help/%s.htm ", convert_sp( strlower( help->keyword ) ) );
        one_argument( help->keyword, buf3 );
        snprintf( buf5, MSL, "/~delete_help/%s.htm ", strlower( buf3 ) );

        if ( help->level >= 100 || hmax > 100 )
        {
            snprintf( buf, MSL, "/~immhelp/%s.htm ", convert_sp( strlower( help->keyword ) ) );
            one_argument( help->keyword, buf3 );
            snprintf( buf2, SUB_MSL, "/~immhelp/%s.htm ", strlower( buf3 ) );
        }
        else
        {
            snprintf( buf, MSL, "/help/%s.htm ", convert_sp( strlower( help->keyword ) ) );
            one_argument( help->keyword, buf3 );
            snprintf( buf2, SUB_MSL, "/help/%s.htm ", strlower( buf3 ) );
        }

        if ( strstr( wdesc->request, buf ) || strstr( wdesc->request, buf2 ) )
        {
            // handle_web_help(wdesc);
            send_buf( wdesc->fd, "<html>\n" );
            send_buf( wdesc->fd, "<head>\n" );
            send_buf( wdesc->fd, "<title>Legends of the Jedi - Online Help</title>\n" );
            send_buf( wdesc->fd, "</head>\n" );
            send_buf( wdesc->fd, "<BODY TEXT=""#C0C0C0"" BGCOLOR=""#000000"" LINK=""#00FFFF""\n" );
            send_buf( wdesc->fd, "VLINK=""#FFFFFF"" ALINK=""#008080"">\n" );
            send_buf( wdesc->fd, "<FONT FACE=""courier"">\n" );

            if ( help->level >= 100 )
                send_buf( wdesc->fd, "AVP Immortal-Only Web help: " );

            if ( help->level <  100 )
                send_buf( wdesc->fd, "AVP Web help: " );

            send_buf( wdesc->fd, help->keyword );
            send_buf( wdesc->fd, "<br><br>\n" );
            send_buf( wdesc->fd, conv_tag( conv_hcolor( help->text ) ) );

            if ( hmax > 100 )
            {
                snprintf( buf2, SUB_MSL, "/~delete_help/%s.htm ", convert_sp( strlower( help->keyword ) ) );
                snprintf( buf, MSL, "<br><a href=""%s"">[Delete this helpfile]</a><br>\n", buf2 );
                send_buf( wdesc->fd, buf );
            }

            send_buf( wdesc->fd, "</font>\n" );
            send_buf( wdesc->fd, "</body>\n" );
            return TRUE;
        }
        else if ( strstr( wdesc->request, buf4 ) || strstr( wdesc->request, buf5 ) )
        {
            UNLINK( help, first_help, last_help, next, prev );
            STRFREE( help->text );
            STRFREE( help->keyword );
            DISPOSE( help );
            handle_web_help_request( wdesc, -2, 200 );
            return TRUE;
        }
    }

    return FALSE;
}

/*
    ####################################################
     Web HELP LISTING Request
    ####################################################
*/
void handle_web_help_request( WEB_DESCRIPTOR* wdesc, int hmin, int hmax )
{
    HELP_DATA* help;
    char buf[MSL], buf2[SUB_MSL];
    char tmp = ' ', chk = ' ';
    int cnt = 0, inqt = 0;
    send_buf( wdesc->fd, "<html>\n" );
    send_buf( wdesc->fd, "<head>\n" );
    send_buf( wdesc->fd, "<title>Legends of the Jedi - Help Listing</title>\n" );
    send_buf( wdesc->fd, "</head>\n" );
    send_buf( wdesc->fd, "<BODY TEXT=\"#C0C0C0\" BGCOLOR=\"#000000\" LINK=\"#00FFFF\"\n" );
    send_buf( wdesc->fd, "VLINK=\"#FFFFFF\" ALINK=\"#008080\">\n" );
    send_buf( wdesc->fd, "<h1><center>Summary of AVP Help Topics</center></h1>\n" );
    send_buf( wdesc->fd, "<b><center><font size=\"2\">\n" );
    send_buf( wdesc->fd, "<br><hr color=\"#FFFFFF\"><br>\n" );

    for ( help = first_help; help; help = help->next )
    {
        if ( help->level >= hmax || help->level <= hmin )
            continue;

        chk = help->keyword[0];

        if ( help->keyword[0] == '!' )
        {
            continue; // Skip the ! Command
        }
        else if ( help->keyword[0] == '\'' || help->keyword[0] == '"' )
        {
            chk = help->keyword[1];

            if ( help->keyword[0] == '"' )
                inqt = 1;
        }
        else
        {
            if ( inqt == 1 )
                send_buf( wdesc->fd, "<br><hr color=\"#FFFFFF\"><br>\n" );

            inqt = 0;
        }

        if ( tmp != chk )
        {
            tmp = chk;

            if ( inqt == 1 )
                sprintf( buf, "<br></font><font size=\"4\">'%c'</font><font size=\"2\"><br>\n", tmp );

            if ( inqt == 0 )
                sprintf( buf, "<br></font><font size=\"4\">%c</font><font size=\"2\"><br>\n", tmp );

            send_buf( wdesc->fd, buf );
        }

        if ( help->level >= 100 )
        {
            snprintf( buf2, SUB_MSL, "/~immhelp/%s.htm ", convert_sp( strlower( help->keyword ) ) );
            snprintf( buf, MSL, "<a href=\"%s\">%s *</a><br>\n", buf2, help->keyword );
        }
        else
        {
            if ( hmax > 100 )
            {
                snprintf( buf2, SUB_MSL, "/~immhelp/%s.htm ", convert_sp( strlower( help->keyword ) ) );
                snprintf( buf, MSL, "<a href=\"%s\">%s</a><br>\n", buf2, help->keyword );
            }
            else
            {
                snprintf( buf2, SUB_MSL, "/help/%s.htm ", convert_sp( strlower( help->keyword ) ) );
                snprintf( buf, MSL, "<a href=\"%s\">%s</a><br>\n", buf2, help->keyword );
            }
        }

        send_buf( wdesc->fd, buf );
        cnt++;
    }

    send_buf( wdesc->fd, "<br><br><hr color=\"#FFFFFF\"><br>\n" );
    send_buf( wdesc->fd, "<font face=\"Times New Roman\">\n" );

    if ( cnt > 0 )
    {
        if ( hmax >= 100 )
            send_buf( wdesc->fd, "Links marked with a '*' means they are over level 100.<br>\n" );

        sprintf( buf, "-There are [ %d ] help files currently on AVP-<br>\n", cnt );
        send_buf( wdesc->fd, buf );
    }
    else
        send_buf( wdesc->fd, "-There are no help files on AVP right now-<br>\n" );

    snprintf( buf, MSL, "<br>This file last updated at %s Eastern Time.\n", ( ( char* ) ctime( &current_time ) ) );
    send_buf( wdesc->fd, buf );
    send_buf( wdesc->fd, "</center></font>\n" );
    send_buf( wdesc->fd, "</body>\n" );
    send_buf( wdesc->fd, "</html>\n" );
    return;
}

/*
    ####################################################
     Web WHO Request
    ####################################################
*/
void handle_web_who_request( WEB_DESCRIPTOR* wdesc )
{
    DESCRIPTOR_DATA* d;
    char buf[MIL], buf2[MIL];
    int ppl = 0;
    buf[0] = '\0';
    buf2[0] = '\0';
    send_buf( wdesc->fd, "<html>\n" );
    send_buf( wdesc->fd, "<head>\n" );
    send_buf( wdesc->fd, "<title>Legends of the Jedi - Who list</title>\n" );
    send_buf( wdesc->fd, "</head>\n" );
    send_buf( wdesc->fd, "<BODY TEXT=\"#C0C0C0\" BGCOLOR=\"#000000\" LINK=\"#00FFFF\"\n" );
    send_buf( wdesc->fd, "VLINK=\"#FFFFFF\" ALINK=\"#008080\">\n" );
    send_buf( wdesc->fd, "<h1><center>Who's on AVP?</center></h1>\n" );
    send_buf( wdesc->fd, "<b><center><font size=\"2\">\n" );

    for ( d = first_descriptor; d; d = d->next )
        if ( !IS_IMMORTAL( ( d->original != NULL ) ? d->original : d->character ) )
            ppl++;

    if ( ppl > 0 )
    {
        sprintf( buf, "-There are [ %d ] people currently on AVP-<br>\n", ppl );
        send_buf( wdesc->fd, buf );
        send_buf( wdesc->fd, "<br><hr color=\"#FFFFFF\"><br>\n" );
    }

    for ( d = first_descriptor; d; d = d->next )
    {
        CHAR_DATA* wch;

        if ( d->connected != CON_PLAYING && d->connected != CON_EDITING )
            continue;

        wch   = ( d->original != NULL ) ? d->original : d->character;

        if ( !IS_IMMORTAL( wch ) )
        {
            sprintf( buf, "--==|    %s    |==--<br>\n", conv_hcolor( wch->pcdata->title ) );
            send_buf( wdesc->fd, buf );
        }
    }

    send_buf( wdesc->fd, "<br><br><hr color=\"#FFFFFF\"><br>\n" );

    for ( d = first_descriptor; d; d = d->next )
    {
        CHAR_DATA* wch;

        if ( d->connected != CON_PLAYING && d->connected != CON_EDITING )
            continue;

        wch   = ( d->original != NULL ) ? d->original : d->character;

        if ( !IS_IMMORTAL( wch ) || !xIS_SET( wch->pcdata->flags, PCFLAG_NOWHO ) )
        {
            if ( !nifty_is_name( wch->name, wch->pcdata->title ) )
                sprintf( buf2, "%s - %s", wch->name, wch->pcdata->title );
            else
                sprintf( buf2, "%s", wch->pcdata->title );

            sprintf( buf, "PLAYER: %s <font color= #C0C0C0><br>\n", conv_hcolor( buf2 ) );
            send_buf( wdesc->fd, buf );
            sprintf( buf, "RACE: [%s%s<font color= #C0C0C0>]<br>\n", conv_hcolor( "&B" ), get_race( wch ) );
            send_buf( wdesc->fd, buf );
            sprintf( buf, "LEVEL: [%s%d<font color= #C0C0C0>]<br>\n", conv_hcolor( "&B" ), wch->top_level );
            send_buf( wdesc->fd, buf );

            if ( wch->pcdata->bio[0] != '\0' )
            {
                sprintf( buf, "BIO: %s<font color= #C0C0C0>\n", conv_hcolor( wch->pcdata->bio ) );
                send_buf( wdesc->fd, buf );
            }

            send_buf( wdesc->fd, "<br><br><hr color=\"#FFFFFF\"><br>\n" );
        }
    }

    send_buf( wdesc->fd, "<font face=\"Times New Roman\">\n" );

    if ( ppl > 0 )
    {
        sprintf( buf, "-There are [ %d ] people currently on AVP-<br>\n", ppl );
        send_buf( wdesc->fd, buf );
    }
    else
        send_buf( wdesc->fd, "-There is nobody currently connected to AVP-<br>\n" );

    sprintf( buf, "<br>This file last updated at %s Eastern Time.\n", ( ( char* ) ctime( &current_time ) ) );
    send_buf( wdesc->fd, buf );
    send_buf( wdesc->fd, "</center></font>\n" );
    send_buf( wdesc->fd, "</body>\n" );
    send_buf( wdesc->fd, "</html>\n" );
    return;
}
