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
*	    Misc module for general commands: not skills or spells	   *
****************************************************************************
* Note: Most of the stuff in here would go in act_obj.c, but act_obj was   *
* getting big.								   *
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"

/* Added for Marriage */
#include <ctype.h>

/*
 * Global Variables
 */
#define BFS_ERROR	   -1
#define BFS_ALREADY_THERE  -2
#define BFS_NO_PATH	   -3

bool hail_route_check   args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *toroom ) );
bool reload_sentry( CHAR_DATA *ch, char * arg1, char * arg2 );
bool rem_sentry( OBJ_DATA * obj, CHAR_DATA * ch );
void add_sentry( OBJ_DATA * obj, CHAR_DATA * ch, int dir );
SENTRY_DATA * get_sentry( OBJ_DATA * obj );

extern int	top_exit;

SENTRY_DATA * first_sentry;
SENTRY_DATA * last_sentry;

/*
 * Arming subroutine. Modifyed for arming of Heavy Explosives.
 *        -Ghost
 */
void do_arm( CHAR_DATA *ch, char *argument )
{
    char name[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int mint=0, maxt=0;
    int cnt=0;
  
    if ( ch->race == RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( is_spectator( ch ) ) { send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch ); return; }

    obj = get_eq_char( ch, WEAR_HOLD );
    
    if ( !obj )
    {
       ch_printf( ch, "You don't seem to be holding a explosive!\n\r" );
       return;
    }
          
    if ( obj->item_type != ITEM_GRENADE )
    {
       if ( obj->item_type == ITEM_C4 )
       {
           ch_printf( ch, "C4 can only be activated with the USE command.\n\r" );
           return;
       }
       else
       {
           ch_printf( ch, "You don't seem to be holding a explosive!\n\r" );
           return;
       }
    }

    if ( xIS_SET( obj->extra_flags, ITEM_MARINE ) && ch->race != RACE_MARINE )
    {
       send_to_char( "&RYou don't reconize this type of explosive.\n\r", ch );
       return;
    }

    if ( xIS_SET( obj->extra_flags, ITEM_PREDATOR ) && ch->race != RACE_PREDATOR )
    {
       send_to_char( "&RYou don't reconize this type of explosive.\n\r", ch );
       return;
    }

    if ( obj->value[5] <= 0 )
    { 
       ch_printf( ch, "This explosive doesn't seem to be used this way. Maybe its remote-triggered.\n\r" );
       return;
    }

    if ( IS_NPC( ch ) )
    {
       if ( ch->pIndexData ) { sprintf( name, "%d", ch->pIndexData->vnum ); }
    }
    else
    {
       strcpy( name, ch->name );
    }
    
    mint = obj->value[5];
    maxt = obj->value[5]*5;

    if ( argument[0] == '\0' )
    {
       if ( obj->value[4] > 0 && obj->armed_by != NULL )
       {
           if ( !str_cmp( obj->armed_by, name ) )
           {
                obj->value[4] = 0;
                STRFREE ( obj->armed_by );
                obj->armed_by = STRALLOC( "" );
                obj->parent = NULL;
                ch_printf( ch, "You disarm %s.\n\r", obj->short_descr );
                act( AT_PLAIN, "$n disarms $p.", ch, obj, NULL, TO_ROOM );
                return;
           }                
       }
       send_to_char( "&RSyntax: ARM <delay in rounds>\n\r", ch );
       ch_printf( ch, "&RValid delay is %d to %d rounds.\n\r", mint, maxt );
       return;
    }

    cnt = atoi( argument );

    if ( cnt < mint || cnt > maxt )
    {
       do_arm( ch, "" );
       return;
    }

    if ( obj->value[4] > 0 && obj->armed_by )
    {
       if ( str_cmp( obj->armed_by, name ) )
       {
            ch_printf( ch, "You didn't arm %s!\n\r", obj->short_descr );
            act( AT_PLAIN, "$n fails to disarm $p.", ch, obj, NULL, TO_ROOM );
            return;
       }                
    }

    /* Armed */
    obj->value[4] = cnt;

    if ( obj->armed_by ) STRFREE ( obj->armed_by ); 
    obj->armed_by = STRALLOC ( name );

    obj->parent = ch;

    ch_printf( ch, "You arm %s.\n\r", obj->short_descr );
    act( AT_PLAIN, "$n arms $p.", ch, obj, NULL, TO_ROOM );
}

void do_reload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA * obj;
    OBJ_DATA * ammo;
    int shotgun = -1;
    
    if ( is_spectator( ch ) ) { send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch ); return; }

    if ( ch->race == RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "&RSyntax: RELOAD (weapon | MAIN) (ammo)\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "main" ) )
    {
        OBJ_DATA * obj2;
        int cnt=0;

        obj = get_eq_char( ch, WEAR_WIELD );
        obj2 = get_eq_char( ch, WEAR_DUAL_WIELD );

        if ( obj == NULL && obj2 == NULL )
        {
            send_to_char("&RYour not wielding any weapons.\n\r", ch );
            return;
        }
        if ( obj ) reload_main( ch, obj, arg2 );
        if ( obj2 ) reload_main( ch, obj2, arg2 );
        return;
    }

    if ( reload_sentry( ch, arg1, arg2 ) ) return;

    if ( ( obj = get_obj_wear( ch, arg1 ) ) == NULL )
    {
        if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char("&RYou don't seem to have that item.\n\r", ch );
            return;
        }
    }

    if ( obj->item_type != ITEM_WEAPON && ( obj->item_type != ITEM_ATTACHMENT || obj->value[0] != 1 ) )
    {
        send_to_char( "&RYou know, that ain't a weapon.\n\r", ch );
        return;
    }

    if ( obj->item_type == ITEM_WEAPON && ( !is_ranged( obj->value[0] ) || obj->value[0] == WEAPON_ERANGED ) )
    {
        send_to_char( "&RAtleast pick a weapon that requires ammo.\n\r", ch );
        return;
    }

    if ( ( ammo = get_obj_carry( ch, arg2 ) ) == NULL )
    {
        send_to_char("&RYou can't seem to find that ammo.\n\r", ch );
        return;
    }

    if ( ammo->item_type != ITEM_AMMO )
    {
        send_to_char( "&RLets assume the only thing your going to be loading is AMMUNITION.\n\r", ch );
        return;
    }

    if ( obj->item_type == ITEM_WEAPON )
    {
       if ( ammo->value[3] != obj->value[1] && ammo->value[4] != obj->value[1] && ammo->value[3] != obj->value[1] && ammo->value[5] != obj->value[1] )
       {
          if ( obj->attach )
          {
             if ( obj->attach->value[0] == 1 ) obj = obj->attach;
             if ( ammo->value[3] != obj->value[2] && ammo->value[4] != obj->value[1] && ammo->value[3] != obj->value[2] && ammo->value[5] != obj->value[2] )
             {
                send_to_char( "&RYou can't load that ammo into that weapon!\n\r", ch );
                return;
             }
          }
          else
          {
             send_to_char( "&RYou can't load that ammo into that weapon!\n\r", ch );
             return;
          }
       }
    }
    else
    {
       if ( ammo->value[3] != obj->value[2] && ammo->value[4] != obj->value[1] && ammo->value[3] != obj->value[2] && ammo->value[5] != obj->value[2] )
       {
          send_to_char( "&RYou can't load that ammo into that attachment!\n\r", ch );
          return;
       }
    }

    /* Remove the existing clip */
    if ( obj->value[0] != WEAPON_SHOTGUN )
    {
        if ( obj->ammo )
        {
             ch_printf( ch, "You remove a %s from a %s.\n\r", obj->ammo->short_descr, obj->short_descr );
             act( AT_PLAIN, "$n removes $p from a weapon.", ch, obj->ammo, NULL, TO_ROOM );        

             obj_to_char( obj->ammo, ch );
             obj->ammo = NULL;
        }
    }
    else
    {
        if ( obj->ammo )
        {
             if ( obj->ammo->pIndexData != ammo->pIndexData )
             {
                 ch_printf( ch, "You remove %s from a %s.\n\r", obj->ammo->short_descr, obj->short_descr );
                 act( AT_PLAIN, "$n removes $p from a shotgun.", ch, obj->ammo, NULL, TO_ROOM );        

                 obj_to_char( obj->ammo, ch );
                 obj->ammo = NULL;
             }
             else
             {
                 shotgun = UMAX( 0, ammo->value[2] );
             }
        }
    }

    if ( shotgun > 0 )
    {
        int rounds;

        rounds = get_max_rounds( obj->ammo ) - obj->ammo->value[2];

        if ( rounds <= 0 )
        {
             ch_printf( ch, "&RSorry. That weapon can't hold any more rounds. UNLOAD it first.\n\r" );
             return;
        }

        shotgun = URANGE( 0, shotgun, rounds );

        obj->ammo->value[2] += shotgun;
        ammo->value[2] -= shotgun;

        ch_printf( ch, "You load more rounds into a %s. &z(&W+%d rounds&z)\n\r", obj->short_descr, shotgun );
        act( AT_PLAIN, "$n reloads a $p.", ch, obj, NULL, TO_ROOM );

        ch->ap = UMAX( 0, ch->ap - shotgun );

        if ( ammo->value[2] <= 0 )
        {
            separate_obj(ammo);
            obj_from_char(ammo);
            extract_obj(ammo);
        }
    }
    else
    {
        separate_obj(ammo);
        obj_from_char(ammo);

        ch_printf( ch, "You load a %s into a %s.\n\r", ammo->short_descr, obj->short_descr );
        act( AT_PLAIN, "$n reloads a $p.", ch, obj, NULL, TO_ROOM );

        obj->ammo = ammo;

        if ( obj->attach )
        {
          if ( obj->attach->value[0] == 9 )
             ch->ap -= obj->attach->value[2];
          else
             ch->ap = 0;
        }
        else
          ch->ap = 0;
    }

    return;
}

void reload_main( CHAR_DATA * ch, OBJ_DATA * obj, char * argument )
{
    OBJ_DATA * ammo;
    
    if ( obj->item_type != ITEM_WEAPON )
    {
        send_to_char( "&RYou know, that ain't a weapon.\n\r", ch );
        return;
    }

    if ( !is_ranged( obj->value[0] ) )
    {
        send_to_char( "&RAtleast pick a weapon that requires ammo.\n\r", ch );
        return;
    }

    if ( ( ammo = get_obj_carry( ch, argument ) ) == NULL )
    {
        send_to_char("&RYou can't seem to find that ammo.\n\r", ch );
        return;
    }

    if ( ammo->item_type != ITEM_AMMO )
    {
        send_to_char( "&RLets assume the only thing your going to be loading is AMMUNITION.\n\r", ch );
        return;
    }

    if ( ammo->value[3] != obj->value[1] && ammo->value[4] != obj->value[1] && ammo->value[3] != obj->value[1] && ammo->value[5] != obj->value[1] )
    {
        send_to_char( "&RYou can't load that ammo into that weapon!\n\r", ch );
        return;
    }

    if ( obj->ammo )
    {
        send_to_char( "&RThat weapon is already loaded. Try UNLOAD or EJECT first.\n\r", ch );
        return;
    }

    separate_obj(ammo);
    obj_from_char(ammo);

    ch_printf( ch, "You load a %s into a %s.\n\r", ammo->short_descr, obj->short_descr );
    act( AT_PLAIN, "$n reloads a $p.", ch, obj, NULL, TO_ROOM );

    obj->ammo = ammo;
    ch->ap = 0;

    return;
}

