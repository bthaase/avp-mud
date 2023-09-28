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
*                       Cybernetics Module for SWR                         *
*                         Designed by CYBER_Aeon                           *
****************************************************************************/
/*
 * Cybernetic Equipment
 * WARNING: CODE HAS BEEN ADDED TO:     mud.h
 *                                      save.c
 *                                      build.c
 *                                      tables.c
 *                                      act_comm.c
 *                                      update.c
 * Instructions:
 *   Seller must have the spec SPEC_CYBERSTORE
 *   The command CYBER Works as:
 *       CYBER INSTALL <item>  - Installs a item your carrying
 *                               Costs 50000 to have a item custom installed.
 *       CYBER                 - Lists the items for sale
 *       CYBER         <item>  - Buys and installs a item
 *  ITEM_CYBER:
 *     v0: Cybernetic Type (See CYBER Flags)
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"


void do_cyber (CHAR_DATA * ch, char *argument)
{
    CHAR_DATA   * mob;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    /* char buf[MAX_STRING_LENGTH]; */
    int cost;

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );

    /* Check for Surgeon    */
    for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
    {
       if ( IS_NPC( mob ) && xIS_SET( mob->act, ACT_SURGEON ) )
           break;
    }

    if ( mob == NULL )
    {
       if ( arg2[0] == '\0' )
       {
           cyber_bit( ch );
           return;
       }
       else
       {
           send_to_char( "&RYou'll need to find a surgeon to do that!&w\n\r", ch );
           return;
       }
    }

    if ( ( ch->hit - ( ch->max_hit / 4 ) ) <= 0 )
    {
       do_say( mob, "I can't do that... The stress would kill you!" );
       return;
    }

    /* If no args passed, just show the list    */
    if ( arg[0] == '\0' )
    {
       act( AT_LBLUE,"$N says 'I only have these parts in stock:'",ch,NULL,mob, TO_CHAR);
       do_emote( mob, "hands you a small pamphlet with a listing." );
       send_to_char("&w&C  muscle     &z: Argumented Muscles           &B30000 &zCred(s).\n\r", ch);
       send_to_char("&w&C  bioplug    &z: Biothermal Recharge Port     &B30000 &zCred(s).\n\r", ch);
       send_to_char("&w&C  recover    &z: Advanced Recovery System     &B35000 &zCred(s).\n\r", ch);
       send_to_char("&w&C  vision     &z: Infrared Vision System       &B40000 &zCred(s).\n\r", ch);
       send_to_char("&w&C  immune     &z: Advance Immune System        &B50000 &zCred(s).\n\r", ch);
       send_to_char("&w&C  oxygen     &z: Oxygen Extraction System     &B50000 &zCred(s).\n\r", ch);
       send_to_char("&WUSE CYBER <ITEM> TO PURCHASE A ENHANCEMENT.\n\r", ch);
       send_to_char("&WUSE CYBER INSTALL <ITEM> TO INSTALL A CUSTOM ITEM.\n\r", ch);
       return;
    }

    if ( !str_cmp(arg, "install") )
    {
        c_install( ch, mob, arg2 );
        return;
    }
   /* else if ( !str_cmp(arg, "comlink") )
    {
       cost = 20000;
       if ( !c_afford( ch, mob, cost ) ) return;
       if ( c_gotpart( ch, mob, CYBER_COMLINK ) ) return;
       xSET_BIT( ch->cyber_flags, CYBER_COMLINK );
    } */
    else if ( !str_cmp(arg, "muscle") )
    {
       cost = 30000;
       if ( !c_afford( ch, mob, cost ) ) return;
       if ( c_gotpart( ch, mob, CYBER_MUSCLE ) ) return;
       xSET_BIT( ch->cyber_flags, CYBER_MUSCLE );
       ch->perm_str += 3;
    }
    else if ( !str_cmp(arg, "vision") )
    {
       cost = 40000;
       if ( !c_afford( ch, mob, cost ) ) return;
       if ( c_gotpart( ch, mob, CYBER_VISION ) ) return;
       xSET_BIT( ch->cyber_flags, CYBER_VISION );
    }
    else if ( !str_cmp(arg, "bioplug") )
    {
       cost = 30000;
       if ( !c_afford( ch, mob, cost ) ) return;
       if ( c_gotpart( ch, mob, CYBER_BIOPLUG ) ) return;
       xSET_BIT( ch->cyber_flags, CYBER_BIOPLUG );
    }
    else if ( !str_cmp(arg, "recover") )
    {
       cost = 35000;
       if ( !c_afford( ch, mob, cost ) ) return;
       if ( c_gotpart( ch, mob, CYBER_RECOVER ) ) return;
       xSET_BIT( ch->cyber_flags, CYBER_RECOVER );
    }
    else if ( !str_cmp(arg, "oxygen") )
    {
       cost = 50000;
       if ( !c_afford( ch, mob, cost ) ) return;
       if ( c_gotpart( ch, mob, CYBER_OXYGEN ) ) return;
       xSET_BIT( ch->cyber_flags, CYBER_OXYGEN );
    }
    else if ( !str_cmp(arg, "immune") )
    {
       /*
        *  Immune system removes the following affects:
        *  POISON (40%), PARALYSIS (30%), ADDICITONS (-1 rnd)
        */  
       cost = 50000;
       if ( !c_afford( ch, mob, cost ) ) return;
       if ( c_gotpart( ch, mob, CYBER_IMMUNE ) ) return;
       xSET_BIT( ch->cyber_flags, CYBER_IMMUNE );
    }
    else
    {
       act( AT_LBLUE, "$N says 'Please choose a enhancement or move on.'",ch,NULL,mob, TO_CHAR);
       return;
    }

    WAIT_STATE( ch, PULSE_VIOLENCE );

    ch->gold -= cost;
    ch->hit -= ( ch->max_hit / 4 );
    do_say( mob, "The surgery will take a lot of a person..." );
    do_say( mob, "I recommend getting some rest now!" );
    do_emote( mob, "chuckles politely." );
    // act( AT_LBLUE,"$n says 'There we go, Better than the original!'", mob, NULL, NULL, TO_ROOM);
    return;
}

