/***************************************************************************
*                           STAR WARS REALITY 1.0                          *
*--------------------------------------------------------------------------*
* Star Wars Reality Code Additions and changes from the Smaug Code         *
* copyright (c) 1997 by Sean Cooper                                        *
* -------------------------------------------------------------------------*
* Starwars and Starwars Names copyright(c) Lucas Film Ltd.                 *
*--------------------------------------------------------------------------*
* SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider                           *
* SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,                    *
* Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops                *
* ------------------------------------------------------------------------ *
* Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
* Chastain, Michael Quan, and Mitchell Tse.                                *
* Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
* Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
* ------------------------------------------------------------------------ *
*                               Arena Module                               *   
****************************************************************************/
 
#include <math.h> 
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

ARENA_DATA * first_arena;
ARENA_DATA * last_arena;
ARENA_DATA * curr_arena;

VOTE_DATA  * curr_vote;

int arenaNext=0;

void write_arena_list( )
{
    ARENA_DATA *tarena;
    FILE *fpout;
    char filename[256];
    
    sprintf( filename, "%s%s", ARENA_DIR, ARENA_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
         bug( "FATAL: cannot open arena.lst for writing!\n\r", 0 );
         return;
    }
    for ( tarena = first_arena; tarena; tarena = tarena->next )
    {
      fprintf( fpout, "%s\n", tarena->filename );
    }

    fprintf( fpout, "$\n" );
    fclose( fpout );
}

/*
 * Save a arena's data to its data file
 */