void do_unload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA * obj;
    OBJ_DATA * ammo;
    
    if ( is_spectator( ch ) ) { send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch ); return; }

    if ( ch->race == RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "&RSyntax: UNLOAD (weapon | MAIN)\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "main" ) )
    {
        OBJ_DATA * obj2;
        int cnt=0;

        obj = get_eq_char( ch, WEAR_WIELD );
        obj2 = get_eq_char( ch, WEAR_DUAL_WIELD );

        if ( obj == NULL && obj2 == NULL )
        {
            send_to_char("&RYour not wielding any weapons.\n\r", ch );
            return;
        }
        if ( obj )
        {
            if ( obj->item_type == ITEM_WEAPON )
            {
               if ( !obj->ammo )
               {
                  send_to_char( "&RThat weapon doesn't have any loaded ammo.\n\r", ch );
               }
               else
               {
                  ch_printf( ch, "You remove a %s from a %s.\n\r", obj->ammo->short_descr, obj->short_descr );
                  act( AT_PLAIN, "$n removes $p from a weapon.", ch, obj->ammo, NULL, TO_ROOM );

                  obj_to_char( obj->ammo, ch );
                  obj->ammo = NULL;
                  cnt++;
               }
            }
            else
            {
               send_to_char( "&RThat isn't even a loadable weapon.\n\r", ch );
            }
        }
        if ( obj2 )
        {
            if ( obj2->item_type == ITEM_WEAPON )
            {
               if ( !obj2->ammo )
               {
                  send_to_char( "&RThat weapon doesn't have any loaded ammo.\n\r", ch );
               }
               else
               {
                  ch_printf( ch, "You remove a %s from a %s.\n\r", obj2->ammo->short_descr, obj2->short_descr );
                  act( AT_PLAIN, "$n removes $p from a weapon.", ch, obj2->ammo, NULL, TO_ROOM );

                  obj_to_char( obj2->ammo, ch );
                  obj2->ammo = NULL;
                  cnt++;
               }
            }
            else
            {
               send_to_char( "&RThat isn't even a loadable weapon.\n\r", ch );
            }
        }
        if ( cnt > 0 ) ch->ap = 0;
        return;
    }

    if ( ( obj = get_obj_wear( ch, arg1 ) ) == NULL )
    {
        if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char("&RYou don't seem to have that item.\n\r", ch );
            return;
        }
    }

    if ( obj->item_type != ITEM_WEAPON )
    {
        send_to_char( "&RYou know, that ain't a weapon.\n\r", ch );
        return;
    }

    if ( !obj->ammo )
    {
        send_to_char( "&RThat weapon doesn't have any loaded ammo.\n\r", ch );
        return;
    }

    ch_printf( ch, "You remove a %s from a %s.\n\r", obj->ammo->short_descr, obj->short_descr );
    act( AT_PLAIN, "$n removes $p from a weapon.", ch, obj->ammo, NULL, TO_ROOM );        

    obj_to_char( obj->ammo, ch );
    obj->ammo = NULL;
    ch->ap = 0;

    return;
}

void do_eject( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA * obj;
    

    if ( is_spectator( ch ) ) { send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch ); return; }

    if ( ch->race == RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "&RSyntax: EJECT (weapon | MAIN)\n\r", ch );
        send_to_char( " Ejects the weapons clip onto the ground.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "main" ) )
    {
        OBJ_DATA * obj2;
        int cnt=0;

        obj = get_eq_char( ch, WEAR_WIELD );
        obj2 = get_eq_char( ch, WEAR_DUAL_WIELD );

        if ( obj == NULL && obj2 == NULL )
        {
            send_to_char("&RYour not wielding any weapons.\n\r", ch );
            return;
        }
        if ( obj )
        {
            if ( obj->item_type == ITEM_WEAPON )
            {
               if ( !obj->ammo )
               {
                  send_to_char( "&RThat weapon doesn't have any loaded ammo.\n\r", ch );
               }
               else
               {
                  if ( obj->value[0] == WEAPON_SHOTGUN )
                  {
                      ch_printf( ch, "&CYou eject the rounds from %s.\n\r", obj->short_descr );
                      sprintf( buf, "&C$n's %s ejects its rounds.", obj->short_descr );
                  }
                  else
                  {
                      ch_printf( ch, "&CYou eject the clip from %s.\n\r", obj->short_descr );
                      sprintf( buf, "&C$n's %s ejects a clip.", obj->short_descr );
                  }
                  act( AT_CYAN, buf, ch, obj, NULL, TO_ROOM );

                  obj_to_room( obj->ammo, ch->in_room );
                  obj->ammo = NULL;
                  cnt++;
               }
            }
            else
            {
               send_to_char( "&RThat isn't even a loadable weapon.\n\r", ch );
            }
        }
        if ( obj2 )
        {
            if ( obj2->item_type == ITEM_WEAPON )
            {
               if ( !obj2->ammo )
               {
                  send_to_char( "&RThat weapon doesn't have any loaded ammo.\n\r", ch );
               }
               else
               {
                  if ( obj2->value[0] == WEAPON_SHOTGUN )
                  {
                      ch_printf( ch, "&CYou eject the rounds from %s.\n\r", obj2->short_descr );
                      sprintf( buf, "&C$n's %s ejects its rounds.", obj2->short_descr );
                  }
                  else
                  {
                      ch_printf( ch, "&CYou eject the clip from %s.\n\r", obj2->short_descr );
                      sprintf( buf, "&C$n's %s ejects a clip.", obj2->short_descr );
                  }
                  act( AT_CYAN, buf, ch, obj2, NULL, TO_ROOM );

                  obj_to_room( obj2->ammo, ch->in_room );
                  obj2->ammo = NULL;
                  cnt++;
               }
            }
            else
            {
               send_to_char( "&RThat isn't even a loadable weapon.\n\r", ch );
            }
        }
        if ( cnt > 0 ) ch->ap = 0;
        return;
    }

    if ( ( obj = get_obj_wear( ch, arg1 ) ) == NULL )
    {
        if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char("&RYou don't seem to have that item.\n\r", ch );
            return;
        }
    }

    if ( obj->item_type != ITEM_WEAPON )
    {
        send_to_char( "&RYou know, that ain't a weapon.\n\r", ch );
        return;
    }

    if ( !obj->ammo )
    {
        send_to_char( "&RThat weapon doesn't have any loaded ammo.\n\r", ch );
        return;
    }

    if ( obj->value[0] == WEAPON_SHOTGUN )
    {
       ch_printf( ch, "&CYou eject the rounds from %s.\n\r", obj->short_descr );
       sprintf( buf, "&C$n's %s ejects its rounds.", obj->short_descr );
    }
    else
    {
       ch_printf( ch, "&CYou eject the clip from %s.\n\r", obj->short_descr );
       sprintf( buf, "&C$n's %s ejects a clip.", obj->short_descr );
    }
    act( AT_CYAN, buf, ch, obj, NULL, TO_ROOM );

    obj_to_room( obj->ammo, ch->in_room );
    obj->ammo = NULL;
    ch->ap = 0;

    return;
}

void do_setmode( CHAR_DATA *ch, char *argument )
{
   OBJ_DATA *wield;
   OBJ_DATA *wield2;
   bool useMode[5];
   int x;
 
   if ( ch->race == RACE_ALIEN )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   wield = get_eq_char( ch, WEAR_WIELD );
   if( wield && !( wield->item_type == ITEM_WEAPON ) )
      wield = NULL;
   wield2 = get_eq_char( ch, WEAR_DUAL_WIELD );
   if( wield2 && !( wield2->item_type == ITEM_WEAPON ) )
      wield2 = NULL;
     
   if ( wield )
   {
     if ( !is_ranged( wield->value[0] ) )
        wield = NULL;     
   }
   if ( wield2 )
   {
     if ( !is_ranged( wield2->value[0] ) )
        wield2 = NULL;
   }

   if ( !wield && !wield2 )
   {
      send_to_char( "&RYou don't seem to be wielding a setable weapon.\n\r&w", ch);
      return;
   }
         
   for ( x = 0; x < 5; x++ ) useMode[x] = FALSE;

   if ( wield )
   {
     if ( xIS_SET( wield->extra_flags, ITEM_SINGLEFIRE ) )  useMode[0] = TRUE;
     if ( xIS_SET( wield->extra_flags, ITEM_SEMIAUTO ) )    useMode[1] = TRUE;
     if ( xIS_SET( wield->extra_flags, ITEM_BURSTFIRE ) )   useMode[2] = TRUE;
     if ( xIS_SET( wield->extra_flags, ITEM_AUTOFIRE ) )    useMode[3] = TRUE;
   }
   if ( wield2 )
   {
     if ( xIS_SET( wield2->extra_flags, ITEM_SINGLEFIRE ) ) useMode[0] = TRUE;
     if ( xIS_SET( wield2->extra_flags, ITEM_SEMIAUTO ) )   useMode[1] = TRUE;
     if ( xIS_SET( wield2->extra_flags, ITEM_BURSTFIRE ) )  useMode[2] = TRUE;
     if ( xIS_SET( wield2->extra_flags, ITEM_AUTOFIRE ) )   useMode[3] = TRUE;
   }

   if ( argument[0] == '\0' )
   {
      ch_printf( ch, "&zSyntax: &WSETMODE &z<%ssingle &z| %ssemi &z| %sburst &z| %sfull &z>\n\r&w",
       useMode[0] ? "&C" : "&B", useMode[1] ? "&C" : "&B", useMode[2] ? "&C" : "&B", useMode[3] ? "&C" : "&B" );

      return;
   }
   
   if ( !str_cmp( argument, "single" ) )
   {     
      if ( wield )
      {
         if ( xIS_SET( wield->extra_flags, ITEM_SINGLEFIRE ) )
         {
             wield->weapon_mode = MODE_SINGLE;
             ch_printf( ch, "You switch a %s to single-round shot.\n\r", wield->short_descr );
             act( AT_PLAIN, "$n switches a $p to single-round shot.", ch, wield, NULL, TO_ROOM );
         }
         else
         {
             ch_printf( ch, "%s doesn't support single-round fire mode.\n\r", wield->short_descr );
         }
      }
      if ( wield2 )
      {
         if ( xIS_SET( wield2->extra_flags, ITEM_SINGLEFIRE ) )
         {
             wield2->weapon_mode = MODE_SINGLE;
             ch_printf( ch, "You switch a %s to single-round shot.\n\r", wield2->short_descr );
             act( AT_PLAIN, "$n switches a $p to single-round shot.", ch, wield2, NULL, TO_ROOM );
         }
         else
         {
             ch_printf( ch, "%s doesn't support single-round fire mode.\n\r", wield2->short_descr );
         }
      }
   }
   else if ( !str_cmp( argument, "semi" ) )
   {     
      if ( wield )
      {
         if ( xIS_SET( wield->extra_flags, ITEM_SEMIAUTO ) )
         {
             wield->weapon_mode = MODE_SEMIAUTO;
             ch_printf( ch, "You switch a %s to semi-automatic fire.\n\r", wield->short_descr );
             act( AT_PLAIN, "$n switches a $p to semi-automatic fire.", ch, wield, NULL, TO_ROOM );
         }
         else
         {
             ch_printf( ch, "%s doesn't support semi-automatic fire.\n\r", wield->short_descr );
         }
      }
      if ( wield2 )
      {
         if ( xIS_SET( wield2->extra_flags, ITEM_SEMIAUTO ) )
         {
             wield2->weapon_mode = MODE_SEMIAUTO;
             ch_printf( ch, "You switch a %s to semi-automatic fire.\n\r", wield2->short_descr );
             act( AT_PLAIN, "$n switches a $p to semi-automatic fire.", ch, wield2, NULL, TO_ROOM );
         }
         else
         {
             ch_printf( ch, "%s doesn't support semi-automatic fire.\n\r", wield2->short_descr );
         }
      }
   }
   else if ( !str_cmp( argument, "burst" ) )
   {     
      if ( wield )
      {
         if ( xIS_SET( wield->extra_flags, ITEM_BURSTFIRE ) )
         {
             wield->weapon_mode = MODE_BURST;
             ch_printf( ch, "You switch a %s to burst-fire.\n\r", wield->short_descr );
             act( AT_PLAIN, "$n switches a $p to burst-fire.", ch, wield, NULL, TO_ROOM );
         }
         else
         {
             ch_printf( ch, "%s doesn't support burst-fire mode.\n\r", wield->short_descr );
         }
      }
      if ( wield2 )
      {
         if ( xIS_SET( wield2->extra_flags, ITEM_BURSTFIRE ) )
         {
             wield2->weapon_mode = MODE_BURST;
             ch_printf( ch, "You switch a %s to burst-fire.\n\r", wield2->short_descr );
             act( AT_PLAIN, "$n switches a $p to burst-fire.", ch, wield2, NULL, TO_ROOM );
         }
         else
         {
             ch_printf( ch, "%s doesn't support burst-fire mode.\n\r", wield2->short_descr );
         }
      }
   }
   else if ( !str_cmp( argument, "full" ) )
   {     
      if ( wield )
      {
         if ( xIS_SET( wield->extra_flags, ITEM_AUTOFIRE ) )
         {
             wield->weapon_mode = MODE_AUTOMATIC;
             ch_printf( ch, "You switch a %s to fully-automatic fire.\n\r", wield->short_descr );
             act( AT_PLAIN, "$n switches a $p to fully-automatic fire.", ch, wield, NULL, TO_ROOM );
         }
         else
         {
             ch_printf( ch, "%s doesn't support fully-automatic fire.\n\r", wield->short_descr );
         }
      }
      if ( wield2 )
      {
         if ( xIS_SET( wield2->extra_flags, ITEM_AUTOFIRE ) )
         {
             wield2->weapon_mode = MODE_AUTOMATIC;
             ch_printf( ch, "You switch a %s to fully-automatic fire.\n\r", wield2->short_descr );
             act( AT_PLAIN, "$n switches a $p to fully-automatic fire.", ch, wield2, NULL, TO_ROOM );
         }
         else
         {
             ch_printf( ch, "%s doesn't support fully-automatic fire.\n\r", wield2->short_descr );
         }
      }
   }
   else
   {
     do_setmode( ch , "" );
   }
   return;
}

