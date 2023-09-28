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
*			Specific object creation module			   *
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void     make_ash    args( ( CHAR_DATA *ch, CHAR_DATA *killer ) );

/*
 * Make a fire.
 */
void make_fire(ROOM_INDEX_DATA *in_room, sh_int timer)
{
    OBJ_DATA *fire;

    fire = create_object( get_obj_index( OBJ_VNUM_FIRE ), 0 );
    fire->timer = number_fuzzy(timer);
    obj_to_room( fire, in_room );
    return;
}

/*
 * Turn an object into scraps.		-Thoric
 */
OBJ_DATA * make_scraps( OBJ_DATA *obj )
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA  *scraps, *tmpobj;
  CHAR_DATA *ch = NULL;

  separate_obj( obj );
  scraps	= create_object( get_obj_index( OBJ_VNUM_SCRAPS ), 0 );
  scraps->timer = 2;

  /* don't make scraps of scraps of scraps of ... */
  if ( obj->pIndexData->vnum == OBJ_VNUM_SCRAPS )
  {
     STRFREE( scraps->short_descr );
     scraps->short_descr = STRALLOC( "some debris" );
     STRFREE( scraps->description );
     scraps->description = STRALLOC( "Bits of debris lie on the ground here." );
  }
  else
  {
     sprintf( buf, scraps->short_descr, obj->short_descr );
     STRFREE( scraps->short_descr );
     scraps->short_descr = STRALLOC( buf );
     sprintf( buf, scraps->description, obj->short_descr );
     STRFREE( scraps->description );
     scraps->description = STRALLOC( buf );
  }

  if ( obj->carried_by )
  {
    act( AT_OBJECT, "$p falls to the ground in scraps!",
		  obj->carried_by, obj, NULL, TO_CHAR );
    if ( obj == get_eq_char( obj->carried_by, WEAR_WIELD )
    &&  (tmpobj = get_eq_char( obj->carried_by, WEAR_DUAL_WIELD)) != NULL )
       tmpobj->wear_loc = WEAR_WIELD;

    obj_to_room( scraps, obj->carried_by->in_room);
  }
  else
  if ( obj->in_room )
  {
    if ( (ch = obj->in_room->first_person ) != NULL )
    {
      act( AT_OBJECT, "$p is reduced to little more than scraps.",
	   ch, obj, NULL, TO_ROOM );
      act( AT_OBJECT, "$p is reduced to little more than scraps.",
	   ch, obj, NULL, TO_CHAR );
    }
    obj_to_room( scraps, obj->in_room);
  }
  if ( (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_CORPSE_PC ) && obj->first_content )
  {
    if ( ch && ch->in_room )
    {
	act( AT_OBJECT, "The contents of $p fall to the ground.",
	   ch, obj, NULL, TO_ROOM );
	act( AT_OBJECT, "The contents of $p fall to the ground.",
	   ch, obj, NULL, TO_CHAR );
    }
    if ( obj->carried_by )
	empty_obj( obj, NULL, obj->carried_by->in_room );
    else
    if ( obj->in_room )
	empty_obj( obj, NULL, obj->in_room );
    else
    if ( obj->in_obj )
	empty_obj( obj, obj->in_obj, NULL );
  }
  extract_obj( obj );
  return scraps;
}


