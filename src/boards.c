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
                 Special boards module
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"
#ifdef USE_IMC
    #include "imc-mercbase.h"
#endif

/* Defines for voting on notes. -- Narn */
#define VOTE_NONE 0
#define VOTE_OPEN 1
#define VOTE_CLOSED 2

BOARD_DATA*         first_board;
BOARD_DATA*         last_board;

bool    is_note_to  args( ( CHAR_DATA* ch, NOTE_DATA* pnote ) );
void    note_attach args( ( CHAR_DATA* ch ) );
void    note_remove args( ( CHAR_DATA* ch, BOARD_DATA* board,
                            NOTE_DATA* pnote ) );
void    do_note     args( ( CHAR_DATA* ch, char* arg_passed, bool IS_MAIL ) );



bool can_remove( CHAR_DATA* ch, BOARD_DATA* board )
{
    /* If your trust is high enough, you can remove it. */
    if ( get_trust( ch ) >= board->min_remove_level )
        return TRUE;

    if ( board->extra_removers[0] != '\0' )
    {
        if ( is_name( ch->name, board->extra_removers ) )
            return TRUE;
    }

    return FALSE;
}

bool can_read( CHAR_DATA* ch, BOARD_DATA* board )
{
    /* If your trust is high enough, you can read it. */
    if ( get_trust( ch ) >= board->min_read_level )
        return TRUE;

    /*  Your trust wasn't high enough, so check if a read_group or extra
        readers have been set up. */
    if ( board->extra_readers[0] != '\0' )
    {
        if ( is_name( ch->name, board->extra_readers ) )
            return TRUE;
    }

    return FALSE;
}

bool can_post( CHAR_DATA* ch, BOARD_DATA* board )
{
    /* If your trust is high enough, you can post. */
    if ( get_trust( ch ) >= board->min_post_level )
        return TRUE;

    return FALSE;
}


/*
    board commands.
*/
void write_boards_txt( )
{
    BOARD_DATA* tboard;
    FILE* fpout;
    char filename[256];
    sprintf( filename, "%s%s", BOARD_DIR, BOARD_FILE );
    fpout = fopen( filename, "w" );

    if ( !fpout )
    {
        bug( "FATAL: cannot open board.txt for writing!\n\r", 0 );
        return;
    }

    for ( tboard = first_board; tboard; tboard = tboard->next )
    {
        fprintf( fpout, "Filename          %s~\n", tboard->note_file        );
        fprintf( fpout, "Vnum              %d\n",  tboard->board_obj        );
        fprintf( fpout, "Min_read_level    %d\n",  tboard->min_read_level   );
        fprintf( fpout, "Min_post_level    %d\n",  tboard->min_post_level   );
        fprintf( fpout, "Min_remove_level  %d\n",  tboard->min_remove_level );
        fprintf( fpout, "Max_posts         %d\n",  tboard->max_posts        );
        fprintf( fpout, "Type 	           %d\n",  tboard->type          );
        fprintf( fpout, "Read_group        %s~\n", tboard->read_group       );
        fprintf( fpout, "Post_group        %s~\n", tboard->post_group       );
        fprintf( fpout, "Extra_readers     %s~\n", tboard->extra_readers    );
        fprintf( fpout, "Extra_removers    %s~\n", tboard->extra_removers    );
        fprintf( fpout, "End\n" );
    }

    fclose( fpout );
}

BOARD_DATA* get_board( OBJ_DATA* obj )
{
    BOARD_DATA* board;

    for ( board = first_board; board; board = board->next )
        if ( board->board_obj == obj->pIndexData->vnum )
            return board;

    return NULL;
}

BOARD_DATA* find_board( CHAR_DATA* ch )
{
    OBJ_DATA* obj;
    BOARD_DATA*  board;

    for ( obj = ch->in_room->first_content;
            obj;
            obj = obj->next_content )
    {
        if ( ( board = get_board( obj ) ) != NULL )
            return board;
    }

    return NULL;
}


bool is_note_to( CHAR_DATA* ch, NOTE_DATA* pnote )
{
    if ( !str_cmp( ch->name, pnote->sender ) )
        return TRUE;

    if ( is_name( "all", pnote->to_list ) )
        return TRUE;

    if ( IS_HERO( ch ) && is_name( "immortal", pnote->to_list ) )
        return TRUE;

    if ( is_name( ch->name, pnote->to_list ) )
        return TRUE;

    return FALSE;
}



void note_attach( CHAR_DATA* ch )
{
    NOTE_DATA* pnote;

    if ( ch->pnote )
        return;

    CREATE( pnote, NOTE_DATA, 1 );
    pnote->next     = NULL;
    pnote->prev     = NULL;
    pnote->sender   = QUICKLINK( ch->name );
    pnote->date     = STRALLOC( "" );
    pnote->to_list  = STRALLOC( "" );
    pnote->subject  = STRALLOC( "" );
    pnote->text     = STRALLOC( "" );
    ch->pnote       = pnote;
    return;
}