void do_switch( CHAR_DATA *ch, char *argument )
{
   bool useMode[4];
   int x;
 
   if ( ch->race != RACE_PREDATOR )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }

   if ( !IS_NPC( ch ) && ch->pcdata )
   {
      if ( ch->pcdata->learned[gsn_vision_modes] <= 0 )
      {
         send_to_char( "&w&RSorry. SWITCH requires the VISION MODES Skill.\n\r", ch );
         return;
      }
   }

   for ( x = 0; x < 4; x++ ) useMode[x] = FALSE;

   if ( ch->vision == RACE_MARINE ) useMode[0] = TRUE;
   else if ( ch->vision == RACE_ALIEN ) useMode[1] = TRUE;
   else if ( ch->vision == RACE_PREDATOR ) useMode[2] = TRUE;
   else useMode[3] = TRUE;

   if ( argument[0] == '\0' )
   {
      ch_printf( ch, "&zSyntax: &WSWITCH &z< %snormal &z| %sinfrared &z| %selectrical &z| %spredtech &z>\n\r&w",
       useMode[3] ? "&C" : "&B", useMode[0] ? "&C" : "&B", useMode[1] ? "&C" : "&B", useMode[2] ? "&C" : "&B" );

      return;
   }
   
   if ( !str_cmp( argument, "normal" ) )
   {
      ch->vision = -1;
      ch_printf( ch, "You deactivate your vision modes.\n\r" );
   }
   else if ( !str_cmp( argument, "infrared" ) )
   {
      ch->vision = RACE_MARINE;
      ch_printf( ch, "You switch to your infrared vision mode.\n\r" );
   }
   else if ( !str_cmp( argument, "electrical" ) )
   {
      ch->vision = RACE_ALIEN;
      ch_printf( ch, "You switch to your electrical vision mode.\n\r" );
   }
   else if ( !str_cmp( argument, "predtech" ) )
   {
      ch->vision = RACE_PREDATOR;
      ch_printf( ch, "You switch to your predtech vision mode.\n\r" );
   }
   else
   {
     do_switch( ch , "" );
   }

   return;
}

void do_use( CHAR_DATA * ch, char * argument )
{
    char arg[MAX_INPUT_LENGTH];
    int ticks = 1;
    
    strcpy( arg, argument );

    if ( is_spectator( ch ) ) { send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch ); return; }

    switch( ch->substate )
    { 
    	default:         
           ticks = use_obj( ch, arg, 1 );

           if ( ticks == 0 ) return;

           if ( IS_IMMORTAL( ch ) ) ticks = 1;

           // send_to_char( "&GYou begin working on the construction.\n\r", ch);
           // act( AT_PLAIN, "$n begins constructing something.", ch, NULL, argument , TO_ROOM );
           add_timer ( ch, TIMER_DO_FUN, ticks, do_use, 1 );
           ch->dest_buf = str_dup(arg);

           return;
    	
    	case 1:
           if ( !ch->dest_buf ) return;
           strcpy(arg, ch->dest_buf);
           DISPOSE( ch->dest_buf);
           break;
    		
    	case SUB_TIMER_DO_ABORT:
           if ( !ch->dest_buf ) return;
           strcpy(arg, ch->dest_buf);
           DISPOSE( ch->dest_buf);
           ch->substate = SUB_NONE;                                                   

           ticks = use_obj( ch, arg, 3 );  // Interruption

           // send_to_char("&RYou are interupted before you can finish using the item.\n\r", ch);
           return;
    }
    
    ch->substate = SUB_NONE;

    ticks = use_obj( ch, arg, 2 );
    
    return;
}

/*
 * MODE = 1: First run, nontimed items run here.
 * MODE = 2: Timer done, timed items run here.
 * MODE = 3: Timer interrupted.
 */