void c_update( CHAR_DATA *ch )
{
   OBJ_DATA  * wield;
   OBJ_DATA  * obj=NULL;

   /*
    *   Update BIOPLUG Enhancement
    *   Recharges ONLY Lightsabers, Vibro blades, and Force pikes
    *   Uses *2* mv per *1* Charges
    */
   if ( ch->move >= 50 && xIS_SET(ch->cyber_flags, CYBER_BIOPLUG ) )
   {
      wield = get_eq_char( ch, WEAR_WIELD );
      if (wield)
        obj = get_eq_char( ch, WEAR_DUAL_WIELD );
    
     if ( wield != NULL )
      if ( wield->item_type == ITEM_WEAPON && wield->value[4] < wield->value[5] )
       if (wield->value[3] == WEAPON_LIGHTSABER || wield->value[3] == WEAPON_VIBRO_BLADE || wield->value[3] == WEAPON_FORCE_PIKE)
       {
         if ( wield->value[4]+1 < wield->value[5] ) wield->value[4] += 1;
         ch->move -= 2;
       }
     if ( obj != NULL )
      if ( obj->item_type == ITEM_WEAPON && obj->value[4] < obj->value[5] )
       if (obj->value[3] == WEAPON_LIGHTSABER || obj->value[3] == WEAPON_VIBRO_BLADE || obj->value[3] == WEAPON_FORCE_PIKE)
       {
         if ( obj->value[4]+1 < obj->value[5] ) obj->value[4] += 1;
         ch->move -= 2;
       }
   }

   /*
    * Infrared Vision Enhancement.
    *  -If not currently affected by INFRARED, Cast it on the player.
    */
   if ( xIS_SET( ch->cyber_flags, CYBER_VISION ) )
   {
      if ( IS_AFFECTED(ch, AFF_INFRARED ) )
      {
         /* Cures blinding */
      }
      else  /* Give the PC Infrared vision */
      {
         send_to_char("&YYour vision wavers as the infrared optics kick in...\n\r", ch);
         xSET_BIT( ch->affected_by, AFF_INFRARED );
      }
   }

   /*
    * Oxygen Extraction System.
    *  -If not currently affected by AQUABREATH, Cast it on the player.
    */
   if ( xIS_SET( ch->cyber_flags, CYBER_OXYGEN ) )
   {
      if ( !IS_AFFECTED(ch, AFF_AQUA_BREATH ) )
      {
         send_to_char("&YYour breathing slows as the oxygen system kicks in...\n\r", ch);
         xSET_BIT( ch->affected_by, AFF_AQUA_BREATH );
      }
   }

   /*
    *   More to come.....
    */

  return;
}

void c_install( CHAR_DATA *ch, CHAR_DATA *mob, char *arg )
{
  bug ("[ALERT]: CYBER INSTALL <object> is INCOMPLEATE!" );
  return;
}

bool c_afford( CHAR_DATA *ch, CHAR_DATA *mob, int cost )
{
  if ( ch->gold < cost )
  {
     act( AT_LBLUE,"$N says 'You do not have enough credits for my services.'",ch,NULL,mob, TO_CHAR);
     return FALSE;
  }
  return TRUE;
}

bool c_gotpart( CHAR_DATA *ch, CHAR_DATA *mob, int bit )
{
  if ( xIS_SET( ch->cyber_flags, bit ) )
  {
     act( AT_LBLUE,"$N says 'You already have that part installed.'",ch,NULL,mob, TO_CHAR);
     return TRUE;
  }
  return FALSE;
}

void cyber_bit( CHAR_DATA *ch )
{
  send_to_char("\n\r&z| Cybernetic Parts:\n\r", ch);
  if xIS_EMPTY( ch->cyber_flags )
     send_to_char("|   &R(None)\n\r", ch);
  if xIS_SET( ch->cyber_flags, CYBER_MUSCLE )
     send_to_char("&z|   &BA&Crgumented Muscles\n\r", ch);
  if xIS_SET( ch->cyber_flags, CYBER_BIOPLUG )
     send_to_char("&z|   &BB&Ciothermal Recharge Port\n\r", ch);
  if xIS_SET( ch->cyber_flags, CYBER_RECOVER )
     send_to_char("&z|   &BA&Cdvanced Recovery System\n\r", ch);
  if xIS_SET( ch->cyber_flags, CYBER_IMMUNE )
     send_to_char("&z|   &BA&Cdvanced Immune System\n\r", ch);
  if xIS_SET( ch->cyber_flags, CYBER_VISION )
     send_to_char("&z|   &BI&Cnfrared Vision System\n\r", ch);
  if xIS_SET( ch->cyber_flags, CYBER_OXYGEN )
     send_to_char("&z|   &BO&Cxygen Extraction System\n\r", ch);
  if xIS_SET( ch->cyber_flags, CYBER_SPECTRAL )
     xREMOVE_BIT( ch->cyber_flags, CYBER_SPECTRAL );
     /* send_to_char("&z|   &BS&Cpectral Analyzer System\n\r", ch); */
  send_to_char("&z\\--------------------------------", ch );
  return;
}
