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
*			   Player movement module			   *
****************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"


#define BFS_MARK            1

const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 5, 7, 4
};

char *	const	dir_name	[]		=
{
    "north", "east", "south", "west", "up", "down",
    "northeast", "northwest", "southeast", "southwest", "somewhere"
};

const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 10
};


ROOM_INDEX_DATA * vroom_hash [64];


/*
 * Local functions.
 */
bool	has_key		args( ( CHAR_DATA *ch, int key ) );


char *	const		sect_names[SECT_MAX][2] =
{
    { "In a room","inside"	},	{ "A City Street","cities"      },
    { "In a field","fields"	},	{ "In a forest","forests"	},
    { "hill",	"hills"		},	{ "On a mountain","mountains"	},
    { "In the water","waters"	},	{ "In rough water","waters"	},
    { "Underwater", "underwaters" },	{ "In the air",	"air"		},
    { "In a desert","deserts"	},	{ "Somewhere",	"unknown"	},
    { "ocean floor", "ocean floor" },	{ "underground", "underground"	}
};


const	int		sent_total[SECT_MAX]		=
{
    4, 24, 4, 4, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1
};

char *	const		room_sents[SECT_MAX][25]	=
{
    {
	"The rough hewn walls are made of granite.",
	"You see an occasional spider crawling around.",
	"You notice signs of a recent battle from the bloodstains on the floor.",
	"This place hasa damp musty odour not unlike rotting vegetation."
    },

    {
	"You notice the occasional stray looking for food.",
	"Tall buildings loom on either side of you stretching to the sky.",
	"Some street people are putting on an interesting display of talent trying to earn some credits.",
	"Two people nearby shout heated words of argument at one another.",
	"You think you can make out several shady figures talking down a dark alleyway."
        "A slight breeze blows through the tall buildings.",
        "A small crowd of people have gathered at one side of the street.",
        "Clouds far above you obscure the tops of the highest skyscrapers.",
        "A speeder moves slowly through the street avoiding pedestrians.",
        "A cloudcar flys by overhead.",
        "The air is thick and hard to breath.",
        "The many smells of the city assault your senses.",
        "You hear a scream far of in the distance.",
        "The buildings around you seem endless in number.",
        "The city stretches seemingly endless in all directions.",
        "The street is wide and long.",
        "A swoop rider passes quickly by weaving in and out of pedestrians and other vehicles.",
        "The surface of the road is worn from many travellers.",
        "You feel it would be very easy to get lost in such an enormous city.",
        "You can see other streets above and bellow this one running in many directions.",
        "There are entrances to several buildings at this level.",
        "Along the edge of the street railings prevent pedestrians from falling to their death.",
        "In between the many towers you can see down into depths of the lower city.",
        "A grate in the street prevents rainwater from building up.",
        "You can see you reflection in several of the transparisteel windows as you pass by."
        "You hear a scream far of in the distance.",
    },

    {
	"You notice sparce patches of brush and shrubs.",
	"There is a small cluster of trees far off in the distance.",
	"Around you are grassy fields as far as the eye can see.",
	"Throughout the plains a wide variety of weeds and wildflowers are scattered."
    },

    {
	"Tall, dark evergreens prevent you from seeing very far.",
	"Many huge oak trees that look several hundred years old are here.",
	"You notice a solitary lonely weeping willow.",
	"To your left is a patch of bright white birch trees, slender and tall."
    },

    {
	"The rolling hills are lightly speckled with violet wildflowers."
    },

    {
	"The rocky mountain pass offers many hiding places."
    },

    {
	"The water is smooth as glass."
    },

    {
	"Rough waves splash about angrily."
    },

    {
	"A small school of fish swims by."
    },

    {
	"The land is far far below.",
	"A misty haze of clouds drifts by."
    },

    {
	"Around you is sand as far as the eye can see.",
	"You think you see an oasis far in the distance."
    },

    {
	"You notice nothing unusual."
    },

    { 
        "There are many rocks and coral which litter the ocean floor."
    },

    {
	"You stand in a lengthy tunnel of rock."
    }

};

int wherehome( CHAR_DATA *ch )
{
    if( get_trust(ch) >= LEVEL_IMMORTAL )
       return ROOM_START_IMMORTAL;

    if ( ch->pcdata )
    {
       if ( ch->pcdata->respawn_loc != NULL )
            return ch->pcdata->respawn_loc->vnum;
    }

    if ( ch->race == RACE_ALIEN ) return 4417;
    if ( ch->race == RACE_MARINE ) return 669;
    if ( ch->race == RACE_PREDATOR ) return 4102;

    /*
    if( ch->race  == RACE_HUMAN)
       return ROOM_START_HUMAN;
    */

    return ROOM_VNUM_LIMBO;   
}

char *grab_word( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
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
	*arg_first++ = *argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

char *wordwrap( char *txt, sh_int wrap )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufp;

    buf[0] = '\0';
    bufp = buf;
    if ( txt != NULL )
    {
        char line[MAX_STRING_LENGTH];
        char temp[MAX_STRING_LENGTH];
        char *ptr, *p;
        int ln, x;

	++bufp;
        line[0] = '\0';
        ptr = txt;
        while ( *ptr )
        {
	  ptr = grab_word( ptr, temp );
          ln = strlen( line );  x = strlen( temp );
          if ( (ln + x + 1) < wrap )
          {
	    if ( line[ln-1] == '.' )
              strcat( line, "  " );
	    else
              strcat( line, " " );
            strcat( line, temp );
            p = strchr( line, '\n' );
            if ( !p )
              p = strchr( line, '\r' );
            if ( p )
            {
                strcat( buf, line );
                line[0] = '\0';
            }
          }
          else
          {
            strcat( line, "\r\n" );
            strcat( buf, line );
            strcpy( line, temp );
          }
        }
        if ( line[0] != '\0' )
            strcat( buf, line );
    }
    return bufp;
}

void decorate_room( ROOM_INDEX_DATA *room )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int nRand;
    int iRand, len;
    int previous[8];
    int sector = room->sector_type;

    if ( room->name )
      STRFREE( room->name );
    if ( room->description )
      STRFREE( room->description );
    if ( room->hdescription )
      STRFREE( room->hdescription );

    room->name	= STRALLOC( sect_names[sector][0] );
    buf[0] = '\0';
    nRand = number_range( 1, UMIN(8,sent_total[sector]) );

    for ( iRand = 0; iRand < nRand; iRand++ )
	previous[iRand] = -1;

    for ( iRand = 0; iRand < nRand; iRand++ )
    {
	while ( previous[iRand] == -1 )
	{
	    int x, z;

	    x = number_range( 0, sent_total[sector]-1 );

	    for ( z = 0; z < iRand; z++ )
		if ( previous[z] == x )
		  break;

	    if ( z < iRand )
		  continue;

	    previous[iRand] = x;

	    len = strlen(buf);
	    sprintf( buf2, "%s", room_sents[sector][x] );
	    if ( len > 5 && buf[len-1] == '.' )
	    {
		strcat( buf, "  " );
		buf2[0] = UPPER(buf2[0] );
	    }
	    else
	    if ( len == 0 )
	        buf2[0] = UPPER(buf2[0] );
	    strcat( buf, buf2 );
	}
    }
    sprintf( buf2, "%s\n\r", wordwrap(buf, 78) );
    room->description = STRALLOC( buf2 );
    room->hdescription = STRALLOC( buf2 );
}

/*
 * Remove any unused virtual rooms				-Thoric
 */
void clear_vrooms( )
{
    int hash;
    ROOM_INDEX_DATA *room, *room_next, *prev;

    for ( hash = 0; hash < 64; hash++ )
    {
	while ( vroom_hash[hash]
	&&     !vroom_hash[hash]->first_person
	&&     !vroom_hash[hash]->first_content )
	{
	    room = vroom_hash[hash];
	    vroom_hash[hash] = room->next;
	    clean_room( room );
	    DISPOSE( room );
	    --top_vroom;
	}
	prev = NULL;
	for ( room = vroom_hash[hash]; room; room = room_next )
	{
	    room_next = room->next;
            if ( !room->first_person && !room->first_content )
	    {
		if ( prev )
		  prev->next = room_next;
		clean_room( room );
		DISPOSE( room );
		--top_vroom;
	    }
	    if ( room )
	      prev = room;
	}
    }
}

char *rev_exit( sh_int vdir )
{
    switch( vdir )
    {
	default: return "somewhere";
	case 0:  return "the south";
	case 1:  return "the west";
	case 2:  return "the north";
	case 3:  return "the east";
	case 4:  return "below";
	case 5:  return "above";
	case 6:  return "the southwest";
	case 7:  return "the southeast";
	case 8:  return "the northwest";
	case 9:  return "the northeast";
    }

    return "<???>";
}