int use_obj( CHAR_DATA *ch, char *argument, int mode )
{
    char buf[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char argd[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL;
    OBJ_DATA *device = NULL;

    if ( is_spectator( ch ) ) { send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch ); return 0; }

    strcpy( buf, argument );
    argument = one_argument( argument, argd );
    argument = one_argument( argument, arg );
    
    if ( !str_cmp( arg , "on" ) )
       argument = one_argument( argument, arg );
    
    if ( argd[0] == '\0' )
    {
	send_to_char( "Use what?\n\r", ch );
        return 0;
    }

    if ( ( device = get_obj_wear( ch, argd ) ) == NULL )
    {
        if ( ( device = get_obj_carry( ch, argd ) ) == NULL )
        {
            if ( ( device = get_obj_list_rev( ch, argd, ch->in_room->last_content ) ) == NULL )
            {
                send_to_char("&RYou cant find that item.\n\r", ch );
                return 0;
            }
        }
    }

    separate_obj( device );

    /*
     * Species checks
     */
    if ( ch->race != RACE_MARINE && IS_OBJ_STAT(device, ITEM_MARINE) )
    {
        send_to_char( "&RThis item can only be used by Marines.\n\r", ch );
        return 0;
    }
    if ( ch->race != RACE_ALIEN && IS_OBJ_STAT(device, ITEM_ALIEN) )
    {
        send_to_char( "&RThis item can only be used by Aliens.\n\r", ch );
        return 0;
    }
    if ( ch->race != RACE_PREDATOR && IS_OBJ_STAT(device, ITEM_PREDATOR) )
    {
        send_to_char( "&RThis item can only be used by Predators.\n\r", ch );
        return 0;
    }

    if ( IS_OBJ_STAT( device, ITEM_USEON ) )
    {
       oprog_useoff_trigger( ch, device );
       xREMOVE_BIT( device->extra_flags, ITEM_USEON );
       xSET_BIT( device->extra_flags, ITEM_USEOFF );
       return 0;
    }
    else if ( IS_OBJ_STAT( device, ITEM_USEOFF ) )
    {
       oprog_useon_trigger( ch, device );
       xREMOVE_BIT( device->extra_flags, ITEM_USEOFF );
       xSET_BIT( device->extra_flags, ITEM_USEON );
       return 0;
    }

    /*
     * Ability to turn on/off lights
     */
    if ( device->item_type == ITEM_LIGHT )
    {      
       if ( IS_SET(device->value[3], BV00 ) )
       {
          REMOVE_BIT( device->value[3], BV00 );
          act( AT_ACTION, "$n turns $p off.",  ch, device, NULL, TO_ROOM );
          act( AT_ACTION, "You turn $p off.", ch, device, NULL, TO_CHAR );
          ch->in_room->light -= 1;
       }
       else
       {
          if ( device->value[2] <= 0 )
          {
              send_to_char("Its power is depleted! Wait for it to recharge!\n\r", ch );
              return 0;
          }
          SET_BIT( device->value[3], BV00 );
          act( AT_ACTION, "$n turns $p on, dousing the room in light.", ch, device, NULL, TO_ROOM );
          act( AT_ACTION, "You turn $p on, dousing the room in light.", ch, device, NULL, TO_CHAR );
          ch->in_room->light += 1;
       }
       return 0;
    }
    else if ( device->item_type == ITEM_NVGOGGLE )
    {      
       if ( IS_SET(device->value[3], BV00 ) )
       {
          REMOVE_BIT( device->value[3], BV00 );
          act( AT_ACTION, "$n turns $p off.",  ch, device, NULL, TO_ROOM );
          act( AT_ACTION, "You turn $p off.", ch, device, NULL, TO_CHAR );
       }
       else
       {
          if ( device->value[0] <= 0 )
          {
              send_to_char("Its power is depleted! Wait for it to recharge!\n\r", ch );
              return 0;
          }
          SET_BIT( device->value[3], BV00 );
          act( AT_ACTION, "$n turns $p on, which emits a light glow.", ch, device, NULL, TO_ROOM );
          act( AT_ACTION, "You turn $p on, illuminating the room.", ch, device, NULL, TO_CHAR );
       }
       return 0;
    }
    else if ( device->item_type == ITEM_GPS )
    {
       send_to_char( "\n\r", ch );
       act( AT_ACTION, "$n fiddles with a $p.", ch, device, NULL, TO_ROOM );
       act( AT_ACTION, "You fiddle with a $p.", ch, device, NULL, TO_CHAR );

       match_log( "ITEM;%s used 'GPS:%s'", ch->name, device->short_descr );

       ch_printf( ch, "&z[&cMonitor Readout&z] - GPS Location: &W(&C%d, %d&W) &zAltitude: &Y%d&z.\n\r",
        ch->in_room->x, ch->in_room->y, ch->in_room->z );

       return 0;
    }
    else if ( device->item_type == ITEM_FLARE )
    {
       if ( device->value[2] != 0 )
       {
          ch_printf( ch, "&w&RThat flare is already lit! Go drop it somewhere.\n\r" );
          return 0;
       }

       match_log( "ITEM;%s used 'FLARE:%s'", ch->name, device->short_descr );

       device->timer = device->value[0];
       device->value[2] = 1;
       device->cost = 0;

       ch->in_room->light++;

       act( AT_YELLOW, "$n lights $p, which begins to burn brightly.",  ch, device, NULL, TO_ROOM );
       act( AT_YELLOW, "You light $p, which begins to burn brightly.", ch, device, NULL, TO_CHAR );

       return 0;
    }
    else if ( device->item_type == ITEM_MEDSTATION )
    {
       int hp=0;

       if ( mode == 1 )
       {
          if ( device->value[1] <= 0 )
          {
             ch_printf( ch, "&w&RThis Medical Station is empty right now.\n\r" );
             return 0;
          }

          if ( ch->hit >= ch->max_hit )
          {
             ch_printf( ch, "&w&RYou aren't even injured, stupid.\n\r" );
             return 0;
          }

          send_to_char( "&G(Wait) You step up to the Medical Kit and begin treatment.\n\r", ch);
          act( AT_PLAIN, "$n steps up to the medical kit to use it.", ch, NULL, argument , TO_ROOM );

          return 2;
       }
       else if ( mode == 3 )
       {
           send_to_char("&RYou are interupted before you can finish healing.\n\r", ch);
           return 0;        
       }
       else
       {
          if ( ch->hit >= ch->max_hit )
          {
             ch_printf( ch, "&w&RYou aren't even injured, stupid.\n\r" );
             return 0;
          }

          match_log( "ITEM;%s used 'MEDSTATION:%s'", ch->name, device->short_descr );

          hp = URANGE( 0, device->value[0], ch->max_hit - ch->hit );

          device->value[1]--;

          ch->hit += hp;

          act( AT_YELLOW, "$n uses $p, regaining some health.",  ch, device, NULL, TO_ROOM );
          act( AT_YELLOW, "You use $p and treat your injuries.", ch, device, NULL, TO_CHAR );

          ch_printf( ch, "&w&C(Medical Station) You regain %d hitpoints.\n\r", hp );

          // WAIT_STATE( ch, 8 );

          return 0;
       }
    }
    else if ( device->item_type == ITEM_TOOLCHEST )
    {
       OBJ_DATA * work = NULL;
       int repair = 0;

       if ( device->value[1] <= 0 )
       {
          ch_printf( ch, "&w&RThis Toolchest is empty right now.\n\r" );
          return 0;
       }

       match_log( "ITEM;%s used 'TOOLCHEST:%s'", ch->name, device->short_descr );

       for ( work = ch->first_carrying; work; work = work->next_content )
       {           
          if ( work->item_type != ITEM_ARMOR ) continue;
          if ( work->wear_loc < 0 || work->wear_loc > MAX_WEAR ) continue;

          if ( work->value[0] < work->value[1] )
          {
             repair++;
             work->value[0] = URANGE( 0, work->value[0] + device->value[0], work->value[1] );
          }
       }
        
       if ( repair > 0 )
       {
          device->value[1]--;

          act( AT_YELLOW, "$n uses $p to repair some armor.",  ch, device, NULL, TO_ROOM );
          act( AT_YELLOW, "You use $p and repair some of your armor.", ch, device, NULL, TO_CHAR );

          ch_printf( ch, "&w&C(Toolchest) You repaired %d piece%s of armor.\n\r", repair, (repair == 1) ? "" : "s" );

          WAIT_STATE( ch, 8 );
       }
       else
       {
          ch_printf( ch, "&w&RAll of your equipped armor is in perfect shape.\n\r" );
       }

       return 0;
    }
    else if ( device->item_type == ITEM_AMMOBOX )
    {
       OBJ_INDEX_DATA * pAmmo = NULL;
       OBJ_DATA * ammo = NULL;
       OBJ_DATA * work = NULL;
       int clips = 0, gain = 0;
       int i = 0;

       if ( device->value[0] != 1 )
       {
          ch_printf( ch, "&w&RThis Ammobox is empty right now.\n\r" );
          return 0;
       }

       clips = device->value[1];

       match_log( "ITEM;%s used 'AMMOBOX:%s'", ch->name, device->short_descr );

       act( AT_YELLOW, "$n opens $p and collects some ammo.",  ch, device, NULL, TO_ROOM );
       act( AT_YELLOW, "You open $p and collect some ammo.", ch, device, NULL, TO_CHAR );

       for ( work = ch->first_carrying; work; work = work->next_content )
       {           
          if ( work->item_type != ITEM_WEAPON ) continue;
          if ( work->wear_loc < 0 || work->wear_loc > MAX_WEAR ) continue;

          if ( work->ammo )
          {
             if ( ( pAmmo = get_obj_index( work->ammo->pIndexData->vnum ) ) == NULL ) continue;

             for ( i = 0; i < clips; i++ )
             {
                 ammo = create_object( pAmmo, 1 );
                 ammo = obj_to_char( ammo, ch );
                 gain++;
             }

             ch_printf( ch, "&w&C(Ammobox) You gain %d more clips for %s!", clips, work->short_descr );
          }

          if ( work->attach )
          {
             if ( work->attach->ammo == NULL ) continue;

             if ( ( pAmmo = get_obj_index( work->attach->ammo->pIndexData->vnum ) ) == NULL ) continue;

             for ( i = 0; i < clips; i++ )
             {
                 ammo = create_object( pAmmo, 1 );
                 ammo = obj_to_char( ammo, ch );
                 gain++;
             }

             ch_printf( ch, "&w&C(Ammobox) You gain %d more clips for %s!", clips, work->attach->short_descr );
          }
       }

       if ( gain > 0 )
       {
          device->value[0] = 0;
          device->value[2] = 0;
       }
       else
       {
          ch_printf( ch, "&w&R(Ammobox) You have no ammo-using weapons wielded.\n\r" );
       }
        
       return 0;
    }
    else if ( device->item_type == ITEM_SIFT )
    {
       int per;

       if ( ch->race != RACE_PREDATOR )
       {
          ch_printf( ch, "&w&ROnly Predators can use energy sifts.\n\r" );
          return 0;
       }

       if ( device->wear_loc != WEAR_HOLD )
       {
          ch_printf( ch, "&w&RYou must be holding the energy sift to use it.\n\r" );
          return 0;
       }

       if ( device->value[0] < device->value[1] )
       {
          ch_printf( ch, "&w&RThis device has to finish recharging first.\n\r" );
          return 0;
       }

       match_log( "ITEM;%s used 'SIFT:%s'", ch->name, device->short_descr );

       device->value[0] = 0;
       per = (int)( (float)((float)(ch->max_field) / (float)(100)) * (float)(device->value[2]) );

       act( AT_ACTION, "$n activates $p, which lets off a pulse of light!",  ch, device, NULL, TO_ROOM );
       act( AT_ACTION, "You activate $p, boosting your field charge.", ch, device, NULL, TO_CHAR );

       ch_printf( ch, "&Y(Use Action) It will take 3 rounds to complete.\n\r" );
       WAIT_STATE( ch, 4 * 3 );

       short_target( ch, ch, 6 );

       ch->field = URANGE( 0, ch->field + per, ch->max_field );

       player_ping( ch, ch );

       return 0;
    }
    else if ( device->item_type == ITEM_MEDICOMP )
    {
       int per;
       int hp;

       if ( ch->race != RACE_PREDATOR )
       {
          ch_printf( ch, "&w&ROnly Predators can use medicomps.\n\r" );
          return 0;
       }

       if ( !IS_NPC( ch ) && ch->pcdata )
       {
          if ( ch->pcdata->learned[gsn_medical] < 3 )
          {
              ch_printf( ch, "&w&RYou must have Expert Medical in order to use Medicomps.\n\r" );
              return 0;
          }
       }

       if ( device->wear_loc == WEAR_NONE )
       {
          ch_printf( ch, "&w&RYou must be wearing the medicomp to use it.\n\r" );
          return 0;
       }

       if ( device->value[0] < device->value[1] )
       {
          ch_printf( ch, "&w&RThis device has to finish recharging first.\n\r" );
          return 0;
       }

       match_log( "ITEM;%s used 'MEDICOMP:%s'", ch->name, device->short_descr );

       device->value[0] = 0;

       hp = UMIN( ch->max_hit - ch->hit, device->value[3] );
       hp = URANGE( 0, hp, (ch->field * device->value[2]) );

       ch->hit += hp;

       act( AT_ACTION, "$n actives $p, and quickly regains life!",  ch, device, NULL, TO_ROOM );
       act( AT_ACTION, "You active $p, restoring much of your health.", ch, device, NULL, TO_CHAR );

       short_target( ch, ch, 10 );

       ch_printf( ch, "&Y(Use Action) It will take 5 rounds to complete.\n\r" );
       WAIT_STATE( ch, (4*5) );

       ch->field = URANGE( 0, ch->field - (hp / device->value[2]), ch->max_field );

       player_ping( ch, ch );

       return 0;
    }
    else if ( device->item_type == ITEM_SPAWNER )
    {
       OBJ_INDEX_DATA * pObjI = NULL;
       OBJ_DATA * nObj = NULL;
       int weight = 0;

       if ( device->value[0] <= 0 )
       {
          ch_printf( ch, "Sorry, but its empty!\n\r" );
          return 0;
       }

       if ( ( pObjI = get_obj_index( device->value[2] ) ) == NULL )
       {
          bug("do_use: spawner failed to create %d!", device->value[2]);
          return 0;
       }
       nObj = create_object( pObjI, UMAX(1, ch->top_level) );
       nObj = obj_to_char( nObj, ch );

       device->value[0]--;

       // Counter balance the case.
       weight = nObj->weight;
       if ( device->weight - weight < 0 ) { weight = device->weight - 1; }
       device->weight -= weight;
       ch->carry_weight -= weight;
       device->cost = UMAX( 0, device->cost - nObj->cost );

       match_log( "ITEM;%s used 'SPAWNER:%s'", ch->name, device->short_descr );

       send_to_char( "\n\r", ch );
       act( AT_ACTION, "$n grabs something from $p.", ch, device, NULL, TO_ROOM );
       act( AT_ACTION, "You reach into $p.", ch, device, NULL, TO_CHAR );

       if ( device->value[3] == 1 ) // Autowear
         wear_obj( ch, nObj, FALSE, -1 );

       if ( device->value[0] <= 0 )
       {
         /* Empty - Discard it */
         ch_printf( ch, "&C(Automatic) You discard the empty item.\n\r" );
         device->timer = 2;
         unequip_char( ch, device );
         obj_from_char( device );
         device = obj_to_room( device, ch->in_room );
         oprog_drop_trigger ( ch, device );   /* mudprogs */

       }
 
       return 0;
    }
    else if ( device->item_type == ITEM_DEPLOYER )
    {
       OBJ_INDEX_DATA * pObjI = NULL;
       OBJ_DATA * nObj = NULL;
       int dCnt = 0;

       if ( mode == 1 )
       {
          if ( device->value[3] > 0 )
          {
             if ( device->value[3] == 1 && !IS_INSIDE(ch) )
             {
                ch_printf( ch, "&R(Deploy Item) This item can only be used indoors.\n\r" );
                return 0;
             }
             if ( device->value[3] == 2 && !IS_OUTSIDE(ch) )
             {
                ch_printf( ch, "&R(Deploy Item) This item can only be used outdoors.\n\r" );
                return 0;
             }
             if ( device->value[3] == 3 && !IS_INHIVE(ch) )
             {
                ch_printf( ch, "&R(Deploy Item) This item can only be used in hives.\n\r" );
                return 0;
             }
          }

          act( AT_ACTION, "$n starts using $p.", ch, device, NULL, TO_ROOM );
          act( AT_ACTION, "You start using $p.", ch, device, NULL, TO_CHAR );

          return UMAX( 1, device->value[0] );
       }
       else if ( mode == 2 )
       {
          if ( ( pObjI = get_obj_index( device->value[1] ) ) == NULL )
          {
             bug("do_use: deployer failed to create %d!", device->value[1]);
             return 0;
          }

          for ( dCnt = 0; dCnt < URANGE( 1, device->value[2], 99 ); dCnt ++ )
          {
             nObj = create_object( pObjI, UMAX(1, ch->top_level) );
             nObj = obj_to_room( nObj, ch->in_room );
          }

          match_log( "ITEM;%s used 'DEPLOYER:%s'", ch->name, device->short_descr );
          
          send_to_char( "\n\r", ch );
          act( AT_ACTION, "$n deploys $p.", ch, nObj, NULL, TO_ROOM );
          act( AT_ACTION, "You deploy $p.", ch, nObj, NULL, TO_CHAR );

          unequip_char( ch, device );
          obj_from_char( device );
          if ( obj_extracted(device) ) return;
          if ( cur_obj == device->serial ) global_objcode = rOBJ_SACCED;
          extract_obj( device );

          return 0;
       }
       else if ( mode == 3 )
       {
          ch_printf( ch, "Your interrupted before you can finish your work.\n\r" );
          return 0;
       }

       return 0;
    }
    else if ( device->item_type == ITEM_LANDMINE )
    {
       if ( IS_OBJ_STAT( device, ITEM_BURRIED ) )
       {
          send_to_char( "&RThat landmine is already armed!\n\r", ch );
          return 0;
       }
  
       match_log( "ITEM;%s used 'LANDMINE:%s'", ch->name, device->short_descr );

       // unequip_char( ch, device );
       obj_from_char( device );
       device = obj_to_room( device, ch->in_room );
       oprog_drop_trigger ( ch, device );   /* mudprogs */

       if ( device->armed_by ) STRFREE( device->armed_by ); 
       device->armed_by = STRALLOC( ch->name );

       xSET_BIT( device->extra_flags, ITEM_BURRIED );

       act( AT_ACTION, "$n activate $p.", ch, device, NULL, TO_ROOM );
       act( AT_ACTION, "You activate $p.", ch, device, NULL, TO_CHAR );

       return 0;
    }
    else if ( device->item_type == ITEM_MEDIKIT && ch->race == RACE_PREDATOR )
    {      
       if ( !IS_NPC( ch ) )
       {
           if ( ch->pcdata->learned[gsn_medical] < 2 )
           {
                send_to_char( "You need Advanced Medical skill to use Medikits.\n\r", ch );
                return 0;
           }
       }
       
       if ( device->value[1] > 0 )
       {
           if ( ( victim = get_char_room( ch, arg ) ) != NULL )
           {
                if ( victim->hit >= victim->max_hit )
                {
                    send_to_char( "&RThey don't seem to be injured at all.\n\r", ch );
                    return 0;
                }
                act( AT_GREEN, "You use the medikit to heal $N.",  ch, NULL, victim, TO_CHAR    );
                act( AT_GREEN, "$n uses a medikit to heal you.", ch, NULL, victim, TO_VICT    );
                act( AT_GREEN, "$n uses a medikit on $N.",  ch, NULL, victim, TO_NOTVICT );
                victim->hit = UMIN( victim->max_hit, victim->hit + device->value[0] );
                device->value[1]--;

                match_log( "ITEM;%s used 'MEDIKIT:%s'", ch->name, device->short_descr );

                ch_printf( ch, "&Y(Use Action) It will take 3 rounds to complete.\n\r" );
                WAIT_STATE( ch, (4 * 3) );

                short_target( ch, ch, 3 );
           }
           else
           {
               send_to_char( "&RUse the Medikit on whom?\n\r", ch );
               return 0;
           }
       }
       else
       {
          send_to_char( "&RThe medikit seems to be empty.\n\r", ch );
          return 0;
       }
    }
    else if ( device->item_type == ITEM_STERIL )
    {
       int hp;

       if ( !IS_NPC( ch ) )
       {
           if ( ch->pcdata->learned[gsn_medical] < 1 )
           {
                send_to_char( "You need Basic Medical skill to use these.\n\r", ch );
                return 0;
           }
       }
       
       if ( ch->hit >= ch->max_hit )
       {
           send_to_char( "&RYou don't seem to be injured at all.\n\r", ch );
           return 0;
       }

       hp = UMAX( 0, UMIN( ch->max_hit - ch->hit, device->value[0] ) );
       ch->hit += hp;

       act( AT_ACTION, "$n actives $p, and quickly regains life!",  ch, device, NULL, TO_ROOM );
       act( AT_ACTION, "You active $p, restoring much of your health.", ch, device, NULL, TO_CHAR );

       ch_printf( ch, "&Y(Use Action) It will take %d rounds to complete.\n\r", device->value[1] );
       WAIT_STATE( ch, (4 * device->value[1]) );

       match_log( "ITEM;%s used 'STERIL:%s'", ch->name, device->short_descr );

       extract_obj( device );
       return 0;
    }
    else if ( device->item_type == ITEM_SENTRY )
    {
       if ( mode == 2 )
       {
           if ( !ch->pcdata || ch->pcdata->learned[gsn_electronics] < 2 )
           {
              ch_printf( ch, "&w&RSentry Guns require a minimum of two ranks in Electronics.\n\r" );
              return 0;
           }

           if ( get_sentry( device ) == NULL )
           {
              int dir = 0;

              if ( arg[0] == '\0' || ( dir = get_door( arg ) ) == -1 || dir > DIR_SOUTHWEST ) return 0;

              if ( !device->in_room )
              {
                 act( AT_ACTION, "$n drops $p.", ch, device, NULL, TO_ROOM );
                 act( AT_ACTION, "You drop $p.", ch, device, NULL, TO_CHAR );
                 obj_from_char( device );
                 device = obj_to_room( device, ch->in_room );
              }

              // Sentry Gun is now armed.
              act( AT_ACTION, "$n activates $p.", ch, device, NULL, TO_ROOM );
              act( AT_ACTION, "You activate $p.", ch, device, NULL, TO_CHAR );

              if ( !device->armed_by )
              {
                 match_log( "ITEM;%s used 'SENTRY:%s'", ch->name, device->short_descr );

                 STRFREE( device->armed_by );
                 device->armed_by = STRALLOC( ch->name );

                 ch_printf( ch, "&w&RDont forget to load the Sentry Gun!\n\r" );
              }

              add_sentry( device, ch, dir );

              // ch_printf( ch, "&Y(Use Action) It will take 5 rounds to complete.\n\r" );
              // WAIT_STATE( ch, (4 * 5) );

              return 0;
           }
           else
           {
              if ( rem_sentry( device, ch ) == TRUE )
              {
                  act( AT_ACTION, "$n deactivates $p.", ch, device, NULL, TO_ROOM );
                  act( AT_ACTION, "You deactivate $p.", ch, device, NULL, TO_CHAR );

                  if ( device->in_room )
                  {
                      if ( ch->carry_number + get_obj_number( device ) > can_carry_n( ch ) ) return 0;
                      if ( ch->carry_weight + get_obj_weight( device ) > can_carry_w( ch ) ) return 0;

                      act( AT_ACTION, "$n gets $p.", ch, device, NULL, TO_ROOM );
                      act( AT_ACTION, "You get $p.", ch, device, NULL, TO_CHAR );
                      obj_from_room( device );
                      device = obj_to_char( device, ch );
                  }
              }

              return 0;
           }
       }
       else if ( mode == 1 )
       {
           int ticks = 3;
           int dir = 0;

           if ( !ch->pcdata || ch->pcdata->learned[gsn_electronics] < 2 )
           {
              ch_printf( ch, "&w&RSentry Guns require a minimum of two ranks in Electronics.\n\r" );
              return 0;
           }

           if ( get_sentry( device ) == NULL )
           {
              if ( arg[0] == '\0' || ( dir = get_door( arg ) ) == -1 || dir > DIR_SOUTHWEST )
              {
                 ch_printf( ch, "&w&RSyntax: USE SENTRY (Direction)\n\r" );
                 ch_printf( ch, "&w&R Sentry Guns have limited firing arc.\n\r" );
                 return 0;
              }

              if ( dir == DIR_UP || dir == DIR_DOWN )
              {
                 ch_printf( ch, "&w&RSentry Guns cannot fire up or down between floors.\n\r" );
                 return 0;
              }
           
              ticks = 5;
           }

           ch_printf( ch, "&Y(Use Action) It will take %d rounds to complete, Please Wait.\n\r", ticks );
           return ticks;
       }
       else
       {
           send_to_char("&RYou are interrupted before you can finish your work.\n\r", ch);
           return 0;
       }
       return 0;
    }
    else
    {
       if ( !use_attachment( ch, device ) )
       {
          send_to_char( "&RYou have no clue what to do with that.\n\r", ch );
          return 0;
       }
    }

    return 0;
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;

    argument = one_argument( argument, arg );
    /* munch optional words */
    if ( !str_cmp( arg, "from" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
        if ( (obj->item_type == ITEM_FOUNTAIN) )
		break;

	if ( !obj )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
        send_to_char( "You cant find it.\n\r", ch );
	    return;
	}
    }

    if ( obj->count > 1 && obj->item_type != ITEM_FOUNTAIN )
       separate_obj(obj);

    switch ( obj->item_type )
    {
       default:
       if ( obj->carried_by == ch )
       {
          act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
          act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
       }
       else
       {
          act( AT_ACTION, "$n gets down and tries to drink from $p... (Is $e feeling ok?)", ch, obj, NULL, TO_ROOM );
          act( AT_ACTION, "You get down on the ground and try to drink from $p...", ch, obj, NULL, TO_CHAR );
       }
       break;

       case ITEM_FOUNTAIN:
         if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
         {
            act( AT_ACTION, "$n drinks from the fountain.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You take a long thirst quenching drink.\n\r", ch );
         }
         break;

    }
    WAIT_STATE(ch, PULSE_PER_SECOND );
    return;
}

void do_eat( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int foodcond;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) )
	if ( ms_find_obj(ch) )
	    return;

    if ( (obj = find_obj(ch, argument, TRUE)) == NULL )
	return;

    if ( !IS_IMMORTAL(ch) )
    {
        if ( obj->item_type != ITEM_FOOD )
        {
	    act( AT_ACTION, "$n starts to nibble on $p... ($e must really be hungry)",  ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You try to nibble on $p...", ch, obj, NULL, TO_CHAR );
	    return;
        }
    }

    /* required due to object grouping */
    separate_obj( obj );
    
    WAIT_STATE( ch, PULSE_PER_SECOND/2 );
    
    if ( obj->in_obj )
    {
	act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
	act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
    }
    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
    {
      if ( !obj->action_desc || obj->action_desc[0]=='\0' )
      {
        act( AT_ACTION, "$n eats $p.",  ch, obj, NULL, TO_ROOM );
        act( AT_ACTION, "You eat $p.", ch, obj, NULL, TO_CHAR );
      }
      else
        actiondesc( ch, obj, NULL ); 
    }

    switch ( obj->item_type )
    {

        case ITEM_FOOD:

           ch->hit = URANGE( 0, ch->hit + obj->value[0], ch->max_hit );
           ch->move = URANGE( 0, ch->move + obj->value[1], ch->max_move );

           if ( !IS_NPC(ch) )  send_to_char( "&CYou feel very refreshed.\n\r", ch );

           break;

    }

    if ( obj->serial == cur_obj )
      global_objcode = rOBJ_EATEN;
    extract_obj( obj );
    return;
}

