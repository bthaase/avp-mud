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
*			    Battle & death module			   *
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "mud.h"

#ifdef _WIN32
#include <sys/dirent.h>
#else
#include <sys/dir.h>
#endif

#define BFS_MARK         BV01

extern char		lastplayercmd[MAX_INPUT_LENGTH];
extern CHAR_DATA *	gch_prev;

/*
 * Local functions.
 */
void	death_cry	args( ( CHAR_DATA *ch ) );
int     xp_compute  args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
int     align_compute   args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
ch_ret	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int     obj_hitroll args( ( OBJ_DATA *obj ) );
bool    get_cover( CHAR_DATA *ch );
bool	dual_flip = FALSE;
void    add_xname( char *arg);
char *  current_weapon( CHAR_DATA * ch );
bool    check_rescue( CHAR_DATA * ch );
bool    is_enemy( CHAR_DATA * ch, CHAR_DATA * victim );
int     cdamage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, bool silent );
bool    is_ranged( int type );
bool    mob_reload( CHAR_DATA * ch, OBJ_DATA * weapon );


/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hunting || ch->hunting->who != victim )
      return FALSE;
    
    return TRUE;    
}

bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hating || ch->hating->who != victim )
      return FALSE;
    
    return TRUE;    
}

bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->fearing || ch->fearing->who != victim )
      return FALSE;
    
    return TRUE;    
}

void stop_hunting( CHAR_DATA *ch )
{
    if ( ch->hunting )
    {
	STRFREE( ch->hunting->name );
	DISPOSE( ch->hunting );
	ch->hunting = NULL;
    }
    return;
}

void stop_hating( CHAR_DATA *ch )
{
    if ( ch->hating )
    {
	STRFREE( ch->hating->name );
	DISPOSE( ch->hating );
	ch->hating = NULL;
    }
    return;
}

void stop_fearing( CHAR_DATA *ch )
{
    if ( ch->fearing )
    {
	STRFREE( ch->fearing->name );
	DISPOSE( ch->fearing );
	ch->fearing = NULL;
    }
    return;
}

void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hunting )
      stop_hunting( ch );

    CREATE( ch->hunting, HHF_DATA, 1 );
    ch->hunting->name = QUICKLINK( victim->name );
    ch->hunting->who  = victim;
    return;
}

void start_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->race == victim->race ) return;

    if ( ch->hating )
      stop_hating( ch );

    CREATE( ch->hating, HHF_DATA, 1 );
    ch->hating->name = QUICKLINK( victim->name );
    ch->hating->who  = victim;
    return;
}

void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fearing )
      stop_fearing( ch );

    CREATE( ch->fearing, HHF_DATA, 1 );
    ch->fearing->name = QUICKLINK( victim->name );
    ch->fearing->who  = victim;
    return;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    CHAR_DATA *lst_ch;
    CHAR_DATA *victim;
    CHAR_DATA *rch, *rch_next;
    AFFECT_DATA *paf, *paf_next;
    TIMER	*timer, *timer_next;
    ch_ret     retcode;
    SKILLTYPE	*skill;
    int        i;
    int        mA, mB;

    lst_ch = NULL;
    for ( ch = last_char; ch; lst_ch = ch, ch = gch_prev )
    {
        set_cur_char( ch );

        if ( ch == first_char && ch->prev )
        {
           bug( "ERROR: first_char->prev != NULL, fixing...", 0 );
           ch->prev = NULL;
        }

        gch_prev    = ch->prev;

        if ( gch_prev && gch_prev->next != ch )
        {
            sprintf( buf, "FATAL: violence_update: %s->prev->next doesn't point to ch.", ch->name );
            bug( buf, 0 );      
            bug( "Short-cutting here", 0 );
            ch->prev = NULL;
            gch_prev = NULL;
        }

        /*
         * Alien Resin Replenish
         */
        if ( xIS_SET( ch->in_room->room_flags, ROOM_HIVED ) )
         if ( ch->race == RACE_ALIEN ) ch->resin = UMIN( get_max_resin(ch), ch->resin + 1 );

        /*
         * Refresh skill table for players
         */
        if ( !IS_NPC( ch ) && ch->pcdata )
        {
            int sboost = 1;

            if ( ch->race == RACE_ALIEN && xIS_SET( ch->in_room->room_flags, ROOM_HIVED ) )
              sboost = 2;

            for ( i = 0; i < top_sn; i++ )
             if ( ch->pcdata->learned[i] > 0 )
               if ( ch->pcdata->prepared[i] < skill_table[i]->reset )
               {
                 ch->pcdata->prepared[i] += sboost;
                 if ( skill_table[i]->reset > 5 )
                  if ( ch->pcdata->prepared[i] >= skill_table[i]->reset )
                    if ( xIS_SET( ch->pcdata->flags, PCFLAG_SHOWRESET ) )
                     ch_printf( ch, "&w&GSkill Reset: %s is now ready.\n\r", capitalize(skill_table[i]->name) );
               }
        }

        /*
	 * See if we got a pointer to someone who recently died...
	 * if so, either the pointer is bad... or it's a player who
	 * "died", and is back at the healer...
	 * Since he/she's in the char_list, it's likely to be the later...
	 * and should not already be in another fight already
	 */
	if ( char_died(ch) )
	    continue;

	/*
	 * See if we got a pointer to some bad looking data...
	 */
	if ( !ch->in_room || !ch->name )
	{
	    log_string( "violence_update: bad ch record!  (Shortcutting.)" );
	    sprintf( buf, "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d",
	    	(int) ch, (int) ch->in_room, (int) ch->prev, (int) ch->next );
	    log_string( buf );
	    log_string( lastplayercmd );
	    if ( lst_ch )
	      sprintf( buf, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d",
	      		(int) lst_ch, (int) lst_ch->prev, (int) lst_ch->next );
	    else
	      strcpy( buf, "lst_ch: NULL" );
	    log_string( buf );
	    gch_prev = NULL;
	    continue;
	}

       /*
        * Update the AP counter for this character
        */
        mA = get_max_ap( ch );
        mB = get_max_mp( ch );

        if ( ch->ap >= mA && ch->ap <= 7 ) ch->ap++;

        if ( ch->ap < mA || ch->mp < mB )
        {
            if ( ch->ap < mA ) ch->ap++;
            if ( ch->mp < mB ) ch->mp = URANGE( 0, ch->mp + 2, mB );

            // Line-break alarm to redisplay prompt
            if ( !IS_NPC( ch ) ) send_to_char( " ", ch );
        }
        
	for ( timer = ch->first_timer; timer; timer = timer_next )
	{
	    timer_next = timer->next;
	    if ( --timer->count <= 0 )
	    {
		if ( timer->type == TIMER_DO_FUN )
		{
		    int tempsub;

		    tempsub = ch->substate;
		    ch->substate = timer->value;
		    (timer->do_fun)( ch, "" );
		    if ( char_died(ch) )
		      break;
		    ch->substate = tempsub;
		}
		extract_timer( ch, timer );
	    }
	}

	if ( char_died(ch) )
	  continue;

        /*
	 * We need spells that have shorter durations than an hour.
	 * So a melee round sounds good to me... -Thoric
	 */
	for ( paf = ch->first_affect; paf; paf = paf_next )
	{
            paf_next  = paf->next;
            if ( paf->duration > 0 )
            {
                 paf->duration--;

                 if ( xIS_SET( paf->bitvector, AFF_NAPALM ) )
                 {
                     if ( ch->hit >= 0 )
                     {
                        act( AT_RED, "$n screams in pain and rolls on the ground.", ch, NULL, NULL, TO_ROOM );
                        act( AT_RED, "You scream in pain and roll on the ground.", ch, NULL, NULL, TO_CHAR );
                        ch->hit -= 10;
                        if ( IS_NPC( ch ) ) ch->hit = UMAX( 1, ch->hit );
                        update_pos( ch );
                        // damage( ch, ch, 10, TYPE_GENERIC + RIS_FIRE );
                     }
                     else
                     {
                        if ( paf->duration > 0 ) paf->duration = 0;
                     }
                 }

                 if ( paf->type == gsn_confuse && ch->in_room )
                 {
                    int x, y, z;
                    int range = 3;

                    if ( !IS_NPC( ch ) ) range = 2 + ch->pcdata->learned[gsn_confuse];

                    x = (ch->in_room->x + (number_range( range, range * 2 )-range) );
                    y = (ch->in_room->y + (number_range( range, range * 2 )-range) );
                    z = ch->in_room->z;

                    motion_ping(x, y, z, ch->in_room->area, NULL);
                 }

                 if ( xIS_SET( paf->bitvector, AFF_SHORTAGE ) )
                 {
                    if ( !IS_AFFECTED(ch, AFF_CLOAK) )
                    {
                        ch->field = URANGE( 0, ch->field - (paf->duration * 2), ch->max_field );
                        paf->duration = 0;
                    }
                    else
                    {
                        ch->field = URANGE( 0, ch->field - 3, ch->max_field );
                    }
                 }
            }
            else if ( paf->duration < 0 ) ;
            else
            {
                  if ( !paf_next || paf_next->type != paf->type || paf_next->duration > 0 )
		  {
		      skill = get_skilltype(paf->type);
		      if ( paf->type > 0 && skill && skill->msg_off )
		      {
                          set_char_color( AT_WEAROFF, ch );
                          send_to_char( skill->msg_off, ch );
                          send_to_char( "\n\r", ch );
		      }
		  }

                  if ( xIS_SET( paf->bitvector, AFF_SHORTAGE ) ) clear_shortage( ch );

                  affect_remove( ch, paf );
            }
	}
	
        /* Update the follower routine - Moved to char_check in update.c */
        // if ( ch->master != NULL ) follow_victim( ch );

        /*
         * Only mobs need continue in the loop, as all human attacks
         * must be launched via commands. Bye!
         */
        if ( IS_BOT( ch ) || !IS_NPC( ch ) || ch->ap < get_max_ap( ch ) ) continue;

        if ( ch->wait > 0 )
        {
           ch->wait--;
           continue;
        }

        retcode = rNONE;

        if ( check_rescue( ch ) ) continue;

	if ( !ch->in_room || ch->in_room == NULL ) continue;
        
	/*
         * First, check the players in here if any are 'hated'
         */	
	for ( rch = ch->in_room->first_person; rch; rch = rch_next )
	{
	    rch_next = rch->next_in_room;

            if ( is_hating( ch, rch ) && can_see( ch, rch ) && ch->vent == rch->vent && !is_spectator( rch ) && ch->ap >= get_max_ap(ch) )
	    {
                 retcode = multi_hit( ch, rch, TYPE_UNDEFINED );
                 ch->ap = 0;
                 break;
	    }
        }

        if ( rch == NULL )
        {
           /*
            * Otherwise, check for anything 'enemy', or diffrent race.
            */
            for ( rch = ch->in_room->first_person; rch; rch = rch_next )
            {                                     
                 rch_next = rch->next_in_room;

                 if ( !can_see( ch, rch ) || ch->vent != rch->vent || is_spectator( rch ) ) continue;
          
                 if ( is_enemy( ch, rch ) )
                 {
                     if ( ch->race == RACE_MARINE ) do_emote( ch, "screams 'Bloody hell!'" );
                     if ( ch->race == RACE_ALIEN ) do_emote( ch, "screechs in anger!" );
                     start_hating( ch, rch );
                     start_hunting( ch, rch );
                     // retcode = multi_hit( ch, rch, TYPE_UNDEFINED );
                     break;
                 }
            }
        }

        if ( char_died(ch) )
            continue;

        if ( retcode == rCHAR_DIED )
	    continue;
    }

    return;
}



/*
 * Do one group of attacks.
 */
ch_ret multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance;
    ch_ret  retcode;

    /* add timer if player is attacking another player */
    if ( !IS_NPC(ch) && !IS_NPC(victim) )
      add_timer( ch, TIMER_RECENTFIGHT, 20, NULL, 0 );

    if ( !IS_NPC(ch) && xIS_SET( ch->act, PLR_NICE ) && !IS_NPC( victim ) )
      return rNONE;

    /*
     * Character gets one attack here...
     */
    if ( (retcode = one_hit( ch, victim, dt )) != rNONE ) return retcode;

    /*
     * Dual wielding skills. Allows for the extra attack
     */
    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) && !char_died( victim ) )
    {
      if ( (retcode = one_hit( ch, victim, dt )) != rNONE ) return retcode;
    }

    /*
     * Double Strike
     */
    if ( !IS_NPC( ch ) && ch->race == RACE_PREDATOR && !char_died( victim ) && !IS_NPC( victim ) )
    {
      OBJ_DATA * objA = NULL;
      OBJ_DATA * objB = NULL;
      bool unarmed = FALSE;
      int strike;
      
      strike = ch->pcdata->learned[gsn_double_strike];

      if ( ch->pcdata->prepared[gsn_double_strike] < skill_table[gsn_double_strike]->reset ) strike = 0;

      if ( strike > 0 ) strike = (strike * 25) + 25;

      objA = get_eq_char( ch, WEAR_WIELD );
      objB = get_eq_char( ch, WEAR_DUAL_WIELD );

      if ( !objA && !objB ) unarmed = TRUE;
                
      if ( !objA || is_ranged(objA->value[0]) ) objA = NULL;
      if ( !objB || is_ranged(objB->value[0]) ) objB = NULL;
      
      if ( (unarmed || objA || objB) && number_percent() < strike )
      {
           ch_printf( ch, "&w&C(Double Strike) You get another attack round!\n\r" );
           ch->pcdata->prepared[gsn_double_strike] = 0;

           /* Bonus strike with unarmed */
           if ( unarmed )
           {
              if ( (retcode = one_hit( ch, victim, dt )) != rNONE ) return retcode;
           }

           /* Bonus strike with primary */
           if ( objA )
           {
              if ( (retcode = one_hit( ch, victim, dt )) != rNONE ) return retcode;
           }

           /* Bonus strike with dual wielded */
           if ( objB && !char_died( victim ) )
           {
              if ( (retcode = one_hit( ch, victim, dt )) != rNONE ) return retcode;
           }

           return retcode;
      }
    }
 
    return retcode;
}


