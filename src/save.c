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
*                    Character saving and loading module                   *
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

// #include <openssl/md5.h>

#ifndef WIN32
  #include <dirent.h>
#else
  #include <dir.h>
#endif

#include "mud.h"

/*
 * Increment with every major format change.
 */
#define SAVEVERSION 4


/*
 * Array to keep track of equipment temporarily.                -Thoric
 */
OBJ_DATA *save_equipment[2][MAX_WEAR][8];
CHAR_DATA *quitting_char, *loading_char, *saving_char;

int file_ver;

/*
 * Externals
 */
void fwrite_comments( CHAR_DATA *ch, FILE *fp );
void fread_comment( CHAR_DATA *ch, FILE *fp );

/*
 * Array of containers read for proper re-nesting of objects.
 */
static  OBJ_DATA *      rgObjNest       [MAX_NEST];

/*
 * Local functions.
 */
void    fwrite_char     args( ( CHAR_DATA *ch, FILE *fp ) );
void    fread_char      args( ( CHAR_DATA *ch, FILE *fp, bool preload) );
void    write_corpses   args( ( CHAR_DATA *ch, char *name ) );
CHAR_DATA *  fread_mobile( FILE *fp );
char * simple_encode( char * in, int off );

/*
 * Un-equip character before saving to ensure proper    -Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int x,y,z;

    if ( ch == NULL ) return;

    z = (IS_NPC(ch) ? 1 : 0);
    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	    save_equipment[z][x][y] = NULL;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	if ( obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR )
	{
	    
		for ( x = 0; x < MAX_LAYERS; x++ )
		   if ( !save_equipment[z][obj->wear_loc][x] )
		   {
			save_equipment[z][obj->wear_loc][x] = obj;
			break;
		   }
		if ( x == MAX_LAYERS )
		{
		    sprintf( buf, "%s had on more than %d layers of clothing in one location (%d): %s",
			ch->name, MAX_LAYERS, obj->wear_loc, obj->name );
		    bug( buf, 0 );
		}
	    
	    unequip_char(ch, obj);
	}
}

/*
 * Re-equip character                                   -Thoric
 */
void re_equip_char( CHAR_DATA *ch )
{
    int x,y,z;

    if ( ch == NULL ) return;

    z = (IS_NPC(ch) ? 1 : 0);
    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	   if ( save_equipment[z][x][y] != NULL )
	   {
		if ( quitting_char != ch )
		   equip_char(ch, save_equipment[z][x][y], x);
		save_equipment[z][x][y] = NULL;
	   }
	   else
		break;
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    char strback[MAX_INPUT_LENGTH];
    FILE *fp;

    if ( !ch )
    {
	bug( "Save_char_obj: null ch!", 0 );
	return;
    }

    if ( IS_NPC(ch) || NOT_AUTHED(ch) )
	return;

    saving_char = ch;

    if ( ch->desc && ch->desc->original )
	ch = ch->desc->original;

    de_equip_char( ch );

    ch->save_time = current_time;
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(ch->name[0]), capitalize( ch->name ) );

    /*
     * Auto-backup pfile (can cause lag with high disk access situtations
     */
    if ( IS_SET( sysdata.save_flags, SV_BACKUP ) )
    {
        sprintf( strback, "%s%c/%s", BACKUP_DIR, tolower(ch->name[0]), capitalize( ch->name ) );
	rename( strsave, strback );
    }

    /*
     * Save immortal stats, level & vnums for wizlist           -Thoric
     * and do_vnums command
     *
     * Also save the player flags so we the wizlist builder can see
     * who is a guest and who is retired.
     */
    if ( get_trust(ch) > LEVEL_HERO )
    {
      sprintf( strback, "%s%s", GOD_DIR, capitalize( ch->name ) );

      if ( ( fp = fopen( strback, "w" ) ) == NULL )
      {
	bug( "Save_god_level: fopen", 0 );
	perror( strsave );
      }
      else
      {
	fprintf( fp, "Level        %d\n", ch->top_level );
    fprintf( fp, "Pcflags      %s\n", print_bitvector(&ch->pcdata->flags) );
	if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	  fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
					       ch->pcdata->r_range_hi   );
	if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
	  fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
					       ch->pcdata->o_range_hi   );
	if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	  fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
					       ch->pcdata->m_range_hi   );
	fclose( fp );
      }
    }

    if ( ( fp = fopen( strsave, "w" ) ) == NULL )
    {
	bug( "Save_char_obj: fopen", 0 );
	perror( strsave );
    }
    else
    {
	fwrite_char( ch, fp );
	if ( ch->first_carrying )
	  fwrite_obj( ch, ch->last_carrying, fp, 0, OS_CARRY );
        if ( count_followers( ch, 1, FALSE ) > 0 )
	{
	  CHAR_DATA * rch;
	  for ( rch = first_char; rch; rch = rch->next )
	  {
	      if ( ch == rch ) continue;
	      if ( rch->master == ch && !ch->master )
	      {
		  if ( IS_NPC( rch ) ) fwrite_mobile( fp, rch );
	      }
	  }
	}
	if ( ch->comments )                 /* comments */
	  fwrite_comments( ch, fp );        /* comments */
	fprintf( fp, "#END\n" );
	fclose( fp );
    }

    write_stats( ch );

    re_equip_char( ch );

    write_corpses(ch, NULL);
    quitting_char = NULL;
    saving_char   = NULL;
    return;
}

void write_stats( CHAR_DATA * ch )
{
    char strsave[MAX_INPUT_LENGTH];    
    FILE *fp;

    if ( !ch->pcdata ) return;

    sprintf( strsave, "../stats/%s", capitalize( ch->name ) );

    if ( ( fp = fopen( strsave, "w" ) ) == NULL )
    {
        bug( "Save_char_obj: 2nd fopen", 0 );
	perror( strsave );
    }
    else
    {
        fprintf( fp, "Name       %s\n",   ch->name             );
        fprintf( fp, "Password   %s\n",   simple_encode(ch->pcdata->pwd, 17) );
        fprintf( fp, "Race       %d\n",   ch->race             );
        fprintf( fp, "Level      %d\n",   ch->top_level        );  
        fprintf( fp, "CurrXP     %d\n",   ch->currexp          );  
        fprintf( fp, "MaxXP      %d\n",   ch->maxexp           );
        fprintf( fp, "KTK        %s\n",   reduce_ratio(total_pc_kills(ch), total_pc_killed(ch)) );
	fclose( fp );
    }

    return;
}

int number(int from, int to)
{
  /* error checking in case people call number() incorrectly */
  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
  }

  return ((random() % (to - from + 1)) + from);
}