void write_board( BOARD_DATA* board )
{
    FILE* fp;
    char filename[256];
    NOTE_DATA* pnote;
    /*
        Rewrite entire list.
    */
    fclose( fpReserve );
    sprintf( filename, "%s%s", BOARD_DIR, board->note_file );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        perror( filename );
    }
    else
    {
        for ( pnote = board->first_note; pnote; pnote = pnote->next )
        {
            fprintf( fp, "Sender  %s~\nDate    %s~\nTo      %s~\nSubject %s~\nVoting %d\nYesvotes %s~\nNovotes %s~\nAbstentions %s~\nText\n%s~\n\n",
                     pnote->sender,
                     pnote->date,
                     pnote->to_list,
                     pnote->subject,
                     pnote->voting,
                     pnote->yesvotes,
                     pnote->novotes,
                     pnote->abstentions,
                     pnote->text
                   );
        }

        fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


void free_note( NOTE_DATA* pnote )
{
    STRFREE( pnote->text    );
    STRFREE( pnote->subject );
    STRFREE( pnote->to_list );
    STRFREE( pnote->date    );
    STRFREE( pnote->sender  );

    if ( pnote->yesvotes )
        DISPOSE( pnote->yesvotes );

    if ( pnote->novotes )
        DISPOSE( pnote->novotes );

    if ( pnote->abstentions )
        DISPOSE( pnote->abstentions );

    DISPOSE( pnote );
}

void note_remove( CHAR_DATA* ch, BOARD_DATA* board, NOTE_DATA* pnote )
{
    if ( !board )
    {
        bug( "note remove: null board", 0 );
        return;
    }

    if ( !pnote )
    {
        bug( "note remove: null pnote", 0 );
        return;
    }

    /*
        Remove note from linked list.
    */
    UNLINK( pnote, board->first_note, board->last_note, next, prev );
    --board->num_posts;
    free_note( pnote );
    write_board( board );
}


void do_noteroom( CHAR_DATA* ch, char* argument )
{
    BOARD_DATA* board;
    char arg[MAX_STRING_LENGTH];
    char arg_passed[MAX_STRING_LENGTH];
    strcpy( arg_passed, argument );

    switch ( ch->substate )
    {
        case SUB_WRITING_NOTE:
            do_note( ch, arg_passed, FALSE );
            break;

        default:
            argument = one_argument( argument, arg );
            smash_tilde( argument );

            if ( !str_cmp( arg, "write" ) || !str_cmp( arg, "to" )
                    ||  !str_cmp( arg, "subject" ) || !str_cmp( arg, "show" ) )
            {
                do_note( ch, arg_passed, FALSE );
                return;
            }

            board = find_board( ch );

            if ( !board )
            {
                send_to_char( "There is no bulletin board here to look at.\n\r", ch );
                return;
            }

            if ( board->type != BOARD_NOTE )
            {
                send_to_char( "You can only use note commands on a message terminal.\n\r", ch );
                return;
            }
            else
            {
                do_note( ch, arg_passed, FALSE );
                return;
            }
    }
}

void do_mailroom( CHAR_DATA* ch, char* argument )
{
    BOARD_DATA* board;
    char arg[MAX_STRING_LENGTH];
    char arg_passed[MAX_STRING_LENGTH];
    strcpy( arg_passed, argument );

    switch ( ch->substate )
    {
        case SUB_WRITING_NOTE:
            do_note( ch, arg_passed, TRUE );
            break;

        default:
            argument = one_argument( argument, arg );
            smash_tilde( argument );

            if ( !str_cmp( arg, "write" ) || !str_cmp( arg, "to" )
                    ||  !str_cmp( arg, "subject" ) || !str_cmp( arg, "show" ) )
            {
                do_note( ch, arg_passed, TRUE );
                return;
            }

            board = find_board( ch );

            if ( !board )
            {
                send_to_char( "There is no mail facility here.\n\r", ch );
                return;
            }

            if ( board->type != BOARD_MAIL )
            {
                send_to_char( "You can only use mail commands in a post office.\n\r", ch );
                return;
            }
            else
            {
                do_note( ch, arg_passed, TRUE );
                return;
            }
    }
}

void do_note( CHAR_DATA* ch, char* arg_passed, bool IS_MAIL )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    NOTE_DATA*  pnote;
    BOARD_DATA* board;
    int vnum;
    int anum;
    int first_list;
    OBJ_DATA* quill, *paper, *tmpobj = NULL;
    EXTRA_DESCR_DATA* ed = NULL;
    char notebuf[MAX_STRING_LENGTH];
    char short_desc_buf[MAX_STRING_LENGTH];
    char long_desc_buf[MAX_STRING_LENGTH];
    char keyword_buf[MAX_STRING_LENGTH];
    bool mfound = FALSE;
    bool wasfound = FALSE;

    if ( IS_NPC( ch ) )
        return;

    if ( !ch->desc )
    {
        bug( "do_note: no descriptor", 0 );
        return;
    }

    switch ( ch->substate )
    {
        default:
            break;

        case SUB_WRITING_NOTE:
            if ( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL
                    ||     paper->item_type != ITEM_PAPER )
            {
                bug( "do_note: player not holding paper", 0 );
                stop_editing( ch );
                return;
            }

            ed = ch->dest_buf;
            STRFREE( ed->description );
            ed->description = copy_buffer( ch );
            stop_editing( ch );
            return;
    }

    set_char_color( AT_NOTE, ch );
    arg_passed = one_argument( arg_passed, arg );
    smash_tilde( arg_passed );

    if ( !str_cmp( arg, "list" ) )
    {
        board = find_board( ch );

        if ( !board )
        {
            send_to_char( "There is no board here to look at.\n\r", ch );
            return;
        }

        if ( !can_read( ch, board ) )
        {
            send_to_char( "You cannot make any sense of the cryptic scrawl on this board...\n\r", ch );
            return;
        }

        first_list = atoi( arg_passed );

        if ( first_list )
        {
            if ( IS_MAIL )
            {
                send_to_char( "You cannot use a list number (at this time) with mail.\n\r", ch );
                return;
            }

            if ( first_list < 1 )
            {
                send_to_char( "You can't read a message before 1!\n\r", ch );
                return;
            }
        }

        if ( !IS_MAIL )
        {
            vnum = 0;
            set_pager_color( AT_NOTE, ch );

            for ( pnote = board->first_note; pnote; pnote = pnote->next )
            {
                vnum++;

                if ( ( first_list && vnum >= first_list ) || !first_list )
                    pager_printf( ch, "&R%2d&z%c &W%-10s &z%c &c%-14s &z: &W%s\n\r",
                                  vnum,
                                  is_note_to( ch, pnote ) ? ')' : '}',
                                  pnote->sender,
                                  ( pnote->voting != VOTE_NONE ) ? ( pnote->voting == VOTE_OPEN ? 'V' : 'C' ) : ':',
                                  pnote->to_list,
                                  pnote->subject );
            }

            act( AT_ACTION, "$n glances over the messages.", ch, NULL, NULL, TO_ROOM );
            return;
        }
        else
        {
            vnum = 0;

            if ( IS_MAIL ) /* SB Mail check for Brit */
            {
                for ( pnote = board->first_note; pnote; pnote = pnote->next )
                    if ( is_note_to( ch, pnote ) )
                        mfound = TRUE;

                if ( !mfound && get_trust( ch ) < sysdata.read_all_mail )
                {
                    ch_printf( ch, "You have no mail.\n\r" );
                    return;
                }
            }

            for ( pnote = board->first_note; pnote; pnote = pnote->next )
                if ( is_note_to( ch, pnote ) || get_trust( ch ) > sysdata.read_all_mail )
                    ch_printf( ch, "&C%2d &z%c &c%s&z: &W%s\n\r",
                               ++vnum,
                               is_note_to( ch, pnote ) ? '-' : '}',
                               pnote->sender,
                               pnote->subject );

            return;
        }
    }

    if ( !str_cmp( arg, "read" ) )
    {
        bool fAll;
        board = find_board( ch );

        if ( !board )
        {
            send_to_char( "There is no board here to look at.\n\r", ch );
            return;
        }

        if ( !can_read( ch, board ) )
        {
            send_to_char( "You cannot make any sense of the cryptic scrawl on this board...\n\r", ch );
            return;
        }

        if ( !str_cmp( arg_passed, "all" ) )
        {
            fAll = TRUE;
            anum = 0;
        }
        else if ( is_number( arg_passed ) )
        {
            fAll = FALSE;
            anum = atoi( arg_passed );
        }
        else
        {
            send_to_char( "Note read which number?\n\r", ch );
            return;
        }

        set_pager_color( AT_NOTE, ch );

        if ( !IS_MAIL )
        {
            vnum = 0;

            for ( pnote = board->first_note; pnote; pnote = pnote->next )
            {
                vnum++;

                if ( vnum == anum || fAll )
                {
                    wasfound = TRUE;
                    pager_printf( ch, "&z[&R%3d&z] &W%s&z: &C%s\n\r&z%s\n\r&zTo: &C%s\n\r&G%s",
                                  vnum,
                                  pnote->sender,
                                  pnote->subject,
                                  pnote->date,
                                  pnote->to_list,
                                  pnote->text );

                    if ( pnote->yesvotes[0] != '\0' || pnote->novotes[0] != '\0'
                            || pnote->abstentions[0] != '\0' )
                    {
                        send_to_pager( "\n\r&z+-----------------------------------------------------------------+\n\r", ch );
                        pager_printf( ch, "&z| Votes:   %-54s |\n\r|&z Yes:     &c%-54s&z |\n\r|&z No:      &c%-54s&z |\n\r|&z Abstain: &c%-54s&z |\n\r",
                                      " ", pnote->yesvotes, pnote->novotes, pnote->abstentions );
                        send_to_pager( "&z+-----------------------------------------------------------------+\n\r", ch );
                    }

                    act( AT_ACTION, "$n reads a message.", ch, NULL, NULL, TO_ROOM );
                }
            }

            if ( !wasfound )
                ch_printf( ch, "No such message: %d\n\r", anum );

            return;
        }
        else
        {
            vnum = 0;

            for ( pnote = board->first_note; pnote; pnote = pnote->next )
            {
                if ( is_note_to( ch, pnote ) || get_trust( ch ) > sysdata.read_all_mail )
                {
                    vnum++;

                    if ( vnum == anum || fAll )
                    {
                        wasfound = TRUE;
                        pager_printf( ch, "&z[&R%3d&z] &W%s&z: &C%s\n\r&z%s\n\r&zTo: &C%s\n\r&G%s",
                                      vnum,
                                      pnote->sender,
                                      pnote->subject,
                                      pnote->date,
                                      pnote->to_list,
                                      pnote->text );
                    }
                }
            }

            if ( !wasfound )
                ch_printf( ch, "No such message: %d\n\r", anum );

            return;
        }
    }

    /* Voting added by Narn, June '96 */
    if ( !str_cmp( arg, "vote" ) )
    {
        char arg2[MAX_INPUT_LENGTH];
        arg_passed = one_argument( arg_passed, arg2 );
        board = find_board( ch );

        if ( !board )
        {
            send_to_char( "There is no bulletin board here.\n\r", ch );
            return;
        }

        if ( !can_read( ch, board ) )
        {
            send_to_char( "You cannot vote on this board.\n\r", ch );
            return;
        }

        if ( is_number( arg2 ) )
            anum = atoi( arg2 );
        else
        {
            send_to_char( "Note vote which number?\n\r", ch );
            return;
        }

        vnum = 1;

        for ( pnote = board->first_note; pnote && vnum < anum; pnote = pnote->next )
            vnum++;

        if ( !pnote )
        {
            send_to_char( "No such note.\n\r", ch );
            return;
        }

        /* Options: open close yes no abstain */
        /*  If you're the author of the note and can read the board you can open
            and close voting, if you can read it and voting is open you can vote.
        */
        if ( !str_cmp( arg_passed, "open" ) )
        {
            if ( str_cmp( ch->name, pnote->sender ) )
            {
                send_to_char( "You are not the author of this message.\n\r", ch );
                return;
            }

            pnote->voting = VOTE_OPEN;
            act( AT_ACTION, "$n opens voting on a note.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "Voting opened.\n\r", ch );
            write_board( board );
            return;
        }

        if ( !str_cmp( arg_passed, "close" ) )
        {
            if ( str_cmp( ch->name, pnote->sender ) )
            {
                send_to_char( "You are not the author of this message.\n\r", ch );
                return;
            }

            pnote->voting = VOTE_CLOSED;
            act( AT_ACTION, "$n closes voting on a note.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "Voting closed.\n\r", ch );
            write_board( board );
            return;
        }

        /* Make sure the note is open for voting before going on. */
        if ( pnote->voting != VOTE_OPEN )
        {
            send_to_char( "Voting is not open on this note.\n\r", ch );
            return;
        }

        /* Can only vote once on a note. */
        sprintf( buf, "%s %s %s",
                 pnote->yesvotes, pnote->novotes, pnote->abstentions );

        if ( is_name( ch->name, buf ) )
        {
            send_to_char( "You have already voted on this note.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg_passed, "yes" ) )
        {
            sprintf( buf, "%s %s", pnote->yesvotes, ch->name );
            DISPOSE( pnote->yesvotes );
            pnote->yesvotes = str_dup( buf );
            act( AT_ACTION, "$n votes on a note.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "Ok.\n\r", ch );
            write_board( board );
            return;
        }

        if ( !str_cmp( arg_passed, "no" ) )
        {
            sprintf( buf, "%s %s", pnote->novotes, ch->name );
            DISPOSE( pnote->novotes );
            pnote->novotes = str_dup( buf );
            act( AT_ACTION, "$n votes on a note.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "Ok.\n\r", ch );
            write_board( board );
            return;
        }

        if ( !str_cmp( arg_passed, "abstain" ) )
        {
            sprintf( buf, "%s %s", pnote->abstentions, ch->name );
            DISPOSE( pnote->abstentions );
            pnote->abstentions = str_dup( buf );
            act( AT_ACTION, "$n votes on a note.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "Ok.\n\r", ch );
            write_board( board );
            return;
        }

        do_note( ch, "", FALSE );
    }

    if ( !str_cmp( arg, "write" ) )
    {
        if ( ch->substate == SUB_RESTRICTED )
        {
            send_to_char( "You cannot write a note from within another command.\n\r", ch );
            return;
        }

        if ( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL
                ||     paper->item_type != ITEM_PAPER )
        {
            if ( get_trust( ch ) < sysdata.write_mail_free )
            {
                send_to_char( "You need to be holding a message disk to write a note.\n\r", ch );
                return;
            }

            paper = create_object( get_obj_index( OBJ_VNUM_NOTE ), 0 );

            if ( ( tmpobj = get_eq_char( ch, WEAR_HOLD ) ) != NULL )
                unequip_char( ch, tmpobj );

            paper = obj_to_char( paper, ch );
            equip_char( ch, paper, WEAR_HOLD );
            act( AT_MAGIC, "$n grabs a message disk to record a note.",
                 ch, NULL, NULL, TO_ROOM );
            act( AT_MAGIC, "You get a message disk to record your note.",
                 ch, NULL, NULL, TO_CHAR );
        }

        if ( paper->value[0] < 2 )
        {
            paper->value[0] = 1;
            ed = SetOExtra( paper, "_text_" );
            ch->substate = SUB_WRITING_NOTE;
            ch->dest_buf = ed;

            if ( get_trust( ch ) < sysdata.write_mail_free )
                --quill->value[0];

            start_editing( ch, ed->description );
            return;
        }
        else
        {
            send_to_char( "You cannot modify this message.\n\r", ch );
            return;
        }
    }

    if ( !str_cmp( arg, "subject" ) )
    {
        if ( !arg_passed || arg_passed[0] == '\0' )
        {
            send_to_char( "What do you wish the subject to be?\n\r", ch );
            return;
        }

        if ( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL
                ||     paper->item_type != ITEM_PAPER )
        {
            if ( get_trust( ch ) < sysdata.write_mail_free )
            {
                send_to_char( "You need to be holding a message disk to record a note.\n\r", ch );
                return;
            }

            paper = create_object( get_obj_index( OBJ_VNUM_NOTE ), 0 );

            if ( ( tmpobj = get_eq_char( ch, WEAR_HOLD ) ) != NULL )
                unequip_char( ch, tmpobj );

            paper = obj_to_char( paper, ch );
            equip_char( ch, paper, WEAR_HOLD );
            act( AT_MAGIC, "$n grabs a message disk.",
                 ch, NULL, NULL, TO_ROOM );
            act( AT_MAGIC, "You get a message disk to record your note.",
                 ch, NULL, NULL, TO_CHAR );
        }

        if ( paper->value[1] > 1 )
        {
            send_to_char( "You cannot modify this message.\n\r", ch );
            return;
        }
        else
        {
            paper->value[1] = 1;
            ed = SetOExtra( paper, "_subject_" );
            STRFREE( ed->description );
            ed->description = STRALLOC( arg_passed );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
    }

    if ( !str_cmp( arg, "to" ) )
    {
        struct stat fst;
        char fname[1024];

        if ( !arg_passed || arg_passed[0] == '\0' )
        {
            send_to_char( "Please specify an addressee.\n\r", ch );
            return;
        }

        if ( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL
                ||     paper->item_type != ITEM_PAPER )
        {
            if ( get_trust( ch ) < sysdata.write_mail_free )
            {
                send_to_char( "You need to be holding a message disk to record a note.\n\r", ch );
                return;
            }

            paper = create_object( get_obj_index( OBJ_VNUM_NOTE ), 0 );

            if ( ( tmpobj = get_eq_char( ch, WEAR_HOLD ) ) != NULL )
                unequip_char( ch, tmpobj );

            paper = obj_to_char( paper, ch );
            equip_char( ch, paper, WEAR_HOLD );
            act( AT_MAGIC, "$n gets a message disk to record a note.",
                 ch, NULL, NULL, TO_ROOM );
            act( AT_MAGIC, "You grab a message disk to record your note.",
                 ch, NULL, NULL, TO_CHAR );
        }

        if ( paper->value[2] > 1 )
        {
            send_to_char( "You cannot modify this message.\n\r", ch );
            return;
        }

        arg_passed[0] = UPPER( arg_passed[0] );
        sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower( arg_passed[0] ),
                 capitalize( arg_passed ) );

        if ( !IS_MAIL || stat( fname, &fst ) != -1 || !str_cmp( arg_passed, "all" ) )
        {
            paper->value[2] = 1;
            ed = SetOExtra( paper, "_to_" );
            STRFREE( ed->description );
            ed->description = STRALLOC( arg_passed );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        else
        {
            send_to_char( "No player exists by that name.\n\r", ch );
            return;
        }
    }

    if ( !str_cmp( arg, "show" ) )
    {
        char* subject, *to_list, *text;

        if ( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL
                ||     paper->item_type != ITEM_PAPER )
        {
            send_to_char( "You are not holding a message disk.\n\r", ch );
            return;
        }

        if ( ( subject = get_extra_descr( "_subject_", paper->first_extradesc ) ) == NULL )
            subject = "(no subject)";

        if ( ( to_list = get_extra_descr( "_to_", paper->first_extradesc ) ) == NULL )
            to_list = "(nobody)";

        sprintf( buf, "%s: %s\n\rTo: %s\n\r",
                 ch->name,
                 subject,
                 to_list );
        send_to_char( buf, ch );

        if ( ( text = get_extra_descr( "_text_", paper->first_extradesc ) ) == NULL )
            text = "The disk is blank.\n\r";

        send_to_char( text, ch );
        return;
    }

    if ( !str_cmp( arg, "post" ) )
    {
        char* strtime, *to, *subj, *text;

        if ( ( paper = get_eq_char( ch, WEAR_HOLD ) ) == NULL
                ||     paper->item_type != ITEM_PAPER )
        {
            send_to_char( "You are not holding a message disk.\n\r", ch );
            return;
        }

        if ( paper->value[0] == 0 )
        {
            send_to_char( "There is nothing written on this disk.\n\r", ch );
            return;
        }

        if ( paper->value[1] == 0 )
        {
            send_to_char( "This message has no subject... using 'none'.\n\r", ch );
            paper->value[1] = 1;
            ed = SetOExtra( paper, "_subject_" );
            STRFREE( ed->description );
            ed->description = STRALLOC( "none" );
        }

        if ( paper->value[2] == 0 )
        {
            if ( IS_MAIL )
            {
                send_to_char( "This message is addressed to no one!\n\r", ch );
                return;
            }
            else
            {
                send_to_char( "This message is addressed to no one... sending to 'all'!\n\r", ch );
                paper->value[2] = 1;
                ed = SetOExtra( paper, "_to_" );
                STRFREE( ed->description );
                ed->description = STRALLOC( "All" );
            }
        }

        board = find_board( ch );

        if ( !board )
        {
            send_to_char( "There is no terminal here to upload your message to.\n\r", ch );
            return;
        }

        if ( !can_post( ch, board ) )
        {
            send_to_char( "You cannot use this terminal. It is encrypted...\n\r", ch );
            return;
        }

        if ( board->num_posts >= board->max_posts )
        {
            send_to_char( "This terminal is full. There is no room for your message.\n\r", ch );
            return;
        }

        /*act( AT_ACTION, "$n uploads a message.", ch, NULL, NULL, TO_ROOM );*/
        strtime             = ctime( &current_time );
        strtime[strlen( strtime ) - 1]  = '\0';
        /* handle IMC notes */
        to = get_extra_descr( "_to_", paper->first_extradesc );
        subj = get_extra_descr( "_subject_", paper->first_extradesc );
        text = get_extra_descr( "_text_", paper->first_extradesc );
        act( AT_ACTION, "$n uploads a message.", ch, NULL, NULL, TO_ROOM );
        CREATE( pnote, NOTE_DATA, 1 );
        pnote->date         = STRALLOC( strtime );
        text = get_extra_descr( "_text_", paper->first_extradesc );
        pnote->text = text ? STRALLOC( text ) : STRALLOC( "" );
        text = get_extra_descr( "_to_", paper->first_extradesc );
        pnote->to_list = text ? STRALLOC( text ) : STRALLOC( "all" );
        text = get_extra_descr( "_subject_", paper->first_extradesc );
        pnote->subject = text ? STRALLOC( text ) : STRALLOC( "" );
        pnote->sender  = QUICKLINK( ch->name );
        pnote->voting      = 0;
        pnote->yesvotes    = str_dup( "" );
        pnote->novotes     = str_dup( "" );
        pnote->abstentions = str_dup( "" );
        LINK( pnote, board->first_note, board->last_note, next, prev );
        board->num_posts++;
        write_board( board );
        send_to_char( "You upload your message to the terminal.\n\r", ch );
        extract_obj( paper );
        return;
    }

    if ( !str_cmp( arg, "remove" )
            ||   !str_cmp( arg, "take" )
            ||   !str_cmp( arg, "copy" ) )
    {
        char take;
        board = find_board( ch );

        if ( !board )
        {
            send_to_char( "There is no terminal here to download a note from!\n\r", ch );
            return;
        }

        if ( !str_cmp( arg, "take" ) )
            take = 1;
        else if ( !str_cmp( arg, "copy" ) )
        {
            if ( !IS_IMMORTAL( ch ) )
            {
                send_to_char( "Huh?  Type 'help note' for usage.\n\r", ch );
                return;
            }

            take = 2;
        }
        else
            take = 0;

        if ( !is_number( arg_passed ) )
        {
            send_to_char( "Note remove which number?\n\r", ch );
            return;
        }

        if ( !can_read( ch, board ) )
        {
            send_to_char( "You can't make any sense of what's posted here, let alone remove anything!\n\r", ch );
            return;
        }

        anum = atoi( arg_passed );
        vnum = 0;

        for ( pnote = board->first_note; pnote; pnote = pnote->next )
        {
            if ( IS_MAIL && ( ( is_note_to( ch, pnote ) )
                              ||  get_trust( ch ) >= sysdata.take_others_mail ) )
                vnum++;
            else if ( !IS_MAIL )
                vnum++;

            if ( ( is_note_to( ch, pnote )
                    ||      can_remove ( ch, board ) )
                    &&   ( vnum == anum ) )
            {
                if ( ( is_name( "all", pnote->to_list ) )
                        &&   ( get_trust( ch ) < sysdata.take_others_mail )
                        &&   ( take == 1 ) )
                {
                    send_to_char( "Notes addressed to 'all' can not be taken.\n\r", ch );
                    return;
                }

                if ( take != 0 )
                {
                    paper = create_object( get_obj_index( OBJ_VNUM_NOTE ), 0 );
                    ed = SetOExtra( paper, "_sender_" );
                    STRFREE( ed->description );
                    ed->description = QUICKLINK( pnote->sender );
                    ed = SetOExtra( paper, "_text_" );
                    STRFREE( ed->description );
                    ed->description = QUICKLINK( pnote->text );
                    ed = SetOExtra( paper, "_to_" );
                    STRFREE( ed->description );
                    ed->description = QUICKLINK( pnote->to_list );
                    ed = SetOExtra( paper, "_subject_" );
                    STRFREE( ed->description );
                    ed->description = QUICKLINK( pnote->subject );
                    ed = SetOExtra( paper, "_date_" );
                    STRFREE( ed->description );
                    ed->description = QUICKLINK( pnote->date );
                    ed = SetOExtra( paper, "note" );
                    STRFREE( ed->description );
                    sprintf( notebuf, "From: " );
                    strcat( notebuf, pnote->sender );
                    strcat( notebuf, "\n\rTo: " );
                    strcat( notebuf, pnote->to_list );
                    strcat( notebuf, "\n\rSubject: " );
                    strcat( notebuf, pnote->subject );
                    strcat( notebuf, "\n\r\n\r" );
                    strcat( notebuf, pnote->text );
                    strcat( notebuf, "\n\r" );
                    ed->description = STRALLOC( notebuf );
                    paper->value[0] = 2;
                    paper->value[1] = 2;
                    paper->value[2] = 2;
                    sprintf( short_desc_buf, "a note from %s to %s",
                             pnote->sender, pnote->to_list );
                    STRFREE( paper->short_descr );
                    paper->short_descr = STRALLOC( short_desc_buf );
                    sprintf( long_desc_buf, "A note from %s to %s lies on the ground.",
                             pnote->sender, pnote->to_list );
                    STRFREE( paper->description );
                    paper->description = STRALLOC( long_desc_buf );
                    sprintf( keyword_buf, "note parchment paper %s",
                             pnote->to_list );
                    STRFREE( paper->name );
                    paper->name = STRALLOC( keyword_buf );
                }

                if ( take != 2 )
                    note_remove( ch, board, pnote );

                send_to_char( "Ok.\n\r", ch );

                if ( take == 1 )
                {
                    act( AT_ACTION, "$n downloads a message.", ch, NULL, NULL, TO_ROOM );
                    obj_to_char( paper, ch );
                }
                else if ( take == 2 )
                {
                    act( AT_ACTION, "$n copies a message.", ch, NULL, NULL, TO_ROOM );
                    obj_to_char( paper, ch );
                }
                else
                    act( AT_ACTION, "$n removes a message.", ch, NULL, NULL, TO_ROOM );

                return;
            }
        }

        send_to_char( "No such message.\n\r", ch );
        return;
    }

    send_to_char( "Huh?  Type 'help note' for usage.\n\r", ch );
    return;
}

BOARD_DATA* read_board( char* boardfile, FILE* fp )
{
    BOARD_DATA* board;
    char* word;
    char  buf[MAX_STRING_LENGTH];
    bool fMatch;
    char letter;

    do
    {
        letter = getc( fp );

        if ( feof( fp ) )
        {
            fclose( fp );
            return NULL;
        }
    } while ( isspace( letter ) );

    ungetc( letter, fp );
    CREATE( board, BOARD_DATA, 1 );
#ifdef KEY
#undef KEY
#endif
#define KEY( literal, field, value )                    \
    if ( !str_cmp( word, literal ) )    \
    {                   \
        field  = value;         \
        fMatch = TRUE;          \
        break;              \
    }

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
                KEY( "Extra_readers",   board->extra_readers,   fread_string_nohash( fp ) );
                KEY( "Extra_removers",       board->extra_removers,   fread_string_nohash( fp ) );

                if ( !str_cmp( word, "End" ) )
                {
                    board->num_posts  = 0;
                    board->first_note = NULL;
                    board->last_note  = NULL;
                    board->next   = NULL;
                    board->prev   = NULL;

                    if ( !board->read_group )
                        board->read_group    = str_dup( "" );

                    if ( !board->post_group )
                        board->post_group    = str_dup( "" );

                    if ( !board->extra_readers )
                        board->extra_readers = str_dup( "" );

                    if ( !board->extra_removers )
                        board->extra_removers = str_dup( "" );

                    return board;
                }

            case 'F':
                KEY( "Filename",    board->note_file,   fread_string_nohash( fp ) );

            case 'M':
                KEY( "Min_read_level",  board->min_read_level,  fread_number( fp ) );
                KEY( "Min_post_level",  board->min_post_level,  fread_number( fp ) );
                KEY( "Min_remove_level",    board->min_remove_level, fread_number( fp ) );
                KEY( "Max_posts",       board->max_posts,   fread_number( fp ) );

            case 'P':
                KEY( "Post_group",  board->post_group,  fread_string_nohash( fp ) );

            case 'R':
                KEY( "Read_group",  board->read_group,  fread_string_nohash( fp ) );

            case 'T':
                KEY( "Type",    board->type,        fread_number( fp ) );

            case 'V':
                KEY( "Vnum",    board->board_obj,   fread_number( fp ) );
        }

        if ( !fMatch )
        {
            sprintf( buf, "read_board: no match: %s", word );
            bug( buf, 0 );
        }
    }

    return board;
}

NOTE_DATA* read_note( char* notefile, FILE* fp )
{
    NOTE_DATA* pnote;
    char* word;

    for ( ; ; )
    {
        char letter;

        do
        {
            letter = getc( fp );

            if ( feof( fp ) )
            {
                fclose( fp );
                return NULL;
            }
        } while ( isspace( letter ) );

        ungetc( letter, fp );
        CREATE( pnote, NOTE_DATA, 1 );

        if ( str_cmp( fread_word( fp ), "sender" ) )
            break;

        pnote->sender   = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "date" ) )
            break;

        pnote->date = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "to" ) )
            break;

        pnote->to_list  = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "subject" ) )
            break;

        pnote->subject  = fread_string( fp );
        word = fread_word( fp );

        if ( !str_cmp( word, "voting" ) )
        {
            pnote->voting = fread_number( fp );

            if ( str_cmp( fread_word( fp ), "yesvotes" ) )
                break;

            pnote->yesvotes   = fread_string_nohash( fp );

            if ( str_cmp( fread_word( fp ), "novotes" ) )
                break;

            pnote->novotes    = fread_string_nohash( fp );

            if ( str_cmp( fread_word( fp ), "abstentions" ) )
                break;

            pnote->abstentions    = fread_string_nohash( fp );
            word = fread_word( fp );
        }

        if ( str_cmp( word, "text" ) )
            break;

        pnote->text = fread_string( fp );

        if ( !pnote->yesvotes )
            pnote->yesvotes  = str_dup( "" );

        if ( !pnote->novotes )
            pnote->novotes   = str_dup( "" );

        if ( !pnote->abstentions )
            pnote->abstentions   = str_dup( "" );

        pnote->next     = NULL;
        pnote->prev     = NULL;
        return pnote;
    }

    bug( "read_note: bad key word.", 0 );
    exit( 1 );
}

