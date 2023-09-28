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
* 		Commands for personal player settings/statictics	   *
****************************************************************************/
 
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 *  Locals
 */
char *tiny_affect_loc_name(int location);

char *drawalign(int align)
{
  static char buf[MAX_STRING_LENGTH];
  if (align >= 1000) sprintf(buf, "&W[&C============&G|&W]");
  else if (align >= 900 ) sprintf(buf, "&W[&C===========&G|&C=&W]");
  else if (align >= 600 ) sprintf(buf, "&W[&C==========&G|&C==&W]");
  else if (align >= 400 ) sprintf(buf, "&W[&C=========&G|&C===&W]");
  else if (align >= 200 ) sprintf(buf, "&W[&C========&G|&C====&W]");
  else if (align >= 100 ) sprintf(buf, "&W[&C=======&G|&C=====&W]");
  else if (align <= -1000) sprintf(buf, "&W[&R|&C============&W]");
  else if (align <= -900 ) sprintf(buf, "&W[&C=&R|&C===========&W]");
  else if (align <= -600 ) sprintf(buf, "&W[&C==&R|&C==========&W]");
  else if (align <= -400 ) sprintf(buf, "&W[&C===&R|&C=========&W]");
  else if (align <= -200 ) sprintf(buf, "&W[&C====&R|&C========&W]");
  else if (align <= -100 ) sprintf(buf, "&W[&C=====&R|&C=======&W]");
  else sprintf(buf, "&W[&C======&W|&C======&W]");
  
  return buf;
}

char *drawlevel( CHAR_DATA * ch )
{
  static char buf[MAX_STRING_LENGTH];
  int perc = 0;
  int a = 0, b = 0;

  strcpy( buf, "" );

  perc = ( ch->currexp * 100 ) / exp_level( ch->top_level + 1 );

  if ( perc >= 100 )
  {
      strcpy( buf, "&C||||||||||" );
      return buf;
  }

  for ( b = 1; b < 10; b++ ) if ( b * 10 <= perc ) a++;

  if ( a > 0 )
  {
      strcat( buf, "&R" );

      for ( b = 0; b < a; b++ ) strcat( buf, "|" );
  }

  b = 10 - a;
  
  if ( b > 0 )
  {
      strcat( buf, "&r" );

      for ( a = 0; a < b; a++ ) strcat( buf, "-" );
  }
 
  return buf;
}

char *drawcharge( CHAR_DATA * ch )
{
  static char buf[MAX_STRING_LENGTH];
  int perc = 0;
  int a = 0, b = 0;

  strcpy( buf, "" );

  perc = ( ch->field * 100 ) / ch->max_field;

  if ( perc >= 100 )
  {
      strcpy( buf, "&C|||||||||||||||||||||||||" );
      return buf;
  }

  for ( b = 1; b < 25; b++ ) if ( b * ( 100 / 25 ) <= perc ) a++;

  if ( a > 0 )
  {
      strcat( buf, "&C" );

      for ( b = 0; b < a; b++ ) strcat( buf, "|" );
  }

  b = 25 - a;
  
  if ( b > 0 )
  {
      strcat( buf, "&c" );

      for ( a = 0; a < b; a++ ) strcat( buf, "-" );
  }
 
  return buf;
}

char * drawbar( int bars, int curr, int max, char * cA, char * cB )
{
  static char buf[MAX_STRING_LENGTH];
  float percA = 0.0;
  float percB = 0.0;
  int a=0, b=0;

  strcpy( buf, "" );

  if ( max == 0 ) return buf;
  if ( curr > max ) curr = max;

  percA = (float)( (float)(curr) * (float)(100) ) / (float)(max);

  for ( b = 1; b <= bars; b++ )
  {
    percB = (float)( (float)(b) * (float)(100) ) / (float)(bars);
    if ( percB <= percA ) a++;
    // if ( ( ( (float)(b) * (float)(100) ) / (float)(bars) ) <= (float)(perc) ) a++;
  }

  if ( a > 0 )
  {
      strcat( buf, cA );

      for ( b = 0; b < a; b++ ) strcat( buf, "|" );
  }

  b = bars - a;
  
  if ( b > 0 )
  {
      strcat( buf, cB );

      for ( a = 0; a < b; a++ ) strcat( buf, "-" );
  }

  return (char *)(buf);
}

