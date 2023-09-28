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
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

#define MAX_WIDTH     60
#define MAX_HEIGHT    30

void Map_Exits        args( ( ROOM_INDEX_DATA * ) );
void Format_Map       args( ( char * , CHAR_DATA * ) );
void Check_Exits      args( ( ROOM_INDEX_DATA *, int, int ) );
void Check_Room       args( ( ROOM_INDEX_DATA *, int, int ) );

const int OffX[10] = { +0, +1, +0, -1, +0, +0, +1, -1, +1, -1 };
const int OffY[10] = { -1, +0, +1, +0, +0, +0, -1, -1, +1, +1 };

int map[MAX_WIDTH][MAX_HEIGHT];       // Map Array
static int mapWidth, mapHeight;       // Map Width and Height
static int mapCX, mapCY;              // Center X and Center Y

// Map Configuration Flags
bool ShowVents     = FALSE;
bool ShowHived     = FALSE;
bool ShowFriendly  = FALSE;
bool StopAtDoors   = FALSE;
bool ModeExpanded  = FALSE;

// Decided to make these flags. Bwhaha.
#define MAP_BLANK       0       // This isn't a flag, its the default value.
#define MAP_INDOORS     BV00
#define MAP_OUTDOORS    BV01
#define MAP_HIVED       BV02    // This room is hived
#define MAP_VENT        BV03    // This room contains a vent
#define MAP_CENTER      BV04    // This is the center of the map
#define MAP_PIT         BV05    // This room has no floor
#define MAP_CP          BV06    // This room is a Control Point
#define MAP_WALL        BV07    // Used with Expanded Maps
#define MAP_UPDOWN      BV08    // Used with Overlap Maps
#define MAP_EASTWEST    BV09    // Used with Overlap Maps
#define MAP_VERT1       BV10    // Used with Overlap Maps
#define MAP_VERT2       BV11    // Used with Overlap Maps
#define MAP_ALIEN       BV12    // Aliens are present
#define MAP_MARINE      BV13    // Marines are present
#define MAP_PREDATOR    BV14    // Predators are present
#define MAP_HASUP       BV15    // This room has an up exit
#define MAP_HASDOWN     BV16    // This room has a down exit
#define MAP_CLOSED      BV17    // This exit is a closed door.
#define MAP_OPEN        BV18    // This is an open exit. (Expanded)

/*
 * Not a command right now, triggered by LOOK MAP
 */
void do_lookmap( CHAR_DATA * ch, char * argument )
{
   char buf[MAX_STRING_LENGTH];
   int height;

   ShowVents = IS_IMMORTAL(ch) ? TRUE : FALSE;
   ShowHived = IS_IMMORTAL(ch) ? TRUE : FALSE;
   ShowFriendly = IS_IMMORTAL(ch) ? TRUE : FALSE;
   StopAtDoors = FALSE;
   ModeExpanded = FALSE;

   height = MAX_HEIGHT;

   if ( !str_cmp( argument, "auto" ) )
        height = 5;              
   else if ( atoi(argument) == 666 )
        ModeExpanded = TRUE;
   else if ( atoi(argument) == 999 )
   {     ModeExpanded = TRUE; StopAtDoors = TRUE;   }
   else
        height = atoi(argument);

   Initilize_Map( height );

   Map_Exits( ch->in_room );

   map[mapCX][mapCY] = MAP_CENTER;

   Format_Map( buf, IS_IMMORTAL(ch) ? NULL : ch );

   send_to_char( buf, ch );

   return;
}

int MAP_OFFSET( void )
{
   return ( ModeExpanded ? 3 : 2 );
}

void Initilize_Map( int height )
{
   mapHeight = URANGE( 3, height, MAX_HEIGHT );
   mapWidth = URANGE( 3, mapHeight * 2, MAX_WIDTH );

   mapCX = ( mapWidth / 2 ) + 1;
   mapCY = ( mapHeight / 2 ) + 1;

   Reset_Map( );

   return;
}