void do_empty( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "into" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Empty what?\n\r", ch );
	return;
    }
    if ( ms_find_obj(ch) )
	return;

    if ( (obj = get_obj_carry( ch, arg1 )) == NULL )
    {
    send_to_char( "You arent carrying that.\n\r", ch );
	return;
    }
    if ( obj->count > 1 )
      separate_obj(obj);

    switch( obj->item_type )
    {
	default:
	  act( AT_ACTION, "You shake $p in an attempt to empty it...", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n begins to shake $p in an attempt to empty it...", ch, obj, NULL, TO_ROOM );
	  return;
	case ITEM_DRINK_CON:
	  if ( obj->value[1] < 1 )
	  {
              send_to_char( "Its already empty.\n\r", ch );
              return;
	  }
	  act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
	  obj->value[1] = 0;
	  return;
        case ITEM_WEAPON:
          if ( obj->ammo == NULL )
          {
               send_to_char( "It doesn't have any removeable ammo loaded.\n\r", ch );
               return;
          }
          else
          {
               ch_printf( ch, "You remove a %s from a %s.\n\r", obj->ammo->short_descr, obj->short_descr );
               act( AT_PLAIN, "$n removes $p from a weapon.", ch, obj->ammo, NULL, TO_ROOM );        

               obj_to_char( obj->ammo, ch );
               obj->ammo = NULL;
               return;
          }
          return;
	case ITEM_CONTAINER:
	  if ( IS_SET(obj->value[1], CONT_CLOSED) )
	  {
		act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
		return;
	  }
	  if ( !obj->first_content )
	  {
        send_to_char( "Its already empty.\n\r", ch );
		return;
	  }
	  if ( arg2[0] == '\0' )
	  {
        if ( xIS_SET( ch->in_room->room_flags, ROOM_NODROP )
        || ( !IS_NPC(ch) &&  xIS_SET( ch->act, PLR_LITTERBUG ) ) )
		{
		       set_char_color( AT_MAGIC, ch );
		       send_to_char( "A magical force stops you!\n\r", ch );
		       set_char_color( AT_TELL, ch );
		       send_to_char( "Someone tells you, 'No littering here!'\n\r", ch );
		       return;
		}
        if ( xIS_SET( ch->in_room->room_flags, ROOM_NODROPALL ) )
		{
           send_to_char( "You cant seem to do that here...\n\r", ch );
		   return;
		}
		if ( empty_obj( obj, NULL, ch->in_room ) )
		{
		    act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
		    act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
            if ( IS_SET( sysdata.save_flags, SV_DROP ) )
			save_char_obj( ch );
		}
		else
            send_to_char( "Hmmm... didnt work.\n\r", ch );
	  }
	  else
	  {
		OBJ_DATA *dest = get_obj_here( ch, arg2 );

		if ( !dest )
		{
            send_to_char( "You cant find it.\n\r", ch );
		    return;
		}
		if ( dest == obj )
		{
            send_to_char( "You cant empty something into itself!\n\r", ch );
		    return;
		}
		if ( dest->item_type != ITEM_CONTAINER )
		{
            send_to_char( "Thats not a container!\n\r", ch );
		    return;
		}
		if ( IS_SET(dest->value[1], CONT_CLOSED) )
		{
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, dest->name, TO_CHAR );
		    return;
		}
		separate_obj( dest );
		if ( empty_obj( obj, dest, NULL ) )
		{
		    act( AT_ACTION, "You empty $p into $P.", ch, obj, dest, TO_CHAR );
		    act( AT_ACTION, "$n empties $p into $P.", ch, obj, dest, TO_ROOM );
		    if ( !dest->carried_by
		    &&    IS_SET( sysdata.save_flags, SV_PUT ) )
			save_char_obj( ch );
		}
		else
		    act( AT_ACTION, "$P is too full.", ch, obj, dest, TO_CHAR );
	  }
	  return;
    }
}
 