const char *wis_score[] = {
"Fool","Dim","of Average Wisdom","Good","Wise","Excellent"};
const char *str_score[] = {
"Wimpy", "Weak", "of Average Strength", "Strong",  "Herculean","Titantic"};
const char *con_score[] = {
"Fragile","Poor","of Average Build","Healthy","Hearty","Iron"};
const char *int_score[] = {
"Hopeless","Dumb","of Average Intellect","Smart","Clever","Genius"};
const char *dex_score[] = {
"Slow","Clumsy","of Average Dexterity","Agile","Quick","Fast"};
const char *cha_score[] = {
"Mongol","Ugly","of Average looks", "Good Looking","Handsome","Charismatic"};


/*
 * New score command by Ghost
 * Various Pages:
 *   STAT   : Detailed Stat Information
 *   LEVEL  : Detailed Level Information
 *   AFFECT : Detailed Affect Information
 *   TIME   : Log In/Out Times
 */
void do_score(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int iLang, ability, i, cnt=0;
    int kA, kB;
    int ratio = 0;
    AFFECT_DATA    *paf;
    EXT_BV xbit;
   
    argument = one_argument( argument, arg );

    if ( IS_NPC(ch) )
    {
	do_oldscore(ch, argument);
	return;
    }

    send_to_char( "\n\r", ch );
    ch_printf(ch, "&zName: &W%s. &z[&W%s&z]\n\r", ch->pcdata->title, race_table[ch->race].race_name );
    sprintf( buf, "&z%-15s &B%-2d  &z(&G%+-d&z)", "(&CStr&z)ength:", get_curr_str( ch ), ch->mod_str );
    sprintf( buf2, "&z%-20s &B%-2d  &z(&G%+-d&z)", "(&CSta&z)mina:", get_curr_sta( ch ), ch->mod_sta );
    ch_printf(ch, "%-35s  %-39s\n\r", buf, buf2 );
    sprintf( buf, "&z%-15s &B%-2d  &z(&G%+-d&z)", "(&CRec&z)overy:", get_curr_rec( ch ), ch->mod_rec );
    sprintf( buf2, "&z%-20s &B%-2d  &z(&G%+-d&z)", "(&CPer&z)ception:", get_curr_per( ch ), get_mod_per( ch ) );
    ch_printf(ch, "%-35s  %-39s\n\r", buf, buf2 );                          
    sprintf( buf, "&z%-15s &B%-2d  &z(&G%+-d&z)", "(&CBra&z)very:", get_curr_bra( ch ), ch->mod_bra );
    sprintf( buf2, "&z%-20s &B%-2d  &z(&G%+-d&z)", "(&CInt&z)elligence:", get_curr_int( ch ), ch->mod_int );
    ch_printf(ch, "%-35s  %-39s\n\r", buf, buf2 );

    sprintf( buf, "&zCarry:   &B%d&b/%d &zitems.", ch->carry_number, can_carry_n(ch) );
    if ( ch->carry_weight >= ( can_carry_w(ch) * 0.75 ) )
    { sprintf( buf2, "&zWeight:      &R%d&r/%d &zpounds.", ch->carry_weight, can_carry_w(ch) ); }
    else
    { sprintf( buf2, "&zWeight:      &B%d&b/%d &zpounds.", ch->carry_weight, can_carry_w(ch) ); }
    ch_printf(ch, "%-29s  %-39s\n\r", buf, buf2 );

    sprintf( buf, "&zArmor:   &G%d%%", armor_status( ch, 1 ) );
    sprintf( buf2, "&zExperience:  &Y%d point(s).", ch->currexp );
    ch_printf(ch, "%-25s  %-39s\n\r", buf, buf2 );
    sprintf( buf, "&zHealth:  &R%d&r/%d", ch->hit, ch->max_hit );
    sprintf( buf2, "&zMovement:    &G%d&g/%d", ch->move, ch->max_move );
    ch_printf(ch, "%-27s  %-39s\n\r", buf, buf2 );

    if ( ch->race == RACE_ALIEN )
       sprintf( buf, "&zResin:   [%s&w&z]", drawbar( 10, ch->resin, get_max_resin( ch ), "&G", "&g" ) );
    if ( ch->race == RACE_MARINE )
       sprintf( buf, "&zMorale:  [%s&w&z]", drawbar( 10, ch->morale, get_max_morale( ch ), "&G", "&g" ) );
    if ( ch->race == RACE_PREDATOR )
       sprintf( buf, "&zCharge:  &w&C%d%%", find_percent( ch->field, ch->max_field ) );

    sprintf( buf2, "&zTeam Kills:  &B%d&b/%d", ch->teamkill, get_max_teamkill( ch ) );
    if ( ch->teamkill <= 1 )
      sprintf( buf2, "&zTeam Kills:  &R%d&r/%d", ch->teamkill, get_max_teamkill( ch ) );

    ch_printf(ch, "%-27s  %-39s\n\r", buf, buf2 );
    
    // if ( ch->race == RACE_PREDATOR ) ch_printf(ch, "&zField Charge:          [%s&z]\n\r", drawcharge( ch ) );
    ch_printf(ch, "&zRank:    &W%-11s ( &zProgress:    [%s&z] &W)\n\r", get_rank( ch->race, ch->top_level ), drawlevel( ch ) );

    kA = total_pc_kills( ch );
    kB = total_pc_killed( ch );

    ch_printf(ch, "&zKills:   %d total kills. (Non-mob)\n\r", kA );       
    sprintf( buf, "&z[&Rx%d&z]", ch->pcdata->kills.pc[RACE_ALIEN] + ch->pcdata->kills.npc[RACE_ALIEN] + ch->pcdata->kills.bot[RACE_ALIEN] );
    ch_printf(ch, " &WAliens     %-16s ( &W%d players. %d mobs. %d bots. &z)\n\r", buf,
       ch->pcdata->kills.pc[RACE_ALIEN], ch->pcdata->kills.npc[RACE_ALIEN], ch->pcdata->kills.bot[RACE_ALIEN] );
    sprintf( buf, "&z[&Rx%d&z]", ch->pcdata->kills.pc[RACE_MARINE] + ch->pcdata->kills.npc[RACE_MARINE] + ch->pcdata->kills.bot[RACE_MARINE] );
    ch_printf(ch, " &WMarines    %-16s ( &W%d players. %d mobs. %d bots. &z)\n\r", buf,
       ch->pcdata->kills.pc[RACE_MARINE], ch->pcdata->kills.npc[RACE_MARINE], ch->pcdata->kills.bot[RACE_MARINE] );
    sprintf( buf, "&z[&Rx%d&z]", ch->pcdata->kills.pc[RACE_PREDATOR] + ch->pcdata->kills.npc[RACE_PREDATOR] + ch->pcdata->kills.bot[RACE_PREDATOR] );
    ch_printf(ch, " &WPredators  %-16s ( &W%d players. %d mobs. %d bots. &z)\n\r", buf,
       ch->pcdata->kills.pc[RACE_PREDATOR], ch->pcdata->kills.npc[RACE_PREDATOR], ch->pcdata->kills.bot[RACE_PREDATOR] );

    ch_printf(ch, "&zKilled:  %d total times. (Non-Mob)\n\r", kB );
    sprintf( buf, "&z[&Rx%d&z]", ch->pcdata->killed.pc[RACE_ALIEN] + ch->pcdata->killed.npc[RACE_ALIEN] + ch->pcdata->killed.bot[RACE_ALIEN] );
    ch_printf(ch, " &WAliens     %-16s ( &W%d players. %d mobs. %d bots. &z)\n\r", buf, 
       ch->pcdata->killed.pc[RACE_ALIEN], ch->pcdata->killed.npc[RACE_ALIEN], ch->pcdata->killed.bot[RACE_ALIEN] );
    sprintf( buf, "&z[&Rx%d&z]", ch->pcdata->killed.pc[RACE_MARINE] + ch->pcdata->killed.npc[RACE_MARINE] + ch->pcdata->killed.bot[RACE_MARINE] );
    ch_printf(ch, " &WMarines    %-16s ( &W%d players. %d mobs. %d bots. &z)\n\r", buf,      
       ch->pcdata->killed.pc[RACE_MARINE], ch->pcdata->killed.npc[RACE_MARINE], ch->pcdata->killed.bot[RACE_MARINE] );
    sprintf( buf, "&z[&Rx%d&z]", ch->pcdata->killed.pc[RACE_PREDATOR] + ch->pcdata->killed.npc[RACE_PREDATOR] + ch->pcdata->killed.bot[RACE_PREDATOR] );
    ch_printf(ch, " &WPredators  %-16s ( &W%d players. %d mobs. %d bots. &z)\n\r", buf,
       ch->pcdata->killed.pc[RACE_PREDATOR], ch->pcdata->killed.npc[RACE_PREDATOR], ch->pcdata->killed.bot[RACE_PREDATOR] );
            
    ch_printf( ch, "&zKill-to-Killed Ratio: &C%s\n\r", reduce_ratio( kA, kB ) );
    /*
    if ( kB > 0 )
    {
        ratio = (int)( (float)( (float)(kA) / (float)(kB) ) * (float)(100) );
        ch_printf( ch, "&zKill-to-Killed Ratio: &C%d%%\n\r", ratio );
    }
    else
    {
        ch_printf( ch, "&zKill-to-Killed Ratio: &cUnknown\n\r" );
    }
    */

    return;

    send_to_char("&z---------------------------------------------------------------------------\n\r", ch);

    // ch_printf(ch, "&zDamroll: &Y%-2.2d   &zArmor: &Y%-4.4d  &zAlign: &C%-15s\n\r", GET_DAMROLL(ch), GET_AC(ch), drawalign(ch->alignment) );
    // ch_printf(ch, "&zHit Points: &R%d &zof &R%d     &zMove: &Y%d &zof &Y%d     \n\r", ch->hit, ch->max_hit, ch->move, ch->max_move );
    // ch_printf(ch, "&zStr: &C%2d  &zDex: &C%2d  &zCon: &C%2d  &zInt: &C%2d  &zWis: &C%2d  &zCha: &C%2d  &zLck: &C??  &zFrc: &C??\n\r", get_curr_str(ch), get_curr_dex(ch),get_curr_con(ch),get_curr_int(ch),get_curr_wis(ch),get_curr_cha(ch));

    return;
}