void save_arena( ARENA_DATA *arena )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];
    int i;

    if ( !arena )
    {
        bug( "save_arena: null arena pointer!", 0 );
	return;
    }
        
    if ( !arena->filename || arena->filename[0] == '\0' )
    {
        sprintf( buf, "save_arena: %s has no filename", arena->name );
	bug( buf, 0 );
	return;
    }
 
    sprintf( filename, "%s%s", ARENA_DIR, arena->filename );
    
    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "save_arena: fopen", 0 );
    	perror( filename );
    }
    else
    {
        fprintf( fp, "#ARENA\n" );
        fprintf( fp, "Name         %s~\n",      arena->name );
        fprintf( fp, "Desc         %s~\n",      arena->desc );
        fprintf( fp, "Filename     %s~\n",      arena->filename    );
        fprintf( fp, "Firstroom    %d\n",       arena->firstroom );
        fprintf( fp, "Lastroom     %d\n",       arena->lastroom );
        fprintf( fp, "Timer        %d\n",       arena->timer );
        for ( i = 0; i < 2; i++ )
          fprintf( fp, "Maxsupport   %d %d\n",    i, arena->maxsupport[i] );
        fprintf( fp, "Minp         %d\n",       arena->min_p );
        fprintf( fp, "Maxp         %d\n",       arena->max_p );
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

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


void fread_arena( ARENA_DATA *arena, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    char *line;
    bool fMatch;
    int x0, x1;

 
    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

        case 'D':
            KEY( "Desc",        arena->desc,               fread_string( fp ) );
	    break;

	case 'E':
            if ( !str_cmp( word, "End" ) )
	    {
                if (!arena->name) arena->name = STRALLOC( "" );
                if (!arena->desc) arena->desc = STRALLOC( "" );
                if (!arena->filename) arena->filename = STRALLOC( "" );

                if (arena->prev != NULL)
                  arena->id = arena->prev->id + 1;
                else
                  arena->id = 1;

		return;
	    }
	    break;
	    
        case 'T':
            KEY( "Timer",       arena->timer,       fread_number( fp ) );
            break;

        case 'F':
            KEY( "Filename",    arena->filename,           fread_string_nohash( fp ) );
            KEY( "Firstroom",   arena->firstroom,          fread_number( fp ) );
	    break;

        case 'L':
            KEY( "Lastroom",    arena->lastroom,           fread_number( fp ) );
            break;

	case 'N':
            KEY( "Name",        arena->name,               fread_string( fp ) );
	    break;
        
        case 'M':
            KEY( "Minp",        arena->min_p,              fread_number( fp ) ); 
            KEY( "Maxp",        arena->max_p,              fread_number( fp ) );
            if ( !str_cmp( word, "Maxsupport"  ) )
            {
                line = fread_line( fp );
                x0=x1=0;
                sscanf( line, "%d %d", &x0, &x1 );
                if ( x0 >= 0 && x0 < MAX_RACE ) arena->maxsupport[x0] = x1;
                fMatch = TRUE;
                break;
            }
            break;
                 
       	}
	
	if ( !fMatch )
	{
            sprintf( buf, "Fread_arena: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

/*
 * Load a arena file
 */
bool load_arena( char *arenafile )
{
    char filename[256];
    ARENA_DATA *arena;
    FILE *fp;
    bool found;

    CREATE( arena, ARENA_DATA, 1 );

    found = FALSE;
    sprintf( filename, "%s%s", ARENA_DIR, arenafile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
        LINK( arena, first_arena, last_arena, next, prev );
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
                bug( "Load_arena_file: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "ARENA"        ) )
	    {
                fread_arena( arena, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

                sprintf( buf, "Load_arena_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }
    else
        bug("load_arena: failed to open arena file!");

    if ( !(found) )
      DISPOSE( arena );

    return found;
}

void load_arenas( )
{
    FILE *fpList;
    char *filename;
    char arenalist[256];
    char buf[MAX_STRING_LENGTH];
    
    arenaNext      = 0;
    first_arena    = NULL;
    last_arena     = NULL;

    sprintf( arenalist, "%s%s", ARENA_DIR, ARENA_LIST );
    fclose( fpReserve );
    if ( ( fpList = fopen( arenalist, "r" ) ) == NULL )
    {
        perror( arenalist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	if ( filename[0] == '$' )
	  break;	  
       
        if ( !load_arena( filename ) )
	{
          sprintf( buf, "Cannot load arena file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    fpReserve = fopen( NULL_FILE, "r" );
    return;

}

/*
 * Get pointer to an arena from the arena ID.
 */
ARENA_DATA *arena_from_id( int id )
{
    ARENA_DATA *arena;
       
    for ( arena = first_arena; arena; arena = arena->next )
       if ( arena->id == id )
         return arena;
    
    return NULL;
}


/*
 * Get pointer to an arena from the arena name.
 */
ARENA_DATA *arena_from_name( char *name )
{
    ARENA_DATA *arena;
       
    if ( ( arena = arena_from_id( atoi(name) ) ) != NULL )
       return arena;

    for ( arena = first_arena; arena; arena = arena->next )
       if ( !str_cmp( name, arena->name ) )
         return arena;
    
    for ( arena = first_arena; arena; arena = arena->next )
       if ( !str_prefix( name, arena->name ) )
         return arena;
    
    return NULL;
}

/*
 * Get pointer to an arena from a vnum.
 */
ARENA_DATA *arena_from_vnum( int vnum )
{
    ARENA_DATA *arena;

    for ( arena = first_arena; arena; arena = arena->next )
    {
       if ( !arena || arena == NULL )
          continue;
       if ( arena->firstroom <= vnum && arena->lastroom >= vnum )
          return arena;
    }

    return NULL;
}

/*
 * Makes a new arena structure
 */
void do_makearena( CHAR_DATA *ch, char *argument )
{   
    char arg[MAX_INPUT_LENGTH];
    char filename[256];
    ARENA_DATA *arena;

    argument = one_argument( argument, arg );

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage: makearena <filename> <arena name>\n\r", ch );
        send_to_char( " IE: makearena 'MACompoundRush' USCM Compound Rush\n\r", ch );
	return;
    }

    CREATE( arena, ARENA_DATA, 1 );
    LINK( arena, first_arena, last_arena, next, prev );

    if ( arena->prev == NULL )
       arena->id = 1;
    else
       arena->id = arena->prev->id + 1;

    arena->name            = STRALLOC( argument );
 
    sprintf( filename, "%s", arg );
    arena->filename = STRALLOC( filename );
    save_arena( arena );
    write_arena_list();
}

void do_allarenas( CHAR_DATA *ch, char *argument )
{
    char info[MAX_STRING_LENGTH];
    ARENA_DATA *arena;
    int count = 0, ppl = 0;

    ch_printf( ch, "&zArenas: -----------------------\n\r" );
    for ( arena = first_arena; arena; arena = arena->next )
    {
        if ( !str_cmp(argument, "people") )
        {
           CHAR_DATA *victim;
           ppl = 0;
           for ( victim = first_char; victim; victim = victim->next )
           { 
                if ( victim == NULL )
                    continue;
                if ( victim->in_room == NULL )
                    continue;
                if ( victim->in_room->vnum >= arena->firstroom && victim->in_room->vnum <= arena->lastroom )
                    ppl++;
           }
           sprintf( info, "People inside: %d", ppl );
        }
        else
           sprintf( info, "%s", arena->filename);


        ch_printf( ch, "&z[&C%-3d&z]: [ &B%-26s&z]  &C%s\n\r", arena->id, info, arena->name );
        count++;
    }

    if ( !count )
    {
        send_to_char( "There are no arenas currently formed.\n\r", ch );
	return;
    }
}

void do_showarena( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    ARENA_DATA *arena;
    int count=0;

    if ( argument[0] == '\0' )
    {
       send_to_char( "Syntax: showarena <name>\n\r", ch );
       return;
    }

    arena = arena_from_name( argument );

    if ( arena == NULL )
    {
       send_to_char("&RArena not found.\n\r", ch );
       return;
    }

    /* Count the people inside */
    for ( victim = first_char; victim; victim = victim->next )
    { 
        if ( victim == NULL )
           continue;
        if ( victim->in_room == NULL )
           continue;
        if ( victim->in_room->vnum >= arena->firstroom && victim->in_room->vnum <= arena->lastroom )
           count++;
    }

    ch_printf( ch, "&zName:      &C%s &z[&WID: %d&z]\n\r", arena->name, arena->id );
    ch_printf( ch, "&zFilename:  &C%s\n\r", arena->filename );
    ch_printf( ch, "&zDescription:\n\r&C%s\n\r", arena->desc );
    ch_printf( ch, "&zPeople Inside: &C%d\n\r", count );
    ch_printf( ch, "&z---&BRooms&z-----------------------------------------------\n\r" );
    ch_printf( ch, "&zFirstroom: &C%d\n\r", arena->firstroom );
    ch_printf( ch, "&zLastroom:  &C%d\n\r", arena->lastroom );
    ch_printf( ch, "&z---&BStats&z-----------------------------------------------\n\r" );
    ch_printf( ch, "&zMinp:  &C%d           &zMaxp: &C%d\n\r", arena->min_p, arena->max_p );
    ch_printf( ch, "&zTimer: &C%d\n\r", arena->timer );
}

void do_setarena( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    ARENA_DATA *arena;

    if ( !ch->desc || IS_NPC(ch) ) return;

    switch( ch->substate )
    {
	default:
	  break;
        case SUB_ARENA_DESC:
          arena = ch->dest_buf;
          if ( !arena )
	  {
                bug( "setarena: sub_arena_desc: NULL ch->dest_buf", 0 );
                stop_editing( ch );
                ch->substate = ch->tempnum;
                return;
	  }
          STRFREE( arena->desc );
          arena->desc = copy_buffer( ch );
	  stop_editing( ch );
	  ch->substate = ch->tempnum;
          send_to_char( "Arena Desc set.\n\r", ch );
          save_arena( arena );
	  return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
       send_to_char( "&zSyntax: setarena <name> <field> <value>\n\r\n\r", ch );
       send_to_char( "&zField being one of:\n\r", ch );
       send_to_char( " &zname, filename, desc, firstroom, lastroom\n\r", ch );
       send_to_char( " &ztimer, minp, maxp, maxsupport\n\r", ch );
       return;
    }

    arena = arena_from_name( arg1 );

    if ( arena == NULL )
    {
       send_to_char("&RArena not found.\n\r", ch );
       return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
       STRFREE( arena->name );
       arena->name = STRALLOC( argument );
       send_to_char( "Arena name set.\n\r", ch );
       save_arena( arena );
    }
    else if ( !str_cmp( arg2, "filename" ) )
    {
       STRFREE( arena->filename );
       arena->filename = STRALLOC( argument );
       send_to_char( "Arena filename set. (Resaving Arenas)\n\r", ch );
       save_arena( arena );
       write_arena_list();
    }
    else if ( !str_cmp( arg2, "desc" ) )
    {
       if ( ch->substate == SUB_RESTRICTED )
       {
          send_to_char( "You cannot write a note from within another command.\n\r", ch );
          return;
       }
       ch->substate = SUB_ARENA_DESC;
       ch->dest_buf = arena;
       start_editing( ch, arena->desc );
    }
    else if ( !str_cmp( arg2, "firstroom" ) )
    {
       if ( atoi(argument) < 1 )
       {
           send_to_char("Invalid arena setting for FIRSTROOM.\n\r", ch );
           return;
       }
       arena->firstroom = atoi(argument);
       send_to_char( "Arena firstroom set.\n\r", ch );
       save_arena( arena );
    }   
    else if ( !str_cmp( arg2, "lastroom" ) )
    {
       if ( arena->firstroom < 1 )
       {
           send_to_char("Invalid setting. Set FIRSTROOM before LASTROOM.\n\r", ch );
           return;
       }
       if ( atoi(argument) < arena->firstroom )
       {
           send_to_char("Invalid arena setting for LASTROOM. Must be ABOVE the firstroom!\n\r", ch );
           return;
       }
       arena->lastroom = atoi(argument);
       send_to_char( "Arena lastroom set.\n\r", ch );
       save_arena( arena );
    }   
    else if ( !str_cmp( arg2, "timer" ) )
    {
       arena->timer = URANGE(5, atoi(argument), 999);
       send_to_char( "Arena timer set.\n\r", ch );
       save_arena( arena );
    }   
    else if ( !str_cmp( arg2, "maxsupport" ) )
    {
       char arg3[MAX_STRING_LENGTH];

       argument = one_argument( argument, arg3 );

       if ( atoi(arg3) >= 0 && atoi(arg3) < MAX_RACE)
       {
         arena->maxsupport[atoi(arg3)] = atoi(argument);
         send_to_char( "Arena Maxsupport set.\n\r", ch );
       }
       else
       {
         send_to_char( "Invalid settings for MAXSUPPORT.\n\r", ch );
       }
       save_arena( arena );
    }   
    else if ( !str_cmp( arg2, "minp" ) )
    {
       arena->min_p = URANGE(0, atoi(argument), 999);
       arena->max_p = URANGE( arena->min_p, arena->max_p, 999 );
       send_to_char( "Arena Minimum Players set.\n\r", ch );
       save_arena( arena );
    }   
    else if ( !str_cmp( arg2, "maxp" ) )
    {
       arena->max_p = URANGE(arena->min_p, atoi(argument), 999);
       send_to_char( "Arena Maximum Players set.\n\r", ch );
       save_arena( arena );
    }   
    else
       do_setarena(ch, "");
}

void set_arena( ARENA_DATA * arena )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA * room;
    AREA_DATA *area = NULL;
    CHAR_DATA *victim, *vnext;
    OBJ_DATA  *object, *onext;
    int vnum, num, i;

    arena->hive = 0;
    arena->max_hive = 0;

    for ( vnum = arena->firstroom; vnum < arena->lastroom; vnum++ )
    {
         if ( ( room = get_room_index( vnum ) ) == NULL ) continue;

         /* Attempt a Peusdo-PURGE on the room */
         for ( victim = room->first_person; victim; victim = vnext )
         {
            vnext = victim->next_in_room;
            if ( IS_NPC(victim) && !IS_BOT(victim) && !xIS_SET(victim->act, ACT_POLYMORPHED) ) extract_char( victim, TRUE, FALSE );
         }
         for ( object = room->first_content; object; object = onext )
         {
             onext = object->next_content;
             extract_obj( object );
         }

         clear_variables( room->area );
    }

    for ( vnum = arena->firstroom; vnum < arena->lastroom; vnum++ )
    {
         if ( ( room = get_room_index( vnum ) ) == NULL ) continue;

         /* Reset the area */
         if ( area != room->area )
         {
            /* Reset the area */
            area = room->area;
            num = area->nplayer;
            area->nplayer = 0;
            reset_area(area);
            area->nplayer = num;
         }

         /* Calculate the Hive settings */
         if ( can_hive_room( NULL, room ) )
         {
            arena->max_hive++;
            if ( xIS_SET( room->room_flags, ROOM_HIVED ) ) arena->hive++;
         }
    }

    /* Reset Support Values */
    for ( i = 0; i < MAX_RACE; i++ )
      arena->support[i] = arena->maxsupport[i];

    arena->kills[0] = 0;
    arena->kills[1] = 0;
    arena->kills[2] = 0;

    curr_arena = arena;
    curr_arena->ctimer = URANGE( 5, curr_arena->timer, 999 );

    sprintf( buf, "&GLoading Arena ... %s", arena->name );
    send_monitor( NULL, buf );

    return;
}

void empty_arena( ARENA_DATA * arena )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA * room;
    AREA_DATA *area = NULL;
    CHAR_DATA *victim, *vnext;
    OBJ_DATA  *object, *onext;
    int vnum, num;

    for ( vnum = arena->firstroom; vnum <= arena->lastroom; vnum++ )
    {
         if ( ( room = get_room_index( vnum ) ) == NULL ) continue;

         /* Attempt a Peusdo-PURGE on the room */
         for ( victim = room->first_person; victim; victim = vnext )
         {
            vnext = victim->next_in_room;
            if ( IS_NPC(victim) && !IS_BOT(victim) && !xIS_SET(victim->act, ACT_POLYMORPHED) )
            {
              extract_char( victim, TRUE, FALSE );
            }
            else
            {
              if ( victim->race == RACE_ALIEN )
                ch_printf( victim, "&RYou feel drawn back to the hive by an unseen presence.\n\r" );
              if ( victim->race == RACE_MARINE )
                ch_printf( victim, "&RYour rushed from the battlefield by medical units.\n\r" );
              if ( victim->race == RACE_PREDATOR )
                ch_printf( victim, "&RYou leave the torn battlezone for a better hunt.\n\r" );
              victim->vent = FALSE;
              victim->hit = victim->max_hit;
              char_from_room( victim );
              char_to_room( victim, get_room_index( wherehome( victim ) ) );              
            }
         }
         for ( object = room->first_content; object; object = onext )
         {
             onext = object->next_content;
             extract_obj( object );
         }

         /* Clear the stack! */
         clear_variables( room->area );
    }

    /* Empty the vents! */
    for ( vnum = 100001; vnum < 150000; vnum++ )
    {
         if ( ( room = get_room_index( vnum ) ) == NULL ) continue;

         /* Attempt a Peusdo-PURGE on the room */
         for ( victim = room->first_person; victim; victim = vnext )
         {
            vnext = victim->next_in_room;
            if ( IS_NPC(victim) && !IS_BOT(victim) && !xIS_SET(victim->act, ACT_POLYMORPHED) )
            {
              extract_char( victim, TRUE, FALSE );
            }
            else
            {
              if ( victim->race == RACE_ALIEN )
                ch_printf( victim, "&RYou feel drawn back to the hive by an unseen presence.\n\r" );
              if ( victim->race == RACE_MARINE )
                ch_printf( victim, "&RYour rushed from the battlefield by medical units.\n\r" );
              if ( victim->race == RACE_PREDATOR )
                ch_printf( victim, "&RYou leave the torn battlezone for a better hunt.\n\r" );
              char_from_room( victim );
              char_to_room( victim, get_room_index( wherehome( victim ) ) );              
            }
         }
    }

    return;
}

void update_arena( void )
{
    ARENA_DATA * arena;
    int pcount, safty, i;

    /* Might be temporary */
    pcount = count_players();

    if ( curr_arena )
    {
        /* Support Point Refresh */
	if ( curr_arena->ctimer % 5 == 0 )
        {
          for ( i = 0; i <= 2; i++ )
	    curr_arena->support[i] = URANGE( 0, curr_arena->support[i] + 1, curr_arena->maxsupport[i] );
	}

	curr_arena->ctimer--;

        if ( curr_arena->ctimer == 5 ) send_monitor( NULL, "&YArena will evacuate in 5 minutes." );
        if ( curr_arena->ctimer == 1 ) send_monitor( NULL, "&YArena will evacuate in 1 minute." );

        if ( curr_arena->ctimer > 0 ) return;
    }

    for( safty = 0; safty < 99999; safty++ )
    {
        arenaNext++;
                                        
        if ( arenaNext > last_arena->id ) arenaNext = first_arena->id;

        /*
         new = number_range( first_arena->id, last_arena->id );
         arena = arena_from_id( new );
         */

        arena = arena_from_id( arenaNext );
	
	if ( arena == NULL ) continue;

        if ( curr_arena )
        {
           if ( curr_arena->id == arena->id && last_arena->id > first_arena->id ) continue;
        }

        if ( pcount < arena->min_p || pcount > arena->max_p ) continue;

        if ( curr_arena ) empty_arena( curr_arena );

        set_arena( arena );

        break;
    }

    close_match( );
    open_match( );

    if ( curr_arena != NULL )
    {
       match_log( "ARENANAME;Current Arena is %s", curr_arena->name );
       match_log( "ARENATIME;Match Time = %d", curr_arena->ctimer );
    }

    write_serverstats();

    return;
}

void do_arena( CHAR_DATA *ch, char *argument )
{
    ARENA_DATA *arena;

    arena = curr_arena;

    if ( IS_IMMORTAL( ch ) && !str_cmp( argument, "skip" ) )
    {
       if ( curr_arena ) curr_arena->ctimer = 0;
       update_arena();
       return;
    }

    if ( arena == NULL )
    {
       send_to_char("&RNo Arena currently loaded. Please wait.\n\r", ch );
       return;
    }

    ch_printf( ch, "&zName: &C%s &z[&WID: %d&z]\n\r", arena->name, arena->id );
    ch_printf( ch, "&zPlayers: &W%d - %d\n\r", arena->min_p, arena->max_p );
    ch_printf( ch, "&zTime Left: &C%d minutes.\n\r", arena->ctimer );
    ch_printf( ch, "&zTime Limit: &W%d minutes.\n\r", arena->timer );
    ch_printf( ch, "&zHive Progress: &R%d of %d rooms. &z(&W%d%%&z)\n\r", arena->hive, arena->max_hive, find_percent(arena->hive, arena->max_hive) );
    ch_printf( ch, "&BPlayer Kills:\n\r" );
    ch_printf( ch, "&zAliens &Cx%d &z:: &zMarines &Cx%d &z:: &zPredators &Cx%d\n\r",
      arena->kills[RACE_ALIEN], arena->kills[RACE_MARINE], arena->kills[RACE_PREDATOR] );
    ch_printf( ch, "&zSupport Remaining for your Race: &w[&C%s&w]\n\r", drawbar( 10, arena->support[ch->race], arena->maxsupport[ch->race], "&C", "&c" ) );

    ch_printf( ch, "&BDescription:\n\r&z%s\n\r", arena->desc );
}

void do_deploy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA * room;
    ARENA_DATA *arena;
    int tar=0, vnum=0, count=0;

    if ( is_spectator( ch ) )
    {
      send_to_char( "&R[Spectator Mode]: You may not deploy yet. Please wait.\n\r", ch );
      return;
    }

    if ( IN_VENT( ch ) )
    {
      send_to_char( "&RYou cannot deploy while in a vent.\n\r", ch );
      return;
    }

    if ( ( xIS_SET( ch->in_room->room_flags, ROOM_DEPLOY_A ) && ch->race == RACE_ALIEN )
      || ( xIS_SET( ch->in_room->room_flags, ROOM_DEPLOY_M ) && ch->race == RACE_MARINE )
      || ( xIS_SET( ch->in_room->room_flags, ROOM_DEPLOY_P ) && ch->race == RACE_PREDATOR ) )
    {
        ch_printf( ch, "\n\r...Now leaving the Battleground. Chicken...\n\r" );
        char_from_room( ch );
        char_to_room( ch, get_room_index( wherehome( ch ) ) );
        return;
    }

    if ( !xIS_SET( ch->in_room->room_flags, ROOM_DEPLOY_FROM ) )
    {
        send_to_char( "&RYou can't deploy from your current location.\n\r", ch );
        return;
    }

    if ( !curr_arena )
    {
        send_to_char( "&RPlease wait until an arena is loaded.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "&zSyntax: &WDeploy <destination ID>\n\r", ch );

        /* List destinations */
        for ( vnum = curr_arena->firstroom; vnum < curr_arena->lastroom; vnum++ )
        {
              if ( ( room = get_room_index( vnum ) ) == NULL ) continue;

              if ( ch->race == RACE_ALIEN && xIS_SET( room->room_flags, ROOM_DEPLOY_A ) )
                ch_printf( ch, "&C#%d&z) &W%s\n\r", ++count, (room->name) );
              if ( ch->race == RACE_MARINE && xIS_SET( room->room_flags, ROOM_DEPLOY_M ) )
                ch_printf( ch, "&C#%d&z) &W%s\n\r", ++count, (room->name) );
              if ( ch->race == RACE_PREDATOR && xIS_SET( room->room_flags, ROOM_DEPLOY_P ) )
                ch_printf( ch, "&C#%d&z) &W%s\n\r", ++count, (room->name) );
        }

        if ( count <= 0 )
        {
              ch_printf( ch, "&C(No Destinations Found - Sorry)\n\r" );
        }

        return;
    }

    tar = atoi( argument );

    for ( vnum = curr_arena->firstroom; vnum < curr_arena->lastroom; vnum++ )
    {
         if ( ( room = get_room_index( vnum ) ) == NULL ) continue;

         if ( ch->race == RACE_ALIEN && xIS_SET( room->room_flags, ROOM_DEPLOY_A ) )
         if ( ++count == tar ) break;
         if ( ch->race == RACE_MARINE && xIS_SET( room->room_flags, ROOM_DEPLOY_M ) )
         if ( ++count == tar ) break;
         if ( ch->race == RACE_PREDATOR && xIS_SET( room->room_flags, ROOM_DEPLOY_P ) )
         if ( ++count == tar ) break;

         room = NULL;
    }

    if ( room != NULL )
    {
         sprintf( buf, "&C$n has deployed to location %d.", tar );
         act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );

         if ( ch->race == RACE_ALIEN )
            ch_printf( ch, "\n\r&RApproaching Warzone : %s\n\r", curr_arena->name );
         if ( ch->race == RACE_MARINE ) 
            ch_printf( ch, "\n\r&RDeploying to Combat Zone : %s\n\r", curr_arena->name );
         if ( ch->race == RACE_PREDATOR ) 
            ch_printf( ch, "\n\r&REntering the Proving Grounds : %s\n\r", curr_arena->name );
         char_from_room( ch );
         char_to_room( ch, room );

         if ( room->area ) motion_ping( room->x, room->y, room->z, room->area, ch );
    }
    else
    {
         do_deploy( ch, "" );
         return;
    }

    return;
}

void do_vote( CHAR_DATA * ch, char * argument )
{
   VOTER_DATA * vote = NULL;

   char arg1[MAX_STRING_LENGTH];
   char arg2[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
    
   if ( curr_vote->timer > 0 )
   {
      /* Active Vote */
      if ( arg1[0] == '\0' )
      {
         ch_printf( ch, "&z" );
         switch ( curr_vote->mode )
         {
            default:
              ch_printf( ch, "Current Vote: &wUnknown", curr_vote->data );
              break;
            case 1:
              ch_printf( ch, "Current Vote: &wSkip Current Map", curr_vote->data );
              break;     
         }

         ch_printf( ch, "\n\r&z Time Left:&W %d seconds.\n\r\n\r", curr_vote->timer );

         ch_printf( ch, "&zTotal Votes: &C%d &z(&GYea - %d&z, &RNay - %d&z)\n\r", curr_vote->votes, curr_vote->yea, curr_vote->nay );
         ch_printf( ch, "&zMinimum Votes: &C%d votes are required.\n\r", UMAX( 1, count_players() / 2 ) );
         ch_printf( ch, "\n\r&RSyntax: VOTE (Yea or Yes, Nay or No)\n\r" );

         return;
      }

      if  ( ( !str_cmp( arg1, "stop" ) || !str_cmp( arg1, "cancel" ) ) && IS_IMMORTAL( ch ) )
      {
         send_monitor( NULL, "&RThe current vote has been stopped by the Immortals." );
         reset_vote( );
      }
      else if ( !str_cmp( arg1, "yes" ) || !str_cmp( arg1, "yea" ) )
      {
         /* Log a YES Vote */
         log_vote( ch, TRUE );
      }
      else if ( !str_cmp( arg1, "no" ) || !str_cmp( arg1, "nay" ) )
      {
         /* Log a NO Vote */
         log_vote( ch, FALSE );
      }
      else
      {
         do_vote( ch, "" );
      }

      return;
   }
   else
   {
      /* Start a vote */
      if ( arg1[0] == '\0' )
      {
         ch_printf( ch, "&zSyntax: VOTE (Subject) [Optional]\n\r\n\r" );
         ch_printf( ch, "&BSKIP_MAP: &wSkip to the next map in sequence.\n\r" );
         return;
      }

      reset_vote( );

      if ( !str_cmp( arg1, "SKIP_MAP" ) )
      {
         curr_vote->mode = 1;
      }
      else
      {
         do_vote( ch, "" );
         return;
      }

      curr_vote->post = ch;
      curr_vote->timer = 90;  /* 90 Seconds to a vote */

      // Alert the players
      sprintf( buf, "&GA new vote has been started by %s. Please cast your votes.", ch->name );
      send_monitor( NULL, buf );

      curr_vote->votes++;
   
      curr_vote->yea += 1;

      CREATE( vote, VOTER_DATA, 1 );
      LINK( vote, curr_vote->first_vote, curr_vote->last_vote, next, prev );
      vote->ch = ch;
   }
 
   return;
}

void update_votes( void )
{
   char buf[MAX_STRING_LENGTH];

   if ( curr_vote->timer > 0 )
   {
       if ( --curr_vote->timer <= 0 )
       {
           process_vote( );
       }
       else if ( curr_vote->votes >= count_players() )
       {
           process_vote( );
       }
       else if ( curr_vote->timer % 30 == 0 )
       {
           sprintf( buf, "&YCurrent vote closes in %d seconds. Cast your votes now.", curr_vote->timer );
           send_monitor( NULL, buf );
       }
   }
   return;
}

void process_vote( void )
{
   char buf[MAX_STRING_LENGTH];
   int min_votes = 0;

   min_votes = UMAX( 1, count_players() / 2 );

   /* Vote has come to a close */
   if ( curr_vote->votes < min_votes )
   {
      sprintf( buf, "&RThe current vote has failed, Only recieved %d of %d required votes.", curr_vote->votes, min_votes );
      send_monitor( NULL, buf );
   }
   else if ( curr_vote->yea > curr_vote->nay )
   {
      sprintf( buf, "&GThe current vote has passed, %d against %d.", curr_vote->yea, curr_vote->nay );
      send_monitor( NULL, buf );

      switch ( curr_vote->mode )
      {
         case 1:
           if ( curr_arena ) curr_arena->ctimer = 0;
           update_arena();
           break;
      }    
   }
   else
   {
      sprintf( buf, "&RThe current vote has failed, %d votes against %d.", curr_vote->nay, curr_vote->yea );
      send_monitor( NULL, buf );
   }

   reset_vote( );

   return;
}

void log_vote( CHAR_DATA * ch, bool yea )
{
   VOTER_DATA * vote = NULL;

   if ( IS_IMMORTAL( ch ) )
   {
      ch_printf( ch, "&RImmortals cannot participate in player-based votes.\n\r" );
      return;
   }

   for ( vote = curr_vote->first_vote; vote; vote = vote->next )
   {
      if ( vote->ch == ch )
      {
         ch_printf( ch, "&RYou have already cast your vote. Nice try.\n\r" );
         return;
      }
   }

   curr_vote->votes++;
   
   curr_vote->yea += (yea == TRUE) ? 1 : 0;
   curr_vote->nay += (yea == TRUE) ? 0 : 1;

   CREATE( vote, VOTER_DATA, 1 );
   LINK( vote, curr_vote->first_vote, curr_vote->last_vote, next, prev );
   vote->ch = ch;

   ch_printf( ch, "&GYour vote has been recorded. Please wait for the voting to finish.\n\r" );

   return;
}

void init_vote( void )
{
    curr_vote = NULL;

    CREATE( curr_vote, VOTE_DATA, 1 );

    curr_vote->timer = 0;
    curr_vote->mode = 0;
    curr_vote->post = NULL;

    curr_vote->votes = 0;
    curr_vote->yea = 0;
    curr_vote->nay = 0;

    curr_vote->first_vote = NULL;
    curr_vote->last_vote = NULL;

    return;
}

void reset_vote( void )
{
    VOTER_DATA * vTmp = NULL;
    VOTER_DATA * vNext = NULL;

    curr_vote->timer = 0;
    curr_vote->mode = 0;
    curr_vote->post = NULL;

    curr_vote->votes = 0;
    curr_vote->yea = 0;
    curr_vote->nay = 0;

    for ( vTmp = curr_vote->first_vote; vTmp; vTmp = vNext )
    {
       vNext = vTmp->next;

       vTmp->ch = NULL;

       UNLINK( vTmp, curr_vote->first_vote, curr_vote->last_vote, next, prev );
       DISPOSE( vTmp );
    }

    curr_vote->first_vote = NULL;
    curr_vote->last_vote = NULL;

    return;
}

