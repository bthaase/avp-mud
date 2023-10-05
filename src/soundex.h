/***************************************************************************
                           STAR WARS REALITY [GOLD]
    --------------------------------------------------------------------------
    Star Wars Reality GOLD
    copyright (c) 2001, 2002 by Brian Haase
    -------------------------------------------------------------------------
    Star Wars Reality 1.0
    copyright (c) 1997, 1998 by Sean Cooper
    -------------------------------------------------------------------------
    Starwars and Starwars Names copyright(c) Lucasfilm Ltd.
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
                               Soundex parser
****************************************************************************/

/***************************************************************************
    Snippet: Soundex parser.
    Author:  Richard Woolcock (aka KaVir)
    Date:    20th December 2000.
****************************************************************************
    This code is copyright (C) 2000 by Richard Woolcock.  It may used and
    distributed freely, as long as you don't remove this copyright notice.
****************************************************************************/

#ifndef SOUNDEX_HEADER
#define SOUNDEX_HEADER

#define KEY_SIZE 4    /* Size of Soundex key */

char* GetSoundexKey ( const char* szTxt );
int  SoundexMatch   ( char* szFirst, char* szSecond );

#endif /* SOUNDEX_HEADER */