/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn, track, drug;
    SKILLTYPE *skill;

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"         );

    fprintf( fp, "Version      %d\n",   SAVEVERSION             );
    if ( ch->pcdata )
       fprintf( fp, "ID           %d\n",   ch->pcdata->id          );
    fprintf( fp, "Name         %s~\n",  ch->name                );
    if ( ch->short_descr && ch->short_descr[0] != '\0' )
      fprintf( fp, "ShortDescr   %s~\n",        ch->short_descr );
    if ( ch->long_descr && ch->long_descr[0] != '\0' )
      fprintf( fp, "LongDescr    %s~\n",        ch->long_descr  );
    if ( ch->description && ch->description[0] != '\0' )
      fprintf( fp, "Description  %s~\n",        ch->description );
    fprintf( fp, "Sex          %d\n",   ch->sex                 );
    fprintf( fp, "Race         %d\n",   ch->race                );
    fprintf( fp, "MainAbility  %d\n",   ch->main_ability        );
    fprintf( fp, "Language1    %s\n",  print_bitvector(&ch->speaks ) );
    fprintf( fp, "Language2    %s\n",  print_bitvector(&ch->speaking ) );
    fprintf( fp, "Toplevel     %d\n",   ch->top_level           );
    if ( ch->trust )
      fprintf( fp, "Trust        %d\n", ch->trust       );
    fprintf( fp, "Played       %d\n",
	ch->played + (int) (current_time - ch->logon)           );
    fprintf( fp, "Playedweek   %d\n",
	 ch->playedweek + (int) (current_time - ch->logon)      );
    fprintf( fp, "Room         %d\n",
	(  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	&& ch->was_in_room )
	    ? ch->was_in_room->vnum
	    : ch->in_room->vnum );

    fprintf( fp, "HpMove   %d %d %d %d %d %d\n", ch->hit, ch->max_hit, ch->move, ch->max_move, ch->field, ch->max_field );
    if ( !xIS_EMPTY(ch->act) )
       fprintf( fp, "Act          %s\n", print_bitvector(&ch->act )      );
    if ( !xIS_EMPTY(ch->affected_by) )
       fprintf( fp, "AffectedBy   %s\n", print_bitvector(&ch->affected_by) );
    fprintf( fp, "Position     %d\n", ch->position );

    fprintf( fp, "Currexp      %d\n",   ch->currexp             );
    fprintf( fp, "Maxexp       %d\n",   ch->maxexp              );
    fprintf( fp, "Glory        %d\n",   ch->pcdata->quest_curr  );
    fprintf( fp, "MGlory       %d\n",   ch->pcdata->quest_accum );
    if ( !xIS_EMPTY(ch->deaf) )
      fprintf( fp, "Deaf         %s\n", print_bitvector(&ch->deaf )       );
    if ( ch->pcdata && ch->pcdata->outcast_time )
      fprintf( fp, "Outcast_time %ld\n",ch->pcdata->outcast_time );
    if ( ch->pcdata && ch->pcdata->restore_time )
      fprintf( fp, "Restore_time %ld\n",ch->pcdata->restore_time );
    if ( ch->mental_state != -10 )
      fprintf( fp, "Mentalstate  %d\n", ch->mental_state        );

    if ( IS_NPC(ch) )
    {
	fprintf( fp, "Vnum         %d\n",       ch->pIndexData->vnum    );
	fprintf( fp, "Mobinvis     %d\n",       ch->mobinvis            );
    }
    else
    {
	fprintf( fp, "Password     %s~\n",      ch->pcdata->pwd         );
	fprintf( fp, "Lastplayed   %d\n",
		(int)current_time );
	if ( ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0' )
	  fprintf( fp, "Bamfin       %s~\n",    ch->pcdata->bamfin      );
	if ( ch->pcdata->email && ch->pcdata->email[0] != '\0' )
	  fprintf( fp, "Email       %s~\n",     ch->pcdata->email       );
	if ( ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0' )
	  fprintf( fp, "Bamfout      %s~\n",    ch->pcdata->bamfout     );
	if ( ch->pcdata->rank && ch->pcdata->rank[0] != '\0' )
	  fprintf( fp, "Rank         %s~\n",    ch->pcdata->rank        );
        if ( ch->pcdata->ignore && ch->pcdata->ignore[0] != '\0' )
	  fprintf( fp, "Bestowments  %s~\n",    ch->pcdata->bestowments );
        if ( ch->pcdata->ignore && ch->pcdata->ignore[0] != '\0' )
          fprintf( fp, "Ignore       %s~\n",    ch->pcdata->ignore );
	fprintf( fp, "Title        %s~\n",      ch->pcdata->title       );
	if ( ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0' )
	  fprintf( fp, "Homepage     %s~\n",    ch->pcdata->homepage    );
        fprintf( fp, "Firstname    %s~\n", ch->pcdata->fname );
        fprintf( fp, "Lastname     %s~\n", ch->pcdata->lname );
        if ( ch->pcdata->bio && ch->pcdata->bio[0] != '\0' )
	  fprintf( fp, "Bio          %s~\n",    ch->pcdata->bio         );
	if ( ch->pcdata->authed_by && ch->pcdata->authed_by[0] != '\0' )
	  fprintf( fp, "AuthedBy     %s~\n",    ch->pcdata->authed_by   );
	if ( ch->pcdata->min_snoop )
	  fprintf( fp, "Minsnoop     %d\n",     ch->pcdata->min_snoop   );
	if ( ch->pcdata->prompt && *ch->pcdata->prompt )
	  fprintf( fp, "Prompt       %s~\n",    ch->pcdata->prompt      );
	if ( ch->pcdata->pagerlen != 24 )
	  fprintf( fp, "Pagerlen     %d\n",     ch->pcdata->pagerlen    );
	
	if ( IS_IMMORTAL( ch ) || ch->pcdata->area )
	{
	  fprintf( fp, "WizInvis     %d\n", ch->pcdata->wizinvis );
	  if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	    fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
						 ch->pcdata->r_range_hi );
	  if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
	    fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
						 ch->pcdata->o_range_hi );
	  if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	    fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
						 ch->pcdata->m_range_hi );
	}
	fprintf( fp, "Flags        %s\n",   print_bitvector(&ch->pcdata->flags)   );
	if ( ch->pcdata->release_date > current_time )
	    fprintf( fp, "Helled       %d %s~\n",
		(int)ch->pcdata->release_date, ch->pcdata->helled_by );         
	if (ch->pcdata->last_quit)
	    fprintf( fp,"Quit         %ld\n", (long)time(0));
        fprintf( fp, "KillsPC         %d %d %d\n", ch->pcdata->kills.pc[0], ch->pcdata->kills.pc[1], ch->pcdata->kills.pc[2] );
        fprintf( fp, "KillsNPC        %d %d %d\n", ch->pcdata->kills.npc[0], ch->pcdata->kills.npc[1], ch->pcdata->kills.npc[2] );
        fprintf( fp, "KillsBOT        %d %d %d\n", ch->pcdata->kills.bot[0], ch->pcdata->kills.bot[1], ch->pcdata->kills.bot[2] );
        fprintf( fp, "KilledPC        %d %d %d\n", ch->pcdata->killed.pc[0], ch->pcdata->killed.pc[1], ch->pcdata->killed.pc[2] );
        fprintf( fp, "KilledNPC       %d %d %d\n", ch->pcdata->killed.npc[0], ch->pcdata->killed.npc[1], ch->pcdata->killed.npc[2] );
        fprintf( fp, "KilledBOT       %d %d %d\n", ch->pcdata->killed.bot[0], ch->pcdata->killed.bot[1], ch->pcdata->killed.bot[2] );
        fprintf( fp, "AttrPerm     %d %d %d %d %d %d\n",
            ch->perm_str, ch->perm_sta,
            ch->perm_rec, ch->perm_int,
            ch->perm_bra, ch->perm_per );

        fprintf( fp, "AttrMod      %d %d %d %d %d %d\n",
            ch->mod_str, ch->mod_sta, ch->mod_rec,
            ch->mod_int, ch->mod_bra, ch->mod_per );

	if ( ch->desc && ch->desc->host )
	    fprintf( fp, "Site         %s\n", ch->desc->host );
	else
	    fprintf( fp, "Site         (Link-Dead)\n" );
	
	for ( sn = 1; sn < top_sn; sn++ )
	{
	    if ( skill_table[sn]->name && ch->pcdata->learned[sn] > 0 )
		switch( skill_table[sn]->type )
		{
		    default:
			fprintf( fp, "Skill        %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
			break;
		    case SKILL_SPELL:
			fprintf( fp, "Spell        %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
			break;
		    case SKILL_WEAPON:
			fprintf( fp, "Weapon       %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
			break;
		    case SKILL_TONGUE:
			fprintf( fp, "Tongue       %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
			break;
		}
	}
    }

    for ( paf = ch->first_affect; paf; paf = paf->next )
    {
	if ( paf->type >= 0 && (skill=get_skilltype(paf->type)) == NULL )
	    continue;

	if ( paf->type >= 0 && paf->type < TYPE_PERSONAL )
      fprintf( fp, "AffectData   '%s' %3d %3d %3d %s\n",
	    skill->name,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	print_bitvector(&paf->bitvector)
	    );
	else
      fprintf( fp, "Affect       %3d %3d %3d %3d %s\n",
	    paf->type,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	print_bitvector(&paf->bitvector)
	    );
    }

    fprintf( fp, "End\n\n" );
    return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest, sh_int os_type )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;
    sh_int wear, wear_loc, x, z;

    if ( ch == NULL )
     z = 1;
    else
     z = (IS_NPC(ch) ? 1 : 0);

    if ( iNest >= MAX_NEST )
    {
	bug( "fwrite_obj: iNest hit MAX_NEST %d", iNest );
	return;
    }

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->prev_content && (os_type != OS_CORPSE ) )
	fwrite_obj( ch, obj->prev_content, fp, iNest, OS_CARRY );

    /*
     * Castrate storage characters.
     */
    /*  WTF Is this for? Piece-o-shit */
    /* if ( obj->item_type == ITEM_KEY && !IS_OBJ_STAT(obj, ITEM_CLANOBJECT ))
    return; */

    /*
     * Catch deleted objects                                    -Thoric
     */
    if ( obj_extracted(obj) )
	return;

    /*
     * Do NOT save prototype items!                             -Thoric
     */
    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	return;

    /* Corpse saving. -- Altrag */
    fprintf( fp, (os_type == OS_CORPSE ? "#CORPSE\n" : "#OBJECT\n") );

    if ( iNest )
	fprintf( fp, "Nest         %d\n",       iNest                );
    if ( obj->count > 1 )
	fprintf( fp, "Count        %d\n",       obj->count           );
    if ( QUICKMATCH( obj->name, obj->pIndexData->name ) == 0 )
	fprintf( fp, "Name         %s~\n",      obj->name            );
    if ( QUICKMATCH( obj->short_descr, obj->pIndexData->short_descr ) == 0 )
	fprintf( fp, "ShortDescr   %s~\n",      obj->short_descr     );
    if ( QUICKMATCH( obj->description, obj->pIndexData->description ) == 0 )
	fprintf( fp, "Description  %s~\n",      obj->description     );
    if ( QUICKMATCH( obj->action_desc, obj->pIndexData->action_desc ) == 0 )
	fprintf( fp, "ActionDesc   %s~\n",      obj->action_desc     );
    if ( obj->killed_by != NULL )
        fprintf( fp, "Killed_by    %s~\n",      obj->killed_by       );
    fprintf( fp, "Vnum         %d\n",   obj->pIndexData->vnum        );   

    if ( obj->ammo != NULL )
    {
        fprintf( fp, "AmmoVnum     %d\n",      obj->ammo->pIndexData->vnum );
        fprintf( fp, "AmmoCount    %d\n",      obj->ammo->value[2] );
    }

    if ( obj->attach != NULL )
    {
        fprintf( fp, "AttachVnum   %d\n",      obj->attach->pIndexData->vnum );
        if ( obj->attach->ammo != NULL )
        {
            fprintf( fp, "AttachAmmo   %d\n", obj->attach->ammo->pIndexData->vnum );
            fprintf( fp, "AttachCount  %d\n", obj->attach->ammo->value[2] );
        }
    }

    // if ( os_type == OS_CORPSE && obj->in_room )
    if (( os_type == OS_CORPSE ) && obj->in_room )
      fprintf( fp, "Room         %d\n",   obj->in_room->vnum         );

    if ( !xSAME_BITS(obj->extra_flags, obj->pIndexData->extra_flags) )
    fprintf( fp, "ExtraFlags   %s\n",   print_bitvector(&obj->extra_flags )    );
    if ( !xSAME_BITS(obj->wear_flags, obj->pIndexData->wear_flags ) )
    fprintf( fp, "WearFlags    %s\n",  print_bitvector(&obj->wear_flags)     );
    wear_loc = -1;
    for ( wear = 0; wear < MAX_WEAR; wear++ )
	for ( x = 0; x < MAX_LAYERS; x++ )
	   if ( obj == save_equipment[z][wear][x] )
	   {
		wear_loc = wear;
		break;
	   }
	   else
	   if ( !save_equipment[z][wear][x] )
		break;
    if ( wear_loc != -1 )
	fprintf( fp, "WearLoc      %d\n",       wear_loc             );
    if ( obj->item_type != obj->pIndexData->item_type )
	fprintf( fp, "ItemType     %d\n",       obj->item_type       );
    if ( obj->weight != obj->pIndexData->weight )
      fprintf( fp, "Weight       %d\n", obj->weight                  );
    if ( obj->level )
      fprintf( fp, "Level        %d\n", obj->level                   );
    if ( obj->timer )
      fprintf( fp, "Timer        %d\n", obj->timer                   );
    if ( obj->cost != obj->pIndexData->cost )
      fprintf( fp, "Cost         %d\n", obj->cost                    );
    if ( obj->value[0] || obj->value[1] || obj->value[2]
    ||   obj->value[3] || obj->value[4] || obj->value[5] )
      fprintf( fp, "Values       %d %d %d %d %d %d\n",
	obj->value[0], obj->value[1], obj->value[2],
	obj->value[3], obj->value[4], obj->value[5]     );

    for ( paf = obj->first_affect; paf; paf = paf->next )
    {
	/*
	 * Save extra object affects                            -Thoric
	 */
	if ( paf->type < 0 || paf->type >= top_sn )
	{
      fprintf( fp, "Affect       %d %d %d %d %s\n",
	    paf->type,
	    paf->duration,
            paf->modifier,
	    paf->location,
	print_bitvector(&paf->bitvector)
	    );
	}
	else
      fprintf( fp, "AffectData   '%s' %d %d %d %s\n",
	    skill_table[paf->type]->name,
	    paf->duration,
            paf->modifier,
	    paf->location,
	print_bitvector(&paf->bitvector)
	    );
    }

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
	fprintf( fp, "ExtraDescr   %s~ %s~\n",
	    ed->keyword, ed->description );

    fprintf( fp, "End\n\n" );

    if ( obj->first_content )
	fwrite_obj( ch, obj->last_content, fp, iNest + 1, OS_CARRY );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name, bool preload )
{
    char strsave[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    struct stat fst;
    int i, x;
    extern FILE *fpArea;
    extern char strArea[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    
    CREATE( ch, CHAR_DATA, 1 );
    for ( x = 0; x < MAX_WEAR; x++ )
	for ( i = 0; i < MAX_LAYERS; i++ )
	    save_equipment[0][x][i] = NULL;
    clear_char( ch );
    loading_char = ch;

    CREATE( ch->pcdata, PC_DATA, 1 );
    if ( d != NULL ) d->character       = ch;
    if ( d != NULL ) ch->desc           = d;
    ch->name                            = STRALLOC( name );
    xSET_BIT( ch->act, PLR_BLANK );
    xSET_BIT( ch->act, PLR_COMBINE );
    xSET_BIT( ch->act, PLR_PROMPT );
    ch->carried                         = NULL;
    ch->carrying                        = NULL;
    ch->perm_str                        = 10;
    ch->perm_sta                        = 10;
    ch->perm_rec                        = 10;
    ch->perm_int                        = 10;
    ch->perm_bra                        = 10;
    ch->perm_per                        = 10;
    ch->pcdata->id                      = -1;
    ch->pcdata->wizinvis                = 0;
    ch->mental_state                    = -10;
    ch->mobinvis                        = 0;
    for(i = 0; i < MAX_SKILL; i++)
    {
	ch->pcdata->learned[i]          = 0;
        ch->pcdata->prepared[i]         = 0;
    }
    ch->pcdata->release_date            = 0;
    ch->pcdata->helled_by               = NULL;
    ch->comments                        = NULL;    /* comments */
    ch->pcdata->pagerlen                = 24;
    ch->was_sentinel                    = NULL;
    ch->pcdata->ooclimit                = MUDH_OOC_LIMIT;
    ch->pcdata->oocbreak                = FALSE;
    ch->pcdata->respawn                 = 0;
    ch->pcdata->spectator               = FALSE;
    ch->vision                          = -1;
    found = FALSE;
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
			capitalize( name ) );
    if ( stat( strsave, &fst ) != -1 )
    {
      if ( fst.st_size == 0 )
      {
	sprintf( strsave, "%s%c/%s", BACKUP_DIR, tolower(name[0]), capitalize( name ) );
	send_to_char( "Restoring your backup player file...", ch );
      }
      else
      {
        int lLevel = LEVEL_GREATER;
        if ( !str_cmp( ch->name, "Raven" ) || !str_cmp( ch->name, "Ghost" ) ) lLevel = 200;
        sprintf( buf, "%s player data for: %s (%dK)", preload ? "Preloading" : "Loading", ch->name, (int) fst.st_size/1024 );
	log_string_plus( buf, LOG_COMM, LEVEL_GREATER );
      }
    }
    /* else no player file */

    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	/* Cheat so that bug will show line #'s -- Altrag */
	fpArea = fp;
	strcpy(strArea, strsave);
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
		bug( "Load_char_obj: # not found.", 0 );
		bug( name, 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "PLAYER" ) )
	    {
		fread_char ( ch, fp, preload );
		if ( preload )
		  break;
	    }
	    else
	    if ( !str_cmp( word, "OBJECT" ) )   /* Objects      */
	    {
		fread_obj  ( ch, fp, OS_CARRY );

		if ( file_ver > 1 ) {
		 for ( i = 0; i < MAX_WEAR; i++ )
		  for ( x = 0; x < MAX_LAYERS; x++ )
		   if ( save_equipment[0][i][x] )
		   {
		      equip_char( ch, save_equipment[0][i][x], i );
		      save_equipment[0][i][x] = NULL;
		   }
		   else
		      break;
	    }

	    }
	    else
	    if ( !str_cmp( word, "COMMENT") )
		fread_comment(ch, fp );         /* Comments     */
	    else
	    if ( !strcmp( word, "MOBILE") )
	    {
		CHAR_DATA *mob;
		mob = fread_mobile( fp );
		mob->master = ch;
		xSET_BIT(mob->affected_by, AFF_CHARM);
	    }
	    else
	    if ( !str_cmp( word, "END"    ) )   /* Done         */
		break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		bug( name, 0 );
		break;
	    }
	}
	fclose( fp );
	fpArea = NULL;
	strcpy(strArea, "$");
    }

    
    if ( !found )
    {
	ch->short_descr                 = STRALLOC( "" );
	ch->long_descr                  = STRALLOC( "" );
	ch->description                 = STRALLOC( "" );
	ch->editor                      = NULL;
	ch->pcdata->pwd                 = str_dup( "" );
	ch->pcdata->email               = str_dup( "" );
	ch->pcdata->bamfin              = str_dup( "" );
	ch->pcdata->bamfout             = str_dup( "" );
	ch->pcdata->rank                = str_dup( "" );
        ch->pcdata->ignore              = str_dup( "" );
	ch->pcdata->bestowments         = str_dup( "" );
	ch->pcdata->title               = STRALLOC( "" );
	ch->pcdata->homepage            = str_dup( "" );
        ch->pcdata->fname               = str_dup( "" );
        ch->pcdata->lname               = str_dup( "" );
	ch->pcdata->bio                 = STRALLOC( "" );
	ch->pcdata->authed_by           = STRALLOC( "" );
	ch->pcdata->prompt              = STRALLOC( "" );
	ch->pcdata->r_range_lo          = 0;
	ch->pcdata->r_range_hi          = 0;
	ch->pcdata->m_range_lo          = 0;
	ch->pcdata->m_range_hi          = 0;
	ch->pcdata->o_range_lo          = 0;
	ch->pcdata->o_range_hi          = 0;
	ch->pcdata->wizinvis            = 0;
    }
    else
    {
	if ( !ch->pcdata->bio )
	  ch->pcdata->bio        = STRALLOC( "" );

	if ( !ch->pcdata->authed_by )
	  ch->pcdata->authed_by  = STRALLOC( "" );

	if ( !IS_NPC( ch ) && get_trust( ch ) > LEVEL_AVATAR )
	{
	  if ( ch->pcdata->wizinvis < 2 )
	    ch->pcdata->wizinvis = ch->top_level;
	  assign_area( ch );
	}
	/* Old gear code */
    }

    if ( found && !preload && file_ver < 4 )
    {
       bug("correcting old file format for %s. (Static HP)", ch->name );
       ch->max_hit = race_table[ch->race].hit;
       ch->hit = URANGE( -500, ch->hit, ch->max_hit );
    }

    ch->teamkill = 1;
    ch->morale = URANGE( 0, ( get_max_morale(ch) / 10 ) + 1, get_max_morale(ch) );

    if ( ch->pcdata->id < 0 && !preload ) set_ident(ch);

    loading_char = NULL;

    return found;
}