char *main_exit( sh_int vdir )
{
    switch( vdir )
    {
	default: return "somewhere";
        case 0:  return "the north";
        case 1:  return "the east";
        case 2:  return "the south";
        case 3:  return "the west";
        case 4:  return "above";
        case 5:  return "below";
        case 6:  return "the northeast";
        case 7:  return "the northwest";
        case 8:  return "the southeast";
        case 9:  return "the southwest";
    }

    return "<???>";
}

/*
 * Function to get the equivelant exit of DIR 0-MAXDIR out of linked list.
 * Made to allow old-style diku-merc exit functions to work.	-Thoric
 */
EXIT_DATA *get_exit( ROOM_INDEX_DATA *room, sh_int dir )
{
    EXIT_DATA *xit;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
       if ( xit->vdir == dir )
         return xit;
    return NULL;
}

/*
 * Function to get an exit, leading the the specified room
 */
EXIT_DATA *get_exit_to( ROOM_INDEX_DATA *room, sh_int dir, int vnum )
{
    EXIT_DATA *xit;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
       if ( xit->vdir == dir && xit->vnum == vnum )
         return xit;
    return NULL;
}

/*
 * Function to get the nth exit of a room			-Thoric
 */
EXIT_DATA *get_exit_num( ROOM_INDEX_DATA *room, sh_int count )
{
    EXIT_DATA *xit;
    int cnt;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (cnt = 0, xit = room->first_exit; xit; xit = xit->next )
       if ( ++cnt == count )
         return xit;
    return NULL;
}


/*
 * Modify movement due to encumbrance				-Thoric
 */
sh_int encumbrance( CHAR_DATA *ch, sh_int move )
{
    int cur, max;

    max = can_carry_w(ch);
    cur = ch->carry_weight;
    if ( cur >= max )
      return move * 10;
    else
    if ( cur >= max * 0.95 )
      return move * 8.5;
    else
    if ( cur >= max * 0.90 )
      return move * 7;
    else
    if ( cur >= max * 0.85 )
      return move * 4.5;
    else
    if ( cur >= max * 0.80 )
      return move * 3;
    else
    if ( cur >= max * 0.75 )
      return move * 2;
    else
      return move;
}


/*
 * Check to see if a character can fall down, checks for looping   -Thoric
 */
bool will_fall( CHAR_DATA *ch, int fall )
{
    if ( is_spectator( ch ) ) return FALSE;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_NOFLOOR )
    &&   CAN_GO(ch, DIR_DOWN)
    && (!IS_AFFECTED( ch, AFF_FLYING )
    || ( ( ch->carried && !IS_AFFECTED( ch->carried, AFF_FLYING ) )
    || ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLYING ) ) ) ) )
    {
	if ( fall > 80 )
	{
	   bug( "Falling (in a loop?) more than 80 rooms: vnum %d", ch->in_room->vnum );
	   char_from_room( ch );
	   char_to_room( ch, get_room_index( wherehome(ch) ) );
	   fall = 0;
	   return TRUE;
	}
	set_char_color( AT_FALLING, ch );
	send_to_char( "You're falling down...\n\r", ch );
        move_char( ch, get_exit(ch->in_room, DIR_DOWN), DIR_DOWN, ++fall );
        abort_follow( ch );
	return TRUE;
    }
    return FALSE;
}


/*
 * create a 'virtual' room					-Thoric
 */
ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, EXIT_DATA **pexit )
{
    EXIT_DATA *xit, *bxit;
    EXIT_DATA *orig_exit = (EXIT_DATA *) *pexit;
    ROOM_INDEX_DATA *room, *backroom;
    int brvnum;
    int serial;
    int roomnum;
    int distance = -1;
    int vdir = orig_exit->vdir;
    sh_int hash;
    bool found = FALSE;

    if ( in_room->vnum > MAX_VNUMS )  /* room is virtual */
    {
	serial = in_room->vnum;
	roomnum = in_room->tele_vnum;
	if ( (serial & MAX_VNUMS) == orig_exit->vnum )
	{
	  brvnum = serial >> 16;
	  --roomnum;
	  distance = roomnum;
	}
	else
	{
	  brvnum = serial & MAX_VNUMS;
	  ++roomnum;
	  distance = orig_exit->distance - 1;
	}
	backroom = get_room_index( brvnum );
    }
    else
    {
	int r1 = in_room->vnum;
	int r2 = orig_exit->vnum;

	brvnum = r1;
	backroom = in_room;
	serial = (UMAX( r1, r2 ) << 16) | UMIN( r1, r2 );
	distance = orig_exit->distance - 1;
	roomnum = r1 < r2 ? 1 : distance;
    }
    hash = serial % 64;
    
    for ( room = vroom_hash[hash]; room; room = room->next )
	if ( room->vnum == serial && room->tele_vnum == roomnum )
	{
	    found = TRUE;
	    break;
	}
    if ( !found )
    {
	CREATE( room, ROOM_INDEX_DATA, 1 );
	room->area	  = in_room->area;
	room->vnum	  = serial;
	room->tele_vnum	  = roomnum;
	room->sector_type = in_room->sector_type;
	room->room_flags  = in_room->room_flags;
	decorate_room( room );
	room->next	  = vroom_hash[hash];
	vroom_hash[hash]  = room;
	++top_vroom;
    }
    if ( !found || (xit=get_exit(room, vdir))==NULL )
    {
	xit = make_exit(room, orig_exit->to_room, vdir);
	xit->keyword		= STRALLOC( "" );
	xit->description	= STRALLOC( "" );
	xit->key		= -1;
	xit->distance = distance;
    }
    if ( !found )
    {
	bxit = make_exit(room, backroom, rev_dir[vdir]);
	bxit->keyword		= STRALLOC( "" );
	bxit->description	= STRALLOC( "" );
	bxit->key		= -1;
	if ( (serial & MAX_VNUMS) != orig_exit->vnum )
	  bxit->distance = roomnum;
	else
	{
	  EXIT_DATA *tmp;
	  int fulldist;
	  if ( ( tmp = get_exit( backroom, vdir )) !=NULL)
	  {
	    fulldist = tmp->distance;
	  }
	  
	  bxit->distance = fulldist - distance;
	}
    }
    (EXIT_DATA *) pexit = xit;
    return room;
}