/*
 * Apply a salve/ointment					-Thoric
 */
void do_apply( CHAR_DATA *ch, char *argument )
{
    /*
    OBJ_DATA *obj;
    ch_ret retcode;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Apply what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
    {
	send_to_char( "You do not have that.\n\r", ch );
	return;
    }

    if ( obj->item_type != ITEM_SALVE )
    {
	act( AT_ACTION, "$n starts to rub $p on $mself...",  ch, obj, NULL, TO_ROOM );
	act( AT_ACTION, "You try to rub $p on yourself...", ch, obj, NULL, TO_CHAR );
	return;
    }

    separate_obj( obj );

    --obj->value[1];
    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
    {
	if ( !obj->action_desc || obj->action_desc[0]=='\0' )
	{
	    act( AT_ACTION, "$n rubs $p onto $s body.",  ch, obj, NULL, TO_ROOM );
	    if ( obj->value[1] <= 0 )
		act( AT_ACTION, "You apply the last of $p onto your body.", ch, obj, NULL, TO_CHAR );
	    else
		act( AT_ACTION, "You apply $p onto your body.", ch, obj, NULL, TO_CHAR );
	}
	else
	    actiondesc( ch, obj, NULL ); 
    }

    WAIT_STATE( ch, obj->value[2] );
    retcode = obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL );
    if ( retcode == rNONE )
	retcode = obj_cast_spell( obj->value[5], obj->value[0], ch, ch, NULL );

    if ( !obj_extracted(obj) && obj->value[1] <= 0 )
	extract_obj( obj );
    */
    return;
}

void actiondesc( CHAR_DATA *ch, OBJ_DATA *obj, void *vo )
{
    char charbuf[MAX_STRING_LENGTH];
    char roombuf[MAX_STRING_LENGTH];
    char *srcptr = obj->action_desc;
    char *charptr = charbuf;
    char *roomptr = roombuf;
    const char *ichar;
    const char *iroom;

while ( *srcptr != '\0' )
{
  if ( *srcptr == '$' ) 
  {
    srcptr++;
    switch ( *srcptr )
    {
      case 'e':
        ichar = "you";
        iroom = "$e";
        break;

      case 'm':
        ichar = "you";
        iroom = "$m";
        break;

      case 'n':
        ichar = "you";
        iroom = "$n";
        break;

      case 's':
        ichar = "your";
        iroom = "$s";
        break;

      /*case 'q':
        iroom = "s";
        break;*/

      default: 
        srcptr--;
        *charptr++ = *srcptr;
        *roomptr++ = *srcptr;
        break;
    }
  }
  else if ( *srcptr == '%' && *++srcptr == 's' ) 
  {
    ichar = "You";
    iroom = IS_NPC( ch ) ? ch->short_descr : ch->name;
  }
  else
  {
    *charptr++ = *srcptr;
    *roomptr++ = *srcptr;
    srcptr++;
    continue;
  }

  while ( ( *charptr = *ichar ) != '\0' )
  {
    charptr++;
    ichar++;
  }

  while ( ( *roomptr = *iroom ) != '\0' )
  {
    roomptr++;
    iroom++;
  }
  srcptr++;
}

*charptr = '\0';
*roomptr = '\0';

/*
sprintf( buf, "Charbuf: %s", charbuf );
log_string_plus( buf, LOG_HIGH, LEVEL_LESSER ); 
sprintf( buf, "Roombuf: %s", roombuf );
log_string_plus( buf, LOG_HIGH, LEVEL_LESSER ); 
*/

switch( obj->item_type )
{
  case ITEM_FOUNTAIN:
    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
    return;

  case ITEM_ARMOR:
  case ITEM_WEAPON:
  case ITEM_LIGHT:
    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
    return;
 
  case ITEM_FOOD:
    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
    return;

  default:
    return;
}
return;
}


#define MPADD_FRMT "allowmp <add|delete> <site|name> <site or name>"
void save_allowmp( void)
{
  struct allowmp_data *mp;
  FILE *fp;
  if(!(fp = fopen(SYSTEM_DIR MP_LIST, "w" ))){
    /*monitor("UNABLE TO SAVE MPLIST -- tell quema",MON_ERROR,LEVEL_IMMORTAL,1);*/
        bug( "Unable to save mplist", 0 );
	return;
  }
  for(mp = mplist; mp ; mp = mp->next)
	fprintf(fp,"%s %s %s~\n",mp->by,mp->name,mp->host);
  fprintf(fp,"###~\n\n\n");
  fclose(fp);
}


int allowedmp(DESCRIPTOR_DATA *d){
  struct allowmp_data *mp;

  for(mp=mplist; mp; mp = mp->next)
	if(((strstr(mp->host,d->host))) || (!(str_cmp(mp->name,d->character->name)))) return 1;
  return 0;
}