/*
 * Return ascii name of an affect location.
 */
char           *
tiny_affect_loc_name(int location)
{
	switch (location) {
	case APPLY_NONE:		return "NIL";
	case APPLY_STR:			return " STR  ";
        case APPLY_STA:                 return " STA  ";
        case APPLY_REC:                 return " REC  ";
	case APPLY_INT:			return " INT  ";
        case APPLY_BRA:                 return " BRA  ";
        case APPLY_PER:                 return " PER  ";
        case APPLY_HIT:                 return " HP   ";
	case APPLY_MOVE:		return " MOVE ";
	case APPLY_AFFECT:		return "AFF BY";
        case APPLY_FIRE:                return " FIRE ";
        case APPLY_ENERGY:              return "ENERGY";
        case APPLY_IMPACT:              return "IMPACT";
        case APPLY_PIERCE:              return "PIERCE";
        case APPLY_ACID:                return " ACID ";
	}

	bug("Affect_location_name: unknown location %d.", location);
	return "(???)";
}

char * get_race( CHAR_DATA *ch )
{
    if ( ch->race < MAX_NPC_RACE && ch->race >= 0 )
	return ( npc_race[ch->race] );
    return ("Unknown");
}

char * get_sex( CHAR_DATA *ch )
{
    if ( ch->sex == 0 ) return "neutral";
    if ( ch->sex == 1 ) return "male";
    if ( ch->sex == 2 ) return "female";

    return ("unknown");
}

