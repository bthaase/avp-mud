/***************************************************************************
                            STAR WARS REALITY 1.0
    --------------------------------------------------------------------------
    Star Wars Reality Code Additions and changes from the Smaug Code
    copyright (c) 1997 by Sean Cooper
    -------------------------------------------------------------------------
    Starwars and Starwars Names copyright(c) Lucas Film Ltd.
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
                 Player skills module
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

char* const target_type[] =
{ "ignore", "offensive", "defensive", "self", "objinv" };


void show_char_to_char( CHAR_DATA* list, CHAR_DATA* ch );
void show_list_to_char( OBJ_DATA* list, CHAR_DATA* ch, bool fShort,
                        bool fShowN );

int ris_save( CHAR_DATA* ch, int chance, int ris );

/* from magic.c */
void failed_casting( struct skill_type* skill, CHAR_DATA* ch,
                     CHAR_DATA* victim, OBJ_DATA* obj );

int     xp_compute      args( ( CHAR_DATA* gch, CHAR_DATA* victim ) );

ROOM_INDEX_DATA* generate_exit( ROOM_INDEX_DATA* in_room, EXIT_DATA** pexit );

/*
    Dummy function
*/
void skill_notfound( CHAR_DATA* ch, char* argument )
{
    send_to_char( "Huh?\n\r", ch );
    return;
}


bool is_legal_kill( CHAR_DATA* ch, CHAR_DATA* vch )
{
    if ( IS_NPC( ch ) || IS_NPC( vch ) )
        return TRUE;

    return TRUE;
}


/*
    The kludgy global is for skills who want more stuff from command line.
*/
char* target_name;

/*
    Is immune to a damage type
*/
bool is_immune( CHAR_DATA* ch, sh_int damtype )
{
    switch ( damtype )
    {
            // case SD_FIRE:        if (xIS_SET(ch->immune, RIS_FIRE))   return TRUE;
            // case SD_COLD:        if (xIS_SET(ch->immune, RIS_COLD))   return TRUE;
            // case SD_ELECTRICITY: if (xIS_SET(ch->immune, RIS_ELECTRICITY)) return TRUE;
            // case SD_ENERGY:      if (xIS_SET(ch->immune, RIS_ENERGY)) return TRUE;
            // case SD_ACID:        if (xIS_SET(ch->immune, RIS_ACID))   return TRUE;
            // case SD_POISON:      if (xIS_SET(ch->immune, RIS_POISON)) return TRUE;
            // case SD_DRAIN:       if (xIS_SET(ch->immune, RIS_DRAIN))  return TRUE;
    }

    bug( "IS_IMMUNE still offline." );
    return FALSE;
}

/*
    Lookup a skill by name, only stopping at skills the player has.
*/
int ch_slookup( CHAR_DATA* ch, const char* name )
{
    int sn;

    if ( IS_NPC( ch ) )
        return skill_lookup( name );

    for ( sn = 0; sn < top_sn; sn++ )
    {
        if ( !skill_table[sn]->name )
            break;

        if ( ch->pcdata->learned[sn] > 0 && LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] ) && !str_prefix( name, skill_table[sn]->name ) )
            return sn;
    }

    return -1;
}

/*
    Lookup an herb by name.
*/
int herb_lookup( const char* name )
{
    int sn;

    for ( sn = 0; sn < top_herb; sn++ )
    {
        if ( !herb_table[sn] || !herb_table[sn]->name )
            return -1;

        if ( LOWER( name[0] ) == LOWER( herb_table[sn]->name[0] )
                &&  !str_prefix( name, herb_table[sn]->name ) )
            return sn;
    }

    return -1;
}

/*
    Lookup a personal skill
*/
int personal_lookup( CHAR_DATA* ch, const char* name )
{
    return -1;
}

/*
    Lookup a skill by name.
*/
int skill_lookup( const char* name )
{
    int sn;

    if ( ( sn = bsearch_skill( name, gsn_first_skill, gsn_first_weapon - 1 ) ) == -1 )
        if ( ( sn = bsearch_skill( name, gsn_first_weapon, gsn_first_tongue - 1 ) ) == -1 )
            if ( ( sn = bsearch_skill( name, gsn_first_tongue, gsn_top_sn - 1 ) ) == -1
                    &&    gsn_top_sn < top_sn )
            {
                for ( sn = gsn_top_sn; sn < top_sn; sn++ )
                {
                    if ( !skill_table[sn] || !skill_table[sn]->name )
                        return -1;

                    if ( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] )
                            &&  !str_prefix( name, skill_table[sn]->name ) )
                        return sn;
                }

                return -1;
            }

    return sn;
}

/*
    Return a skilltype pointer based on sn           -Thoric
    Returns NULL if bad, unused or personal sn.
*/
SKILLTYPE* get_skilltype( int sn )
{
    if ( sn >= TYPE_PERSONAL )
        return NULL;

    if ( sn >= TYPE_HERB )
        return IS_VALID_HERB( sn - TYPE_HERB ) ? herb_table[sn - TYPE_HERB] : NULL;

    if ( sn >= TYPE_HIT )
        return NULL;

    return IS_VALID_SN( sn ) ? skill_table[sn] : NULL;
}

/*
    Perform a binary search on a section of the skill table  -Thoric
    Each different section of the skill table is sorted alphabetically
*/
int bsearch_skill( const char* name, int first, int top )
{
    int sn;

    for ( ;; )
    {
        sn = ( first + top ) >> 1;

        if ( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] )
                &&  !str_prefix( name, skill_table[sn]->name ) )
            return sn;

        if ( first >= top )
            return -1;

        if ( strcmp( name, skill_table[sn]->name ) < 1 )
            top = sn - 1;
        else
            first = sn + 1;
    }

    return -1;
}

/*
    Perform a binary search on a section of the skill table  -Thoric
    Each different section of the skill table is sorted alphabetically
    Check for exact matches only
*/
int bsearch_skill_exact( const char* name, int first, int top )
{
    int sn;

    for ( ;; )
    {
        sn = ( first + top ) >> 1;

        if ( !str_prefix( name, skill_table[sn]->name ) )
            return sn;

        if ( first >= top )
            return -1;

        if ( strcmp( name, skill_table[sn]->name ) < 1 )
            top = sn - 1;
        else
            first = sn + 1;
    }

    return -1;
}

/*
    Perform a binary search on a section of the skill table
    Each different section of the skill table is sorted alphabetically
    Only match skills player knows               -Thoric
*/
int ch_bsearch_skill( CHAR_DATA* ch, const char* name, int first, int top )
{
    int sn;

    for ( ;; )
    {
        sn = ( first + top ) >> 1;

        if ( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] )
                &&  !str_prefix( name, skill_table[sn]->name )
                &&   ch->pcdata->learned[sn] > 0 )
            return sn;

        if ( first >= top )
            return -1;

        if ( strcmp( name, skill_table[sn]->name ) < 1 )
            top = sn - 1;
        else
            first = sn + 1;
    }

    return -1;
}


int find_skill( CHAR_DATA* ch, const char* name, bool know )
{
    if ( IS_NPC( ch ) || !know )
        return bsearch_skill( name, gsn_first_skill, gsn_first_weapon - 1 );
    else
        return ch_bsearch_skill( ch, name, gsn_first_skill, gsn_first_weapon - 1 );
}

int find_weapon( CHAR_DATA* ch, const char* name, bool know )
{
    if ( IS_NPC( ch ) || !know )
        return bsearch_skill( name, gsn_first_weapon, gsn_first_tongue - 1 );
    else
        return ch_bsearch_skill( ch, name, gsn_first_weapon, gsn_first_tongue - 1 );
}

int find_tongue( CHAR_DATA* ch, const char* name, bool know )
{
    if ( IS_NPC( ch ) || !know )
        return bsearch_skill( name, gsn_first_tongue, gsn_top_sn - 1 );
    else
        return ch_bsearch_skill( ch, name, gsn_first_tongue, gsn_top_sn - 1 );
}


/*
    Lookup a skill by slot number.
    Used for object loading.
*/
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
        return -1;

    for ( sn = 0; sn < top_sn; sn++ )
        if ( slot == skill_table[sn]->slot )
            return sn;

    if ( fBootDb )
    {
        bug( "Slot_lookup: bad slot %d.", slot );
        abort( );
    }

    return -1;
}

/*
    Perform a binary search on a section of the skill table
    Each different section of the skill table is sorted alphabetically
    Only match skills player knows               -Thoric
*/
bool check_skill( CHAR_DATA* ch, char* command, char* argument )
{
    int sn;
    int first = gsn_first_skill;
    int top   = gsn_first_weapon - 1;
    struct timeval time_used;

    if ( is_spectator( ch ) )
        return FALSE;

    /* bsearch for the skill */
    for ( ;; )
    {
        sn = ( first + top ) >> 1;

        if ( LOWER( command[0] ) == LOWER( skill_table[sn]->name[0] )
                &&  !str_prefix( command, skill_table[sn]->name )
                &&  ( skill_table[sn]->skill_fun )
                &&  ( IS_NPC( ch )
                      ||  ( ch->pcdata->learned[sn] > 0 ) ) )
            break;

        if ( first >= top )
            return FALSE;

        if ( strcmp( command, skill_table[sn]->name ) < 1 )
            top = sn - 1;
        else
            first = sn + 1;
    }

    if ( !check_pos( ch, skill_table[sn]->minimum_position ) )
        return TRUE;

    if ( ch->pcdata )
    {
        if ( ch->pcdata->prepared[sn] < skill_table[sn]->reset )
        {
            send_to_char( "&RYou cannot use that skill again yet.\n\r", ch );
            return TRUE;
        }
    }

    if ( IS_NPC( ch )
            &&  ( IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_POSSESS ) ) )
    {
        send_to_char( "For some reason, you seem unable to perform that...\n\r", ch );
        act( AT_GREY, "$n looks around.", ch, NULL, NULL, TO_ROOM );
        return TRUE;
    }

    /*
        Is this a real do-fun, or a really a spell?
    */
    if ( !skill_table[sn]->skill_fun )
    {
        return TRUE;
    }

    ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
    ch->last_cmd = skill_table[sn]->skill_fun;
    start_timer( &time_used );
    ( *skill_table[sn]->skill_fun ) ( ch, argument );
    end_timer( &time_used );
    update_userec( &time_used, &skill_table[sn]->userec );
    tail_chain( );
    return TRUE;
}