ch_ret move_char( CHAR_DATA *ch, EXIT_DATA *pexit, int edir, int fall )
{
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    char buf[MAX_STRING_LENGTH];
    char *txt;
    char *dtxt;
    CHAR_DATA *tmp;
    ch_ret retcode;
    sh_int door, distance;
    bool brief = FALSE;

    retcode = rNONE;
    txt = NULL;

    if ( IS_NPC(ch) && xIS_SET( ch->act, ACT_MOUNTED ) )
      return retcode;

    in_room = ch->in_room;
    from_room = in_room;
    if ( !pexit || (to_room = pexit->to_room) == NULL )
    {
        if ( IS_BUILDWALKING( ch ) )
        {
           ROOM_INDEX_DATA * room;
           EXIT_DATA * xit;

           room = bw_create( ch );

           if ( room == NULL ) return;

           xit = make_exit( ch->in_room, room, edir );
           xit->keyword                = STRALLOC( "" );
           xit->description            = STRALLOC( "" );
           xit->key                    = -1;
           xCLEAR_BITS(xit->exit_info);

           xit = make_exit( room, ch->in_room, rev_dir[edir] );
           xit->keyword                = STRALLOC( "" );
           xit->description            = STRALLOC( "" );
           xit->key                    = -1;
           xCLEAR_BITS(xit->exit_info);

           if ( get_exit(ch->in_room, edir) == NULL ) return rNONE;

           return move_char( ch, get_exit(ch->in_room, edir), edir, 0 );
        }
        else
        {
           send_to_char( "Alas, you cannot go that way.\n\r", ch );
           return rNONE;
        }
    }

    door = pexit->vdir;
    distance = pexit->distance;

    if ( xIS_SET( pexit->exit_info, EX_BLASTOPEN ) && !xIS_SET( pexit->exit_info, EX_BLASTED ) )
    {
        send_to_char( "Alas, you cannot go that way.\n\r", ch );
        return rNONE;
    }

    if ( xIS_SET( pexit->exit_info, EX_NOVENT ) && ch->vent )
    {
        send_to_char( "Alas, you cannot go that way.\n\r", ch );
        return rNONE;
    }

    /*
     * Exit is only a "window", there is no way to travel in that direction
     * unless it's a door with a window in it		-Thoric
     */
    if ( !is_spectator( ch ) && !ch->vent )
    {
        if ( xIS_SET( pexit->exit_info, EX_WINDOW )
        &&  ( ch->race != RACE_ALIEN || xIS_SET( pexit->exit_info, EX_BARRED ) )
        &&  !xIS_SET( pexit->exit_info, EX_ISDOOR ) )
        {
            send_to_char( "Alas, you cannot go that way.\n\r", ch );
            return rNONE;
        }

        if ( xIS_SET(pexit->exit_info, EX_PORTAL) && IS_NPC(ch) )
        {
            act( AT_PLAIN, "Mobs can't use portals.", ch, NULL, NULL, TO_CHAR );
            return rNONE;
        }

        if ( (xIS_SET(pexit->exit_info, EX_NOMOB) && IS_NPC(ch) && ch->master == NULL && ch->leader == NULL ) ||
           (IS_NPC(ch) && xIS_SET(to_room->room_flags,ROOM_NO_MOB) && ch->master == NULL && ch->leader == NULL ))
        {
            act( AT_PLAIN, "Mobs can't enter there.", ch, NULL, NULL, TO_CHAR );
            return rNONE;
        }

        if ( xIS_SET(pexit->exit_info, EX_CLOSED) && (!IS_AFFECTED(ch, AFF_PASS_DOOR) || xIS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
        {
            if ( !xIS_SET( pexit->exit_info, EX_SECRET ) && !xIS_SET( pexit->exit_info, EX_DIG ) )
            {
               act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
            } 
            else
            {
               send_to_char( "Alas, you cannot go that way.\n\r", ch );
            }
            return rNONE;
        }

        /*
         * Allow characters to block exits
         */
        for ( tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room )
        {
            if ( tmp == ch ) continue;
            if ( is_spectator( tmp ) ) continue;
            if ( tmp->block == NULL ) continue;
            if ( tmp->block == pexit )
            {
                ch_printf( ch, "You can't go that way, %s is blocking that exit.\n\r", PERS(tmp, ch) );
                return rNONE;
            }
        }
    }
    else
    {
        if ( is_spectator( ch ) && xIS_SET( pexit->exit_info, EX_NOSPEC ) )
        {
            send_to_char( "&RSpectators may not pass through that exit.\n\r", ch );
            return rNONE;
        }
    }

    /* Terminate hide */
    xREMOVE_BIT(ch->affected_by, AFF_HIDE);

    /* Can't block a door AND move at the same time */
    ch->block = NULL;

    /* Can't take cover AND move at the same time */
    remove_cover( ch, NULL );

    /*
     * Crazy virtual room idea, created upon demand.		-Thoric
     */
    if ( distance > 1 )
	if ( (to_room=generate_exit(in_room, &pexit)) == NULL )
	    send_to_char( "Alas, you cannot go that way.\n\r", ch );

    if ( !fall
    &&   IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "What?  And leave your beloved master?\n\r", ch );
	return rNONE;
    }

    if ( !fall && !IS_NPC(ch) )
    {
        int move;

        if ( in_room->sector_type == SECT_AIR || to_room->sector_type == SECT_AIR
        || xIS_SET( pexit->exit_info, EX_FLY ) )
	{
	    if ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLYING ) )
	    {
		send_to_char( "Your mount can't fly.\n\r", ch );
		return rNONE;
	    }
	    if ( !ch->mount && !IS_AFFECTED(ch, AFF_FLYING) )
	    {
		send_to_char( "You'd need to fly to go there.\n\r", ch );
		return rNONE;
	    }
	}

	if ( in_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_NOSWIM )
	{
	    bool found;

	    found = FALSE;
	    if ( ch->mount )
	    {
		if ( IS_AFFECTED( ch->mount, AFF_FLYING )
		||   IS_AFFECTED( ch->mount, AFF_FLOATING ) )
		  found = TRUE;
	    }
	    else
	    if ( IS_AFFECTED(ch, AFF_FLYING)
	    ||   IS_AFFECTED(ch, AFF_FLOATING) )
		found = TRUE;

            return rNONE;
	}

	if ( xIS_SET( pexit->exit_info, EX_CLIMB ) )
	{
	    bool found;

	    found = FALSE;
	    if ( ch->mount && IS_AFFECTED( ch->mount, AFF_FLYING ) )
	      found = TRUE;
	    else
	    if ( IS_AFFECTED(ch, AFF_FLYING) )
	      found = TRUE;

            if ( !found && !ch->mount && !is_spectator( ch ) )
	    {
                if ( ch->mental_state < -90 )
		{
		   send_to_char( "You start to climb... but lose your grip and fall!\n\r", ch);
		   if ( pexit->vdir == DIR_DOWN )
		   {
                        retcode = move_char( ch, pexit, DIR_DOWN, 1 );
                        abort_follow( ch );
			return retcode;
		   }
		   set_char_color( AT_HURT, ch );
                   send_to_char( "WHAM! You smash into the ground.\n\r", ch );
                   WAIT_STATE( ch, 10 );
                   skill_power( ch, gsn_near_miss, -1 );
                   retcode = damage( ch, ch, (pexit->vdir == DIR_UP ? 10 : 5), TYPE_UNDEFINED );
                   skill_power( ch, gsn_near_miss, 1 );
		   return retcode;
		}
		found = TRUE;
		txt = "climbs";
	    }

            if ( !found && !is_spectator( ch ) )
	    {
		send_to_char( "You can't climb.\n\r", ch );
		return rNONE;
	    }
	}

        if ( ch->mount && !is_spectator( ch ) )
	{
	  switch (ch->mount->position)
	  {
	    case POS_DEAD:
            send_to_char( "Your mount is dead!\n\r", ch );
	    return rNONE;
            break;
        
            case POS_MORTAL:
            case POS_INCAP:
            send_to_char( "Your mount is hurt far too badly to move.\n\r", ch );
	    return rNONE;
            break;
            
            case POS_STUNNED:
            send_to_char( "Your mount is too stunned to do that.\n\r", ch );
     	    return rNONE;
            break;
       
            case POS_SITTING:
            send_to_char( "Your mount is sitting down.\n\r", ch);
	    return rNONE;
            break;

	    default:
	    break;
  	  }

	  if ( !IS_AFFECTED(ch->mount, AFF_FLYING)
	  &&   !IS_AFFECTED(ch->mount, AFF_FLOATING) )
	    move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)];
	  else
	    move = 1;
	  if ( ch->mount->move < move )
	  {
	    send_to_char( "Your mount is too exhausted.\n\r", ch );
	    return rNONE;
	  }
	}
	else
	{
	  if ( !IS_AFFECTED(ch, AFF_FLYING)
	  &&   !IS_AFFECTED(ch, AFF_FLOATING) )
	    move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)] );
	  else
	    move = 1;
	  if ( ch->move < move )
	  {
            send_to_char( "You are too exhausted, you might want to rest a while.\n\r", ch );
	    return rNONE;
	  }
          if ( ch->mp < 1 )
	  {
            send_to_char( "You are too exhausted, rest for a moment.\n\r", ch );
            return rNONE;
	  }

	}

        WAIT_STATE( ch, 2 );

	if ( ch->mount )
	  ch->mount->move -= move;
        else
          if ( !IS_IMMORTAL( ch ) && !is_spectator( ch ) && !is_home( ch ) )
          {
              ch->move -= move;
              ch->mp--;
          }
    }

    /*
     * Check if player can fit in the room
     */
    if ( !is_spectator( ch ) && to_room->tunnel > 0 )
    {
	CHAR_DATA *ctmp;
	int count = ch->mount ? 1 : 0;
	
	for ( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
	  if ( ++count >= to_room->tunnel )
	  {
		if ( ch->mount && count == to_room->tunnel )
		  send_to_char( "There is no room for both you and your mount in there.\n\r", ch );
                else if ( ch->carrying && count == to_room->tunnel )
                  send_to_char( "There is no room for both you and the person your carrying in there.\n\r", ch );
		else
		  send_to_char( "There is no room for you in there.\n\r", ch );
		return rNONE;
	  }
    }


    if ( !IS_AFFECTED(ch, AFF_SNEAK) && ( IS_NPC(ch) || !xIS_SET(ch->act, PLR_WIZINVIS) ) && !is_spectator( ch ) )
    {
      if ( fall )
        txt = "falls";
      else
      if ( !txt )
      {
        if ( ch->mount )
        {
	  if ( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
	    txt = "floats";
 	  else
	  if ( IS_AFFECTED( ch->mount, AFF_FLYING ) )
	    txt = "flys";
	  else
	    txt = "rides";
        }
        else
        {
	  if ( IS_AFFECTED( ch, AFF_FLOATING ) )
	  {
            txt = "floats";
	  }
	  else
	  if ( IS_AFFECTED( ch, AFF_FLYING ) )
	  {
            txt = "flys";
	  }
	  else
          if ( ch->position == POS_SHOVE )
            txt = "is shoved";
 	  else
	  if ( ch->position == POS_DRAG )
            txt = "is dragged";
  	  else
	  {
            txt = "leaves";
	  }
        }
      }
      if ( ch->mount )
      {
	sprintf( buf, "$n %s %s upon $N.", txt, dir_name[door] );
	act( AT_ACTION, buf, ch, NULL, ch->mount, TO_NOTVICT );
      }
      else if ( xIS_SET(ch->in_room->room_flags, ROOM_HIVED) && ch->race == RACE_ALIEN )
      {
        sprintf( buf, "leaves %s", dir_name[door] );
        hive_message( ch, buf );
      }
      else if ( xIS_SET( ch->affected_by, AFF_CLOAK ) )
      {
        sprintf( buf, "leaves %s", dir_name[door] );
        cloak_message( ch, buf );
      }
      else if ( ch->carrying )
      {
        sprintf( buf, "$n %s %s carrying $N.", txt, dir_name[door] );
        act( AT_ACTION, buf, ch, NULL, ch->carrying, TO_NOTVICT );
      }
      else
      {
	sprintf( buf, "$n %s $T.", txt );
	act( AT_ACTION, buf, ch, NULL, dir_name[door], TO_ROOM );
      }
    }

    /*
    rprog_leave_trigger( ch );
    if( char_died(ch) )
      return global_retcode;
    */

    char_from_room( ch );
    if ( ch->mount )
    {
      /*
      rprog_leave_trigger( ch->mount );
      if( char_died(ch) )
        return global_retcode;
      */
      if( ch->mount )
      {
        char_from_room( ch->mount );
        char_to_room( ch->mount, to_room );
      }
    }
    if ( ch->carrying )
    {
      /*
      rprog_leave_trigger( ch->carrying );
      if( char_died(ch) )
        return global_retcode;
      */
      if( ch->carrying )
      {
        char_from_room( ch->carrying );
        char_to_room( ch->carrying, to_room );
      }
    }

    char_to_room( ch, to_room );

    if ( ch->race == RACE_MARINE && xIS_SET( to_room->room_flags, ROOM_HIVED ) )
    {
        /* Morale Penalty for travelling in the hive */
        drop_morale( ch, 1 );        
    }

    if ( !is_spectator( ch ) && !xIS_SET( to_room->room_flags, ROOM_PROTOTYPE ) )
    {
        if ( to_room->area )
        {
           motion_ping( to_room->x, to_room->y, to_room->z, to_room->area, ch );
        }
    }

    if ( !IS_AFFECTED(ch, AFF_SNEAK) && !is_spectator( ch )
    && ( IS_NPC(ch) || !xIS_SET(ch->act, PLR_WIZINVIS) ) )
    {
      if ( fall )
        txt = "falls";
      else
      if ( ch->mount )
      {
	if ( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
	  txt = "floats in";
	else
	if ( IS_AFFECTED( ch->mount, AFF_FLYING ) )
	  txt = "flys in";
	else
	  txt = "rides in";
      }
      else
      {
	if ( IS_AFFECTED( ch, AFF_FLOATING ) )
	{
          txt = "floats in";
	}
	else
	if ( IS_AFFECTED( ch, AFF_FLYING ) )
	{
          txt = "flys in";
	}
	else
	if ( ch->position == POS_SHOVE )
          txt = "is shoved in";
	else
	if ( ch->position == POS_DRAG )
	  txt = "is dragged in";
  	else
	{
          txt = "arrives";
	}
      }
      switch( door )
      {
      default: dtxt = "somewhere";	break;
      case 0:  dtxt = "the south";	break;
      case 1:  dtxt = "the west";	break;
      case 2:  dtxt = "the north";	break;
      case 3:  dtxt = "the east";	break;
      case 4:  dtxt = "below";		break;
      case 5:  dtxt = "above";		break;
      case 6:  dtxt = "the south-west";	break;
      case 7:  dtxt = "the south-east";	break;
      case 8:  dtxt = "the north-west";	break;
      case 9:  dtxt = "the north-east";	break;
      }
      if ( ch->mount )
      {
	sprintf( buf, "$n %s from %s upon $N.", txt, dtxt );
	act( AT_ACTION, buf, ch, NULL, ch->mount, TO_ROOM );
      }
      else if ( xIS_SET(ch->in_room->room_flags, ROOM_HIVED) && ch->race == RACE_ALIEN )
      {
        sprintf( buf, "enters from %s", dtxt );
        hive_message( ch, buf );
      }
      else if ( xIS_SET( ch->affected_by, AFF_CLOAK ) )
      {
        sprintf( buf, "enters from %s", dtxt );
        cloak_message( ch, buf );
      }
      else if ( ch->carrying )
      {
        sprintf( buf, "$n %s from %s carrying $N.", txt, dtxt );
        act( AT_ACTION, buf, ch, NULL, ch->carrying, TO_ROOM );
      }
      else
      {
	sprintf( buf, "$n %s from %s.", txt, dtxt );
	act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
      }
    }

    do_look( ch, "auto" );
    if ( brief ) 
      xSET_BIT( ch->act, PLR_BRIEF );

    detonate_traps( ch, to_room );

    /* BIG ugly looping problem here when the character is mptransed back
       to the starting room.  To avoid this, check how many chars are in 
       the room at the start and stop processing followers after doing
       the right number of them.  -- Narn
    */

    /* if ( !fall ) */

    /* AUTOMATIC FOLLOWERS HAVE BEEN SHUTDOWN */
    if ( 1 == 0 )
    {
      CHAR_DATA *fch;
      CHAR_DATA *nextinroom;
      int chars = 0, count = 0;

      for ( fch = from_room->first_person; fch; fch = fch->next_in_room )
        chars++;

      for ( fch = from_room->first_person; fch && ( count < chars ); fch = nextinroom )
      {
	nextinroom = fch->next_in_room;
        count++;
	if ( fch != ch		/* loop room bug fix here by Thoric */
	&& fch->master == ch
	&& fch->position == POS_STANDING )
	{
            act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
            move_char( fch, pexit, pexit->vdir, 0 );
	}
      }
    }


    if ( check_rescue( ch ) ) return retcode;

    if ( retcode != rNONE )
      return retcode;

    if ( char_died(ch) )
      return retcode;

    if ( !IN_VENT( ch ) )
    {
       mprog_entry_trigger( ch );
       if ( char_died(ch) ) return retcode;

       rprog_enter_trigger( ch );
       if ( char_died(ch) ) return retcode;

       mprog_greet_trigger( ch );
       if ( char_died(ch) ) return retcode;

       oprog_greet_trigger( ch );
       if ( char_died(ch) ) return retcode;
    }

    if ( !will_fall( ch, fall ) && fall > 0 )
    {
        if (!IS_AFFECTED( ch, AFF_FLOATING ) || ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLOATING ) ) )
	{
          if ( !xIS_SET( ch->in_room->room_flags, ROOM_FALLCATCH ) && ch->race != RACE_ALIEN )
          {
             set_char_color( AT_HURT, ch );
             send_to_char( "OUCH! You hit the ground!\n\r", ch );
             WAIT_STATE( ch, 20 );
             retcode = damage( ch, ch, (75 * fall), TYPE_UNDEFINED );
          }
          else if ( ch->race == RACE_ALIEN )
          {
             send_to_char( "&CYou hit the ground with a thud, but recover easily.\n\r", ch );
          }
          else
          {
             // send_to_char( "&CYou float lightly to the ground.\n\r", ch );
          }
	}
	else
	{
	  set_char_color( AT_MAGIC, ch );
	  send_to_char( "You lightly float down to the ground.\n\r", ch );
	}
    }
    return retcode;
}