void Reset_Map( void )
{
   int x, y;

   for ( y = 0; y <= MAX_HEIGHT; ++y )
    for ( x = 0; x <= MAX_WIDTH; ++x )
     map[x][y] = MAP_BLANK;

   return;
}

void Map_Exits( ROOM_INDEX_DATA * room )
{
   if ( room == NULL ) return;

   if ( ModeExpanded ) { Check_Exits_Ext( room, mapCX, mapCY ); }
   else                { Check_Exits_Sim( room, mapCX, mapCY ); }

   return;
}                                  

void Check_Exits_Ext( ROOM_INDEX_DATA * room, int cX, int cY )
{
   EXIT_DATA * xit = NULL;
   int nX, nY, i;

   for ( i = 0; i < DIR_SOMEWHERE; i++ )
   {
      if ( ( xit = get_exit( room, i ) ) != NULL )
      {       
         if ( i == DIR_UP ) { SET_BIT( map[cX][cY], MAP_HASUP ); continue; }
         if ( i == DIR_DOWN ) { SET_BIT( map[cX][cY], MAP_HASDOWN ); continue; }    

         nX = cX + (OffX[i] * MAP_OFFSET());
         nY = cY + (OffY[i] * MAP_OFFSET());
                                                    
         if ( xIS_SET( xit->exit_info, EX_CLOSED ) )
           map[(cX + OffX[i])][(cY + OffY[i])] = MAP_CLOSED;
         else
           map[(cX + OffX[i])][(cY + OffY[i])] = MAP_OPEN;

         if ( map[nX][nY] == MAP_BLANK && ( !StopAtDoors || !xIS_SET( xit->exit_info, EX_CLOSED ) ) )
             Check_Room( xit->to_room, nX, nY );
      }
   } 

   return;
}

void Check_Exits_Sim( ROOM_INDEX_DATA * room, int cX, int cY )
{
   EXIT_DATA * xit = NULL;
   bool dShut;
   int nX, nY, i;

   for ( i = 0; i < DIR_SOMEWHERE; i++ )
   {
      if ( ( xit = get_exit( room, i ) ) != NULL )
      {  
         if ( i == DIR_UP ) { SET_BIT( map[cX][cY], MAP_HASUP ); continue; }
         if ( i == DIR_DOWN ) { SET_BIT( map[cX][cY], MAP_HASDOWN ); continue; }    

         nX = cX + (OffX[i] * MAP_OFFSET());
         nY = cY + (OffY[i] * MAP_OFFSET());

         dShut = xIS_SET( xit->exit_info, EX_CLOSED );
                                                    
         /*
         if ( i == DIR_NORTH )     map[cX+0][cY-1] = ( dShut ? MAP_CLOSED : MAP_UPDOWN );
         if ( i == DIR_SOUTH )     map[cX+0][cY+1] = ( dShut ? MAP_CLOSED : MAP_UPDOWN );
         if ( i == DIR_WEST )      map[cX-1][cY+0] = ( dShut ? MAP_CLOSED : MAP_EASTWEST );
         if ( i == DIR_EAST )      map[cX+1][cY+0] = ( dShut ? MAP_CLOSED : MAP_EASTWEST );
         if ( i == DIR_NORTHEAST ) map[cX+1][cY-1] = ( dShut ? MAP_CLOSED : MAP_VERT2 ); 
         if ( i == DIR_NORTHWEST ) map[cX-1][cY-1] = ( dShut ? MAP_CLOSED : MAP_VERT1 ); 
         if ( i == DIR_SOUTHEAST ) map[cX+1][cY+1] = ( dShut ? MAP_CLOSED : MAP_VERT1 ); 
         if ( i == DIR_SOUTHWEST ) map[cX-1][cY+1] = ( dShut ? MAP_CLOSED : MAP_VERT2 ); 
         */

         if ( i == DIR_NORTH )     map[cX+0][cY-1] = MAP_UPDOWN;
         if ( i == DIR_SOUTH )     map[cX+0][cY+1] = MAP_UPDOWN;
         if ( i == DIR_WEST )      map[cX-1][cY+0] = MAP_EASTWEST;
         if ( i == DIR_EAST )      map[cX+1][cY+0] = MAP_EASTWEST;
         if ( i == DIR_NORTHEAST ) map[cX+1][cY-1] = MAP_VERT2; 
         if ( i == DIR_NORTHWEST ) map[cX-1][cY-1] = MAP_VERT1; 
         if ( i == DIR_SOUTHEAST ) map[cX+1][cY+1] = MAP_VERT1; 
         if ( i == DIR_SOUTHWEST ) map[cX-1][cY+1] = MAP_VERT2; 
 
         if ( map[nX][nY] == MAP_BLANK && ( !StopAtDoors || !dShut ) )
            Check_Room( xit->to_room, nX, nY );
      }
   }  

   return;
}