/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    int cnt = 0;

    for ( obj = ch->first_carrying; obj; obj = obj_next )
    {
	obj_next = obj->next_content;
        if ( IS_OBJ_STAT( obj, ITEM_RESPAWN ) && (!IS_NPC(ch) || IS_BOT(ch)) )
        {
            if ( IS_OBJ_STAT( obj, ITEM_ALIEN ) && ch->race == RACE_ALIEN ) continue;
            if ( IS_OBJ_STAT( obj, ITEM_MARINE ) && ch->race == RACE_MARINE ) continue;
            if ( IS_OBJ_STAT( obj, ITEM_PREDATOR ) && ch->race == RACE_PREDATOR ) continue;
        }
        if ( IS_OBJ_STAT( obj, ITEM_DEATHROT ) ) continue;
        cnt++;
    }
    
    if ( cnt <= 0 ) return;

    if ( IS_NPC(ch) && !IS_BOT(ch) )
    {
       name  = ch->short_descr;
       corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);

       corpse->timer   = 6;

       if ( !killer || killer == NULL )
       {
           corpse->killed_by = STRALLOC( "Unknown" );
       }
       else if ( !IS_NPC(killer) )
       {
           corpse->killed_by = STRALLOC( killer->name );
       }
       else
       {
           corpse->killed_by = STRALLOC( killer->short_descr );
       }

       /*  Using corpse cost to cheat, since corpses not sellable */
       corpse->cost     = (-(int)ch->pIndexData->vnum);
       corpse->value[2] = corpse->timer; 
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
        corpse->timer   = 10;
        corpse->value[2] = (int)(corpse->timer/8);
	corpse->value[3] = 0;

        if ( !killer || killer == NULL )
        {
            corpse->killed_by = STRALLOC( "Unknown" );
        }
        else if ( !IS_NPC(killer) )
        {
            corpse->killed_by = STRALLOC( killer->name );
        }
        else
        {
            corpse->killed_by = STRALLOC( killer->short_descr );
        }
    }

    /* Added corpse name - make locate easier , other skills */
    sprintf( buf, "backpack %s", name );
    STRFREE( corpse->name );
    corpse->name = STRALLOC( buf );

    sprintf( buf, corpse->short_descr, name );
    STRFREE( corpse->short_descr );
    corpse->short_descr = STRALLOC( buf );

    sprintf( buf, corpse->description, name );
    STRFREE( corpse->description );
    corpse->description = STRALLOC( buf );

    for ( obj = ch->first_carrying; obj; obj = obj_next )
    {
	obj_next = obj->next_content;
        if ( IS_OBJ_STAT( obj, ITEM_RESPAWN ) )
        {
            if ( IS_OBJ_STAT( obj, ITEM_ALIEN ) && ch->race == RACE_ALIEN ) continue;
            if ( IS_OBJ_STAT( obj, ITEM_MARINE ) && ch->race == RACE_MARINE ) continue;
            if ( IS_OBJ_STAT( obj, ITEM_PREDATOR ) && ch->race == RACE_PREDATOR ) continue;
        }
        obj_from_char( obj );
        if ( IS_OBJ_STAT( obj, ITEM_DEATHROT ) )
	    extract_obj( obj );
        else
	    obj_to_obj( obj, corpse );
    }

    obj_to_room( corpse, ch->in_room );
    return;
}

void make_blood( CHAR_DATA *ch )
{
        // OBJ_DATA *obj;

        bug ("--> BLOOD GENERATION CODE OFFLINE! <--");
        /*
	obj		= create_object( get_obj_index( OBJ_VNUM_BLOOD ), 0 );
	obj->timer	= number_range( 2, 4 );
	obj->value[1]   = number_range( 3, UMIN(5, ch->top_level) );
	obj_to_room( obj, ch->in_room );
        */
}


void make_bloodstain( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	obj		= create_object( get_obj_index( OBJ_VNUM_BLOODSTAIN ), 0 );
	obj->timer	= number_range( 1, 2 );
	obj_to_room( obj, ch->in_room );
}


extern int        top_affect;