/*
    Load boards file.
*/
void load_boards( void )
{
    FILE*    board_fp;
    FILE*    note_fp;
    BOARD_DATA*  board;
    NOTE_DATA*   pnote;
    char    boardfile[256];
    char    notefile[256];
    first_board = NULL;
    last_board  = NULL;
    sprintf( boardfile, "%s%s", BOARD_DIR, BOARD_FILE );

    if ( ( board_fp = fopen( boardfile, "r" ) ) == NULL )
        return;

    while ( ( board = read_board( boardfile, board_fp ) ) != NULL )
    {
        LINK( board, first_board, last_board, next, prev );
        sprintf( notefile, "%s%s", BOARD_DIR, board->note_file );

        // log_string( notefile );
        if ( ( note_fp = fopen( notefile, "r" ) ) != NULL )
        {
            while ( ( pnote = read_note( notefile, note_fp ) ) != NULL )
            {
                LINK( pnote, board->first_note, board->last_note, next, prev );
                board->num_posts++;
            }
        }
    }

    return;
}


void do_makeboard( CHAR_DATA* ch, char* argument )
{
    BOARD_DATA* board;

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage: makeboard <filename>\n\r", ch );
        return;
    }

    smash_tilde( argument );
    CREATE( board, BOARD_DATA, 1 );
    LINK( board, first_board, last_board, next, prev );
    board->note_file       = str_dup( strlower( argument ) );
    board->read_group      = str_dup( "" );
    board->post_group      = str_dup( "" );
    board->extra_readers   = str_dup( "" );
    board->extra_removers  = str_dup( "" );
}