/*
 * Hit one guy once.
 */
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int dam;
    int diceroll;
    ch_ret retcode;
    int chance;
    int rounds=0, rhit=0;
    bool melee = FALSE;
    bool flame = FALSE;  // Using a flamethrower?
                
    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
       return rVICT_DIED;

    /*
     * Figure out the weapon doing the damage			-Thoric
     */
    if ( (wield = get_eq_char( ch, WEAR_DUAL_WIELD )) != NULL )
    {
       if ( dual_flip == FALSE )
       {
          dual_flip = TRUE;
          wield = get_eq_char( ch, WEAR_WIELD );
       }
       else
          dual_flip = FALSE;
    }
    else
      wield = get_eq_char( ch, WEAR_WIELD );

    /*
     * Add in the weapon-damage type
     */
    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
        melee = TRUE;
        if ( wield )
        {
           if ( wield->item_type == ITEM_WEAPON ) dt += wield->value[0];
        }
        else
        {
           /* Predator uses Wrist blades, Aliens use claws */
           if ( ch->race != RACE_MARINE ) dt = TYPE_GENERIC + RIS_PIERCE;
           if ( ch->race == RACE_MARINE ) dt = TYPE_GENERIC + RIS_IMPACT;
        }
    }

    /*
     * Calculate hit chance (0% = 0, 100% = 100)
     */
    chance = 80;

    if ( wield )
    {
       if ( wield->value[0] == WEAPON_FLAMETHROWER ) flame = TRUE;

       if ( wield->value[0] == WEAPON_ROCKETFIRED )
       {
           send_to_char( "&YThis is not a close combat weapon!\n\r", ch );
           return rNONE;
       }
       else if ( is_ranged( wield->value[0] ) )
       {
          if ( xIS_SET( wield->extra_flags, ITEM_RANGEDONLY ) )
          {
              send_to_char( "&YThis is a ranged-only weapon. Use SNIPE or SPRAY.\n\r", ch );
              return rNONE;
          }
          if ( wield->value[0] == WEAPON_ERANGED )
          {
              if ( ch->field < wield->value[1] )
              {
                 send_to_char("&RYour too low on field charge. Recharge first.\n\r", ch );
                 return rNONE;
              }
          }
          else if ( !wield->ammo )
          {
              send_to_char( "&YMaybe you should load your weapon first!\n\r", ch );
              if ( !mob_reload( ch, wield ) ) return rNONE;
          }
          if ( victim->pcdata && victim->hit > 0 )
          {
              int dodge, primal;

              dodge = victim->pcdata->learned[gsn_dodge];
              primal = victim->pcdata->learned[gsn_primal_instinct];

              if ( victim->pcdata->prepared[gsn_dodge] < skill_table[gsn_dodge]->reset ) dodge = 0;
              if ( victim->pcdata->prepared[gsn_primal_instinct] < skill_table[gsn_primal_instinct]->reset ) primal = 0;

              dodge *= 10;
              primal *= 25;

              if ( number_percent() < dodge )
              {
                   ch_printf( victim, "&w&C(Dodge) You successfully dodged an attack.\n\r" );
                   victim->pcdata->prepared[gsn_dodge] = 0;
                   chance = -100;
              }
              if ( number_percent() < primal )
              {
                   ch_printf( victim, "&w&C(Primal Instinct) You successfully dodged an attack.\n\r" );
                   victim->pcdata->prepared[gsn_primal_instinct] = 0;
                   chance = -100;
              }
          }
       }
       else
       {
          /* Add the weapons reach to the accuracy. 5% for each 'unit' */
          chance = 75;
          chance += ( wield->value[3] * 5 );
          melee = TRUE;
       }
       if ( wield->ammo != NULL )
       {
          if ( wield->ammo->value[2] <= 0 )
          {
              send_to_char( "&YMaybe you should reload your weapon first!\n\r", ch );
              if ( !mob_reload( ch, wield ) ) return rNONE;
          }
       }

       chance += weapon_accuracy[ (wield->value[0]) ];

       if ( wield->weapon_mode == MODE_SINGLE ) chance += 20;
       if ( wield->weapon_mode == MODE_BURST ) chance += 10;
       if ( wield->weapon_mode == MODE_SEMIAUTO ) chance += 30;
       if ( wield->weapon_mode == MODE_AUTOMATIC ) chance += 20;

       if ( wield->value[0] == WEAPON_PISTOL && ch->pcdata )
       {
          chance += ( 10 * ch->pcdata->learned[gsn_handweapons] );
       }
    }
    else
    {
       chance += ( ch->race == RACE_ALIEN ) ? 0 : 25;
    }

    /* Factor in the Low Stamina Penalty */
    if ( chance > -100 ) chance = stamina_penalty( ch, chance );

    /* Alien accuracy bonus */
    if ( ch->race == RACE_ALIEN && chance > -100 ) chance += 20;

    /* Factor in character skills */
    if ( chance > -100 ) chance = char_acc_modify( victim, chance );

    /* Factor in the weapon's accuracy attachment */
    if ( wield != NULL && wield->attach )
    {
       if ( wield->attach->value[0] == 2 )
       {
          if ( ch->in_room == victim->in_room )
            chance += wield->attach->value[2];
          else
            chance += wield->attach->value[3];
       }
    }

    /* Reset the AP gauge */
    if ( wield )
    {
       if ( wield->weapon_mode == MODE_SEMIAUTO ) ch->ap = get_max_ap(ch) - 1;
       if ( wield->weapon_mode != MODE_SEMIAUTO ) ch->ap = 0;
    }
    else
    {
       ch->ap = 0;
    }

    if ( !can_see( ch, victim ) ) chance -= 25;
    if ( !can_see( victim, ch ) ) chance += 50;

    if ( !IS_AWAKE ( victim ) ) chance += 50;

    if ( victim->hit <= 0 ) chance += 75;

    if ( chance >= 0 ) chance = URANGE( 10, chance, 100 );

    /*
     * The moment of excitement!
     */
    diceroll = number_range( 1, 100 );

    /*
     * Calc damage.
     */
    if ( !wield )
    {
       char bufA[MAX_STRING_LENGTH], bufB[MAX_STRING_LENGTH], bufC[MAX_STRING_LENGTH];

       switch ( ch->race )
       {
           default:
           case RACE_MARINE:
             dam = number_range( 5, 10 ) + ( get_curr_str( ch ) );
             sprintf( bufA, "&YYou swing at %s, hoping to land a blow!\n\r", PERS( victim, ch ) );
             sprintf( bufB, "&Y$n swings wildly at $N." );
             sprintf( bufC, "&Y$n takes a swing at you!" );
             break;
           case RACE_PREDATOR:  // Wristblade
             dam = ( number_range( 30, 40 ) ) + ( get_curr_str( ch ) * 2 );
             sprintf( bufA, "&YYou slash at %s, hoping to land a blow!\n\r", PERS( victim, ch ) );
             sprintf( bufB, "&Y$n slashes wildly at $N." );
             sprintf( bufC, "&Y$n takes a wild slash at you!" );
             break;
           case RACE_ALIEN:
             dam = number_range( 80, 90 ) + ( get_curr_str( ch ) * 2 );
             if ( ch->swarm > 0 ) dam += (int)((float)((float)(dam) / (float)(100)) * (float)(ch->swarm));
             sprintf( bufA, "&YYou claw wildly at %s, hoping to land a hit!\n\r", PERS( victim, ch ) );
             sprintf( bufB, "&Y$n claws wildly at $N." );
             sprintf( bufC, "&Y$n claws wildly at you!" );
             break;
       }
       ch_printf( ch, bufA );
       act( AT_YELLOW, bufB, ch, NULL, victim, TO_NOTVICT );
       act( AT_YELLOW, bufC, ch, NULL, victim, TO_VICT    );
    }
    else
    {
       char bufA[MAX_STRING_LENGTH], bufB[MAX_STRING_LENGTH];
                 
       if ( wield->ammo )
       {
           dam = wield->ammo->value[1];
 
           if ( wield->weapon_mode == MODE_SINGLE )
           {
                rounds = 1;
                wield->ammo->value[2] = UMAX( 0, wield->ammo->value[2] - rounds );
           }
           else if ( wield->weapon_mode == MODE_SEMIAUTO )
           {
                rounds = 1;
                wield->ammo->value[2] = UMAX( 0, wield->ammo->value[2] - rounds );
           }
           else if ( wield->weapon_mode == MODE_BURST )
           {                
                if ( wield->value[3] > wield->ammo->value[2] ) rounds = wield->ammo->value[2];
                else rounds = wield->value[3];
                rhit = URANGE( 1, number_range( 1, rounds+1 ), rounds );
                dam *= ( rhit );
                wield->ammo->value[2] = UMAX( 0, wield->ammo->value[2] - rounds );
           }
           else if ( wield->weapon_mode == MODE_AUTOMATIC )
           {                
                if ( wield->value[4] > wield->ammo->value[2] ) rounds = wield->ammo->value[2];
                else rounds = wield->value[4];                
                rhit = URANGE( 1, number_range( 1, rounds+1 ), rounds );
                dam *= ( rhit );
                wield->ammo->value[2] = UMAX( 0, wield->ammo->value[2] - rounds );
           }

           if ( wield->attach )
             if ( wield->attach->value[0] == 4 )
               dam = (int)((float)((float)(dam)/(float)(100)) * (float)(100 + wield->attach->value[2]));
       }
       else if ( wield->value[0] == WEAPON_ERANGED )
       {
           dam = wield->value[4];

           if ( wield->weapon_mode == MODE_SINGLE )
           {
                rounds = 1;
                ch->field = UMAX( 0, ch->field - wield->value[1] );
           }
           else if ( wield->weapon_mode == MODE_SEMIAUTO )
           {
                rounds = 1;
                ch->field = UMAX( 0, ch->field - wield->value[1] );
           }
           else if ( wield->weapon_mode == MODE_BURST )
           {
                rounds = URANGE( 0, wield->value[3], (int)( ch->field / wield->value[1] ) );
                ch->field = UMAX( 0, ch->field - (wield->value[1] * rounds) );

                dam *= rounds;

                if ( ch->field <= wield->value[1] )
                  ch_printf( ch, "&w&R(Warning) Field Charge is depleted. You must recharge.\n\r" );
           }
           else
           {
                rounds = 0;
                rhit = 0;
                dam = 0;
           }

           if ( wield->attach )
             if ( wield->attach->value[0] == 4 )
               dam = (int)((float)((float)(dam)/(float)(100)) * (float)(100 + wield->attach->value[2]));
       }
       else
       {
           dam = number_range( wield->value[1], wield->value[2] );

           if ( wield->attach )
             if ( wield->attach->value[0] == 4 )
               dam = (int)((float)((float)(dam)/(float)(100)) * (float)(100 + wield->attach->value[2]));
       }

       /*
        * Ready? Sound the firing echo!
        */
       switch( wield->value[0] )
       {
          default:
          case WEAPON_NONE:
          case WEAPON_KNIFE:
          case WEAPON_BLADE:
             sprintf( bufA, "&YYou lunge at your target, trying to strike a blow.\n\r" );
             sprintf( bufB, "&Y$n lunges with the deadly weapon." );
             break;
          case WEAPON_ERANGED:
          case WEAPON_RANGED:
             sprintf( bufA, "&YYou carefully take aim and open fire. &z(&WFired %d rounds&z)\n\r", rounds );
             sprintf( bufB, "&Y$n opens fire with a %s.", wield->short_descr );
             break;         
          case WEAPON_SPEAR:
             sprintf( bufA, "&YYou lunge at your target, swinging the spear.\n\r" );
             sprintf( bufB, "&Y$n lunges with the deadly spear." );
             break;
          case WEAPON_PISTOL:
             sprintf( bufA, "&YThe weapon sounds a loud report as it fires! &z(&WFired %d rounds&z)\n\r", rounds );
             sprintf( bufB, "&Y$n opens fire with a %s.", wield->short_descr );

             if ( ch->pcdata )
             {
               int perc = 0;
               perc = 100 + ( 10 * ch->pcdata->learned[gsn_handweapons] );
               dam = (int)((float)(dam) * (float)((float)(perc)/(float)(100)));
             }

             break;
          case WEAPON_FLAMETHROWER:
             dam = number_range( dam / 2, dam );
             sprintf( bufA, "&RYou fire a jet of solid flame at your target.\n\r", rounds );
             sprintf( bufB, "&Y$n opens fire with a %s.", wield->short_descr );
             break;
          case WEAPON_SHOTGUN:
             /* Shotgun damage varies */
             dam = number_range( dam - 20, dam + 20 );
          case WEAPON_RIFLE:
          case WEAPON_AUTOMATIC:
          case WEAPON_ROCKETFIRED:
             sprintf( bufA, "&YThe weapon roars loudly as it opens fire! &z(&WFired %d rounds&z)\n\r", rounds );
             sprintf( bufB, "&Y$n opens fire with a %s.", wield->short_descr );
             break;
       }

       ch_printf( ch, bufA );
       act( AT_YELLOW, bufB, ch, wield, NULL, TO_ROOM );

       weapon_echo( ch, wield );
    }

    if ( diceroll == 1 || diceroll > chance )
    {
        /* DAMN, Missed! */
	damage( ch, victim, 0, dt );
        if ( wield )
        {
            if ( wield->ammo )
            {
               if ( wield->weapon_mode == MODE_AUTOMATIC ) miss_message( ch, victim, -2, wield->ammo->value[0] );
               else if ( wield->weapon_mode == MODE_BURST ) miss_message( ch, victim, -1, wield->ammo->value[0] );
               else miss_message( ch, victim, wield->value[0], wield->ammo->value[0] );
            }
            else if ( wield->value[0] == WEAPON_ERANGED )
            {
               if ( wield->weapon_mode == MODE_AUTOMATIC ) miss_message( ch, victim, -2, 1 );
               else if ( wield->weapon_mode == MODE_BURST ) miss_message( ch, victim, -1, 1 );
               else miss_message( ch, victim, wield->value[0], 1 );
            }
            else
            {
               miss_message( ch, victim, wield->value[0], wield->value[4] );
            }
        }
        else
        {
            miss_message( ch, victim, 0, dt );
        }

        auto_eject( ch, wield ); // Check for clip ejection

        showammo_option( ch, wield ); // ShowAmmo Notification

        tail_chain( );
	return rNONE;
    }


    /*
     * Damage Modifers
     */
    if ( !IS_AWAKE(victim) )
       dam *= 3;

    /* Immune to damage */
    if ( dam < 0 ) dam = 0;

    /*
     * Deal the damage
     */
    if ( wield )
    {
       if ( !melee && victim->cover )
       {
           /* Awww. We hit a cover object. */
           dam = cdamage( ch, victim, dam, FALSE );
       }
       if ( dam > -1 )
       {
          if ( wield->ammo )
          {
             retcode = damage( ch, victim, dam, TYPE_GENERIC + wield->ammo->value[0] );
          }
          else if ( wield->value[0] == WEAPON_ERANGED )
          {
             retcode = damage( ch, victim, dam, TYPE_GENERIC + RIS_ENERGY );
          }
          else
          {
             retcode = damage( ch, victim, dam, TYPE_GENERIC + wield->value[4] );
          }
       }
    }
    else
    {
       retcode = damage( ch, victim, dam, dt );
    }


    if ( char_died(ch) )
      return rCHAR_DIED;

    /*
     * Flaming critters. Weee.
     */
    if ( retcode == rNONE && flame )
    {
        int duration = 3;

        if ( victim->race == RACE_ALIEN ) duration = URANGE( 3, get_trust(victim), 8 );
        if ( victim->race == RACE_MARINE ) duration = URANGE( 4, get_trust(victim), 12 );
        if ( victim->race == RACE_PREDATOR ) duration = URANGE( 5, get_trust(victim), 12 );

        duration = number_range( duration - 2, duration + 2 );

        ignite_target( ch, victim, duration );
    }

    /*
     * Point-blank alien bloodspray
     *  (2.8) = 100% Protection (2.4) = Just a bit less than 100%.
     */
    if ( retcode == rNONE && !flame && victim->race == RACE_ALIEN && ch->in_room == victim->in_room && number_percent( ) > ( ( get_curr_per( ch ) * 2.4 ) + 10 ) )
    {
        int spray = 0, tspray = 0;

        spray = ( dam / 2 );

        /* Acid Potency Skill */
        if ( victim->pcdata )
        {
          if ( victim->pcdata->learned[gsn_acid_potency] == 1 ) spray *= 1.10;
          if ( victim->pcdata->learned[gsn_acid_potency] == 2 ) spray *= 1.20;
          if ( victim->pcdata->learned[gsn_acid_potency] == 3 ) spray *= 1.30;
        }

        if ( spray > 0 )
        {
           char tmp[MAX_STRING_LENGTH];

           tspray = ris_damage( ch, spray, RIS_ACID, FALSE );  // Acid-proof armor

           sprintf( tmp, "$n is doused in acid from an alien! (%d acid damage)", tspray );
           act( AT_GREEN, tmp, ch, NULL, NULL, TO_ROOM );

           ch_printf( ch, "&GYour doused in acid from the blood spray! (%d acid damage)\n\r", tspray );

           retcode = damage( victim, ch, spray, TYPE_GENERIC + RIS_ACID );
        }
    }

    if ( char_died(ch) )
      return rCHAR_DIED;

    auto_eject( ch, wield ); // Check for clip ejection

    showammo_option( ch, wield ); // ShowAmmo Notification

    if ( char_died(victim) )
      return rVICT_DIED;

    retcode = rNONE;
    if ( dam == 0 )
      return retcode;

    /*
     * Folks with ranged weapons move and snipe instead of getting neatin up in one spot.
     */
     if ( IS_NPC(victim) && victim->race != RACE_ALIEN )
     {
         OBJ_DATA *wield;
         
         wield = get_eq_char( victim, WEAR_WIELD );                 
         if ( wield != NULL && get_cover( victim ) == TRUE )
         {
               start_hating( victim, ch );
	       start_hunting( victim, ch );
         }
     }
     
    tail_chain( );
    return retcode;
}

/*
 * Calculate damage based on armor resistances. - Ghost.
 */
sh_int ris_damage( CHAR_DATA *ch, sh_int dam, int ris, bool apply )
{
   char buf[MAX_STRING_LENGTH];
   AFFECT_DATA * paf;
   OBJ_DATA * obj, * obj_next;
   sh_int modifier;
   int value = 0;
   int result = 0;
   int protect = 0;
   int soak = 0;
   int osum = 0;

   if ( ris < RIS_FIRE || ris > RIS_ACID ) return dam;

   modifier = 100;

   protect = ch->protect[ris];

   /* Special Modifiers */
   if ( ch->swarm > 0 ) protect += ( ch->swarm * 10 );

   modifier -= URANGE( -100, protect, 100 );

   result = ( dam * modifier ) / 100;

   soak = dam - result;

   /* Damage armor */
   if ( apply )
   {
       for ( obj = ch->first_carrying; obj; obj = obj_next )
       {           
           obj_next = obj->next_content;
           if ( obj->wear_loc < 0 || obj->wear_loc > MAX_WEAR ) continue;

           for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
           {
               value = 0;

               if ( paf->location == APPLY_FIRE && ris == RIS_FIRE ) value = paf->modifier;
               if ( paf->location == APPLY_ENERGY && ris == RIS_ENERGY ) value = paf->modifier;
               if ( paf->location == APPLY_IMPACT && ris == RIS_IMPACT ) value = paf->modifier;
               if ( paf->location == APPLY_PIERCE && ris == RIS_PIERCE ) value = paf->modifier;
               if ( paf->location == APPLY_ACID && ris == RIS_ACID ) value = paf->modifier;

               /* Damage it now */
               if ( value != 0 )
               {
                   int perc, odam;

                   perc = (int)( (float)(value) * (float)(100) ) / (float)(protect);
                   odam = (int)( (float)(soak) * (float)(perc) ) / (float)(100);

                   odam = apply_armor_skill( ch, obj, odam );
                   osum += odam;

                   if ( damage_obj( obj, odam ) == rOBJ_SCRAPPED ) break;
               }
           }
           for ( paf = obj->first_affect; paf; paf = paf->next )
           {
               value = 0;

               if ( paf->location == APPLY_FIRE && ris == RIS_FIRE ) value = paf->modifier;
               if ( paf->location == APPLY_ENERGY && ris == RIS_ENERGY ) value = paf->modifier;
               if ( paf->location == APPLY_IMPACT && ris == RIS_IMPACT ) value = paf->modifier;
               if ( paf->location == APPLY_PIERCE && ris == RIS_PIERCE ) value = paf->modifier;
               if ( paf->location == APPLY_ACID && ris == RIS_ACID ) value = paf->modifier;

               /* Damage it now */
               if ( value != 0 )
               {
                   int perc, odam;

                   perc = (int)( (float)(value) * (float)(100) ) / (float)(protect);
                   odam = (int)( (float)(soak) * (float)(perc) ) / (float)(100);

                   odam = apply_armor_skill( ch, obj, odam );
                   osum += odam;

                   if ( damage_obj( obj, odam ) == rOBJ_SCRAPPED ) break;
               }
           }
       }
   }

   /* Damage absorbed is dam - result */
   if ( apply && soak > 0 )
   {
      if ( osum > 0 )
      {
         ch_printf( ch, "&GYour armor soaked %d damage. (Armor lost %d points)\n\r", soak, osum );
      }
      else
      {
         if ( ch->race == RACE_ALIEN )
         {
           ch_printf( ch, "&GYou automatically soaked %d damage.\n\r", soak );
         }
         else
         {
           ch_printf( ch, "&GYour armor soaked %d damage.\n\r", soak );
         }
      }
   }

   return result;
}