void Check_Room( ROOM_INDEX_DATA * room, int cX, int cY )
{
   if ( room == NULL ) return;
   if ( map[cX][cY] != MAP_BLANK ) return;

   map[cX][cY] = get_room_type( room, map[cX][cY] );

   if ( ModeExpanded )
   {
      SET_BIT( map[cX-1][cY-1], MAP_WALL );
      SET_BIT( map[cX-0][cY-1], MAP_WALL );
      SET_BIT( map[cX+1][cY-1], MAP_WALL );
      SET_BIT( map[cX-1][cY+0], MAP_WALL );
      SET_BIT( map[cX+1][cY+0], MAP_WALL );
      SET_BIT( map[cX-1][cY+1], MAP_WALL );
      SET_BIT( map[cX-0][cY+1], MAP_WALL );
      SET_BIT( map[cX+1][cY+1], MAP_WALL );

      Check_Exits_Ext( room, cX, cY );
   }
   else
   {
      Check_Exits_Sim( room, cX, cY );
   }

   return;
}                                  

int get_room_type( ROOM_INDEX_DATA * room, int tmp )
{
   CHAR_DATA *rch;
   int flags;

   flags = tmp;

   if ( room == NULL ) return MAP_BLANK;

   for ( rch = room->first_person; rch; rch = rch->next_in_room )
   {
       if ( is_spectator( rch ) || IN_VENT( rch ) ) continue;

       if ( rch->race == RACE_ALIEN ) SET_BIT( flags, MAP_ALIEN );
       if ( rch->race == RACE_MARINE ) SET_BIT( flags, MAP_MARINE );
       if ( rch->race == RACE_PREDATOR ) SET_BIT( flags, MAP_PREDATOR );
   }

   if ( xIS_SET( room->room_flags, ROOM_HIVED ) ) SET_BIT( flags, MAP_HIVED );
   if ( xIS_SET( room->room_flags, ROOM_INDOORS ) ) SET_BIT( flags, MAP_INDOORS );
   if ( !xIS_SET( room->room_flags, ROOM_INDOORS ) ) SET_BIT( flags, MAP_OUTDOORS );

   if ( xIS_SET( room->room_flags, ROOM_VENTED_A ) ) SET_BIT( flags, MAP_VENT );
   if ( xIS_SET( room->room_flags, ROOM_VENTED_B ) ) SET_BIT( flags, MAP_VENT );
   if ( xIS_SET( room->room_flags, ROOM_VENTED_C ) ) SET_BIT( flags, MAP_VENT );
   if ( xIS_SET( room->room_flags, ROOM_VENTED_D ) ) SET_BIT( flags, MAP_VENT );

   if ( xIS_SET( room->room_flags, ROOM_NOFLOOR ) ) SET_BIT( flags, MAP_PIT );
   if ( xIS_SET( room->room_flags, ROOM_CP ) ) SET_BIT( flags, MAP_CP );

   return flags;
}