void do_bset( CHAR_DATA* ch, char* argument )
{
    BOARD_DATA* board;
    bool found;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int value;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Usage: bset <board filename> <field> value\n\r", ch );
        send_to_char( "\n\rField being one of:\n\r", ch );
        send_to_char( "  vnum read post remove maxpost filename type\n\r", ch );
        send_to_char( "  read_group post_group extra_readers extra_removers\n\r", ch );
        return;
    }

    value = atoi( argument );
    found = FALSE;

    for ( board = first_board; board; board = board->next )
        if ( !str_cmp( arg1, board->note_file ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
    {
        send_to_char( "Board not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "vnum" ) )
    {
        if ( !get_obj_index( value ) )
        {
            send_to_char( "No such object.\n\r", ch );
            return;
        }

        board->board_obj = value;
        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "read" ) )
    {
        if ( value < 0 || value > MAX_LEVEL )
        {
            send_to_char( "Value out of range.\n\r", ch );
            return;
        }

        board->min_read_level = value;
        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "read_group" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "No group specified.\n\r", ch );
            return;
        }

        DISPOSE( board->read_group );

        if ( !str_cmp( argument, "none" ) )
            board->read_group = str_dup( "" );
        else
            board->read_group = str_dup( argument );

        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "post_group" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "No group specified.\n\r", ch );
            return;
        }

        DISPOSE( board->post_group );

        if ( !str_cmp( argument, "none" ) )
            board->post_group = str_dup( "" );
        else
            board->post_group = str_dup( argument );

        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "extra_removers" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "No names specified.\n\r", ch );
            return;
        }

        if ( !str_cmp( argument, "none" ) )
            buf[0] = '\0';
        else
            sprintf( buf, "%s %s", board->extra_removers, argument );

        DISPOSE( board->extra_removers );
        board->extra_removers = str_dup( buf );
        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "extra_readers" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "No names specified.\n\r", ch );
            return;
        }

        if ( !str_cmp( argument, "none" ) )
            buf[0] = '\0';
        else
            sprintf( buf, "%s %s", board->extra_readers, argument );

        DISPOSE( board->extra_readers );
        board->extra_readers = str_dup( buf );
        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "filename" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "No filename specified.\n\r", ch );
            return;
        }

        DISPOSE( board->note_file );
        board->note_file = str_dup( argument );
        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "post" ) )
    {
        if ( value < 0 || value > MAX_LEVEL )
        {
            send_to_char( "Value out of range.\n\r", ch );
            return;
        }

        board->min_post_level = value;
        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "remove" ) )
    {
        if ( value < 0 || value > MAX_LEVEL )
        {
            send_to_char( "Value out of range.\n\r", ch );
            return;
        }

        board->min_remove_level = value;
        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "maxpost" ) )
    {
        if ( value < 1 || value > 1000 )
        {
            send_to_char( "Value out of range.\n\r", ch );
            return;
        }

        board->max_posts = value;
        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "type" ) )
    {
        if ( value < 0 || value > 1 )
        {
            send_to_char( "Value out of range.\n\r", ch );
            return;
        }

        board->type = value;
        write_boards_txt( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    do_bset( ch, "" );
    return;
}


void do_bstat( CHAR_DATA* ch, char* argument )
{
    BOARD_DATA* board;
    bool found;
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Usage: bstat <board filename>\n\r", ch );
        return;
    }

    found = FALSE;

    for ( board = first_board; board; board = board->next )
        if ( !str_cmp( arg, board->note_file ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
    {
        send_to_char( "Board not found.\n\r", ch );
        return;
    }

    ch_printf( ch, "%-12s Vnum: %5d Read: %2d Post: %2d Rmv: %2d Max: %2d Posts: %d Type: %d\n\r",
               board->note_file,    board->board_obj,
               board->min_read_level,   board->min_post_level,
               board->min_remove_level, board->max_posts,
               board->num_posts, board->type );
    ch_printf( ch, "Read_group: %-15s Post_group: %-15s \n\rExtra_readers: %-10s\n\r",
               board->read_group, board->post_group, board->extra_readers );
    return;
}


void do_boards( CHAR_DATA* ch, char* argument )
{
    BOARD_DATA* board;

    if ( !str_cmp( argument, "clear now" ) && get_trust( ch ) >= 105 )
    {
        NOTE_DATA* pnote, *pnext = NULL;

        for ( board = first_board; board; board = board->next )
        {
            for ( pnote = board->first_note; pnote; pnote = pnext )
            {
                pnext = pnote->next;
                note_remove( ch, board, pnote );
            }
        }

        send_to_char( "Boards cleared.\n\r", ch );
        return;
    }

    if ( !first_board )
    {
        send_to_char( "There are no boards.\n\r", ch );
        return;
    }

    set_char_color( AT_NOTE, ch );

    for ( board = first_board; board; board = board->next )
        ch_printf( ch, "%-16s Vnum: %5d Read: %2d Post: %2d Rmv: %2d Max: %2d Posts: %d Type: %d\n\r",
                   board->note_file,    board->board_obj,
                   board->min_read_level,   board->min_post_level,
                   board->min_remove_level, board->max_posts, board->num_posts,
                   board->type );
}

void mail_count( CHAR_DATA* ch )
{
    BOARD_DATA* board = NULL;
    NOTE_DATA* note = NULL;
    int cnt = 0;

    for ( board = first_board; board; board = board->next )
        if ( board->type == BOARD_MAIL && can_read( ch, board ) )
            for ( note = board->first_note; note; note = note->next )
                if ( is_note_to( ch, note ) )
                    ++cnt;

    if ( cnt )
        ch_printf( ch, "&z(&CY&z)ou have [&C%d&z] mail messages waiting.\n\r", cnt );

    return;
}