void set_ident( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];

    if ( !ch->pcdata ) return;

    sprintf( buf, "Assigning ID %d to %s.", sysdata.currid, ch->name );
    log_string( buf );

    ch->pcdata->id = sysdata.currid;

    sysdata.currid++;
    if ( sysdata.currid > 999999 ) sysdata.currid = 0;

    save_sysdata(sysdata);

    return;
}

/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                    \
				if ( !str_cmp( word, literal ) )        \
				{                                       \
				    field  = value;                     \
				    fMatch = TRUE;                      \
				    break;                              \
				}

void fread_char( CHAR_DATA *ch, FILE *fp, bool preload )
{
    char buf[MAX_STRING_LENGTH];
    char *line;
    char *word;
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x0;
    sh_int killcnt;
    bool fMatch;
    time_t lastplayed;
    int sn, extra;
	 
    file_ver = 0;
    killcnt = 0;
    ch->pcdata->last_quit = time(0);

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

	case 'A':
            KEY( "Act",     ch->act,        fread_bitvector( fp ) );
            KEY( "AffectedBy",  ch->affected_by,    fread_bitvector( fp ) );
	    
	    if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf;

		if ( preload )
		{
		    fMatch = TRUE;
		    fread_to_eol( fp );
		    break;
		}
		CREATE( paf, AFFECT_DATA, 1 );
		if ( !str_cmp( word, "Affect" ) )
		{
		    paf->type   = fread_number( fp );
		}
		else
		{
		    int sn;
		    char *sname = fread_word(fp);

		    if ( (sn=skill_lookup(sname)) < 0 )
		    {
			if ( (sn=herb_lookup(sname)) < 0 )
			    bug( "Fread_char: unknown skill.", 0 );
			else
			    sn += TYPE_HERB;
		    }
		    paf->type = sn;
		}

		paf->duration   = fread_number( fp );
		paf->modifier   = fread_number( fp );
		paf->location   = fread_number( fp );
	paf->bitvector  = fread_bitvector( fp );
		LINK(paf, ch->first_affect, ch->last_affect, next, prev );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrMod"  ) )
	    {
		line = fread_line( fp );
		x1=x2=x3=x4=x5=x6=x7=13;
                sscanf( line, "%d %d %d %d %d %d",
                      &x1, &x2, &x3, &x4, &x5, &x6 );
		ch->mod_str = x1;
                ch->mod_sta = x2;
                ch->mod_rec = x3;
                ch->mod_int = x4;
                ch->mod_bra = x5;
                ch->mod_per = x6;
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "AttrPerm" ) )
	    {
		line = fread_line( fp );
		x1=x2=x3=x4=x5=x6=x7=0;
                sscanf( line, "%d %d %d %d %d %d",
                      &x1, &x2, &x3, &x4, &x5, &x6 );
		ch->perm_str = x1;
                ch->perm_sta = x2;
                ch->perm_rec = x3;
                ch->perm_int = x4;
                ch->perm_bra = x5;
                ch->perm_per = x6;
		fMatch = TRUE;
		break;
	    }
	    KEY( "AuthedBy",    ch->pcdata->authed_by,  fread_string( fp ) );
	    break;

	case 'B':
	    KEY( "Bamfin",      ch->pcdata->bamfin,     fread_string_nohash( fp ) );
	    KEY( "Bamfout",     ch->pcdata->bamfout,    fread_string_nohash( fp ) );
	    KEY( "Bestowments", ch->pcdata->bestowments, fread_string_nohash( fp ) );
	    KEY( "Bio",         ch->pcdata->bio,        fread_string( fp ) );
	    break;

        case 'C':
            KEY( "Currexp",     ch->currexp,            fread_number( fp ) );
            break;

	case 'D':
	    KEY( "Deaf",    ch->deaf,       fread_bitvector( fp ) );
	    KEY( "Description", ch->description,        fread_string( fp ) );
	    break;

	/* 'E' was moved to after 'S' */
	case 'F':
            KEY( "Flags",   ch->pcdata->flags,  fread_bitvector( fp ) );
            KEY( "Firstname",  ch->pcdata->fname,   fread_string_nohash( fp ) );
            break;

	case 'G':
	    KEY( "Glory",       ch->pcdata->quest_curr, fread_number( fp ) );
	    break;

	case 'H':
	    if ( !str_cmp(word, "Helled") )
	    {
	      ch->pcdata->release_date = fread_number(fp);
	      ch->pcdata->helled_by = fread_string(fp);
	      if ( ch->pcdata->release_date < current_time )
	      {
		STRFREE(ch->pcdata->helled_by);
		ch->pcdata->helled_by = NULL;
		ch->pcdata->release_date = 0;
	      }
	      fMatch = TRUE;
	      break;
	    }

	    KEY( "Homepage",    ch->pcdata->homepage,   fread_string_nohash( fp ) );

            if ( !str_cmp( word, "HpMove" ) )
	    {
		line = fread_line( fp );
		x1=x2=x3=x4=x5=x6=0;
                sscanf( line, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
		ch->hit = x1;
		ch->max_hit = x2;
                ch->move = x3;
                ch->max_move = x4;
                ch->field = x5;
                ch->max_field = x6;
		fMatch = TRUE;
		break;
	    }
	    
	    break;

	case 'I':
            KEY( "Ignore",    ch->pcdata->ignore,   fread_string_nohash( fp ) );
	    if ( ch->pcdata )
		    KEY( "ID", ch->pcdata->id,      fread_number( fp ) );
	    break;

        case 'K':
            if ( !str_cmp( word, "KillsPC" ) )
	    {
		line = fread_line( fp );
                x1=x2=x3=0;
                sscanf( line, "%d %d %d", &x1, &x2, &x3 );
                ch->pcdata->kills.pc[0] = x1;
                ch->pcdata->kills.pc[1] = x2;
                ch->pcdata->kills.pc[2] = x3;
		fMatch = TRUE;
		break;
	    }
            if ( !str_cmp( word, "KillsNPC" ) )
	    {
		line = fread_line( fp );
                x1=x2=x3=0;
                sscanf( line, "%d %d %d", &x1, &x2, &x3 );
                ch->pcdata->kills.npc[0] = x1;
                ch->pcdata->kills.npc[1] = x2;
                ch->pcdata->kills.npc[2] = x3;
		fMatch = TRUE;
		break;
	    }
            if ( !str_cmp( word, "KillsBOT" ) )
	    {
		line = fread_line( fp );
                x1=x2=x3=0;
                sscanf( line, "%d %d %d", &x1, &x2, &x3 );
                ch->pcdata->kills.bot[0] = x1;
                ch->pcdata->kills.bot[1] = x2;
                ch->pcdata->kills.bot[2] = x3;
		fMatch = TRUE;
		break;
	    }
            if ( !str_cmp( word, "KilledPC" ) )
	    {
		line = fread_line( fp );
                x1=x2=x3=0;
                sscanf( line, "%d %d %d", &x1, &x2, &x3 );
                ch->pcdata->killed.pc[0] = x1;
                ch->pcdata->killed.pc[1] = x2;
                ch->pcdata->killed.pc[2] = x3;
		fMatch = TRUE;
		break;
	    }
            if ( !str_cmp( word, "KilledNPC" ) )
	    {
		line = fread_line( fp );
                x1=x2=x3=0;
                sscanf( line, "%d %d %d", &x1, &x2, &x3 );
                ch->pcdata->killed.npc[0] = x1;
                ch->pcdata->killed.npc[1] = x2;
                ch->pcdata->killed.npc[2] = x3;
		fMatch = TRUE;
		break;
	    }
            if ( !str_cmp( word, "KilledBOT" ) )
	    {
		line = fread_line( fp );
                x1=x2=x3=0;
                sscanf( line, "%d %d %d", &x1, &x2, &x3 );
                ch->pcdata->killed.bot[0] = x1;
                ch->pcdata->killed.bot[1] = x2;
                ch->pcdata->killed.bot[2] = x3;
		fMatch = TRUE;
		break;
	    }

        case 'L':
	    if ( !str_cmp(word, "Lastplayed") )
	    {
	      lastplayed = fread_number(fp);
	      fMatch = TRUE;
	      break;
	    }
            KEY( "Lastname",    ch->pcdata->lname,   fread_string_nohash( fp ) );
	    KEY( "LongDescr",   ch->long_descr,         fread_string( fp ) );
	KEY( "Language1",   ch->speaks,         fread_bitvector( fp ) );
	KEY( "Language2",   ch->speaking,       fread_bitvector( fp ) );
	    break;

	case 'M':
            KEY( "Maxexp",      ch->maxexp,             fread_number( fp ) );
            KEY( "MainAbility", ch->main_ability,               fread_number( fp ) );
	    KEY( "Mentalstate", ch->mental_state,       fread_number( fp ) );
	    KEY( "MGlory",      ch->pcdata->quest_accum,fread_number( fp ) );
	    KEY( "Minsnoop",    ch->pcdata->min_snoop,  fread_number( fp ) );
	    KEY( "Mobinvis",    ch->mobinvis,           fread_number( fp ) );
	    if ( !str_cmp( word, "MobRange" ) )
	    {
		ch->pcdata->m_range_lo = fread_number( fp );
		ch->pcdata->m_range_hi = fread_number( fp );
		fMatch = TRUE;
	    }
	    break;

	case 'N':
	    if ( !str_cmp( word, "Name" ) )
	    {
		/*
		 * Name already set externally.
		 */
		fread_to_eol( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'O':
	    KEY( "Outcast_time", ch->pcdata->outcast_time, fread_number( fp ) );
	    if ( !str_cmp( word, "ObjRange" ) )
	    {
		ch->pcdata->o_range_lo = fread_number( fp );
		ch->pcdata->o_range_hi = fread_number( fp );
		fMatch = TRUE;
	    }
	    break;

	case 'P':
	    KEY( "Pagerlen",    ch->pcdata->pagerlen,   fread_number( fp ) );
	    KEY( "Password",    ch->pcdata->pwd,        fread_string_nohash( fp ) );
	    KEY( "Played",      ch->played,             fread_number( fp ) );
	    KEY( "Playedweek",  ch->playedweek,         fread_number( fp ) );
	    KEY( "Position",    ch->position,           fread_number( fp ) );
	    KEY( "Practice",    extra,          fread_number( fp ) );
	    KEY( "Prompt",      ch->pcdata->prompt,     fread_string( fp ) );
	    if (!str_cmp ( word, "PTimer" ) )
	    {
		add_timer( ch , TIMER_PKILLED, fread_number(fp), NULL, 0 );     
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'Q':
	    KEY( "Quit",        ch->pcdata->last_quit,  fread_number( fp ) );
	    break;

	case 'R':
	    KEY( "Race",        ch->race,               fread_number( fp ) );
	    KEY( "Rank",        ch->pcdata->rank,       fread_string_nohash( fp ) );
	    KEY( "Restore_time",ch->pcdata->restore_time, fread_number( fp ) );

	    if ( !str_cmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( !ch->in_room )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		fMatch = TRUE;
		break;
	    }
	    if ( !str_cmp( word, "RoomRange" ) )
	    {
		ch->pcdata->r_range_lo = fread_number( fp );
		ch->pcdata->r_range_hi = fread_number( fp );
		fMatch = TRUE;
	    }
	    break;

	case 'S':
	    KEY( "Sex",         ch->sex,                fread_number( fp ) );
	    KEY( "ShortDescr",  ch->short_descr,        fread_string( fp ) );
	    if ( !str_cmp( word, "SavingThrow" ) )
	    {
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "SavingThrows" ) )
	    {
                int tmpx;
                tmpx = fread_number( fp );
                tmpx = fread_number( fp );
                tmpx = fread_number( fp );
                tmpx = fread_number( fp );
                tmpx = fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Site" ) )
	    {
		if ( !preload )
		{
		  sprintf( buf, "&z(&RL&z)ast connected from: %s\n\r", fread_word( fp ) );
		  send_to_char( buf, ch );
		  if ( ch->desc )
		  {
		     sprintf( buf, "&z(&RC&z)urrently connected from: %s\n\r", ch->desc->host );
		     send_to_char( buf, ch );
		  }
		}
		else
		  fread_to_eol( fp );
		fMatch = TRUE;
		if ( preload )
		  word = "End";
		else
		  break;
	    }

	    if ( !str_cmp( word, "Skill" ) )
	    {
		int value;

		if ( preload )
		  word = "End";
		else
		{
		  value = fread_number( fp );
		  if ( file_ver < 3 )
		    sn = skill_lookup( fread_word( fp ) );
		  else
		    sn = bsearch_skill_exact( fread_word( fp ), gsn_first_skill, gsn_first_weapon-1 );
		  if ( sn < 0 )
		    bug( "Fread_char: unknown skill.", 0 );
		  else
		  {
		    ch->pcdata->learned[sn] = value;
		    
		  }
		  fMatch = TRUE;
		  break;
		}
	    }

	    if ( str_cmp( word, "End" ) )
		break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		assign_gname( ch );
		if (!ch->short_descr)
		  ch->short_descr       = STRALLOC( "" );
		if (!ch->long_descr)
		  ch->long_descr        = STRALLOC( "" );
		if (!ch->description)
		  ch->description       = STRALLOC( "" );
		if (!ch->pcdata->pwd)
		  ch->pcdata->pwd       = str_dup( "" );
		if (!ch->pcdata->email)
		  ch->pcdata->email     = str_dup( "" );
		if (!ch->pcdata->bamfin)
		  ch->pcdata->bamfin    = str_dup( "" );
		if (!ch->pcdata->bamfout)
		  ch->pcdata->bamfout   = str_dup( "" );
		if (!ch->pcdata->bio)
		  ch->pcdata->bio       = STRALLOC( "" );
		if (!ch->pcdata->rank)
		  ch->pcdata->rank      = str_dup( "" );
                if (!ch->pcdata->ignore)
                  ch->pcdata->ignore    = str_dup( "" );
		if (!ch->pcdata->bestowments)
		  ch->pcdata->bestowments = str_dup( "" );
		if (!ch->pcdata->title)
		  ch->pcdata->title     = STRALLOC( "" );
		if (!ch->pcdata->homepage)
		  ch->pcdata->homepage  = str_dup( "" );
                if (!ch->pcdata->fname)
                  ch->pcdata->fname     = str_dup( "" );
                if (!ch->pcdata->lname)
                  ch->pcdata->lname     = str_dup( "" );
		if (!ch->pcdata->authed_by)
		  ch->pcdata->authed_by = STRALLOC( "" );
		if (!ch->pcdata->prompt )
		  ch->pcdata->prompt    = STRALLOC( "" );
		if (!ch->pcdata->email)
		  ch->pcdata->email = str_dup( "" );
		ch->editor              = NULL;
		if ( !IS_IMMORTAL( ch ) && xIS_EMPTY(ch->speaking) )
		{
		   xSET_BIT( ch->speaking, race_table[ch->race].language );
		}
		if ( IS_IMMORTAL( ch ) )
		{
	    xCLEAR_BITS(ch->speaks);
	    if ( xIS_EMPTY(ch->speaking) )
		xCLEAR_BITS(ch->speaking);
		}
		if ( !ch->pcdata->prompt )
		  ch->pcdata->prompt = STRALLOC("");
		  
		if ( lastplayed != 0 )
		{
		   int hitgain;
		   hitgain = ( ( int ) ( current_time - lastplayed ) / 60 );
		   ch->hit = URANGE( 1 , ch->hit + hitgain , ch->max_hit );                
		   ch->move = URANGE( 1 , ch->move + hitgain , ch->max_move );             
		   better_mental_state( ch , hitgain );
		}
		for ( sn = 0; sn < top_sn; sn++ )
		{       
		   if ( !skill_table[sn]->name )
		      break;

                   if ( skill_table[sn]->race != ch->race )
		      continue;
		
                   if ( ch->pcdata->learned[sn] > 0 && ch->top_level < skill_table[sn]->min_level )
		      ch->pcdata->learned[sn] = 0;
		}
		return;
	    }
	    KEY( "Email",       ch->pcdata->email,      fread_string_nohash( fp ) );
	    break;

	case 'T':
	    KEY( "Toplevel",    ch->top_level,          fread_number( fp ) );
	if ( !str_cmp( word, "Tongue" ) )
	    {
		int sn;
		int value;

		if ( preload )
		  word = "End";
		else
		{
		  value = fread_number( fp );

		  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_tongue, gsn_top_sn-1 );
		  if ( sn < 0 )
		    bug( "Fread_char: unknown tongue.", 0 );
		  else
		  {
		    ch->pcdata->learned[sn] = value;
		    
		  }
		  fMatch = TRUE;
		}
		break;
	    }
	    KEY( "Trust", ch->trust, fread_number( fp ) );
	    /* Let no character be trusted higher than one below maxlevel -- Narn */
	    ch->trust = UMIN( ch->trust, MAX_LEVEL - 1 );

	    if ( !str_cmp( word, "Title" ) )
	    {
		ch->pcdata->title = fread_string( fp );
		if ( isalpha(ch->pcdata->title[0])
		||   isdigit(ch->pcdata->title[0]) )
		{
                    sprintf( buf, "%s", ch->pcdata->title );
		    if ( ch->pcdata->title )
		      STRFREE( ch->pcdata->title );
		    ch->pcdata->title = STRALLOC( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    KEY( "Version",     file_ver,               fread_number( fp ) );
	    break;

	case 'W':
	    if ( !str_cmp( word, "Weapon" ) )
	    {
		int sn;
		int value;

		if ( preload )
		  word = "End";
		else
		{
		  value = fread_number( fp );

		  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_weapon, gsn_first_tongue-1 );
		  if ( sn < 0 )
		    bug( "Fread_char: unknown weapon.", 0 );
		  else
		  {
		    ch->pcdata->learned[sn] = value;
		    
		  }
		  fMatch = TRUE;
		}
		break;
	    }
	    KEY( "WizInvis",    ch->pcdata->wizinvis,   fread_number( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_char: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}


void fread_obj( CHAR_DATA *ch, FILE *fp, sh_int os_type )
{
    OBJ_DATA *obj;
    char *word;
    int iNest, z;
    bool fMatch;
    bool fNest;
    bool fVnum;
    ROOM_INDEX_DATA *room;

    if ( ch == NULL )
      z = 1;
    else
      z = (IS_NPC(ch) ? 1 : 0);

    CREATE( obj, OBJ_DATA, 1 );
    obj->count          = 1;
    obj->wear_loc       = -1;
    obj->weight         = 1;

    fNest               = TRUE;         /* Requiring a Nest 0 is a waste */
    fVnum               = TRUE;
    iNest               = 0;

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

	case 'A':
	    if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf;
		int pafmod;

		CREATE( paf, AFFECT_DATA, 1 );
		if ( !str_cmp( word, "Affect" ) )
		{
		    paf->type   = fread_number( fp );
		}
		else
		{
		    int sn;

		    sn = skill_lookup( fread_word( fp ) );
		    if ( sn < 0 )
			bug( "Fread_obj: unknown skill.", 0 );
		    else
			paf->type = sn;
		}
		paf->duration   = fread_number( fp );
		pafmod          = fread_number( fp );
		paf->location   = fread_number( fp );
                paf->bitvector  = fread_bitvector( fp );
                paf->modifier         = pafmod;
		LINK(paf, obj->first_affect, obj->last_affect, next, prev );
		fMatch                          = TRUE;
		break;
	    }
            if ( !str_cmp( word, "AmmoVnum" ) )
	    {
                OBJ_INDEX_DATA *pAmmoIndex;
                int vnum;

                vnum = fread_number( fp );

                if ( ( pAmmoIndex = get_obj_index( vnum ) ) != NULL )
                {
                   obj->ammo = create_object( pAmmoIndex, 1 );
                   fMatch = TRUE;
                }
	    }
            if ( !str_cmp( word, "AmmoCount" ) )
	    {
                int cnt;

                cnt = fread_number( fp );

                if ( obj->ammo != NULL )
                {
                   obj->ammo->value[2] = cnt;
                   fMatch = TRUE;
                }
	    }
            if ( !str_cmp( word, "AttachVnum" ) )
	    {
                OBJ_INDEX_DATA *pAttachIndex;
                int vnum;

                vnum = fread_number( fp );

                if ( ( pAttachIndex = get_obj_index( vnum ) ) != NULL )
                {
                   obj->attach = create_object( pAttachIndex, 1 );
                   fMatch = TRUE;
                }
	    }
            if ( !str_cmp( word, "AttachAmmo" ) )
	    {
                OBJ_INDEX_DATA *pAmmoIndex;
                int vnum;

                vnum = fread_number( fp );

                if ( obj->attach && ( ( pAmmoIndex = get_obj_index( vnum ) ) != NULL ) )
                {
                   obj->attach->ammo = create_object( pAmmoIndex, 1 );
                   fMatch = TRUE;
                }
	    }
            if ( !str_cmp( word, "AttachCount" ) )
	    {
                int cnt;

                cnt = fread_number( fp );

                if ( obj->attach != NULL && obj->attach->ammo != NULL )
                {
                   obj->attach->ammo->value[2] = cnt;
                   fMatch = TRUE;
                }
	    }

	    KEY( "Actiondesc",  obj->action_desc,       fread_string( fp ) );
	    break;

	case 'C':
	    KEY( "Cost",        obj->cost,              fread_number( fp ) );
	    KEY( "Count",       obj->count,             fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description", obj->description,       fread_string( fp ) );
	    break;

	case 'E':
	KEY( "ExtraFlags",  obj->extra_flags,   fread_bitvector( fp ) );

	    if ( !str_cmp( word, "ExtraDescr" ) )
	    {
		EXTRA_DESCR_DATA *ed;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword             = fread_string( fp );
		ed->description         = fread_string( fp );
		LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		fMatch                          = TRUE;
	    }

	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !fNest || !fVnum )
		{
		    bug( "Fread_obj: incomplete object.", 0 );
		    if ( obj->name )
		      STRFREE( obj->name        );
		    if ( obj->description )
		      STRFREE( obj->description );
		    if ( obj->short_descr )
		      STRFREE( obj->short_descr );
		    DISPOSE( obj );
		    return;
		}
		else
		{
		    sh_int wear_loc = obj->wear_loc;

		    if ( !obj->name )
			obj->name = QUICKLINK( obj->pIndexData->name );
		    if ( !obj->description )
			obj->description = QUICKLINK( obj->pIndexData->description );
		    if ( !obj->short_descr )
			obj->short_descr = QUICKLINK( obj->pIndexData->short_descr );
		    if ( !obj->action_desc )
			obj->action_desc = QUICKLINK( obj->pIndexData->action_desc );
		    LINK(obj, first_object, last_object, next, prev );
		    obj->pIndexData->count += obj->count;
		    if ( !obj->serial )
		    {
			cur_obj_serial = UMAX((cur_obj_serial + 1 ) & (BV30-1), 1);
			obj->serial = obj->pIndexData->serial = cur_obj_serial;
		    }
                    if ( obj->item_type == ITEM_WEAPON && is_ranged( obj->value[0] ) )
                        obj->weapon_mode = default_weapon_mode( obj );
                    if ( fNest )
		      rgObjNest[iNest] = obj;
		    numobjsloaded += obj->count;
		    ++physicalobjects;
		    if ( file_ver > 1 || obj->wear_loc < -1
		    ||   obj->wear_loc >= MAX_WEAR )
		      obj->wear_loc = -1;
		    /* Corpse saving. -- Altrag */
		    if ( os_type == OS_CORPSE )
		    {
			if ( !room )
			{
			  bug( "Fread_obj: Corpse without room", 0);
			  room = get_room_index(ROOM_VNUM_LIMBO);
			}
			obj = obj_to_room( obj, room );
		    }
		    else if ( iNest == 0 || rgObjNest[iNest] == NULL )
		    {
			int slot;
			bool reslot = FALSE;

			if ( file_ver > 1
			&&   wear_loc > -1
			&&   wear_loc < MAX_WEAR )
			{
			   int x;

			   for ( x = 0; x < MAX_LAYERS; x++ )
			      if ( !save_equipment[z][wear_loc][x] )
			      {
				  save_equipment[z][wear_loc][x] = obj;
				  slot = x;
				  reslot = TRUE;
				  break;
			      }
			   if ( x == MAX_LAYERS )
				bug( "Fread_obj: too many layers %d", wear_loc );
			}
			obj = obj_to_char( obj, ch );
			if ( reslot )
			  save_equipment[z][wear_loc][slot] = obj;
		    }
		    else
		    {
			if ( rgObjNest[iNest-1] )
			{
			   separate_obj( rgObjNest[iNest-1] );
			   obj = obj_to_obj( obj, rgObjNest[iNest-1] );
			}
			else
			   bug( "Fread_obj: nest layer missing %d", iNest-1 );
		    }
		    if ( fNest )
		      rgObjNest[iNest] = obj;
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",    obj->item_type,         fread_number( fp ) );
	    break;

	case 'K':
	    KEY( "Killed_by",   obj->killed_by,         fread_string( fp ) );
	    break;

	case 'L':
	    KEY( "Level",       obj->level,             fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",        obj->name,              fread_string( fp ) );

	    if ( !str_cmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		    iNest = 0;
		    fNest = FALSE;
		}
		fMatch = TRUE;
	    }
	    break;
	    
	case 'R':
	    KEY( "Room", room, get_room_index(fread_number(fp)) );

	case 'S':
	    KEY( "ShortDescr",  obj->short_descr,       fread_string( fp ) );

	    if ( !str_cmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 5 )
		    bug( "Fread_obj: bad iValue %d.", iValue );
		else if ( sn < 0 )
		    bug( "Fread_obj: unknown skill.", 0 );
		else
		    obj->value[iValue] = sn;
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",       obj->timer,             fread_number( fp ) );
	    break;

	case 'V':
	    if ( !str_cmp( word, "Values" ) )
	    {
		int x1,x2,x3,x4,x5,x6;
		char *ln = fread_line( fp );

		x1=x2=x3=x4=x5=x6=0;
		sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
		/* clean up some garbage */
		if ( obj->item_type == ITEM_WEAPON )
		{
		   if ( x5 <= 0 ) x5 = 2000;
		   if ( x6 <= 0 ) x6 = 2000;
		}
		obj->value[0]   = x1;
		obj->value[1]   = x2;
		obj->value[2]   = x3;
		obj->value[3]   = x4;
		obj->value[4]   = x5;
		obj->value[5]   = x6;
		fMatch          = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		{
		    fVnum = FALSE;
		    bug( "Fread_obj: bad vnum %d.", vnum ); 
		}
		else
		{
		    fVnum = TRUE;
		    obj->cost = obj->pIndexData->cost;
		    obj->weight = obj->pIndexData->weight;
		    obj->item_type = obj->pIndexData->item_type;
		    obj->wear_flags = obj->pIndexData->wear_flags;
		    obj->extra_flags = obj->pIndexData->extra_flags;
		}
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	KEY( "WearFlags",   obj->wear_flags,    fread_bitvector( fp ) );
	    KEY( "WearLoc",     obj->wear_loc,          fread_number( fp ) );
	    KEY( "Weight",      obj->weight,            fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    EXTRA_DESCR_DATA *ed;
	    AFFECT_DATA *paf;

	    bug( "Fread_obj: no match.", 0 );
	    bug( word, 0 );
	    fread_to_eol( fp );
	    if ( obj->name )
		STRFREE( obj->name        );
	    if ( obj->description )
		STRFREE( obj->description );
	    if ( obj->short_descr )
		STRFREE( obj->short_descr );
	    while ( (ed=obj->first_extradesc) != NULL )
	    {
		STRFREE( ed->keyword );
		STRFREE( ed->description );
		UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		DISPOSE( ed );
	    }
	    while ( (paf=obj->first_affect) != NULL )
	    {
		UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
		DISPOSE( paf );
	    }
	    DISPOSE( obj );
	    return;
	}
    }
}

void set_alarm( long seconds )
{
    alarm( seconds );
}

/*
 * Based on last time modified, show when a player was last on  -Thoric
 */
void do_last( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    char arg [MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    struct stat fst;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Usage: last <playername>\n\r", ch );
	return;
    }
    strcpy( name, capitalize(arg) );
    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name );
    if ( stat( buf, &fst ) != -1 )
      sprintf( buf, "%s was last on: %s\r", name, ctime( &fst.st_mtime ) );
    else
      sprintf( buf, "%s was not found.\n\r", name );
   send_to_char( buf, ch );
}

void write_corpses( CHAR_DATA *ch, char *name )
{
  OBJ_DATA *corpse;
  FILE *fp = NULL;
  
  /* Name and ch support so that we dont have to have a char to save their
     corpses.. (ie: decayed corpses while offline) */
  if ( ch && IS_NPC(ch) )
  {
    bug( "Write_corpses: writing NPC corpse.", 0 );
    return;
  }
  if ( ch )
    name = ch->name;
  /* Go by vnum, less chance of screwups. -- Altrag */
  for ( corpse = first_object; corpse; corpse = corpse->next )
    if ( corpse->pIndexData->vnum == OBJ_VNUM_CORPSE_PC &&
	 corpse->in_room != NULL &&
        !str_cmp(corpse->short_descr, name) )
    {
      if ( !fp )
      {
	char buf[127];
	
	sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
	if ( !(fp = fopen(buf, "w")) )
	{
	  bug( "Write_corpses: Cannot open file.", 0 );
	  perror(buf);
	  return;
	}
      }
      fwrite_obj(ch, corpse, fp, 0, OS_CORPSE);
    }
  if ( fp )
  {
    fprintf(fp, "#END\n\n");
    fclose(fp);
  }
  else
  {
    char buf[127];
    
    sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
    remove(buf);
  }
  return;
}

void load_corpses( void )
{
  DIR *dp;
  struct dirent *de;
  extern FILE *fpArea;
  extern char strArea[MAX_INPUT_LENGTH];
  extern int falling;
  
  if ( !(dp = opendir(CORPSE_DIR)) )
  {
    bug( "Load_corpses: can't open CORPSE_DIR", 0);
    perror(CORPSE_DIR);
    return;
  }

  falling = 1; /* Arbitrary, must be >0 though. */
  while ( (de = readdir(dp)) != NULL )
  {
    if ( de->d_name[0] != '.' )
    {
      sprintf(strArea, "%s%s", CORPSE_DIR, de->d_name );
      fprintf(stderr, "Corpse -> %s\n", strArea);
      if ( !(fpArea = fopen(strArea, "r")) )
      {
	perror(strArea);
	continue;
      }
      for ( ; ; )
      {
	char letter;
	char *word;
	
	letter = fread_letter( fpArea );
	if ( letter == '*' )
	{
	  fread_to_eol(fpArea);
	  continue;
	}
	if ( letter != '#' )
	{
	  bug( "Load_corpses: # not found.", 0 );
	  break;
	}
	word = fread_word( fpArea );
	if ( !str_cmp(word, "CORPSE" ) )
	  fread_obj( NULL, fpArea, OS_CORPSE );
	else if ( !str_cmp(word, "OBJECT" ) )
	  fread_obj( NULL, fpArea, OS_CARRY );
	else if ( !str_cmp( word, "END" ) )
	  break;
	else
	{
	  bug( "Load_corpses: bad section.", 0 );
	  break;
	}
      }
      fclose(fpArea);
    }
  }
  fpArea = NULL;
  strcpy(strArea, "$");
  closedir(dp);
  falling = 0;
  return;
}

/*
 * This will write one mobile structure pointed to be fp --Shaddai
 */
void fwrite_mobile( FILE *fp, CHAR_DATA *mob )
{
  if ( !IS_NPC( mob ) || !fp )
	return;
  fprintf( fp, "#MOBILE\n" );
  fprintf( fp, "Vnum	%d\n", mob->pIndexData->vnum );
  if ( mob->in_room )
	fprintf( fp, "Room	%d\n", 
		(  mob->in_room == get_room_index( ROOM_VNUM_LIMBO )
			&& mob->was_in_room )
			? mob->was_in_room->vnum
			: mob->in_room->vnum ); 
  if ( QUICKMATCH( mob->name, mob->pIndexData->player_name) == 0 )
	fprintf( fp, "Name     %s~\n", mob->name );
  if ( QUICKMATCH( mob->short_descr, mob->pIndexData->short_descr) == 0 )
	fprintf( fp, "Short	%s~\n", mob->short_descr );
  if ( QUICKMATCH( mob->long_descr, mob->pIndexData->long_descr) == 0 )
	fprintf( fp, "Long	%s~\n", mob->long_descr );
  if ( QUICKMATCH( mob->description, mob->pIndexData->description) == 0 )
	fprintf( fp, "Description %s~\n", mob->description );
  fprintf( fp, "Level %d\n", mob->top_level );
  fprintf( fp, "Hp %d\n", mob->hit );
  fprintf( fp, "Maxhp %d\n", mob->max_hit );
  fprintf( fp, "Race %d\n", mob->race );
  fprintf( fp, "Sex %d\n", mob->sex );

  fprintf( fp, "Position %d\n", mob->position );
  fprintf( fp, "Flags %s\n\n",   print_bitvector(&mob->act) );
/* Might need these later --Shaddai */
  de_equip_char( mob );
  if ( mob->first_carrying )
	fwrite_obj( mob, mob->last_carrying, fp, 0, OS_CARRY );
  re_equip_char( mob );
  fprintf( fp, "EndMobile\n" );
  return;
}

/*
 * This will read one mobile structure pointer to by fp --Shaddai
 */
CHAR_DATA *  fread_mobile( FILE *fp )
{
  CHAR_DATA *mob = NULL;
  char *word;
  bool fMatch;
  int inroom = 0;
  int safty=0;
  int i, x;
  ROOM_INDEX_DATA *pRoomIndex = NULL;

  for ( x = 0; x < MAX_WEAR; x++ )
     for ( i = 0; i < MAX_LAYERS; i++ )
	save_equipment[1][x][i] = NULL;

  word   = feof( fp ) ? "EndMobile" : fread_word( fp );
  if ( !strcmp(word, "Vnum") )
  {
    int vnum;
    
    vnum = fread_number( fp );
    mob = create_mobile( get_mob_index(vnum) );
    if ( !mob )
    {
	for ( safty = 0; safty < 999999; safty++) {
	  word   = feof( fp ) ? "EndMobile" : fread_word( fp );
	  /* So we don't get so many bug messages when something messes up
	   * --Shaddai 
	   */
	  if ( !strcmp( word, "EndMobile" ) )
		break;
	}
	bug("Fread_mobile: No index data for vnum %d", vnum );
	return NULL;
    }
  }
  else
  {
	for ( safty=0; safty < 999999; safty++) {
	  word   = feof( fp ) ? "EndMobile" : fread_word( fp );
	  /* So we don't get so many bug messages when something messes up
	   * --Shaddai 
	   */
	  if ( !strcmp( word, "EndMobile" ) )
		break;
	}
        extract_char(mob, TRUE, FALSE);
	bug("Fread_mobile: Vnum not found", 0 );
	return NULL;
  }
  for ( safty=0; safty < 999999 ; safty++ ) {
       word   = feof( fp ) ? "EndMobile" : fread_word( fp );
       fMatch = FALSE;
       switch ( UPPER(word[0]) ) {
	case '*':
	   fMatch = TRUE;
	   fread_to_eol( fp );
	   break;  
	case '#':
		if ( !strcmp( word, "#OBJECT" ) )
		{
		   fread_obj ( mob, fp, OS_CARRY );
		   fMatch = TRUE;
		}
		break;
	case 'D':
		KEY( "Description", mob->description, fread_string(fp));
		break;
	case 'E':
		if ( !strcmp( word, "EndMobile" ) )
		{
		if ( inroom == 0 )
			inroom = ROOM_VNUM_TEMPLE;
		pRoomIndex = get_room_index( inroom );
		if ( !pRoomIndex )
			pRoomIndex = get_room_index( ROOM_VNUM_TEMPLE );
		char_to_room(mob, pRoomIndex);

		for ( i = 0; i < MAX_WEAR; i++ )
		 for ( x = 0; x < MAX_LAYERS; x++ )
		  if ( save_equipment[1][i][x] )
		  {
		     equip_char( mob, save_equipment[1][i][x], i );
		     save_equipment[1][i][x] = NULL;
		  }
		  else
		     break;

                return mob;
		}
		break;
	case 'F':
		KEY( "Flags", mob->act, fread_bitvector(fp));
                break;
        case 'H':
                KEY( "Hp", mob->hit, fread_number( fp ) );
                break;
	case 'L':
		KEY( "Long", mob->long_descr, fread_string(fp ) );
                KEY( "Level", mob->top_level, fread_number( fp ) );
		break;
	case 'N':
		KEY( "Name", mob->name, fread_string( fp ) );
		break;
        case 'M':
                KEY( "Maxhp", mob->max_hit, fread_number( fp ) );
                break;          
	case 'P':
		KEY( "Position", mob->position, fread_number( fp ) );
		break;
	case 'R':
		KEY( "Room",  inroom, fread_number(fp));
                KEY( "Race", mob->race, fread_number( fp ) );
		break;
	case 'S':
		KEY( "Short", mob->short_descr, fread_string(fp));
                KEY( "Sex", mob->sex, fread_number( fp ) );
		break;
	}
	if ( !fMatch )
	{
	   bug ( "Fread_mobile: no match.", 0 );
	   bug ( word, 0 );
	}
  }
  return NULL;
}

char * simple_encode( char * in, int off )
{
  static char buf[MAX_STRING_LENGTH];
  char * p;

  strcpy( buf, in );

  for ( p = buf; *p != '\0'; p++ )
  {
     *p = (char)((int)(*p) + (int)(off));
  }

  return buf;
}