/*
    Lookup a skills information
    High god command
*/
void do_slookup( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;
    SKILLTYPE* skill = NULL;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Slookup what?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        for ( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
            pager_printf( ch, "Sn: %4d Slot: %4d Skill '%-20s' \n\r",
                          sn, skill_table[sn]->slot, skill_table[sn]->name );
    }
    else if ( !str_cmp( arg, "herbs" ) )
    {
        for ( sn = 0; sn < top_herb && herb_table[sn] && herb_table[sn]->name; sn++ )
            pager_printf( ch, "%d) %s\n\r", sn, herb_table[sn]->name );
    }
    else
    {
        if ( arg[0] == 'h' && is_number( arg + 1 ) )
        {
            sn = atoi( arg + 1 );

            if ( !IS_VALID_HERB( sn ) )
            {
                send_to_char( "Invalid herb.\n\r", ch );
                return;
            }

            skill = herb_table[sn];
        }
        else if ( is_number( arg ) )
        {
            sn = atoi( arg );

            if ( ( skill = get_skilltype( sn ) ) == NULL )
            {
                send_to_char( "Invalid sn.\n\r", ch );
                return;
            }

            sn %= 1000;
        }
        else if ( ( sn = skill_lookup( arg ) ) >= 0 )
            skill = skill_table[sn];
        else if ( ( sn = herb_lookup( arg ) ) >= 0 )
            skill = herb_table[sn];
        else
        {
            send_to_char( "No such skill, proficiency or tongue.\n\r", ch );
            return;
        }

        if ( !skill )
        {
            send_to_char( "Not created yet.\n\r", ch );
            return;
        }

        ch_printf( ch, "&zSn: &C%d &z(Slot: &W%d&z)\n\r&z%s: '&B%-20s&z'\n\r",
                   sn, skill->slot, skill_tname[skill->type], skill->name );
        ch_printf( ch, "&zType: &C%s  &zMinpos: &C%d  &zReset: &C%d  &zRace: &C%d\n\r",
                   skill_tname[skill->type], skill->minimum_position, skill->reset, skill->race );
        ch_printf( ch, "&zCode: &B%s\n\r", skill->skill_fun ? skill_name( skill->skill_fun ) : "(none)" );
        ch_printf( ch, "&zWearoff: &W%s&B\n\r", skill->msg_off ? skill->msg_off : "(none set)" );

        if ( skill->userec.num_uses )
            send_timer( &skill->userec, ch );

        if ( skill->type != SKILL_HERB && skill->race >= 0 && skill->race <= 2 )
        {
            sprintf( buf, "&zRace: &C%s   &zLevel: &C%3d\n\r", race_table[skill->race].race_name, skill->min_level );
            send_to_char( buf, ch );
        }

        send_to_char( "\n\r", ch );
    }

    return;
}