void do_allowmp( CHAR_DATA *ch, char *argument )
{
  struct allowmp_data *mp,*mp2;
   char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH],buf3[MAX_STRING_LENGTH];
   argument=one_argument(argument,buf); 
   argument=one_argument(argument,buf2); 
   one_argument(argument,buf3); 
   if(buf[0] == '\0'){
        sprintf(buf,"\r\n\r\n&W   Allowed sites for multiplaying\r\n"
		    "   Allowed by | Allowed For  | Site\r\n");
	send_to_char(buf,ch);
      for(mp= mplist; mp && mp->name[0] != '\0'; mp= mp->next){
        sprintf(buf,"   %-10s | %-10s | %-25s\r\n",mp->by,mp->name,mp->host);
	send_to_char(buf,ch);
      }
	return;
   }
  if(!(str_cmp(buf,"add"))){
	if(!(str_cmp(buf2,"site"))){
		if(buf3[0] == '\0'){
			send_to_char(MPADD_FRMT,ch);
			return;
		}
	 	CREATE(mp,struct allowmp_data,1);
		strcpy(mp->by,ch->name);
		strcpy(mp->host,buf3);
		strcpy(mp->name,"*");	
		mp->next = mplist;
		mplist = mp;
		save_allowmp();
		send_to_char("OK\r\n",ch);
	}
	else if(!(str_cmp(buf2,"name"))){
		if(buf3[0] == '\0'){
			send_to_char(MPADD_FRMT,ch);
			return;
		}
	 	CREATE(mp,struct allowmp_data,1);
		strcpy(mp->by,ch->name);
		strcpy(mp->host,"*");
		strcpy(mp->name,buf3);
		mp->next = mplist;
		mplist = mp;
		save_allowmp();
		send_to_char("OK\r\n",ch);
        }else{
		send_to_char(MPADD_FRMT,ch);
		return;
        }
		return;	
  }else if(!(str_cmp(buf,"delete"))){
	if(!(str_cmp(buf2,"name"))){
		if(buf3[0] == '\0'){
			send_to_char(MPADD_FRMT,ch);
			return;
		}
        for(mp = mplist; mp; mp2 = mp,mp = mp->next)
	{
	   if(!(str_cmp(mp->name,buf3)))
	   {
		if(mp == mplist)
		   mplist = mp->next;
		else
		   mp2->next = mp->next;
	   }
	}
             save_allowmp();
		send_to_char("OK\r\n",ch);
        }else if(!str_cmp(buf2,"site")){
		if(buf3[0] == '\0'){
			send_to_char(MPADD_FRMT,ch);
			return;
		}
        for(mp = mplist; mp; mp2 = mp,mp = mp->next)
	{
	    if(!(str_cmp(mp->host,buf3)))
	    {
		if(mp == mplist)
		   mplist = mp->next;
		else
		   mp2->next = mp->next;
	    }
	}
             save_allowmp();
		send_to_char("OK\r\n",ch);
             return;
        }
	return;
     }
		send_to_char(MPADD_FRMT,ch);
		return;
}
	


#define XNAMEUSE "xname <noarg/add/delete> <name> <time><m/h/d>\r\n"
void do_xname( CHAR_DATA *ch, char *argument ){
   char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH],buf3[MAX_STRING_LENGTH];
   struct xname_data *xname, *x2;
   int i,j,mult;
   char mine[5];
   argument=one_argument(argument,buf); 
   argument=one_argument(argument,buf2); 
   one_argument(argument,buf3); 

   if(buf[0] == '\0'){
     sprintf(buf," ---------Xnames-----------------\r\n");
      for(xname = xnames; xname; xname = xname->next)
          sprintf(buf,"%s    %-11s %s\r\n",buf,xname->name ,ctime(&xname->time));
      send_to_char(buf,ch);
   }

   if(str_cmp(buf,"add")){
        if(buf2[0] == '\0' || buf3[0] == '\0'){
		send_to_char(XNAMEUSE,ch);
		return;
        }
        CREATE(xname,struct xname_data,1);
        strcpy(xname->name, buf2);
        for(i=0;buf3[i] != '\0'; i++){
            switch (buf3[i]){
		case 'h':
		case 'H':
                      mult = 60 * 60;
		      break;
		case 'd':	   
		case 'D':	   
		      mult = 60 * 60 * 24;
		      break;
		case 'm':	   
		case 'M' :
		      mult = 60;
		      break;
                default: 
                    mine[j] = buf3[i] ;
		    j++;
                    break;
            };
            mine[j] = '\0';
            xname->time = atol(mine) * (long)mult;
            xname->next = xnames;
     }
           send_to_char("DoNe!!!!\r\n",ch);
    }
   if(str_cmp(buf,"delete")){
        if(buf2[0] == '\0'){
		send_to_char(XNAMEUSE,ch);
		return;
        }
        x2 = NULL;
        for(xname = xnames; xname; xname = xname->next)
	{
	      if(str_cmp(xname->name,buf2))
	      {
                      if(xname->next)
		      {
			 if(x2)
			     x2->next = xname->next;
			 else
			     xnames = xname->next;
		      }
                      else
		      {
			if(x2)
			    x2->next = NULL;
			else
			     xnames = NULL;
		      }
	      }
	}
	send_to_char("Done.\r\n",ch);
   }
         

}

void do_uptime( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  FILE *fp;
  int pid;
  int psize;
  int i;
  int c;

  fp = popen(UPTIME_PATH, "r");
  
  /* just in case the system is screwy */
  if (fp == NULL) {
     bug("** ERROR ** popen for uptime returned NULL.", 0);
     return;
     }
  /* print system uptime */
  for (i = 0; (c = getc(fp)) != '\n'; i++)
     buf[i] = c;
  buf[i] = '\0';
  pclose(fp);
  
  send_to_char(buf, ch);
  
  pid = getpid();
  psize = getpagesize();
  sprintf(buf, "\n\rProcess ID: %10d\n\r", pid);
  send_to_char(buf, ch);
}

/* check to see if the extended bitvector is completely empty */
bool ext_is_empty( EXT_BV *bits )
{
    int x;

    for ( x = 0; x < XBI; x++ )
	if ( bits->bits[x] != 0 )
	    return FALSE;

    return TRUE;
}

void ext_clear_bits( EXT_BV *bits )
{
    int x;

    for ( x = 0; x < XBI; x++ )
	bits->bits[x] = 0;
}

/* for use by xHAS_BITS() -- works like IS_SET() */
int ext_has_bits( EXT_BV *var, EXT_BV *bits )
{
    int x, bit;

    for ( x = 0; x < XBI; x++ )
	if ( (bit=(var->bits[x] & bits->bits[x])) != 0 )
	    return bit;

    return 0;
}

/* for use by xSAME_BITS() -- works like == */
bool ext_same_bits( EXT_BV *var, EXT_BV *bits )
{
    int x;

    for ( x = 0; x < XBI; x++ )
	if ( var->bits[x] != bits->bits[x] )
	    return FALSE;

    return TRUE;
}

/* for use by xSET_BITS() -- works like SET_BIT() */
void ext_set_bits( EXT_BV *var, EXT_BV *bits )
{
    int x;

    for ( x = 0; x < XBI; x++ )
	var->bits[x] |= bits->bits[x];
}

/* for use by xREMOVE_BITS() -- works like REMOVE_BIT() */
void ext_remove_bits( EXT_BV *var, EXT_BV *bits )
{
    int x;

    for ( x = 0; x < XBI; x++ )
	var->bits[x] &= ~(bits->bits[x]);
}

/* for use by xTOGGLE_BITS() -- works like TOGGLE_BIT() */
void ext_toggle_bits( EXT_BV *var, EXT_BV *bits )
{
    int x;

    for ( x = 0; x < XBI; x++ )
	var->bits[x] ^= bits->bits[x];
}

/*
 * Read an extended bitvector from a file.			-Thoric
 */
EXT_BV fread_bitvector( FILE *fp )
{
    EXT_BV ret;
    int c, x = 0;
    int num = 0;
    
    memset( &ret, '\0', sizeof(ret) );
    for ( ;; )
    {
	num = fread_number(fp);
	if ( x < XBI )
	    ret.bits[x] = num;
	++x;
	if ( (c=getc(fp)) != '&' )
	{
	    ungetc(c, fp);
	    break;
	}
    }

    return ret;
}

/* return a string for writing a bitvector to a file */
char *print_bitvector( EXT_BV *bits )
{
    static char buf[XBI * 12];
    char *p = buf;
    int x, cnt = 0;

    for ( cnt = XBI-1; cnt > 0; cnt-- )
	if ( bits->bits[cnt] )
	    break;
    for ( x = 0; x <= cnt; x++ )
    {
	sprintf(p, "%d", bits->bits[x]);
	p += strlen(p);
	if ( x < cnt )
	    *p++ = '&';
    }
    *p = '\0';

    return buf;
}

/*
 * Write an extended bitvector to a file			-Thoric
 */
void fwrite_bitvector( EXT_BV *bits, FILE *fp )
{
    fputs( print_bitvector(bits), fp );
}


EXT_BV meb( int bit )
{
    EXT_BV bits;

    xCLEAR_BITS(bits);
    if ( bit >= 0 )
	xSET_BIT(bits, bit);

    return bits;
}


EXT_BV multimeb( int bit, ... )
{
    EXT_BV bits;
    va_list param;
    int b;
    
    xCLEAR_BITS(bits);
    if ( bit < 0 )
	return bits;

    xSET_BIT(bits, bit);

    va_start(param, bit);

    while ((b = va_arg(param, int)) != -1)
	xSET_BIT(bits, b);

    va_end(param);

    return bits;
}

void add_sentry( OBJ_DATA * obj, CHAR_DATA * ch, int dir )
{
    SENTRY_DATA * gun;

    if ( get_sentry( obj ) != NULL ) return;

    CREATE( gun, SENTRY_DATA, 1 );

    LINK( gun, first_sentry, last_sentry, next, prev );

    gun->arc = dir;
    gun->temp = 0;
    gun->wait = 0;

    gun->owner = ch;
    gun->gun = obj;

    return;
}

bool rem_sentry( OBJ_DATA * obj, CHAR_DATA * ch )
{
  SENTRY_DATA * tmp = NULL;

  /*
   * CH may be null - It means this object MUST be removed.
   */
  for ( tmp = first_sentry; tmp; tmp = tmp->next )
  {
     if ( tmp->gun == obj )
     {
         if ( ch != NULL && tmp->owner != ch )
         {
             ch_printf( ch, "&w&RThis weapon was not activated by you! Nice try, thief.\n\r" );
             return FALSE;
         }

         UNLINK( tmp, first_sentry, last_sentry, next, prev);
         DISPOSE( tmp );

         return TRUE;
     }
  }

  return FALSE;
}

void rempc_sentry( CHAR_DATA * ch )
{
  SENTRY_DATA * tmp = NULL;
  SENTRY_DATA * tnext = NULL;

  for ( tmp = first_sentry; tmp; tmp = tnext )
  {
      tnext = tmp->next;

      if ( tmp->owner == ch )
      {
         UNLINK( tmp, first_sentry, last_sentry, next, prev);
         DISPOSE( tmp );
      }
  }

  return;
}

SENTRY_DATA * get_sentry( OBJ_DATA * obj )
{
   SENTRY_DATA * tmp = NULL;

   for ( tmp = first_sentry; tmp; tmp = tmp->next )
     if ( tmp->gun == obj ) return tmp;

   return NULL;
}