void newbie_create( CHAR_DATA *ch, int type )
{
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    AFFECT_DATA *paf;
    
    switch( type )
    {
        default:
             bug("newbie_create: bad newbie_item type." );
             return;
        case 1:        /* Combat Knife */
             obj = pull_blank_obj( );
             obj->name = STRALLOC( "combat knife" );
             obj->short_descr = STRALLOC( "Combat Knife" );
             obj->description = STRALLOC( "A combat knife was left here." );
             obj->item_type = ITEM_WEAPON;
             obj->weight = 1;
             obj->cost = 10;
             obj->value[0] = WEAPON_KNIFE;
             obj->value[1] = 20;              /* Minimum damage  */
             obj->value[2] = 30;              /* Maximum damage */
             obj->value[3] = 2;               /* 10% reach bonus */         
             obj->value[4] = RIS_PIERCE;      /* Piercing damage */
             obj->value[5] = 3;
             xSET_BIT( obj->wear_flags, ITEM_WIELD );
             xSET_BIT( obj->extra_flags, ITEM_RESPAWN );
             xSET_BIT( obj->extra_flags, ITEM_MARINE );
             obj = obj_to_char( obj, ch );
             break;
        case 2:        /* M4A3 Pistol */
             if ( ( pObjIndex = get_obj_index( OBJ_VNUM_M4A3_PISTOL ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create M4A3 pistol!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             // equip_char( ch, obj, WEAR_WIELD );
             break;
        case 3:        /* Standard M4A3 Clip */
             if ( ( pObjIndex = get_obj_index( OBJ_VNUM_M4A3_AMMO ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create M4A3 ammo!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             break;
        case 4:        /* Single Ration */
             obj = pull_blank_obj( );
             obj->name = STRALLOC( "ration food" );
             obj->short_descr = STRALLOC( "Packaged Ration" );
             obj->description = STRALLOC( "A small packaged ration was left lying here." );
             obj->item_type = ITEM_FOOD;
             obj->weight = 1;
             obj->value[0] = 20;             /* Hitpoint gain */
             obj->value[1] = 50;             /* Movement gain */
             obj->cost = 0;
             obj = obj_to_char( obj, ch );
             break;
        case 5:         /* Kevlar Helmet */
             obj = pull_blank_obj( );
             obj->name = STRALLOC( "kevlar helmet combat" );
             obj->short_descr = STRALLOC( "Kevlar Helmet" );
             obj->description = STRALLOC( "A kevlar combat helmet was left here." );
             obj->item_type = ITEM_ARMOR;
             obj->weight = 2;
             obj->cost = 30;
             obj->value[0] = 180;
             obj->value[1] = 180;
             obj->value[3] = 1;

             CREATE( paf, AFFECT_DATA, 1 );
             paf->type               = -1;
             paf->duration           = -1;
             paf->location           = get_atype( "impact" );
             paf->modifier           = 5;
             xCLEAR_BITS(paf->bitvector);
             paf->next               = NULL;
             LINK( paf, obj->first_affect, obj->last_affect, next, prev );
             ++top_affect;

             xSET_BIT( obj->wear_flags, ITEM_WEAR_HEAD );
             xSET_BIT( obj->extra_flags, ITEM_MARINE );
             obj = obj_to_char( obj, ch );
             equip_char( ch, obj, WEAR_HEAD );
             break;
        case 6:         /* Kevlar Vest */
             obj = pull_blank_obj( );
             obj->name = STRALLOC( "kevlar vest" );
             obj->short_descr = STRALLOC( "Kevlar Vest" );
             obj->description = STRALLOC( "A kevlar vest was left here." );
             obj->item_type = ITEM_ARMOR;
             obj->weight = 3;
             obj->cost = 90;
             obj->value[0] = 220;
             obj->value[1] = 220;
             obj->value[3] = 1;

             CREATE( paf, AFFECT_DATA, 1 );
             paf->type               = -1;
             paf->duration           = -1;
             paf->location           = get_atype( "impact" );
             paf->modifier           = 10;
             xCLEAR_BITS(paf->bitvector);
             paf->next               = NULL;
             LINK( paf, obj->first_affect, obj->last_affect, next, prev );
             ++top_affect;

             xSET_BIT( obj->wear_flags, ITEM_WEAR_BODY );
             xSET_BIT( obj->extra_flags, ITEM_MARINE );
             obj = obj_to_char( obj, ch );
             equip_char( ch, obj, WEAR_BODY );
             break;
        case 7:         /* Protective Boots */
             obj = pull_blank_obj( );
             obj->name = STRALLOC( "protective boots" );
             obj->short_descr = STRALLOC( "Protective Boots" );
             obj->description = STRALLOC( "A pair of protective combat boots were left here." );
             obj->item_type = ITEM_ARMOR;
             obj->weight = 1;
             obj->cost = 25;
             obj->value[0] = 150;
             obj->value[1] = 150;
             obj->value[3] = 1;

             CREATE( paf, AFFECT_DATA, 1 );
             paf->type               = -1;
             paf->duration           = -1;
             paf->location           = get_atype( "fire" );
             paf->modifier           = 10;
             xCLEAR_BITS(paf->bitvector);
             paf->next               = NULL;
             LINK( paf, obj->first_affect, obj->last_affect, next, prev );
             ++top_affect;

             xSET_BIT( obj->wear_flags, ITEM_WEAR_FEET );
             xSET_BIT( obj->extra_flags, ITEM_MARINE );
             obj = obj_to_char( obj, ch );
             equip_char( ch, obj, WEAR_FEET );
             break;
        case 21:        /* Predator Cloaking Module */
             if ( ( pObjIndex = get_obj_index( OBJ_VNUM_CLOAKING_DEVICE ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create Cloaking device!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             break;
        case 22:        /* Combi-Spear */
             obj = pull_blank_obj( );
             obj->name = STRALLOC( "combispear combi-spear spear" );
             obj->short_descr = STRALLOC( "Combi-Spear" );
             obj->description = STRALLOC( "A combi-spear was left lying here." );
             obj->item_type = ITEM_WEAPON;
             obj->weight = 3;
             obj->cost = 25;
             obj->value[0] = WEAPON_SPEAR;
             obj->value[1] = 50;              /* Minimum damage  */
             obj->value[2] = 75;              /* Maximum damage */
             obj->value[3] = 6;               /* 30% reach bonus */
             obj->value[4] = RIS_IMPACT;      /* Impact damage */
             obj->value[5] = 4;
             xSET_BIT( obj->wear_flags, ITEM_WIELD );
             xSET_BIT( obj->extra_flags, ITEM_RESPAWN );
             xSET_BIT( obj->extra_flags, ITEM_NODUAL );
             xSET_BIT( obj->extra_flags, ITEM_PREDATOR );
             obj = obj_to_char( obj, ch );
             equip_char( ch, obj, WEAR_WIELD );
             break;
        case 23:        /* Side Blade */
             obj = pull_blank_obj( );
             obj->name = STRALLOC( "side blade sideblade" );
             obj->short_descr = STRALLOC( "Side Blade" );
             obj->description = STRALLOC( "A predator side blade was left lying here." );
             obj->item_type = ITEM_WEAPON;
             obj->weight = 2;
             obj->cost = 40;
             obj->value[0] = WEAPON_BLADE;
             obj->value[1] = 30;              /* Minimum damage  */
             obj->value[2] = 45;              /* Maximum damage */
             obj->value[3] = 3;               /* 15% reach bonus */
             obj->value[4] = RIS_PIERCE;      /* Piercing damage */
             obj->value[5] = 3;
             xSET_BIT( obj->wear_flags, ITEM_WIELD );
             xSET_BIT( obj->extra_flags, ITEM_RESPAWN );
             xSET_BIT( obj->extra_flags, ITEM_PREDATOR );
             obj = obj_to_char( obj, ch );
             break;
        case 24:        /* Energy Grenade */
             if ( ( pObjIndex = get_obj_index( OBJ_VNUM_ENERGY_GRENADE ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create Energy grenade!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             break;
        case 25:        /* M40 Hand Grenade */
             if ( ( pObjIndex = get_obj_index( OBJ_VNUM_MARINE_GRENADE ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create M40 Hand grenade!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             break;            
        case 26:        /* Field Radio */
             if ( ( pObjIndex = get_obj_index( OBJ_VNUM_MARINE_RADIO ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create Field Radio!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             break;

        case 27:        /* Light Sphere */
             if ( ( pObjIndex = get_obj_index( 124 ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create Light Sphere!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             equip_char( ch, obj, WEAR_LIGHT );
             break;

        case 28:        /* Flashlight */
             if ( ( pObjIndex = get_obj_index( 671 ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create Flashlight!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             equip_char( ch, obj, WEAR_LIGHT );
             break;

        case 29:        /* Energy Pistol */
             if ( ( pObjIndex = get_obj_index( 99 ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create Energy Pistol!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             break;

        case 30:        /* Hunting Pack */
             if ( ( pObjIndex = get_obj_index( 253 ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create Hunting Pack!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             equip_char( ch, obj, WEAR_BODY );
             break;

        case 31:        /* Pulse Rifle */
             if ( ( pObjIndex = get_obj_index( OBJ_VNUM_PULSE_RIFLE ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create Pulse Rifles!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             equip_char( ch, obj, WEAR_WIELD );
             break;

        case 32:        /* M309 Clip */
             if ( ( pObjIndex = get_obj_index( OBJ_VNUM_PULSE_AMMO ) ) == NULL )
             {
                 bug("makeobjs.c: failed to create M309 ammo!");
                 return;
             }
             obj = create_object( pObjIndex, UMAX(1, ch->top_level) );
             obj = obj_to_char( obj, ch );
             break;

    }

    return;                                                            
}

OBJ_DATA * pull_blank_obj( void )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA * obj;

    if ( ( pObjIndex = get_obj_index( OBJ_VNUM_BLANK ) ) == NULL )
        return NULL;

    obj = create_object( pObjIndex, 1 );

    /*
     * Default settings
     */
    obj->level = 1;
    xCLEAR_BITS( obj->wear_flags );
    xSET_BIT( obj->wear_flags, ITEM_TAKE );
    xCLEAR_BITS( obj->extra_flags );

    STRFREE( obj->name );
    STRFREE( obj->short_descr );
    STRFREE( obj->description );

    return obj;
}