void do_level( CHAR_DATA *ch, char *argument )
{
    send_to_char( "Sorry, incomplete routine.\n\r", ch );
    bug( "player.c: incomplete code under do_level." );
    return;
}

void do_armor( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int percent;

    ch_printf( ch, "&zArmor status: [%s&w&z]\n\r", drawbar( 10, armor_status( ch, 1 ), 100, "&G", "&g" ) );

    ch_printf( ch, "\n\r&WCurrent protection:\n\r" );
    ch_printf( ch, " &zFrom %-7s &G%+-d%%\n\r", "fire:", ch->protect[RIS_FIRE] );
    ch_printf( ch, " &zFrom %-7s &G%+-d%%\n\r", "energy:", ch->protect[RIS_ENERGY] );
    ch_printf( ch, " &zFrom %-7s &G%+-d%%\n\r", "impact:", ch->protect[RIS_IMPACT] );
    ch_printf( ch, " &zFrom %-7s &G%+-d%%\n\r", "pierce:", ch->protect[RIS_PIERCE] );
    ch_printf( ch, " &zFrom %-7s &G%+-d%%\n\r", "acid:", ch->protect[RIS_ACID] );

    return;
}

void do_affected ( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    SKILLTYPE *skill;
 
    if ( IS_NPC(ch) )
        return;

    argument = one_argument( argument, arg );

    if ( !ch->first_affect )
    {
        send_to_char( "\n\r&YThere is currently nothing affecting you.\n\r", ch );
    }
    else
    {
	send_to_char( "\n\r", ch );
        for (paf = ch->first_affect; paf; paf = paf->next)
        {
         // bug("PAF Type: %d", paf->type);
         if ( (skill=get_skilltype(paf->type)) != NULL )
          {
            send_to_char( "&BAffected:  ", ch );
            if ( ch->top_level >= 1 )
            {
                if (paf->duration < 25 )      ch_printf( ch, "&z(&Y%5d&z )   ", paf->duration );
                else if (paf->duration < 6  ) ch_printf( ch, "&z(&R%5d&z )   ", paf->duration );
                else                          ch_printf( ch, "&z(&C%5d&z )   ", paf->duration );
	    }
            if (paf->modifier == 0)
               ch_printf( ch, "&z[ &R------------ &z]   " );
            else if (paf->modifier > 999)
               ch_printf( ch, "&z[          %7.7s&z ]   ", paf->modifier, tiny_affect_loc_name(paf->location) );
            else                                      
               ch_printf( ch, "&z[ &R%+-4.4d %7.7s&z]   ", paf->modifier, tiny_affect_loc_name(paf->location) );
            ch_printf( ch, "&C%-14s&z", skill->name );
            send_to_char( "\n\r", ch );
         }
        }
    }
    return;
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_RED, ch );
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->first_carrying, ch, TRUE, TRUE );
    return;
}