/*
 * Inflict damage from a hit.
 */
ch_ret damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    char buf[MAX_STRING_LENGTH];
    bool npcvict;
    bool canresist = TRUE;
    bool tripped = FALSE;
    AFFECT_DATA *paf;
    ch_ret retcode;
    sh_int dampmod;

    retcode = rNONE;

    if ( !ch )
    {
	bug( "Damage: null ch!", 0 );
	return rERROR;
    }
    if ( !victim )
    {
	bug( "Damage: null victim!", 0 );
	return rVICT_DIED;
    }
    
    if ( victim->position == POS_DEAD )
    {
       return rVICT_DIED;
    }

    npcvict = (IS_NPC(victim) && !IS_BOT(victim));

    if ( victim )
     if ( victim->in_room )
      if ( ( xIS_SET( victim->in_room->room_flags, ROOM_SAFE ) && victim->hit >= victim->max_hit ) )
       dam = 0;

    if ( ch )
     if ( ch->in_room )
      if ( ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) && ch->hit >= ch->max_hit ) )
       dam = 0;

    if ( ch == victim ) tripped = TRUE;

    /*
     * Critical Strike for Aliens.
     */
    if ( ch->race == RACE_ALIEN && ch->pcdata && dt == (TYPE_GENERIC + RIS_PIERCE) && dam > 0 && !tripped )
    {
       int sn;

       sn = gsn_critical_strike;

       if ( ch->pcdata->prepared[sn] >= skill_table[sn]->reset && ch->in_room == victim->in_room && !IS_NPC( victim ) )
       {
           if ( number_range( 1, 100 ) <= (ch->pcdata->learned[sn] * 10) )
           {
               dam *= 2;
               tripped = TRUE;
               ch->pcdata->prepared[sn] = 0;
               ch_printf( ch, "&R(Critical Strike) Dealing double attack damage.\n\r" );
               ch_printf( victim, "&R(Critical Strike) Taking double attack damage.\n\r" );
           }
       }
    }
                                
    /*
     * Battle Method for Aliens.
     */
    if ( ch->race == RACE_ALIEN && ch->pcdata && dam > 0 && !tripped )
    {
       int sn;

       sn = gsn_battle_method;

       if ( ch->pcdata->prepared[sn] >= skill_table[sn]->reset && ch->in_room == victim->in_room && !IS_NPC( victim ) )
       {
           if ( number_range( 1, 100 ) <= (ch->pcdata->learned[sn] * 10) )
           {
               canresist = FALSE;
               tripped = TRUE;
               ch->pcdata->prepared[sn] = 0;
               ch_printf( ch, "&R(Battle Method) Your attack is bypassing the armor.\n\r" );
               ch_printf( victim, "&R(Battle Method) Your armor has been bypassed.\n\r" );
           }
       }
    }
                                
    /*
     * Near Miss for Marines.
     */
    if ( victim->pcdata && victim->hit > 0 && dam > 0 )
    {
         int near;

         near = victim->pcdata->learned[gsn_near_miss];
         if ( near == 1 ) near = 90;
         if ( near == 2 ) near = 70;
         if ( near == 3 ) near = 50;

         if ( victim->pcdata->prepared[gsn_near_miss] < skill_table[gsn_near_miss]->reset ) near = 0;

         if ( ( dam >= near && dam <= 150 ) && near > 0 )
         {
             ch_printf( victim, "&w&C(Near Miss) You narrowly avoided taking damage from the attack!\n\r" );
             victim->pcdata->prepared[gsn_near_miss] = 0;
             dam = 0;
             return rNONE;             
         }
    }

    /*
     * Alien Rage for, well... Aliens.
     */
    for ( paf = victim->first_affect; paf; paf = paf->next )
     if ( paf->type == gsn_alien_rage && victim->in_room ) dam /= 2;

    /*
     * Can we resist any of this damage?
     */
    if ( dam && dt >= TYPE_GENERIC && canresist )
    {
        int type = 0;

        type = ( dt - TYPE_GENERIC );

        dam = ris_damage( victim, dam, type, TRUE );
    }

    if ( dam && npcvict && ch != victim )
    {
        if ( !xIS_SET( victim->act, ACT_SENTINEL ) )
 	{
	   if ( victim->hunting )
	   {
              if ( victim->hunting->who != ch )
              {
                 STRFREE( victim->hunting->name );
                 victim->hunting->name = QUICKLINK( ch->name );
                 victim->hunting->who  = ch;
              }
           }
	   else
	     start_hunting( victim, ch );
	}

        if ( victim->hating )
        {
           if ( victim->hating->who != ch )
           {
             STRFREE( victim->hating->name );
             victim->hating->name = QUICKLINK( ch->name );
             victim->hating->who  = ch;
           }
        }
        else
           start_hating( victim, ch );
    }

    if ( victim != ch )
    {      
	/* Take away Hide */
        if ( IS_AFFECTED(ch, AFF_HIDE) ) xREMOVE_BIT(ch->affected_by, AFF_HIDE);

        if ( dam < 0 ) dam = 0;
     
        /*
         * Check control panel settings and modify damage
         */
        if ( IS_NPC(ch) && !IS_BOT(ch) )
        {
           if ( npcvict ) dampmod = sysdata.dam_mob_vs_mob;
           else dampmod = sysdata.dam_mob_vs_plr;
        }
        else
        {
           if ( npcvict ) dampmod = sysdata.dam_plr_vs_mob;
           else dampmod = sysdata.dam_plr_vs_plr;
        }

        /*
         * Apply Victim's armor value
         *  Scale range: -600 AC =  90% Damage blocked
         *  Scale range:    0 AC =   0% Damage blocked
         */    
        /*
        {
            int soak=0;

            soak = (600 + ( 0 - UMAX(-600, GET_AC(victim)) )) / (40/3);

            dam -= (dam * (soak / 100));
        }
        */

        if ( dampmod > 0 )
            dam = ( dam * dampmod ) / 100;

    }


    /*
     * Inform those around the target, but only if its not acid.
     */
    if ( dam > 0 && dt != TYPE_GENERIC + RIS_ACID )
    {
        sprintf( buf, "screams loudly! &w&z(&WTook %d Damage&z)", dam );
        do_emote( victim, buf );
    }  

    /*
     * Energy Damage - Stuns players
     */
    if ( dam > 0 && dt == (TYPE_GENERIC + RIS_ENERGY) )
    {
        int stun = 0;

        stun = URANGE( 0, (int)( dam / 20 ), 5 );

        short_target( ch, victim,  6 + (stun * 2) );

        if ( stun > 0 )
        {
            ch_printf( victim, "&w&C(Energy Damage) You are stunned for %d rounds.\n\r", stun );
            WAIT_STATE( victim, stun * 4 );
        }
    }

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;

    if ( !IS_NPC(victim) && victim->top_level >= LEVEL_IMMORTAL && victim->hit < 1 )
       victim->hit = 1;

    update_pos( victim );

    /* Automatic Request for help */
    if ( victim->position <= POS_STUNNED && victim->position > POS_DEAD )
    {
        char buf[MAX_STRING_LENGTH];

        sprintf( buf, "I need a medic at %d:%d:%d!", victim->in_room->x, victim->in_room->y, victim->in_room->z );

        if ( victim->race == RACE_MARINE ) do_radio( victim, buf );
    }

    switch( victim->position )
    {
       case POS_MORTAL:
	act( AT_DYING, "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
        send_to_char( "&RYou are mortally wounded, and will die soon, if not aided.\n\r",victim);
	break;

       case POS_INCAP:
	act( AT_DYING, "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
        send_to_char( "&RYou are incapacitated and will slowly die, if not aided.\n\r",victim);
	break;

       case POS_STUNNED:
        if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
        {
          act( AT_ACTION, "$n is stunned, but will probably recover.&w",
	    victim, NULL, NULL, TO_ROOM );
          send_to_char( "&RYou are stunned, but will probably recover.\n\r",victim);
	}
	break;

       case POS_DEAD:
         if ( IS_NPC(victim) && xIS_SET( victim->act, ACT_NOKILL )  )
	   act( AT_YELLOW, "$n flees for $s life ... barely escaping certain death!", victim, 0, 0, TO_ROOM );
         else if ( IS_NPC(victim) && (xIS_SET( victim->act, ACT_DROID ) || xIS_SET( victim->act, ACT_PET))  )
	   act( AT_DEAD, "$n EXPLODES into many small pieces!", victim, 0, 0, TO_ROOM );
	else
	   act( AT_DEAD, "$n is DEAD!", victim, 0, 0, TO_ROOM );
        send_to_char( "&YYou have been KILLED!\n\r", victim);
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	{
           // act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );
	   if ( number_bits(3) == 0 )
		worsen_mental_state( victim, 1 );
	}
        /*
        if ( victim->hit < victim->max_hit / 4 )

	{
	   act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!",
		victim, 0, 0, TO_CHAR );
	   if ( number_bits(2) == 0 )
		worsen_mental_state( victim, 1 );
	}
        */
	break;
    }

    add_timer( victim, TIMER_RECENTFIGHT, 100, NULL, 0 );
       
    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
        radius_gain( ch, victim );

        if ( !npcvict && ch != victim )
	{
            char note[MAX_STRING_LENGTH];
            DESCRIPTOR_DATA *d;
            int msg = 0;

            strcpy( note, "" );

            sprintf( log_buf, "%s killed by %s at %d", victim->name, (IS_NPC(ch) ? ch->short_descr : ch->name), victim->in_room->vnum );
            // log_string( log_buf );
            // to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
           
            if ( !is_enemy(ch, victim) ) strcpy( note, " [TEAMKILL]" );

            /* Broadcast kill log to all players - Taunts! */
            msg = number_range( 1, 12 );
            for (d = first_descriptor; d; d = d->next)
            {
                if(d->connected != CON_PLAYING) continue;
                if(!d->character) continue;

                if ( msg == 1 )  ch_printf( d->character, "&R(Alert) %s bashed %s with a whuppin' stick! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 2 )  ch_printf( d->character, "&R(Alert) %s pasted %s! Ouch! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 3 )  ch_printf( d->character, "&R(Alert) %s learned %s some manners! Bwhahaha! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 4 )  ch_printf( d->character, "&R(Alert) %s opened a can of whoop-ass on %s! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 5 )  ch_printf( d->character, "&R(Alert) %s blasted %s's ass off! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 6 )  ch_printf( d->character, "&R(Alert) %s showed %s where the trigger was! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 7 )  ch_printf( d->character, "&R(Alert) %s taught %s where not to stand! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 8 )  ch_printf( d->character, "&R(Alert) %s thought %s needed to pay more attention! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 9 )  ch_printf( d->character, "&R(Alert) %s told %s where he could shove it! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 10 ) ch_printf( d->character, "&R(Alert) %s rearranged %s's facial features! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 11 ) ch_printf( d->character, "&R(Alert) %s demonstrated the art of ass-kicking for %s! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
                if ( msg == 12 ) ch_printf( d->character, "&R(Alert) %s bitchslapped %s into submission! %s\n\r", (IS_NPC(ch) ? ch->short_descr : ch->name), victim->name, note );
            }
	}

	check_killer( ch, victim );

        set_cur_char(victim);
	raw_kill( ch, victim );
	victim = NULL;

        if ( IS_SET( sysdata.save_flags, SV_KILL ) ) save_char_obj( ch );
	return rVICT_DIED;
    }

    if ( victim != NULL && ch != NULL )
    {
        // Vision Mode Emergancy Cutout.
        if ( victim->vision != -1 && ch->race != victim->vision )
        {
           do_switch( victim, "normal" );
        }
    }

    if ( victim == ch ) return rNONE;

    tail_chain( );
    return rNONE;
}

void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{   
    if ( ch == victim )
    {
       // Record a suicide
       if ( IS_BOT( ch ) )
       {
          match_log( "KILL;'BOT:%s' suicided with '%s'", ch->name, current_weapon( ch ) );
       }
       else
       {
          if ( IS_NPC( ch ) ) match_log( "KILL;'NPC:%s' suicided with '%s'", ch->name, current_weapon( ch ) );
          if ( !IS_NPC( ch ) ) match_log( "KILL;'PC:%s' suicided with '%s'", ch->name, current_weapon( ch ) );
       }
       return;
    }
    else if ( IS_BOT( ch ) )
    {
       if ( IS_NPC( victim ) )
       {
          if ( !IS_BOT( victim ) ) match_log( "KILL;'BOT:%s' killed 'NPC:%s' with '%s'", ch->name, victim->name, current_weapon( ch ) );
          if ( IS_BOT( victim ) ) match_log( "KILL;'BOT:%s' killed 'BOT:%s' with '%s'", ch->name, victim->name, current_weapon( ch ) );
          return;
       }
       else
       {
          match_log( "KILL;'BOT:%s' killed 'PC:%s' with '%s'", ch->name, victim->name, current_weapon( ch ) );
          if ( victim->pcdata ) victim->pcdata->killed.bot[ch->race]++;
       }
    }
    else if ( IS_NPC( ch ) )
    {
       if ( IS_NPC( victim ) )
       {
          if ( !IS_BOT( victim ) ) match_log( "KILL;'NPC:%s' killed 'NPC:%s' with '%s'", ch->name, victim->name, current_weapon( ch ) );
          if ( IS_BOT( victim ) ) match_log( "KILL;'NPC:%s' killed 'BOT:%s' with '%s'", ch->name, victim->name, current_weapon( ch ) );
          return;
       }
       else
       {
         match_log( "KILL;'NPC:%s' killed 'PC:%s' with '%s'", ch->name, victim->name, current_weapon( ch ) );
         if ( victim->pcdata ) victim->pcdata->killed.npc[ch->race]++;
       }
    }
    else
    {
       if ( IS_BOT( victim ) )
       {
         match_log( "KILL;'PC:%s' killed 'BOT:%s' with '%s'", ch->name, victim->name, current_weapon( ch ) );
         if ( ch->pcdata ) ch->pcdata->kills.bot[victim->race]++;
       }
       else if ( IS_NPC( victim ) )
       {
         match_log( "KILL;'PC:%s' killed 'NPC:%s' with '%s'", ch->name, victim->name, current_weapon( ch ) );
         if ( ch->pcdata ) ch->pcdata->kills.npc[victim->race]++;
       }
       else
       {
         match_log( "KILL;'PC:%s' killed 'PC:%s' with '%s'", ch->name, victim->name, current_weapon( ch ) );
         manage_streak( ch, NULL, FALSE );
         if ( curr_arena ) curr_arena->kills[ch->race]++;
         if ( ch->pcdata ) ch->pcdata->kills.pc[victim->race]++;
         if ( victim->pcdata ) victim->pcdata->killed.pc[ch->race]++;
       }
    }

    return;
}

void manage_streak( CHAR_DATA * ch, CHAR_DATA * rch, bool reset )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch == NULL ) return;

    if ( reset )
    {
       if ( rch == NULL || rch == ch )
       {
          if ( ch->streak > 10 )
          {
             sprintf( buf, "&w&R%s's killing spree has come to an end. (%d Kills)", ch->name, ch->streak );
             send_monitor( NULL, buf );
          }
       }
       else
       {
          if ( ch->streak > 10 )
          {
             sprintf( buf, "&w&R%s's killing spree was ended by %s! (%d Kills)", ch->name, rch->name, ch->streak );
             send_monitor( NULL, buf );
          }
       }
       ch->streak = 0;
    }
    else
    {
       ch->streak++;

       if ( ch->streak == 10 )
       {
           sprintf( buf, "&w&R%s is on a killing spree!", ch->name );
           send_monitor( NULL, buf );
       }
    }
}