void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTH), DIR_NORTH, 0 );
    abort_follow( ch );
    return;
}


void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_EAST), DIR_EAST, 0 );
    abort_follow( ch );
    return;
}


void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTH), DIR_SOUTH, 0 );
    abort_follow( ch );
    return;
}


void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_WEST), DIR_WEST, 0 );
    abort_follow( ch );
    return;
}


void do_up( CHAR_DATA *ch, char *argument )
{                                                 
    move_char( ch, get_exit(ch->in_room, DIR_UP), DIR_UP, 0 );
    abort_follow( ch );
    return;
}


void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_DOWN), DIR_DOWN, 0 );
    abort_follow( ch );
    return;
}

void do_northeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTHEAST), DIR_NORTHEAST, 0 );
    abort_follow( ch );
    return;
}

void do_northwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTHWEST), DIR_NORTHWEST, 0 );
    abort_follow( ch );
    return;
}

void do_southeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTHEAST), DIR_SOUTHEAST, 0 );
    abort_follow( ch );
    return;
}

void do_southwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTHWEST), DIR_SOUTHWEST, 0 );
    abort_follow( ch );
    return;
}



EXIT_DATA *find_door( CHAR_DATA *ch, char *arg, bool quiet )
{
    EXIT_DATA *pexit;
    int door;

    if (arg == NULL || !str_cmp(arg,""))
      return NULL;

    pexit = NULL;
	 if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "north"	  ) ) door = 0;
    else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east"	  ) ) door = 1;
    else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "south"	  ) ) door = 2;
    else if ( !str_cmp( arg, "w"  ) || !str_cmp( arg, "west"	  ) ) door = 3;
    else if ( !str_cmp( arg, "u"  ) || !str_cmp( arg, "up"	  ) ) door = 4;
    else if ( !str_cmp( arg, "d"  ) || !str_cmp( arg, "down"	  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = 6;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = 8;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = 9;
    else
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	{
	    if ( (quiet || xIS_SET(pexit->exit_info, EX_ISDOOR))
	    &&    pexit->keyword
	    &&    nifty_is_name( arg, pexit->keyword ) )
		return pexit;
	}
	if ( !quiet )
	  act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( (pexit = get_exit( ch->in_room, door )) == NULL )
    {
	if ( !quiet)
	  act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( quiet )
	return pexit;

    if ( xIS_SET(pexit->exit_info, EX_SECRET) )
    {
	act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( !xIS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "You can't do that.\n\r", ch );
	return NULL;
    }

    return pexit;
}


void toggle_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    xTOGGLE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	xTOGGLE_BIT( pexit_rev->exit_info, flag );
}

void set_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    xSET_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	xSET_BIT( pexit_rev->exit_info, flag );
}