void update_sentry( void )
{
   ROOM_INDEX_DATA * my_room = NULL;
   ROOM_INDEX_DATA * to_room = NULL;
   SENTRY_DATA     * tmp = NULL;
   CHAR_DATA       * rch = NULL;
   CHAR_DATA       * ch = NULL;
   EXIT_DATA       * pexit = NULL;
   OBJ_DATA        * gun = NULL;
   bool tripped = FALSE;
   int dist = 0, dir = 0;

   if ( first_sentry == NULL ) return;

   for ( tmp = first_sentry; tmp; tmp = tmp->next )
   {
       ch = tmp->owner;
       gun = tmp->gun;

       if ( !ch || ch == NULL ) continue;
       if ( !gun || gun == NULL ) continue;

       if ( !ch->in_room ) continue;
       if ( !gun->in_room ) continue;

       if ( tmp->wait > 0 )
       {
           tmp->wait--;
           continue;
       }

       if ( !gun->ammo )
       {
           /* Maybe add a 'out of ammo' beeping alert here. */
           continue;
       }

       if ( tmp->temp > gun->value[5] )
       {
           // Weapon is overheating. Slow it down a bit.
           if ( number_range(1, 3) == 1 ) tmp->temp--;
           if ( number_range(1, 3) == 1 ) tmp->wait++;
           if ( number_range(1, 3) == 1 ) continue;
       }

       tripped = FALSE;

       // Scan directions for targets.
       for ( dir = 0; dir <= 9; dir++ )
       {  
           // Limited Firing Arc
           switch ( tmp->arc )
           {
             default:
             case DIR_NORTH:
               if ( dir != DIR_NORTH && dir != DIR_NORTHEAST && dir != DIR_NORTHWEST ) continue;
               break;
             case DIR_EAST:
               if ( dir != DIR_EAST && dir != DIR_NORTHEAST && dir != DIR_SOUTHEAST ) continue;
               break;
             case DIR_SOUTH:
               if ( dir != DIR_SOUTH && dir != DIR_SOUTHEAST && dir != DIR_SOUTHWEST ) continue;
               break;
             case DIR_WEST:
               if ( dir != DIR_WEST && dir != DIR_NORTHWEST && dir != DIR_SOUTHWEST ) continue;
               break;
             case DIR_NORTHWEST:
               if ( dir != DIR_NORTH && dir != DIR_WEST && dir != DIR_NORTHWEST ) continue;
               break;
             case DIR_NORTHEAST:
               if ( dir != DIR_NORTH && dir != DIR_NORTHEAST && dir != DIR_EAST ) continue;
               break;
             case DIR_SOUTHWEST:
               if ( dir != DIR_SOUTH && dir != DIR_SOUTHWEST && dir != DIR_WEST ) continue;
               break;
             case DIR_SOUTHEAST:
               if ( dir != DIR_SOUTH && dir != DIR_SOUTHEAST && dir != DIR_EAST ) continue;
               break;
           }

           // if ( dir == DIR_UP || dir == DIR_DOWN ) continue;

           if ( ( pexit = get_exit( gun->in_room, dir ) ) == NULL ) continue;

           if ( tripped ) break;

           for ( dist = 1; dist <= gun->value[2]; dist++ )
           {
              if ( xIS_SET( pexit->exit_info, EX_CLOSED ) ) break;      
              if ( !pexit->to_room ) break;
     
              if ( tripped ) break;

              to_room = NULL;

              if ( pexit->distance > 1 ) to_room = generate_exit( gun->in_room , &pexit );
                                 
              if ( to_room == NULL ) to_room = pexit->to_room;
    
              my_room = to_room;

              for ( rch = my_room->first_person; rch; rch = rch->next_in_room )
              {
                 if ( tripped ) break;
                 if ( is_enemy( rch, ch ) && !is_spectator( rch ) && !IN_VENT( rch ) && rch->position != POS_PRONE )
                 {
                    // Found Enemy - Commence Firing
                    deploy_fire( tmp, dir );
                    tripped = TRUE;
                 }
              }

              if ( ( pexit = get_exit( my_room, dir ) ) == NULL ) break;
           }  
       }

       // Done Scanning - Permit the gun to cool.
       if ( !tripped )
       {
          tmp->temp = UMAX( 0, tmp->temp - 1 );
       }
   }

   return;
}

/*
 * Commence Sentry Gun Fire - SPRAY Pattern
 */
void deploy_fire( SENTRY_DATA * gun, int dir )
{
   char buf[MAX_STRING_LENGTH];
   ROOM_INDEX_DATA * to_room;
   CHAR_DATA * ch = NULL;
   CHAR_DATA * rnext = NULL;
   CHAR_DATA * rch = NULL;
   OBJ_DATA  * weap = NULL;
   OBJ_DATA  * ammo = NULL;
   EXIT_DATA * pexit = NULL;
   int rounds=0, mrounds=0;
   int range=0, mrange=0;
   int hits=0, misses=0;
   int chance=0, dam=0;

   if ( !gun || gun == NULL ) return;

   ch = gun->owner;
   weap = gun->gun;
   ammo = weap->ammo;

   if ( !ch || ch == NULL ) return;
   if ( !weap || weap == NULL ) return;
   if ( !ammo || ammo == NULL ) return;

   if ( !ch->desc || ch->desc == NULL ) return;

   mrange = weap->value[2];

   mrounds = URANGE( 0, weap->value[1], ammo->value[2] );

   if ( ( rounds = mrounds ) <= 0 ) return;

   ammo->value[2] -= rounds;

   gun->temp += 4;

   sprintf( buf, "%s fires a burst of rounds %s.", weap->short_descr, dir_name[dir] );
   echo_to_room( AT_RED, weap->in_room, buf );

   pexit = get_exit( weap->in_room, dir );

   for ( range = 1; range <= mrange; range++ )
   {
       if ( !pexit ) break;
       if ( xIS_SET( pexit->exit_info, EX_CLOSED ) ) break;
       if ( !pexit->to_room ) break;    

       to_room = NULL;
       if ( pexit->distance > 1 ) to_room = generate_exit( weap->in_room , &pexit );
    
       if ( to_room == NULL ) to_room = pexit->to_room;
    
       // Scan for targets
       for ( rch = to_room->first_person; rch; rch = rnext )
       {
           rnext = rch->next_in_room;
           if ( is_spectator( rch ) || IN_VENT( rch ) ) continue;

           chance = number_range( -1, (int)((float)(rounds)/(float)(2)) );
           chance = URANGE( -1, chance, rounds );

           if ( rch->position == POS_PRONE ) chance = 0;

           if ( rch->pcdata && rch->hit > 0 )
           {
              int dodge, friend, primal;

              dodge = rch->pcdata->learned[gsn_dodge];
              friend = rch->pcdata->learned[gsn_friendly_fire];
              primal = rch->pcdata->learned[gsn_primal_instinct];

              if ( rch->race == ch->race ) chance -= ( 25 * friend );

              if ( rch->pcdata->prepared[gsn_dodge] < skill_table[gsn_dodge]->reset ) dodge = 0;
              if ( rch->pcdata->prepared[gsn_primal_instinct] < skill_table[gsn_primal_instinct]->reset ) primal = 0;

              dodge *= 10;
              primal *= 25;

              if ( number_percent() < dodge )
              {
                   ch_printf( rch, "&w&C(Dodge) You successfully dodged an attack.\n\r" );
                   rch->pcdata->prepared[gsn_dodge] = 0;
                   chance = 0;
              }
              if ( number_percent() < primal )
              {
                   ch_printf( rch, "&w&C(Primal Instinct) You successfully dodged an attack.\n\r" );
                   rch->pcdata->prepared[gsn_primal_instinct] = 0;
                   chance = 0;
              }
           }

           if ( chance > 0 )
           {
              hits++;

              dam = ( chance * ammo->value[1] );
              rounds -= chance;
              ch_printf( rch, "&rIncoming rounds from %s hit you! &z(&W%d round%s&z)\n\r", rev_exit( dir ), rounds, (rounds > 1) ? "s" : "" );

              if ( ( dam = cdamage( ch, rch, dam, TRUE ) ) >-1 )
              {
                 damage( ch, rch, dam, TYPE_GENERIC + ammo->value[0] );
              }
           }
           else
           {
              misses++;
              ch_printf( rch, "&rIncoming rounds from %s barely miss you!\n\r", rev_exit( dir ) );
           }
       }

       if ( rounds <= 0 )
       {
           if ( ammo->value[2] <= 0 )
           {
              ammo->timer = 2;
              obj_to_room( ammo, weap->in_room );
              weap->ammo = NULL;

              sprintf( buf, "%s ejects a spent ammunition drum.\n\r", weap->short_descr );
              echo_to_room( AT_CYAN, weap->in_room, buf );
           }
          
           return;
       }

       if ( ( pexit = get_exit( to_room, dir ) ) == NULL ) break;
   }

   if ( ammo->value[2] <= 0 )
   {
      ammo->timer = 2;
      obj_to_room( ammo, weap->in_room );
      weap->ammo = NULL;

      sprintf( buf, "%s ejects a spent ammunition drum.\n\r", weap->short_descr );
      echo_to_room( AT_CYAN, weap->in_room, buf );
   }

   return;
}

bool reload_sentry( CHAR_DATA *ch, char * arg1, char * arg2 )
{
    OBJ_DATA * obj;
    OBJ_DATA * ammo;
    
    if ( ( obj = get_obj_list( ch, arg1, ch->in_room->first_content ) ) == NULL )
        return FALSE;

    if ( get_sentry( obj ) == NULL ) return FALSE;

    if ( ( ammo = get_obj_carry( ch, arg2 ) ) == NULL )
    {
        send_to_char( "&RYou cannot find that ammo anywhere on you.\n\r", ch );
        return TRUE;
    }

    if ( ammo->item_type != ITEM_AMMO )
    {
        send_to_char( "&RLets assume the only thing your going to be loading is AMMUNITION.\n\r", ch );
        return TRUE;
    }

    if ( ammo->value[3] != obj->value[0] && ammo->value[4] != obj->value[0] && ammo->value[3] != obj->value[1] && ammo->value[5] != obj->value[0] )
    {
       send_to_char( "&RYou can't load that ammo into that turret!\n\r", ch );
       return TRUE;
    }

    /* Remove the existing clip */
    if ( obj->ammo )
    {
        ch_printf( ch, "You remove a %s from a %s.\n\r", obj->ammo->short_descr, obj->short_descr );
        act( AT_PLAIN, "$n removes $p from a weapon.", ch, obj->ammo, NULL, TO_ROOM );        

        obj_to_char( obj->ammo, ch );
        obj->ammo = NULL;
    }

    separate_obj(ammo);
    obj_from_char(ammo);

    ch_printf( ch, "You load a %s into a %s.\n\r", ammo->short_descr, obj->short_descr );
    act( AT_CYAN, "$n reloads a $p.", ch, obj, NULL, TO_ROOM );

    obj->ammo = ammo;

    WAIT_STATE( ch, 4 );

    return TRUE;
}

void do_cloak( CHAR_DATA * ch, char * argument )
{
   if ( !ch ) return;

   if ( ch->race != RACE_PREDATOR )
   {
       do_nothing( ch, "" );
       return;
   }

   if ( IS_AFFECTED( ch, AFF_CLOAK ) )
   {
       xREMOVE_BIT(ch->affected_by, AFF_CLOAK);
       act( AT_ACTION, "$n deactivates his cloaking.",  ch, NULL, NULL, TO_ROOM );
       act( AT_ACTION, "You deactivate your cloaking.", ch, NULL, NULL, TO_CHAR );
   }
   else
   {
       if ( ch->field <= 5 )
       {
            send_to_char("&RYou need more field charge to cloak!\n\r", ch );
            return;
       }

       xSET_BIT( ch->affected_by, AFF_CLOAK );
       act( AT_ACTION, "$n activates his cloaking.", ch, NULL, NULL, TO_ROOM );
       act( AT_ACTION, "You activate your cloaking.", ch, NULL, NULL, TO_CHAR );

       ch->field -= 5;
   }

   return;
}