/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( !victim )
    {
      bug( "update_pos: null victim", 0 );
      return;
    }

    if ( victim->hit > 0 )
    {
      if ( victim->position <= POS_STUNNED )
         victim->position = POS_STANDING;
      if ( IS_AFFECTED( victim, AFF_PARALYSIS ) )
         victim->position = POS_STUNNED;
      if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) && victim->position == POS_STUNNED )
         victim->position = POS_STANDING;
      return;
    }

    if ( IS_NPC(victim) || victim->hit <= -50 )
    {
	if ( victim->mount )
	{
          act( AT_ACTION, "$n falls from $N.", victim, NULL, victim->mount, TO_ROOM );
          xREMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	  victim->mount = NULL;
	}
	victim->position = POS_DEAD;

	return;
    }

         if ( victim->hit <= -30 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -10 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    if ( victim->position > POS_STUNNED && IS_AFFECTED( victim, AFF_PARALYSIS ) )
      victim->position = POS_STUNNED;

    if ( victim->mount )
    {
        act( AT_ACTION, "$n falls unconscious from $N.", victim, NULL, victim->mount, TO_ROOM );
        xREMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	victim->mount = NULL;
    }
    return;
}


/*
 * Restored Death Cry code
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    EXIT_DATA *pexit;
    int vnum;

    if ( !ch )
    {
      bug( "DEATH_CRY: null ch!", 0 );
      return;
    }

    vnum = 0;

    if ( IS_NPC(ch) )
       msg = "You hear something's agonized cry.";
    else
       msg = "You hear someone's agonized cry of death!";

    if ( !ch->in_room ) return;

    was_in_room = ch->in_room;
    for ( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
    {
        if ( pexit->to_room && pexit->to_room != was_in_room )
        {
           ch->in_room = pexit->to_room;
           act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );
        }
    }
    ch->in_room = was_in_room;
    return;
}



void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{
    
    CHAR_DATA *victmp;
    
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];    
    OBJ_DATA *obj, *obj_next;
    FILE *fp;
    
    if ( !victim )
    {
      bug( "raw_kill: null victim!", 0 );
      return;
    }
   
    if ( !IS_NPC( victim ) ) match_log( "DEATH;'PC:%s' killed while using '%s'", victim->name, current_weapon( victim ) );
 
    manage_streak( victim, ch, TRUE );

    /*
     * Carrying Patch
     */
    if ( victim->carried )
    {      
        act( AT_LBLUE, "You release $N, dropping $M on the ground.", victim->carried, NULL, victim, TO_CHAR );
        act( AT_LBLUE, "$n releases $N, dropping $M on the ground..", victim->carried, NULL, victim, TO_NOTVICT );
        victim->carried->carrying = NULL;
        victim->carried = NULL;
    }
   
    strcpy( arg , victim->name );

    /* Take care of polymorphed chars */
    if(IS_NPC(victim) && xIS_SET(victim->act, ACT_POLYMORPHED))
    {
      char_from_room(victim->desc->original);
      char_to_room(victim->desc->original, victim->in_room);
      victmp = victim->desc->original;
      raw_kill(ch, victmp);
      return;
    }

    if ( !IS_NPC(victim) || !xIS_SET( victim->act, ACT_NOKILL  ) )
    {
       if ( ch != NULL ) mprog_death_trigger( ch, victim );
    }

    if ( char_died(victim) ) return;

    if ( !IS_NPC(victim) || !xIS_SET( victim->act, ACT_NOKILL  ) )
    {
       if ( ch != NULL ) rprog_death_trigger( ch, victim );
    }

    if ( char_died(victim) ) return;

    if ( !IS_NPC(victim) || ( !xIS_SET( victim->act, ACT_NOKILL  ) && !xIS_SET( victim->act, ACT_NOCORPSE ) ) )
    {
       make_corpse( victim, ch );
    }
    else
    {
       for ( obj = victim->last_carrying; obj; obj = obj_next )
       {
           obj_next = obj->prev_content;
           obj_from_char( obj );
           extract_obj( obj );
       }
    }
    
    die_follower(victim);

    clear_timers(victim);
    clear_effects(victim);

    victim->vent = FALSE;
    victim->wait = 0;

    if ( IS_BOT(victim) && victim->bot != NULL )
    {
        victim->bot->respawn = 30;
        victim->position = POS_STANDING;
        victim->hit = victim->max_hit;
        victim->move = victim->max_move;
        victim->field = victim->max_field;
        victim->resin = get_max_resin( victim );
        victim->ap = get_max_ap( victim );
        victim->mp = get_max_mp( victim );
        char_from_room( victim );
        char_to_room( victim, get_room_index(5) );
        return;
    }
    else if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
        extract_char( victim, TRUE, FALSE );
	victim = NULL;
	return;
    }

    if ( victim && victim->pcdata )
    {        
        int diff = 0;

        diff = ( get_max_teamkill(victim) - victim->teamkill );

        victim->pcdata->respawn = 30 + ( diff * 30 );
        victim->pcdata->spectator = TRUE;
        victim->position = POS_STANDING;
        victim->hit = victim->max_hit;
        victim->move = victim->max_move;
        victim->field = victim->max_field;
        victim->resin = get_max_resin( victim );
        victim->ap = get_max_ap( victim );
        victim->mp = get_max_mp( victim );
    }

    if ( !victim )
    {
       DESCRIPTOR_DATA *d;
    
       /* Make sure they aren't halfway logged in. */
       for ( d = first_descriptor; d; d = d->next )
         if ( (victim = d->character) && !IS_NPC(victim)  )
            break;

       if ( d )
         close_socket( d, TRUE );
   }

   return;
}         



void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( is_spectator( ch ) ) { send_to_char( "&RYou may not engage in combat while in spectator mode.\n\r", ch ); return; }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
        send_to_char( "Why are you attacking yourself?\n\r", ch );
	return;
    }

    if ( ch->ap < get_max_ap( ch ) )
    {
        send_to_char( "&RYou must wait for the attack gauge to replenish first.\n\r", ch );
        return;
    }

    multi_hit( ch, victim, TYPE_UNDEFINED );

    return;
}

void do_tackle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA * objA;
    OBJ_DATA * objB;
    int chance = -15;
    int power = 0;

    one_argument( argument, arg );

    if ( ch->race != RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( is_spectator( ch ) ) { send_to_char( "&RYou may not engage in combat while in spectator mode.\n\r", ch ); return; }

    if ( arg[0] == '\0' )
    {
        send_to_char( "&RSyntax: TACKLE (target)\n\r", ch );
	return;
    }

    power = ( get_curr_str( ch ) / 10 );

    if ( power <= 0 )
    {
        send_to_char( "&RYour not strong enough to even attempt that!\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
        send_to_char( "Why are you attacking yourself?\n\r", ch );
	return;
    }

    if ( ch->ap < get_max_ap( ch ) )
    {
        send_to_char( "&RYou must wait for the attack gauge to replenish first.\n\r", ch );
        return;
    }

    if ( ch->move < ( power * 10 ) )
    {
        send_to_char( "&RNot enough movement to tackle. Wait a few rounds.\n\r", ch );
        return;   
    }

    chance = URANGE( 80, get_curr_per( ch ) + 65, 95 );

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) ) chance = 0;

    if ( number_percent( ) > chance )
    {
       ch->ap = 0;
       ch_printf( ch, "&RYou throw yourself at %s, but miss!\n\r", PERS( victim, ch ) );
       act( AT_YELLOW, "$n misses $N with a flying tackle!", ch, NULL, victim, TO_NOTVICT );
       act( AT_YELLOW, "$n misses you with a flying tackle!", ch, NULL, victim, TO_VICT    );
       return;
    }

    /*
     * Tackle
     */
    ch->ap = 0;
    victim->ap = UMAX( 0, victim->ap - (power+1) );
    victim->mp = UMAX( 0, victim->mp - power );

    ch->block = NULL;
    victim->block = NULL;

    ch_printf( ch, "&rYou throw yourself at %s, tackling them!\n\r", PERS( victim, ch ) );
    act( AT_RED, "$n hits $N with a flying tackle!", ch, NULL, victim, TO_NOTVICT );
    act( AT_RED, "$n hits you with a flying tackle!", ch, NULL, victim, TO_VICT    );

    // Remove hide
    if ( IS_AFFECTED(ch, AFF_HIDE) ) xREMOVE_BIT(ch->affected_by, AFF_HIDE);

    // Tackle damage
    damage( ch, victim, (power * 10)+20, TYPE_GENERIC + RIS_IMPACT );

    if ( char_died(victim) ) return;

    // Potential Disarm
    chance += get_curr_str( ch ) * 2;

    chance -= get_curr_str( victim );

    objA = get_eq_char( victim, WEAR_WIELD );
    objB = get_eq_char( victim, WEAR_DUAL_WIELD );
    if ( objB == NULL ) objB = get_eq_char( victim, WEAR_HOLD );

    if ( objA != NULL ) if ( xIS_SET(objA->extra_flags, ITEM_NOTACKLE) ) objA = NULL;
    if ( objB != NULL ) if ( xIS_SET(objB->extra_flags, ITEM_NOTACKLE) ) objB = NULL;

    if ( objA != NULL ) chance += (objA->weight * 2);
    if ( objB != NULL ) chance += (objB->weight * 2);
    if ( objA && objB ) chance /= 2;

    chance = URANGE( 20, chance, 80 );
    
    if ( number_percent() < chance && victim->hit > 0 && ch->wait <= 2 )
    {
       if ( objA )
       {
          act( AT_CYAN, "&w&C(Tackle) $n drops $p!", victim, objA, NULL, TO_ROOM );
          act( AT_CYAN, "&w&C(Tackle) You drop $p!", victim, objA, NULL, TO_CHAR );
          drop_morale( victim, objA->weight );
          unequip_char( victim, objA );
          obj_from_char( objA );
          objA = obj_to_room( objA, victim->in_room );
       }
       if ( objB )
       {
          act( AT_CYAN, "&w&C(Tackle) $n drops $p!", victim, objB, NULL, TO_ROOM );
          act( AT_CYAN, "&w&C(Tackle) You drop $p!", victim, objB, NULL, TO_CHAR );
          drop_morale( victim, objB->weight );
          unequip_char( victim, objB );
          obj_from_char( objB );
          objB = obj_to_room( objB, victim->in_room );
       }      
    }

    // Victim loses morale. (Marine only)
    drop_morale( victim, (power*5) );

    // Waitstate lag
    WAIT_STATE( ch, 4 );
    WAIT_STATE( victim, 8 );

    return;
}