/*
    Set a skill's attributes or what skills a player has.
    High god command, with support for creating skills/herbs/etc
*/
void do_sset( CHAR_DATA* ch, char* argument )
{
    char arg1[MIL], arg2 [MIL];
    char buf[MSL];
    CHAR_DATA* victim;
    int value;
    int sn;
    bool fAll;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Syntax: sset <victim> <skill> <value>\n\r",  ch );
        send_to_char( "or:     sset <victim> all     <value>\n\r",  ch );

        if ( get_trust( ch ) > LEVEL_SUB_IMPLEM )
        {
            send_to_char( "or:     sset save skill table\n\r",        ch );
            send_to_char( "or:     sset save herb table\n\r",     ch );
            send_to_char( "or:     sset create skill 'new skill'\n\r",    ch );
            send_to_char( "or:     sset create herb 'new herb'\n\r",  ch );
        }

        if ( get_trust( ch ) > LEVEL_GREATER )
        {
            send_to_char( "or:     sset <sn>     <field> <value>\n\r",    ch );
            send_to_char( "\n\rField being one of:\n\r",          ch );
            send_to_char( "  name code minpos slot reset wearoff race minlevel type\n\r", ch );
        }

        send_to_char( "Skill being any skill.\n\r",            ch );
        return;
    }

    if ( get_trust( ch ) > LEVEL_SUB_IMPLEM
            &&  !str_cmp( arg1, "save" )
            &&  !str_cmp( argument, "table" ) )
    {
        if ( !str_cmp( arg2, "skill" ) )
        {
            send_to_char( "Saving skill table...\n\r", ch );
            save_skill_table();
            return;
        }

        if ( !str_cmp( arg2, "herb" ) )
        {
            send_to_char( "Saving herb table...\n\r", ch );
            save_herb_table();
            return;
        }
    }

    if ( get_trust( ch ) > LEVEL_SUB_IMPLEM
            &&  !str_cmp( arg1, "create" )
            && ( !str_cmp( arg2, "skill" ) || !str_cmp( arg2, "herb" ) ) )
    {
        struct skill_type* skill;
        sh_int type = SKILL_UNKNOWN;

        if ( !str_cmp( arg2, "herb" ) )
        {
            type = SKILL_HERB;

            if ( top_herb >= MAX_HERB )
            {
                ch_printf( ch, "The current top herb is %d, which is the maximum.  "
                           "To add more herbs,\n\rMAX_HERB will have to be "
                           "raised in mud.h, and the mud recompiled.\n\r",
                           top_sn );
                return;
            }
        }
        else if ( top_sn >= MAX_SKILL )
        {
            ch_printf( ch, "The current top sn is %d, which is the maximum.  "
                       "To add more skills,\n\rMAX_SKILL will have to be "
                       "raised in mud.h, and the mud recompiled.\n\r",
                       top_sn );
            return;
        }

        CREATE( skill, struct skill_type, 1 );

        if ( type == SKILL_HERB )
        {
            int max, x;
            herb_table[top_herb++] = skill;

            for ( max = x = 0; x < top_herb - 1; x++ )
                if ( herb_table[x] && herb_table[x]->slot > max )
                    max = herb_table[x]->slot;

            skill->slot = max + 1;
        }
        else
            skill_table[top_sn++] = skill;

        skill->name = str_dup( argument );
        skill->msg_off = str_dup( "" );
        skill->type = type;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( arg1[0] == 'h' )
        sn = atoi( arg1 + 1 );
    else
        sn = atoi( arg1 );

    if ( get_trust( ch ) > LEVEL_GREATER
            && ( ( arg1[0] == 'h' && is_number( arg1 + 1 ) && ( sn = atoi( arg1 + 1 ) ) >= 0 )
                 ||  ( is_number( arg1 ) && ( sn = atoi( arg1 ) ) >= 0 ) ) )
    {
        struct skill_type* skill;

        if ( arg1[0] == 'h' )
        {
            if ( sn >= top_herb )
            {
                send_to_char( "Herb number out of range.\n\r", ch );
                return;
            }

            skill = herb_table[sn];
        }
        else
        {
            if ( ( skill = get_skilltype( sn ) ) == NULL )
            {
                send_to_char( "Skill number out of range.\n\r", ch );
                return;
            }

            sn %= 1000;
        }

        if ( !str_cmp( arg2, "code" ) )
        {
            DO_FUN*    dofun;

            if ( ( dofun = skill_function( argument ) ) != skill_notfound )
            {
                skill->skill_fun = dofun;
            }
            else
            {
                send_to_char( "Not a skill.\n\r", ch );
                return;
            }

            send_to_char( "Ok.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg2, "minpos" ) )
        {
            skill->minimum_position = URANGE( POS_DEAD, atoi( argument ), POS_DRAG );
            send_to_char( "Ok.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg2, "minlevel" ) )
        {
            skill->min_level = URANGE( 1, atoi( argument ), MAX_LEVEL );
            send_to_char( "Ok.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg2, "slot" ) )
        {
            skill->slot = URANGE( 0, atoi( argument ), 30000 );
            send_to_char( "Ok.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg2, "reset" ) )
        {
            skill->reset = URANGE( 0, atoi( argument ), 120 );
            send_to_char( "Ok.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg2, "race" ) )
        {
            skill->race = atoi( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg2, "type" ) )
        {
            skill->type = get_skill( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg2, "minlevel" ) )
        {
            skill->min_level = URANGE( 1, atoi( argument ), MAX_LEVEL );
            send_to_char( "Ok.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg2, "name" ) )
        {
            DISPOSE( skill->name );
            skill->name = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg2, "wearoff" ) )
        {
            DISPOSE( skill->msg_off );

            if ( str_cmp( argument, "clear" ) )
                skill->msg_off = str_dup( argument );

            send_to_char( "Ok.\n\r", ch );
            return;
        }

        do_sset( ch, "" );
        return;
    }

    if ( ( victim = get_char_world_full( ch, arg1 ) ) == NULL )
    {
        if ( ( sn = skill_lookup( arg1 ) ) >= 0 )
        {
            snprintf( buf, MSL, "%d %s %s", sn, arg2, argument );
            do_sset( ch, buf );
        }
        else
            send_to_char( "They aren't here.\n\r", ch );

        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;

    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
        send_to_char( "No such skill or spell.\n\r", ch );
        return;
    }

    /*
        Snarf the value.
    */
    if ( !is_number( argument ) )
    {
        send_to_char( "Value must be numeric.\n\r", ch );
        return;
    }

    value = atoi( argument );

    if ( value < 0 || value > 3 )
    {
        send_to_char( "Value range is 0 to 3.\n\r", ch );
        return;
    }

    if ( fAll )
    {
        for ( sn = 0; sn < top_sn; sn++ )
        {
            /* Fix by Narn to prevent ssetting skills the player shouldn't have. */
            if ( skill_table[sn]->race != victim->race )
                continue;

            if ( skill_table[sn]->name
                    && ( victim->top_level >= skill_table[sn]->min_level || value == 0 ) )
                victim->pcdata->learned[sn] = value;
        }
    }
    else
        victim->pcdata->learned[sn] = value;

    return;
}


void learn_from_success( CHAR_DATA* ch, int sn )
{
    return;
}


void learn_from_failure( CHAR_DATA* ch, int sn )
{
    return;
}

void do_dig( CHAR_DATA* ch, char* argument )
{
    send_to_char( "skills.c: incomplete code under do_dig", ch );
    return;
}


void do_search( CHAR_DATA* ch, char* argument )
{
    send_to_char( "skills.c: incomplete code under do_search", ch );
    return;
}


void do_hide( CHAR_DATA* ch, char* argument )
{
    CHAR_DATA* rch;

    if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_NAPALM ) )
    {
        send_to_char( "&RYou can't hide while your burning to a crisp.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    {
        send_to_char( "&RYou can't hide while blind - You can't find anywhere to hide!\n\r", ch );
        return;
    }

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( !is_enemy( ch, rch ) )
            continue;

        if ( rch->hit <= 0 )
            continue;

        if ( can_see( ch, rch ) )
        {
            /* Failed hide */
            send_to_char( "&YYou cannot hide while enemies are present.\n\r", ch );
            return;
        }
    }

    if ( xIS_SET( ch->in_room->room_flags, ROOM_NOHIDE ) )
    {
        ch_printf( ch, "&w&RYou cannot hide in this room.\n\r" );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_HIDE ) )
    {
        send_to_char( "&RYou are no longer hidden.\n\r", ch );
        xREMOVE_BIT( ch->affected_by, AFF_HIDE );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CLOAK ) )
    {
        send_to_char( "&RYou can't hide while cloaked.\n\r", ch );
        return;
    }

    if ( ch->block != NULL )
    {
        send_to_char( "&RYou can't hide while blocking.\n\r", ch );
        return;
    }

    if ( ch->mp < 2 )
    {
        send_to_char( "&RNot enough MP to hide.\n\r", ch );
        return;
    }

    ch->mp = UMAX( 0, ch->mp - 2 );
    xSET_BIT( ch->affected_by, AFF_HIDE );
    send_to_char( "&YYou are now hidden from view.\n\r", ch );
    return;
}

void do_recall( CHAR_DATA* ch, char* argument )
{
    ROOM_INDEX_DATA* location;
    location = NULL;
    location = get_room_index( wherehome( ch ) );

    if ( get_trust( ch ) < LEVEL_IMMORTAL )
    {
        AREA_DATA* pArea;

        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            send_to_char( "Only builders can recall.\n\r", ch );
            return;
        }

        if  ( ch->in_room->vnum < pArea->low_r_vnum
                || ch->in_room->vnum > pArea->hi_r_vnum )
        {
            send_to_char( "You can only recall from your assigned area.\n\r", ch );
            return;
        }
    }

    if ( !location )
    {
        send_to_char( "You are completely lost.\n\r", ch );
        return;
    }

    if ( ch->in_room == location )
        return;

    if ( IS_AFFECTED( ch, AFF_CURSE ) )
    {
        send_to_char( "You are cursed and cannot recall!\n\r", ch );
        return;
    }

    act( AT_ACTION, "$n disappears in a swirl of the Force.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );

    if ( ch->mount )
    {
        char_from_room( ch->mount );
        char_to_room( ch->mount, location );
    }

    act( AT_ACTION, "$n appears in a swirl of the Force.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return;
}

void do_scan( CHAR_DATA* ch, char* argument )
{
    ROOM_INDEX_DATA* to_room = NULL;
    EXIT_DATA* pexit = NULL;
    bool can_see = FALSE;
    sh_int dir = -1;
    sh_int mdark = 0;
    sh_int dist;
    sh_int max_dist;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    {
        send_to_char( "&RYour blind, what exactly are you trying to accomplish?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "&RSyntax: SCAN (direction)\n\r", ch );
        return;
    }

    if ( ( dir = get_door( argument ) ) == -1 )
    {
        send_to_char( "&RNot a valid direction, try again.\n\r", ch );
        return;
    }

    act( AT_GREY, "Scanning $t...", ch, dir_name[dir], NULL, TO_CHAR );
    // act( AT_GREY, "$n scans $t.", ch, dir_name[dir], NULL, TO_ROOM );
    max_dist = 1;

    if ( get_curr_per( ch ) >= 10 )
        max_dist++;

    if ( get_curr_per( ch ) >= 20 )
        max_dist++;

    if ( get_curr_per( ch ) >= 30 )
        max_dist++;

    if ( ch->race == RACE_ALIEN )
        max_dist += 2;

    if ( ch->race == RACE_PREDATOR && ch->vision > -1 )
        max_dist += 2;

    if ( ch->race == RACE_PREDATOR && ch->vision < 0 )
        max_dist += 1;

    if ( !IS_NPC( ch ) && ch->pcdata )
    {
        max_dist += ch->pcdata->learned[gsn_eagle_eye];
        mdark = get_dark_range( ch );
    }

    if ( IS_NPC( ch ) || max_dist <= 0 )
    {
        act( AT_GREY, "You stop scanning $t as your vision blurs.", ch, dir_name[dir], NULL, TO_CHAR );
        return;
    }

    if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
    {
        act( AT_GREY, "You can't see $t.", ch, dir_name[dir], NULL, TO_CHAR );
        return;
    }

    for ( dist = 1; dist <= max_dist; )
    {
        if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
        {
            if ( xIS_SET( pexit->exit_info, EX_SECRET ) )
                act( AT_GREY, "Your view $t is blocked by a wall.", ch, dir_name[dir], NULL, TO_CHAR );
            else
                act( AT_GREY, "Your view $t is blocked by a door.", ch, dir_name[dir], NULL, TO_CHAR );

            break;
        }

        if ( xIS_SET( pexit->exit_info, EX_BLASTOPEN ) && !xIS_SET( pexit->exit_info, EX_BLASTED ) )
        {
            act( AT_GREY, "Your view $t is blocked by a wall.", ch, dir_name[dir], NULL, TO_CHAR );
            break;
        }

        to_room = NULL;

        if ( pexit->distance > 1 )
            to_room = generate_exit( to_room, &pexit );

        if ( to_room == NULL )
            to_room = pexit->to_room;

        can_see = TRUE;

        if ( room_is_dark( ch, to_room ) )
        {
            if ( mdark > 0 )
            {
                mdark--;
            }
            else
            {
                can_see = FALSE;
            }
        }

        if ( can_see )
        {
            // char_from_room( ch );
            // char_to_room( ch, to_room );
            ch_printf( ch, "&w&R(%d meters) ", dist * 10 );
            set_char_color( AT_RMNAME, ch );
            send_to_char( to_room->name, ch );
            send_to_char( "\n\r", ch );

            if ( !is_spectator( ch ) )
                run_awareness( to_room->first_person, ch );

            show_list_to_char( to_room->first_content, ch, FALSE, FALSE );
            show_char_to_char( to_room->first_person, ch );
        }
        else
        {
            ch_printf( ch, "&w&R(%d meters) ", dist * 10 );
            set_char_color( AT_DGREY, ch );
            send_to_char( "It is too dark to see. \n\r", ch );
        }

        dist++;

        if ( dist >= max_dist )
        {
            act( AT_GREY, "Your vision blurs with distance and you see no farther $t.", ch, dir_name[dir], NULL, TO_CHAR );
            break;
        }

        if ( ( pexit = get_exit( to_room, dir ) ) == NULL )
        {
            act( AT_GREY, "Your view $t is blocked by a wall.", ch, dir_name[dir], NULL, TO_CHAR );
            break;
        }
    }

    // char_from_room( ch );
    // char_to_room( ch, was_in_room );
    return;
}

void run_awareness( CHAR_DATA* first, CHAR_DATA* ch )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* rch;

    if ( first == NULL )
        return;

    sprintf( buf, "&w&C(Awareness) You get the distinct feeling your being watched.\n\r" );

    for ( rch = first; rch; rch = rch->next_in_room )
    {
        if ( rch->race == RACE_MARINE && rch->pcdata && rch->race != ch->race )
        {
            int sn;

            if ( is_spectator( rch ) )
                continue;

            sn = gsn_awareness;

            if ( rch->pcdata->prepared[sn] >= skill_table[sn]->reset )
            {
                if ( number_range( 1, 100 ) <= ( 25 + ( rch->pcdata->learned[sn] * 25 ) ) )
                {
                    rch->pcdata->prepared[sn] = 0;
                    ch_printf( rch, buf );
                }
            }
        }
    }

    return;
}

void do_nothing( CHAR_DATA* ch, char* argument )
{
    send_to_char( "Huh?\n\r", ch );
    return;
}

void do_treat( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* victim = NULL;
    OBJ_DATA* obj = NULL;
    int levelA = 0;
    int levelB = 0;
    int heal = 0;
    int hmax = 0;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( ch->race != RACE_MARINE )
    {
        do_nothing( ch, "" );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: TREAT (target)\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "You don't see them here.\n\r", ch );
        return;
    }

    if ( !IS_NPC( ch ) )
        levelA = ch->pcdata->learned[gsn_advanced_medical];

    if ( !IS_NPC( ch ) )
        levelB = ch->pcdata->learned[gsn_basic_medical];

    if ( levelA > 0 )
    {
        if ( ch->pcdata->prepared[gsn_advanced_medical] < skill_table[gsn_advanced_medical]->reset )
        {
            send_to_char( "You must wait until Advanced Medical is ready again.\n\r", ch );
            return;
        }

        if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
        {
            send_to_char( "You must be holding a medical kit.\n\r", ch );
            return;
        }

        if ( obj->item_type != ITEM_MEDIKIT )
        {
            send_to_char( "You must be holding a medical kit.\n\r", ch );
            return;
        }

        if ( !IS_NPC( ch ) )
            ch->pcdata->prepared[gsn_advanced_medical] = 0;

        /* Get HP Boost */
        heal = URANGE( 0, victim->max_hit - victim->hit, 30 );
        victim->hit += heal;
        victim->mp = UMAX( 0, victim->mp - 1 );
        ch->mp = 0;

        if ( ch == victim )
        {
            ch_printf( ch, "&wYou carefully treat your injuries. &z(&G+%d Hitpoints&z)\n\r", heal );
            act( AT_WHITE, "&w$n carefully tends to $s wounds.", ch, NULL, victim, TO_ROOM );
        }
        else
        {
            sprintf( buf, "&wYou carefully tend to $N's injuries. &z(&G+%d Hitpoints&z)", heal );
            act( AT_WHITE, buf,  ch, NULL, victim, TO_CHAR );
            sprintf( buf, "&w$n carefully tends to your wounds. &z(&G+%d Hitpoints&z)", heal );
            act( AT_WHITE, buf, ch, NULL, victim, TO_VICT );
            act( AT_WHITE, "&w$n carefully tends to $N.",  ch, NULL, victim, TO_NOTVICT );
        }

        update_pos( victim );

        /* Cure rolly-burny */
        if ( levelA > 1 && IS_AFFECTED( victim, AFF_NAPALM ) )
        {
            AFFECT_DATA* paf;

            for ( paf = victim->first_affect; paf; paf = paf->next )
                if ( xIS_SET( paf->bitvector, AFF_NAPALM ) )
                    if ( paf->duration > 0 )
                        paf->duration = 0;

            send_to_char( "&CThe fires are extinguished!\n\r", victim );
        }

        /* Cure blindness/shock */
        if ( levelA > 1 && IS_AFFECTED( victim, AFF_BLIND ) )
        {
            affect_strip( victim, gsn_acidspit );
            send_to_char( "&CYour vision returns!\n\r", victim );
        }

        /* Cure chestbursters. */
        if ( levelA > 2 )
        {
        }

        return;
    }
    else if ( levelB > 0 )
    {
        if ( ch->pcdata->prepared[gsn_basic_medical] < skill_table[gsn_basic_medical]->reset )
        {
            send_to_char( "You must wait until Basic Medical is ready again.\n\r", ch );
            return;
        }

        hmax = ( int )( ( float )( ( float )( victim->max_hit ) / ( float )( 100 ) ) * ( float )( 60 + ( levelB * 10 ) ) );

        if ( victim->hit >= hmax )
        {
            send_to_char( "You can't heal them anymore than their at.\n\r", ch );
            return;
        }

        if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
        {
            send_to_char( "You must be holding a roll of bandages.\n\r", ch );
            return;
        }

        if ( obj->item_type != ITEM_BANDAGE )
        {
            send_to_char( "You must be holding a roll of bandages.\n\r", ch );
            return;
        }

        heal = ( levelB * 10 );
        hmax = UMAX( 0, ( hmax - victim->hit ) );
        heal = URANGE( 0, heal, hmax );
        victim->hit += heal;
        victim->mp = UMAX( 0, victim->mp - 1 );
        ch->mp = 0;

        if ( !IS_NPC( ch ) )
            ch->pcdata->prepared[gsn_basic_medical] = 0;

        if ( obj != NULL )
        {
            separate_obj( obj );
            obj_from_char( obj );
            extract_obj( obj );
        }

        if ( ch == victim )
        {
            ch_printf( ch, "&wYou carefully bandage your wounds. &z(&G+%d Hitpoints&z)\n\r", heal );
            act( AT_WHITE, "&w$n carefully bandages up $s wounds.", ch, NULL, victim, TO_ROOM );
        }
        else
        {
            sprintf( buf, "&wYou carefully bandage up $N. &z(&G+%d Hitpoints&z)", heal );
            act( AT_WHITE, buf,  ch, NULL, victim, TO_CHAR );
            sprintf( buf, "&w$n carefully applies a roll of bandages. &z(&G+%d Hitpoints&z)", heal );
            act( AT_WHITE, buf, ch, NULL, victim, TO_VICT );
            act( AT_WHITE, "&w$n carefully applies a roll of bandages to $N.",  ch, NULL, victim, TO_NOTVICT );
        }

        update_pos( victim );
    }
    else
    {
        send_to_char( "You need either BASIC or ADVANCED Medical to use TREAT.\n\r", ch );
        return;
    }

    return;
}

void do_spit( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA* victim = NULL;
    OBJ_DATA* obj = NULL;
    AFFECT_DATA af;
    int duration = 0;
    int level = 2;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( ch->race != RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: SPIT (target)\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "You don't see them here.\n\r", ch );
        return;
    }

    if ( victim == ch || victim->race == RACE_ALIEN )
    {
        send_to_char( "I dont think you want to attack them.\n\r", ch );
        return;
    }

    if ( victim->race == RACE_PREDATOR )
    {
        send_to_char( "Predators are immune to acid spit!\n\r", ch );
        return;
    }

    if ( !IS_NPC( ch ) )
        level = ch->pcdata->learned[gsn_acidspit];

    if ( level > 0 )
    {
        if ( !IS_NPC( ch ) )
        {
            if ( ch->pcdata->prepared[gsn_acidspit] < skill_table[gsn_acidspit]->reset )
            {
                send_to_char( "You must wait until Acid Spit is ready again.\n\r", ch );
                return;
            }

            ch->pcdata->prepared[gsn_acidspit] = 0;
        }

        ch->hit -= ( ( level + 2 ) * 3 );
        ch->ap = UMAX( 0, ch->ap - 1 );
        victim->ap = UMAX( 0, victim->ap - 1 );

        if ( ( obj = get_eq_char( victim, WEAR_EYES ) ) == NULL )
        {
            victim->hit -= ( ( level + 2 ) * 3 );
            duration = number_range( level * 10, ( level + 1 ) * 10 );
            af.type      = gsn_acidspit;
            af.location  = APPLY_PER;
            af.modifier  = -10;
            af.duration  = duration;
            xCLEAR_BITS( af.bitvector );
            xSET_BIT( af.bitvector, AFF_BLIND );
            affect_join( victim, &af );
            xREMOVE_BIT( ch->affected_by, AFF_HIDE );
            sprintf( buf, "&w&CYou spit searing acid into $N's eyes. &z(&W%d Rounds&z)", duration );
            act( AT_WHITE, buf,  ch, NULL, victim, TO_CHAR );
            sprintf( buf, "&w&C$n spits searing acid into your eyes! &z(&W%d Rounds&z)", duration );
            act( AT_WHITE, buf, ch, NULL, victim, TO_VICT );
            act( AT_WHITE, "&w&C$n spits acid into $N's eyes!",  ch, NULL, victim, TO_NOTVICT );
            act( AT_WHITE, "&w&RYou can't see a damn thing!", victim, NULL, NULL, TO_CHAR );
            update_pos( victim );
        }
        else
        {
            int rtn = 0;

            if ( rtn != rOBJ_SCRAPPED )
                rtn = damage_obj( obj, level + 2 );

            if ( rtn != rOBJ_SCRAPPED )
                rtn = damage_obj( obj, level + 2 );

            if ( rtn != rOBJ_SCRAPPED )
                rtn = damage_obj( obj, level + 2 );

            if ( rtn == rOBJ_SCRAPPED )
            {
                victim->hit -= ( ( level + 2 ) * 3 );
                duration = number_range( level * 10, ( level + 1 ) * 10 ) / 2;
                af.type      = gsn_acidspit;
                af.location  = APPLY_PER;
                af.modifier  = -10;
                af.duration  = duration;
                xCLEAR_BITS( af.bitvector );
                xSET_BIT( af.bitvector, AFF_BLIND );
                affect_join( victim, &af );
                xREMOVE_BIT( ch->affected_by, AFF_HIDE );
                sprintf( buf, "&w&CYou spit searing acid into $N's eyes. &z(&W%d Rounds&z)", duration );
                act( AT_WHITE, buf,  ch, NULL, victim, TO_CHAR );
                sprintf( buf, "&w&C$n spits searing acid into your eyes! &z(&W%d Rounds&z)", duration );
                act( AT_WHITE, buf, ch, NULL, victim, TO_VICT );
                act( AT_WHITE, "&w&C$n spits acid into $N's eyes!",  ch, NULL, victim, TO_NOTVICT );
                act( AT_WHITE, "&w&RYou can't see a damn thing!", victim, NULL, NULL, TO_CHAR );
                update_pos( victim );
            }
            else
            {
                xREMOVE_BIT( ch->affected_by, AFF_HIDE );
                sprintf( buf, "&w&CYou spit searing acid into $N's eyes. &z(&WSoaked!&z)" );
                act( AT_WHITE, buf,  ch, NULL, victim, TO_CHAR );
                sprintf( buf, "&w&C$n spits searing acid into your eyes! &z(&WSoaked!&z)" );
                act( AT_WHITE, buf, ch, NULL, victim, TO_VICT );
                act( AT_WHITE, "&w&C$n spits acid into $N's eyes! (Soaked)",  ch, NULL, victim, TO_NOTVICT );
            }
        }

        return;
    }
    else
    {
        send_to_char( "You need ACID SPIT to use this skill.\n\r", ch );
        return;
    }

    return;
}

void do_request( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA*   pMobIndex;
    OBJ_INDEX_DATA*   pObjIndex;
    CHAR_DATA*        mob[3];
    OBJ_DATA*         obj;
    int level = 0;
    int i = 0;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( ch->race != RACE_MARINE )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( !IS_NPC( ch ) )
        level = ch->pcdata->learned[gsn_requestassist];

    if ( level <= 0 )
    {
        send_to_char( "You need REQUEST ASSISTANCE to use this skill.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: REQUEST (type)\n\r", ch );
        send_to_char( " Sniper  :: Request a USCM sniper team. Level 1.\n\r", ch );
        send_to_char( " Assault :: Request a USCM assault team. Level 2.\n\r", ch );
        send_to_char( " Support :: Request a USCM support team. Level 3.\n\r", ch );
        return;
    }

    if ( xIS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
    {
        send_to_char( "You must be outdoors to request assistance.\n\r", ch );
        return;
    }

    if ( ch->mp < get_max_mp( ch ) || ch->ap < get_max_ap( ch ) )
    {
        send_to_char( "You must be at full AP and MP to request assistance.\n\r", ch );
        return;
    }

    if ( !IS_NPC( ch ) )
    {
        if ( ch->pcdata->prepared[gsn_requestassist] < skill_table[gsn_requestassist]->reset )
        {
            send_to_char( "You must wait until Request Assistance is ready again.\n\r", ch );
            return;
        }
    }

    if ( ( pMobIndex = get_mob_index( MOB_VNUM_TEMPLATE ) ) == NULL )
        return;

    if ( !str_cmp( arg, "sniper" ) && level >= 1 )
    {
        ch->ap = 0;
        ch->mp = 0;

        if ( !IS_NPC( ch ) )
            ch->pcdata->prepared[gsn_requestassist] = 0;

        for ( i = 0; i < 2; i++ )
        {
            mob[i] = create_mobile( pMobIndex );
            char_to_room( mob[i], ch->in_room );
            STRFREE( mob[i]->name );
            STRFREE( mob[i]->short_descr );
            STRFREE( mob[i]->long_descr );
            mob[i]->name = STRALLOC( "marine guard" );
            mob[i]->short_descr = STRALLOC( "A Marine" );
            mob[i]->long_descr = STRALLOC( "A Marine is standing here.\n\r" );
            act( AT_IMMORT, "$N has arrived.", ch, NULL, mob[i], TO_ROOM );
            mob[i]->top_level = ch->top_level;
            mob[i]->max_hit = 200;
            mob[i]->hit = mob[i]->max_hit;
            mob[i]->race = ch->race;
            mob[i]->sex = ch->sex;
            // mob[i]->master = ch;

            if ( i == 0 )
                pObjIndex = get_obj_index( 681 );  // Scope Ammo

            if ( i == 1 )
                pObjIndex = get_obj_index( 686 );  // Slugshot

            if ( pObjIndex != NULL )
            {
                obj = create_object( pObjIndex, mob[i]->top_level );
                obj_to_char( obj, mob[i] );
                obj = create_object( pObjIndex, mob[i]->top_level );
                obj_to_char( obj, mob[i] );
                obj = create_object( pObjIndex, mob[i]->top_level );
                obj_to_char( obj, mob[i] );
            }

            if ( i == 0 )
                pObjIndex = get_obj_index( 680 );  // Scope Rifle

            if ( i == 1 )
                pObjIndex = get_obj_index( 684 );  // Shotgun

            if ( pObjIndex != NULL )
            {
                obj = create_object( pObjIndex, mob[i]->top_level );
                obj_to_char( obj, mob[i] );
                equip_char( mob[i], obj, WEAR_WIELD );
                mob_reload( mob[i], obj );
            }
        }

        return;
    }
    else if ( !str_cmp( arg, "assault" ) && level >= 2 )
    {
        ch->ap = 0;
        ch->mp = 0;

        if ( !IS_NPC( ch ) )
            ch->pcdata->prepared[gsn_requestassist] = 0;

        for ( i = 0; i < 3; i++ )
        {
            mob[i] = create_mobile( pMobIndex );
            char_to_room( mob[i], ch->in_room );
            STRFREE( mob[i]->name );
            STRFREE( mob[i]->short_descr );
            STRFREE( mob[i]->long_descr );
            mob[i]->name = STRALLOC( "marine guard" );
            mob[i]->short_descr = STRALLOC( "A Marine" );
            mob[i]->long_descr = STRALLOC( "A Marine is standing here.\n\r" );
            act( AT_IMMORT, "$N has arrived.", ch, NULL, mob[i], TO_ROOM );
            mob[i]->top_level = ch->top_level;
            mob[i]->max_hit = 200;
            mob[i]->hit = mob[i]->max_hit;
            mob[i]->race = ch->race;
            mob[i]->sex = ch->sex;
            mob[i]->master = ch;

            if ( i == 2 )
                pObjIndex = get_obj_index( 674 );  // M56 Ammo

            if ( i != 2 )
                pObjIndex = get_obj_index( 81 );   // M309 Ammo

            if ( pObjIndex != NULL )
            {
                obj = create_object( pObjIndex, mob[i]->top_level );
                obj_to_char( obj, mob[i] );
                obj = create_object( pObjIndex, mob[i]->top_level );
                obj_to_char( obj, mob[i] );

                if ( i != 2 )
                {
                    obj = create_object( pObjIndex, mob[i]->top_level );
                    obj_to_char( obj, mob[i] );
                }
            }

            if ( i == 2 )
                pObjIndex = get_obj_index( 673 );  // Smartgun

            if ( i != 2 )
                pObjIndex = get_obj_index( 80 );   // Pulse Rifle

            if ( pObjIndex != NULL )
            {
                obj = create_object( pObjIndex, mob[i]->top_level );
                obj_to_char( obj, mob[i] );
                equip_char( mob[i], obj, WEAR_WIELD );
                mob_reload( mob[i], obj );
            }
        }

        return;
    }
    else if ( !str_cmp( arg, "support" ) && level >= 3 )
    {
        ch->ap = 0;
        ch->mp = 0;

        if ( !IS_NPC( ch ) )
            ch->pcdata->prepared[gsn_requestassist] = 0;

        for ( i = 0; i < 3; i++ )
        {
            mob[i] = create_mobile( pMobIndex );
            char_to_room( mob[i], ch->in_room );
            STRFREE( mob[i]->name );
            STRFREE( mob[i]->short_descr );
            STRFREE( mob[i]->long_descr );
            mob[i]->name = STRALLOC( "marine guard" );
            mob[i]->short_descr = STRALLOC( "A Marine" );
            mob[i]->long_descr = STRALLOC( "A Marine is standing here.\n\r" );
            act( AT_IMMORT, "$N has arrived.", ch, NULL, mob[i], TO_ROOM );
            mob[i]->top_level = ch->top_level;
            mob[i]->max_hit = 200;
            mob[i]->hit = mob[i]->max_hit;
            mob[i]->race = ch->race;
            mob[i]->sex = ch->sex;
            mob[i]->master = ch;

            if ( i == 0 )
                pObjIndex = get_obj_index( 163 );  // M90 Ammo

            if ( i == 1 )
                pObjIndex = get_obj_index( 674 );  // M56 Ammo

            if ( i == 2 )
                pObjIndex = get_obj_index( 81 );   // M309 Ammo

            if ( pObjIndex != NULL )
            {
                obj = create_object( pObjIndex, mob[i]->top_level );
                obj_to_char( obj, mob[i] );
                obj = create_object( pObjIndex, mob[i]->top_level );
                obj_to_char( obj, mob[i] );

                if ( i == 2 )
                {
                    obj = create_object( pObjIndex, mob[i]->top_level );
                    obj_to_char( obj, mob[i] );
                }
            }

            if ( i == 0 )
                pObjIndex = get_obj_index( 162 );  // Minigun

            if ( i == 1 )
                pObjIndex = get_obj_index( 673 );  // Smartgun

            if ( i == 2 )
                pObjIndex = get_obj_index( 80 );   // Pulse Rifle

            if ( pObjIndex != NULL )
            {
                obj = create_object( pObjIndex, mob[i]->top_level );
                obj_to_char( obj, mob[i] );
                equip_char( mob[i], obj, WEAR_WIELD );
                mob_reload( mob[i], obj );
            }
        }

        return;
    }
    else
    {
        do_request( ch, "" );
    }

    return;
}

void do_confuse( CHAR_DATA* ch, char* argument )
{
    AFFECT_DATA af;
    int duration = 0;
    int level = 2;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( ch->race != RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( !IS_NPC( ch ) )
        level = ch->pcdata->learned[gsn_confuse];

    if ( level > 0 )
    {
        if ( !IS_NPC( ch ) )
        {
            if ( ch->pcdata->prepared[gsn_confuse] < skill_table[gsn_confuse]->reset )
            {
                send_to_char( "You must wait until Confuse is ready again.\n\r", ch );
                return;
            }

            ch->pcdata->prepared[gsn_confuse] = 0;
        }

        duration = ( 10 * level );
        af.type      = gsn_confuse;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.duration  = duration;
        xCLEAR_BITS( af.bitvector );
        affect_join( ch, &af );
        ch_printf( ch, "&C(Confuse) You silently call for your fellow aliens.\n\r" );
        return;
    }
    else
    {
        send_to_char( "You need CONFUSE to use this skill.\n\r", ch );
        return;
    }

    return;
}

void do_rage( CHAR_DATA* ch, char* argument )
{
    AFFECT_DATA af;
    int duration = 0;
    int level = 2;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( ch->race != RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( ch->pcdata )
        level = ch->pcdata->learned[gsn_alien_rage];

    if ( level > 0 )
    {
        if ( !IS_NPC( ch ) )
        {
            if ( ch->pcdata->prepared[gsn_alien_rage] < skill_table[gsn_alien_rage]->reset )
            {
                send_to_char( "You must wait until Alien Rage is ready again.\n\r", ch );
                return;
            }

            ch->pcdata->prepared[gsn_alien_rage] = 0;
        }

        duration = URANGE( 10, 5 + ( 5 * level ), 20 );
        af.type      = gsn_alien_rage;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.duration  = duration;
        xCLEAR_BITS( af.bitvector );
        affect_join( ch, &af );
        ch_printf( ch, "&C(Alien Rage) You feel nearly invincible. 50%% of all damage will be soaked.\n\r" );
        return;
    }
    else
    {
        send_to_char( "You need ALIEN RAGE to use this skill.\n\r", ch );
        return;
    }

    return;
}

void do_hive ( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    int ticks = 4;
    int exp = 10;
    strcpy( arg, argument );

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( ch->vent )
    {
        send_to_char( "&RYou can't hive a room while your inside a vent.\n\r", ch );
        return;
    }

    if ( ch->race != RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    switch ( ch->substate )
    {
        default:
            if ( xIS_SET( ch->in_room->room_flags, ROOM_HIVED ) )
            {
                send_to_char( "&RThis room is already hived, genius.\n\r", ch );
                return;
            }

            if ( !can_hive_room( ch, ch->in_room ) )
            {
                send_to_char( "&RSorry, this room cannot be hived.\n\r", ch );
                return;
            }

            if ( ch->resin < 10 )
            {
                send_to_char( "&RSorry, you dont have enough resin stored.\n\r", ch );
                return;
            }

            ch->resin -= 2;
            // ch->ap = UMAX( 0, ch->ap - 2 );
            // ch->mp = UMAX( 0, ch->mp - 2 );
            send_to_char( "&GYou begin working to hive the room.\n\r", ch );
            act( AT_PLAIN, "$n begins hiving the room.", ch, NULL, argument, TO_ROOM );
            add_timer ( ch, TIMER_DO_FUN, ticks, do_hive, 1 );
            ch->dest_buf = str_dup( arg );
            return;

        case 1:
            if ( !ch->dest_buf )
                return;

            strcpy( arg, ch->dest_buf );
            DISPOSE( ch->dest_buf );
            break;

        case SUB_TIMER_DO_ABORT:
            DISPOSE( ch->dest_buf );
            ch->substate = SUB_NONE;
            send_to_char( "&RYou are interupted before you can finish your work.\n\r", ch );
            return;
    }

    ch->substate = SUB_NONE;
    ch->mp = 0;
    ch->resin -= 8;
    send_to_char( "&GYou have succesfully hived this room.\n\r", ch );

    if ( xIS_SET( ch->in_room->room_flags, ROOM_HIVED ) )
        return;

    hflag_toggle( ch->in_room );
    xTOGGLE_BIT( ch->in_room->room_flags, ROOM_HIVED );

    if ( curr_arena )
        curr_arena->hive++;

    ch_printf( ch, "&CYou gain %d experience for your success.\n\r", exp );
    gain_exp( ch, exp );
    return;
}

void do_dehive ( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA* obj = NULL;
    int ticks = 6;
    int exp = 10;
    strcpy( arg, argument );

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( ch->race == RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    switch ( ch->substate )
    {
        default:
            if ( !xIS_SET( ch->in_room->room_flags, ROOM_HIVED ) )
            {
                send_to_char( "&RThis room isn't even hived, genius.\n\r", ch );
                return;
            }

            if ( !can_hive_room( ch, ch->in_room ) )
            {
                send_to_char( "&RSorry, this room cannot be dehived.\n\r", ch );
                return;
            }

            // Flamethrower bonus?
            if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
                if ( obj->item_type == ITEM_WEAPON )
                    if ( obj->value[0] == WEAPON_FLAMETHROWER )
                    {
                        ticks = 2;
                        ch->ap = 0;
                    }

            // Predator Penalty
            if ( ch->race == RACE_PREDATOR )
                ticks += 2;

            // Trapped Penalty
            if ( count_traps( ch->in_room ) > 0 )
                ticks += 2;

            // ch->ap = UMAX( 0, ch->ap - 2 );
            // ch->mp = UMAX( 0, ch->mp - 2 );
            send_to_char( "&GYou begin working to dehive the room.\n\r", ch );
            act( AT_PLAIN, "$n begins dehiving the room.", ch, NULL, argument, TO_ROOM );
            add_timer ( ch, TIMER_DO_FUN, ticks, do_dehive, 1 );
            ch->dest_buf = str_dup( arg );
            return;

        case 1:
            if ( !ch->dest_buf )
                return;

            strcpy( arg, ch->dest_buf );
            DISPOSE( ch->dest_buf );
            break;

        case SUB_TIMER_DO_ABORT:
            DISPOSE( ch->dest_buf );
            ch->substate = SUB_NONE;
            send_to_char( "&RYou are interupted before you can finish your work.\n\r", ch );
            return;
    }

    ch->substate = SUB_NONE;
    ch->mp = 0;
    send_to_char( "&GYou have succeeded in dehiving this room.\n\r", ch );

    if ( !xIS_SET( ch->in_room->room_flags, ROOM_HIVED ) )
        return;

    hflag_toggle( ch->in_room );
    xTOGGLE_BIT( ch->in_room->room_flags, ROOM_HIVED );
    clear_traps( ch->in_room );

    if ( curr_arena )
        curr_arena->hive--;

    ch_printf( ch, "&CYou gain %d experience for your success.\n\r", exp );
    gain_exp( ch, exp );
    return;
}


void do_lunge( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* to_room;
    CHAR_DATA* rch;
    EXIT_DATA* pexit;
    bool multistun = FALSE;
    bool found = FALSE;
    int range = 0, mrange = 2;
    int dir = 0;
    int stun = 3;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ch->race != RACE_ALIEN )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RYou may not engage in combat while in spectator mode.\n\r", ch );
        return;
    }

    if ( ch->vent )
    {
        send_to_char( "&RYou cannot lunge while inside a vent.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' || ( dir = get_door( arg1 ) ) == -1 )
    {
        send_to_char( "&RSyntax: LUNGE (direction) [optional target]\n\r", ch );
        return;
    }

    if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
    {
        send_to_char( "&RAre you expecting to lunge through a wall!?\n\r", ch );
        return;
    }

    if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
    {
        send_to_char( "Are you expecting to lunge through a door!?\n\r", ch );
        return;
    }

    xREMOVE_BIT( ch->affected_by, AFF_HIDE );
    ch->ap = 0;
    ch->mp -= 2;

    if ( !IS_NPC( ch ) )
    {
        if ( ch->pcdata->learned[gsn_lunge] == 1 )
        {
            mrange = 2;
            stun = 3;
            multistun = FALSE;
        }

        if ( ch->pcdata->learned[gsn_lunge] == 2 )
        {
            mrange = 3;
            stun = 6;
            multistun = FALSE;
        }

        if ( ch->pcdata->learned[gsn_lunge] == 3 )
        {
            mrange = 3;
            stun = 6;
            multistun = TRUE;
        }

        ch->pcdata->prepared[gsn_lunge] = 0;
    }

    ch_printf( ch, "&RYou start running and lunge to %s.\n\r", main_exit( dir ) );
    sprintf( buf, "$n takes a running lunge %s.", dir_name[dir] );
    act( AT_BLOOD, buf, ch, NULL, NULL, TO_ROOM );
    pexit = get_exit( ch->in_room, dir );

    for ( range = 1; range <= mrange; range++ )
    {
        if ( !pexit )
            break;

        if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
        {
            ch_printf( ch, "&Y(Stopped) You *SMASH* into the closed door!\n\r" );
            damage( ch, ch, get_curr_str( ch ), TYPE_GENERIC + RIS_IMPACT );
            WAIT_STATE( ch, 6 );
            return;
        }

        if ( xIS_SET( pexit->exit_info, EX_NOVENT ) && ch->vent == TRUE )
        {
            ch_printf( ch, "&Y(Stopped) You *SMASH* into the vent wall!\n\r" );
            damage( ch, ch, get_curr_str( ch ), TYPE_GENERIC + RIS_IMPACT );
            WAIT_STATE( ch, 6 );
            return;
        }

        if ( xIS_SET( pexit->exit_info, EX_BARRED ) )
        {
            ch_printf( ch, "&Y(Stopped) You *SMASH* into the barred window!\n\r" );
            damage( ch, ch, get_curr_str( ch ), TYPE_GENERIC + RIS_IMPACT );
            WAIT_STATE( ch, 6 );
            return;
        }

        if ( xIS_SET( pexit->exit_info, EX_BLASTOPEN ) && !xIS_SET( pexit->exit_info, EX_BLASTED ) )
        {
            ch_printf( ch, "&Y(Stopped) You *SMASH* into the wall!\n\r" );
            damage( ch, ch, get_curr_str( ch ), TYPE_GENERIC + RIS_IMPACT );
            WAIT_STATE( ch, 6 );
            return;
        }

        if ( !pexit->to_room )
            break;

        to_room = NULL;

        if ( pexit->distance > 1 )
            to_room = generate_exit( ch->in_room, &pexit );

        if ( to_room == NULL )
            to_room = pexit->to_room;

        found = FALSE;
        char_from_room( ch );
        char_to_room( ch, to_room );
        set_char_color( AT_RMNAME, ch );
        send_to_char( to_room->name, ch );
        send_to_char( "\n\r", ch );
        // show_list_to_char( to_room->first_content, ch, FALSE, FALSE );
        show_char_to_char( to_room->first_person, ch );

        if ( ( rch = get_char_room( ch, arg2 ) ) != NULL )
            found = TRUE;

        // Target located
        if ( found )
        {
            if ( !is_spectator( rch ) && can_see( ch, rch ) && rch->vent == ch->vent )
            {
                if ( rch->race == RACE_PREDATOR )
                    stun = 5;

                ch_printf( rch, "&CAn Alien hits you with a flying lunge from %s!\n\r", rev_exit( dir ) );
                ch_printf( rch, "&Y(Stunned) You have been stunned for %d rounds.\n\r", stun );
                act( AT_LBLUE, "\n\rYou hit $N with a flying lunge, stunning $M.", ch, NULL, rch, TO_CHAR );
                damage( ch, rch, get_curr_str( ch ) * 4, TYPE_GENERIC + RIS_IMPACT );
                WAIT_STATE( rch, stun * 4 );
                WAIT_STATE( ch, 8 );

                if ( to_room->area )
                    motion_ping( to_room->x, to_room->y, to_room->z, to_room->area, ch );

                return;
            }
        }
        else if ( multistun == TRUE && arg2[0] == '\0' )
        {
            CHAR_DATA* rtmp;
            int tstun;

            // ANY Enemies present?
            for ( rch = to_room->first_person; rch; rch = rtmp )
            {
                rtmp = rch->next_in_room;

                if ( rch == ch )
                    continue;

                if ( rch->vent != ch->vent )
                    continue;

                if ( is_spectator( rch ) )
                    continue;

                if ( rch->cover != NULL )
                    return;

                if ( !can_see( ch, rch ) )
                    continue;

                if ( !is_enemy( ch, rch ) )
                    continue;

                if ( IS_AFFECTED( rch, AFF_HIDE ) )
                    if ( number_range( 1, 2 ) == 1 )
                        continue;

                tstun = stun;

                if ( rch->race == RACE_PREDATOR )
                    tstun = 4;

                ch_printf( rch, "&CAn Alien hits you with a flying lunge from %s!\n\r", rev_exit( dir ) );
                ch_printf( rch, "&Y(Stunned) You have been stunned for %d rounds.\n\r", stun );
                act( AT_LBLUE, "You hit $N with a flying lunge, stunning $M.", ch, NULL, rch, TO_CHAR );
                damage( ch, rch, get_curr_str( ch ) * 2, TYPE_GENERIC + RIS_IMPACT );
                WAIT_STATE( rch, tstun * 4 );
                WAIT_STATE( ch, 6 );
                found = TRUE;
            }

            if ( found )
            {
                if ( to_room->area )
                    motion_ping( to_room->x, to_room->y, to_room->z, to_room->area, ch );

                return;
            }
        }

        if ( ( pexit = get_exit( to_room, dir ) ) == NULL )
        {
            sprintf( buf, "&rAn Alien lunges from %s and hits a wall.", rev_exit( dir ) );
            echo_to_room_ignore( ch, to_room, buf );
            break;
        }

        if ( range <= mrange )
        {
            sprintf( buf, "&rAn Alien lunges through the room. (%s to %s)", dir_name[flip_dir( dir )], dir_name[dir] );
            echo_to_room_ignore( ch, to_room, buf );
        }
    }

    if ( range > mrange )
    {
        sprintf( buf, "&rAn Alien lunges in from %s and lands neatly.", rev_exit( dir ) );
        echo_to_room_ignore( ch, ch->in_room, buf );
    }

    to_room = ch->in_room;

    if ( to_room->area )
        motion_ping( to_room->x, to_room->y, to_room->z, to_room->area, ch );

    /* Trigger room progs */
    if ( !IN_VENT( ch ) )
    {
        mprog_entry_trigger( ch );

        if ( char_died( ch ) )
            return;

        rprog_enter_trigger( ch );

        if ( char_died( ch ) )
            return;

        mprog_greet_trigger( ch );

        if ( char_died( ch ) )
            return;

        oprog_greet_trigger( ch );

        if ( char_died( ch ) )
            return;
    }

    ch_printf( ch, "&rDamn. Doesn't look like you hit anything.\n\r" );
    return;
}

bool destroy_door( ROOM_INDEX_DATA* room, EXIT_DATA* pexit, bool bonus )
{
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA* rexit = NULL;
    bool val = FALSE;

    if ( room == NULL || pexit == NULL )
        return FALSE;

    if ( !xIS_SET( pexit->exit_info, EX_CLOSED ) )
        return FALSE;

    sprintf( buf, "&w&YThe door to %s bursts apart!", main_exit( pexit->vdir ) );
    echo_to_room( -1, room, buf );
    xREMOVE_BIT( pexit->exit_info, EX_CLOSED );
    xREMOVE_BIT( pexit->exit_info, EX_LOCKED );
    val = TRUE;

    if ( bonus && pexit->to_room )
    {
        if ( ( rexit = get_exit( pexit->to_room, rev_dir[pexit->vdir] ) ) == NULL )
            return val;

        if ( destroy_door( pexit->to_room, pexit, FALSE ) )
            val = TRUE;
    }

    return val;
}

void do_leap( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* to_room;
    EXIT_DATA* pexit;
    bool antidoor = FALSE;
    int range = 0, mrange = 1;
    int dir = 0;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ch->race != RACE_ALIEN || IS_NPC( ch ) )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RYou may not activate skills while in spectator mode.\n\r", ch );
        return;
    }

    if ( ch->pcdata->prepared[gsn_leap] < skill_table[gsn_leap]->reset )
    {
        ch_printf( ch, "&w&GYou must wait for your LEAP skill to reset.\n\r" );
        return;
    }

    if ( arg1[0] == '\0' || ( dir = get_door( arg1 ) ) == -1 )
    {
        send_to_char( "&RSyntax: LEAP (direction) [optional distance]\n\r", ch );
        return;
    }

    if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
    {
        send_to_char( "&RAre you expecting to leap through a wall!?\n\r", ch );
        return;
    }

    mrange += ch->pcdata->learned[gsn_leap];

    if ( ch->pcdata->learned[gsn_leap] == 3 )
        antidoor = TRUE;

    if ( arg2[0] != '\0' && atoi( arg2 ) > 0 )
        mrange = URANGE( 1, atoi( arg2 ), mrange );

    if ( xIS_SET( pexit->exit_info, EX_CLOSED ) && ( xIS_SET( pexit->exit_info, EX_ARMORED ) || !antidoor ) )
    {
        send_to_char( "Are you expecting to leap through a solid door!?\n\r", ch );
        return;
    }

    xREMOVE_BIT( ch->affected_by, AFF_HIDE );
    ch->ap -= 1;
    ch->mp -= 1;
    ch->pcdata->prepared[gsn_leap] = 0;
    ch_printf( ch, "&RYou start running and leap to %s.\n\r", main_exit( dir ) );
    sprintf( buf, "$n takes a running leap %s.", dir_name[dir] );
    act( AT_BLOOD, buf, ch, NULL, NULL, TO_ROOM );
    destroy_door( ch->in_room, pexit, TRUE );
    pexit = get_exit( ch->in_room, dir );

    for ( range = 1; range <= mrange; range++ )
    {
        if ( !pexit )
            break;

        if ( xIS_SET( pexit->exit_info, EX_CLOSED ) && ( xIS_SET( pexit->exit_info, EX_ARMORED ) || !antidoor ) )
        {
            ch_printf( ch, "&Y(Stopped) You *SMASH* into the closed door!\n\r" );
            damage( ch, ch, get_curr_str( ch ), TYPE_GENERIC + RIS_IMPACT );
            WAIT_STATE( ch, 6 );
            return;
        }
        else
        {
            destroy_door( ch->in_room, pexit, TRUE );
        }

        if ( xIS_SET( pexit->exit_info, EX_NOVENT ) && ch->vent == TRUE )
            break;

        if ( xIS_SET( pexit->exit_info, EX_BARRED ) )
        {
            ch_printf( ch, "&Y(Stopped) You *SMASH* into the barred window!\n\r" );
            damage( ch, ch, get_curr_str( ch ), TYPE_GENERIC + RIS_IMPACT );
            WAIT_STATE( ch, 6 );
            return;
        }

        if ( xIS_SET( pexit->exit_info, EX_BLASTOPEN ) && !xIS_SET( pexit->exit_info, EX_BLASTED ) )
        {
            ch_printf( ch, "&Y(Stopped) You *SMASH* into the wall!\n\r" );
            damage( ch, ch, get_curr_str( ch ), TYPE_GENERIC + RIS_IMPACT );
            WAIT_STATE( ch, 6 );
            return;
        }

        if ( !pexit->to_room )
            break;

        to_room = NULL;

        if ( pexit->distance > 1 )
            to_room = generate_exit( ch->in_room, &pexit );

        if ( to_room == NULL )
            to_room = pexit->to_room;

        char_from_room( ch );
        char_to_room( ch, to_room );
        set_char_color( AT_RMNAME, ch );
        send_to_char( to_room->name, ch );
        send_to_char( "\n\r", ch );
        // show_list_to_char( to_room->first_content, ch, FALSE, FALSE );
        show_char_to_char( to_room->first_person, ch );

        if ( ( pexit = get_exit( to_room, dir ) ) == NULL )
        {
            sprintf( buf, "&rAn Alien leaps from %s and hits a wall.", rev_exit( dir ) );
            echo_to_room_ignore( ch, to_room, buf );
            break;
        }

        if ( range <= mrange )
        {
            sprintf( buf, "&rAn Alien leaps through the room. (%s to %s)", dir_name[flip_dir( dir )], dir_name[dir] );
            echo_to_room_ignore( ch, to_room, buf );
        }
    }

    if ( range > mrange )
    {
        sprintf( buf, "&rAn Alien leaps in from %s and lands neatly.", rev_exit( dir ) );
        echo_to_room_ignore( ch, ch->in_room, buf );
    }

    to_room = ch->in_room;

    if ( to_room->area )
        motion_ping( to_room->x, to_room->y, to_room->z, to_room->area, ch );

    /* Trigger room progs */
    if ( !IN_VENT( ch ) )
    {
        mprog_entry_trigger( ch );

        if ( char_died( ch ) )
            return;

        rprog_enter_trigger( ch );

        if ( char_died( ch ) )
            return;

        mprog_greet_trigger( ch );

        if ( char_died( ch ) )
            return;

        oprog_greet_trigger( ch );

        if ( char_died( ch ) )
            return;
    }

    return;
}

void echo_to_room_ignore( CHAR_DATA* ch, ROOM_INDEX_DATA* room, char* argument )
{
    CHAR_DATA* vic;

    if ( room == NULL )
        return;

    for ( vic = room->first_person; vic; vic = vic->next_in_room )
    {
        if ( vic == ch )
            continue;

        send_to_char( argument, vic );
        send_to_char( "\n\r",   vic );
    }
}

void do_breach( CHAR_DATA* ch, char* argument )
{
    send_to_char( "&zSyntax: &CBREACH (direction)\n\r", ch );
    send_to_char( " &WAllows Aliens to rip open passages.\n\r", ch );
    send_to_char( " &RINCOMPLETE - CONTACT GHOST!\n\r", ch );
    bug( "swskills.c: incomplete code for do_breach." );
    return;
}

void do_construct ( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA* pObjIndex;
    OBJ_DATA* obj;
    int ticks = 1;
    int support = 0;
    int cost = 10;
    int vnum = 0;
    strcpy( arg, argument );

    if ( !curr_arena )
        return;

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RSpectator mode. Please wait for respawn.\n\r", ch );
        return;
    }

    if ( ch->vent )
    {
        send_to_char( "&RYou can't construct while your inside a vent.\n\r", ch );
        return;
    }

    if ( ch->race != RACE_ALIEN && get_trust( ch ) <= 105 )
    {
        do_nothing( ch, "" );
        return;
    }

    switch ( ch->substate )
    {
        default:
            if ( !xIS_SET( ch->in_room->room_flags, ROOM_HIVED ) )
            {
                send_to_char( "&RYou can only construct inside hived rooms.\n\r", ch );
                return;
            }

            if ( !str_cmp( argument, "cubby" ) )
            {
                ticks = 8;
                cost = 20;
            }
            else if ( !str_cmp( argument, "acid trap" ) )
            {
                ticks = 6;
                cost = 10;
            }
            else if ( !str_cmp( argument, "resin pool" ) )
            {
                ticks = 12;
                cost = 50;
            }
            else if ( !str_cmp( argument, "regenerate" ) )
            {
                ticks = 10;
                cost = 40;
            }
            else if ( !str_cmp( argument, "sentry" ) )
            {
                ticks = 6;
                cost = 10;
            }
            else if ( !str_cmp( argument, "ambush" ) )
            {
                ticks = 10;
                cost = 40;
                support = 1;
            }
            else if ( !str_cmp( argument, "entangle" ) )
            {
                ticks = 6;
                cost = 12;
            }
            else if ( !str_cmp( argument, "tunnel" ) )
            {
                ticks = 10;
                cost = 40;
                support = 1;
            }
            else if ( !str_cmp( argument, "barrier" ) )
            {
                ticks = 6;
                cost = 15;
            }
            else if ( !str_cmp( argument, "acid burst" ) )
            {
                ticks = 8;
                cost = 20;
            }
            else if ( !str_cmp( argument, "shock web" ) )
            {
                ticks = 8;
                cost = 25;
            }
            else if ( !str_cmp( argument, "lure" ) )
            {
                ticks = 6;
                cost = 16;
            }
            else
            {
                // send_to_char( "&RSorry, unknown construction. Try HELP CONSTRUCT.\n\r", ch );
                do_help( ch, "construct" );
                return;
            }

            if ( ticks == 0 )
            {
                send_to_char( "&RSorry, that construction is not yet available.\n\r", ch );
                return;
            }

            if ( IS_IMMORTAL( ch ) )
                ticks = 1;

            if ( ch->resin < cost )
            {
                ch_printf( ch, "&w&RSorry, that construction requires more resin than you have yet.\n\r" );
                return;
            }

            if ( support > curr_arena->support[RACE_ALIEN] )
            {
                ch_printf( ch, "&w&RThis construction costs %d support points, you need more!\n\r", support );
                return;
            }

            send_to_char( "&GYou begin working on the construction.\n\r", ch );
            act( AT_PLAIN, "$n begins constructing something.", ch, NULL, argument, TO_ROOM );
            add_timer ( ch, TIMER_DO_FUN, ticks, do_construct, 1 );
            ch->dest_buf = str_dup( arg );
            return;

        case 1:
            if ( !ch->dest_buf )
                return;

            strcpy( arg, ch->dest_buf );
            DISPOSE( ch->dest_buf );
            break;

        case SUB_TIMER_DO_ABORT:
            DISPOSE( ch->dest_buf );
            ch->substate = SUB_NONE;
            send_to_char( "&RYou are interupted before you can finish your work.\n\r", ch );
            return;
    }

    ch->substate = SUB_NONE;

    if ( !str_cmp( arg, "cubby" ) )
    {
        cost = 20;
        vnum = 297;
    }
    else if ( !str_cmp( arg, "acid trap" ) )
    {
        cost = 10;
        vnum = 296;
    }
    else if ( !str_cmp( arg, "sentry" ) )
    {
        cost = 10;
        vnum = 0;
    }
    else if ( !str_cmp( arg, "ambush" ) )
    {
        cost = 40;
        vnum = 298;
        support = 1;
    }
    else if ( !str_cmp( arg, "entangle" ) )
    {
        cost = 12;
        vnum = 294;
    }
    else if ( !str_cmp( arg, "tunnel" ) )
    {
        cost = 40;
        vnum = 299;
        support = 1;
    }
    else if ( !str_cmp( arg, "barrier" ) )
    {
        cost = 15;
        vnum = 0;
    }
    else if ( !str_cmp( arg, "resin pool" ) )
    {
        cost = 50;
        vnum = 281;
    }
    else if ( !str_cmp( arg, "regenerate" ) )
    {
        cost = 40;
        vnum = 282;
    }
    else if ( !str_cmp( arg, "acid burst" ) )
    {
        cost = 20;
        vnum = 283;
    }
    else if ( !str_cmp( arg, "shock web" ) )
    {
        cost = 25;
        vnum = 284;
    }
    else if ( !str_cmp( arg, "lure" ) )
    {
        cost = 16;
        vnum = 295;
    }

    if ( curr_arena->support[RACE_ALIEN] < support )
    {
        ch_printf( ch, "&w&RSorry. Not enough support points available to complete it.\n\r" );
        return;
    }

    ch->mp = 0;
    ch->resin -= cost;
    curr_arena->support[RACE_ALIEN] -= support;

    if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
    {
        ch_printf( ch, "&w&RSorry, this construction item is incomplete.\n\r" );
        return;
    }

    obj = create_object( pObjIndex, 1 );
    obj_to_room( obj, ch->in_room );
    obj->level = 1;
    obj->parent = ch;
    send_to_char( "&GYou have completed your current construction.\n\r", ch );
    // ch_printf( ch, "&CYou gain %d experience for your success.\n\r", exp );
    // gain_exp( ch, exp );
    return;
}

void do_sc( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* to_room;
    CHAR_DATA* rch;
    EXIT_DATA* pexit;
    OBJ_DATA* cannon = NULL;
    int dam = 0;
    int dir = 0;
    int accuracy = 100;
    int range = 0;
    int mrange = 0;
    int drange = 0;
    int charge = 0;
    bool found = FALSE, cansee = FALSE;

    if ( !ch->pcdata )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( ch->pcdata->learned[gsn_shoulder_cannon] <= 0 )
    {
        do_nothing( ch, "" );
        return;
    }

    if ( ( cannon = get_eq_char( ch, WEAR_SHOULDER ) ) == NULL )
    {
        ch_printf( ch, "&RYou must purchase and equip a shoulder cannon first.\n\r" );
        return;
    }

    if ( cannon->value[4] >= cannon->value[5] )
    {
        ch_printf( ch, "&RYour shoulder cannon is overheating, wait for it to cool a bit.\n\r" );
        return;
    }

    if ( ch->pcdata->prepared[gsn_shoulder_cannon] < skill_table[gsn_shoulder_cannon]->reset )
    {
        ch_printf( ch, "&RYou must wait for the Shoulder Cannon skill to reset.\n\r" );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( is_spectator( ch ) )
    {
        send_to_char( "&RYou may not engage in combat while in spectator mode.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    {
        send_to_char( "&RYou can't even SEE while blind, let alone shoot!\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' || ( dir = get_door( arg1 ) ) == -1 )
    {
        send_to_char( "&RSyntax: CANNON <direction> <target>\n\r", ch );
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

    dam = cannon->value[1];
    mrange = UMAX( 1, cannon->value[0] );
    charge = ( int )( ( float )( ( float )( cannon->value[3] ) / ( float )( 100 ) ) * ( float )( 100 - ( ch->pcdata->learned[gsn_shoulder_cannon] * 10 ) ) );

    if ( ch->field < charge )
    {
        send_to_char( "&RNot enough field charge to fire the shoulder cannon.\n\r", ch );
        return;
    }

    WAIT_STATE( ch, UMAX( 4, cannon->value[2] ) );
    drange = get_dark_range( ch );
    pexit = get_exit( ch->in_room, dir );

    for ( range = 1; range <= mrange; range++ )
    {
        if ( !pexit )
            break;

        if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
            break;

        if ( !pexit->to_room )
            break;

        to_room = NULL;

        if ( pexit->distance > 1 )
            to_room = generate_exit( ch->in_room, &pexit );

        if ( to_room == NULL )
            to_room = pexit->to_room;

        found = FALSE;
        cansee = TRUE;

        if ( room_is_dark( ch, to_room ) )
        {
            if ( --drange < 0 )
                cansee = FALSE;
        }

        if ( ( rch = get_char_far_room( ch, to_room, arg2 ) ) != NULL )
            found = TRUE;

        // Target located
        if ( found )
        {
            if ( !is_spectator( rch ) && cansee && can_see( ch, rch ) && !IN_VENT( rch ) ) { }
            else
            {
                found = FALSE;
            }

            break;
        }

        if ( ( pexit = get_exit( to_room, dir ) ) == NULL )
            break;
    }

    if ( !found )
    {
        send_to_char( "&RYou fail to sight the target, it may have moved.\n\r", ch );
        return;
    }

    accuracy = stamina_penalty( ch, accuracy );
    ch->field -= charge;
    ch->pcdata->prepared[gsn_shoulder_cannon] = 0;
    // Firing Message
    ch_printf( ch, "&RYou fire your shoulder cannon to %s.\n\r", main_exit( dir ) );
    sprintf( buf, "$n fires his shoulder cannon %s.", dir_name[dir] );
    act( AT_BLOOD, buf, ch, NULL, NULL, TO_ROOM );
    send_sound( "You hear energy blasts", ch->in_room, 2, ch );
    cannon->value[4]++;
    drange = get_dark_range( ch );
    pexit = get_exit( ch->in_room, dir );

    for ( range = 1; range <= mrange; range++ )
    {
        if ( !pexit )
            break;

        if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
            break;

        if ( !pexit->to_room )
            break;

        accuracy -= 5;
        to_room = NULL;

        if ( pexit->distance > 1 )
            to_room = generate_exit( ch->in_room, &pexit );

        if ( to_room == NULL )
            to_room = pexit->to_room;

        found = FALSE;
        cansee = TRUE;

        if ( room_is_dark( ch, to_room ) )
        {
            if ( --drange < 0 )
                cansee = FALSE;
        }

        if ( ( rch = get_char_far_room( ch, to_room, arg2 ) ) != NULL )
            found = TRUE;

        // Target located
        if ( found )
        {
            if ( !is_spectator( rch ) && cansee && can_see( ch, rch ) && !IN_VENT( rch ) )
            {
                accuracy = 40 + ( ch->pcdata->learned[gsn_shoulder_cannon] * 10 );

                if ( ch->vision == rch->race )
                    accuracy += 60;

                if ( accuracy > 0 )
                    accuracy = char_acc_modify( rch, accuracy );

                if ( number_percent( ) < accuracy )
                {
                    ch_printf( rch, "&rShoulder cannon fire from %s hit you! &z(&W1 round&z)\n\r", rev_exit( dir ) );

                    if ( ( dam = cdamage( ch, rch, dam, TRUE ) ) > -1 )
                    {
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

                return;
            }

            break;
        }

        if ( ( pexit = get_exit( to_room, dir ) ) == NULL )
            break;
    }

    ch_printf( ch, "&rDamn. Doesn't look like you hit anything.\n\r" );
    return;
}

void do_throw( CHAR_DATA* ch, char* argument )
{
    OBJ_DATA*         obj;
    OBJ_DATA*         tmpobj;
    char              arg[MAX_INPUT_LENGTH];
    char              arg2[MAX_INPUT_LENGTH];
    char              arg3[MAX_INPUT_LENGTH];
    sh_int            dir;
    EXIT_DATA*        pexit;
    ROOM_INDEX_DATA* was_in_room;
    ROOM_INDEX_DATA* to_room;
    CHAR_DATA*        victim = NULL;
    char              buf[MAX_STRING_LENGTH];
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    was_in_room = ch->in_room;

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: THROW (direction) (target)\n\r", ch );
        send_to_char( " Throws your currently wielded weapon.\n\r", ch );
        return;
    }

    obj = get_eq_char( ch, WEAR_MISSILE_WIELD );

    if ( !obj || !nifty_is_name( arg, obj->name ) )
        obj = get_eq_char( ch, WEAR_HOLD );

    if ( !obj || !nifty_is_name( arg, obj->name ) )
        obj = get_eq_char( ch, WEAR_WIELD );

    if ( !obj || !nifty_is_name( arg, obj->name ) )
        obj = get_eq_char( ch, WEAR_DUAL_WIELD );

    if ( !obj || !nifty_is_name( arg, obj->name ) )
        if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
            obj = get_eq_char( ch, WEAR_HOLD );

    if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
        obj = get_eq_char( ch, WEAR_WIELD );

    if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
        obj = get_eq_char( ch, WEAR_DUAL_WIELD );

    if ( !obj || !nifty_is_name_prefix( arg, obj->name ) )
    {
        ch_printf( ch, "You don't seem to be holding or wielding %s.\n\r", arg );
        return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
    {
        act( AT_PLAIN, "You can't throw $p.", ch, obj, NULL, TO_CHAR );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        sprintf( buf, "$n throws %s at the floor.", obj->short_descr );
        act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
        ch_printf( ch, "You throw %s at the floor.\n\r", obj->short_descr );
        victim = NULL;
    }
    else  if ( ( dir = get_door( arg2 ) ) != -1 )
    {
        if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
        {
            send_to_char( "Are you expecting to throw it through a wall!?\n\r", ch );
            return;
        }

        if ( xIS_SET( pexit->exit_info, EX_CLOSED ) )
        {
            send_to_char( "Are you expecting to throw it  through a door!?\n\r", ch );
            return;
        }

        switch ( dir )
        {
            case 0:
            case 1:
                dir += 2;
                break;

            case 2:
            case 3:
                dir -= 2;
                break;

            case 4:
            case 7:
                dir += 1;
                break;

            case 5:
            case 8:
                dir -= 1;
                break;

            case 6:
                dir += 3;
                break;

            case 9:
                dir -= 3;
                break;
        }

        to_room = NULL;

        if ( pexit->distance > 1 )
            to_room = generate_exit( ch->in_room, &pexit );

        if ( to_room == NULL )
            to_room = pexit->to_room;

        char_from_room( ch );
        char_to_room( ch, to_room );
        victim = get_char_room( ch, arg3 );

        if ( victim )
        {
            if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
            {
                act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
                return;
            }

            if ( !IS_NPC( victim ) && xIS_SET( ch->act, PLR_NICE ) )
            {
                send_to_char( "You feel too nice to do that!\n\r", ch );
                return;
            }

            char_from_room( ch );
            char_to_room( ch, was_in_room );
            to_room = NULL;

            if ( pexit->distance > 1 )
                to_room = generate_exit( ch->in_room, &pexit );

            if ( to_room == NULL )
                to_room = pexit->to_room;

            char_from_room( ch );
            char_to_room( ch, to_room );
            sprintf( buf, "Someone throws %s at you from the %s.", obj->short_descr, dir_name[dir] );
            act( AT_ACTION, buf, victim, NULL, ch, TO_CHAR );
            act( AT_ACTION, "You throw %p at $N.", ch, obj, victim, TO_CHAR );
            sprintf( buf, "%s is thrown at $N from the %s.", obj->short_descr, dir_name[dir] );
            act( AT_ACTION, buf, ch, NULL, victim, TO_NOTVICT );
        }
        else
        {
            ch_printf( ch, "You throw %s %s.\n\r", obj->short_descr, dir_name[get_dir( arg2 )] );
            sprintf( buf, "%s is thrown from the %s.", obj->short_descr, dir_name[dir] );
            act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
        }
    }
    else if ( ( victim = get_char_room( ch, arg2 ) ) != NULL )
    {
        if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
        {
            act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
            return;
        }

        if ( !IS_NPC( victim ) && xIS_SET( ch->act, PLR_NICE ) )
        {
            send_to_char( "You feel too nice to do that!\n\r", ch );
            return;
        }
    }
    else
    {
        ch_printf( ch, "They don't seem to be here!\n\r" );
        return;
    }

    if ( obj == get_eq_char( ch, WEAR_WIELD )
            && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD ) ) != NULL )
        tmpobj->wear_loc = WEAR_WIELD;

    unequip_char( ch, obj );
    separate_obj( obj );
    obj_from_char( obj );
    obj = obj_to_room( obj, ch->in_room );

    if ( obj->item_type != ITEM_GRENADE )
        damage_obj ( obj, 5 );

    /*  NOT NEEDED UNLESS REFERING TO OBJECT AGAIN

        if( obj_extracted(obj) )
          return;
    */
    if ( ch->in_room != was_in_room )
    {
        char_from_room( ch );
        char_to_room( ch, was_in_room );
    }

    global_retcode = damage( ch, victim, number_range( obj->weight * 3, ( obj->weight * 3 + ch->perm_str ) ), TYPE_HIT );

    if ( victim && IS_NPC( victim ) && !char_died ( victim ) )
    {
        if ( xIS_SET( victim->act, ACT_SENTINEL ) )
        {
            victim->was_sentinel = victim->in_room;
            xREMOVE_BIT( victim->act, ACT_SENTINEL );
        }

        start_hating( victim, ch );
        start_hunting( victim, ch );
    }

    return;
}