void Format_Map( char * buf, CHAR_DATA * ch )
{
   int x, y;
   char * val;
 
   strcpy( buf, "" );

   strcat( buf, "&w&B+&b-" );
   for ( x = 1; x <= mapWidth; ++x ) strcat( buf, "-" );
   strcat( buf, "-&B+\n\r" );

   for ( y = 1; y <= mapHeight; ++y )
   {
     strcat( buf, "&w&b| " );

     for ( x = 1; x <= mapWidth; ++x )
     {
         val = " ";

         // Render Basic Rooms
         if ( IS_SET( map[x][y], MAP_INDOORS ) )
         {
              val = ( ModeExpanded ? "&w&C." : "&W+" );
              if ( IS_SET( map[x][y], MAP_HASUP ) )   val = "&W>";
              if ( IS_SET( map[x][y], MAP_HASDOWN ) ) val = "&W<";
              if ( IS_SET( map[x][y], MAP_HASUP )
                && IS_SET( map[x][y], MAP_HASDOWN ) ) val = "&W+";
         }
         if ( IS_SET( map[x][y], MAP_OUTDOORS ) )
         {
              val = ( ModeExpanded ? "&w&C." : "&w&c+" );
              if ( IS_SET( map[x][y], MAP_HASUP ) )   val = "&g>";
              if ( IS_SET( map[x][y], MAP_HASDOWN ) ) val = "&g<";
              if ( IS_SET( map[x][y], MAP_HASUP )
                && IS_SET( map[x][y], MAP_HASDOWN ) ) val = "&g+";
         }
         if ( IS_SET( map[x][y], MAP_HIVED ) )
         {
              val = ( ModeExpanded ? "&w&G%" : "&w&G+" );
              if ( IS_SET( map[x][y], MAP_HASUP ) )   val = "&G>";
              if ( IS_SET( map[x][y], MAP_HASDOWN ) ) val = "&G<";
              if ( IS_SET( map[x][y], MAP_HASUP )
                && IS_SET( map[x][y], MAP_HASDOWN ) ) val = "&G+";
         }

         // Render Rooms with Content
         if ( IS_SET( map[x][y], MAP_VENT ) )     val = "&w&CV";
         if ( IS_SET( map[x][y], MAP_PIT ) )      val = "&w&RX";
         if ( IS_SET( map[x][y], MAP_CP ) )       val = "&w&Y$";

         if ( ch != NULL )
         {
            if ( IS_SET(map[x][y], MAP_ALIEN) && ch->race == RACE_ALIEN )  val = "&w&B@";
            if ( IS_SET(map[x][y], MAP_MARINE) && ch->race == RACE_MARINE )  val = "&w&R@";
            if ( IS_SET(map[x][y], MAP_PREDATOR) && ch->race == RACE_PREDATOR )  val = "&w&G@";
         }
         else
         {
            if ( IS_SET(map[x][y], MAP_ALIEN) )    val = "&w&B@";
            if ( IS_SET(map[x][y], MAP_MARINE) )   val = "&w&R@";
            if ( IS_SET(map[x][y], MAP_PREDATOR) ) val = "&w&G@";
         }

         // Rendering Exits
         if ( IS_SET( map[x][y], MAP_WALL ) )     val = "&w&z#";
         if ( IS_SET( map[x][y], MAP_UPDOWN ) )   val = "&w&z|";
         if ( IS_SET( map[x][y], MAP_EASTWEST ) ) val = "&w&z-";
         if ( IS_SET( map[x][y], MAP_VERT1 ) )    val = "&w&z\\";
         if ( IS_SET( map[x][y], MAP_VERT2 ) )    val = "&w&z/";
         // if ( IS_SET( map[x][y], MAP_CLOSED ) )   val = "&w&z=";

         if ( ModeExpanded )
         {
            if ( IS_SET( map[x][y], MAP_OPEN ) )     val = "&w&C.";
            if ( IS_SET( map[x][y], MAP_CLOSED ) )   val = "&w&b.";
         }

         // Render Player Override
         if ( IS_SET( map[x][y], MAP_CENTER ) )   val = "&w&R+";

         strcat( buf, val );
     }
     strcat( buf, " &w&b|\n\r" );
   }


   strcat( buf, "&w&B+&b-" );
   for ( x = 1; x <= mapWidth; ++x ) strcat( buf, "-" );
   strcat( buf, "-&B+\n\r" );

   return;
}