bool get_cover( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    int attempt;
    sh_int door;
    EXIT_DATA *pexit;

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 10; attempt++ )
    {
       door = number_door( );
       if ( ( pexit = get_exit(was_in, door) ) == NULL
       ||   !pexit->to_room
       || ( xIS_SET(pexit->exit_info, EX_CLOSED)
       &&   !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
       || ( IS_NPC(ch)
       &&   xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) ) )
          continue;
 
        xREMOVE_BIT( ch->affected_by, AFF_SNEAK );
        move_char( ch, pexit, pexit->vdir, 0 );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	act( AT_FLEE, "$n sprints for cover!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;
	act( AT_FLEE, "$n spins around and takes aim.", ch, NULL, NULL, TO_ROOM );

	return TRUE;
    }

    return FALSE;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    if ( arg[0] == '\0' )
    {
    send_to_char( "Syntax: SLAY <target> <type>\n\r", ch );
    send_to_char( "        SLAY [list]\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "list" ) )
    {
       send_to_char("&YTypes:\n\r", ch );
       send_to_char("&RIMMOLATE, SHATTER, DEMON, POUNCE, SLIT, BLADE,\n\r", ch );
       send_to_char("&RTWIDDLE, SNEEZE, GRENADE, SPOON, WALL, JD,\n\r", ch );
       send_to_char("&RPEACE, SLOW\n\r", ch);
       return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && ( get_trust( victim ) >= get_trust( ch ) || get_trust( ch ) < 103) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "immolate" ) )
    {
      act( AT_FIRE, "Your fireball turns $N into a blazing inferno.",  ch, NULL, victim, TO_CHAR    );
      act( AT_FIRE, "$n releases a searing fireball in your direction.", ch, NULL, victim, TO_VICT    );
      act( AT_FIRE, "$n points at $N, who bursts into a flaming inferno.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "shatter" ) )
    {
      act( AT_LBLUE, "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",  ch, NULL, victim, TO_CHAR    );
      act( AT_LBLUE, "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim, TO_VICT    );
      act( AT_LBLUE, "$n freezes $N with a glance and shatters the frozen body into tiny shards.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "demon" ) )
    {
      act( AT_IMMORT, "You gesture, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "$n gestures, and a slavering demon appears.  The foul creature turns on",  ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "you with a horrible grin.   You scream in panic before being eaten alive.",  ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "$n gestures, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_NOTVICT );
      act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "pounce" ) && get_trust(ch) >= LEVEL_ASCENDANT )
    {
      act( AT_BLOOD, "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...",  ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",  ch, NULL, victim, TO_NOTVICT );
    }
    else if ( !str_cmp( arg2, "slit" ) && get_trust(ch) >= LEVEL_ASCENDANT )
    {
      act( AT_BLOOD, "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
    }
    else if ( !str_cmp( arg2, "blade" ) && get_trust(ch) >= LEVEL_ASCENDANT )
    {
      act( AT_BLOOD, "You reach out and *RAM* a blade into $N's chest!", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n reaches out and *RAMS* a sharped blade into your chest!", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n reachs out and *RAMS* a blade into $N's chest!", ch, NULL, victim, TO_NOTVICT );
    }
    else if ( !str_cmp( arg2, "twiddle" ) && get_trust(ch) >= LEVEL_ASCENDANT )
    {
      act( AT_BLOOD, "You twiddle your fingers, and $N clutchs his throat!", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n twiddles his fingers, and the air is drained from your lungs!", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n twiddles his fingers, and $N gasps for air!", ch, NULL, victim, TO_NOTVICT );
    }
    else if ( !str_cmp( arg2, "sneeze" ) && get_trust(ch) >= LEVEL_ASCENDANT )
    {
      act( AT_BLOOD, "You sneeze loudly, spraying $N with the acidic goo! Awww, Crap...", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n sneezes, spraying you with an acidic goo!", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n sneezes, spraying $N with an acidic goo!", ch, NULL, victim, TO_NOTVICT );
    }
    else if ( !str_cmp( arg2, "grenade" ) )
    {
      act( AT_SAY, "You shove a grenade down $N's throat and pull the pin.", ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "BOOM!!!...$N's brain matter splatters all over everyone in the room.", ch, NULL, victim, TO_CHAR );
      act( AT_SAY, "$n shoves a grenade down your throat and pulls the pin.", ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "BOOM!!!...Your brain matter splatters all over everyone in the room.", ch, NULL, victim, TO_VICT );
      act( AT_SAY, "$n shoves a grenade down $N's throat and pulls the pin.", ch, NULL, victim, TO_NOTVICT );
      act( AT_IMMORT, "BOOM!!!...$N's brain matter splatters all over you.", ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "spoon" ) )
    {
      act( AT_IMMORT, "You gingerly insert a spoon into $N's chest, pry out $S heart,", ch, NULL, victim, TO_CHAR ); 
      act( AT_IMMORT, "and force-feed it to $M moments before $E dies.", ch, NULL, victim, TO_CHAR );
      act( AT_IMMORT, "$n gingerly inserts a spoon into your chest, pries out your heart,", ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "and force-feeds it to you moments before you die.", ch, NULL, victim, TO_VICT );
      act( AT_IMMORT, "$n gingerly inserts a spoon into $N's chest, pries out $S heart,", ch, NULL, victim, TO_NOTVICT );
      act( AT_IMMORT, "and force-feeds it to  $M moments before $E dies.", ch, NULL, victim, TO_NOTVICT );
    }
    else if ( !str_cmp( arg2, "wall" ) )
    {
      act( AT_DEAD, "You repeatedly slam $N's head into a wall, causing $S brains to ooze out of $S ears.", ch, NULL, victim, TO_CHAR );
      act( AT_DEAD, "$n repeatedly slams your head into a wall, causing your brains to ooze out of your ears.", ch, NULL, victim, TO_VICT );
      act( AT_DEAD, "$n repeatedly slams $N's head into a wall, causing $S brains to ooze out of $S ears.", ch, NULL, victim, TO_NOTVICT );
    }   
    else if ( !str_cmp( arg2, "jd" ) )
    {
      act( AT_GREY, "You cut $N into small chunks with a chainsaw, then lick your fingers after eating $M piece by piece.", ch, NULL, victim, TO_CHAR );
      act( AT_GREY, "$n cuts you into small chunks with a chainsaw, then licks $s fingers after eating you piece by piece.", ch, NULL, victim, TO_VICT );
      act( AT_GREY, "$n cuts $N into small chunks with a chainsaw, then licks $s fingers after eating $M piece by piece.", ch, NULL, victim, TO_NOTVICT );
    }
    else if ( !str_cmp( arg2, "peace" ) && get_trust(ch) >= LEVEL_ASCENDANT )
    {
      act( AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, victim, TO_ROOM );
      act( AT_BLOOD, "The sheer volume of $n's voice causes a massive hemorage in $N's head!", ch, NULL, victim, TO_NOTVICT );
      act( AT_BLOOD, "The sheer volume of $n's voice causes a massive hemorage in your head!", ch, NULL, victim, TO_VICT );
    }
    else if ( !str_cmp( arg2, "slow" ) && get_trust(ch) >= LEVEL_ASCENDANT )
    {
      act( AT_BLOOD, "You calmly slit $N's throat. He colapses, not yet dead.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "Everything fades to black...", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n calmly slits $N's throat. He colapses, bleeding to death.", ch, NULL, victim, TO_NOTVICT );
      victim->hit -= ( victim->hit + 400 );
      update_pos( victim );
      return;
    }
    else
    {
      act( AT_IMMORT, "You slay $N in cold blood!",  ch, NULL, victim, TO_CHAR    );
      act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
      act( AT_IMMORT, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    }

    if ( !IS_NPC( victim ) )
    {
        sprintf( buf, "&w&z[&COOC Information&z] &R- %s has been slain!", victim->name );
        echo_to_all ( AT_RED , buf, 0 );
    }

    set_cur_char(victim);
    raw_kill( ch, victim );
    return;
}

void add_xname( char *arg){
  struct xname_data *xname;
  time_t xtime;

  CREATE(xname,struct xname_data,1);
  strcpy(xname->name,arg); 
  xtime = time(0);
  xtime += 10080;
  xname->time = xtime;
  xname->next = xnames;
};

void do_bite( CHAR_DATA *ch, char *argument )
{
}

void do_claw( CHAR_DATA *ch, char *argument )
{
}

void do_sting( CHAR_DATA *ch, char *argument )
{
}

void do_tail( CHAR_DATA *ch, char *argument )
{
}

void miss_message( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int at )
{
    char buf1[256], buf2[256], buf3[256];

    if ( dt == TYPE_HIT || dt == 0 )
    {
        sprintf( buf1, "$n's attack misses $N." );
        sprintf( buf2, "Your attack misses $N." );
        sprintf( buf3, "$n's attack misses you." );
    }
    else
    {
        if ( dt < 0 )
        {
           sprintf( buf1, "$n's %s misses $N.", ( dt == -2 ) ? "automatic fire" : "burst-fire" );
           sprintf( buf2, "Your %s misses $N.", ( dt == -2 ) ? "automatic fire" : "burst-fire" );
           sprintf( buf3, "$n's %s misses you.", ( dt == -2 ) ? "automatic fire" : "burst-fire" );
        }
        else if ( dt == TYPE_HIT + WEAPON_KNIFE || dt == TYPE_HIT + WEAPON_BLADE )
        {
           sprintf( buf1, "$n's slash misses $N." );
           sprintf( buf2, "Your slash misses $N." );
           sprintf( buf3, "$n's slash misses you." );
        }
        else if ( dt == TYPE_HIT + WEAPON_DISC )
        {
           sprintf( buf1, "$n's disc misses $N." );
           sprintf( buf2, "Your disc misses $N." );
           sprintf( buf3, "$n's disc misses you." );
        }
        else if ( dt == TYPE_HIT + WEAPON_SPEAR )
        {
           sprintf( buf1, "$n's spear misses $N." );
           sprintf( buf2, "Your spear misses $N." );
           sprintf( buf3, "$n's spear misses you." );
        }
        else if ( dt == TYPE_HIT + WEAPON_NATURAL )
        {
           sprintf( buf1, "$n's attack misses $N." );
           sprintf( buf2, "Your attack misses $N." );
           sprintf( buf3, "$n's attack misses you." );
        }
        else 
        {
           sprintf( buf1, "$n's shot misses $N." );
           sprintf( buf2, "Your shot misses $N." );
           sprintf( buf3, "$n's shot misses you." );
        }            
    }

    at -= TYPE_GENERIC;
    if ( at < 0 || at > 4 ) at = 2;

    sprintf( buf2, "%s (0 damage / %s)", buf2, ris_flags[at] );
    
    act( AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT );
    act( AT_YELLOW, buf2, ch, NULL, victim, TO_CHAR );
    act( AT_HITME, buf3, ch, NULL, victim, TO_VICT );

    return;
}

void radius_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int exp;
    int range;          

    // if ( IS_NPC(ch) )
    //    return;

    /*
    if ( IS_NPC( ch ) )
    {
       if ( ch->master )
       {
          ch = ch->master;
       }
       else
       {
          return;
       }
    }
    */

    if ( victim == ch )
    {
        /*
        exp = get_exp_worth( victim ) / 2;
        ch_printf( victim, "&YYou lost %d experience for comitting suicide. Idiot.\n\r", exp );
        victim->currexp = UMAX( 0, victim->currexp - exp );
        victim->maxexp = UMAX( 0, victim->maxexp - exp );
        */
        return;
    }
    if ( !is_enemy( ch, victim ) )
    {
        exp = get_exp_worth( victim ) * 2;
        ch_printf( ch, "&YYou lost %d experience for killing a teammate. Moron.\n\r", exp );
        ch->teamkill = UMAX( 0, ch->teamkill - 1 );

        if ( ch->teamkill <= 0 && !xIS_SET( ch->act, PLR_FREEZE ) && !IS_IMMORTAL( ch ) )
        {
           /* Failsafe - Multiable kills results in penalities */
           ch_printf( ch, "&w&R[Failsafe Trigger]: You are now frozen. Won't do that again, huh.\n\r" );
           xSET_BIT( ch->act, PLR_FREEZE );
        }

        ch->currexp = UMAX( 0, ch->currexp - exp );
        ch->maxexp = UMAX( 0, ch->maxexp - exp );

        return;
    }

    /* Starting XP and maximum gain range */
    exp = get_exp_worth( victim );
    if ( ch->race == RACE_PREDATOR )
    {
        if ( victim->in_room == ch->in_room ) exp *= 2;
        if ( IS_AFFECTED( ch, AFF_CLOAK ) && victim->race != RACE_ALIEN ) exp /= 4;
    }
    range = 5;

    xp_radius_2( ch->in_room, range );
    xp_radius_1( ch, ch->in_room, victim, exp, range );
    xp_radius_2( ch->in_room, range );

    return;
}

void xp_radius_1( CHAR_DATA *ch, ROOM_INDEX_DATA *room, CHAR_DATA * victim, int xp, int range )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    bool same=FALSE;

    if ( xIS_SET( room->room_flags, BFS_MARK ) )   return;

    xSET_BIT( room->room_flags , BFS_MARK );

    for ( gch = room->first_person; gch; gch = gch->next_in_room )
    {
        int exp = xp;

        if ( is_enemy( ch, gch ) || victim == gch )
	    continue;   

        /* Multiplaying Penalty */
        if ( gch != ch )
        { 
           DESCRIPTOR_DATA *dA;
           DESCRIPTOR_DATA *dB;

           if ( ( dA = ch->desc ) != NULL )
           {
              if ( ( dB = gch->desc ) != NULL )
              {
                 if ( !str_cmp( dA->host, dB->host ) ) continue;               
              }
           }
        }

        if ( gch == ch )
        {
            if ( is_spectator( ch ) )
            {
                exp = 0;
                // sprintf( buf, "&w&YYou earned %d experience for the last-ditch death of a %s. (+50%% bonus)\n\r", exp, race_table[victim->race].race_name );
            }
            else if ( ch->race == RACE_PREDATOR )
            {
                /* Work in HONOR Bonus */
                sprintf( buf, "&w&YYou earned %d experience for the death of a %s.\n\r", exp, race_table[victim->race].race_name );
            }
            else
            {
                sprintf( buf, "&w&YYou earned %d experience for the death of a %s.\n\r", exp, race_table[victim->race].race_name );
            }
	}
        else if ( !is_spectator( gch )  )
        {
            sprintf( buf, "&w&YYou earned %d experience for the death of a %s. (Thanks to %s)\n\r", exp, race_table[victim->race].race_name, ch->name );
        }
        else
        {
            continue;
        }

        send_to_char( buf, gch );

        gain_exp( gch, exp );
    }

    /* Branch out */
    {
       EXIT_DATA *pexit;                               
       for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       {
          if ( pexit->to_room && pexit->to_room != room )
          { 
             if ( range > 0 )
             {
                xp_radius_1( ch, pexit->to_room, victim, xp/2, range-1 );
             }
          }
       }
    }

    return;
}

void xp_radius_2( ROOM_INDEX_DATA *room, int range )
{

    if ( !xIS_SET( room->room_flags, BFS_MARK ) ) return;
         
    xREMOVE_BIT( room->room_flags , BFS_MARK );
    	                        
    if ( range > 0 ) 
    {
       EXIT_DATA *pexit;
                             
       for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       {
          if ( pexit->to_room && pexit->to_room != room ) xp_radius_2( pexit->to_room, range-1 );
       }
    }

    return;
}


void radius_rescue( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int range;
    int exp;

    /* Starting XP and maximum gain range */
    exp = get_exp_worth( victim ) * 2;
    range = 5;

    res_radius_2( ch->in_room, range );
    res_radius_1( ch, ch->in_room, victim, exp, range );
    res_radius_2( ch->in_room, range );

    return;
}

void res_radius_1( CHAR_DATA *ch, ROOM_INDEX_DATA *room, CHAR_DATA * victim, int xp, int range )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    bool same=FALSE;

    if ( xIS_SET( room->room_flags, BFS_MARK ) )   return;

    xSET_BIT( room->room_flags , BFS_MARK );

    for ( gch = room->first_person; gch; gch = gch->next_in_room )
    {
        int exp = xp;

        if ( gch->race != ch->race || victim == gch )
	    continue;

        if ( gch == ch )
        {
            sprintf( buf, "&w&YYou earned %d experience for completing a rescue.\n\r", exp );
	}
        else if ( !is_spectator( gch )  )
        {
            sprintf( buf, "&w&YYou earned %d experience for a rescue. (Thanks to %s)\n\r", exp, ch->name );
        }
        else
        {
            continue;
        }

        send_to_char( buf, gch );

        gain_exp( gch, exp );
    }

    /* Branch out */
    {
       EXIT_DATA *pexit;                               
       for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       {
          if ( pexit->to_room && pexit->to_room != room )
          { 
             if ( range > 0 )
             {
                res_radius_1( ch, pexit->to_room, victim, xp/2, range-1 );
             }
          }
       }
    }

    return;
}

void res_radius_2( ROOM_INDEX_DATA *room, int range )
{

    if ( !xIS_SET( room->room_flags, BFS_MARK ) ) return;
         
    xREMOVE_BIT( room->room_flags , BFS_MARK );
    	                        
    if ( range > 0 ) 
    {
       EXIT_DATA *pexit;
                             
       for ( pexit = room->first_exit; pexit; pexit = pexit->next )
       {
          if ( pexit->to_room && pexit->to_room != room ) res_radius_2( pexit->to_room, range-1 );
       }
    }

    return;
}

bool delete_player( char * name )
{
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if ( !name || name == NULL ) return;

    strcpy( arg, name );

    sprintf( buf, "Delete_player - Deleting character %s.", arg );
    log_string( buf );   

    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), capitalize( arg ) );
    sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower(arg[0]), capitalize( arg ) );
   
    return rename( buf, buf2 );
}

bool mob_reload( CHAR_DATA * ch, OBJ_DATA * weapon )
{
    OBJ_DATA * ammo;

    if ( ch == NULL ) return FALSE;
    if ( weapon == NULL ) return FALSE;

    if ( !IS_NPC( ch ) ) return FALSE;

    if ( weapon->ammo != NULL )
    {
        if ( weapon->ammo->value[2] > 0 ) return FALSE;
    }

    for ( ammo = ch->first_carrying; ammo; ammo = ammo->next_content )
    {
        if ( ammo == NULL ) continue;
        if ( ammo->item_type != ITEM_AMMO ) continue;

        if ( ammo->value[2] <= 0 ) continue;

        if ( ammo->value[3] == weapon->value[1] || ammo->value[4] == weapon->value[1] || ammo->value[5] == weapon->value[1] )
        {
            if ( weapon->ammo )
            {
                 act( AT_PLAIN, "$n removes $p from a weapon.", ch, weapon->ammo, NULL, TO_ROOM );        
                 obj_to_char( weapon->ammo, ch );
                 weapon->ammo->timer = 1;
                 weapon->ammo = NULL;
            }

            separate_obj(ammo);
            obj_from_char(ammo);

            act( AT_PLAIN, "$n reloads a $p.", ch, weapon, NULL, TO_ROOM );
            weapon->ammo = ammo;

            mob_weapon_set( ch );

            return TRUE;
        }
    }

    return FALSE;
}

int apply_armor_skill( CHAR_DATA * ch, OBJ_DATA * obj, int odam )
{
    char buf[MAX_STRING_LENGTH];
    int tmp = 0;
    int pr = 0;

    if ( !ch ) return odam;
    if ( !obj ) return odam;
    if ( odam <= 0 ) return 0;
    if ( IS_NPC( ch ) ) return odam;

    tmp = odam;

    if ( obj->item_type == ITEM_ARMOR )
    {
        switch( obj->value[3] )
        {
           case 1: pr = ch->pcdata->learned[gsn_lightarmor]; break;
           case 2: pr = ch->pcdata->learned[gsn_mediumarmor]; break;
           case 3: pr = ch->pcdata->learned[gsn_heavyarmor]; break;
           default:
            sprintf( buf, "apply_armor_skill: armor item %d does not have v3 set.", obj->pIndexData->vnum );
            log_string( buf );
            break;
        }

        if ( pr > 0 )
        {
           tmp = (int)( ( (float)(tmp) * (float)( (int)(100) - (int)( (int)(20) * (int)(pr) ) )  ) / (float)(100) );
        }
    }

    return UMAX( tmp, 1 );
}

/*
 * Returns a characters current armor in several formats.
 *  Modes:
 *   1 - Current Armor Health (Percent)
 *   2 - Max Armor Health (Units)
 */
int armor_status( CHAR_DATA * ch, int mode )
{
    OBJ_DATA * obj;
    int max = 0;
    int cur = 0;
    int perc = 0;

    if ( ch == NULL ) return -1;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
    {           
       if ( obj->item_type != ITEM_ARMOR ) continue;
       if ( obj->wear_loc < 0 || obj->wear_loc > MAX_WEAR ) continue;

       cur += obj->value[0];
       max += obj->value[1];
    }

    if ( max <= 0 ) return 0;

    if ( mode == 1 )
    {
       perc = (int)( ((float)(cur)/(float)(max)) * (float)(100) );
       perc = URANGE( 0, perc, 100 );
    }
    else if ( mode == 2 )
    {
       perc = max;
    }

    return perc;
}

bool is_ranged( int type )
{
    switch( type )
    {
       case WEAPON_KNIFE:        return FALSE; break;
       case WEAPON_PISTOL:       return TRUE; break;
       case WEAPON_RIFLE:        return TRUE; break;
       case WEAPON_SHOTGUN:      return TRUE; break;
       case WEAPON_AUTOMATIC:    return TRUE; break;
       case WEAPON_FLAMETHROWER: return TRUE; break;
       case WEAPON_ROCKETFIRED:  return TRUE; break;
       case WEAPON_BLADE:        return FALSE; break;
       case WEAPON_SPEAR:        return FALSE; break;
       case WEAPON_ERANGED:      return TRUE; break;
       case WEAPON_RANGED:       return TRUE; break;
       case WEAPON_DISC:         return TRUE; break;
       case WEAPON_NATURAL:      return FALSE; break;
       default:
          bug( "is_ranged: invalid weapon type %d.", type );
          break;
    }
    return FALSE;
}

bool has_ammo( int type )
{
    switch( type )
    {
       case WEAPON_KNIFE:        return FALSE; break;
       case WEAPON_PISTOL:       return TRUE; break;
       case WEAPON_RIFLE:        return TRUE; break;
       case WEAPON_SHOTGUN:      return TRUE; break;
       case WEAPON_AUTOMATIC:    return TRUE; break;
       case WEAPON_FLAMETHROWER: return TRUE; break;
       case WEAPON_ROCKETFIRED:  return TRUE; break;
       case WEAPON_BLADE:        return FALSE; break;
       case WEAPON_SPEAR:        return FALSE; break;
       case WEAPON_ERANGED:      return FALSE; break;
       case WEAPON_RANGED:       return TRUE; break;
       case WEAPON_DISC:         return FALSE; break;
       case WEAPON_NATURAL:      return FALSE; break;
       default:
          bug( "is_ranged: invalid weapon type %d.", type );
          break;
    }
    return FALSE;
}

int default_weapon_mode( OBJ_DATA * obj )
{
    if ( !obj ) return 0;

    if ( xIS_SET( obj->extra_flags, ITEM_SINGLEFIRE ) ) return MODE_SINGLE;
    if ( xIS_SET( obj->extra_flags, ITEM_SEMIAUTO ) )   return MODE_SEMIAUTO;
    if ( xIS_SET( obj->extra_flags, ITEM_BURSTFIRE ) )  return MODE_BURST;
    if ( xIS_SET( obj->extra_flags, ITEM_AUTOFIRE ) )   return MODE_AUTOMATIC;

    return 0;
}

void do_spray( CHAR_DATA * ch, char * argument )
{
    char arg[MAX_INPUT_LENGTH];
    EXIT_DATA * pexit;
    OBJ_DATA  * wieldA;
    OBJ_DATA  * wieldB;
    int ret = 0;
    int dir = 0;

    argument = one_argument( argument, arg );

    if ( ch->race != RACE_MARINE )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( is_spectator( ch ) ) { send_to_char( "&RYou may not engage in combat while in spectator mode.\n\r", ch ); return; }

    if ( arg[0] == '\0' || ( dir = get_door( arg ) ) == -1 )
    {
        send_to_char( "&RSyntax: SPRAY <direction>\n\r", ch );
        return;
    }

    wieldA = get_eq_char( ch, WEAR_WIELD );
    wieldB = get_eq_char( ch, WEAR_DUAL_WIELD );

    if ( !wieldA && !wieldB )
    {
        send_to_char( "You don't seem to be wielding any weapons.\n\r", ch );
        return;
    }

    if ( ch->ap < get_max_ap( ch ) )
    {
        send_to_char( "&RPlease wait for the attack gauge to reset.\n\r", ch );
        return;
    }

    if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
    {
        send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
        return;
    }

    if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
    {
        send_to_char( "Are you expecting to fire through a door!?\n\r", ch );
        return;
    }

    // Remove Hide
    if ( IS_AFFECTED(ch, AFF_HIDE) ) xREMOVE_BIT(ch->affected_by, AFF_HIDE);

    // Execute Spray
    if ( ret >= 0 ) ret += spray_direction( ch, wieldA, dir, 1 );
    if ( ret >= 0 ) ret += spray_direction( ch, wieldB, dir, 2 );

    if ( ret >= 0 ) ch->ap = 0;

    return;
}

int spray_direction( CHAR_DATA * ch, OBJ_DATA * wield, int dir, int dual )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA * to_room;
    CHAR_DATA * rch, * rnext;
    EXIT_DATA * pexit;
    int range=0, mrange=0;
    int rounds=0, mrounds=0;
    int chance=0, dam=0;
    int hits=0, misses=0;
    bool flame = FALSE;

    if(!ch) { 
        bug("spray_direction: ch is NULL", 0);
        return -1; 
    }

    if ( !wield )
    {
        if ( dual == 1 ) send_to_char( "&RYou dont seem to have a weapon!\n\r", ch );
        return -1;
    }
    if ( wield->item_type != ITEM_WEAPON )
    {
        if ( dual == 1 ) send_to_char( "&RThats... not a weapon.\n\r", ch );
        return -1;
    }
    if ( wield->value[0] == WEAPON_FLAMETHROWER ) flame = TRUE;
    if ( wield->value[0] == WEAPON_ROCKETFIRED )
    {
        send_to_char("&RThis weapon cannot spray. Try using FIRE.\n\r", ch );
        return -1;
    }
    if ( !is_ranged( wield->value[0] ) )
    {
        if ( dual == 1 ) send_to_char( "&RUse RANGED Weapons. RANGED.\n\r", ch );
        return -1;
    }
    if ( !wield->ammo )
    {
        if ( dual == 1 ) send_to_char( "&RLoad the gun first. Trust me on this.\n\r", ch );
        return -1;
    }
    if ( ( mrange = wield->value[2] ) <= 0 )
    {
        if ( dual == 1 ) send_to_char( "&RTry picking a gun with range, buddy.\n\r", ch );        
        return -1;
    }

    if ( wield->weapon_mode == MODE_BURST )
    { mrounds = wield->value[3]; }
    else if ( wield->weapon_mode == MODE_AUTOMATIC )
    { mrounds = wield->value[4]; }
    else if ( flame )
    { mrounds = 1; }
    else
    {
      if ( dual == 1 ) send_to_char( "&RUse SETMODE to select either BURST or AUTOMATIC mode first.\n\r", ch );
      return -1;
    }

    mrounds = URANGE( 0, mrounds, wield->ammo->value[2] );

    if ( ( rounds = mrounds ) <= 0 ) return -1;

    wield->ammo->value[2] -= rounds;

    // Firing Message
    if ( !flame )
    {
      ch_printf( ch, "&RYou scream and open fire to %s. &z(&W%d rounds&z)\n\r", main_exit(dir), rounds );
      sprintf( buf, "$n screams and opens fire %s." , dir_name[dir] );
      act( AT_BLOOD, buf, ch, NULL, NULL, TO_ROOM );          
    }
    else
    {
      ch_printf( ch, "&RYou fire a huge burst of flame to %s.\n\r", main_exit(dir) );
      sprintf( buf, "$n fires a huge burst of flame %s." , dir_name[dir] );
      act( AT_BLOOD, buf, ch, NULL, NULL, TO_ROOM );          
    }
    weapon_echo( ch, wield );

    pexit = get_exit( ch->in_room, dir );

    if ( rounds > 10 && !IS_OBJ_STAT(wield, ITEM_FULLHIT) )
    {
       // 25% of the rounds go nowhere when firing more than 10 rounds.
       rounds = (int)((float)((float)(rounds) / (float)(100)) * (float)(75));
    }

    for ( range = 1; range <= mrange; range++ )
    {
       if ( !pexit ) break;
       if ( xIS_SET( pexit->exit_info, EX_CLOSED ) ) break;
       if ( !pexit->to_room ) break;    

       to_room = NULL;
       if ( pexit->distance > 1 ) to_room = generate_exit( ch->in_room , &pexit );
    
       if ( to_room == NULL ) to_room = pexit->to_room;
    
       // Scan for targets
       for ( rch = to_room->first_person; rch; rch = rnext )
       {
           rnext = rch->next_in_room;
           if ( is_spectator( rch ) || IN_VENT( rch ) ) continue;

           /* Regular weapons */
           if ( !flame )
           {
              chance = number_range( (int)((float)(rounds)/(float)(4)), (int)((float)(rounds)/(float)(3)) );
              if ( IS_OBJ_STAT( wield, ITEM_FULLHIT ) ) chance = number_range( (int)((float)(rounds)/(float)(2)), rounds );
              chance = URANGE( -1, chance, rounds );

              if ( !can_see( ch, rch ) && !IS_OBJ_STAT(wield, ITEM_FULLHIT) ) chance = (int)( (float)(rounds) / (float)(2) );

              if ( rch->position <= POS_PRONE )  chance = 0;
           }
           /* Flamethrowers */
           else
           {
              chance = 100;
           }

           if ( rch->pcdata && rch->hit > 0 )
           {
              int dodge, friend, primal;

              dodge = rch->pcdata->learned[gsn_dodge];
              friend = rch->pcdata->learned[gsn_friendly_fire];
              primal = rch->pcdata->learned[gsn_primal_instinct];

              if ( !is_enemy( ch, rch ) ) chance -= ( 25 * friend );

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
              if ( !flame )
              {
                 dam = ( chance * wield->ammo->value[1] );
                 rounds -= chance;
                 ch_printf( rch, "&rIncoming rounds from %s hit you! &z(&W%d round%s&z)\n\r", rev_exit( dir ), chance, (chance > 1) ? "s" : "" );
              }
              else
              {
                 dam = (int)((float)( rounds * wield->ammo->value[1] ) / (float)(2));
                 ch_printf( rch, "&rAn Incoming stream of fire from %s hits you!\n\r", rev_exit( dir ) );
              }

              if ( wield->attach )
               if ( wield->attach->value[0] == 4 )
                 dam = (int)((float)((float)(dam)/(float)(100)) * (float)(100 + wield->attach->value[2]));

              if ( ( dam = cdamage( ch, rch, dam, TRUE ) ) >-1 )
              {
                 damage( ch, rch, dam, TYPE_GENERIC + wield->ammo->value[0] );
                 if ( flame ) ignite_target( ch, rch, number_range( 5, 10 ) );
              }
              if ( !char_died( rch ) )
                 ch_printf( ch, "&rYou hear a scream from %s.\n\r", main_exit( dir ) );
           }
           else
           {
              misses++;
              if ( !flame )
                ch_printf( rch, "&rIncoming rounds from %s barely miss you!\n\r", rev_exit( dir ) );
              else
                ch_printf( rch, "&rA burst of flame from %s barely misses you!\n\r", rev_exit( dir ) );
           }
       }

       if ( rounds <= 0 )
       {
           if ( hits > 0 ) { }
           else if ( misses > 0 ) { ch_printf( ch, "&rShit, missed by just a few inches...\n\r" ); }
           else { ch_printf( ch, "&rDamn. Doesn't look like you hit anything.\n\r" ); }

           auto_eject( ch, wield ); // Check for clip ejection
           
           showammo_option( ch, wield ); // ShowAmmo Notification

           return 1;
       }

       if ( ( pexit = get_exit( to_room, dir ) ) == NULL ) break;
    }

    auto_eject( ch, wield ); // Check for clip ejection

    showammo_option( ch, wield ); // ShowAmmo Notification

    if ( hits > 0 ) { }
    else if ( misses > 0 ) { ch_printf( ch, "&rShit, missed by just a few inches...\n\r" ); }
    else { ch_printf( ch, "&rDamn. Doesn't look like you hit anything.\n\r" ); }

    return 1;
}

int total_kills( CHAR_DATA * ch )
{
    int total = 0;
    int tmp = 0;

    if ( !ch ) return 0;
    if ( !ch->pcdata ) return 0;

    for( tmp = 0; tmp <= 2; tmp++ )
    {
       total += ch->pcdata->kills.pc[tmp];
       total += ch->pcdata->kills.bot[tmp];
       total += ch->pcdata->kills.npc[tmp];
    }

    return total;
}

int total_killed( CHAR_DATA * ch )
{
    int total = 0;
    int tmp = 0;

    if ( !ch ) return 0;
    if ( !ch->pcdata ) return 0;

    for( tmp = 0; tmp <= 2; tmp++ )
    {
       total += ch->pcdata->killed.pc[tmp];
       total += ch->pcdata->killed.bot[tmp];
       total += ch->pcdata->killed.npc[tmp];
    }

    return total;
}

int total_pc_kills( CHAR_DATA * ch )
{
    int total = 0;
    int tmp = 0;

    if ( !ch ) return 0;
    if ( !ch->pcdata ) return 0;

    for( tmp = 0; tmp <= 2; tmp++ )
    {
       total += ch->pcdata->kills.pc[tmp];
       total += ch->pcdata->kills.bot[tmp];
    }

    return total;
}

int total_pc_killed( CHAR_DATA * ch )
{
    int total = 0;
    int tmp = 0;

    if ( !ch ) return 0;
    if ( !ch->pcdata ) return 0;

    for( tmp = 0; tmp <= 2; tmp++ )
    {
       total += ch->pcdata->killed.pc[tmp];
       total += ch->pcdata->killed.bot[tmp];
    }

    return total;
}

int stamina_penalty( CHAR_DATA * ch, int chance )
{
    int sc;
    int perc;
    int half;

    if ( chance <= 0 ) return chance;

    /* Perception Bonus */
    chance += get_curr_per(ch);

    /* Position Bonus */
    if ( ch->position == POS_KNEELING ) chance += 10;
    if ( ch->position == POS_PRONE ) chance += 20;

    /*
     * Hive penality
     */
    if ( ch->race == RACE_MARINE && xIS_SET(ch->in_room->room_flags, ROOM_HIVED) )
      chance -= 10;

    if ( ch->move < ch->max_move )
    {
       sc = chance;

       perc = ( ch->move * 100 ) / ch->max_move;

       half = ( chance / 2 );

       chance -= half;

       chance = (int)( (float)(chance) * ( (float)(perc) / (float)(100) ) );

       chance += half;

       return chance;
    }
    else
    {
       return chance;
    }
}

void auto_eject( CHAR_DATA * ch, OBJ_DATA * wield )
{
    if ( wield == NULL ) return;
    if ( wield->ammo == NULL ) return;

    if ( wield->ammo->value[2] <= 0 )
    {
        char bufA[MAX_STRING_LENGTH];
        char bufB[MAX_STRING_LENGTH];
       
        /* Eject empty clips - Saves time */
        if ( wield->value[0] == WEAPON_SHOTGUN && wield->item_type == ITEM_WEAPON )
        {
            sprintf( bufA, "&C%s ejects the spent rounds, which land on the ground.\n\r", wield->short_descr );
            sprintf( bufB, "&C$n's %s ejects the spent rounds.", wield->short_descr );
        }
        else
        {
            sprintf( bufA, "&C%s ejects the spent clip, which lands on the ground.\n\r", wield->short_descr );
            sprintf( bufB, "&C$n's %s ejects a spent clip.", wield->short_descr );
        }
        ch_printf( ch, bufA );
        act( AT_CYAN, bufB, ch, wield, NULL, TO_ROOM );
        wield->ammo->timer = 2;
        obj_to_room( wield->ammo, ch->in_room );
        wield->ammo = NULL;
        mob_reload( ch, wield );
    }

    return;
}

void do_snipe( CHAR_DATA * ch, char * argument )
{
    char arg[MAX_INPUT_LENGTH];
    EXIT_DATA * pexit;
    OBJ_DATA  * wieldA;
    OBJ_DATA  * wieldB;
    int ret = 0;
    int dir = 0;

    argument = one_argument( argument, arg );

    if ( ch->race == RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( is_spectator( ch ) ) { send_to_char( "&RYou may not engage in combat while in spectator mode.\n\r", ch ); return; }

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    {
        send_to_char( "&RYou can't even SEE while blind, let alone shoot!\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' || ( dir = get_door( arg ) ) == -1 )
    {
        send_to_char( "&RSyntax: SNIPE <direction> <target>\n\r", ch );
        return;
    }

    wieldA = get_eq_char( ch, WEAR_WIELD );
    wieldB = get_eq_char( ch, WEAR_DUAL_WIELD );

    if ( !wieldA && !wieldB )
    {
        send_to_char( "You don't seem to be wielding any weapons.\n\r", ch );
        return;
    }

    if ( ch->ap < get_max_ap( ch ) )
    {
        send_to_char( "&RPlease wait for the attack gauge to reset.\n\r", ch );
        return;
    }

    if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
    {
        send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
        return;
    }

    if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
    {
        send_to_char( "Are you expecting to fire through a door!?\n\r", ch );
        return;
    }

    if ( ret >= 0 )
    {
        ret = snipe_direction( ch, NULL, argument, wieldA, dir, 1 );
        if ( ret == 1 ) ch->ap = 0;
        if ( ret == 2 ) ch->ap -= 1;
    }
    if ( ret >= 0 )
    {
        ret = snipe_direction( ch, NULL, argument, wieldB, dir, 2 );
        if ( ret == 1 ) ch->ap = 0;
        if ( ret == 2 ) ch->ap -= 1;
    }

    // Sniping Lag
    WAIT_STATE( ch, 8 );

    return;
}

/*
 * Must specify either VIC or ARG (Victim name)
 * VIC automaticly overrides the contents of ARG
 */
int snipe_direction( CHAR_DATA * ch, CHAR_DATA * vic, char * arg, OBJ_DATA * wield, int dir, int dual )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA * to_room;
    CHAR_DATA * rch, * rnext;
    EXIT_DATA * pexit;
    int range=0, mrange=0;
    int rounds=0, mrounds=0;
    int chance=0, dam=0;
    int accuracy=110, drange=0;
    bool semi=FALSE, found=FALSE, cansee=FALSE;

    if ( !wield )
    {
        if ( dual == 1 ) send_to_char( "&RYou dont seem to have a weapon!\n\r", ch );
        return -1;
    }
    if ( wield->item_type != ITEM_WEAPON )
    {
        if ( dual == 1 ) send_to_char( "&RThats... not a weapon.\n\r", ch );
        return -1;
    }
    if ( !is_ranged( wield->value[0] ) )
    {
        if ( dual == 1 ) send_to_char( "&RUse RANGED Weapons. RANGED.\n\r", ch );
        return -1;
    }
    if ( wield->value[0] == WEAPON_FLAMETHROWER )
    {
        send_to_char("&RYou cannot snipe with a flamethrower. Try SPRAY.\n\r", ch );
        return -1;
    }
    if ( wield->value[0] == WEAPON_ROCKETFIRED )
    {
        send_to_char("&RThis weapon cannot snipe. Try using FIRE.\n\r", ch );
        return -1;
    }
    if ( !wield->ammo && wield->value[0] != WEAPON_ERANGED )
    {
        if ( dual == 1 ) send_to_char( "&RLoad the gun first. Trust me on this.\n\r", ch );
        return -1;
    }
    if ( wield->value[0] == WEAPON_ERANGED && ch->field < wield->value[1] )
    {
        if ( dual == 1 ) send_to_char( "&RNot enough field charge to fire this weapon.\n\r", ch );
        return -1;
    }
    if ( ( mrange = wield->value[2] ) <= 0 )
    {
        if ( dual == 1 ) send_to_char( "&RTry picking a gun with range, buddy.\n\r", ch );        
        return -1;
    }

    drange = get_dark_range( ch );
    pexit = get_exit( ch->in_room, dir );

    for ( range = 1; range <= mrange; range++ )
    {
       if ( !pexit ) break;
       if ( xIS_SET( pexit->exit_info, EX_CLOSED ) ) break;
       if ( !pexit->to_room ) break;    

       to_room = NULL;
       if ( pexit->distance > 1 ) to_room = generate_exit( ch->in_room , &pexit );
    
       if ( to_room == NULL ) to_room = pexit->to_room;

       found = FALSE;
    
       cansee = TRUE;

       if ( room_is_dark( ch, to_room ) ) { if ( --drange < 0 ) cansee = FALSE; }

       if ( vic )
       {
           if ( vic->in_room == to_room ) { rch = vic; found = TRUE; }
       }
       else
       {
           if ( ( rch = get_char_far_room( ch, to_room, arg ) ) != NULL ) found = TRUE;
       }

       // Target located
       if ( found )
       {
           if ( !is_spectator( rch ) && cansee && can_see( ch, rch ) && !IN_VENT( rch ) ) { }
           else { found = FALSE; }

           break;
       }

       if ( ( pexit = get_exit( to_room, dir ) ) == NULL ) break;
    }

    if ( !found )
    {
        if ( dual == 1 ) send_to_char( "&RYou fail to sight the target, it may have moved.\n\r", ch );
        return -1;
    }

    chance += weapon_accuracy[ (wield->value[0]) ];

    /* Factor in the weapon's accuracy attachment */
    if ( wield->attach )
    {
       if ( wield->attach->value[0] == 2 )
       {
          accuracy += wield->attach->value[3];
       }
    }

    if ( wield->weapon_mode == MODE_SINGLE )
    {
        mrounds = 1;
        accuracy += 0;
    }
    else if ( wield->weapon_mode == MODE_BURST )
    {
        mrounds = wield->value[3];
        accuracy -= 10;
    }
    else if ( wield->weapon_mode == MODE_SEMIAUTO )
    {
        semi = TRUE;
        mrounds = 1;
        accuracy -= 5;
    }
    else if ( wield->weapon_mode == MODE_AUTOMATIC )
    {
        mrounds = wield->value[4];
        accuracy -= 20;
    }
    else { return -1; }

    accuracy = stamina_penalty( ch, accuracy );

    if ( wield->ammo )
      mrounds = URANGE( 0, mrounds, wield->ammo->value[2] );
    else
      mrounds = URANGE( 0, mrounds, (int)((float)(ch->field)/(float)(wield->value[1])) );

    if ( ( rounds = mrounds ) <= 0 ) return -1;

    if ( wield->value[0] == WEAPON_ERANGED )
    {
        ch->field = UMAX(0, ch->field - (wield->value[1] * mrounds));
    }
    else
    {
        if ( wield->ammo ) wield->ammo->value[2] -= rounds;
    }

    // Firing Message
    ch_printf( ch, "&RYou draw a bead and open fire to %s. &z(&W%d round%s&z)\n\r", dir_name[dir], rounds, (rounds > 1 ? "s" : "" ) );
    sprintf( buf, "$n takes aim and opens fire %s." , dir_name[dir] );
    act( AT_BLOOD, buf, ch, NULL, NULL, TO_ROOM );          

    weapon_echo( ch, wield );

    drange = get_dark_range( ch );

    pexit = get_exit( ch->in_room, dir );

    for ( range = 1; range <= mrange; range++ )
    {
       if ( !pexit ) break;
       if ( xIS_SET( pexit->exit_info, EX_CLOSED ) ) break;
       if ( !pexit->to_room ) break;    

       accuracy -= 5;

       to_room = NULL;
       if ( pexit->distance > 1 ) to_room = generate_exit( ch->in_room , &pexit );
    
       if ( to_room == NULL ) to_room = pexit->to_room;

       found = FALSE;
    
       cansee = TRUE;

       if ( room_is_dark( ch, to_room ) )
       {
          if ( --drange < 0 ) cansee = FALSE;
       }

       if ( vic )
       {
           if ( vic->in_room == to_room ) { rch = vic; found = TRUE; }
       }
       else
       {
           if ( ( rch = get_char_far_room( ch, to_room, arg ) ) != NULL ) found = TRUE;
       }

       // Target located
       if ( found )
       {
           if ( !is_spectator( rch ) && cansee && can_see( ch, rch ) && !IN_VENT( rch ) )
           {
               if ( rch->pcdata && rch->hit > 0 )
               {
                   int dodge;
                   int primal;
        
                   dodge = rch->pcdata->learned[gsn_dodge];
                   primal = rch->pcdata->learned[gsn_primal_instinct];

                   if ( rch->pcdata->prepared[gsn_dodge] < skill_table[gsn_dodge]->reset ) dodge = 0;
                   if ( rch->pcdata->prepared[gsn_primal_instinct] < skill_table[gsn_primal_instinct]->reset ) primal = 0;

                   dodge *= 10;
                   primal *= 25;

                   if ( number_percent() < dodge )
                   {
                       ch_printf( rch, "&w&C(Dodge) You successfully dodged an attack.\n\r" );
                       rch->pcdata->prepared[gsn_dodge] = 0;
                       accuracy = 0;
                   }
                   if ( number_percent() < primal )
                   {
                       ch_printf( rch, "&w&C(Primal Instinct) You successfully dodged an attack.\n\r" );
                       rch->pcdata->prepared[gsn_primal_instinct] = 0;
                       accuracy = 0;
                   }
               }

               if ( accuracy > 0 ) accuracy = char_acc_modify( rch, accuracy );
               if ( number_percent( ) < accuracy )
               {
                  // Make some rounds miss with burst or automatic weapons.
                  rounds = number_range( (mrounds / 2), mrounds );
                  rounds = URANGE( 1, rounds, mrounds );

                  if ( wield->ammo )
                    dam = ( rounds * wield->ammo->value[1] );
                  else
                    dam = ( rounds * wield->value[4] );

                  if ( wield->attach )
                   if ( wield->attach->value[0] == 4 )
                     dam = (int)((float)((float)(dam)/(float)(100)) * (float)(100 + wield->attach->value[2]));

                  ch_printf( rch, "&rIncoming fire from %s hit you! &z(&W%d round%s&z)\n\r", rev_exit( dir ), rounds, (rounds > 1) ? "s" : "" );
                  if ( ( dam = cdamage( ch, rch, dam, TRUE ) ) > -1 )
                  {
                     if ( wield->ammo )
                       damage( ch, rch, dam, TYPE_GENERIC + wield->ammo->value[0] );
                     else
                       damage( ch, rch, dam, TYPE_GENERIC + RIS_ENERGY );
                  }
                  if ( !char_died( rch ) )
                     ch_printf( ch, "&rYou hear a scream from %s.\n\r", main_exit( dir ) );
               }
               else
               {
                  ch_printf( rch, "&rIncoming fire from %s barely misses you!\n\r", rev_exit( dir ) );
                  ch_printf( ch, "&rShit, missed by just a few inches...\n\r" );
               }

               auto_eject( ch, wield ); // Check for clip ejection

               showammo_option( ch, wield ); // ShowAmmo Notification

               return (semi) ? 2 : 1;
           }
           break;
       }

       if ( ( pexit = get_exit( to_room, dir ) ) == NULL ) break;
    }

    auto_eject( ch, wield ); // Check for clip ejection

    showammo_option( ch, wield ); // ShowAmmo Notification

    ch_printf( ch, "&rDamn. Doesn't look like you hit anything.\n\r" );

    return (semi) ? 2 : 1;
}

void do_lob( CHAR_DATA * ch, char * argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    EXIT_DATA * pexit;
    OBJ_DATA  * obj;
    int range, mrange;
    int dir = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ch->race == RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( is_spectator( ch ) ) { send_to_char( "&RYou may not engage in combat while in spectator mode.\n\r", ch ); return; }

    mrange = ( get_curr_str( ch ) / 5 );

    range = URANGE( 0, atoi( arg2 ), mrange );

    if ( mrange <= 0 )
    {
        send_to_char( "&RYour too weak to even lob!\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' || ( dir = get_door( arg1 ) ) == -1 || range <= 0 )
    {
        ch_printf( ch, "&RSyntax: LOB <direction> <range>\n\r" );
        ch_printf( ch, "&rMax range with &R%d &rstrength is &R%d &rrooms.\n\r", get_curr_str( ch ), mrange );
        return;
    }

    obj = get_eq_char( ch, WEAR_HOLD );

    if ( !obj )
    {
        send_to_char( "You don't seem to be holding anything.\n\r", ch );
        return;
    }

    if ( ch->move < ( range * 5 ) )
    {
        send_to_char( "&RYou dont have enough movement to do that.\n\r", ch );
        return;
    }

    if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
    {
        send_to_char( "Are you expecting to throw through a wall!?\n\r", ch );
        return;
    }

    if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
    {
        send_to_char( "Are you expecting to throw through a door!?\n\r", ch );
        return;
    }

    // Remove hide
    if ( IS_AFFECTED(ch, AFF_HIDE) ) xREMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( lob_direction( ch, obj, dir, range ) != 0 )
    {
        ch->move -= ( range * 5 );
    }       

    return;
}

int lob_direction( CHAR_DATA * ch, OBJ_DATA * obj, int dir, int range )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA * to_room;
    EXIT_DATA * pexit;
    int cnt;

    if ( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        if ( ch->hit >= ch->max_hit ) ch->hit = ch->max_hit - 1;        
        ch_printf( ch, "&RSmile. Your fucked now, buddy boy.\n\r" );
        do_emote( ch, "frowns" );
        return 0;
    }

    // Firing Message
    ch_printf( ch, "&RYou wind back and lob to %s.\n\r", main_exit(dir) );
    sprintf( buf, "$n winds back and lobs something %s." , dir_name[dir] );
    act( AT_BLOOD, buf, ch, NULL, NULL, TO_ROOM );

    unequip_char( ch, obj );
    separate_obj( obj );
    obj_from_char( obj );

    pexit = get_exit( ch->in_room, dir );

    for ( cnt = 1; ; cnt++ )
    {
       if ( !pexit )
       {
          sprintf( buf, "&r%s sails from %s and hits a wall." , obj->short_descr , rev_exit( dir ) );
          echo_to_room( -1, to_room, buf ); 
          break;
       }
       if ( !pexit->to_room )
       {
          sprintf( buf, "&r%s sails from %s and hits a wall." , obj->short_descr , rev_exit( dir ) );
          echo_to_room( -1, to_room, buf ); 
          break;
       }
       if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
       {
          sprintf( buf, "&r%s sails from %s and hits a door." , obj->short_descr , rev_exit( dir ) );
          echo_to_room( -1, to_room, buf ); 
          break;
       }

       to_room = NULL;
       if ( pexit->distance > 1 ) to_room = generate_exit( ch->in_room , &pexit );
    
       if ( to_room == NULL ) to_room = pexit->to_room;
    
       if ( !to_room ) break;

       if ( cnt >= range )
       {
           sprintf( buf, "&r%s sails from %s and lands on the floor." , obj->short_descr , rev_exit( dir ) );
           echo_to_room( -1, to_room, buf ); 
           break;         
       }
       else if ( ( pexit = get_exit( to_room, dir ) ) == NULL )
       {
           sprintf( buf, "&r%s sails from %s and hits a wall." , obj->short_descr , rev_exit( dir ) );
           echo_to_room( -1, to_room, buf ); 
           break;
       }
       else
       {
           sprintf( buf, "&r%s sails through the room. (%s to %s).", obj->short_descr, dir_name[flip_dir(dir)], dir_name[dir] );
           echo_to_room( -1, to_room, buf ); 
       }
    }

    if ( !to_room ) to_room = ch->in_room;

    obj = obj_to_room( obj, to_room );

    if ( !xIS_SET( to_room->room_flags, ROOM_PROTOTYPE ) )
    {
        if ( to_room->area )
        {
           motion_ping( to_room->x, to_room->y, to_room->z, to_room->area, NULL );
        }
    }

    return 1;
}

int char_acc_modify( CHAR_DATA * ch, int ac )
{
    if ( !ch ) return ac;
    if ( IS_NPC( ch ) || !ch->pcdata ) return ac;

    if ( ac <= 0 ) return ac;

    if ( ch->race == RACE_PREDATOR ) ac += 10;
    if ( ch->race == RACE_MARINE )   ac += 0;
    if ( ch->race == RACE_ALIEN )
    {
        ac += 0;

        ac -= ( ch->pcdata->learned[gsn_evasive] * 10 );
    }

    ac = URANGE( 0, ac, 100 );

    return ac;
}

void do_cover( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj = NULL;

    if ( is_spectator( ch ) ) { send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch ); return; }

    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: COVER <object | NONE>\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "none" ) )
    {
        if ( ch->cover )
        {
            act( AT_ACTION, "$n steps from behind a $p.", ch, ch->cover, NULL, TO_ROOM );
            act( AT_ACTION, "You step from behind a $p.", ch, ch->cover, NULL, TO_CHAR );
            remove_cover( ch, NULL );
            return;
        }
        else
        {
            ch_printf( ch, "&RMaybe if you had taken cover first.\n\r" );
            return;
        }
    }

    if ( ( obj = get_obj_list_rev( ch, arg, ch->in_room->last_content ) ) == NULL )
    {
        send_to_char("&RYou scan the room, but fail to locate it!\n\r", ch );
        return;
    }

    if ( obj->item_type == ITEM_COVER )
    {
        if ( ch->race == RACE_ALIEN && obj->value[0] < 3 )
        {
            ch_printf( ch, "&RTheres not enough room left behind that.\n\r" );
            return;
        }
        if ( ch->race == RACE_MARINE && obj->value[0] < 4 )
        {
            ch_printf( ch, "&RTheres not enough room left behind that.\n\r" );
            return;
        }
        if ( ch->race == RACE_PREDATOR && obj->value[0] < 5 )
        {
            ch_printf( ch, "&RTheres not enough room left behind that.\n\r" );
            return;
        }
        act( AT_ACTION, "$n ducks behind a $p.", ch, obj, NULL, TO_ROOM );
        act( AT_ACTION, "You duck behind a $p.", ch, obj, NULL, TO_CHAR );

        add_cover( ch, obj );
    }
    else
    {
        send_to_char( "&RYou can't take cover behind that!\n\r", ch );
        return;
    }

    return;
}

void remove_cover( CHAR_DATA * ch, OBJ_DATA * obj )
{
   if ( ch->cover == NULL ) return;

   if ( obj != NULL )
   {
        if ( ch->cover != obj ) return;
   }

   if ( obj_extracted( ch->cover ) )
   {
        ch->cover = NULL;
        return;
   }

   if ( ch->race == RACE_ALIEN ) ch->cover->value[0] += 3;
   if ( ch->race == RACE_MARINE ) ch->cover->value[0] += 4;
   if ( ch->race == RACE_PREDATOR ) ch->cover->value[0] += 5;

   ch->cover = NULL;
}

void add_cover( CHAR_DATA * ch, OBJ_DATA * obj )
{
   remove_cover( ch, NULL );

   if ( obj == NULL ) return;

   if ( obj_extracted( obj ) ) return;

   ch->cover = obj;

   if ( ch->race == RACE_ALIEN ) ch->cover->value[0] -= 3;
   if ( ch->race == RACE_MARINE ) ch->cover->value[0] -= 4;
   if ( ch->race == RACE_PREDATOR ) ch->cover->value[0] -= 5;
}

/*
 * Inflict damage from a hit on a cover object.
 */
int cdamage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, bool silent )
{
    char buf[MAX_STRING_LENGTH];
    int ndam = 0;

    if ( ch == NULL || victim == NULL ) return -1;

    if ( dam <= 0 ) return -1;

    if ( victim->cover == NULL ) return dam;

    if ( obj_extracted( victim->cover ) )
    {
         victim->cover = NULL;
         return dam;
    }

    ndam = dam - ( victim->cover->value[2] * 10 );

    if ( ndam <= 0 )
    {
         if ( !silent ) ch_printf( ch, "&YYour attack strikes a %s! (%d damage)\n\r", victim->cover->short_descr, dam );
         ch_printf( victim, "&YIncoming fire strikes a %s! (%d damage)\n\r", victim->cover->short_descr, dam );
         return -1;
    }
    else
    {
         if ( ( victim->cover->value[3] - ndam ) <= 0 )
         {
            dam -= ( ndam - victim->cover->value[3] );
            ndam = victim->cover->value[3];
            if ( !silent ) ch_printf( ch, "&RYour attack devastates a %s! (%d damage)\n\r", victim->cover->short_descr, ndam );
            ch_printf( victim, "&RIncoming fire devastates a %s! (%d damage)\n\r", victim->cover->short_descr, ndam );
            damage_obj( victim->cover, ndam );
            return dam;
         }
         else
         {
            if ( !silent ) ch_printf( ch, "&YYour attack strikes a %s! (%d damage)\n\r", victim->cover->short_descr, ndam );
            ch_printf( victim, "&YIncoming fire strikes a %s! (%d damage)\n\r", victim->cover->short_descr, ndam );
            damage_obj( victim->cover, ndam );
            return -1;
         }
    }

    return -1;
}