void remove_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    xREMOVE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	xREMOVE_BIT( pexit_rev->exit_info, flag );
}

void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	return;
    }

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectators have no need to open or close doors.\n\r", ch );
        return;
    }        

    if ( ch->vent == TRUE )
    {
        send_to_char( "&RYou can't do that while in a vent, genius.\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'open door' */
	EXIT_DATA *pexit_rev;

	if ( !xIS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( !xIS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already open.\n\r",      ch ); return; }
	if (  xIS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's locked.\n\r",            ch ); return; }

	if ( !xIS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    act( AT_ACTION, "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	    act( AT_ACTION, "You open the $d.", ch, NULL, pexit->keyword, TO_CHAR );
	    if ( (pexit_rev = pexit->rexit) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
	    {
		CHAR_DATA *rch;

		for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
		    act( AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	        sound_to_room( pexit->to_room , "!!SOUND(door)" );
	    }
	    remove_bexit_flag( pexit, EX_CLOSED );
            
            sound_to_room( ch->in_room , "!!SOUND(door)" );
            return;
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'open object' */
	if ( obj->item_type != ITEM_CONTAINER )
	{ 
          ch_printf( ch, "%s isn't a container.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	{ 
          ch_printf( ch, "%s is already open.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	{ 
          ch_printf( ch, "%s cannot be opened or closed.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	{ 
          ch_printf( ch, "%s is locked.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act( AT_ACTION, "You open $p.", ch, obj, NULL, TO_CHAR );
	act( AT_ACTION, "$n opens $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	return;
    }

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectators have no need to open or close doors.\n\r", ch );
        return;
    }        

    if ( ch->vent == TRUE )
    {
        send_to_char( "&RYou can't do that while in a vent, genius.\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'close door' */
	EXIT_DATA *pexit_rev;

	if ( !xIS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( xIS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's already closed.\n\r",    ch ); return; }

	act( AT_ACTION, "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
	act( AT_ACTION, "You close the $d.", ch, NULL, pexit->keyword, TO_CHAR );

	/* close the other side */
	if ( ( pexit_rev = pexit->rexit ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    xSET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
		act( AT_ACTION, "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
	set_bexit_flag( pexit, EX_CLOSED );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	{ 
          ch_printf( ch, "%s isn't a container.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 
	if (IS_SET(obj->value[1], CONT_CLOSED) )
	{ 
          ch_printf( ch, "%s is already closed.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
        { 
          ch_printf( ch, "%s cannot be opened or closed.\n\r", capitalize( obj->short_descr ) ); 
          return;
        } 

	SET_BIT(obj->value[1], CONT_CLOSED);
	act( AT_ACTION, "You close $p.", ch, obj, NULL, TO_CHAR );
	act( AT_ACTION, "$n closes $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    return;
}


bool has_key( CHAR_DATA *ch, int key )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
     if ( obj->pIndexData->vnum == key || obj->value[0] == key )
       return TRUE;

    /* The Immortal Toolkit   -Ghost   */
    if ( IS_IMMORTAL(ch) )
    {
       sprintf( buf, "%s locked/unlocked door at %d without a key", ch->name, ch->in_room->vnum );
       log_string_plus( buf, LOG_NORMAL, ch->top_level );
       return TRUE;
    }

    return FALSE;
}


void do_setlock( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *area;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( is_spectator( ch ) )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "&RUsage: SETLOCK <direction> <old code> <new code>\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg1, TRUE ) ) != NULL )
    {
	if ( !xIS_SET(pexit->exit_info, EX_ISDOOR) )
            { send_to_char( "It only works on a door!\n\r",      ch ); return; }
        if ( !xIS_SET(pexit->exit_info, EX_KEYPAD) )
            { send_to_char( "The door isn't equiped with a keypad!\n\r",      ch ); return; }
        if ( pexit->key != atoi( arg2 ) )
            { send_to_char( "Thats not the current code!\n\r",      ch ); return; }

        if ( atoi( arg3 ) < 1000 || atoi( arg3 ) > 99999 )
            { send_to_char( "The code value must between 1000 and 99999!\n\r",      ch ); return; }
          
        pexit->key = atoi(arg3);

        echo_to_room( AT_YELLOW, ch->in_room, "The keypad display flicks on: 'Code changed'");

        if ( (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
        {
            pexit_rev->key = atoi(arg3);
            echo_to_room( AT_YELLOW, pexit->to_room, "The keypad display flicks on: 'Code changed'");
        }

        /* Save the area change */
        area = ch->in_room->area;

        fold_area( area, area->filename, FALSE );

        if ( pexit->to_room->area != area )
          fold_area( pexit->to_room->area, pexit->to_room->area->filename, FALSE );

        return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg1 );
    return;
}

void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;
    CHAR_DATA *rch;
    int code=0;

    argument = one_argument( argument, arg );

    if ( ch->race == RACE_ALIEN )
    {
        send_to_char( "Aliens can't lock doors.\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Lock what?\n\r", ch );
	return;
    }

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectators have no need to lock or unlock doors.\n\r", ch );
        return;
    }        

    if ( ch->vent == TRUE )
    {
        send_to_char( "&RYou can't do that while in a vent, genius.\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'lock door' */

	if ( !xIS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( !xIS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }

        if ( !xIS_SET(pexit->exit_info, EX_KEYPAD) )
        {
            if ( !has_key( ch, pexit->key) )
                 { send_to_char( "You lack the key.\n\r",       ch ); return; }
        }
        else
        {
            if ( argument[0] == '\0' )
            {
                 send_to_char("&RThis is a keypad-protected doorway.\n\r&RUsage: LOCK <direction> <key code>\n\r", ch );
                 return;
            }

            code = atoi( argument );

            if ( code != pexit->key )
            {
                 echo_to_room( AT_YELLOW, ch->in_room, "The keypad display flicks on: 'Invalid access code'");
                 if ( pexit->to_room )
                   echo_to_room( AT_YELLOW, pexit->to_room, "The keypad display flicks on: 'Invalid access code'");
                 return;
            }
        }

	if ( xIS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	if ( !xIS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    send_to_char( "*Click*\n\r", ch );
	    act( AT_ACTION, "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

        if ( (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
         for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
          act(AT_SKILL, "The $d *CLICKS* and locks.", rch, NULL, pexit_rev->keyword, TO_CHAR );         

	    set_bexit_flag( pexit, EX_LOCKED );
            return;
        }
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be locked.\n\r",     ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already locked.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( AT_ACTION, "$n locks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;
    CHAR_DATA *rch;
    int code=0;

    argument = one_argument( argument, arg );

    if ( ch->race == RACE_ALIEN )
    {
        send_to_char( "Aliens can't unlock doors.\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Unlock what?\n\r", ch );
	return;
    }

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectators have no need to lock or unlock doors.\n\r", ch );
        return;
    }        

    if ( ch->vent == TRUE )
    {
        send_to_char( "&RYou can't do that while in a vent, genius.\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'unlock door' */

	if ( !xIS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "You can't do that.\n\r",      ch ); return; }
	if ( !xIS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
        if ( !xIS_SET(pexit->exit_info, EX_KEYPAD) )
        {
            if ( !has_key( ch, pexit->key) )
                { send_to_char( "You lack the key.\n\r",       ch ); return; }
        }
        else
        {
            if ( argument[0] == '\0' )
            {
                 send_to_char("&RThis is a keypad-protected doorway.\n\r&RUsage: UNLOCK <direction> <key code>\n\r", ch );
                 return;
            }

            code = atoi( argument );

            if ( code != pexit->key )
            {
                 echo_to_room( AT_YELLOW, ch->in_room, "The keypad display flicks on: 'Invalid access code'");
                 if ( pexit->to_room )
                   echo_to_room( AT_YELLOW, pexit->to_room, "The keypad display flicks on: 'Invalid access code'");
                 return;
            }
        }

        if ( !xIS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	if ( !xIS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    send_to_char( "*Click*\n\r", ch );
	    act( AT_ACTION, "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

        if ( (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
         for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
          act(AT_SKILL, "The $d *CLICKS* and unlocks.", rch, NULL, pexit_rev->keyword, TO_CHAR );         

            remove_bexit_flag( pexit, EX_LOCKED );
            return;   
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "That's not a container.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "It's not closed.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
	if ( !has_key( ch, obj->value[2] ) )
	    { send_to_char( "You lack the key.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	act( AT_ACTION, "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
	return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}

void do_bashdoor( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    char       arg [ MAX_INPUT_LENGTH ];

    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }

    if ( IS_NPC( ch ) || ch->race == RACE_ALIEN )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "&RSyntax: BASHDOOR (direction)\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA       *pexit_rev;
        int              chance;
        int              cost;
        char            *keyword;

        if ( !xIS_SET( pexit->exit_info, EX_CLOSED ) )
        {
           send_to_char( "&RChill. Its already open.\n\r", ch );
           return;
        }

        if ( xIS_SET( pexit->exit_info, EX_ARMORED ) )
        {
           send_to_char( "&RHate to spoil the fun, but that door is ARMORED.\n\r", ch );
           return;
        }

        cost = ( 30 - get_curr_str( ch ) ) + 10;

        if ( ch->move < cost )
        {
           send_to_char( "&RYour too exhausted to even try.\n\r", ch );
           return;
        }

        ch->move -= cost;

        if ( xIS_SET( pexit->exit_info, EX_SECRET ) )
           keyword = "wall";
        else
           keyword = pexit->keyword;

        chance = ( get_curr_str( ch ) * 2 ) + 40;

        if ( !xIS_SET( pexit->exit_info, EX_BASHPROOF ) && number_percent( ) < chance )
        {
             xREMOVE_BIT( pexit->exit_info, EX_CLOSED );
             if ( xIS_SET( pexit->exit_info, EX_LOCKED ) ) xREMOVE_BIT( pexit->exit_info, EX_LOCKED );
             xSET_BIT( pexit->exit_info, EX_BASHED );

             act(AT_RED, "BAWHAM! You bashed open the $d!", ch, NULL, keyword, TO_CHAR );
             act(AT_GREEN, "$n bashes open the $d!", ch, NULL, keyword, TO_ROOM );

             if ( (to_room = pexit->to_room) != NULL && (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
             {
                 CHAR_DATA *rch;

                 xREMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
                 if ( xIS_SET( pexit_rev->exit_info, EX_LOCKED ) ) xREMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
                 xSET_BIT( pexit_rev->exit_info, EX_BASHED );

                 for ( rch = to_room->first_person; rch; rch = rch->next_in_room )
                 {
                      act(AT_GREEN, "The $d bursts open!", rch, NULL, pexit_rev->keyword, TO_CHAR );
                 }
             }
        }
        else
        {
             act(AT_RED, "WHAM! You bash against the $d, but it doesn't budge.", ch, NULL, keyword, TO_CHAR );
             act(AT_RED, "WHAM! $n bashes against the $d, but it holds strong.", ch, NULL, keyword, TO_ROOM );

             if ( (to_room = pexit->to_room) != NULL && (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room )
             {
                 CHAR_DATA *rch;

                 for ( rch = to_room->first_person; rch; rch = rch->next_in_room )
                 {
                      act(AT_RED, "The $d shudders from an impact!", rch, NULL, pexit_rev->keyword, TO_CHAR );
                 }
             }
	}
    }
    else
    {
        send_to_char( "&RIn case you didn't notice, thats a wall.\n\r", ch );
        return;
    }

    return;
}


void do_stand( CHAR_DATA *ch, char *argument )
{
    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }

    switch ( ch->position )
    {
    case POS_PRONE:
        WAIT_STATE( ch, 12 );
        send_to_char( "You gather yourself and stand up. &z(&W3 Round Delay&z)\n\r", ch );
        act( AT_ACTION, "$n rises from $s prone position.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_KNEELING:
        WAIT_STATE( ch, 4 );
        send_to_char( "You gather yourself and stand up. &z(&W1 Round Delay&z)\n\r", ch );
        act( AT_ACTION, "$n rises from $s kneeling position.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_STANDING;
        break;

    case POS_SITTING:
	send_to_char( "You move quickly to your feet.\n\r", ch );
	act( AT_ACTION, "$n rises up.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "You are already standing.\n\r", ch );
	break;
    }

    return;
}


void do_sit( CHAR_DATA *ch, char *argument )
{
    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }

    switch ( ch->position )
    {
      case POS_PRONE:
        send_to_char( "You can't sit up from prone, you need to stand first.\n\r", ch );
	break;

    case POS_KNEELING:
        send_to_char( "You sit down. &z(&W1 Round Delay&z)\n\r", ch );
	act( AT_ACTION, "$n sits down.", ch, NULL, NULL, TO_ROOM );
        WAIT_STATE( ch, 4 );
        ch->position = POS_SITTING;
	break;

    case POS_STANDING:
         send_to_char( "You sit down.\n\r", ch );
	act( AT_ACTION, "$n sits down.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SITTING;
        break;
    case POS_SITTING:
	send_to_char( "You are already sitting.\n\r", ch );
	return;

    case POS_MOUNTED:
        if ( !ch->mount )
        {
            ch->position = POS_SITTING;
            return;
        }
        send_to_char( "You are already sitting - on your mount.\n\r", ch );
        return;
    }

    return;
}


void do_kneel( CHAR_DATA *ch, char *argument )
{
    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }

    if ( ch->race == RACE_ALIEN ) { do_nothing( ch, "" ); return; }

    switch ( ch->position )
    {
     case POS_PRONE:
        send_to_char( "You raise yourself to a kneel. &z(&W2 Round Delay&z)\n\r", ch );
        act( AT_ACTION, "$n raise themself to a kneeling position.", ch, NULL, NULL, TO_ROOM );
        WAIT_STATE( ch, 8 );
        ch->block = NULL;
        ch->position = POS_KNEELING;
	break;

    case POS_KNEELING:
        send_to_char( "You are already kneeling.\n\r", ch );
	return;

    case POS_STANDING:
    case POS_SITTING:
        send_to_char( "You drop to one knee and scan the area. &z(&W1 Round Delay&z)\n\r", ch );
        act( AT_ACTION, "$n drops to one knee and scans the area.", ch, NULL, NULL, TO_ROOM );
        WAIT_STATE( ch, 4 );
        ch->block = NULL;
        ch->position = POS_KNEELING;
        break;

    case POS_MOUNTED:
        send_to_char( "You'd better dismount first.\n\r", ch );
        return;
    }

    // rprog_rest_trigger( ch );
    return;
}


void do_prone( CHAR_DATA *ch, char *argument )
{
    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }

    if ( ch->race == RACE_ALIEN ) { do_nothing( ch, "" ); return; }

    switch ( ch->position )
    {
    case POS_PRONE:
        send_to_char( "You are already lying prone.\n\r", ch );
	return;

    case POS_KNEELING:
        send_to_char( "You drop to the ground, and take aim. &z(&W2 Round Delay&z)\n\r", ch );
        act( AT_ACTION, "$n drops to the ground in preperation.", ch, NULL, NULL, TO_ROOM );
        WAIT_STATE( ch, 8 );
        ch->block = NULL;
        ch->position = POS_PRONE;
        break;

    case POS_SITTING:
    case POS_STANDING:
        send_to_char( "You drop to the ground, and take aim. &z(&W3 Round Delay&z)\n\r", ch );
        act( AT_ACTION, "$n drops to the ground in preperation.", ch, NULL, NULL, TO_ROOM );
        WAIT_STATE( ch, 12 );
        ch->block = NULL;
        ch->position = POS_PRONE;
        break;

    case POS_MOUNTED:
        send_to_char( "You really should dismount first.\n\r", ch );
        return;
    }

    // rprog_sleep_trigger( ch );
    return;
}



/*
 * teleport a character to another room
 */
void teleportch( CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool show )
{
    act( AT_ACTION, "$n disappears suddenly!", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, room );
    act( AT_ACTION, "$n arrives suddenly!", ch, NULL, NULL, TO_ROOM );
    if ( show )
      do_look( ch, "auto" );
}

void teleport( CHAR_DATA *ch, int room, int flags )
{
    CHAR_DATA *nch, *nch_next;
    ROOM_INDEX_DATA *pRoomIndex;
    bool show;

    pRoomIndex = get_room_index( room );
    if ( !pRoomIndex )
    {
	bug( "teleport: bad room vnum %d", room );
	return;
    }

    if ( IS_SET( flags, TELE_SHOWDESC ) )
      show = TRUE;
    else
      show = FALSE;
    if ( !IS_SET( flags, TELE_TRANSALL ) )
    {
	teleportch( ch, pRoomIndex, show );
	return;
    }
    for ( nch = ch->in_room->first_person; nch; nch = nch_next )
    {
	nch_next = nch->next_in_room;
	teleportch( nch, pRoomIndex, show );
    }
}

/*
 * "Climb" in a certain direction.				-Thoric
 */
void do_climb( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }

    found = FALSE;
    if ( argument[0] == '\0' )
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( xIS_SET( pexit->exit_info, EX_xCLIMB ) )
	    {
                move_char( ch, pexit, pexit->vdir, 0 );
                abort_follow( ch );
		return;
	    }
	send_to_char( "You cannot climb here.\n\r", ch );
	return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   xIS_SET( pexit->exit_info, EX_xCLIMB ))
    {
        move_char( ch, pexit, pexit->vdir, 0 );
        abort_follow( ch ); 
	return;
    }
    send_to_char( "You cannot climb there.\n\r", ch );
    return;
}

/*
 * "enter" something (moves through an exit)			-Thoric
 */
void do_enter( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
      if ( xIS_SET( ch->in_room->room_flags, ROOM_VENTED_A ) ||
           xIS_SET( ch->in_room->room_flags, ROOM_VENTED_B ) ||
           xIS_SET( ch->in_room->room_flags, ROOM_VENTED_C ) ||
           xIS_SET( ch->in_room->room_flags, ROOM_VENTED_D ) )
      {
          // ROOM_INDEX_DATA * toroom;

          if ( ch->vent == TRUE )
          {
              ch_printf( ch, "&RYour already inside a vent passage!\n\r" );
              return;
          }

          if ( ch->race != RACE_ALIEN )
          {
              ch_printf( ch, "&ROnly Aliens may enter vent passages.\n\r" );
              return;
          }

          if ( ch->mp < 1 )
	  {
            send_to_char( "You are too exhausted, rest for a moment.\n\r", ch );
            return rNONE;
	  }

          /*
          if ( ch->in_room->link == NULL )
          {
              ch_printf( ch, "&RThat vent doesn't look very safe.\n\r" );
              return;
          }
          */

          act( AT_PLAIN, "$n vanishes into the nearby vent.", ch, NULL, NULL, TO_ROOM );
          act( AT_PLAIN, "You vanish into the nearby vent.", ch, NULL, NULL, TO_CHAR );

          /*
          toroom = ch->in_room->link;
          char_from_room( ch );
          char_to_room( ch , toroom);
          */ 

          ch->vent = TRUE;
          ch->mp--;
          abort_follow( ch );
          ch->block = NULL;

          act( AT_PLAIN, "$n enters the vent from outside.", ch, NULL, NULL, TO_ROOM );
          do_look( ch , "auto" );

          if ( !is_spectator( ch ) && !xIS_SET( ch->in_room->room_flags, ROOM_PROTOTYPE ) )
          {
              if ( ch->in_room->area )
              {
                  motion_ping( ch->in_room->x, ch->in_room->y, ch->in_room->z, ch->in_room->area, ch );
              }
          }

          return;
      }
      for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
      {
        if ( xIS_SET( pexit->exit_info, EX_xENTER ) )
        {
           move_char( ch, pexit, pexit->vdir, 0 );
           abort_follow( ch );
           return;
        }
      }
      send_to_char( "You cannot find an entrance here.\n\r", ch );
      return;
    }

    if ( ( pexit = find_door( ch, arg1, TRUE ) ) != NULL && xIS_SET( pexit->exit_info, EX_xENTER ) )
    {
       move_char( ch, pexit, pexit->vdir, 0 );
       abort_follow( ch );
       return;
    }
    return;
}

/*
 * Leave through an exit.					-Thoric
 */
void do_leave( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( xIS_SET( pexit->exit_info, EX_xLEAVE ) )
	    {
                move_char( ch, pexit, pexit->vdir, 0 );
                abort_follow( ch );
		return;
            }
        if ( xIS_SET( ch->in_room->room_flags, ROOM_VENTED_A ) ||
             xIS_SET( ch->in_room->room_flags, ROOM_VENTED_B ) ||
             xIS_SET( ch->in_room->room_flags, ROOM_VENTED_C ) ||
             xIS_SET( ch->in_room->room_flags, ROOM_VENTED_D ) )
        {
                // ROOM_INDEX_DATA * toroom;
        
                if ( ch->vent == FALSE )
                {
                     ch_printf( ch, "&RYour not even in a vent passage!\n\r" );
                     return;
                }

                if ( ch->race != RACE_ALIEN )
                {
                     ch_printf( ch, "&ROnly Aliens may use vent passages.\n\r" );
                     return;
                }

                if ( ch->mp < 1 )
                {
                     send_to_char( "You are too exhausted, rest for a moment.\n\r", ch );
                     return rNONE;
                }

                /*
                if ( ch->in_room->link == NULL )
                {
                     ch_printf( ch, "&RThat vent doesn't look very safe.\n\r" );
                     return;
                }
                */

                act( AT_PLAIN, "$n lunges out of the vent.", ch, NULL, NULL, TO_ROOM );
                act( AT_PLAIN, "You lunge out of the vent.", ch, NULL, NULL, TO_CHAR );

                /*
                toroom = ch->in_room->link;
                char_from_room( ch );
                char_to_room( ch, toroom );
                */

                ch->vent = FALSE;
                ch->mp--;
                abort_follow( ch );
                ch->block = NULL;

                act( AT_PLAIN, "$n lunges from the vent.", ch, NULL, NULL, TO_ROOM );
                do_look( ch , "auto" );

                if ( !is_spectator( ch ) && !xIS_SET( ch->in_room->room_flags, ROOM_PROTOTYPE ) )
                {
                  if ( ch->in_room->area )
                  {
                     motion_ping( ch->in_room->x, ch->in_room->y, ch->in_room->z, ch->in_room->area, ch );
                  }
                }
                return;
        }
	return;
    }

    if ( (pexit = find_door( ch, arg1, TRUE )) != NULL
    &&   xIS_SET( pexit->exit_info, EX_xLEAVE ))
    {
        move_char( ch, pexit, pexit->vdir, 0 );
        abort_follow( ch );
	return;
    }
    return;
}

void do_shove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exit_dir;
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    bool nogo;
    ROOM_INDEX_DATA *to_room;    
    int chance;  

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }    

    if ( arg[0] == '\0' )
    {
	send_to_char( "Shove whom?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You shove yourself around, to no avail.\n\r", ch);
	return;
    }
    
    if ( (victim->position) != POS_STANDING )
    {
	act( AT_PLAIN, "$N isn't standing up.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Shove them in which direction?\n\r", ch);
	return;
    }

    exit_dir = get_dir( arg2 );
    victim->position = POS_SHOVE;
    nogo = FALSE;
    if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL )
      nogo = TRUE;
    else
    if ( xIS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
    ||   xIS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
      nogo = TRUE;
    if ( nogo )
    {
	send_to_char( "There's no exit in that direction.\n\r", ch );
        victim->position = POS_STANDING;
	return;
    }
    to_room = pexit->to_room;

    if ( IS_NPC(victim) )
    {
	send_to_char("You can only shove player characters.\n\r", ch);
	return;
    }
    
    if (ch->in_room->area != to_room->area
    &&  !in_hard_range( victim, to_room->area ) )
    {
      send_to_char("That character cannot enter that area.\n\r", ch);
      victim->position = POS_STANDING;
      return;
    }

chance = 50;

/* Add 3 points to chance for every str point above 15, subtract for 
below 15 */

chance += ((get_curr_str(ch) - 15) * 3);

chance += (ch->top_level - victim->top_level);
 
/* Debugging purposes - show percentage for testing */

/* sprintf(buf, "Shove percentage of %s = %d", ch->name, chance);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/

if (chance < number_percent( ))
{
  send_to_char("You failed.\n\r", ch);
  victim->position = POS_STANDING;
  return;
}
    act( AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR );
    act( AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT );
    move_char( victim, get_exit(ch->in_room,exit_dir), exit_dir, 0);
    abort_follow( victim );
    if ( !char_died(victim) )
      victim->position = POS_STANDING;
    WAIT_STATE(ch, 12);
    /* Remove protection from shove/drag if char shoves -- Blodkai */
}

void do_drag( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exit_dir;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    bool nogo;
    int chance;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drag whom?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("You take yourself by the scruff of your neck, but go nowhere.\n\r", ch);
	return; 
    }

    if ( IS_NPC(victim) )
    {
	send_to_char("You can only drag player characters.\n\r", ch);
	return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Drag them in which direction?\n\r", ch);
	return;
    }

    exit_dir = get_dir( arg2 );

    nogo = FALSE;
    if ( ( pexit = get_exit(ch->in_room, exit_dir) ) == NULL )
    {
        nogo = TRUE;
    }
    else if ( xIS_SET(pexit->exit_info, EX_CLOSED) && (!IS_AFFECTED(victim, AFF_PASS_DOOR) || xIS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
    {
        nogo = TRUE;
    }

    if ( nogo )
    {
	send_to_char( "There's no exit in that direction.\n\r", ch );
	return;
    }

    to_room = pexit->to_room;

    if ( ch->in_room->area != to_room->area && !in_hard_range( victim, to_room->area ) )
    {
      send_to_char("That character cannot enter that area.\n\r", ch);
      return;
    }
    
    chance = 50;

    if ( !IS_AWAKE( victim ) ) chance = 100;

    /*
     sprintf(buf, "Drag percentage of %s = %d", ch->name, chance);
     act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
     */

    if ( chance < number_percent( ) )
    {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if ( victim->position < POS_STANDING )
    {
	act( AT_ACTION, "You drag $M into the next room.", ch, NULL, victim, TO_CHAR ); 
        act( AT_ACTION, "$n grabs you and drags you.", ch, NULL, victim, TO_VICT );
        char_from_room( victim );
        char_to_room( victim, to_room );
        move_char( ch, get_exit(ch->in_room,exit_dir), exit_dir, 0);
        abort_follow( ch );
	WAIT_STATE(ch, 12);
	return;
    }
    send_to_char("You cannot do that to someone who is standing.\n\r", ch);
    return;
}

/*
 * CARRY Command
 *  Rules:
 *   weight = 120(player) + Total EQ Weight
 *   Must be stunned.
 */
void do_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int weight=0;

    strcpy( arg, argument );

    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }

    if ( arg[0] == '\0' )
    {
        send_to_char( "Carry whom?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("You take yourself by the scruff of your neck, but go nowhere.\n\r", ch);
	return; 
    }

    if ( ch->carrying != NULL )
    {
        send_to_char( "You may only carry one person at a time!\n\r", ch );
        return;
    }

    if ( ch->carried != NULL )
    {
        send_to_char( "Not going to happen! Get your buddy to set you down!\n\r", ch );
        return;
    }

    if ( victim->carried != NULL )
    {
        send_to_char( "Somebody is already carrying the poor sap. Sorry about that.\n\r", ch );
        return;
    }   

    weight = 120 + victim->carry_weight;

    if ( weight + ch->carry_weight > can_carry_w( ch ) )
    {
        send_to_char( "They weigh too much for you to carry! Try getting rid of their stuff.\n\r", ch );
        return;
    }

    if ( IS_AWAKE(victim) )
    {
        send_to_char( "They look a little too lively to carry around!\n\r", ch );
        return;
    }

    // act( AT_LBLUE, "Blue rings of energy from $N's blaster hit you but have little effect", victim, NULL, ch, TO_CHAR );
    act( AT_LBLUE, "You pick $N up and throw $M over your shoulder.", ch, NULL, victim, TO_CHAR );
    act( AT_LBLUE, "$n picks up $N and throws $M over $s shoulder.", ch, NULL, victim, TO_NOTVICT );

    /* Set the pointers */
    ch->carrying = victim;
    victim->carried = ch;
    
    WAIT_STATE(ch, 4);

    return;
}


/*
 * RELEASE Command
 */
void do_release( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }

    if ( ch->carrying == NULL )
    {
        send_to_char( "Your not carrying anybody! Use CARRY before you can release someone.\n\r", ch );
        return;
    }

    if ( ch->carried != NULL )
    {
        bug("[ERROR]: Carrier is being CARRIED!" );
        send_to_char( "Thats a odd glitch. Who blackjacked you?\n\r", ch );
        return;
    }

    victim = ch->carrying;

    if ( victim->carried == NULL )
    {
        bug("[ERROR]: Carried player ISN'T!" );
        send_to_char( "Thats a odd glitch. Whos hauling you around?\n\r", ch );
        return;
    }   

    // act( AT_LBLUE, "Blue rings of energy from $N's blaster hit you but have little effect", victim, NULL, ch, TO_CHAR );
    act( AT_LBLUE, "You release $N, dropping $M on the ground.", ch, NULL, victim, TO_CHAR );
    act( AT_LBLUE, "$n releases $N, dropping $M on the ground.", ch, NULL, victim, TO_NOTVICT );

    /* Set the pointers */
    ch->carrying = NULL;
    victim->carried = NULL;
    
    WAIT_STATE(ch, 4);

    return;
}

void do_block( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *area;
    char arg1[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    argument = one_argument( argument, arg1 );

    if ( is_spectator( ch ) ) { do_nothing( ch, "" ); return; }

    if ( ch->vent == TRUE )
    {
        send_to_char( "&RYou can't do that while in a vent, genius.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' )
    {
        if ( ch->block == NULL )
        {
             send_to_char( "&RUsage: BLOCK <direction>\n\r", ch );
             send_to_char( "&R Blocks people from leaving in the specified direction.\n\r", ch );
             return;
        }
        else
        {
             ch_printf( ch, "&CYou step away from the %s exit.\n\r", dir_name[ch->block->vdir] );
             act( AT_ACTION, "$n moves away from the $T exit.", ch, NULL, dir_name[ch->block->vdir], TO_ROOM );
             ch->block = NULL;
             return;
        }
    }

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        send_to_char( "&RYour not really going to block in a safe room, are you?\n\r", ch );
        return;
    }

    if ( ch->block != NULL )
    {
        send_to_char( "&RYour already blocking an exit. Type BLOCK to stop.\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg1, TRUE ) ) != NULL )
    {
        if ( pexit->vdir == DIR_UP || pexit->vdir == DIR_DOWN )
        {
             send_to_char( "You cannot block above or below exits.\n\r", ch );
             return;
        }

        if ( xIS_SET(pexit->exit_info, EX_CLOSED) )
        {
             send_to_char( "No point in blocking a closed door!\n\r", ch );
             return;
        }

        ch_printf( ch, "&CYou move to block the %s exit.\n\r", dir_name[pexit->vdir] );
        act( AT_ACTION, "$n moves to block the $T exit.", ch, NULL, dir_name[pexit->vdir], TO_ROOM );

	/* Take away Hide */
        xREMOVE_BIT(ch->affected_by, AFF_HIDE);

        ch->block = pexit;
        return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg1 );
    return;
}

int flip_dir( int dir )
{
    switch ( dir )
    {
       case 0:
       case 1:
         return (dir + 2);
       case 2:
       case 3:
         return (dir - 2);
       case 4:
       case 7:
         return (dir + 1);
       case 5:
       case 8:
         return (dir - 1);
       case 6:
         return (dir + 3);
       case 9:
         return (dir - 3);
    }

    return 0;
}

void do_respawn( CHAR_DATA * ch, char * argument )
{
    return;
}
                 
void cloak_message( CHAR_DATA * ch, char * msg )
{
    char bufA[MAX_STRING_LENGTH]; // Sent to marines who can't see it.
    char bufB[MAX_STRING_LENGTH]; // Sent to blind players.
    char bufC[MAX_STRING_LENGTH]; // Sent to everyone else.
    CHAR_DATA * rch;
    int safe = 0;

    sprintf( bufA, "&w&CThe air shimmers as something passes nearby.\n\r" );
    sprintf( bufB, "Someone %s.\n\r", msg );
    sprintf( bufC, "A Predator %s.\n\r", msg );
    
    if ( ch->in_room == NULL ) return;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( ++safe > 9999 ) { bug( "cloak_message: Failsafe cutout." ); return; }

        if ( rch == ch ) continue;
        if ( rch->hit < 0 ) continue;
        if ( rch->vent != ch->vent ) continue;

        if ( IS_AFFECTED( rch, AFF_BLIND ) )
        {
            send_to_char( bufB, rch );
        }
        else if ( !can_see( rch, ch ) )
        {
            send_to_char( bufA, rch );
        }
        else
        {
            send_to_char( bufC, rch );
        }
    }

    return;
}

void hive_message( CHAR_DATA * ch, char * msg )
{
    char bufA[MAX_STRING_LENGTH]; // Sent to other aliens.
    char bufB[MAX_STRING_LENGTH]; // Sent to blind players.
    char bufC[MAX_STRING_LENGTH]; // Sent to everyone else.
    CHAR_DATA * rch;
    int safe = 0;

    sprintf( bufA, "%s %s.\n\r", ch->name, msg );
    sprintf( bufB, "Someone %s.\n\r", msg );
    sprintf( bufC, "An Alien %s.\n\r", msg );
    
    if ( ch->in_room == NULL ) return;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( ++safe > 9999 ) { bug( "hive_message: Failsafe cutout." ); return; }

        if ( rch == ch ) continue;
        if ( rch->vent != ch->vent ) continue;

        if ( rch->vision == ch->race )
        {
            send_to_char( bufC, rch );
        }
        else if ( rch->race != RACE_ALIEN )
        {
            // Natural sneak... send no message.
            continue;
        }
        else if ( IS_AFFECTED( rch, AFF_BLIND ) || !can_see( rch, ch ) )
        {
            send_to_char( bufB, rch );
        }
        else
        {
            if ( IS_NPC( ch ) )
              send_to_char( bufC, rch );
            else
              send_to_char( bufA, rch );
        }
    }

    return;
}

void player_ping( CHAR_DATA * ch, CHAR_DATA * ignore )
{
    if ( !is_spectator( ch ) && !xIS_SET( ch->in_room->room_flags, ROOM_PROTOTYPE ) )
    {
         if ( ch->in_room->area )
         {
             motion_ping( ch->in_room->x, ch->in_room->y, ch->in_room->z, ch->in_room->area, ignore );
         }
    }

    return;
}

void abort_follow( CHAR_DATA * ch )
{
    if ( ch->master != NULL && !IS_NPC(ch) ) stop_follower( ch );

    return;
}