void do_equipment( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear, dam;
    bool found;
    int a, b;
    
    send_to_char( "&RYou are using:\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
      for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	   if ( obj->wear_loc == iWear )
	   {
                send_to_char( where_name[iWear], ch );
		if ( can_see_obj( ch, obj ) )
		{
                    send_to_char( "&G", ch );
                    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
		    strcpy( buf , "" );
		    switch ( obj->item_type )
                    {
                       default:
                          break;

                       case ITEM_ARMOR:
                          if ( obj->value[1] == 0 ) obj->value[1] = obj->value[0];
                          if ( obj->value[1] == 0 ) obj->value[1] = 1;
                          dam = (sh_int)((obj->value[0] * 100) / obj->value[1]);
                          sprintf( buf, " &B(&Y%d%%&B)", dam );
                          send_to_char( buf, ch );
                          break;
                       case ITEM_LIGHT:
                          ch_printf( ch, " &B(&CTime: &Y%s&B) ", drawbar( 6, obj->value[2], obj->value[4], "&Y", "&Y" )  );
                          if ( IS_SET( obj->value[3], BV00 ) ) strcat( buf, " &B(&YOn&B) " );
                          if ( !IS_SET( obj->value[3], BV00 ) ) strcat( buf, " &B(&COff&B) " );
                          send_to_char( buf, ch );
                          break;
                       case ITEM_NVGOGGLE:
                          ch_printf( ch, " &B(&CTime: &Y%s&B) ", drawbar( 6, obj->value[0], obj->value[1], "&Y", "&Y" )  );
                          if ( IS_SET( obj->value[3], BV00 ) ) strcat( buf, " &B(&YOn&B) " );
                          if ( !IS_SET( obj->value[3], BV00 ) ) strcat( buf, " &B(&COff&B) " );
                          send_to_char( buf, ch );
                          break;
                       case ITEM_SIFT:
                          ch_printf( ch, " &B(&CReset: &Y%s&B) ", drawbar( 6, obj->value[0], obj->value[1], "&Y", "&Y" )  );
                          send_to_char( buf, ch );
                          break;
                       case ITEM_MEDICOMP:
                          ch_printf( ch, " &B(&CReset: &Y%s&B) ", drawbar( 6, obj->value[0], obj->value[1], "&Y", "&Y" )  );
                          send_to_char( buf, ch );
                          break;
                       case ITEM_GRENADE:
                          if ( obj->value[4] > 0 ) ch_printf( ch, " &R[ARMED] &B(&C%d Rounds&B) ", obj->value[4] );
                          if ( obj->value[4] < 1 ) strcat( buf, " &B(&CUnarmed&B) " );
                          send_to_char( buf, ch );
                          break;
                       case ITEM_CONTAINER:
                          a = ( get_obj_weight( obj ) / obj->count );
                          b = obj->value[0] - obj->weight;
                          sprintf( buf, " &B(%s&B)", drawbar( 6, a, b, "&Y", "&Y" ) );             
                          send_to_char( buf, ch );
                          break;
                       case ITEM_SCANNON:
                          sprintf( buf, " &B(&CHeat: %s&B)", drawbar( 6, obj->value[4], obj->value[5], "&R", "&R" ) );
                          send_to_char( buf, ch );
                          break;
                       case ITEM_SPAWNER:
                          ch_printf( ch, " &B(&C%d uses left&B)", obj->value[0] );
                          break;
                       case ITEM_WEAPON:
                          if ( obj->ammo )
                          {
                             if ( obj->value[0] == WEAPON_FLAMETHROWER )
                             {
                                if ( obj->ammo->pIndexData )
                                  ch_printf( ch, " &B(&C%d%%&B)", make_percent( obj->ammo->value[2], get_max_rounds(obj->ammo) ) );
                             }
                             else
                             {
                                ch_printf( ch, " &B(&C%d rounds&B)", obj->ammo->value[2] );
                             }
                          }
                          else
                          {
                             if ( has_ammo(obj->value[0] ) )
                             {
                                ch_printf( ch, " &B(&wNot Loaded&B)" );
                             }
                             else
                             {
                                if ( obj->value[0] == WEAPON_ERANGED )
                                {
                                     a = ch->field;
                                     if ( obj->value[1] > 0 ) a /= obj->value[1];
                                     ch_printf( ch, " &B(&C%d shot%s&B)", a, (a == 1) ? "" : "s" );
                                }
                             }
                          }
                          break;
                       }

                       send_to_char( "\n\r", ch );

                       /* Show attachments */
                       if ( obj->attach )
                       {
                           send_to_char("&B<&Cweapon upgrade&B>    ", ch );
                           ch_printf( ch, "&B%s", obj->attach->short_descr );
                           send_attach_note( ch, obj->attach );
                           send_to_char( "\n\r", ch );
                       }
		}
		else
		    send_to_char( "something.\n\r", ch );
		found = TRUE;
	   }
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    /*
    if ( isalpha(title[0]) || isdigit(title[0]) )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    */

    strcpy( buf, title );

    STRFREE( ch->pcdata->title );
    ch->pcdata->title = STRALLOC( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( xIS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ))
    {
        send_to_char( "You try but the Force resists you.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }

    if ( checkclr( argument, "x" ) )
    {
        send_to_char("&RYou cannot use the BLACK Color token in your title.\n\r", ch );
        return;
    }
  
    if ( nifty_is_name( "^", argument ) )
    {
        send_to_char("&RYou cannot use the background/blinking tokens in your title.\n\r", ch );
        return;
    }

    if ((get_trust(ch) <= 101) && (!nifty_is_name(ch->name, stripclr(argument))))
     {
       send_to_char("You must include your name somewhere in your title!\n\r", ch);
       return;
     }
    if ((get_trust(ch) <= 101) && (strstr(argument, ")") || strstr(argument,"("))){
       send_to_char("You cannot have ( or ) in your title!\n\r", ch);
       return;
    }

    strcat( argument, "&w" );
 
    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}

void do_homepage( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	if ( !ch->pcdata->homepage )
	  ch->pcdata->homepage = str_dup( "" );
	ch_printf( ch, "Your homepage is: %s\n\r",
		show_tilde( ch->pcdata->homepage ) );
	return;
    }

    if ( !str_cmp( argument, "clear" ) )
    {
	if ( ch->pcdata->homepage )
	  DISPOSE(ch->pcdata->homepage);
	ch->pcdata->homepage = str_dup("");
	send_to_char( "Homepage cleared.\n\r", ch );
	return;
    }

    if ( strstr( argument, "://" ) )
	strcpy( buf, argument );
    else
	sprintf( buf, "http://%s", argument );
    if ( strlen(buf) > 70 )
	buf[70] = '\0';

    hide_tilde( buf );
    if ( ch->pcdata->homepage )
      DISPOSE(ch->pcdata->homepage);
    ch->pcdata->homepage = str_dup(buf);
    send_to_char( "Homepage set.\n\r", ch );
}



/*
 * Set your personal description				-Thoric
 */
void do_description( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Monsters are too dumb to do that!\n\r", ch );
	return;	  
    }

    if ( !ch->desc )
    {
	bug( "do_description: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	   bug( "do_description: illegal substate", 0 );
	   return;

	case SUB_RESTRICTED:
	   send_to_char( "You cannot use this command from within another command.\n\r", ch );
	   return;

	case SUB_NONE:
	   ch->substate = SUB_PERSONAL_DESC;
	   ch->dest_buf = ch;
	   start_editing( ch, ch->description );
	   return;

	case SUB_PERSONAL_DESC:
	   STRFREE( ch->description );
	   ch->description = copy_buffer( ch );
	   stop_editing( ch );
	   return;	
    }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Mobs can't set bio's!\n\r", ch );
	return;	  
    }

    if ( !ch->desc )
    {
	bug( "do_bio: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	   bug( "do_bio: illegal substate", 0 );
	   return;
	  	   
	case SUB_RESTRICTED:
	   send_to_char( "You cannot use this command from within another command.\n\r", ch );
	   return;

	case SUB_NONE:
	   ch->substate = SUB_PERSONAL_BIO;
	   ch->dest_buf = ch;
	   start_editing( ch, ch->pcdata->bio );
	   return;

	case SUB_PERSONAL_BIO:
	   STRFREE( ch->pcdata->bio );
	   ch->pcdata->bio = copy_buffer( ch );
	   stop_editing( ch );
	   return;	
    }
}

void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    if ( IS_AFFECTED(ch, AFF_POSSESS) )
    {   
       send_to_char("You can't do that in your current state of mind!\n\r", ch);
       return;
    }

    
      ch_printf( ch,
	"You report: %d/%d hp %d/%d mv.\n\r",
	ch->hit,  ch->max_hit,
	ch->move, ch->max_move   );

    
      sprintf( buf, "$n reports: %d/%d hp %d/%d.",
	ch->hit,  ch->max_hit,
	ch->move, ch->max_move   );

    act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void do_prompt( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  
  if ( IS_NPC(ch) )
  {
    send_to_char( "NPC's can't change their prompt..\n\r", ch );
    return;
  }
  smash_tilde( argument );
  one_argument( argument, arg );
  if ( !*arg || !str_cmp( arg, "display" ) )
  {
    sprintf( buf, "%s\n\r", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)" : ch->pcdata->prompt );
    set_char_color( AT_WHITE, ch );
    write_to_descriptor( ch->desc->descriptor, "Your current prompt string:\n\r", 0 );
    write_to_descriptor( ch->desc->descriptor, buf, 0 );
    write_to_descriptor( ch->desc->descriptor, "Set prompt to what? (try help prompt)\n\r", 0 );
    /* ch_printf( ch, "%s\n\r", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)" : ch->pcdata->prompt ); */
    return;
  }

  send_to_char( "Replacing old prompt of:\n\r", ch );
  set_char_color( AT_WHITE, ch );
  ch_printf( ch, "%s\n\r", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)"
							      : ch->pcdata->prompt );

  if (ch->pcdata->prompt)
    STRFREE(ch->pcdata->prompt);

  if ( strlen(argument) > 128 )
    argument[128] = '\0';

  /* Can add a list of pre-set prompts here if wanted.. perhaps
     'prompt 1' brings up a different, pre-set prompt */
  if ( !str_cmp(arg, "default") )
    ch->pcdata->prompt = STRALLOC("");
  else
    ch->pcdata->prompt = STRALLOC(argument);
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_oldscore( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    SKILLTYPE   *skill;

    if ( IS_AFFECTED(ch, AFF_POSSESS) )
    {   
       send_to_char("You can't do that in your current state of mind!\n\r", ch);
       return;
    }

    if ( !IS_NPC( ch ) )
    {
       do_score( ch, argument );
       return;
    }

    set_char_color( AT_SCORE, ch );
    ch_printf( ch,
	"You are %s%s, level %d, %d years old (%d hours).\n\r",
	ch->name,
	IS_NPC(ch) ? "" : ch->pcdata->title,
	ch->top_level,
	get_age(ch),
	(get_age(ch) - 17) );

    if ( get_trust( ch ) != ch->top_level )
	ch_printf( ch, "You are trusted at level %d.\n\r",
	    get_trust( ch ) );

    if ( xIS_SET(ch->act, ACT_MOBINVIS) )
      ch_printf( ch, "You are mobinvis at level %d.\n\r",
            ch->mobinvis);

    
      ch_printf( ch,
	"You have %d/%d hit, %d/%d movement.\n\r",
	ch->hit,  ch->max_hit,
	ch->move, ch->max_move);

    ch_printf( ch,
	"You are carrying %d/%d items with weight %d/%d kg.\n\r",
	ch->carry_number, can_carry_n(ch),
	ch->carry_weight, can_carry_w(ch) );

    ch_printf( ch,
        "Str: %d  Sta: %d  Rec: %d  Int: %d  Bra: %d  Per: %d\n\r",
	get_curr_str(ch),
        get_curr_sta(ch),
        get_curr_rec(ch),
        get_curr_int(ch),
        get_curr_bra(ch),
        get_curr_per(ch) );

    if ( !IS_NPC(ch) )
    ch_printf( ch,
	"You have achieved %d glory during your life, and currently have %d.\n\r",
	ch->pcdata->quest_accum, ch->pcdata->quest_curr );

    ch_printf( ch,
        "Autoexit: %s   Autoloot: %s   Autosac: %s\n\r",
    (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
    (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
    (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOSAC) ) ? "yes" : "no" );

    switch( ch->mental_state / 10 )
    {
        default:   send_to_char( "You're completely messed up!\n\r", ch ); break;
        case -10:  send_to_char( "You're barely conscious.\n\r", ch ); break;
        case  -9:  send_to_char( "You can barely keep your eyes open.\n\r", ch ); break;
        case  -8:  send_to_char( "You're extremely drowsy.\n\r", ch ); break;
        case  -7:  send_to_char( "You feel very unmotivated.\n\r", ch ); break;
        case  -6:  send_to_char( "You feel sedated.\n\r", ch ); break;
        case  -5:  send_to_char( "You feel sleepy.\n\r", ch ); break;
        case  -4:  send_to_char( "You feel tired.\n\r", ch ); break;
        case  -3:  send_to_char( "You could use a rest.\n\r", ch ); break;
        case  -2:  send_to_char( "You feel a little under the weather.\n\r", ch ); break;
        case  -1:  send_to_char( "You feel fine.\n\r", ch ); break;
        case   0:  send_to_char( "You feel great.\n\r", ch ); break;
        case   1:  send_to_char( "You feel energetic.\n\r", ch ); break;
        case   2:  send_to_char( "Your mind is racing.\n\r", ch ); break;
        case   3:  send_to_char( "You can't think straight.\n\r", ch ); break;
        case   4:  send_to_char( "Your mind is going 100 miles an hour.\n\r", ch ); break;
        case   5:  send_to_char( "You're high as a kite.\n\r", ch ); break;
        case   6:  send_to_char( "Your mind and body are slipping appart.\n\r", ch ); break;
        case   7:  send_to_char( "Reality is slipping away.\n\r", ch ); break;
        case   8:  send_to_char( "You have no idea what is real, and what is not.\n\r", ch ); break;
        case   9:  send_to_char( "You feel immortal.\n\r", ch ); break;
        case  10:  send_to_char( "You are a Supreme Entity.\n\r", ch ); break;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "You are DEAD!!\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_char( "You are mortally wounded.\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_char( "You are incapacitated.\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_char( "You are stunned.\n\r",		ch );
	break;
    case POS_PRONE:
        send_to_char( "You are lying prone.\n\r",       ch );
	break;
    case POS_KNEELING:
        send_to_char( "You are kneeling.\n\r",          ch );
	break;
    case POS_STANDING:
	send_to_char( "You are standing.\n\r",		ch );
	break;
    case POS_MOUNTED:
	send_to_char( "Mounted.\n\r",			ch );
	break;
    case POS_SHOVE:
	send_to_char( "Being shoved.\n\r",		ch );
	break;
    case POS_DRAG:
	send_to_char( "Being dragged.\n\r",		ch );
	break;
    }

    if ( ch->first_affect )
    {
	send_to_char( "You are affected by:\n\r", ch );
	for ( paf = ch->first_affect; paf; paf = paf->next )
	    if ( (skill=get_skilltype(paf->type)) != NULL )
	{
	    ch_printf( ch, "Spell: '%s'", skill->name );

	    if ( ch->top_level >= 20 )
		ch_printf( ch,
		    " modifies %s by %d for %d rounds",
		    affect_loc_name( paf->location ),
		    paf->modifier,
		    paf->duration );

	    send_to_char( ".\n\r", ch );
	}
    }

    if ( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
    {
	ch_printf( ch, "WizInvis level: %d   WizInvis is %s\n\r",
			ch->pcdata->wizinvis,
            xIS_SET( ch->act, PLR_WIZINVIS ) ? "ON" : "OFF" );
	if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	  ch_printf( ch, "Room Range: %d - %d\n\r", ch->pcdata->r_range_lo,
					 	   ch->pcdata->r_range_hi	);
	if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
	  ch_printf( ch, "Obj Range : %d - %d\n\r", ch->pcdata->o_range_lo,
	  					   ch->pcdata->o_range_hi	);
	if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	  ch_printf( ch, "Mob Range : %d - %d\n\r", ch->pcdata->m_range_lo,
	  					   ch->pcdata->m_range_hi	);
    }

    return;
}


char * reduce_ratio( int a, int b )
{
    static char buf[MAX_STRING_LENGTH];
    float tmpA, tmpB;
    int x;

    strcpy( buf, "1:1" );
    if ( a == 0 && b == 0 ) return buf;

    for ( x = a; x > 0; x-- )
    {
        if ( a <= 0 || b <= 0 ) break;

        tmpA = ( (float)(a) / (float)(x) );
        tmpB = ( (float)(b) / (float)(x) );

        if ( (float)((int)(tmpA)) != (float)(tmpA) ) continue;
        if ( (float)((int)(tmpB)) != (float)(tmpB) ) continue;

        sprintf( buf, "%d:%d", (int)(tmpA), (int)(tmpB) );
        return buf;
    }

    sprintf( buf, "%d:%d", a, b );
    return buf;
}

int find_percent( int a, int b )
{
    int perc = 0;

    if ( b <= 0 ) return 0;

    perc = ( a * 100 ) / b;

    return perc;
}

int make_percent( int cur, int max )
{
    int perc = 0;

    if ( max == 0 ) return perc;

    perc = (int)( ((float)(cur)/(float)(max)) * (float)(100) );

    return perc;
}