bool is_enemy( CHAR_DATA * ch, CHAR_DATA * victim )
{
    if ( (IS_NPC(ch) && xIS_SET( ch->act, ACT_HOSTAGE )) && victim->race == RACE_MARINE ) return FALSE;

    if ( (IS_NPC(ch) && xIS_SET( victim->act, ACT_HOSTAGE )) && ch->race == RACE_MARINE ) return FALSE;

    if ( ch->race != victim->race ) return TRUE;

    if ( IS_NPC( ch ) && IS_NPC( victim ) )
    {
        if ( xIS_SET( ch->act, ACT_AGGRESSIVE ) && xIS_SET( victim->act, ACT_AGGRESSIVE ) ) return FALSE;
    }

    if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_AGGRESSIVE ) ) return TRUE;

    if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_AGGRESSIVE ) ) return TRUE;

    return FALSE;
}

void mob_weapon_set( CHAR_DATA * ch )
{
    OBJ_DATA * objA;

    objA = get_eq_char( ch, WEAR_WIELD );
    if( objA && !( objA->item_type == ITEM_WEAPON ) )
        objA = NULL;

    if ( xIS_SET( objA->extra_flags, ITEM_AUTOFIRE ) )
    { do_setmode( ch, "full" ); }
    else if ( xIS_SET( objA->extra_flags, ITEM_BURSTFIRE ) )
    { do_setmode( ch, "burst" ); return; }
    else if ( xIS_SET( objA->extra_flags, ITEM_SEMIAUTO ) )
    { do_setmode( ch, "semi" ); return; }
    else if ( xIS_SET( objA->extra_flags, ITEM_SINGLEFIRE ) )
    { do_setmode( ch, "single" ); return; }

    return;
}

void weapon_echo( CHAR_DATA * ch, OBJ_DATA * obj )
{
    char buf[MAX_STRING_LENGTH];
    int base = 0, factor = 0, volume = 0;

    if ( ch == NULL ) return;
    if ( obj == NULL ) return;
    if ( obj->item_type != ITEM_WEAPON ) return;

    if ( !is_ranged( obj->value[0] ) ) return;

    base = 4;
    if ( xIS_SET( obj->extra_flags, ITEM_LOUD ) ) base = 6;
    if ( xIS_SET( obj->extra_flags, ITEM_MODERATE ) ) base = 4;
    if ( xIS_SET( obj->extra_flags, ITEM_QUIET ) ) base = 2;

    if ( ch->in_room == NULL ) return;

    if ( ch->in_room->area == NULL ) return;

    if ( ch->in_room->area->ambience == 0 ) factor = 2;
    if ( ch->in_room->area->ambience == 1 ) factor = 1;
    if ( ch->in_room->area->ambience == 2 ) factor = 2;
    if ( ch->in_room->area->ambience == 3 ) factor = 3;

    volume = (int)( (float)(base) / (float)(factor) );

    if ( obj->attach )
    {
       /* Silencer? */
       if ( obj->attach->value[0] == 3 )
         volume = UMAX( 0, volume - obj->attach->value[2] );
    }

    if ( volume > 0 )
    {
         if ( obj->value[0] == WEAPON_PISTOL )
         {
            sprintf( buf, "You hear pistol fire" );
         }
         else if ( obj->value[0] == WEAPON_RIFLE )
         {
            if ( obj->weapon_mode == MODE_SINGLE )
            {
              if ( base >= 5 )
              { sprintf( buf, "You hear a loud rifle shot" ); }
              else
              { sprintf( buf, "You hear a rifle firing" ); }
            }
            if ( obj->weapon_mode == MODE_SEMIAUTO )
              sprintf( buf, "You hear several rifle shots" );
            if ( obj->weapon_mode == MODE_BURST )
              sprintf( buf, "You hear a burst of rifle fire" );
            if ( obj->weapon_mode == MODE_AUTOMATIC )
              sprintf( buf, "You hear automatic fire" );
         }
         else if ( obj->value[0] == WEAPON_ROCKETFIRED )
         {
            sprintf( buf, "You hear a launcher" );
         }
         else if ( obj->value[0] == WEAPON_FLAMETHROWER )
         {
            sprintf( buf, "You hear a burst of flame" );
         }
         else if ( obj->value[0] == WEAPON_SHOTGUN )
         {
            sprintf( buf, "You hear a shotgun blast" );
         }
         else if ( obj->value[0] == WEAPON_ERANGED )
         {
            sprintf( buf, "You hear an energy discharge" );
         }
         else if ( obj->value[0] == WEAPON_AUTOMATIC )
         {
            sprintf( buf, "You hear automatic fire" );
         }
         else if ( obj->value[0] == WEAPON_RANGED )
         {
            sprintf( buf, "You hear weapon fire" );
         }
         else
         {
            bug("Weapon_echo: Unknown echo type %d.", obj->value[0] );
            return;
         }

         send_sound( buf, ch->in_room, volume, ch );
    }
}

bool check_rescue( CHAR_DATA * ch )
{
   CHAR_DATA * rescue;

   if ( xIS_SET( ch->act, ACT_HOSTAGE ) && ch->master != NULL )
   {
        if ( !xIS_SET( ch->in_room->room_flags, ROOM_RESCUE ) ) return FALSE;

        rescue = ch->master;
 
        radius_rescue( rescue, ch );

        stop_follower( ch );
 
        xREMOVE_BIT( ch->act, ACT_HOSTAGE );
        xSET_BIT( ch->act, ACT_SENTINEL );

        do_say( ch, "Thank you! I'll stay here and keep an eye out." );

        return TRUE;
   }

   return FALSE;
}

void do_rescue( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA * victim;

   argument = one_argument( argument, arg );

   if ( ch->race != RACE_MARINE )
   {
      do_nothing( ch, "" );
      return;
   }

   if ( is_spectator( ch ) ) { send_to_char( "&RYou may not rescue while in spectator mode.\n\r", ch ); return; }

   if ( arg[0] == '\0' )
   {
      send_to_char( "&RSyntax: RESCUE (target)\n\r", ch );
      send_to_char( "&RTarget hostage will follow you to the rescue point.\n\r", ch );
      return;
   }

   if ( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\n\r", ch );
      return;
   }

   if ( !IS_NPC( victim ) || !xIS_SET( victim->act, ACT_HOSTAGE ) )
   {
      send_to_char( "You can't rescue them, buddy.\n\r", ch );
      return;
   }

   if ( victim->master != NULL )
   {
      if ( victim->master == ch )
      {
          /* Leave the hostage here. */
          do_say( victim, "You.. You can't leave me here!" );
          stop_follower( victim );
          return;
      }
      else
      {
          /* Hostage is already being rescued. */
          send_to_char("Someone is already helping that hostage.\n\r", ch );
          return;
      }
   }

   do_say( victim, "Lets go!" );
   add_follower( victim, ch );

   victim->position = POS_STANDING;
   
   return;
}

void clear_effects( CHAR_DATA * ch )
{ 
   AFFECT_DATA *paf, *paf_next;

   for ( paf = ch->first_affect; paf; paf = paf_next )
   {
       paf_next  = paf->next;
       affect_remove( ch, paf );
   }   

   return;
}

void ignite_target( CHAR_DATA * ch, CHAR_DATA * victim, int duration )
{
   char tmp[MAX_STRING_LENGTH];
   AFFECT_DATA af;

   if ( !IS_AFFECTED( victim, AFF_NAPALM ) )
   {
      sprintf( tmp, "$n is engulfed in flames! (Lasts %d rounds)", duration );
      act( AT_RED, tmp, victim, NULL, NULL, TO_ROOM );
    
      ch_printf( victim, "&RYou are engulfed in flames! (Lasts %d rounds)\n\r", duration );
   }

   af.type      = 0;
   af.location  = APPLY_NONE;
   af.modifier  = 0;
   af.duration  = duration;
   xCLEAR_BITS( af.bitvector );
   xSET_BIT(af.bitvector, AFF_NAPALM);
   xSET_BIT(af.bitvector, AFF_BLIND);
   affect_join( victim, &af );

   return;
}

void short_target( CHAR_DATA * ch, CHAR_DATA * victim, int duration )
{
   char tmp[MAX_STRING_LENGTH];
   AFFECT_DATA af;
   bool echo = FALSE;

   if ( !IS_AFFECTED( victim, AFF_CLOAK ) ) return;

   if ( !IS_AFFECTED( victim, AFF_SHORTAGE ) ) echo = TRUE;

   af.type      = 0;
   af.location  = APPLY_NONE;
   af.modifier  = 0;
   af.duration  = duration;
   xCLEAR_BITS( af.bitvector );
   xSET_BIT(af.bitvector, AFF_SHORTAGE);
   affect_join( victim, &af );

   if ( echo )
   {
      sprintf( tmp, "&w&C$n crackles and shimmers into existance." );
      act( AT_RED, tmp, victim, NULL, NULL, TO_ROOM );
    
      ch_printf( victim, "&w&CYou crackle and shimmer into existance. (Lasts %d rounds)\n\r", duration );
   }

   return;
}

void clear_shortage( CHAR_DATA * victim )
{
   char tmp[MAX_STRING_LENGTH];

   if ( IS_AFFECTED( victim, AFF_CLOAK ) )
   {
      sprintf( tmp, "&w&C$n shimmers and vanishes into thin air." );
      act( AT_RED, tmp, victim, NULL, NULL, TO_ROOM );
    
      ch_printf( victim, "&w&C[Cloak Ready] You shimmer and vanish into thin air.\n\r" );
   }
   else
   {
      sprintf( tmp, "&w&CThe energy crackling over $n clears." );
      act( AT_RED, tmp, victim, NULL, NULL, TO_ROOM );
    
      ch_printf( victim, "&w&CThe electrical short clears, your systems are ready again.\n\r" );
   }

   return;
}

void do_fire( CHAR_DATA * ch, char * argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   EXIT_DATA * pexit = NULL;
   OBJ_DATA * obj = NULL;
   int range = -1;
   int dir = -1;
 
   if ( ch->race != RACE_MARINE )
   {
      do_nothing( ch, "" );
      return;
   }

   if ( is_spectator( ch ) ) { send_to_char( "&RYou may not engage in combat while in spectator mode.\n\r", ch ); return; }

   if ( ch->ap < get_max_ap( ch ) )
   {
       send_to_char( "&RYou must wait for the attack gauge to replenish first.\n\r", ch );
       return;
   }

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if ( arg1[0] == '\0' || ( dir = get_door( arg1 ) ) == -1 )
   {
      ch_printf( ch, "Syntax: FIRE (direction) [optional range]\n\r" );
      return;
   }

   if ( (pexit = get_exit( ch->in_room, dir )) == NULL )
   {
      ch_printf( ch, "\n\rLook. You don't fire weapons into walls. You just dont.\n\r" );
      return;
   }

   if ( xIS_SET( pexit->exit_info, EX_BLASTOPEN ) && !xIS_SET( pexit->exit_info, EX_BLASTED ) )
   {
      ch_printf( ch, "\n\rLook. You don't fire weapons into walls. You just dont.\n\r" );
      return;
   }

   if ( xIS_SET(pexit->exit_info, EX_CLOSED) )
   {
      ch_printf( ch, "\n\rLook. You don't fire weapons into doors. You just dont.\n\r" );
      return;
   }

   if ( atoi( arg2 ) > 0 ) range = atoi( arg2 );

   if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
   {
      if ( obj->item_type == ITEM_WEAPON )
      {
         if ( obj->value[0] == WEAPON_ROCKETFIRED )
         {
            fire_weapon( ch, obj, dir, range );
         }
      }
      if ( obj->attach )
      {
         fire_attachment( ch, obj->attach, dir, range );
      }
   }

   if ( ( obj = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL )
   {
      if ( obj->item_type == ITEM_WEAPON )
      {
         if ( obj->value[0] == WEAPON_ROCKETFIRED )
         {
            fire_weapon( ch, obj, dir, range );
         }
      }
      if ( obj->attach )
      {
         fire_attachment( ch, obj->attach, dir, range );
      }
   }

   return;
}


int fire_weapon( CHAR_DATA * ch, OBJ_DATA * obj, int dir, int range )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA * to_room;
    EXIT_DATA * pexit;
    OBJ_DATA * tmp;
    int ammocode;
    int cnt;    

    if ( ch == NULL ) return 0;
    if ( obj == NULL ) return 0;

    to_room = ch->in_room;

    if ( range < 0 ) range = obj->value[2];
    if ( range > obj->value[2] ) range = obj->value[2];

    if ( !obj->ammo )
    {
       ch_printf( ch, "How about you load the weapon first?\n\r" );
       return 0;
    }

    if ( obj->ammo->value[0] != 5 ) return 0;

    ammocode = obj->ammo->value[1];

    if ( get_obj_index( ammocode ) == NULL )
    {
       bug("Error finding index at fire_weapon in fight.c");
       return 0;
    }

    tmp = create_object( get_obj_index( ammocode ), 1 );

    if ( tmp == NULL )
    {
       bug("Error invoking object at fire_weapon in fight.c");
       return 0;
    }

    // Firing Message
    ch_printf( ch, "&RYou fire a %s to %s.\n\r", obj->short_descr, main_exit(dir) );
    sprintf( buf, "$n fires a %s %s." , obj->short_descr, dir_name[dir] );
    act( AT_BLOOD, buf, ch, NULL, NULL, TO_ROOM );    

    if ( IS_AFFECTED(ch, AFF_HIDE) ) xREMOVE_BIT(ch->affected_by, AFF_HIDE);

    weapon_echo( ch, obj );

    // unequip_char( ch, obj );
    // separate_obj( tmp );
    // obj_from_char( tmp );

    ch->ap = 0;
    obj->ammo->value[2]--;
    auto_eject( ch, obj );

    showammo_option( ch, obj ); // ShowAmmo Notification

    pexit = get_exit( ch->in_room, dir );

    for ( cnt = 1; ; cnt++ )
    {
       if ( !pexit )
       {
          sprintf( buf, "&w&r%s sails from %s and hits a wall." , tmp->short_descr , rev_exit( dir ) );
          echo_to_room( -1, to_room, buf ); 
          break;
       }
       if ( !pexit->to_room )
       {
          sprintf( buf, "&w&r%s sails from %s and hits a wall." , tmp->short_descr , rev_exit( dir ) );
          echo_to_room( -1, to_room, buf ); 
          break;
       }
       if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
       {
          sprintf( buf, "&w&r%s sails from %s and hits a door." , tmp->short_descr , rev_exit( dir ) );
          echo_to_room( -1, to_room, buf ); 
          break;
       }

       to_room = NULL;
       if ( pexit->distance > 1 ) to_room = generate_exit( ch->in_room , &pexit );
    
       if ( to_room == NULL ) to_room = pexit->to_room;
    
       if ( !to_room ) break;

       if ( cnt >= range )
       {
           sprintf( buf, "&w&r%s sails from %s and lands on the floor." , tmp->short_descr , rev_exit( dir ) );
           echo_to_room( -1, to_room, buf ); 
           break;         
       }
       else if ( ( pexit = get_exit( to_room, dir ) ) == NULL )
       {
           sprintf( buf, "&w&r%s sails from %s and hits a wall." , tmp->short_descr , rev_exit( dir ) );
           echo_to_room( -1, to_room, buf ); 
           break;
       }
       else
       {
           sprintf( buf, "&w&r%s sails through the room. (%s to %s).", tmp->short_descr, dir_name[flip_dir(dir)], dir_name[dir] );
           echo_to_room( -1, to_room, buf ); 
       }
    }

    if ( !to_room ) to_room = ch->in_room;

    tmp = obj_to_room( tmp, to_room );

    // Arm this puppy.    
    if ( tmp->armed_by ) STRFREE( tmp->armed_by ); 
    tmp->armed_by = STRALLOC( ch->name );

    if ( !xIS_SET( to_room->room_flags, ROOM_PROTOTYPE ) )
    {
        if ( to_room->area )
        {
           motion_ping( to_room->x, to_room->y, to_room->z, to_room->area, NULL );
        }
    }

    if ( tmp->item_type == ITEM_GRENADE )
    {
        explode( tmp );
    }
    else if ( tmp->item_type == ITEM_FLARE )
    {
       tmp->timer = tmp->value[0];
       tmp->value[2] = 1;
       tmp->cost = 0;

       to_room->light++;
    }

    return 1;
}

char * current_weapon( CHAR_DATA * ch )
{
    OBJ_DATA * obj;

    if ( !ch ) return "";

    if ( ch->race == RACE_ALIEN ) return "Claws";

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    {
       // Unarmed
       if ( ch->race == RACE_MARINE ) return "Fists";
       if ( ch->race == RACE_PREDATOR ) return "Wristblades";
    }
    else
    {
       return obj->short_descr;
    }

    return "";
}

void showammo_option( CHAR_DATA * ch, OBJ_DATA * wield )
{
    if ( !ch ) return;
    if ( !ch->pcdata ) return;
    if ( IS_NPC( ch ) ) return;

    if ( xIS_SET( ch->pcdata->flags, PCFLAG_SHOWAMMO ) )
     if ( wield )
      if ( wield->ammo )
       ch_printf( ch, "&w&O(&YAmmunition Remaining &O:: &Y%d Rounds&O)\n\r", wield->ammo->value[2] );

    return;
}
