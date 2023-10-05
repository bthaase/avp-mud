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
               Online Building and Editing Module
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"


extern int  top_affect;
extern int  top_reset;
extern int  top_ed;
extern bool fBootDb;


char*   const   ex_flags [] =
{
    "isdoor", "closed", "locked", "secret", "swim", "pickproof", "fly", "climb",
    "dig", "r1", "nopassdoor", "hidden", "passage", "portal", "r2", "r3",
    "can_climb", "can_enter", "can_leave", "auto", "r4", "searchable",
    "bashed", "bashproof", "nomob", "window", "can_look", "keypad", "barred",
    "nospec", "blastopen", "blasted", "armored", "novent"
};

char*   const   r_flags [] =
{
    "dark", "reserved", "nomob", "indoors", "can_land", "can_fly", "can_drive",
    "shipyard", "nodropall", "logspeech", "teleport", "hotel", "nofloor",
    "nodrop", "viewport", "spacecraft", "prototype", "underground", "save_eq",
    "bacta", "fallcatch", "lit", "vented_a", "hived", "mapstart", "nohive",
    "safe", "deploy_p", "deploy_m", "deploy_a", "_nocoord", "vented_b",
    "vented_c", "vented_d", "deploy_from", "rescue", "cp", "static", "nohide"
};

char*   const   o_flags [] =
{
    "glow", "hum", "invis", "nodrop", "noremove", "inventory", "hidden",
    "covering", "deathrot", "burried", "prototype", "nodual", "marine",
    "predator", "alien", "respawn", "burstfire", "autofire", "semiauto",
    "singlefire", "useon", "useoff", "quiet", "moderate", "loud", "rangedonly",
    "indestructable", "nohold", "alieninvis", "notackle", "fullhit"
};

char*   const   mag_flags   [] =
{
    "returning", "backstabber", "bane", "loyal", "haste", "drain",
    "lightning_blade"
};

char*   const   w_flags [] =
{
    "take", "finger", "neck", "body", "head", "legs", "feet", "hands", "arms",
    "shield", "about", "waist", "wrist", "wield", "hold", "_dual_", "ears", "eyes",
    "missile", "over", "shoulder", "r4", "r5", "r6", "r7", "r8", "r9",
    "r10", "r11", "r12", "r13", "r14"
};

char*   const   area_flags  [] =
{
    "nopkill", "noreset", ""
};

char*   const   o_types [] =
{
    "none", "light", "weapon", "armor", "furniture", "trash",
    "container", "paper", "drinkcon", "key", "food", "corpse",
    "corpse_pc", "fountain", "bloodstain", "scraps", "fire",
    "ammo", "grenade", "radio", "medikit", "motion", "cloak",
    "binocular", "gps", "cover", "bandage", "c4", "laptop",
    "toolkit", "steril", "medicomp", "spawner", "landmine",
    "attachment", "medstation", "motionb", "ammobox", "toolchest",
    "mapconsole", "trap", "mspawner", "deployer", "remote", "sift",
    "flare", "regenerator", "sentry", "scannon", "nvgoggle"
};

char*   const   a_types [] =
{
    "none", "strength", "stamina", "recovery", "intelligence", "bravery",
    "perception", "null", "hit", "move",
    "affected", "fire", "energy", "impact", "pierce", "acid"
};

char*   const   a_flags [] =
{
    "blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
    "detect_hidden", "weaken", "sanctuary", "faerie_fire", "infrared", "curse",
    "_flaming", "poison", "protect", "paralysis", "sneak", "hide", "sleep",
    "charm", "flying", "pass_door", "floating", "truesight", "detect_traps",
    "scrying", "fireshield", "shockshield", "r1", "iceshield", "possess",
    "aqua_breath", "napalm", "cloak"
};

char*   const   act_flags [] =
{
    "npc", "sentinel", "scavenger", "aggressive", "stayarea",
    "droid", "train", "practice", "immortal", "deadly", "polyself",
    "meta_aggr", "guardian", "running", "nowander", "mountable", "mounted", "scholar",
    "secretive", "polymorphed", "mobinvis", "noassist", "nokill", "droid", "nocorpse",
    "surgeon", "prototype", "enlist", "hostage"
};

char*   const   pc_flags [] =
{
    "r1", "deadly", "unauthed", "norecall", "nointro", "gag", "retired", "guest",
    "nosummon", "pageron", "notitled", "room", "helpstart", "dnd", "itrace", "nowho", "r10", "r11", "r12", "r13",
    "showreset", "showammo", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24",
    "r25"
};

char*   const   plr_flags [] =
{
    "npc", "boughtdroid", "shovedrag", "autoexits", "autoloot", "autosac", "blank",
    "outcast", "brief", "combine", "prompt", "telnet_ga", "holylight",
    "wizinvis", "roomvnum", "silence", "noemote", "attacker", "notell", "log",
    "deny", "freeze", "killer", "pf_3", "litterbug", "ansi", "rip", "nice",
    "flee", "autocred", "automap", "afk", "ncouncil", "censor", "whois",
    "compact", "buildwalk"
};

char*   const   trap_flags [] =
{
    "room", "obj", "enter", "leave", "open", "close", "get", "put", "pick",
    "unlock", "north", "south", "east", "r1", "west", "up", "down", "examine",
    "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13",
    "r14", "r15"
};

char*   const   wear_locs [] =
{
    "light", "finger1", "finger2", "neck1", "neck2", "body", "head", "legs",
    "feet", "hands", "arms", "shield", "about", "waist", "wrist1", "wrist2",
    "wield", "hold", "dual_wield", "ears", "eyes", "missile_wield", "over",
    "back", "shoulder"
};

char*   const   ris_flags [] = { "fire", "energy", "impact", "pierce", "acid" };

char*   const   trig_flags [] =
{
    "up", "unlock", "lock", "d_north", "d_south", "d_east", "d_west", "d_up",
    "d_down", "door", "container", "open", "close", "passage", "oload", "mload",
    "teleport", "teleportall", "teleportplus", "death", "cast", "fakeblade",
    "rand4", "rand6", "trapdoor", "anotherroom", "usedial", "absolutevnum",
    "showroomdesc", "autoreturn", "r2", "r3"
};

/*
    Note: I put them all in one big set of flags since almost all of these
    can be shared between mobs, objs and rooms for the exception of
    bribe and hitprcnt, which will probably only be used on mobs.
    ie: drop -- for an object, it would be triggered when that object is
    dropped; -- for a room, it would be triggered when anything is dropped
            -- for a mob, it would be triggered when anything is dropped

    Something to consider: some of these triggers can be grouped together,
    and differentiated by different arguments... for example:
    hour and time, rand and randiw, speech and speechiw

*/
char*   const   mprog_flags [] =
{
    "act", "speech", "rand", "fight", "death", "hitprcnt", "entry", "greet",
    "allgreet", "give", "bribe", "hour", "time", "wear", "remove", "sac",
    "look", "exa", "zap", "get", "drop", "damage", "repair", "randiw",
    "speechiw", "useon", "useoff", "sleep", "rest", "leave", "script", "use",
    "pulse"
};


char* old_flag_string( int bitvector, char* const flagarray[] )
{
    static char buf[MAX_STRING_LENGTH];
    int x;
    buf[0] = '\0';

    for ( x = 0; x < 32 ; x++ )
        if ( IS_SET( bitvector, 1 << x ) )
        {
            strcat( buf, flagarray[x] );

            /* don't catenate a blank if the last char is blank  --Gorog */
            if ( buf[0] != '\0' && ' ' != buf[strlen( buf ) - 1] )
                strcat( buf, " " );
        }

    if ( ( x = strlen( buf ) ) > 0 )
        buf[--x] = '\0';

    return buf;
}

char* flag_string( EXT_BV* bitvector, char* const flagarray[], int range )
{
    static char buf[MAX_STRING_LENGTH];
    int x;
    buf[0] = '\0';

    for ( x = 0; x < range; x++ )
    {
        if ( xIS_SET( *bitvector, x ) )
        {
            strcat( buf, flagarray[x] );

            /* don't catenate a blank if the last char is blank  --Gorog */
            if ( buf[0] != '\0' && ' ' != buf[strlen( buf ) - 1] )
                strcat( buf, " " );
        }
    }

    if ( ( x = strlen( buf ) ) > 0 )
        buf[--x] = '\0';

    return buf;
}

void hflag_toggle( ROOM_INDEX_DATA* room )
{
    int x;

    if ( xIS_EMPTY( room->hroom_flags ) )
        return;

    for ( x = 0; x < MAX_ROOM_FLAGS; x++ )
    {
        if ( xIS_SET( room->hroom_flags, x ) )
        {
            xTOGGLE_BIT( room->room_flags, x );
        }
    }

    return;
}


bool can_rmodify( CHAR_DATA* ch, ROOM_INDEX_DATA* room )
{
    int vnum = room->vnum;
    AREA_DATA* pArea;

    if ( IS_NPC( ch ) )
        return FALSE;

    if ( get_trust( ch ) >= sysdata.level_modify_proto )
        return TRUE;

    if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
    {
        send_to_char( "You must have an assigned area to modify this room.\n\r", ch );
        return FALSE;
    }

    if ( vnum >= pArea->low_r_vnum
            &&   vnum <= pArea->hi_r_vnum )
        return TRUE;

    send_to_char( "That room is not in your allocated range.\n\r", ch );
    return FALSE;
}

bool can_omodify( CHAR_DATA* ch, OBJ_DATA* obj )
{
    int vnum = obj->pIndexData->vnum;
    AREA_DATA* pArea;

    if ( IS_NPC( ch ) )
        return FALSE;

    if ( get_trust( ch ) >= sysdata.level_modify_proto )
        return TRUE;

    if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
    {
        send_to_char( "You must have an assigned area to modify this object.\n\r", ch );
        return FALSE;
    }

    if ( vnum >= pArea->low_o_vnum
            &&   vnum <= pArea->hi_o_vnum )
        return TRUE;

    send_to_char( "That object is not in your allocated range.\n\r", ch );
    return FALSE;
}

bool can_oedit( CHAR_DATA* ch, OBJ_INDEX_DATA* obj )
{
    int vnum = obj->vnum;
    AREA_DATA* pArea;

    if ( IS_NPC( ch ) )
        return FALSE;

    if ( get_trust( ch ) >= 105 )
        return TRUE;

    if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
    {
        send_to_char( "You must have an assigned area to modify this object.\n\r", ch );
        return FALSE;
    }

    if ( vnum >= pArea->low_o_vnum
            &&   vnum <= pArea->hi_o_vnum )
        return TRUE;

    send_to_char( "That object is not in your allocated range.\n\r", ch );
    return FALSE;
}


bool can_mmodify( CHAR_DATA* ch, CHAR_DATA* mob )
{
    int vnum;
    AREA_DATA* pArea;

    if ( mob == ch )
        return TRUE;

    if ( !IS_NPC( mob ) )
    {
        if ( get_trust( ch ) >= sysdata.level_modify_proto
                && get_trust( ch ) > get_trust( mob ) )
            return TRUE;
        else
            send_to_char( "You can't do that.\n\r", ch );

        return FALSE;
    }

    vnum = mob->pIndexData->vnum;

    if ( IS_NPC( ch ) )
        return FALSE;

    if ( get_trust( ch ) >= sysdata.level_modify_proto )
        return TRUE;

    if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
    {
        send_to_char( "You must have an assigned area to modify this mobile.\n\r", ch );
        return FALSE;
    }

    if ( vnum >= pArea->low_m_vnum
            &&   vnum <= pArea->hi_m_vnum )
        return TRUE;

    send_to_char( "That mobile is not in your allocated range.\n\r", ch );
    return FALSE;
}

bool can_medit( CHAR_DATA* ch, MOB_INDEX_DATA* mob )
{
    int vnum = mob->vnum;
    AREA_DATA* pArea;

    if ( IS_NPC( ch ) )
        return FALSE;

    if ( get_trust( ch ) >= 105 )
        return TRUE;

    if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
    {
        send_to_char( "You must have an assigned area to modify this mobile.\n\r", ch );
        return FALSE;
    }

    if ( vnum >= pArea->low_m_vnum
            &&   vnum <= pArea->hi_m_vnum )
        return TRUE;

    send_to_char( "That mobile is not in your allocated range.\n\r", ch );
    return FALSE;
}

int get_otype( char* type )
{
    int x;

    for ( x = 0; x < ( sizeof( o_types ) / sizeof( o_types[0] ) ); x++ )
        if ( !str_cmp( type, o_types[x] ) )
            return x;

    return -1;
}

int get_aflag( char* flag )
{
    int x;

    for ( x = 0; x < MAX_AFFECTED_BY; x++ )
        if ( !str_cmp( flag, a_flags[x] ) )
            return x;

    return -1;
}

int get_trapflag( char* flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, trap_flags[x] ) )
            return x;

    return -1;
}

int get_atype( char* type )
{
    int x;

    for ( x = 0; x < MAX_APPLY_TYPE; x++ )
        if ( !str_cmp( type, a_types[x] ) )
            return x;

    return -1;
}

int get_npc_race( char* type )
{
    int x;

    for ( x = 0; x < MAX_NPC_RACE; x++ )
        if ( !str_cmp( type, npc_race[x] ) )
            return x;

    return -1;
}

int get_wearloc( char* type )
{
    int x;

    for ( x = 0; x < MAX_WEAR; x++ )
        if ( !str_cmp( type, wear_locs[x] ) )
            return x;

    return -1;
}

int get_exflag( char* flag )
{
    int x;

    for ( x = 0; x < MAX_EXFLAG; x++ )
        if ( !str_cmp( flag, ex_flags[x] ) )
            return x;

    return -1;
}

int get_rflag( char* flag )
{
    int x;

    for ( x = 0; x < MAX_ROOM_FLAGS; x++ )
        if ( !str_cmp( flag, r_flags[x] ) )
            return x;

    return -1;
}

int get_mpflag( char* flag )
{
    int x;

    for ( x = 0; x < MAX_PROG; x++ )
        if ( !str_cmp( flag, mprog_flags[x] ) )
            return ( x + 1 );

    return -1;
}

int get_oflag( char* flag )
{
    int x;

    for ( x = 0; x < MAX_ITEM_FLAG; x++ )
        if ( !str_cmp( flag, o_flags[x] ) )
            return x;

    return -1;
}

int get_areaflag( char* flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, area_flags[x] ) )
            return x;

    return -1;
}

int get_wflag( char* flag )
{
    int x;

    for ( x = 0; x < MAX_WEAR_FLAGS; x++ )
        if ( !str_cmp( flag, w_flags[x] ) )
            return x;

    return -1;
}

int get_actflag( char* flag )
{
    int x;

    for ( x = 0; x < MAX_ACT_FLAGS; x++ )
        if ( !str_cmp( flag, act_flags[x] ) )
            return x;

    return -1;
}

int get_pcflag( char* flag )
{
    int x;

    for ( x = 0; x < MAX_PC_FLAGS; x++ )
        if ( !str_cmp( flag, pc_flags[x] ) )
            return x;

    return -1;
}
int get_plrflag( char* flag )
{
    int x;

    for ( x = 0; x < MAX_PLR_FLAGS; x++ )
        if ( !str_cmp( flag, plr_flags[x] ) )
            return x;

    return -1;
}

int get_risflag( char* flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, ris_flags[x] ) )
            return x;

    return -1;
}

int get_trigflag( char* flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, trig_flags[x] ) )
            return x;

    return -1;
}

int get_langflag( char* flag )
{
    int x;

    for ( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
        if ( !str_cmp( flag, lang_names[x] ) )
            return lang_array[x];

    return LANG_UNKNOWN;
}

/*
    Remove carriage returns from a line
*/
char* strip_cr( char* str )
{
    static char newstr[MAX_STRING_LENGTH];
    int i, j;

    for ( i = j = 0; str[i] != '\0'; i++ )
        if ( str[i] != '\r' )
        {
            newstr[j++] = str[i];
        }

    newstr[j] = '\0';
    return newstr;
}

/*
    Changes carriage returns into <br> for HTML
    Changes spaces into '&nbsp;'
*/
char* conv_tag( char* str )
{
    static char newstr[MAX_STRING_LENGTH];
    int i, j;
    int itag = 0;

    for ( i = j = 0; str[i] != '\0'; i++ )
    {
        if ( str[i] == '\n' )
        {
            newstr[j++] = '<';
            newstr[j++] = 'b';
            newstr[j++] = 'r';
            newstr[j++] = '>';
        }
        else if ( str[i] == '<' )
        {
            itag++;
            newstr[j++] = str[i];
        }
        else if ( str[i] == '>' )
        {
            itag--;
            newstr[j++] = str[i];
        }
        else if ( str[i] == ' ' )
        {
            if ( itag <= 0 )
            {
                newstr[j++] = '&';
                newstr[j++] = 'n';
                newstr[j++] = 'b';
                newstr[j++] = 's';
                newstr[j++] = 'p';
                newstr[j++] = ';';
            }
            else
            {
                newstr[j++] = str[i];
            }
        }
        else if ( str[i] != '\r' )
        {
            newstr[j++] = str[i];
        }
    }

    newstr[j] = '\0';
    return newstr;
}

/*
    Changes spaces into %20 for URL Addresses
*/
char* convert_sp( char* str )
{
    static char newstr[MAX_STRING_LENGTH];
    int i, j;

    for ( i = j = 0; str[i] != '\0'; i++ )
        if ( str[i] != ' ' )
        {
            newstr[j++] = str[i];
        }
        else
        {
            newstr[j++] = '%';
            newstr[j++] = '2';
            newstr[j++] = '0';
        }

    newstr[j] = '\0';
    return newstr;
}

/*
    Removes the tildes from a line, except if it's the last character.
*/
void smush_tilde( char* str )
{
    int len;
    char last;
    char* strptr;
    strptr = str;
    len  = strlen( str );

    if ( len )
        last = strptr[len - 1];
    else
        last = '\0';

    for ( ; *str != '\0'; str++ )
    {
        if ( *str == '~' )
            *str = '-';
    }

    if ( len )
        strptr[len - 1] = last;

    return;
}


void start_editing( CHAR_DATA* ch, char* data )
{
    EDITOR_DATA* edit;
    sh_int lines, size, lpos;
    char c;

    if ( !ch->desc )
    {
        bug( "Fatal: start_editing: no desc", 0 );
        return;
    }

    if ( ch->substate == SUB_RESTRICTED )
        bug( "NOT GOOD: start_editing: ch->substate == SUB_RESTRICTED", 0 );

    set_char_color( AT_GREEN, ch );
    send_to_char( "Begin entering your text (/? =help /s =save /c =clear /l =list /f =format)\n\r", ch );
    send_to_char( "--------------------------------------------------------------------------\n\r> ", ch );

    if ( ch->editor )
        stop_editing( ch );

    CREATE( edit, EDITOR_DATA, 1 );
    edit->numlines = 0;
    edit->on_line  = 0;
    edit->size     = 0;
    size = 0;
    lpos = 0;
    lines = 0;

    if ( !data )
        bug( "editor: data is NULL!\n\r", 0 );
    else
        for ( ;; )
        {
            c = data[size++];

            if ( c == '\0' )
            {
                edit->line[lines][lpos] = '\0';
                break;
            }
            else if ( c == '\r' );
            else if ( c == '\n' || lpos > 78 )
            {
                edit->line[lines][lpos] = '\0';
                lines++;
                lpos = 0;
            }
            else
                edit->line[lines][lpos++] = c;

            if ( lines >= 49 || size > 4096 )
            {
                edit->line[lines][lpos] = '\0';
                break;
            }
        }

    edit->numlines = lines;
    edit->size = size;
    edit->on_line = lines;
    ch->editor = edit;
    ch->desc->connected = CON_EDITING;
}

char* copy_buffer( CHAR_DATA* ch )
{
    char buf[MAX_STRING_LENGTH];
    char tmp[100];
    sh_int x, len;

    if ( !ch )
    {
        bug( "copy_buffer: null ch", 0 );
        return STRALLOC( "" );
    }

    if ( !ch->editor )
    {
        bug( "copy_buffer: null editor", 0 );
        return STRALLOC( "" );
    }

    buf[0] = '\0';

    for ( x = 0; x < ch->editor->numlines; x++ )
    {
        strcpy( tmp, ch->editor->line[x] );
        len = strlen( tmp );

        if ( tmp && tmp[len - 1] == '~' )
            tmp[len - 1] = '\0';
        else
            strcat( tmp, "\n\r" );

        smash_tilde( tmp );
        strcat( buf, tmp );
    }

    return STRALLOC( buf );
}

void stop_editing( CHAR_DATA* ch )
{
    set_char_color( AT_PLAIN, ch );
    DISPOSE( ch->editor );
    ch->editor = NULL;
    send_to_char( "Done.\n\r", ch );
    ch->dest_buf  = NULL;
    ch->spare_ptr = NULL;
    ch->substate  = SUB_NONE;

    if ( !ch->desc )
    {
        bug( "Fatal: stop_editing: no desc", 0 );
        return;
    }

    ch->desc->connected = CON_PLAYING;
}

void do_goto( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* location;
    CHAR_DATA* fch;
    CHAR_DATA* fch_next;
    ROOM_INDEX_DATA* in_room;
    AREA_DATA* pArea;
    bool silent = FALSE;
    int vnum;
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "-silent" ) )
        silent = TRUE;

    if ( !is_number( arg ) && ( fch = get_char_world_full( ch, arg ) ) )
    {
        if ( !IS_NPC( fch ) && get_trust( ch ) < get_trust( fch ) && xIS_SET( fch->pcdata->flags, PCFLAG_DND ) )
        {
            if ( !silent )
                pager_printf( ch,
                              "Sorry. %s does not wish to be disturbed.\n\r", fch->name );

            pager_printf( fch, "Your DND flag just foiled %s's goto command.\n\r", ch->name );
            return;
        }
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        vnum = atoi( arg );

        if ( vnum < 0 || get_room_index( vnum ) )
        {
            if ( !silent )
                send_to_char( "You cannot find that...\n\r", ch );

            return;
        }

        if ( vnum < 1 || IS_NPC( ch ) || !ch->pcdata->area )
        {
            if ( !silent )
                send_to_char( "No such location.\n\r", ch );

            return;
        }

        if ( get_trust( ch ) < sysdata.level_modify_proto )
        {
            if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
            {
                if ( !silent )
                    send_to_char( "You must have an assigned area to create rooms.\n\r", ch );

                return;
            }

            if ( vnum < pArea->low_r_vnum || vnum > pArea->hi_r_vnum )
            {
                if ( !silent )
                    send_to_char( "That room is not within your assigned range.\n\r", ch );

                return;
            }
        }

        location = make_room( vnum );

        if ( !location )
        {
            bug( "Goto: make_room failed", 0 );
            return;
        }

        location->area = ch->pcdata->area;

        if ( !silent )
        {
            set_char_color( AT_WHITE, ch );
            send_to_char( "Waving your hand, you form order from swirling chaos,\n\rand step into a new reality...\n\r", ch );
        }
    }

    if ( ( location->vnum == 124 || location->vnum == 115 ) && get_trust( ch ) < 105 )
    {
        send_to_char( "You aren't allowed in Ghost's or Ravens' Private Office unless invited.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < LEVEL_IMMORTAL )
    {
        vnum = atoi( arg );

        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            if ( !silent )
                send_to_char( "You must have an assigned area to goto.\n\r", ch );

            return;
        }

        if ( vnum < pArea->low_r_vnum || vnum > pArea->hi_r_vnum )
        {
            if ( !silent )
                send_to_char( "That room is not within your assigned range.\n\r", ch );

            return;
        }

        if ( ( ch->in_room->vnum < pArea->low_r_vnum
                ||   ch->in_room->vnum > pArea->hi_r_vnum ) && !xIS_SET( ch->in_room->room_flags, ROOM_HOTEL ) )
        {
            if ( !silent )
                send_to_char( "Builders can only use goto from a hotel or in their zone.\n\r", ch );

            return;
        }
    }

    in_room = ch->in_room;

    if ( !xIS_SET( ch->act, PLR_WIZINVIS ) && !silent )
    {
        if ( ch->pcdata && ch->pcdata->bamfout[0] != '\0' )
            act( AT_IMMORT, "$T", ch, NULL, ch->pcdata->bamfout,  TO_ROOM );
        else
            act( AT_IMMORT, "$n $T", ch, NULL, "leaves in a swirl of the force.",  TO_ROOM );
    }

    ch->regoto = ch->in_room->vnum;
    char_from_room( ch );

    if ( ch->mount )
    {
        char_from_room( ch->mount );
        char_to_room( ch->mount, location );
    }

    if ( ch->carrying )
    {
        char_from_room( ch->carrying );
        char_to_room( ch->carrying, location );
    }

    char_to_room( ch, location );

    if ( !xIS_SET( ch->act, PLR_WIZINVIS ) && !silent )
    {
        if ( ch->pcdata && ch->pcdata->bamfin[0] != '\0' )
            act( AT_IMMORT, "$T", ch, NULL, ch->pcdata->bamfin,  TO_ROOM );
        else
            act( AT_IMMORT, "$n $T", ch, NULL, "enters in a swirl of the force.",  TO_ROOM );
    }

    if ( !silent )
        do_look( ch, "auto" );

    if ( ch->in_room == in_room )
        return;

    for ( fch = in_room->first_person; fch; fch = fch_next )
    {
        fch_next = fch->next_in_room;

        if ( fch->master == ch && IS_IMMORTAL( fch ) )
        {
            if ( !silent )
                act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );

            do_goto( fch, argument );
        }
        /* Experimental change by Gorog so imm's personal mobs follow them */
        else if ( IS_NPC( fch ) && fch->master == ch )
        {
            char_from_room ( fch );
            char_to_room( fch, location );
        }
    }

    return;
}

void do_rgoto( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int vnum;
    int vnum1;
    int vnum2;
    AREA_DATA* pArea;
    char buf[MAX_STRING_LENGTH];
    one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2 == '\0' || !is_number( arg1 ) || !is_number( arg2 ) || atoi( arg1 ) >= atoi( arg2 ) )
    {
        send_to_char( "Syntax: rgoto <firstvnum> <lastvnum>?\n\r", ch );
        return;
    }

    vnum1 = atoi( arg1 );
    vnum2 = atoi( arg2 );

    if ( get_trust( ch ) < sysdata.level_modify_proto )
    {
        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            send_to_char( "You must have an assigned area to create rooms.\n\r", ch );
            return;
        }

        if ( vnum1 < pArea->low_r_vnum
                ||   vnum2 > pArea->hi_r_vnum )
        {
            send_to_char( "That room is not within your assigned range.\n\r", ch );
            return;
        }
    }

    if ( get_trust( ch ) < LEVEL_IMMORTAL )
    {
        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            send_to_char( "You must have an assigned area to goto.\n\r", ch );
            return;
        }

        if ( vnum1 < pArea->low_r_vnum
                ||  vnum2 > pArea->hi_r_vnum )
        {
            send_to_char( "That room is not within your assigned range.\n\r", ch );
            return;
        }

        if ( ( ch->in_room->vnum < pArea->low_r_vnum
                ||   ch->in_room->vnum > pArea->hi_r_vnum ) && !xIS_SET( ch->in_room->room_flags, ROOM_HOTEL ) )
        {
            send_to_char( "Builders can only use goto from a hotel or in their zone.\n\r", ch );
            return;
        }
    }

    for ( vnum = vnum1; vnum > vnum2; vnum++ )
    {
        sprintf( buf, "%d", vnum );
        do_goto ( ch, buf );
    }

    return;
}

void do_rcreate( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int vnum;
    int vnum1;
    int vnum2;
    char buf[MAX_STRING_LENGTH];
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2 == '\0' || !is_number( arg1 ) || !is_number( arg2 ) || atoi( arg1 ) >= atoi( arg2 ) )
    {
        send_to_char( "&RSyntax: RCREATE (first room) (last room)\n\r", ch );
        return;
    }

    vnum1 = atoi( arg1 );
    vnum2 = atoi( arg2 );

    for ( vnum = vnum1; vnum <= vnum2; vnum++ )
    {
        sprintf( buf, "%d -silent", vnum );
        do_goto( ch, buf );
    }

    ch_printf( ch, "&zCompleted RCREATE from &C%d &zto &C%d&z. Have a nice day.\n\r", vnum1, vnum2 );
    return;
}

void do_mset( CHAR_DATA* ch, char* argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    char outbuf[MAX_STRING_LENGTH];
    int  num, size, plus;
    char char1, char2;
    CHAR_DATA* victim;
    int value, i;
    int minattr, maxattr;
    bool lockvictim;
    char* origarg = argument;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't mset\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }

    switch ( ch->substate )
    {
        default:
            break;

        case SUB_MOB_DESC:
            if ( !ch->dest_buf )
            {
                send_to_char( "Fatal error: report to Thoric.\n\r", ch );
                bug( "do_mset: sub_mob_desc: NULL ch->dest_buf", 0 );
                ch->substate = SUB_NONE;
                return;
            }

            victim = ch->dest_buf;

            if ( char_died( victim ) )
            {
                send_to_char( "Your victim died!\n\r", ch );
                stop_editing( ch );
                return;
            }

            STRFREE( victim->description );
            victim->description = copy_buffer( ch );

            if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            {
                STRFREE( victim->pIndexData->description );
                victim->pIndexData->description = QUICKLINK( victim->description );
            }

            stop_editing( ch );
            ch->substate = ch->tempnum;
            return;
    }

    victim = NULL;
    lockvictim = FALSE;
    smash_tilde( argument );

    if ( ch->substate == SUB_REPEATCMD )
    {
        victim = ch->dest_buf;

        if ( char_died( victim ) )
        {
            send_to_char( "Your victim died!\n\r", ch );
            victim = NULL;
            argument = "done";
        }

        if ( argument[0] == '\0' || !str_cmp( argument, " " )
                ||   !str_cmp( argument, "stat" ) )
        {
            if ( victim )
                do_mstat( ch, victim->name );
            else
                send_to_char( "No victim selected.  Type '?' for help.\n\r", ch );

            return;
        }

        if ( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
        {
            send_to_char( "Mset mode off.\n\r", ch );
            ch->substate = SUB_NONE;
            ch->dest_buf = NULL;

            if ( ch->pcdata && ch->pcdata->subprompt )
            {
                STRFREE( ch->pcdata->subprompt );
                ch->pcdata->subprompt = NULL;
            }

            return;
        }
    }

    if ( victim )
    {
        lockvictim = TRUE;
        strcpy( arg1, victim->name );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );
    }
    else
    {
        lockvictim = FALSE;
        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );
    }

    if ( !str_cmp( arg1, "on" ) )
    {
        send_to_char( "Syntax: mset <victim|vnum> on.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' || ( arg2[0] == '\0' && ch->substate != SUB_REPEATCMD )
            ||   !str_cmp( arg1, "?" ) )
    {
        if ( ch->substate == SUB_REPEATCMD )
        {
            if ( victim )
                send_to_char( "Syntax: <field>  <value>\n\r",       ch );
            else
                send_to_char( "Syntax: <victim> <field>  <value>\n\r",  ch );
        }
        else
            send_to_char( "Syntax: mset <victim> <field>  <value>\n\r", ch );

        send_to_char( "\n\r",                       ch );
        send_to_char( "Field being one of:\n\r",            ch );
        send_to_char( "  str int wis dex con cha lck frc sex\n\r",  ch );
        send_to_char( "  hp force move race\n\r", ch );
        send_to_char( "  armor affected level\n\r",     ch );
        send_to_char( "  thirst drunk full blood flags\n\r",            ch );
        send_to_char( "  pos defpos currexp maxexp\n\r",          ch );
        send_to_char( "  speaking speaks (see LANGUAGES)\n\r",      ch );
        send_to_char( "  name short long description title spec spec2\n\r", ch );
        send_to_char( "  spec3 spec4 division\n\r",                  ch );
        send_to_char( "To toggle area flag: aloaded\n\r", ch );
        return;
    }

    if ( !victim && get_trust( ch ) <= LEVEL_IMMORTAL )
    {
        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }
    }
    else if ( !victim )
    {
        if ( ( victim = get_char_world_full( ch, arg1 ) ) == NULL )
        {
            send_to_char( "No one like that in all the realms.\n\r", ch );
            return;
        }
    }

    if ( get_trust( ch ) < 105 && !IS_NPC( victim ) )
    {
        send_to_char( "You can't do that!\n\r", ch );
        ch->dest_buf = NULL;
        return;
    }

    if ( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
    {
        send_to_char( "You can't do that!\n\r", ch );
        ch->dest_buf = NULL;
        return;
    }

    if ( lockvictim )
        ch->dest_buf = victim;

    if ( IS_NPC( victim ) )
    {
        minattr = 1;
        maxattr = 25;
    }
    else
    {
        minattr = 3;
        maxattr = 18;

        if ( get_trust( ch ) >= 105 )
            maxattr = 30;
    }

    if ( !str_cmp( arg2, "on" ) )
    {
        CHECK_SUBRESTRICTED( ch );
        ch_printf( ch, "Mset mode on. (Editing %s).\n\r",
                   victim->name );
        ch->substate = SUB_REPEATCMD;
        ch->dest_buf = victim;

        if ( ch->pcdata )
        {
            if ( ch->pcdata->subprompt )
                STRFREE( ch->pcdata->subprompt );

            if ( IS_NPC( victim ) )
                sprintf( buf, "<&CMset &W#%d&w> %%i", victim->pIndexData->vnum );
            else
                sprintf( buf, "<&CMset &W%s&w> %%i", victim->name );

            ch->pcdata->subprompt = STRALLOC( buf );
        }

        return;
    }

    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    if ( atoi( arg3 ) < -1 && value == -1 )
        value = atoi( arg3 );

    if ( !str_cmp( arg2, "str" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Strength range is %d to %d.\n\r", minattr, maxattr );
            return;
        }

        victim->perm_str = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_str = value;

        return;
    }

    if ( !str_cmp( arg2, "sta" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Stamina range is %d to %d.\n\r", minattr, maxattr );
            return;
        }

        victim->perm_sta = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_sta = value;

        return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Intelligence range is %d to %d.\n\r", minattr, maxattr );
            return;
        }

        victim->perm_int = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_int = value;

        return;
    }

    if ( !str_cmp( arg2, "rec" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Recovery range is %d to %d.\n\r", minattr, maxattr );
            return;
        }

        victim->perm_rec = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_rec = value;

        return;
    }

    if ( !str_cmp( arg2, "bra" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Bravery range is %d to %d.\n\r", minattr, maxattr );
            return;
        }

        victim->perm_bra = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_bra = value;

        return;
    }

    if ( !str_cmp( arg2, "per" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Perception range is %d to %d.\n\r", minattr, maxattr );
            return;
        }

        victim->perm_per = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_per = value;

        return;
    }

    if ( !str_cmp( arg2, "modreset" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        do_remove( victim, "all" );
        victim->mod_str                 = 0;
        victim->mod_sta                 = 0;
        victim->mod_rec                 = 0;
        victim->mod_int                 = 0;
        victim->mod_bra                 = 0;
        victim->mod_per                 = 0;
        send_to_char( "All Stat Modifiers reset!\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "sex" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < 0 || value > 2 )
        {
            send_to_char( "Sex range is 0 to 2.\n\r", ch );
            return;
        }

        victim->sex = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->sex = value;

        return;
    }

    if ( !str_cmp( arg2, "race" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        value = get_npc_race( arg3 );

        if ( value < 0 )
            value = atoi( arg3 );

        if ( !IS_NPC( victim ) && ( value < 0 || value >= MAX_RACE ) )
        {
            ch_printf( ch, "Race range is 0 to %d.\n", MAX_RACE - 1 );
            return;
        }

        if ( IS_NPC( victim ) && ( value < 0 || value >= MAX_NPC_RACE ) )
        {
            ch_printf( ch, "Race range is 0 to %d.\n", MAX_NPC_RACE - 1 );
            return;
        }

        victim->race = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->race = value;

        return;
    }

    if ( !str_cmp( arg2, "currexp" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < 0 || value > 99999 )
        {
            send_to_char( "Current experience range is 0 to 99999.\n\r", ch );
            return;
        }

        victim->currexp = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->currexp = value;

        return;
    }

    if ( !str_cmp( arg2, "maxexp" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < 0 || value > 999999 )
        {
            send_to_char( "Maximum experience range is 0 to 999999.\n\r", ch );
            return;
        }

        victim->maxexp = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->maxexp = value;

        return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !IS_NPC( victim ) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > LEVEL_AVATAR + 5 )
        {
            ch_printf( ch, "Level range is 0 to %d.\n\r", LEVEL_AVATAR + 5 );
            return;
        }

        victim->top_level = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
        {
            victim->pIndexData->level = value;
        }

        return;
    }

    if ( !str_cmp( arg2, "hp" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < 1 || value > 32700 )
        {
            send_to_char( "Hp range is 1 to 32,700 hit points.\n\r", ch );
            return;
        }

        victim->max_hit = value;
        return;
    }

    if ( !str_cmp( arg2, "move" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < 0 || value > 30000 )
        {
            send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
            return;
        }

        victim->max_move = value;
        return;
    }

    if ( !str_cmp( arg2, "field" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < 0 || value > 30000 )
        {
            send_to_char( "Field range is 0 to 30,000 charge points.\n\r", ch );
            return;
        }

        victim->max_field = value;
        return;
    }

    if ( !str_cmp( arg2, "password" ) )
    {
        char* pwdnew;
        char* p;

        if ( get_trust( ch ) < LEVEL_SUB_IMPLEM )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }

        if ( IS_NPC( victim ) )
        {
            send_to_char( "Mobs don't have passwords.\n\r", ch );
            return;
        }

        if ( strlen( arg3 ) < 5 )
        {
            send_to_char(
                "New password must be at least five characters long.\n\r", ch );
            return;
        }

        /*
            No tilde allowed because of player file format.
        */
        pwdnew = crypt( arg3, ch->name );

        for ( p = pwdnew; *p != '\0'; p++ )
        {
            if ( *p == '~' )
            {
                send_to_char(
                    "New password not acceptable, try again.\n\r", ch );
                return;
            }
        }

        DISPOSE( victim->pcdata->pwd );
        victim->pcdata->pwd = str_dup( pwdnew );

        if ( IS_SET( sysdata.save_flags, SV_PASSCHG ) )
            save_char_obj( victim );

        send_to_char( "Ok.\n\r", ch );
        ch_printf( victim, "Your password has been changed by %s.\n\r", ch->name );
        return;
    }

    if ( !str_cmp( arg2, "quest" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 500 )
        {
            send_to_char( "The current quest range is 0 to 500.\n\r", ch );
            return;
        }

        victim->pcdata->quest_number = value;
        return;
    }

    if ( !str_cmp( arg2, "qpa" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        victim->pcdata->quest_accum = value;
        return;
    }

    if ( !str_cmp( arg2, "qp" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 5000 )
        {
            send_to_char( "The current quest point range is 0 to 5000.\n\r", ch );
            return;
        }

        victim->pcdata->quest_curr = value;
        return;
    }

    if ( !str_cmp( arg2, "mentalstate" ) )
    {
        if ( value < -100 || value > 100 )
        {
            send_to_char( "Value must be in range -100 to +100.\n\r", ch );
            return;
        }

        victim->mental_state = value;
        return;
    }

    if ( !str_cmp( arg2, "emotion" ) )
    {
        if ( value < -100 || value > 100 )
        {
            send_to_char( "Value must be in range -100 to +100.\n\r", ch );
            return;
        }

        victim->emotional_state = value;
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_SUPREME )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        STRFREE( victim->name );
        victim->name = STRALLOC( arg3 );

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
        {
            STRFREE( victim->pIndexData->player_name );
            victim->pIndexData->player_name = QUICKLINK( victim->name );
        }

        return;
    }

    if ( !str_cmp( arg2, "minsnoop" ) )
    {
        if ( get_trust( ch ) < LEVEL_SUB_IMPLEM )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }

        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( victim->pcdata )
        {
            victim->pcdata->min_snoop = value;
            return;
        }
    }

    if ( !str_cmp( arg2, "short" ) )
    {
        STRFREE( victim->short_descr );
        victim->short_descr = STRALLOC( arg3 );

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
        {
            STRFREE( victim->pIndexData->short_descr );
            victim->pIndexData->short_descr = QUICKLINK( victim->short_descr );
        }

        return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
        STRFREE( victim->long_descr );
        strcpy( buf, arg3 );
        strcat( buf, "\n\r" );
        victim->long_descr = STRALLOC( buf );

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
        {
            STRFREE( victim->pIndexData->long_descr );
            victim->pIndexData->long_descr = QUICKLINK( victim->long_descr );
        }

        return;
    }

    if ( !str_cmp( arg2, "description" ) )
    {
        if ( arg3[0] )
        {
            STRFREE( victim->description );
            victim->description = STRALLOC( arg3 );

            if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            {
                STRFREE( victim->pIndexData->description );
                victim->pIndexData->description = QUICKLINK( victim->description );
            }

            return;
        }

        CHECK_SUBRESTRICTED( ch );

        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;

        ch->substate = SUB_MOB_DESC;
        ch->dest_buf = victim;
        start_editing( ch, victim->description );
        return;
    }

    if ( !str_cmp( arg2, "title" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        set_title( victim, arg3 );
        return;
    }

    if ( !str_cmp( arg2, "spec" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !IS_NPC( victim ) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg3, "none" ) )
        {
            victim->spec_fun = NULL;
            send_to_char( "Special function removed.\n\r", ch );

            if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
                victim->pIndexData->spec_fun = victim->spec_fun;

            return;
        }

        if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
        {
            send_to_char( "No such spec fun.\n\r", ch );
            return;
        }

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->spec_fun = victim->spec_fun;

        return;
    }

    if ( !str_cmp( arg2, "spec2" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !IS_NPC( victim ) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg3, "none" ) )
        {
            victim->spec_2 = NULL;
            send_to_char( "Special function removed.\n\r", ch );

            if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
                victim->pIndexData->spec_2 = victim->spec_2;

            return;
        }

        if ( ( victim->spec_2 = spec_lookup( arg3 ) ) == 0 )
        {
            send_to_char( "No such spec fun.\n\r", ch );
            return;
        }

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->spec_2 = victim->spec_2;

        return;
    }

    if ( !str_cmp( arg2, "spec3" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !IS_NPC( victim ) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg3, "none" ) )
        {
            victim->spec_3 = NULL;
            send_to_char( "Special function removed.\n\r", ch );

            if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
                victim->pIndexData->spec_3 = victim->spec_3;

            return;
        }

        if ( ( victim->spec_3 = spec_lookup( arg3 ) ) == 0 )
        {
            send_to_char( "No such spec fun.\n\r", ch );
            return;
        }

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->spec_3 = victim->spec_3;

        return;
    }

    if ( !str_cmp( arg2, "spec4" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !IS_NPC( victim ) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg3, "none" ) )
        {
            victim->spec_4 = NULL;
            send_to_char( "Special function removed.\n\r", ch );

            if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
                victim->pIndexData->spec_4 = victim->spec_4;

            return;
        }

        if ( ( victim->spec_4 = spec_lookup( arg3 ) ) == 0 )
        {
            send_to_char( "No such spec fun.\n\r", ch );
            return;
        }

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->spec_4 = victim->spec_4;

        return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
        bool pcflag;

        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char( "You can only modify a mobile's flags.\n\r", ch );
            return;
        }

        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> flags <flag> [flag]...\n\r", ch );
            send_to_char( "sentinel, scavenger, aggressive, stayarea, practice, immortal,\n\r", ch );
            send_to_char( "deadly, mountable, guardian, nokill, scholar, noassist, droid, nocorpse,\n\r", ch );
            send_to_char( "surgeon, enlist, hostage\n\r", ch );
            return;
        }

        while ( argument[0] != '\0' )
        {
            pcflag = FALSE;
            argument = one_argument( argument, arg3 );
            value = IS_NPC( victim ) ? get_actflag( arg3 ) : get_plrflag( arg3 );

            if ( !IS_NPC( victim ) && ( value < 0 || value > 31 ) )
            {
                pcflag = TRUE;
                value = get_pcflag( arg3 );
            }

            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
            {
                if ( IS_NPC( victim ) && value == ACT_IS_NPC )
                    send_to_char( "If that could be changed, it would cause many problems.\n\r", ch );
                else if ( IS_NPC( victim ) && value == ACT_POLYMORPHED )
                    send_to_char( "Changing that would be a _bad_ thing.\n\r", ch );
                else
                {
                    if ( pcflag )
                        xTOGGLE_BIT( victim->pcdata->flags, value );
                    else
                    {
                        xTOGGLE_BIT( victim->act, value );

                        /* NPC check added by Gorog */
                        if ( IS_NPC( victim ) && ( value == ACT_PROTOTYPE ) )
                            victim->pIndexData->act = victim->act;
                    }
                }
            }
        }

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->act = victim->act;

        return;
    }

    if ( !str_cmp( arg2, "affected" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's flags.\n\r", ch );
            return;
        }

        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> affected <flag> [flag]...\n\r", ch );
            return;
        }

        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_aflag( arg3 );

            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                xTOGGLE_BIT( victim->affected_by, value );
        }

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->affected_by = victim->affected_by;

        return;
    }

    if ( !str_cmp( arg2, "pos" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < 0 || value > POS_STANDING )
        {
            ch_printf( ch, "Position range is 0 to %d.\n\r", POS_STANDING );
            return;
        }

        victim->position = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->position = victim->position;

        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "defpos" ) )
    {
        if ( !IS_NPC( victim ) )
        {
            send_to_char( "Mobiles only.\n\r", ch );
            return;
        }

        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < 0 || value > POS_STANDING )
        {
            ch_printf( ch, "Position range is 0 to %d.\n\r", POS_STANDING );
            return;
        }

        victim->defposition = value;

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->defposition = victim->defposition;

        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "aloaded" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Player Characters only.\n\r", ch );
            return;
        }

        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !IS_SET( victim->pcdata->area->status, AREA_LOADED ) )
        {
            SET_BIT( victim->pcdata->area->status, AREA_LOADED );
            send_to_char( "Your area set to LOADED!\n\r", victim );

            if ( ch != victim )
                send_to_char( "Area set to LOADED!\n\r", ch );

            return;
        }
        else
        {
            REMOVE_BIT( victim->pcdata->area->status, AREA_LOADED );
            send_to_char( "Your area set to NOT-LOADED!\n\r", victim );

            if ( ch != victim )
                send_to_char( "Area set to NON-LOADED!\n\r", ch );

            return;
        }
    }

    if ( !str_cmp( arg2, "speaks" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> speaks <language>\n\r", ch );
            return;
        }

        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_langflag( arg3 );

            if ( value == LANG_UNKNOWN )
                ch_printf( ch, "Unknown language: %s\n\r", arg3 );
            else
                xTOGGLE_BIT( victim->speaks, value );
        }

        if ( !IS_NPC( victim ) )
        {
            for ( i = 0; i < LANG_UNKNOWN; i++ )
                if ( race_table[victim->race].language == i )
                    xSET_BIT( victim->speaks, i );
        }
        else if ( xIS_SET( victim->act, ACT_PROTOTYPE ) )
        {
            xCLEAR_BITS( victim->pIndexData->speaks );

            for ( i = 0; i < LANG_UNKNOWN; i++ )
                if ( xIS_SET( victim->speaks, i ) )
                    xSET_BIT( victim->pIndexData->speaks, i );
        }

        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "speaking" ) )
    {
        if ( !IS_NPC( victim ) )
        {
            send_to_char( "Players must choose the language they speak themselves.\n\r", ch );
            return;
        }

        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> speaking <language>\n\r", ch );
            return;
        }

        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_langflag( arg3 );

            if ( value == LANG_UNKNOWN )
                ch_printf( ch, "Unknown language: %s\n\r", arg3 );
            else
                xTOGGLE_BIT( victim->speaking, value );
        }

        if ( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
        {
            xCLEAR_BITS( victim->pIndexData->speaking );

            for ( i = 0; i < LANG_UNKNOWN; i++ )
                if ( xIS_SET( victim->speaking, i ) )
                    xSET_BIT( victim->pIndexData->speaking, i );
        }

        send_to_char( "Done.\n\r", ch );
        return;
    }

    /*
        Generate usage message.
    */
    if ( ch->substate == SUB_REPEATCMD )
    {
        ch->substate = SUB_RESTRICTED;
        interpret( ch, origarg, FALSE );
        ch->substate = SUB_REPEATCMD;
        ch->last_cmd = do_mset;
    }
    else
        do_mset( ch, "" );

    return;
}


void do_oset( CHAR_DATA* ch, char* argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    char outbuf  [MAX_STRING_LENGTH];
    OBJ_DATA* obj, *tmpobj;
    EXTRA_DESCR_DATA* ed;
    bool lockobj;
    char* origarg = argument;
    int value, tmp;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't oset\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }

    switch ( ch->substate )
    {
        default:
            break;

        case SUB_OBJ_EXTRA:
            if ( !ch->dest_buf )
            {
                send_to_char( "Fatal error: report to Thoric.\n\r", ch );
                bug( "do_oset: sub_obj_extra: NULL ch->dest_buf", 0 );
                ch->substate = SUB_NONE;
                return;
            }

            /*
                hopefully the object didn't get extracted...
                if you're REALLY paranoid, you could always go through
                the object and index-object lists, searching through the
                extra_descr lists for a matching pointer...
            */
            ed  = ch->dest_buf;
            STRFREE( ed->description );
            ed->description = copy_buffer( ch );
            tmpobj = ch->spare_ptr;
            stop_editing( ch );
            ch->dest_buf = tmpobj;
            ch->substate = ch->tempnum;
            return;

        case SUB_OBJ_LONG:
            if ( !ch->dest_buf )
            {
                send_to_char( "Fatal error: report to Thoric.\n\r", ch );
                bug( "do_oset: sub_obj_long: NULL ch->dest_buf", 0 );
                ch->substate = SUB_NONE;
                return;
            }

            obj = ch->dest_buf;

            if ( obj && obj_extracted( obj ) )
            {
                send_to_char( "Your object was extracted!\n\r", ch );
                stop_editing( ch );
                return;
            }

            STRFREE( obj->description );
            obj->description = copy_buffer( ch );

            if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            {
                STRFREE( obj->pIndexData->description );
                obj->pIndexData->description = QUICKLINK( obj->description );
            }

            tmpobj = ch->spare_ptr;
            stop_editing( ch );
            ch->substate = ch->tempnum;
            ch->dest_buf = tmpobj;
            return;
    }

    obj = NULL;
    smash_tilde( argument );

    if ( ch->substate == SUB_REPEATCMD )
    {
        obj = ch->dest_buf;

        if ( obj && obj_extracted( obj ) )
        {
            send_to_char( "Your object was extracted!\n\r", ch );
            obj = NULL;
            argument = "done";
        }

        if ( argument[0] == '\0' || !str_cmp( argument, " " )
                ||   !str_cmp( argument, "stat" ) )
        {
            if ( obj )
                do_ostat( ch, obj->name );
            else
                send_to_char( "No object selected.  Type '?' for help.\n\r", ch );

            return;
        }

        if ( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
        {
            send_to_char( "Oset mode off.\n\r", ch );
            ch->substate = SUB_NONE;
            ch->dest_buf = NULL;

            if ( ch->pcdata && ch->pcdata->subprompt )
                STRFREE( ch->pcdata->subprompt );

            return;
        }
    }

    if ( obj )
    {
        lockobj = TRUE;
        strcpy( arg1, obj->name );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );
    }
    else
    {
        lockobj = FALSE;
        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );
    }

    if ( !str_cmp( arg1, "on" ) )
    {
        send_to_char( "Syntax: oset <object|vnum> on.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !str_cmp( arg1, "?" ) )
    {
        if ( ch->substate == SUB_REPEATCMD )
        {
            if ( obj )
                send_to_char( "Syntax: <field>  <value>\n\r",       ch );
            else
                send_to_char( "Syntax: <object> <field>  <value>\n\r",  ch );
        }
        else
            send_to_char( "&zSyntax: &Coset &z<object> <field> <value>\n\r\n\r", ch );

        send_to_char( "&zField being one of:\n\r",                        ch );
        send_to_char( "&z  flags wear level weight cost rent timer\n\r",  ch );
        send_to_char( "&z  name short long desc ed rmed actiondesc\n\r",  ch );
        send_to_char( "&z  type value0 value1 value2 value3 value4 value5\n\r", ch );
        send_to_char( "&z  affect rmaffect layers\n\r", ch );
        /*
            send_to_char( "&zFor weapons: &Bweapontype, condition, numdamdie,\n\r", ch);
            send_to_char( "&z             &Bsizedamdie, charges, maxcharges\n\r", ch);
            send_to_char( "&zFor armor: &Bac, condition\n\r", ch );
            send_to_char( "&zFor devices: &Bslevel, spell, maxcharges, charges\n\r", ch);
            send_to_char( "&zFor containers: &Bcflags, key, capacity\n\r", ch );
            send_to_char( "&zFor levers/switchs: &Btflags\n\r", ch );
            send_to_char( "&zFor rawspice: &Bspicetype, grade\n\r", ch );
            send_to_char( "&zFor ammo/battery: &Bcharges (at least 1000 for ammo)\n\r", ch );
        */
        /*
            send_to_char( "For weapons:             For armor:\n\r",    ch );
            send_to_char( "  weapontype condition     ac condition\n\r",   ch );
            send_to_char( "  numdamdie sizedamdie                  \n\r",   ch );
            send_to_char( "  charges   maxcharges                  \n\r",   ch );
            send_to_char( "For devices:\n\r",          ch );
            send_to_char( "  slevel spell maxcharges charges\n\r",     ch );
            send_to_char( "For containers:          For levers and switches:\n\r", ch );
            send_to_char( "  cflags key capacity      tflags\n\r",     ch );
            send_to_char( "For rawspice:            For ammo and batteries:\n\r",      ch );
            send_to_char( "  spicetype  grade         charges (at least 1000 for ammo)\n\r",       ch );
            send_to_char( "For crystals:\n\r",     ch );
            send_to_char( "  gemtype\n\r",     ch );
        */
        return;
    }

    if ( !obj && get_trust( ch ) <= LEVEL_IMMORTAL )
    {
        if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
        {
            send_to_char( "You can't find that here.\n\r", ch );
            return;
        }
    }
    else if ( !obj )
    {
        if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
        {
            send_to_char( "There is nothing like that in all the realms.\n\r", ch );
            return;
        }
    }

    if ( lockobj )
        ch->dest_buf = obj;
    else
        ch->dest_buf = NULL;

    separate_obj( obj );
    value = atoi( arg3 );

    if ( !str_cmp( arg2, "on" ) )
    {
        ch_printf( ch, "Oset mode on. (Editing '%s' vnum %d).\n\r",
                   obj->name, obj->pIndexData->vnum );
        ch->substate = SUB_REPEATCMD;
        ch->dest_buf = obj;

        if ( ch->pcdata )
        {
            if ( ch->pcdata->subprompt )
                STRFREE( ch->pcdata->subprompt );

            sprintf( buf, "<&COset &W#%d&w> %%i", obj->pIndexData->vnum );
            ch->pcdata->subprompt = STRALLOC( buf );
        }

        return;
    }

    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->value[0] = value;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[0] = value;

        return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->value[1] = value;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[1] = value;

        return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->value[2] = value;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            obj->pIndexData->value[2] = value;

            if ( obj->item_type == ITEM_WEAPON && value != 0 )
                obj->value[2] = obj->pIndexData->value[1] * obj->pIndexData->value[2];
        }

        return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->value[3] = value;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[3] = value;

        return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->value[4] = value;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[4] = value;

        return;
    }

    if ( !str_cmp( arg2, "value5" ) || !str_cmp( arg2, "v5" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->value[5] = value;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[5] = value;

        return;
    }

    if ( !str_cmp( arg2, "type" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: oset <object> type <type>\n\r", ch );
            send_to_char( "Possible Types:\n\r", ch );
            send_to_char( "None        Light      Weapon     Armor        Furniture  \n\r", ch );
            send_to_char( "Trash       Container  Paper      Drinkcon     Key        \n\r", ch );
            send_to_char( "Food        Corpse     Corpse_pc  Fountain     Bloodstain \n\r", ch );
            send_to_char( "Scraps      Fire       Ammo       Grenade      Radio      \n\r", ch );
            send_to_char( "Medikit     Motion     Cloak      Binocular    GPS        \n\r", ch );
            send_to_char( "Cover       Bandage    C4         Laptop       Toolkit    \n\r", ch );
            send_to_char( "Steril      Medicomp   Spawner    Landmine     Attachment \n\r", ch );
            send_to_char( "Medstation  Motionb    Ammobox    Toolchest    Mapconsole \n\r", ch );
            send_to_char( "Trap        MSpawner   Deployer   Remote       Sift       \n\r", ch );
            send_to_char( "Regenerator Flare      Sentry     SCannon      NVGoggle   \n\r", ch );
            return;
        }

        value = get_otype( argument );

        if ( value < 1 )
        {
            ch_printf( ch, "Unknown type: %s\n\r", arg3 );
            return;
        }

        obj->item_type = ( sh_int ) value;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->item_type = obj->item_type;

        return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: oset <object> flags <flag> [flag]...\n\r", ch );
            send_to_char( "glow, hum, invis, nodrop, noremove, inventory\n\r", ch );
            send_to_char( "hidden, covering, deathrot, burried, prototype\n\r", ch );
            send_to_char( "nodual, marine, predator, alien, respawn,\n\r", ch );
            send_to_char( "burstfire, autofire, semiauto, singlefire,\n\r", ch );
            send_to_char( "useon, useoff, quiet, moderate, loud, rangedonly\n\r", ch );
            send_to_char( "indestructable, nohold, alieninvis, notackle, fullhit\n\r", ch );
            return;
        }

        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_oflag( arg3 );

            if ( value < 0 || value >= MAX_ITEM_FLAG )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
            {
                xTOGGLE_BIT( obj->extra_flags, value );

                if ( value == ITEM_PROTOTYPE )
                    obj->pIndexData->extra_flags = obj->extra_flags;
            }
        }

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->extra_flags = obj->extra_flags;

        return;
    }

    if ( !str_cmp( arg2, "wear" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: oset <object> wear <flag> [flag]...\n\r", ch );
            send_to_char( "Possible locations:\n\r", ch );
            send_to_char( "take   finger   neck    body    head   legs\n\r", ch );
            send_to_char( "feet   hands    arms    shield  about  waist\n\r", ch );
            send_to_char( "wrist  wield    hold    ears    eyes   over\n\r", ch );
            send_to_char( "       shoulder\n\r", ch );
            return;
        }

        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_wflag( arg3 );

            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                xTOGGLE_BIT( obj->wear_flags, value );
        }

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->wear_flags = obj->wear_flags;

        return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->level = value;
        return;
    }

    if ( !str_cmp( arg2, "weight" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->weight = value;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->weight = value;

        return;
    }

    if ( !str_cmp( arg2, "cost" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->cost = value;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->cost = value;

        return;
    }

    if ( !str_cmp( arg2, "rent" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->rent = value;
        else
            send_to_char( "Item must have prototype flag to set this value.\n\r", ch );

        return;
    }

    if ( !str_cmp( arg2, "layers" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->layers = value;
        else
            send_to_char( "Item must have prototype flag to set this value.\n\r", ch );

        return;
    }

    if ( !str_cmp( arg2, "timer" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->timer = value;
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        STRFREE( obj->name );
        obj->name = STRALLOC( arg3 );

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            STRFREE( obj->pIndexData->name );
            obj->pIndexData->name = QUICKLINK( obj->name );
        }

        return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( arg3 );

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            STRFREE( obj->pIndexData->short_descr );
            obj->pIndexData->short_descr = QUICKLINK( obj->short_descr );
        }
        else
            /*  Feature added by Narn, Apr/96
                If the item is not proto, add the word 'rename' to the keywords
                if it is not already there.
            */
        {
            if ( str_infix( "rename", obj->name ) )
            {
                sprintf( buf, "%s %s", obj->name, "rename" );
                STRFREE( obj->name );
                obj->name = STRALLOC( buf );
            }
        }

        return;
    }

    if ( !str_cmp( arg2, "actiondesc" ) )
    {
        if ( strstr( arg3, "%n" )
                ||   strstr( arg3, "%d" )
                ||   strstr( arg3, "%l" ) )
        {
            send_to_char( "Illegal characters!\n\r", ch );
            return;
        }

        STRFREE( obj->action_desc );
        obj->action_desc = STRALLOC( arg3 );

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            STRFREE( obj->pIndexData->action_desc );
            obj->pIndexData->action_desc = QUICKLINK( obj->action_desc );
        }

        return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
        if ( arg3[0] )
        {
            STRFREE( obj->description );
            obj->description = STRALLOC( arg3 );

            if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            {
                STRFREE( obj->pIndexData->description );
                obj->pIndexData->description = QUICKLINK( obj->description );
            }

            return;
        }

        CHECK_SUBRESTRICTED( ch );

        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;

        if ( lockobj )
            ch->spare_ptr = obj;
        else
            ch->spare_ptr = NULL;

        ch->substate = SUB_OBJ_LONG;
        ch->dest_buf = obj;
        start_editing( ch, obj->description );
        return;
    }

    if ( !str_cmp( arg2, "affect" ) )
    {
        AFFECT_DATA* paf;
        sh_int loc;
        int bitv;
        argument = one_argument( argument, arg2 );

        if ( !arg2 || arg2[0] == '\0' || !argument || argument[0] == 0 )
        {
            send_to_char( "Usage: oset <object> affect <field> <value>\n\r", ch );
            send_to_char( "Affect Fields:\n\r", ch );
            send_to_char( "none        strength    stamina     recovery      intelligence bravery\n\r", ch );
            send_to_char( "perception  hit         move        affect        fire         energy\n\r", ch );
            send_to_char( "impact      pierce      acid\n\r", ch );
            return;
        }

        loc = get_atype( arg2 );

        if ( loc < 1 )
        {
            ch_printf( ch, "Unknown field: %s\n\r", arg2 );
            return;
        }

        if ( loc == APPLY_AFFECT )
        {
            bitv = 0;

            while ( argument[0] != '\0' )
            {
                argument = one_argument( argument, arg3 );
                value = get_aflag( arg3 );

                if ( value < 0 || value > 31 )
                    ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
                else
                {
                    SET_BIT( bitv, 1 << value );
                }
            }

            if ( !bitv )
                return;

            value = bitv;
        }
        else
        {
            argument = one_argument( argument, arg3 );
            value = atoi( arg3 );
        }

        CREATE( paf, AFFECT_DATA, 1 );
        paf->type       = -1;
        paf->duration       = -1;
        paf->location       = loc;
        paf->modifier       = value;
        xCLEAR_BITS( paf->bitvector );
        paf->next       = NULL;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            LINK( paf, obj->pIndexData->first_affect,
                  obj->pIndexData->last_affect, next, prev );
        else
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );

        ++top_affect;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "rmaffect" ) )
    {
        AFFECT_DATA* paf;
        sh_int loc, count;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: oset <object> rmaffect <affect#>\n\r", ch );
            return;
        }

        loc = atoi( argument );

        if ( loc < 1 )
        {
            send_to_char( "Invalid number.\n\r", ch );
            return;
        }

        count = 0;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            OBJ_INDEX_DATA* pObjIndex;
            pObjIndex = obj->pIndexData;

            for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
            {
                if ( ++count == loc )
                {
                    UNLINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
                    DISPOSE( paf );
                    send_to_char( "Removed.\n\r", ch );
                    --top_affect;
                    return;
                }
            }

            send_to_char( "Not found.\n\r", ch );
            return;
        }
        else
        {
            for ( paf = obj->first_affect; paf; paf = paf->next )
            {
                if ( ++count == loc )
                {
                    UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
                    DISPOSE( paf );
                    send_to_char( "Removed.\n\r", ch );
                    --top_affect;
                    return;
                }
            }

            send_to_char( "Not found.\n\r", ch );
            return;
        }
    }

    if ( !str_cmp( arg2, "ed" ) )
    {
        if ( !arg3 || arg3[0] == '\0' )
        {
            send_to_char( "Syntax: oset <object> ed <keywords>\n\r",
                          ch );
            return;
        }

        CHECK_SUBRESTRICTED( ch );

        if ( obj->timer )
        {
            send_to_char( "It's not safe to edit an extra description on an object with a timer.\n\rTurn it off first.\n\r", ch );
            return;
        }

        if ( obj->item_type == ITEM_PAPER )
        {
            send_to_char( "You can not add an extra description to a note paper at the moment.\n\r", ch );
            return;
        }

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            ed = SetOExtraProto( obj->pIndexData, arg3 );
        else
            ed = SetOExtra( obj, arg3 );

        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;

        if ( lockobj )
            ch->spare_ptr = obj;
        else
            ch->spare_ptr = NULL;

        ch->substate = SUB_OBJ_EXTRA;
        ch->dest_buf = ed;
        start_editing( ch, ed->description );
        return;
    }

    if ( !str_cmp( arg2, "desc" ) )
    {
        CHECK_SUBRESTRICTED( ch );

        if ( obj->timer )
        {
            send_to_char( "It's not safe to edit a description on an object with a timer.\n\rTurn it off first.\n\r", ch );
            return;
        }

        if ( obj->item_type == ITEM_PAPER )
        {
            send_to_char( "You can not add a description to a note paper at the moment.\n\r", ch );
            return;
        }

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            ed = SetOExtraProto( obj->pIndexData, obj->name );
        else
            ed = SetOExtra( obj, obj->name );

        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;

        if ( lockobj )
            ch->spare_ptr = obj;
        else
            ch->spare_ptr = NULL;

        ch->substate = SUB_OBJ_EXTRA;
        ch->dest_buf = ed;
        start_editing( ch, ed->description );
        return;
    }

    if ( !str_cmp( arg2, "rmed" ) )
    {
        if ( !arg3 || arg3[0] == '\0' )
        {
            send_to_char( "Syntax: oset <object> rmed <keywords>\n\r", ch );
            return;
        }

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            if ( DelOExtraProto( obj->pIndexData, arg3 ) )
                send_to_char( "Deleted.\n\r", ch );
            else
                send_to_char( "Not found.\n\r", ch );

            return;
        }

        if ( DelOExtra( obj, arg3 ) )
            send_to_char( "Deleted.\n\r", ch );
        else
            send_to_char( "Not found.\n\r", ch );

        return;
    }

    /*
        Make it easier to set special object values by name than number
                            -Thoric
    */
    tmp = -1;

    switch ( obj->item_type )
    {
        case ITEM_WEAPON:
            if ( !str_cmp( arg2, "condition" ) )
                tmp = 0;

            if ( !str_cmp( arg2, "numdamdie" ) )
                tmp = 1;

            if ( !str_cmp( arg2, "sizedamdie" ) )
                tmp = 2;

            if ( !str_cmp( arg2, "charges" ) )
                tmp = 4;

            if ( !str_cmp( arg2, "maxcharges" ) )
                tmp = 5;

            if ( !str_cmp( arg2, "charge" ) )
                tmp = 4;

            if ( !str_cmp( arg2, "maxcharge" ) )
                tmp = 5;

            break;

        case ITEM_AMMO:
            if ( !str_cmp( arg2, "charges" ) )
                tmp = 0;

            if ( !str_cmp( arg2, "charge" ) )
                tmp = 0;

            break;

        case ITEM_ARMOR:
            if ( !str_cmp( arg2, "condition" ) )
                tmp = 0;

            if ( !str_cmp( arg2, "ac" ) )
                tmp = 1;

            break;

        case ITEM_CONTAINER:
            if ( !str_cmp( arg2, "capacity" ) )
                tmp = 0;

            if ( !str_cmp( arg2, "cflags" ) )
                tmp = 1;

            if ( !str_cmp( arg2, "key" ) )
                tmp = 2;

            break;
    }

    if ( tmp >= 0 && tmp <= 5 )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        obj->value[tmp] = value;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[tmp] = value;

        return;
    }

    /*
        Generate usage message.
    */
    if ( ch->substate == SUB_REPEATCMD )
    {
        ch->substate = SUB_RESTRICTED;
        interpret( ch, origarg, FALSE );
        ch->substate = SUB_REPEATCMD;
        ch->last_cmd = do_oset;
    }
    else
        do_oset( ch, "" );

    return;
}

/*
    Returns value 0 - 9 based on directional text.
*/
int get_dir( char* txt )
{
    int edir;
    char c1, c2;

    if ( !str_cmp( txt, "northeast" ) )
        return DIR_NORTHEAST;

    if ( !str_cmp( txt, "northwest" ) )
        return DIR_NORTHWEST;

    if ( !str_cmp( txt, "southeast" ) )
        return DIR_SOUTHEAST;

    if ( !str_cmp( txt, "southwest" ) )
        return DIR_SOUTHWEST;

    if ( !str_cmp( txt, "somewhere" ) )
        return 10;

    c1 = txt[0];

    if ( c1 == '\0' )
        return 0;

    c2 = txt[1];
    edir = 0;

    switch ( c1 )
    {
        case 'n':
            switch ( c2 )
            {
                default:
                    edir = 0;
                    break; /* north */

                case 'e':
                    edir = 6;
                    break; /* ne    */

                case 'w':
                    edir = 7;
                    break; /* nw    */
            }

            break;

        case '0':
            edir = 0;
            break; /* north */

        case 'e':
        case '1':
            edir = 1;
            break; /* east  */

        case 's':
            switch ( c2 )
            {
                default:
                    edir = 2;
                    break; /* south */

                case 'e':
                    edir = 8;
                    break; /* se    */

                case 'w':
                    edir = 9;
                    break; /* sw    */
            }

            break;

        case '2':
            edir = 2;
            break; /* south */

        case 'w':
        case '3':
            edir = 3;
            break; /* west  */

        case 'u':
        case '4':
            edir = 4;
            break; /* up    */

        case 'd':
        case '5':
            edir = 5;
            break; /* down  */

        case '6':
            edir = 6;
            break; /* ne    */

        case '7':
            edir = 7;
            break; /* nw    */

        case '8':
            edir = 8;
            break; /* se    */

        case '9':
            edir = 9;
            break; /* sw    */

        case '?':
            edir = 10;
            break; /* somewhere */
    }

    return edir;
}

char* sprint_reset( CHAR_DATA* ch, RESET_DATA* pReset, sh_int num, bool rlist )
{
    static char buf[MAX_STRING_LENGTH];
    char mobname[MAX_STRING_LENGTH];
    char roomname[MAX_STRING_LENGTH];
    char objname[MAX_STRING_LENGTH];
    static ROOM_INDEX_DATA* room;
    static OBJ_INDEX_DATA* obj, *obj2;
    static MOB_INDEX_DATA* mob;
    int rvnum;

    if ( ch->in_room )
        rvnum = ch->in_room->vnum;

    if ( num == 1 )
    {
        room = NULL;
        obj  = NULL;
        obj2 = NULL;
        mob  = NULL;
    }

    switch ( pReset->command )
    {
        default:
            sprintf( buf, "%2d) *** BAD RESET: %c %d %d %d %d ***\n\r",
                     num,
                     pReset->command,
                     pReset->extra,
                     pReset->arg1,
                     pReset->arg2,
                     pReset->arg3 );
            break;

        case 'M':
            mob = get_mob_index( pReset->arg1 );
            room = get_room_index( pReset->arg3 );

            if ( mob )
                strcpy( mobname, mob->player_name );
            else
                strcpy( mobname, "Mobile: *BAD VNUM*" );

            if ( room )
                strcpy( roomname, room->name );
            else
                strcpy( roomname, "Room: *BAD VNUM*" );

            sprintf( buf, "%2d) %s (%d) -> %s (%d) [%d]\n\r",
                     num,
                     mobname,
                     pReset->arg1,
                     roomname,
                     pReset->arg3,
                     pReset->arg2 );
            break;

        case 'E':
            if ( !mob )
                strcpy( mobname, "* ERROR: NO MOBILE! *" );

            if ( ( obj = get_obj_index( pReset->arg1 ) ) == NULL )
                strcpy( objname, "Object: *BAD VNUM*" );
            else
                strcpy( objname, obj->name );

            sprintf( buf, "%2d) %s (%d) -> %s (%s) [%d]\n\r",
                     num,
                     objname,
                     pReset->arg1,
                     mobname,
                     wear_locs[pReset->arg3],
                     pReset->arg2 );
            break;

        case 'H':
            if ( pReset->arg1 > 0
                    &&  ( obj = get_obj_index( pReset->arg1 ) ) == NULL )
                strcpy( objname, "Object: *BAD VNUM*" );
            else if ( !obj )
                strcpy( objname, "Object: *NULL obj*" );

            sprintf( buf, "%2d) Hide %s (%d)\n\r",
                     num,
                     objname,
                     obj ? obj->vnum : pReset->arg1 );
            break;

        case 'G':
            if ( !mob )
                strcpy( mobname, "* ERROR: NO MOBILE! *" );

            if ( ( obj = get_obj_index( pReset->arg1 ) ) == NULL )
                strcpy( objname, "Object: *BAD VNUM*" );
            else
                strcpy( objname, obj->name );

            sprintf( buf, "%2d) %s (%d) -> %s (carry) [%d]\n\r",
                     num,
                     objname,
                     pReset->arg1,
                     mobname,
                     pReset->arg2 );
            break;

        case 'O':
            if ( ( obj = get_obj_index( pReset->arg1 ) ) == NULL )
                strcpy( objname, "Object: *BAD VNUM*" );
            else
                strcpy( objname, obj->name );

            room = get_room_index( pReset->arg3 );

            if ( !room )
                strcpy( roomname, "Room: *BAD VNUM*" );
            else
                strcpy( roomname, room->name );

            sprintf( buf, "%2d) (object) %s (%d) -> %s (%d) [%d]\n\r",
                     num,
                     objname,
                     pReset->arg1,
                     roomname,
                     pReset->arg3,
                     pReset->arg2 );
            break;

        case 'P':
            if ( ( obj2 = get_obj_index( pReset->arg1 ) ) == NULL )
                strcpy( objname, "Object1: *BAD VNUM*" );
            else
                strcpy( objname, obj2->name );

            if ( pReset->arg3 > 0
                    &&  ( obj = get_obj_index( pReset->arg3 ) ) == NULL )
                strcpy( roomname, "Object2: *BAD VNUM*" );
            else if ( !obj )
                strcpy( roomname, "Object2: *NULL obj*" );
            else
                strcpy( roomname, obj->name );

            sprintf( buf, "%2d) (Put) %s (%d) -> %s (%d) [%d]\n\r",
                     num,
                     objname,
                     pReset->arg1,
                     roomname,
                     obj ? obj->vnum : pReset->arg3,
                     pReset->arg2 );
            break;

        case 'D':
            if ( pReset->arg2 < 0 || pReset->arg2 > MAX_DIR + 1 )
                pReset->arg2 = 0;

            if ( ( room = get_room_index( pReset->arg1 ) ) == NULL )
            {
                strcpy( roomname, "Room: *BAD VNUM*" );
                sprintf( objname, "%s (no exit)",
                         dir_name[pReset->arg2] );
            }
            else
            {
                strcpy( roomname, room->name );
                sprintf( objname, "%s%s",
                         dir_name[pReset->arg2],
                         get_exit( room, pReset->arg2 ) ? "" : " (NO EXIT!)" );
            }

            switch ( pReset->arg3 )
            {
                default:
                    strcpy( mobname, "(* ERROR *)" );
                    break;

                case 0:
                    strcpy( mobname, "Open" );
                    break;

                case 1:
                    strcpy( mobname, "Close" );
                    break;

                case 2:
                    strcpy( mobname, "Close and lock" );
                    break;
            }

            sprintf( buf, "%2d) %s [%d] the %s [%d] door %s (%d)\n\r",
                     num,
                     mobname,
                     pReset->arg3,
                     objname,
                     pReset->arg2,
                     roomname,
                     pReset->arg1 );
            break;

        case 'R':
            if ( ( room = get_room_index( pReset->arg1 ) ) == NULL )
                strcpy( roomname, "Room: *BAD VNUM*" );
            else
                strcpy( roomname, room->name );

            sprintf( buf, "%2d) Randomize exits 0 to %d -> %s (%d)\n\r",
                     num,
                     pReset->arg2,
                     roomname,
                     pReset->arg1 );
            break;

        case 'T':
            sprintf( buf, "%2d) TRAP: %d %d %d %d\n\r",
                     num,
                     pReset->extra,
                     pReset->arg1,
                     pReset->arg2,
                     pReset->arg3 );
            break;
    }

    if ( rlist && ( !room || ( room && room->vnum != rvnum ) ) )
        return NULL;

    return buf;
}

void do_redit( CHAR_DATA* ch, char* argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    ROOM_INDEX_DATA* location, *tmp;
    EXTRA_DESCR_DATA*    ed;
    char        dir;
    EXIT_DATA*       xit, *texit;
    int         value;
    int         edir, ekey, evnum;
    char*        origarg = argument;

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor.\n\r", ch );
        return;
    }

    switch ( ch->substate )
    {
        default:
            break;

        case SUB_ROOM_DESC:
            location = ch->dest_buf;

            if ( !location )
            {
                bug( "redit: sub_room_desc: NULL ch->dest_buf", 0 );
                location = ch->in_room;
            }

            STRFREE( location->description );
            location->description = copy_buffer( ch );
            stop_editing( ch );
            ch->substate = ch->tempnum;
            return;

        case SUB_ROOM_DESC2:
            location = ch->dest_buf;

            if ( !location )
            {
                bug( "redit: sub_room_desc: NULL ch->dest_buf", 0 );
                location = ch->in_room;
            }

            STRFREE( location->hdescription );
            location->hdescription = copy_buffer( ch );
            stop_editing( ch );
            ch->substate = ch->tempnum;
            return;

        case SUB_ROOM_EXTRA:
            ed = ch->dest_buf;

            if ( !ed )
            {
                bug( "redit: sub_room_extra: NULL ch->dest_buf", 0 );
                stop_editing( ch );
                return;
            }

            STRFREE( ed->description );
            ed->description = copy_buffer( ch );
            stop_editing( ch );
            ch->substate = ch->tempnum;
            return;
    }

    location = ch->in_room;
    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( ch->substate == SUB_REPEATCMD )
    {
        if ( arg[0] == '\0' )
        {
            do_rstat( ch, "" );
            return;
        }

        if ( !str_cmp( arg, "done" ) || !str_cmp( arg, "off" ) )
        {
            send_to_char( "Redit mode off.\n\r", ch );

            if ( ch->pcdata && ch->pcdata->subprompt )
                STRFREE( ch->pcdata->subprompt );

            ch->substate = SUB_NONE;
            return;
        }
    }

    if ( arg[0] == '\0' || !str_cmp( arg, "?" ) )
    {
        if ( ch->substate == SUB_REPEATCMD )
            send_to_char( "Syntax: <field> value\n\r",            ch );
        else
            send_to_char( "Syntax: redit <field> value\n\r",      ch );

        send_to_char( "\n\r",                       ch );
        send_to_char( "Field being one of:\n\r",            ch );
        send_to_char( "  name desc hdesc ed rmed\n\r",                  ch );
        send_to_char( "  exit bexit exdesc exflags exname exkey\n\r",   ch );
        send_to_char( "  flags hflags sector teledelay televnum\n\r",   ch );
        send_to_char( "  tunnel rlist exdistance flags2\n\r",      ch );
        return;
    }

    if ( !can_rmodify( ch, location ) )
        return;

    /*
        if ( !str_cmp( arg, "on" ) )
        {
        send_to_char( "Redit mode on.\n\r", ch );
        ch->substate = SUB_REPEATCMD;
        if ( ch->pcdata )
        {
           if ( ch->pcdata->subprompt )
            STRFREE( ch->pcdata->subprompt );
           ch->pcdata->subprompt = STRALLOC( "<&CRedit &W#%r&w> %i" );
        }
        return;
        }
    */

    if ( !str_cmp( arg, "substate" ) )
    {
        argument = one_argument( argument, arg2 );

        if ( !str_cmp( arg2, "north" )  )
        {
            ch->inter_substate = SUB_NORTH;
            return;
        }

        if ( !str_cmp( arg2, "east" )  )
        {
            ch->inter_substate = SUB_EAST;
            return;
        }

        if ( !str_cmp( arg2, "south" )  )
        {
            ch->inter_substate = SUB_SOUTH;
            return;
        }

        if ( !str_cmp( arg2, "west" )  )
        {
            ch->inter_substate = SUB_WEST;
            return;
        }

        if ( !str_cmp( arg2, "up" )  )
        {
            ch->inter_substate = SUB_UP;
            return;
        }

        if ( !str_cmp( arg2, "down" )  )
        {
            ch->inter_substate = SUB_DOWN;
            return;
        }

        send_to_char( " unrecognized substate in redit\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "name" ) )
    {
        if ( argument[0] == '\0' )
        {
            send_to_char( "Set the room name.  A very brief single line room description.\n\r", ch );
            send_to_char( "Usage: redit name <Room summary>\n\r", ch );
            return;
        }

        STRFREE( location->name );
        location->name = STRALLOC( argument );
        return;
    }

    if ( !str_cmp( arg, "desc" ) )
    {
        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;

        ch->substate = SUB_ROOM_DESC;
        ch->dest_buf = location;
        start_editing( ch, location->description );
        return;
    }

    if ( !str_cmp( arg, "hdesc" ) )
    {
        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;

        ch->substate = SUB_ROOM_DESC2;
        ch->dest_buf = location;
        start_editing( ch, location->hdescription );
        return;
    }

    if ( !str_cmp( arg, "tunnel" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the maximum characters allowed in the room at one time. (0 = unlimited).\n\r", ch );
            send_to_char( "Usage: redit tunnel <value>\n\r", ch );
            return;
        }

        location->tunnel = URANGE( 0, atoi( argument ), 1000 );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "ed" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Create an extra description.\n\r", ch );
            send_to_char( "You must supply keyword(s).\n\r", ch );
            return;
        }

        CHECK_SUBRESTRICTED( ch );
        ed = SetRExtra( location, argument );

        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;

        ch->substate = SUB_ROOM_EXTRA;
        ch->dest_buf = ed;
        start_editing( ch, ed->description );
        return;
    }

    if ( !str_cmp( arg, "rmed" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Remove an extra description.\n\r", ch );
            send_to_char( "You must supply keyword(s).\n\r", ch );
            return;
        }

        if ( DelRExtra( location, argument ) )
            send_to_char( "Deleted.\n\r", ch );
        else
            send_to_char( "Not found.\n\r", ch );

        return;
    }

    if ( !str_cmp( arg, "rlist" ) )
    {
        RESET_DATA* pReset;
        char* bptr;
        AREA_DATA* tarea;
        sh_int num;
        tarea = location->area;

        if ( !tarea->first_reset )
        {
            send_to_char( "This area has no resets to list.\n\r", ch );
            return;
        }

        num = 0;

        for ( pReset = tarea->first_reset; pReset; pReset = pReset->next )
        {
            num++;

            if ( ( bptr = sprint_reset( ch, pReset, num, TRUE ) ) == NULL )
                continue;

            send_to_char( bptr, ch );
        }

        return;
    }

    if ( !str_cmp( arg, "flags" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Toggle the room flags.\n\r", ch );
            send_to_char( "Usage: redit flags <flag> [flag]...\n\r", ch );
            send_to_char( "\n\rPossible Flags: \n\r", ch );
            send_to_char( "dark, nomob, indoors, can_land, can_fly, can_drive,\n\r", ch );
            send_to_char( "shipyard, nodropall, logspeech, teleport, hotel,\n\r", ch );
            send_to_char( "nofloor, nodrop, viewport, spacecraft, prototype,\n\r", ch );
            send_to_char( "save_eq, underground, bacta, fallcatch, lit, deploy_from,\n\r", ch );
            send_to_char( "hived, mapstart, nohive, safe, deploy_p, deploy_m,\n\r", ch );
            send_to_char( "deploy_a, vented_a, vented_b, vented_c, vented_d\n\r", ch );
            send_to_char( "rescue, cp, static, nohide\n\r", ch );
            return;
        }

        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg2 );
            value = get_rflag( arg2 );

            if ( value < 0 || value >= MAX_ROOM_FLAGS )
                ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
            else
            {
                if ( value == ROOM_MAPSTART && get_trust( ch ) < 105 )
                {
                    send_to_char( "&ROnly Administrators can set coord-mapping start points.\n\r", ch );
                }
                else
                {
                    if ( value == ROOM_HIVED )
                        hflag_toggle( location );

                    xTOGGLE_BIT( location->room_flags, value );
                }
            }
        }

        return;
    }

    if ( !str_cmp( arg, "hflags" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            do_redit( ch, "flags" );
            return;
        }

        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg2 );
            value = get_rflag( arg2 );

            if ( value < 0 || value >= MAX_ROOM_FLAGS )
                ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
            else
            {
                if ( value == ROOM_MAPSTART && get_trust( ch ) < 105 )
                {
                    send_to_char( "&ROnly Administrators can set coord-mapping start points.\n\r", ch );
                }
                else if ( value == ROOM_HIVED )
                {
                    send_to_char( "&RYou can't set HIVED as a HFLAG. It'd get screwy.\n\r", ch );
                }
                else
                {
                    xTOGGLE_BIT( location->hroom_flags, value );
                }
            }
        }

        return;
    }

    if ( !str_cmp( arg, "teledelay" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the delay of the teleport. (0 = off).\n\r", ch );
            send_to_char( "Usage: redit teledelay <value>\n\r", ch );
            return;
        }

        location->tele_delay = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "televnum" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the vnum of the room to teleport to.\n\r", ch );
            send_to_char( "Usage: redit televnum <vnum>\n\r", ch );
            return;
        }

        location->tele_vnum = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "sector" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the sector type.\n\r", ch );
            send_to_char( "Usage: redit sector <value>\n\r", ch );
            send_to_char( "\n\rSector Values:\n\r", ch );
            send_to_char( "0:dark, 1:city, 2:field, 3:forest, 4:hills, 5:mountain, 6:water_swim\n\r", ch );
            send_to_char( "7:water_noswim, 8:underwater, 9:air, 10:desert, 11:unkown, 12:oceanfloor, 13:underground\n\r", ch );
            return;
        }

        location->sector_type = atoi( argument );

        if ( location->sector_type < 0 || location->sector_type >= SECT_MAX )
        {
            location->sector_type = 1;
            send_to_char( "Out of range\n\r.", ch );
        }
        else
            send_to_char( "Done.\n\r", ch );

        return;
    }

    if ( !str_cmp( arg, "exkey" ) )
    {
        argument = one_argument( argument, arg2 );
        argument = one_argument( argument, arg3 );

        if ( arg2[0] == '\0' || arg3[0] == '\0' )
        {
            send_to_char( "Usage: redit exkey <dir> <key vnum>\n\r", ch );
            return;
        }

        if ( arg2[0] == '#' )
        {
            edir = atoi( arg2 + 1 );
            xit = get_exit_num( location, edir );
        }
        else
        {
            edir = get_dir( arg2 );
            xit = get_exit( location, edir );
        }

        value = atoi( arg3 );

        if ( !xit )
        {
            send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
            return;
        }

        xit->key = value;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "exname" ) )
    {
        argument = one_argument( argument, arg2 );

        if ( arg2[0] == '\0' )
        {
            send_to_char( "Change or clear exit keywords.\n\r", ch );
            send_to_char( "Usage: redit exname <dir> [keywords]\n\r", ch );
            return;
        }

        if ( arg2[0] == '#' )
        {
            edir = atoi( arg2 + 1 );
            xit = get_exit_num( location, edir );
        }
        else
        {
            edir = get_dir( arg2 );
            xit = get_exit( location, edir );
        }

        if ( !xit )
        {
            send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
            return;
        }

        STRFREE( xit->keyword );
        xit->keyword = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "exflags" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Toggle or display exit flags.\n\r", ch );
            send_to_char( "Usage: redit exflags <dir> <flag> [flag]...\n\r", ch );
            send_to_char( "\n\rExit flags:\n\r", ch );
            send_to_char( "isdoor, closed, locked, can_look, searchable, can_leave, can_climb,\n\r", ch );
            send_to_char( "nopassdoor, secret, pickproof, fly, climb, dig, window, auto, can_enter\n\r", ch );
            send_to_char( "hidden, no_mob, bashproof, bashed, keypad, barred, nospec, blastopen\n\r", ch );
            send_to_char( "blasted, armored, novent\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg2 );

        if ( arg2[0] == '#' )
        {
            edir = atoi( arg2 + 1 );
            xit = get_exit_num( location, edir );
        }
        else
        {
            edir = get_dir( arg2 );
            xit = get_exit( location, edir );
        }

        if ( !xit )
        {
            send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
            return;
        }

        if ( argument[0] == '\0' )
        {
            sprintf( buf, "Flags for exit direction: %d  Keywords: %s  Key: %d\n\r[ ",
                     xit->vdir, xit->keyword, xit->key );

            for ( value = 0; value <= MAX_EXFLAG; value++ )
            {
                if ( xIS_SET( xit->exit_info, value ) )
                {
                    strcat( buf, ex_flags[value] );
                    strcat( buf, " " );
                }
            }

            strcat( buf, "]\n\r" );
            send_to_char( buf, ch );
            return;
        }

        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg2 );
            value = get_exflag( arg2 );

            if ( value < 0 || value > MAX_EXFLAG )
                ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
            else
                xTOGGLE_BIT( xit->exit_info, value );
        }

        return;
    }

    if ( !str_cmp( arg, "ex_flags" ) )
    {
        argument = one_argument( argument, arg2 );

        switch ( ch->inter_substate )
        {
            case SUB_EAST :
                dir = 'e';
                edir = 1;
                break;

            case SUB_WEST :
                dir = 'w';
                edir = 3;
                break;

            case SUB_SOUTH:
                dir = 's';
                edir = 2;
                break;

            case SUB_UP   :
                dir = 'u';
                edir = 4;
                break;

            case SUB_DOWN :
                dir = 'd';
                edir = 5;
                break;

            default:
            case SUB_NORTH:
                dir = 'n';
                edir = 0;
                break;
        }

        value = get_exflag( arg2 );

        if ( value < 0 )
        {
            send_to_char( "Bad exit flag. \n\r", ch );
            return;
        }

        if ( ( xit = get_exit( location, edir ) ) == NULL )
        {
            sprintf( buf, "exit %c 1", dir );
            do_redit( ch, buf );
            xit = get_exit( location, edir );
        }

        xTOGGLE_BIT( xit->exit_info, value );
        return;
    }

    if ( !str_cmp( arg, "ex_to_room" ) )
    {
        argument = one_argument( argument, arg2 );

        switch ( ch->inter_substate )
        {
            case SUB_EAST :
                dir = 'e';
                edir = 1;
                break;

            case SUB_WEST :
                dir = 'w';
                edir = 3;
                break;

            case SUB_SOUTH:
                dir = 's';
                edir = 2;
                break;

            case SUB_UP   :
                dir = 'u';
                edir = 4;
                break;

            case SUB_DOWN :
                dir = 'd';
                edir = 5;
                break;

            default:
            case SUB_NORTH:
                dir = 'n';
                edir = 0;
                break;
        }

        evnum = atoi( arg2 );

        if ( evnum < 1 || evnum > ( MAX_VNUMS - 1 ) )
        {
            send_to_char( "Invalid room number.\n\r", ch );
            return;
        }

        if ( ( tmp = get_room_index( evnum ) ) == NULL )
        {
            send_to_char( "Non-existant room.\n\r", ch );
            return;
        }

        if ( ( xit = get_exit( location, edir ) ) == NULL )
        {
            sprintf( buf, "exit %c 1", dir );
            do_redit( ch, buf );
            xit = get_exit( location, edir );
        }

        xit->vnum = evnum;
        return;
    }

    if ( !str_cmp( arg, "ex_key" ) )
    {
        argument = one_argument( argument, arg2 );

        switch ( ch->inter_substate )
        {
            case SUB_EAST :
                dir = 'e';
                edir = 1;
                break;

            case SUB_WEST :
                dir = 'w';
                edir = 3;
                break;

            case SUB_SOUTH:
                dir = 's';
                edir = 2;
                break;

            case SUB_UP   :
                dir = 'u';
                edir = 4;
                break;

            case SUB_DOWN :
                dir = 'd';
                edir = 5;
                break;

            default:
            case SUB_NORTH:
                dir = 'n';
                edir = 0;
                break;
        }

        if ( ( xit = get_exit( location, edir ) ) == NULL )
        {
            sprintf( buf, "exit %c 1", dir );
            do_redit( ch, buf );
            xit = get_exit( location, edir );
        }

        xit->key = atoi( arg2 );
        return;
    }

    if ( !str_cmp( arg, "ex_exdesc" ) )
    {
        switch ( ch->inter_substate )
        {
            case SUB_EAST :
                dir = 'e';
                edir = 1;
                break;

            case SUB_WEST :
                dir = 'w';
                edir = 3;
                break;

            case SUB_SOUTH:
                dir = 's';
                edir = 2;
                break;

            case SUB_UP   :
                dir = 'u';
                edir = 4;
                break;

            case SUB_DOWN :
                dir = 'd';
                edir = 5;
                break;

            default:
            case SUB_NORTH:
                dir = 'n';
                edir = 0;
                break;
        }

        if ( ( xit = get_exit( location, edir ) ) == NULL )
        {
            sprintf( buf, "exit %c 1", dir );
            do_redit( ch, buf );
        }

        sprintf( buf, "exdesc %c %s", dir, argument );
        do_redit( ch, buf );
        return;
    }

    if ( !str_cmp( arg, "ex_keywords" ) )  /* not called yet */
    {
        switch ( ch->inter_substate )
        {
            case SUB_EAST :
                dir = 'e';
                edir = 1;
                break;

            case SUB_WEST :
                dir = 'w';
                edir = 3;
                break;

            case SUB_SOUTH:
                dir = 's';
                edir = 2;
                break;

            case SUB_UP   :
                dir = 'u';
                edir = 4;
                break;

            case SUB_DOWN :
                dir = 'd';
                edir = 5;
                break;

            default:
            case SUB_NORTH:
                dir = 'n';
                edir = 0;
                break;
        }

        if ( ( xit = get_exit( location, edir ) ) == NULL )
        {
            sprintf( buf, "exit %c 1", dir );
            do_redit( ch, buf );

            if ( ( xit = get_exit( location, edir ) ) == NULL )
                return;
        }

        sprintf( buf, "%s %s", xit->keyword, argument );
        STRFREE( xit->keyword );
        xit->keyword = STRALLOC( buf );
        return;
    }

    if ( !str_cmp( arg, "exit" ) )
    {
        bool addexit, numnotdir;
        argument = one_argument( argument, arg2 );
        argument = one_argument( argument, arg3 );

        if ( !arg2 || arg2[0] == '\0' )
        {
            send_to_char( "Create, change or remove an exit.\n\r", ch );
            send_to_char( "Usage: redit exit <dir> [room] [flags] [key] [keywords]\n\r", ch );
            return;
        }

        addexit = numnotdir = FALSE;

        switch ( arg2[0] )
        {
            default:
                edir = get_dir( arg2 );
                break;

            case '+':
                edir = get_dir( arg2 + 1 );
                addexit = TRUE;
                break;

            case '#':
                edir = atoi( arg2 + 1 );
                numnotdir = TRUE;
                break;
        }

        if ( !arg3 || arg3[0] == '\0' )
            evnum = 0;
        else
            evnum = atoi( arg3 );

        if ( numnotdir )
        {
            if ( ( xit = get_exit_num( location, edir ) ) != NULL )
                edir = xit->vdir;
        }
        else
            xit = get_exit( location, edir );

        if ( !evnum )
        {
            if ( xit )
            {
                extract_exit( location, xit );
                send_to_char( "Exit removed.\n\r", ch );
                return;
            }

            send_to_char( "No exit in that direction.\n\r", ch );
            return;
        }

        if ( evnum < 1 || evnum > ( MAX_VNUMS - 1 ) )
        {
            send_to_char( "Invalid room number.\n\r", ch );
            return;
        }

        if ( ( tmp = get_room_index( evnum ) ) == NULL )
        {
            send_to_char( "Non-existant room.\n\r", ch );
            return;
        }

        if ( get_trust( ch ) <= LEVEL_IMMORTAL && tmp->area != location->area )
        {
            send_to_char( "You can't make an exit to that room.\n\r", ch );
            return;
        }

        if ( addexit || !xit )
        {
            if ( numnotdir )
            {
                send_to_char( "Cannot add an exit by number, sorry.\n\r", ch );
                return;
            }

            if ( addexit && xit && get_exit_to( location, edir, tmp->vnum ) )
            {
                send_to_char( "There is already an exit in that direction leading to that location.\n\r", ch );
                return;
            }

            xit = make_exit( location, tmp, edir );
            xit->keyword        = STRALLOC( "" );
            xit->description        = STRALLOC( "" );
            xit->key            = -1;
            xCLEAR_BITS( xit->exit_info );
            act( AT_IMMORT, "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM );
        }
        else
            act( AT_IMMORT, "Something is different...", ch, NULL, NULL, TO_ROOM );

        if ( xit->to_room != tmp )
        {
            xit->to_room = tmp;
            xit->vnum = evnum;
            texit = get_exit_to( xit->to_room, rev_dir[edir], location->vnum );

            if ( texit )
            {
                texit->rexit = xit;
                xit->rexit = texit;
            }
        }

        argument = one_argument( argument, arg3 );

        /*
            if ( arg3 && arg3[0] != '\0' )
            xit->exit_info = atoi( arg3 );
        */

        if ( argument && argument[0] != '\0' )
        {
            one_argument( argument, arg3 );
            ekey = atoi( arg3 );

            if ( ekey != 0 || arg3[0] == '0' )
            {
                argument = one_argument( argument, arg3 );
                xit->key = ekey;
            }

            if ( argument && argument[0] != '\0' )
            {
                STRFREE( xit->keyword );
                xit->keyword = STRALLOC( argument );
            }
        }

        send_to_char( "Done.\n\r", ch );
        return;
    }

    /*
        Twisted and evil, but works              -Thoric
        Makes an exit, and the reverse in one shot.
    */
    if ( !str_cmp( arg, "bexit" ) )
    {
        EXIT_DATA* xit, *rxit;
        char tmpcmd[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA* tmploc;
        int vnum, exnum;
        char rvnum[MAX_INPUT_LENGTH];
        bool numnotdir;
        argument = one_argument( argument, arg2 );
        argument = one_argument( argument, arg3 );

        if ( !arg2 || arg2[0] == '\0' )
        {
            send_to_char( "Create, change or remove a two-way exit.\n\r", ch );
            send_to_char( "Usage: redit bexit <dir> [room] [flags] [key] [keywords]\n\r", ch );
            return;
        }

        numnotdir = FALSE;

        switch ( arg2[0] )
        {
            default:
                edir = get_dir( arg2 );
                break;

            case '#':
                numnotdir = TRUE;
                edir = atoi( arg2 + 1 );
                break;

            case '+':
                edir = get_dir( arg2 + 1 );
                break;
        }

        tmploc = location;
        exnum = edir;

        if ( numnotdir )
        {
            if ( ( xit = get_exit_num( tmploc, edir ) ) != NULL )
                edir = xit->vdir;
        }
        else
            xit = get_exit( tmploc, edir );

        rxit = NULL;
        vnum = 0;
        rvnum[0] = '\0';

        if ( xit )
        {
            vnum = xit->vnum;

            if ( arg3[0] != '\0' )
                sprintf( rvnum, "%d", tmploc->vnum );

            if ( xit->to_room )
                rxit = get_exit( xit->to_room, rev_dir[edir] );
            else
                rxit = NULL;
        }

        sprintf( tmpcmd, "exit %s %s %s", arg2, arg3, argument );
        do_redit( ch, tmpcmd );

        if ( numnotdir )
            xit = get_exit_num( tmploc, exnum );
        else
            xit = get_exit( tmploc, edir );

        if ( !rxit && xit )
        {
            vnum = xit->vnum;

            if ( arg3[0] != '\0' )
                sprintf( rvnum, "%d", tmploc->vnum );

            if ( xit->to_room )
                rxit = get_exit( xit->to_room, rev_dir[edir] );
            else
                rxit = NULL;
        }

        if ( vnum )
        {
            sprintf( tmpcmd, "%d redit exit %d %s %s",
                     vnum,
                     rev_dir[edir],
                     rvnum,
                     argument );
            do_at( ch, tmpcmd );
        }

        return;
    }

    if ( !str_cmp( arg, "exdistance" ) )
    {
        argument = one_argument( argument, arg2 );

        if ( !arg2 || arg2[0] == '\0' )
        {
            send_to_char( "Set the distance (in rooms) between this room, and the destination room.\n\r", ch );
            send_to_char( "Usage: redit exdistance <dir> [distance]\n\r", ch );
            return;
        }

        if ( arg2[0] == '#' )
        {
            edir = atoi( arg2 + 1 );
            xit = get_exit_num( location, edir );
        }
        else
        {
            edir = get_dir( arg2 );
            xit = get_exit( location, edir );
        }

        if ( xit )
        {
            xit->distance = URANGE( 1, atoi( argument ), 50 );
            send_to_char( "Done.\n\r", ch );
            return;
        }

        send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "exdesc" ) )
    {
        argument = one_argument( argument, arg2 );

        if ( !arg2 || arg2[0] == '\0' )
        {
            send_to_char( "Create or clear a description for an exit.\n\r", ch );
            send_to_char( "Usage: redit exdesc <dir> [description]\n\r", ch );
            return;
        }

        if ( arg2[0] == '#' )
        {
            edir = atoi( arg2 + 1 );
            xit = get_exit_num( location, edir );
        }
        else
        {
            edir = get_dir( arg2 );
            xit = get_exit( location, edir );
        }

        if ( xit )
        {
            STRFREE( xit->description );

            if ( !argument || argument[0] == '\0' )
                xit->description = STRALLOC( "" );
            else
            {
                sprintf( buf, "%s\n\r", argument );
                xit->description = STRALLOC( buf );
            }

            send_to_char( "Done.\n\r", ch );
            return;
        }

        send_to_char( "No exit in that direction.  Use 'redit exit ...' first.\n\r", ch );
        return;
    }

    /*
        Generate usage message.
    */
    if ( ch->substate == SUB_REPEATCMD )
    {
        ch->substate = SUB_RESTRICTED;
        interpret( ch, origarg, FALSE );
        ch->substate = SUB_REPEATCMD;
        ch->last_cmd = do_redit;
    }
    else
        do_redit( ch, "" );

    return;
}

void do_ocreate( CHAR_DATA* ch, char* argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA*  pObjIndex;
    OBJ_DATA*        obj;
    int          vnum, cvnum;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mobiles cannot create.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );
    vnum = is_number( arg ) ? atoi( arg ) : -1;

    if ( vnum == -1 || !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage: ocreate <vnum> [copy vnum] <item name>\n\r", ch );
        return;
    }

    if ( vnum < 1 || vnum > ( MAX_VNUMS - 1 ) )
    {
        send_to_char( "Bad number.\n\r", ch );
        return;
    }

    one_argument( argument, arg2 );
    cvnum = atoi( arg2 );

    if ( cvnum != 0 )
        argument = one_argument( argument, arg2 );

    if ( cvnum < 1 )
        cvnum = 0;

    if ( get_obj_index( vnum ) )
    {
        send_to_char( "An object with that number already exists.\n\r", ch );
        return;
    }

    if ( IS_NPC( ch ) )
        return;

    if ( get_trust( ch ) <= LEVEL_IMMORTAL )
    {
        AREA_DATA* pArea;

        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            send_to_char( "You must have an assigned area to create objects.\n\r", ch );
            return;
        }

        if ( vnum < pArea->low_o_vnum
                ||   vnum > pArea->hi_o_vnum )
        {
            send_to_char( "That number is not in your allocated range.\n\r", ch );
            return;
        }
    }

    pObjIndex = make_object( vnum, cvnum, argument );

    if ( !pObjIndex )
    {
        send_to_char( "Error.\n\r", ch );
        log_string( "do_ocreate: make_object failed." );
        return;
    }

    obj = create_object( pObjIndex, get_trust( ch ) );
    obj_to_char( obj, ch );
    act( AT_IMMORT, "$n makes some ancient arcane gestures, and opens $s hands to reveal $p!", ch, obj, NULL, TO_ROOM );
    act( AT_IMMORT, "You make some ancient arcane gestures, and open your hands to reveal $p!", ch, obj, NULL, TO_CHAR );
}

void do_mcreate( CHAR_DATA* ch, char* argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA*  pMobIndex;
    CHAR_DATA*       mob;
    int          vnum, cvnum;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mobiles cannot create.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );
    vnum = is_number( arg ) ? atoi( arg ) : -1;

    if ( vnum == -1 || !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage: mcreate <vnum> [cvnum] <mobile name>\n\r", ch );
        return;
    }

    if ( vnum < 1 || vnum > ( MAX_VNUMS - 1 ) )
    {
        send_to_char( "Bad number.\n\r", ch );
        return;
    }

    one_argument( argument, arg2 );
    cvnum = atoi( arg2 );

    if ( cvnum != 0 )
        argument = one_argument( argument, arg2 );

    if ( cvnum < 1 )
        cvnum = 0;

    if ( get_mob_index( vnum ) )
    {
        send_to_char( "A mobile with that number already exists.\n\r", ch );
        return;
    }

    if ( IS_NPC( ch ) )
        return;

    if ( get_trust( ch ) <= LEVEL_IMMORTAL )
    {
        AREA_DATA* pArea;

        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            send_to_char( "You must have an assigned area to create mobiles.\n\r", ch );
            return;
        }

        if ( vnum < pArea->low_m_vnum
                ||   vnum > pArea->hi_m_vnum )
        {
            send_to_char( "That number is not in your allocated range.\n\r", ch );
            return;
        }
    }

    pMobIndex = make_mobile( vnum, cvnum, argument );

    if ( !pMobIndex )
    {
        send_to_char( "Error.\n\r", ch );
        log_string( "do_mcreate: make_mobile failed." );
        return;
    }

    mob = create_mobile( pMobIndex );
    char_to_room( mob, ch->in_room );
    act( AT_IMMORT, "$n waves $s arms about, and $N appears at $s command!", ch, NULL, mob, TO_ROOM );
    act( AT_IMMORT, "You wave your arms about, and $N appears at your command!", ch, NULL, mob, TO_CHAR );
}


/*
    Simple but nice and handle line editor.          -Thoric
*/
void edit_buffer( CHAR_DATA* ch, char* argument )
{
    DESCRIPTOR_DATA* d;
    EDITOR_DATA* edit;
    char cmd[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    sh_int x, line, max_buf_lines;
    bool save;

    if ( ( d = ch->desc ) == NULL )
    {
        send_to_char( "You have no descriptor.\n\r", ch );
        return;
    }

    if ( d->connected != CON_EDITING )
    {
        send_to_char( "You can't do that!\n\r", ch );
        bug( "Edit_buffer: d->connected != CON_EDITING", 0 );
        return;
    }

    if ( ch->substate <= SUB_PAUSE )
    {
        send_to_char( "You can't do that!\n\r", ch );
        bug( "Edit_buffer: illegal ch->substate (%d)", ch->substate );
        d->connected = CON_PLAYING;
        return;
    }

    if ( !ch->editor )
    {
        send_to_char( "You can't do that!\n\r", ch );
        bug( "Edit_buffer: null editor", 0 );
        d->connected = CON_PLAYING;
        return;
    }

    edit = ch->editor;
    save = FALSE;
    max_buf_lines = 40;

    if ( ch->substate == SUB_MPROG_EDIT || ch->substate == SUB_HELP_EDIT )
        max_buf_lines = 80;

    if ( argument[0] == '/' || argument[0] == '\\' )
    {
        one_argument( argument, cmd );

        if ( !str_cmp( cmd + 1, "?" ) )
        {
            send_to_char( "Editing commands\n\r---------------------------------\n\r", ch );
            send_to_char( "/l              list buffer\n\r",    ch );
            send_to_char( "/c              clear buffer\n\r",   ch );
            send_to_char( "/d [line]       delete line\n\r",    ch );
            send_to_char( "/g <line>       goto line\n\r",  ch );
            send_to_char( "/i <line>       insert line\n\r",    ch );
            send_to_char( "/r <old> <new>  global replace\n\r", ch );
            send_to_char( "/a              abort editing\n\r",  ch );
            send_to_char( "/f              format text ( to fit screen )\n\r",  ch );

            if ( get_trust( ch ) > LEVEL_IMMORTAL )
                send_to_char( "/! <command>    execute command (do not use another editing command)\n\r",  ch );

            send_to_char( "/s              save buffer\n\r\n\r> ", ch );
            return;
        }

        if ( !str_cmp( cmd + 1, "c" ) )
        {
            memset( edit, '\0', sizeof( EDITOR_DATA ) );
            edit->numlines = 0;
            edit->on_line   = 0;
            send_to_char( "Buffer cleared.\n\r> ", ch );
            return;
        }

        if ( !str_cmp( cmd + 1, "r" ) )
        {
            char word1[MAX_INPUT_LENGTH];
            char word2[MAX_INPUT_LENGTH];
            char* sptr, *wptr, *lwptr;
            int x, count, wordln, word2ln, lineln;
            sptr = one_argument( argument, word1 );
            sptr = one_argument( sptr, word1 );
            sptr = one_argument( sptr, word2 );

            if ( word1[0] == '\0' || word2[0] == '\0' )
            {
                send_to_char( "Need word to replace, and replacement.\n\r> ", ch );
                return;
            }

            if ( strcmp( word1, word2 ) == 0 )
            {
                send_to_char( "Done.\n\r> ", ch );
                return;
            }

            count = 0;
            wordln = strlen( word1 );
            word2ln = strlen( word2 );
            ch_printf( ch, "Replacing all occurrences of %s with %s...\n\r", word1, word2 );

            for ( x = edit->on_line; x < edit->numlines; x++ )
            {
                lwptr = edit->line[x];

                while ( ( wptr = strstr( lwptr, word1 ) ) != NULL )
                {
                    sptr = lwptr;
                    lwptr = wptr + wordln;
                    sprintf( buf, "%s%s", word2, wptr + wordln );
                    lineln = wptr - edit->line[x] - wordln;
                    ++count;

                    if ( strlen( buf ) + lineln > 79 )
                    {
                        lineln = UMAX( 0, ( 79 - strlen( buf ) ) );
                        buf[lineln] = '\0';
                        break;
                    }
                    else
                        lineln = strlen( buf );

                    buf[lineln] = '\0';
                    strcpy( wptr, buf );
                }
            }

            ch_printf( ch, "Found and replaced %d occurrence(s).\n\r> ", count );
            return;
        }

        if ( !str_cmp( cmd + 1, "f" ) )
        {
            char   temp_buf[5000];
            int    x, ep, old_p, end_mark;
            int    p = 0;

            for ( x = 0; x < edit->numlines; x++ )
            {
                strcpy ( temp_buf + p, edit->line[x] );
                p += strlen( edit->line[x] );
                temp_buf[p] = ' ';
                p++;
            }

            temp_buf[p] = '\0';
            end_mark = p;
            p = 75;
            old_p = 0;
            edit->on_line = 0;
            edit->numlines = 0;

            while ( old_p < end_mark )
            {
                while ( temp_buf[p] != ' ' && p > old_p )
                    p--;

                if ( p == old_p )
                    p += 75;

                if ( p > end_mark )
                    p = end_mark;

                ep = 0;

                for ( x = old_p ; x < p ; x++ )
                {
                    edit->line[edit->on_line][ep] = temp_buf[x];
                    ep++;
                }

                edit->line[edit->on_line][ep] = '\0';
                edit->on_line++;
                edit->numlines++;
                old_p = p + 1 ;
                p += 75;
            }

            send_to_char( "OK.\n\r> ", ch );
            return;
        }

        if ( !str_cmp( cmd + 1, "i" ) )
        {
            if ( edit->numlines >= max_buf_lines )
                send_to_char( "Buffer is full.\n\r> ", ch );
            else
            {
                if ( argument[2] == ' ' )
                    line = atoi( argument + 2 ) - 1;
                else
                    line = edit->on_line;

                if ( line < 0 )
                    line = edit->on_line;

                if ( line < 0 || line > edit->numlines )
                    send_to_char( "Out of range.\n\r> ", ch );
                else
                {
                    for ( x = ++edit->numlines; x > line; x-- )
                        strcpy( edit->line[x], edit->line[x - 1] );

                    strcpy( edit->line[line], "" );
                    send_to_char( "Line inserted.\n\r> ", ch );
                }
            }

            return;
        }

        if ( !str_cmp( cmd + 1, "d" ) )
        {
            if ( edit->numlines == 0 )
                send_to_char( "Buffer is empty.\n\r> ", ch );
            else
            {
                if ( argument[2] == ' ' )
                    line = atoi( argument + 2 ) - 1;
                else
                    line = edit->on_line;

                if ( line < 0 )
                    line = edit->on_line;

                if ( line < 0 || line > edit->numlines )
                    send_to_char( "Out of range.\n\r> ", ch );
                else
                {
                    if ( line == 0 && edit->numlines == 1 )
                    {
                        memset( edit, '\0', sizeof( EDITOR_DATA ) );
                        edit->numlines = 0;
                        edit->on_line   = 0;
                        send_to_char( "Line deleted.\n\r> ", ch );
                        return;
                    }

                    for ( x = line; x < ( edit->numlines - 1 ); x++ )
                        strcpy( edit->line[x], edit->line[x + 1] );

                    strcpy( edit->line[edit->numlines--], "" );

                    if ( edit->on_line > edit->numlines )
                        edit->on_line = edit->numlines;

                    send_to_char( "Line deleted.\n\r> ", ch );
                }
            }

            return;
        }

        if ( !str_cmp( cmd + 1, "g" ) )
        {
            if ( edit->numlines == 0 )
                send_to_char( "Buffer is empty.\n\r> ", ch );
            else
            {
                if ( argument[2] == ' ' )
                    line = atoi( argument + 2 ) - 1;
                else
                {
                    send_to_char( "Goto what line?\n\r> ", ch );
                    return;
                }

                if ( line < 0 )
                    line = edit->on_line;

                if ( line < 0 || line > edit->numlines )
                    send_to_char( "Out of range.\n\r> ", ch );
                else
                {
                    edit->on_line = line;
                    ch_printf( ch, "(On line %d)\n\r> ", line + 1 );
                }
            }

            return;
        }

        if ( !str_cmp( cmd + 1, "l" ) )
        {
            if ( edit->numlines == 0 )
                send_to_char( "Buffer is empty.\n\r> ", ch );
            else
            {
                send_to_char( "------------------\n\r", ch );

                for ( x = 0; x < edit->numlines; x++ )
                    ch_printf( ch, "%2d> %s\n\r", x + 1, edit->line[x] );

                send_to_char( "------------------\n\r> ", ch );
            }

            return;
        }

        if ( !str_cmp( cmd + 1, "a" ) )
        {
            send_to_char( "\n\rAborting... ", ch );
            stop_editing( ch );
            return;
        }

        if ( get_trust( ch ) > LEVEL_IMMORTAL && !str_cmp( cmd + 1, "!" ) )
        {
            DO_FUN* last_cmd;
            int substate = ch->substate;
            last_cmd = ch->last_cmd;
            ch->substate = SUB_RESTRICTED;
            interpret( ch, argument + 3, FALSE );
            ch->substate = substate;
            ch->last_cmd = last_cmd;
            set_char_color( AT_GREEN, ch );
            send_to_char( "\n\r> ", ch );
            return;
        }

        if ( !str_cmp( cmd + 1, "s" ) )
        {
            d->connected = CON_PLAYING;

            if ( !ch->last_cmd )
                return;

            ( *ch->last_cmd ) ( ch, "" );
            return;
        }
    }

    if ( edit->size + strlen( argument ) + 1 >= MAX_STRING_LENGTH - 1 )
        send_to_char( "You buffer is full.\n\r", ch );
    else
    {
        int b_end;
        int bm = 75;
        int bp = 0;
        int ep = 0;
        strcpy( buf, argument );
        b_end = strlen( buf );

        while ( bp < b_end )
        {
            while ( buf[bm] != ' ' && bm > bp )
                bm--;

            if ( bm == bp )
                bm += 75;

            if ( bm > b_end )
                bm = b_end;

            ep = 0;

            while ( bp < bm )
            {
                edit->line[edit->on_line][ep] = buf[bp];
                bp++;
                ep++;
            }

            bm = bp + 75;
            bp ++;
            edit->line[edit->on_line][ep] = '\0';
            edit->on_line++;

            if ( edit->on_line > edit->numlines )
                edit->numlines++;

            if ( edit->numlines > max_buf_lines )
            {
                edit->numlines = max_buf_lines;
                send_to_char( "Buffer full.\n\r", ch );
                save = TRUE;
                break;
            }
        }
    }

    if ( save )
    {
        d->connected = CON_PLAYING;

        if ( !ch->last_cmd )
            return;

        ( *ch->last_cmd ) ( ch, "" );
        return;
    }

    send_to_char( "> ", ch );
}

void free_reset( AREA_DATA* are, RESET_DATA* res )
{
    UNLINK( res, are->first_reset, are->last_reset, next, prev );
    DISPOSE( res );
}

void free_area( AREA_DATA* are )
{
    DISPOSE( are->name );
    DISPOSE( are->filename );

    while ( are->first_reset )
        free_reset( are, are->first_reset );

    DISPOSE( are );
    are = NULL;
}

void assign_area( CHAR_DATA* ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char taf[1024];
    AREA_DATA* tarea, *tmp;
    bool created = FALSE;

    if ( IS_NPC( ch ) )
        return;

    if ( get_trust( ch ) >= LEVEL_AVATAR
            &&   ch->pcdata->r_range_lo
            &&   ch->pcdata->r_range_hi )
    {
        tarea = ch->pcdata->area;
        sprintf( taf, "%s.are", capitalize( ch->name ) );

        if ( !tarea )
        {
            for ( tmp = first_build; tmp; tmp = tmp->next )
                if ( !str_cmp( taf, tmp->filename ) )
                {
                    if ( get_trust( ch ) >= LEVEL_GREATER
                            ||   is_name( tmp->filename, ch->pcdata->bestowments ) )
                    {
                        tarea = tmp;
                        break;
                    }
                    else
                    {
                        send_to_char( "You do not have permission to use that area.\n\r", ch );
                        return;
                    }
                }
        }

        if ( !tarea )
        {
            sprintf( buf, "Creating area entry for %s", ch->name );
            log_string_plus( buf, LOG_NORMAL, ch->top_level );
            CREATE( tarea, AREA_DATA, 1 );
            LINK( tarea, first_build, last_build, next, prev );
            tarea->first_reset  = NULL;
            tarea->last_reset   = NULL;
            sprintf( buf, "{PROTO} %s's area in progress", ch->name );
            tarea->name     = str_dup( buf );
            tarea->filename = str_dup( taf );
            sprintf( buf2, "%s", ch->name );
            tarea->author   = STRALLOC( buf2 );
            tarea->age      = 0;
            tarea->nplayer  = 0;
            tarea->ambience     = 0;
            created = TRUE;
        }
        else
        {
            sprintf( buf, "Updating area entry for %s", ch->name );
            log_string_plus( buf, LOG_NORMAL, ch->top_level );
        }

        tarea->low_r_vnum = ch->pcdata->r_range_lo;
        tarea->low_o_vnum = ch->pcdata->o_range_lo;
        tarea->low_m_vnum = ch->pcdata->m_range_lo;
        tarea->hi_r_vnum  = ch->pcdata->r_range_hi;
        tarea->hi_o_vnum  = ch->pcdata->o_range_hi;
        tarea->hi_m_vnum  = ch->pcdata->m_range_hi;
        ch->pcdata->area  = tarea;

        if ( created )
            sort_area( tarea, TRUE );
    }
}

void do_aassign( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    AREA_DATA* tarea, *tmp;

    if ( IS_NPC( ch ) )
        return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: aassign <filename.are>\n\r", ch );
        return;
    }

    if ( !str_cmp( "none", argument )
            ||   !str_cmp( "null", argument )
            ||   !str_cmp( "clear", argument ) )
    {
        ch->pcdata->area = NULL;
        assign_area( ch );

        if ( !ch->pcdata->area )
            send_to_char( "Area pointer cleared.\n\r", ch );
        else
            send_to_char( "Originally assigned area restored.\n\r", ch );

        return;
    }

    sprintf( buf, "%s", argument );
    tarea = NULL;

    /*  if ( get_trust(ch) >= sysdata.level_modify_proto )   */

    if ( get_trust( ch ) >= 105
            ||  ( is_name( buf, ch->pcdata->bestowments )
                  &&   get_trust( ch ) >= sysdata.level_modify_proto ) )
        for ( tmp = first_area; tmp; tmp = tmp->next )
            if ( !str_cmp( buf, tmp->filename ) )
            {
                tarea = tmp;
                break;
            }

    if ( !tarea )
        for ( tmp = first_build; tmp; tmp = tmp->next )
            if ( !str_cmp( buf, tmp->filename ) )
            {
                /*      if ( get_trust(ch) >= sysdata.level_modify_proto  */
                if ( get_trust( ch ) >= 105
                        ||   is_name( tmp->filename, ch->pcdata->bestowments ) )
                {
                    tarea = tmp;
                    break;
                }
                else
                {
                    send_to_char( "You do not have permission to use that area.\n\r", ch );
                    return;
                }
            }

    if ( !tarea )
    {
        if ( get_trust( ch ) >= sysdata.level_modify_proto )
            send_to_char( "No such area.  Use 'zones'.\n\r", ch );
        else
            send_to_char( "No such area.  Use 'newzones'.\n\r", ch );

        return;
    }

    ch->pcdata->area = tarea;
    ch_printf( ch, "Assigning you: %s\n\r", tarea->name );
    return;
}


EXTRA_DESCR_DATA* SetRExtra( ROOM_INDEX_DATA* room, char* keywords )
{
    EXTRA_DESCR_DATA* ed;

    for ( ed = room->first_extradesc; ed; ed = ed->next )
    {
        if ( is_name( keywords, ed->keyword ) )
            break;
    }

    if ( !ed )
    {
        CREATE( ed, EXTRA_DESCR_DATA, 1 );
        LINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
        ed->keyword = STRALLOC( keywords );
        ed->description = STRALLOC( "" );
        top_ed++;
    }

    return ed;
}

bool DelRExtra( ROOM_INDEX_DATA* room, char* keywords )
{
    EXTRA_DESCR_DATA* rmed;

    for ( rmed = room->first_extradesc; rmed; rmed = rmed->next )
    {
        if ( is_name( keywords, rmed->keyword ) )
            break;
    }

    if ( !rmed )
        return FALSE;

    UNLINK( rmed, room->first_extradesc, room->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}

EXTRA_DESCR_DATA* SetOExtra( OBJ_DATA* obj, char* keywords )
{
    EXTRA_DESCR_DATA* ed;

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
    {
        if ( is_name( keywords, ed->keyword ) )
            break;
    }

    if ( !ed )
    {
        CREATE( ed, EXTRA_DESCR_DATA, 1 );
        LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
        ed->keyword = STRALLOC( keywords );
        ed->description = STRALLOC( "" );
        top_ed++;
    }

    return ed;
}

bool DelOExtra( OBJ_DATA* obj, char* keywords )
{
    EXTRA_DESCR_DATA* rmed;

    for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
    {
        if ( is_name( keywords, rmed->keyword ) )
            break;
    }

    if ( !rmed )
        return FALSE;

    UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}

EXTRA_DESCR_DATA* SetOExtraProto( OBJ_INDEX_DATA* obj, char* keywords )
{
    EXTRA_DESCR_DATA* ed = NULL;

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
    {
        if ( is_name( keywords, ed->keyword ) )
            break;
    }

    if ( !ed )
    {
        CREATE( ed, EXTRA_DESCR_DATA, 1 );
        LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
        ed->keyword = STRALLOC( keywords );
        ed->description = STRALLOC( "" );
        top_ed++;
    }

    return ed;
}

bool DelOExtraProto( OBJ_INDEX_DATA* obj, char* keywords )
{
    EXTRA_DESCR_DATA* rmed;

    for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
    {
        if ( is_name( keywords, rmed->keyword ) )
            break;
    }

    if ( !rmed )
        return FALSE;

    UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}

/*
    Redesigned to Support AVP's new format.
    <-> Ghost <->
*/
void fold_area( AREA_DATA* tarea, char* filename, bool install )
{
    RESET_DATA*          treset = NULL;
    ROOM_INDEX_DATA*     room = NULL;
    MOB_INDEX_DATA*      pMobIndex = NULL;
    OBJ_INDEX_DATA*      pObjIndex = NULL;
    MPROG_DATA*          mprog = NULL;
    EXIT_DATA*           xit = NULL;
    EXTRA_DESCR_DATA*    ed = NULL;
    AFFECT_DATA*         paf = NULL;
    SHOP_DATA*           pShop = NULL;
    FILE*                fpout = NULL;
    char         buf[MAX_STRING_LENGTH];
    int          vnum;
    int          val0, val1, val2, val3, val4, val5;
    bool         complexmob;
    sprintf( buf, "Saving %s...", tarea->filename );
    log_string_plus( buf, LOG_NORMAL, LEVEL_GREATER );
    sprintf( buf, "%s.bak", filename );
    rename( filename, buf );
    fclose( fpReserve );

    if ( ( fpout = fopen( filename, "w" ) ) == NULL )
    {
        bug( "fold_area: fopen", 0 );
        perror( filename );
        fpReserve = fopen( NULL_FILE, "r" );
        return;
    }

    tarea->version = AREA_VERSION;
    fprintf( fpout, "#AREA   %s~\n\n", tarea->name );
    fprintf( fpout, "#VERSION %d\n\n", AREA_VERSION );
    fprintf( fpout, "#AUTHOR %s~\n\n", tarea->author );
    fprintf( fpout, "#AMBIENCE %d\n\r", tarea->ambience );
    fprintf( fpout, "#RANGES\n" );
    fprintf( fpout, "%d %d %d %d\n", tarea->low_soft_range, tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range );
    fprintf( fpout, "$\n\n" );

    if ( tarea->resetmsg )  /* Rennard */
        fprintf( fpout, "#RESETMSG %s~\n\n", tarea->resetmsg );

    /*
        +-------------------------------+
        |       + Save Mobiles +        |
        +-------------------------------+
    */
    fprintf( fpout, "#MOBILES\n" );

    for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
            continue;

        if ( install )
            xREMOVE_BIT( pMobIndex->act, ACT_PROTOTYPE );

        if ( pMobIndex->perm_str != 10      ||   pMobIndex->perm_sta   != 10
                ||   pMobIndex->perm_int != 10      ||   pMobIndex->perm_rec   != 10
                ||   pMobIndex->perm_bra != 10      ||   pMobIndex->perm_per   != 10
                ||   pMobIndex->race     != 0
                ||   pMobIndex->height   != 0       ||   pMobIndex->weight     != 0
                ||   !xIS_EMPTY( pMobIndex->speaks )  ||  !xIS_EMPTY( pMobIndex->speaking ) )
            complexmob = TRUE;
        else
            complexmob = FALSE;

        fprintf( fpout, "#%d\n",       vnum                );
        fprintf( fpout, "%s~\n",       pMobIndex->player_name      );
        fprintf( fpout, "%s~\n",       pMobIndex->short_descr      );
        fprintf( fpout, "%s~\n",       strip_cr( pMobIndex->long_descr ) );
        fprintf( fpout, "%s~\n",       strip_cr( pMobIndex->description ) );
        fprintf( fpout, "%s\n",        print_bitvector( &pMobIndex->affected_by ) );
        fprintf( fpout, "%s\n",        print_bitvector( &pMobIndex->act ) );
        fprintf( fpout, "%c\n",        complexmob ? 'Z' : 'S' );
        /* C changed to Z for swreality vip_flags  */
        fprintf( fpout, "%d ",   pMobIndex->level );
        fprintf( fpout, "%d %d %d\n",  pMobIndex->position, pMobIndex->defposition, pMobIndex->sex          );

        if ( complexmob )
        {
            fprintf( fpout, "%d %d %d %d %d %d\n", pMobIndex->perm_str, pMobIndex->perm_sta, pMobIndex->perm_rec, pMobIndex->perm_int, pMobIndex->perm_bra, pMobIndex->perm_per );
            fprintf( fpout, "%d 0 %d %d\n", pMobIndex->race, pMobIndex->height, pMobIndex->weight );
            fprintf( fpout, "%s\n",     print_bitvector( &pMobIndex->speaks ) );
            fprintf( fpout, "%s\n",     print_bitvector( &pMobIndex->speaking ) );
            fprintf( fpout, "0 0 0 0 0 0 0\n" );
        }

        if ( pMobIndex->mudprogs )
        {
            for ( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
                fprintf( fpout, "> %s %s~\n%s~\n",
                         mprog_type_to_name( mprog->type ),
                         mprog->arglist, strip_cr( mprog->comlist ) );

            fprintf( fpout, "|\n" );
        }
    }

    fprintf( fpout, "#0\n\n\n" );

    if ( install && vnum < tarea->hi_m_vnum )
        tarea->hi_m_vnum = vnum - 1;

    /*
        +-------------------------------+
        |       + Save Objects +        |
        +-------------------------------+
    */
    fprintf( fpout, "#OBJECTS\n" );

    for ( vnum = tarea->low_o_vnum; vnum <= tarea->hi_o_vnum; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
            continue;

        if ( install )
            xREMOVE_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );

        fprintf( fpout, "#%d\n",    vnum                );
        fprintf( fpout, "%s~\n",    pObjIndex->name         );
        fprintf( fpout, "%s~\n",    pObjIndex->short_descr      );
        fprintf( fpout, "%s~\n",    pObjIndex->description      );
        fprintf( fpout, "%s~\n",    pObjIndex->action_desc      );

        if ( pObjIndex->layers )
            fprintf( fpout, "%d %d\n",  pObjIndex->item_type, pObjIndex->layers );
        else
            fprintf( fpout, "%d\n", pObjIndex->item_type );

        fprintf( fpout, "%s\n", print_bitvector( &pObjIndex->extra_flags ) );
        fprintf( fpout, "%s\n", print_bitvector( &pObjIndex->wear_flags ) );
        val0 = pObjIndex->value[0];
        val1 = pObjIndex->value[1];
        val2 = pObjIndex->value[2];
        val3 = pObjIndex->value[3];
        val4 = pObjIndex->value[4];
        val5 = pObjIndex->value[5];
        fprintf( fpout, "%d %d %d %d %d %d\n", val0, val1, val2, val3, val4, val5 );
        fprintf( fpout, "%d %d %d\n",   pObjIndex->weight,
                 pObjIndex->cost,
                 pObjIndex->rent ? pObjIndex->rent :
                 ( int ) ( pObjIndex->cost / 10 )       );

        for ( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
            fprintf( fpout, "E\n%s~\n%s~\n",
                     ed->keyword, strip_cr( ed->description )    );

        for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
            fprintf( fpout, "A\n%d %d\n", paf->location, paf->modifier );

        if ( pObjIndex->mudprogs )
        {
            for ( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
                fprintf( fpout, "> %s %s~\n%s~\n",
                         mprog_type_to_name( mprog->type ),
                         mprog->arglist, strip_cr( mprog->comlist ) );

            fprintf( fpout, "|\n" );
        }
    }

    fprintf( fpout, "#0\n\n\n" );

    if ( install && vnum < tarea->hi_o_vnum )
        tarea->hi_o_vnum = vnum - 1;

    /*
        +-------------------------------+
        |        + Save Rooms +         |
        +-------------------------------+
    */
    fprintf( fpout, "#ROOMS\n" );

    for ( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
    {
        if ( ( room = get_room_index( vnum ) ) == NULL )
            continue;

        if ( install )
        {
            CHAR_DATA* victim, *vnext;
            OBJ_DATA*  obj, *obj_next;
            /* remove prototype flag from room */
            xREMOVE_BIT( room->room_flags, ROOM_PROTOTYPE );

            /* purge room of (prototyped) mobiles */
            for ( victim = room->first_person; victim; victim = vnext )
            {
                vnext = victim->next_in_room;

                if ( IS_NPC( victim ) )
                    extract_char( victim, TRUE, FALSE );
            }

            /* purge room of (prototyped) objects */
            for ( obj = room->first_content; obj; obj = obj_next )
            {
                obj_next = obj->next_content;
                extract_obj( obj );
            }
        }

        fprintf( fpout, "#%d\n",    vnum                );
        fprintf( fpout, "%s~\n",    room->name          );
        fprintf( fpout, "%s~\n",    strip_cr( room->description )   );
        fprintf( fpout, "%s~\n",        strip_cr( room->hdescription )   );
        fprintf( fpout, "%s\n", print_bitvector( &room->room_flags ) );
        fprintf( fpout, "%s\n", print_bitvector( &room->hroom_flags ) );

        if ( ( room->tele_delay > 0 && room->tele_vnum > 0 ) || room->tunnel > 0 )
            fprintf( fpout, "0 %d %d %d %d\n", room->sector_type, room->tele_delay, room->tele_vnum, room->tunnel );
        else
            fprintf( fpout, "0 %d\n", room->sector_type   );

        for ( xit = room->first_exit; xit; xit = xit->next )
        {
            if ( xIS_SET( xit->exit_info, EX_PORTAL ) ) /* don't fold portals */
                continue;

            fprintf( fpout, "D%d\n",     xit->vdir );
            fprintf( fpout, "%s~\n",     strip_cr( xit->description ) );
            fprintf( fpout, "%s~\n",     strip_cr( xit->keyword ) );
            fprintf( fpout, "%s\n",     print_bitvector( &xit->exit_info ) );

            if ( xit->distance > 1 )
                fprintf( fpout, "%d %d %d\n", xit->key, xit->vnum, xit->distance );
            else
                fprintf( fpout, "%d %d\n", xit->key, xit->vnum );
        }

        for ( ed = room->first_extradesc; ed; ed = ed->next )
            fprintf( fpout, "E\n%s~\n%s~\n",
                     ed->keyword, strip_cr( ed->description ) );

        if ( room->mudprogs )
        {
            for ( mprog = room->mudprogs; mprog; mprog = mprog->next )
                fprintf( fpout, "> %s %s~\n%s~\n",
                         mprog_type_to_name( mprog->type ),
                         mprog->arglist, strip_cr( mprog->comlist ) );

            fprintf( fpout, "|\n" );
        }

        fprintf( fpout, "S\n" );
    }

    fprintf( fpout, "#0\n\n\n" );

    if ( install && vnum < tarea->hi_r_vnum )
        tarea->hi_r_vnum = vnum - 1;

    /* save resets   */
    fprintf( fpout, "#RESETS\n" );

    for ( treset = tarea->first_reset; treset; treset = treset->next )
    {
        switch ( treset->command ) /* extra arg1 arg2 arg3 */
        {
            default:
            case '*':
                break;

            case 'm':
            case 'M':
            case 'o':
            case 'O':
            case 'p':
            case 'P':
            case 'e':
            case 'E':
            case 'd':
            case 'D':
            case 't':
            case 'T':
                fprintf( fpout, "%c %d %d %d %d\n", UPPER( treset->command ),
                         treset->extra, treset->arg1, treset->arg2, treset->arg3 );
                break;

            case 'g':
            case 'G':
            case 'r':
            case 'R':
                fprintf( fpout, "%c %d %d %d\n", UPPER( treset->command ),
                         treset->extra, treset->arg1, treset->arg2 );
                break;
        }
    }

    fprintf( fpout, "S\n\n\n" );
    /* save shops */
    fprintf( fpout, "#SHOPS\n" );

    for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
            continue;

        if ( ( pShop = pMobIndex->pShop ) == NULL )
            continue;

        fprintf( fpout, "%d", pShop->keeper );
        fprintf( fpout, "; %s\n", pMobIndex->short_descr );
    }

    fprintf( fpout, "0\n\n\n" );
    /* save specials */
    fprintf( fpout, "#SPECIALS\n" );

    for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
            continue;

        if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
            continue;

        if ( pMobIndex->spec_fun )
            fprintf( fpout, "M  %d %s\n", pMobIndex->vnum, lookup_spec( pMobIndex->spec_fun ) );

        if ( pMobIndex->spec_2 )
            fprintf( fpout, "M  %d %s\n", pMobIndex->vnum, lookup_spec( pMobIndex->spec_2 ) );

        if ( pMobIndex->spec_3 )
            fprintf( fpout, "M  %d %s\n", pMobIndex->vnum, lookup_spec( pMobIndex->spec_3 ) );

        if ( pMobIndex->spec_4 )
            fprintf( fpout, "M  %d %s\n", pMobIndex->vnum, lookup_spec( pMobIndex->spec_4 ) );
    }

    fprintf( fpout, "S\n\n\n" );
    /* END */
    fprintf( fpout, "#$\n" );
    fclose( fpout );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_savearea( CHAR_DATA* ch, char* argument )
{
    AREA_DATA*   tarea;
    char     filename[256];

    if ( IS_NPC( ch ) || get_trust( ch ) < LEVEL_AVATAR || !ch->pcdata
            ||  ( argument[0] == '\0' && !ch->pcdata->area ) )
    {
        send_to_char( "You don't have an assigned area to save.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
        tarea = ch->pcdata->area;
    else
    {
        bool found;

        if ( get_trust( ch ) < LEVEL_GOD )
        {
            send_to_char( "You can only save your own area.\n\r", ch );
            return;
        }

        for ( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
            if ( !str_cmp( tarea->filename, argument ) )
            {
                found = TRUE;
                break;
            }

        if ( !found )
        {
            send_to_char( "Area not found.\n\r", ch );
            return;
        }
    }

    if ( !tarea )
    {
        send_to_char( "No area to save.\n\r", ch );
        return;
    }

    /* Ensure not wiping out their area with save before load - Scryn 8/11 */
    if ( !IS_SET( tarea->status, AREA_LOADED ) )
    {
        send_to_char( "Your area is not loaded!\n\r", ch );
        return;
    }

    sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
    fold_area( tarea, filename, FALSE );
    send_to_char( "Done.\n\r", ch );
}

void do_loadarea( CHAR_DATA* ch, char* argument )
{
    AREA_DATA*   tarea;
    char     filename[256];
    int     tmp;

    if ( IS_NPC( ch ) || get_trust( ch ) < LEVEL_AVATAR || !ch->pcdata
            ||  ( argument[0] == '\0' && !ch->pcdata->area ) )
    {
        send_to_char( "You don't have an assigned area to load.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
        tarea = ch->pcdata->area;
    else
    {
        bool found;

        if ( get_trust( ch ) < LEVEL_GOD )
        {
            send_to_char( "You can only load your own area.\n\r", ch );
            return;
        }

        for ( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
            if ( !str_cmp( tarea->filename, argument ) )
            {
                found = TRUE;
                break;
            }

        if ( !found )
        {
            send_to_char( "Area not found.\n\r", ch );
            return;
        }
    }

    if ( !tarea )
    {
        send_to_char( "No area to load.\n\r", ch );
        return;
    }

    /* Stops char from loading when already loaded - Scryn 8/11 */
    if ( IS_SET ( tarea->status, AREA_LOADED ) )
    {
        send_to_char( "Your area is already loaded.\n\r", ch );
        return;
    }

    sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
    send_to_char( "&z(&YL&z)oading...\n\r", ch );
    load_area_file( tarea, filename );
    send_to_char( "&z(&YL&z)inking exits...\n\r", ch );
    fix_area_exits( tarea );

    if ( tarea->first_reset )
    {
        tmp = tarea->nplayer;
        tarea->nplayer = 0;
        send_to_char( "&z(&YR&z)esetting area...\n\r", ch );
        reset_area( tarea );
        tarea->nplayer = tmp;
    }

    send_to_char( "&z(&YD&z)one.\n\r", ch );
}

/*
    Dangerous command.  Can be used to install an area that was either:
     (a) already installed but removed from area.lst
     (b) designed offline
    The mud will likely crash if:
     (a) this area is already loaded
     (b) it contains vnums that exist
     (c) the area has errors

    NOTE: Use of this command is not recommended.        -Thoric
*/
void do_unfoldarea( CHAR_DATA* ch, char* argument )
{
    if ( !argument || argument[0] == '\0' )
    {
        send_to_char( "Unfold what?\n\r", ch );
        return;
    }

    fBootDb = TRUE;
    load_area_file( last_area, argument );
    fBootDb = FALSE;
    return;
}


void do_foldarea( CHAR_DATA* ch, char* argument )
{
    AREA_DATA*   tarea;
    char         arg[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Usage: foldarea <filename> [remproto]\n\r", ch );
        return;
    }

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
        if ( !str_cmp( tarea->filename, arg ) )
        {
            send_to_char( "Folding...\n\r", ch );

            if ( !strcmp( argument, "remproto" ) )
                fold_area( tarea, tarea->filename, TRUE );
            else
                fold_area( tarea, tarea->filename, FALSE );

            send_to_char( "Done.\n\r", ch );
            return;
        }
    }

    send_to_char( "No such area exists.\n\r", ch );
    return;
}

extern int top_area;

void write_area_list( )
{
    AREA_DATA* tarea;
    FILE* fpout;
    fpout = fopen( AREA_LIST, "w" );

    if ( !fpout )
    {
        bug( "FATAL: cannot open area.lst for writing!\n\r", 0 );
        return;
    }

    fprintf( fpout, "help.are\n" );

    for ( tarea = first_area; tarea; tarea = tarea->next )
        fprintf( fpout, "%s\n", tarea->filename );

    fprintf( fpout, "$\n" );
    fclose( fpout );
}

/*
    A complicated to use command as it currently exists.     -Thoric
    Once area->author and area->name are cleaned up... it will be easier
*/
void do_installarea( CHAR_DATA* ch, char* argument )
{
    AREA_DATA*   tarea;
    char    arg[MAX_INPUT_LENGTH];
    char    buf[MAX_STRING_LENGTH];
    int     num;
    DESCRIPTOR_DATA* d;
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: installarea <filename> [Area title]\n\r", ch );
        return;
    }

    for ( tarea = first_build; tarea; tarea = tarea->next )
    {
        if ( !str_cmp( tarea->filename, arg ) )
        {
            if ( argument && argument[0] != '\0' )
            {
                DISPOSE( tarea->name );
                tarea->name = str_dup( argument );
            }

            /* Fold area with install flag -- auto-removes prototype flags */
            send_to_char( "Saving and installing file...\n\r", ch );
            fold_area( tarea, tarea->filename, TRUE );
            /* Remove from prototype area list */
            UNLINK( tarea, first_build, last_build, next, prev );
            /* Add to real area list */
            LINK( tarea, first_area, last_area, next, prev );

            /* Fix up author if online */
            for ( d = first_descriptor; d; d = d->next )
                if ( d->character
                        &&   d->character->pcdata
                        &&   d->character->pcdata->area == tarea )
                {
                    /* remove area from author */
                    d->character->pcdata->area = NULL;
                    /* clear out author vnums  */
                    d->character->pcdata->r_range_lo = 0;
                    d->character->pcdata->r_range_hi = 0;
                    d->character->pcdata->o_range_lo = 0;
                    d->character->pcdata->o_range_hi = 0;
                    d->character->pcdata->m_range_lo = 0;
                    d->character->pcdata->m_range_hi = 0;
                }

            top_area++;
            send_to_char( "Writing area.lst...\n\r", ch );
            write_area_list( );
            send_to_char( "Resetting new area.\n\r", ch );
            num = tarea->nplayer;
            tarea->nplayer = 0;
            reset_area( tarea );
            tarea->nplayer = num;
            send_to_char( "Renaming author's building file.\n\r", ch );
            sprintf( buf, "%s%s.installed", BUILD_DIR, tarea->filename );
            sprintf( arg, "%s%s", BUILD_DIR, tarea->filename );
            rename( arg, buf );
            send_to_char( "Done.\n\r", ch );
            return;
        }
    }

    send_to_char( "No such area exists.\n\r", ch );
    return;
}

void add_reset_nested( AREA_DATA* tarea, OBJ_DATA* obj )
{
    int limit;

    for ( obj = obj->first_content; obj; obj = obj->next_content )
    {
        limit = obj->pIndexData->count;

        if ( limit < 1 )
            limit = 1;

        add_reset( tarea, 'P', 1, obj->pIndexData->vnum, limit,
                   obj->in_obj->pIndexData->vnum );

        if ( obj->first_content )
            add_reset_nested( tarea, obj );
    }
}


/*
    Parse a reset command string into a reset_data structure
*/
RESET_DATA* parse_reset( AREA_DATA* tarea, char* argument, CHAR_DATA* ch )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char letter;
    int extra;
    int val1, val2, val3;
    int value;
    ROOM_INDEX_DATA* room;
    EXIT_DATA*   pexit;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    extra = 0;
    letter = '*';
    val1 = atoi( arg2 );
    val2 = atoi( arg3 );
    val3 = atoi( arg4 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Reset commands: mob obj give equip door rand trap hide.\n\r", ch );
        return NULL;
    }

    if ( !str_cmp( arg1, "hide" ) )
    {
        if ( arg2[0] != '\0' && !get_obj_index( val1 ) )
        {
            send_to_char( "Reset: HIDE: no such object\n\r", ch );
            return NULL;
        }
        else
            val1 = 0;

        extra = 1;
        val2 = 0;
        val3 = 0;
        letter = 'H';
    }
    else if ( arg2[0] == '\0' )
    {
        send_to_char( "Reset: not enough arguments.\n\r", ch );
        return NULL;
    }
    else if ( val1 < 1 || val1 > MAX_VNUMS )
    {
        send_to_char( "Reset: value out of range.\n\r", ch );
        return NULL;
    }
    else if ( !str_cmp( arg1, "mob" ) )
    {
        if ( !get_mob_index( val1 ) )
        {
            send_to_char( "Reset: MOB: no such mobile\n\r", ch );
            return NULL;
        }

        if ( !get_room_index( val2 ) )
        {
            send_to_char( "Reset: MOB: no such room\n\r", ch );
            return NULL;
        }

        if ( val3 < 1 )
            val3 = 1;

        letter = 'M';
    }
    else if ( !str_cmp( arg1, "obj" ) )
    {
        if ( !get_obj_index( val1 ) )
        {
            send_to_char( "Reset: OBJ: no such object\n\r", ch );
            return NULL;
        }

        if ( !get_room_index( val2 ) )
        {
            send_to_char( "Reset: OBJ: no such room\n\r", ch );
            return NULL;
        }

        if ( val3 < 1 )
            val3 = 1;

        letter = 'O';
    }
    else if ( !str_cmp( arg1, "give" ) )
    {
        if ( !get_obj_index( val1 ) )
        {
            send_to_char( "Reset: GIVE: no such object\n\r", ch );
            return NULL;
        }

        if ( val2 < 1 )
            val2 = 1;

        val3 = val2;
        val2 = 0;
        extra = 1;
        letter = 'G';
    }
    else if ( !str_cmp( arg1, "equip" ) )
    {
        if ( !get_obj_index( val1 ) )
        {
            send_to_char( "Reset: EQUIP: no such object\n\r", ch );
            return NULL;
        }

        if ( !is_number( arg3 ) )
            val2 = get_wearloc( arg3 );

        if ( val2 < 0 || val2 >= MAX_WEAR )
        {
            send_to_char( "Reset: EQUIP: invalid wear location\n\r", ch );
            return NULL;
        }

        if ( val3 < 1 )
            val3 = 1;

        extra  = 1;
        letter = 'E';
    }
    else if ( !str_cmp( arg1, "put" ) )
    {
        if ( !get_obj_index( val1 ) )
        {
            send_to_char( "Reset: PUT: no such object\n\r", ch );
            return NULL;
        }

        if ( val2 > 0 && !get_obj_index( val2 ) )
        {
            send_to_char( "Reset: PUT: no such container\n\r", ch );
            return NULL;
        }

        extra = UMAX( val3, 0 );
        argument = one_argument( argument, arg4 );
        val3 = ( is_number( argument ) ? atoi( arg4 ) : 0 );

        if ( val3 < 0 )
            val3 = 0;

        letter = 'P';
    }
    else if ( !str_cmp( arg1, "door" ) )
    {
        if ( ( room = get_room_index( val1 ) ) == NULL )
        {
            send_to_char( "Reset: DOOR: no such room\n\r", ch );
            return NULL;
        }

        if ( val2 < 0 || val2 > 9 )
        {
            send_to_char( "Reset: DOOR: invalid exit\n\r", ch );
            return NULL;
        }

        if ( ( pexit = get_exit( room, val2 ) ) == NULL
                ||   !xIS_SET( pexit->exit_info, EX_ISDOOR ) )
        {
            send_to_char( "Reset: DOOR: no such door\n\r", ch );
            return NULL;
        }

        if ( val3 < 0 || val3 > 2 )
        {
            send_to_char( "Reset: DOOR: invalid door state (0 = open, 1 = close, 2 = lock)\n\r", ch );
            return NULL;
        }

        letter = 'D';
        value = val3;
        val3  = val2;
        val2  = value;
    }
    else if ( !str_cmp( arg1, "rand" ) )
    {
        if ( !get_room_index( val1 ) )
        {
            send_to_char( "Reset: RAND: no such room\n\r", ch );
            return NULL;
        }

        if ( val2 < 0 || val2 > 9 )
        {
            send_to_char( "Reset: RAND: invalid max exit\n\r", ch );
            return NULL;
        }

        val3 = val2;
        val2 = 0;
        letter = 'R';
    }

    if ( letter == '*' )
        return NULL;
    else
        return make_reset( letter, extra, val1, val3, val2 );
}

void do_astat( CHAR_DATA* ch, char* argument )
{
    ROOM_INDEX_DATA* room;
    AREA_DATA* tarea;
    bool proto, found;
    int vnum, free_vnum = 0, used_vnum = 0, vents = 0;
    found = FALSE;
    proto = FALSE;

    for ( tarea = first_area; tarea; tarea = tarea->next )
        if ( !str_cmp( tarea->filename, argument ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
        for ( tarea = first_build; tarea; tarea = tarea->next )
            if ( !str_cmp( tarea->filename, argument ) )
            {
                found = TRUE;
                proto = TRUE;
                break;
            }

    if ( !found )
    {
        if ( argument && argument[0] != '\0' )
        {
            send_to_char( "Area not found.  Check 'zones'.\n\r", ch );
            return;
        }
        else
        {
            tarea = ch->in_room->area;
        }
    }

    ch_printf( ch, "Name: %s\n\rFilename: %-20s  Prototype: %s\n\r",
               tarea->name,
               tarea->filename,
               proto ? "yes" : "no" );

    if ( !proto )
    {
        ch_printf( ch, "Max players: %d  IllegalPks: %d\n\r", tarea->max_players, tarea->illegal_pk );
        ch_printf( ch, "Mdeaths: %d  Mkills: %d  Pdeaths: %d  Pkills: %d\n\r",
                   tarea->mdeaths,
                   tarea->mkills,
                   tarea->pdeaths,
                   tarea->pkills );
    }

    ch_printf( ch, "Author: %s\n\rAmbience: %d\n\rAge: %d   Number of players: %d\n\r",
               tarea->author,
               tarea->ambience,
               tarea->age,
               tarea->nplayer );
    ch_printf( ch, "Area flags: %s\n\r", flag_string( &tarea->flags, area_flags, MAX_AREA_FLAG ) );
    ch_printf( ch, "low_room: %5d  hi_room: %d\n\r",
               tarea->low_r_vnum,
               tarea->hi_r_vnum );
    ch_printf( ch, "low_obj : %5d  hi_obj : %d\n\r",
               tarea->low_o_vnum,
               tarea->hi_o_vnum );
    ch_printf( ch, "low_mob : %5d  hi_mob : %d\n\r",
               tarea->low_m_vnum,
               tarea->hi_m_vnum );
    ch_printf( ch, "soft range: %d - %d.  hard range: %d - %d.\n\r",
               tarea->low_soft_range,
               tarea->hi_soft_range,
               tarea->low_hard_range,
               tarea->hi_hard_range );
    ch_printf( ch, "Resetmsg: %s\n\r", tarea->resetmsg ? tarea->resetmsg
               : "(default)" ); /* Rennard */
    ch_printf( ch, "Reset frequency: %d minutes.\n\r",
               tarea->reset_frequency ? tarea->reset_frequency : 15 );

    for ( vnum = tarea->low_r_vnum ; vnum <= tarea->hi_r_vnum ; vnum++ )
    {
        if ( ( room = get_room_index( vnum ) ) == NULL )
        {
            free_vnum++;
        }
        else
        {
            if ( xIS_SET( room->room_flags, ROOM_VENTED_A ) )
                vents++;
            else if ( xIS_SET( room->room_flags, ROOM_VENTED_B ) )
                vents++;
            else if ( xIS_SET( room->room_flags, ROOM_VENTED_C ) )
                vents++;
            else if ( xIS_SET( room->room_flags, ROOM_VENTED_D ) )
                vents++;

            used_vnum++;
        }
    }

    ch_printf( ch, "Filled Rooms: %d rooms.\n\r", used_vnum );
    ch_printf( ch, "Vented Rooms: %d rooms.\n\r", vents );
    ch_printf( ch, "Unused Rooms: %d rooms.\n\r", free_vnum );
}


void do_aset( CHAR_DATA* ch, char* argument )
{
    AREA_DATA* tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    bool proto, found;
    int vnum, value;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    vnum = atoi( argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Usage: aset <area filename> <field> <value>\n\r", ch );
        send_to_char( "\n\rField being one of:\n\r", ch );
        send_to_char( "  low_room hi_room low_obj hi_obj low_mob hi_mob\n\r", ch );
        send_to_char( "  name filename low_soft hi_soft low_hard hi_hard\n\r", ch );
        send_to_char( "  author resetmsg resetfreq flags ambience\n\r", ch );
        return;
    }

    found = FALSE;
    proto = FALSE;

    for ( tarea = first_area; tarea; tarea = tarea->next )
        if ( !str_cmp( tarea->filename, arg1 ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
        for ( tarea = first_build; tarea; tarea = tarea->next )
            if ( !str_cmp( tarea->filename, arg1 ) )
            {
                found = TRUE;
                proto = TRUE;
                break;
            }

    if ( !found )
    {
        send_to_char( "Area not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        DISPOSE( tarea->name );
        tarea->name = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "filename" ) )
    {
        DISPOSE( tarea->filename );
        tarea->filename = str_dup( argument );
        write_area_list( );
        fold_area( tarea, tarea->filename, TRUE );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_room" ) )
    {
        tarea->low_r_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hi_room" ) )
    {
        tarea->hi_r_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_obj" ) )
    {
        tarea->low_o_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hi_obj" ) )
    {
        tarea->hi_o_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_mob" ) )
    {
        tarea->low_m_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hi_mob" ) )
    {
        tarea->hi_m_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_soft" ) )
    {
        if ( vnum < 0 || vnum > MAX_LEVEL )
        {
            send_to_char( "That is not an acceptable value.\n\r", ch );
            return;
        }

        tarea->low_soft_range = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hi_soft" ) )
    {
        if ( vnum < 0 || vnum > MAX_LEVEL )
        {
            send_to_char( "That is not an acceptable value.\n\r", ch );
            return;
        }

        tarea->hi_soft_range = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_hard" ) )
    {
        if ( vnum < 0 || vnum > MAX_LEVEL )
        {
            send_to_char( "That is not an acceptable value.\n\r", ch );
            return;
        }

        tarea->low_hard_range = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hi_hard" ) )
    {
        if ( vnum < 0 || vnum > MAX_LEVEL )
        {
            send_to_char( "That is not an acceptable value.\n\r", ch );
            return;
        }

        tarea->hi_hard_range = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "author" ) )
    {
        STRFREE( tarea->author );
        tarea->author = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "resetmsg" ) )
    {
        if ( tarea->resetmsg )
            DISPOSE( tarea->resetmsg );

        if ( str_cmp( argument, "clear" ) )
            tarea->resetmsg = str_dup( argument );

        send_to_char( "Done.\n\r", ch );
        return;
    } /* Rennard */

    if ( !str_cmp( arg2, "ambience" ) )
    {
        if ( !str_cmp( argument, "low" ) )
            tarea->ambience = 1;
        else if ( !str_cmp( argument, "medium" ) )
            tarea->ambience = 2;
        else if ( !str_cmp( argument, "high" ) )
            tarea->ambience = 3;
        else
        {
            send_to_char( "aset (area name) ambience [low/medium/high]\n\r", ch );
            return;
        }

        if ( tarea->ambience == 1 )
            send_to_char( "Ambience level set to LOW. (YELL Range is 6 rooms)\n\r", ch );

        if ( tarea->ambience == 2 )
            send_to_char( "Ambience level set to MEDIUM. (YELL Range is 4 rooms)\n\r", ch );

        if ( tarea->ambience == 3 )
            send_to_char( "Ambience level set to HIGH. (YELL Range is 2 rooms)\n\r", ch );

        return;
    }

    if ( !str_cmp( arg2, "resetfreq" ) )
    {
        tarea->reset_frequency = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: aset <filename> flags <flag> [flag]...\n\r", ch );
            return;
        }

        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_areaflag( arg3 );

            if ( value < 0 || value >= MAX_AREA_FLAG )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
            {
                if ( xIS_SET( tarea->flags, value ) )
                    xREMOVE_BIT( tarea->flags, value );
                else
                    xSET_BIT( tarea->flags, value );
            }
        }

        return;
    }

    do_aset( ch, "" );
    return;
}

void do_abackup( CHAR_DATA* ch, char* argument )
{
    AREA_DATA* tarea;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    bool proto, found;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( get_trust( ch ) < 200 )
    {
        send_to_char( "&ROnly Administrators are allow to use ABACKUP.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Usage: abackup <area filename> <mode>\n\r", ch );
        send_to_char( "\n\rMode being one of:\n\r", ch );
        send_to_char( "  store recall info\n\r", ch );
        return;
    }

    found = FALSE;
    proto = FALSE;

    for ( tarea = first_area; tarea; tarea = tarea->next )
        if ( !str_cmp( tarea->filename, arg1 ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
        for ( tarea = first_build; tarea; tarea = tarea->next )
            if ( !str_cmp( tarea->filename, arg1 ) )
            {
                found = TRUE;
                proto = TRUE;
                break;
            }

    if ( !found )
    {
        send_to_char( "Area not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "store" ) )
    {
        sprintf( buf, "%s.sto", tarea->filename );
        fold_area( tarea, buf, TRUE );
        send_to_char( "Area backup STORED!\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "recall" ) )
    {
        send_to_char( "&YABACKUP: Loading area...\n\r", ch );
        sprintf( buf, "%s.sto", arg1 );
        load_area_file( tarea, buf );
        send_to_char( "&YArea Backup has been recalled.\n\rPlease FOLDA to finish the process.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "info" ) )
    {
        send_to_char( "&YCurrent status of backup is unknown. Sorry =(\n\r", ch );
        return;
    }

    do_abackup( ch, "" );
    return;
}

/*
    RFILL Command - Created by Ghost (Legends of the Jedi - Head Coder)
*/
void do_rfill( CHAR_DATA* ch, char* argument )
{
    ROOM_INDEX_DATA* room;
    int          vnum;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AREA_DATA*       tarea;
    int lrange;
    int trange;
    int cnt = 0;

    if ( IS_NPC( ch ) || get_trust( ch ) < LEVEL_AVATAR || !ch->pcdata || !ch->pcdata->area )
    {
        send_to_char( "You don't have an assigned area.\n\r", ch );
        return;
    }

    tarea = ch->pcdata->area;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "&zSyntax: RFILL <first vnum> <last vnum> [Optional Room Name]\n\r", ch );
        return;
    }

    if ( !is_number( arg1 ) || !is_number( arg2 ) )
    {
        send_to_char( "&RUse valid numbers, Okay?\n\r", ch );
        return;
    }

    if ( atoi( arg1 ) >= atoi( arg2 ) )
    {
        send_to_char( "&RThats not a valid range. Try again.\n\r", ch );
        return;
    }

    lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
    trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );

    if ( tarea )
    {
        if ( ( lrange < tarea->low_r_vnum || trange > tarea->hi_r_vnum ) && get_trust( ch ) < 105 )
        {
            send_to_char( "&RThat range is outside of your vnum range.\n\r", ch );
            return;
        }
    }
    else
    {
        lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
        trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
    }

    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
        if ( ( room = get_room_index( vnum ) ) == NULL )
        {
            room = make_room( vnum );

            if ( !room )
            {
                bug( "RFILL: make_room failed - RFILL Aborted", 0 );
                return;
            }

            room->area = ch->pcdata->area;

            if ( argument[0] != '\0' )
                room->name = STRALLOC( argument );

            cnt++;
            continue;
        }
    }

    ch_printf( ch, "&zCreated &C%d&z rooms in the range of &C%d&z to &C%d&z.\n\r", cnt, lrange, trange );
    send_to_char ( "&z(&CC&z)ompleted RFILL Command.\n\r", ch );
    return;
}

void do_rlist( CHAR_DATA* ch, char* argument )
{
    ROOM_INDEX_DATA* room;
    int          vnum;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AREA_DATA*       tarea;
    int lrange;
    int trange;

    if ( IS_NPC( ch ) || get_trust( ch ) < LEVEL_AVATAR || !ch->pcdata
            || ( !ch->pcdata->area && get_trust( ch ) < LEVEL_GREATER ) )
    {
        send_to_char( "You don't have an assigned area.\n\r", ch );
        return;
    }

    tarea = ch->pcdata->area;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( tarea )
    {
        if ( arg1[0] == '\0' )        /* cleaned a big scary mess */
            lrange = tarea->low_r_vnum; /* here.        -Thoric */
        else
            lrange = atoi( arg1 );

        if ( arg2[0] == '\0' )
            trange = tarea->hi_r_vnum;
        else
            trange = atoi( arg2 );

        if ( ( lrange < tarea->low_r_vnum || trange > tarea->hi_r_vnum )
                && get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char( "That is out of your vnum range.\n\r", ch );
            return;
        }
    }
    else
    {
        lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
        trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
    }

    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
        if ( ( room = get_room_index( vnum ) ) == NULL )
            continue;

        ch_printf( ch, "%5d) %-50s %s %s\n\r", vnum, stripclr( room->name ),
                   ( room->description[0] != '\0' ) ? "    " : "*ND*",
                   ( room->hdescription[0] != '\0' ) ? "     " : "*NHD*" );
    }

    return;
}

void do_olist( CHAR_DATA* ch, char* argument )
{
    OBJ_INDEX_DATA*  obj;
    int          vnum;
    AREA_DATA*       tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int lrange;
    int trange;

    /*
        Greater+ can list out of assigned range - Tri (mlist/rlist as well)
    */
    if ( IS_NPC( ch ) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
            || ( !ch->pcdata->area && get_trust( ch ) < LEVEL_GREATER ) )
    {
        send_to_char( "You don't have an assigned area.\n\r", ch );
        return;
    }

    tarea = ch->pcdata->area;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( tarea )
    {
        if ( arg1[0] == '\0' )        /* cleaned a big scary mess */
            lrange = tarea->low_o_vnum; /* here.        -Thoric */
        else
            lrange = atoi( arg1 );

        if ( arg2[0] == '\0' )
            trange = tarea->hi_o_vnum;
        else
            trange = atoi( arg2 );

        if ( ( lrange < tarea->low_o_vnum || trange > tarea->hi_o_vnum )
                &&   get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char( "That is out of your vnum range.\n\r", ch );
            return;
        }
    }
    else
    {
        lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
        trange = ( is_number( arg2 ) ? atoi( arg2 ) : 3 );
    }

    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
        if ( ( obj = get_obj_index( vnum ) ) == NULL )
            continue;

        ch_printf( ch, "%5d) %-20s ", vnum, stripclr( obj->name ) );
        ch_printf( ch, "(%s)\n\r", stripclr( obj->short_descr ) );
    }

    return;
}

void do_mlist( CHAR_DATA* ch, char* argument )
{
    MOB_INDEX_DATA*  mob;
    int          vnum;
    AREA_DATA*       tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int lrange;
    int trange;

    if ( IS_NPC( ch ) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
            ||  ( !ch->pcdata->area && get_trust( ch ) < LEVEL_GREATER ) )
    {
        send_to_char( "You don't have an assigned area.\n\r", ch );
        return;
    }

    tarea = ch->pcdata->area;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( tarea )
    {
        if ( arg1[0] == '\0' )        /* cleaned a big scary mess */
            lrange = tarea->low_m_vnum; /* here.        -Thoric */
        else
            lrange = atoi( arg1 );

        if ( arg2[0] == '\0' )
            trange = tarea->hi_m_vnum;
        else
            trange = atoi( arg2 );

        if ( ( lrange < tarea->low_m_vnum || trange > tarea->hi_m_vnum )
                && get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char( "That is out of your vnum range.\n\r", ch );
            return;
        }
    }
    else
    {
        lrange = ( is_number( arg1 ) ? atoi( arg1 ) : 1 );
        trange = ( is_number( arg2 ) ? atoi( arg2 ) : 1 );
    }

    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
        if ( ( mob = get_mob_index( vnum ) ) == NULL )
            continue;

        ch_printf( ch, "%5d) %-20s '%s'\n\r", vnum,
                   mob->player_name,
                   mob->short_descr );
    }
}

void mpedit( CHAR_DATA* ch, MPROG_DATA* mprg, int mptype, char* argument )
{
    if ( mptype != -1 )
    {
        mprg->type = mptype;

        if ( mprg->arglist )
            STRFREE( mprg->arglist );

        mprg->arglist = STRALLOC( argument );
    }

    ch->substate = SUB_MPROG_EDIT;
    ch->dest_buf = mprg;

    if ( !mprg->comlist )
        mprg->comlist = STRALLOC( "" );

    start_editing( ch, mprg->comlist );
    return;
}

/*
    Mobprogram editing - cumbersome              -Thoric
*/
void do_mpedit( CHAR_DATA* ch, char* argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    CHAR_DATA*  victim;
    MPROG_DATA* mprog, *mprg, *mprg_next;
    int value, mptype, cnt;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't mpedit\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }

    switch ( ch->substate )
    {
        default:
            break;

        case SUB_MPROG_EDIT:
            if ( !ch->dest_buf )
            {
                send_to_char( "Fatal error: report to Thoric.\n\r", ch );
                bug( "do_mpedit: sub_mprog_edit: NULL ch->dest_buf", 0 );
                ch->substate = SUB_NONE;
                return;
            }

            mprog  = ch->dest_buf;

            if ( mprog->comlist )
                STRFREE( mprog->comlist );

            mprog->comlist = copy_buffer( ch );
            stop_editing( ch );
            return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    value = atoi( arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: mpedit <victim> <command> [number] <program> <value>\n\r", ch );
        send_to_char( "\n\r",                       ch );
        send_to_char( "Command being one of:\n\r",          ch );
        send_to_char( "  add delete insert edit list\n\r",      ch );
        send_to_char( "Program being one of:\n\r",          ch );
        send_to_char( "  act speech rand fight hitprcnt greet allgreet\n\r", ch );
        send_to_char( "  entry give bribe death time hour script pulse\n\r",  ch );
        return;
    }

    if ( get_trust( ch ) < LEVEL_GOD )
    {
        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( ( victim = get_char_world_full( ch, arg1 ) ) == NULL )
        {
            send_to_char( "No one like that in all the realms.\n\r", ch );
            return;
        }
    }

    if ( get_trust( ch ) < get_trust( victim ) || !IS_NPC( victim ) )
    {
        send_to_char( "You can't do that!\n\r", ch );
        return;
    }

    if ( !can_mmodify( ch, victim ) )
        return;

    if ( !xIS_SET( victim->act, ACT_PROTOTYPE ) )
    {
        send_to_char( "A mobile must have a prototype flag to be mpset.\n\r", ch );
        return;
    }

    mprog = victim->pIndexData->mudprogs;
    set_char_color( AT_GREEN, ch );

    if ( !str_cmp( arg2, "list" ) )
    {
        cnt = 0;

        if ( !mprog )
        {
            send_to_char( "That mobile has no mob programs.\n\r", ch );
            return;
        }

        for ( mprg = mprog; mprg; mprg = mprg->next )
            ch_printf( ch, "%d>%s %s\n\r%s\n\r",
                       ++cnt,
                       mprog_type_to_name( mprg->type ),
                       mprg->arglist,
                       mprg->comlist );

        return;
    }

    if ( !str_cmp( arg2, "edit" ) )
    {
        if ( !mprog )
        {
            send_to_char( "That mobile has no mob programs.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg4 );

        if ( arg4[0] != '\0' )
        {
            mptype = get_mpflag( arg4 );

            if ( mptype == -1 )
            {
                send_to_char( "Unknown program type.\n\r", ch );
                return;
            }
        }
        else
            mptype = -1;

        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        cnt = 0;

        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mpedit( ch, mprg, mptype, argument );
                xCLEAR_BITS( victim->pIndexData->progtypes );

                for ( mprg = mprog; mprg; mprg = mprg->next )
                    xSET_BIT( victim->pIndexData->progtypes, mprg->type );

                return;
            }
        }

        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "delete" ) )
    {
        int num;
        bool found;

        if ( !mprog )
        {
            send_to_char( "That mobile has no mob programs.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg4 );

        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        cnt = 0;
        found = FALSE;

        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mptype = mprg->type;
                found = TRUE;
                break;
            }
        }

        if ( !found )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        cnt = num = 0;

        for ( mprg = mprog; mprg; mprg = mprg->next )
            if ( IS_SET( mprg->type, mptype ) )
                num++;

        if ( value == 1 )
        {
            mprg_next = victim->pIndexData->mudprogs;
            victim->pIndexData->mudprogs = mprg_next->next;
        }
        else
            for ( mprg = mprog; mprg; mprg = mprg_next )
            {
                mprg_next = mprg->next;

                if ( ++cnt == ( value - 1 ) )
                {
                    mprg->next = mprg_next->next;
                    break;
                }
            }

        STRFREE( mprg_next->arglist );
        STRFREE( mprg_next->comlist );
        DISPOSE( mprg_next );

        if ( num <= 1 )
            xREMOVE_BIT( victim->pIndexData->progtypes, mptype );

        send_to_char( "Program removed.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "insert" ) )
    {
        if ( !mprog )
        {
            send_to_char( "That mobile has no mob programs.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg4 );
        mptype = get_mpflag( arg4 );

        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type.\n\r", ch );
            return;
        }

        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        if ( value == 1 )
        {
            CREATE( mprg, MPROG_DATA, 1 );
            // victim->pIndexData->progtypes |= ( 1 << mptype );
            xSET_BIT( victim->pIndexData->progtypes, mptype );
            mpedit( ch, mprg, mptype, argument );
            mprg->next = mprog;
            victim->pIndexData->mudprogs = mprg;
            return;
        }

        cnt = 1;

        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value && mprg->next )
            {
                CREATE( mprg_next, MPROG_DATA, 1 );
                // victim->pIndexData->progtypes |= ( 1 << mptype );
                xSET_BIT( victim->pIndexData->progtypes, mptype );
                mpedit( ch, mprg_next, mptype, argument );
                mprg_next->next = mprg->next;
                mprg->next  = mprg_next;
                return;
            }
        }

        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "add" ) )
    {
        mptype = get_mpflag( arg3 );

        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type.\n\r", ch );
            return;
        }

        if ( mprog != NULL )
            for ( ; mprog->next; mprog = mprog->next );

        CREATE( mprg, MPROG_DATA, 1 );

        if ( mprog )
            mprog->next           = mprg;
        else
            victim->pIndexData->mudprogs  = mprg;

        // victim->pIndexData->progtypes   |= ( 1 << mptype );
        xSET_BIT( victim->pIndexData->progtypes, mptype );
        mpedit( ch, mprg, mptype, argument );
        mprg->next = NULL;
        return;
    }

    do_mpedit( ch, "" );
}

void do_opedit( CHAR_DATA* ch, char* argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    OBJ_DATA*   obj;
    MPROG_DATA* mprog, *mprg, *mprg_next;
    int value, mptype, cnt;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't opedit\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }

    switch ( ch->substate )
    {
        default:
            break;

        case SUB_MPROG_EDIT:
            if ( !ch->dest_buf )
            {
                send_to_char( "Fatal error: report to Thoric.\n\r", ch );
                bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
                ch->substate = SUB_NONE;
                return;
            }

            mprog  = ch->dest_buf;

            if ( mprog->comlist )
                STRFREE( mprog->comlist );

            mprog->comlist = copy_buffer( ch );
            stop_editing( ch );
            return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    value = atoi( arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: opedit <object> <command> [number] <program> <value>\n\r", ch );
        send_to_char( "\n\r",                       ch );
        send_to_char( "Command being one of:\n\r",          ch );
        send_to_char( "  add delete insert edit list\n\r",      ch );
        send_to_char( "Program being one of:\n\r",          ch );
        send_to_char( "  act speech rand wear remove sac zap get\n\r",  ch );
        send_to_char( "  drop damage repair greet exa use\n\r", ch );
        send_to_char( "  useon useoff (for USE toggling)\n\r", ch );
        send_to_char( "\n\r", ch );
        send_to_char( "Object should be in your inventory to edit.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < LEVEL_GOD )
    {
        if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char( "You aren't carrying that.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
        {
            send_to_char( "Nothing like that in all the realms.\n\r", ch );
            return;
        }
    }

    if ( !can_omodify( ch, obj ) )
        return;

    if ( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
    {
        send_to_char( "An object must have a prototype flag to be opset.\n\r", ch );
        return;
    }

    mprog = obj->pIndexData->mudprogs;
    set_char_color( AT_GREEN, ch );

    if ( !str_cmp( arg2, "list" ) )
    {
        cnt = 0;

        if ( !mprog )
        {
            send_to_char( "That object has no obj programs.\n\r", ch );
            return;
        }

        for ( mprg = mprog; mprg; mprg = mprg->next )
            ch_printf( ch, "%d>%s %s\n\r%s\n\r",
                       ++cnt,
                       mprog_type_to_name( mprg->type ),
                       mprg->arglist,
                       mprg->comlist );

        return;
    }

    if ( !str_cmp( arg2, "edit" ) )
    {
        if ( !mprog )
        {
            send_to_char( "That object has no obj programs.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg4 );

        if ( arg4[0] != '\0' )
        {
            mptype = get_mpflag( arg4 );

            if ( mptype == -1 )
            {
                send_to_char( "Unknown program type.\n\r", ch );
                return;
            }
        }
        else
            mptype = -1;

        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        cnt = 0;

        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mpedit( ch, mprg, mptype, argument );
                xCLEAR_BITS( obj->pIndexData->progtypes );

                for ( mprg = mprog; mprg; mprg = mprg->next )
                    xSET_BIT( obj->pIndexData->progtypes, mprg->type );

                return;
            }
        }

        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "delete" ) )
    {
        int num;
        bool found;

        if ( !mprog )
        {
            send_to_char( "That object has no obj programs.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg4 );

        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        cnt = 0;
        found = FALSE;

        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mptype = mprg->type;
                found = TRUE;
                break;
            }
        }

        if ( !found )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        cnt = num = 0;

        for ( mprg = mprog; mprg; mprg = mprg->next )
            if ( IS_SET( mprg->type, mptype ) )
                num++;

        if ( value == 1 )
        {
            mprg_next = obj->pIndexData->mudprogs;
            obj->pIndexData->mudprogs = mprg_next->next;
        }
        else
            for ( mprg = mprog; mprg; mprg = mprg_next )
            {
                mprg_next = mprg->next;

                if ( ++cnt == ( value - 1 ) )
                {
                    mprg->next = mprg_next->next;
                    break;
                }
            }

        STRFREE( mprg_next->arglist );
        STRFREE( mprg_next->comlist );
        DISPOSE( mprg_next );

        if ( num <= 1 )
            xREMOVE_BIT( obj->pIndexData->progtypes, mptype );

        send_to_char( "Program removed.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "insert" ) )
    {
        if ( !mprog )
        {
            send_to_char( "That object has no obj programs.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg4 );
        mptype = get_mpflag( arg4 );

        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type.\n\r", ch );
            return;
        }

        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        if ( value == 1 )
        {
            CREATE( mprg, MPROG_DATA, 1 );
            // obj->pIndexData->progtypes   |= ( 1 << mptype );
            xSET_BIT( obj->pIndexData->progtypes, mptype );
            mpedit( ch, mprg, mptype, argument );
            mprg->next = mprog;
            obj->pIndexData->mudprogs = mprg;
            return;
        }

        cnt = 1;

        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value && mprg->next )
            {
                CREATE( mprg_next, MPROG_DATA, 1 );
                //  obj->pIndexData->progtypes |= ( 1 << mptype );
                xSET_BIT( obj->pIndexData->progtypes, mptype );
                mpedit( ch, mprg_next, mptype, argument );
                mprg_next->next = mprg->next;
                mprg->next  = mprg_next;
                return;
            }
        }

        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "add" ) )
    {
        mptype = get_mpflag( arg3 );

        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type.\n\r", ch );
            return;
        }

        if ( mprog != NULL )
            for ( ; mprog->next; mprog = mprog->next );

        CREATE( mprg, MPROG_DATA, 1 );

        if ( mprog )
            mprog->next            = mprg;
        else
            obj->pIndexData->mudprogs  = mprg;

        // obj->pIndexData->progtypes      |= ( 1 << mptype );
        xSET_BIT( obj->pIndexData->progtypes, mptype );
        mpedit( ch, mprg, mptype, argument );
        mprg->next = NULL;
        return;
    }

    do_opedit( ch, "" );
}



/*
    RoomProg Support
*/
void rpedit( CHAR_DATA* ch, MPROG_DATA* mprg, int mptype, char* argument )
{
    if ( mptype != -1 )
    {
        mprg->type = mptype;

        if ( mprg->arglist )
            STRFREE( mprg->arglist );

        mprg->arglist = STRALLOC( argument );
    }

    ch->substate = SUB_MPROG_EDIT;
    ch->dest_buf = mprg;

    if ( !mprg->comlist )
        mprg->comlist = STRALLOC( "" );

    start_editing( ch, mprg->comlist );
    return;
}

void do_rpedit( CHAR_DATA* ch, char* argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    MPROG_DATA* mprog, *mprg, *mprg_next;
    int value, mptype, cnt;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't rpedit\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }

    switch ( ch->substate )
    {
        default:
            break;

        case SUB_MPROG_EDIT:
            if ( !ch->dest_buf )
            {
                send_to_char( "Fatal error: report to Thoric.\n\r", ch );
                bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
                ch->substate = SUB_NONE;
                return;
            }

            mprog  = ch->dest_buf;

            if ( mprog->comlist )
                STRFREE( mprog->comlist );

            mprog->comlist = copy_buffer( ch );
            stop_editing( ch );
            return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    value = atoi( arg2 );
    /* argument = one_argument( argument, arg3 ); */

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: rpedit <command> [number] <program> <value>\n\r", ch );
        send_to_char( "\n\r",                       ch );
        send_to_char( "Command being one of:\n\r",          ch );
        send_to_char( "  add delete insert edit list\n\r",      ch );
        send_to_char( "Program being one of:\n\r",          ch );
        send_to_char( "  act speech rand sleep rest rfight enter\n\r",  ch );
        send_to_char( "  leave death pulse\n\r",                        ch );
        send_to_char( "\n\r",                       ch );
        send_to_char( "You should be standing in room you wish to edit.\n\r", ch );
        return;
    }

    if ( !can_rmodify( ch, ch->in_room ) )
        return;

    mprog = ch->in_room->mudprogs;
    set_char_color( AT_GREEN, ch );

    if ( !str_cmp( arg1, "list" ) )
    {
        cnt = 0;

        if ( !mprog )
        {
            send_to_char( "This room has no room programs.\n\r", ch );
            return;
        }

        for ( mprg = mprog; mprg; mprg = mprg->next )
            ch_printf( ch, "%d>%s %s\n\r%s\n\r",
                       ++cnt,
                       mprog_type_to_name( mprg->type ),
                       mprg->arglist,
                       mprg->comlist );

        return;
    }

    if ( !str_cmp( arg1, "edit" ) )
    {
        if ( !mprog )
        {
            send_to_char( "This room has no room programs.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg3 );

        if ( arg3[0] != '\0' )
        {
            mptype = get_mpflag( arg3 );

            if ( mptype == -1 )
            {
                send_to_char( "Unknown program type.\n\r", ch );
                return;
            }
        }
        else
            mptype = -1;

        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        cnt = 0;

        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mpedit( ch, mprg, mptype, argument );
                xCLEAR_BITS( ch->in_room->progtypes );

                for ( mprg = mprog; mprg; mprg = mprg->next )
                    xSET_BIT( ch->in_room->progtypes, mprg->type );

                return;
            }
        }

        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "delete" ) )
    {
        int num;
        bool found;

        if ( !mprog )
        {
            send_to_char( "That room has no room programs.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg3 );

        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        cnt = 0;
        found = FALSE;

        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mptype = mprg->type;
                found = TRUE;
                break;
            }
        }

        if ( !found )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        cnt = num = 0;

        for ( mprg = mprog; mprg; mprg = mprg->next )
            if ( IS_SET( mprg->type, mptype ) )
                num++;

        if ( value == 1 )
        {
            mprg_next = ch->in_room->mudprogs;
            ch->in_room->mudprogs = mprg_next->next;
        }
        else
            for ( mprg = mprog; mprg; mprg = mprg_next )
            {
                mprg_next = mprg->next;

                if ( ++cnt == ( value - 1 ) )
                {
                    mprg->next = mprg_next->next;
                    break;
                }
            }

        STRFREE( mprg_next->arglist );
        STRFREE( mprg_next->comlist );
        DISPOSE( mprg_next );

        if ( num <= 1 )
            xREMOVE_BIT( ch->in_room->progtypes, mptype );

        send_to_char( "Program removed.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "insert" ) )
    {
        if ( !mprog )
        {
            send_to_char( "That room has no room programs.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg3 );
        mptype = get_mpflag( arg2 );

        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type.\n\r", ch );
            return;
        }

        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }

        if ( value == 1 )
        {
            CREATE( mprg, MPROG_DATA, 1 );
            // ch->in_room->progtypes |= ( 1 << mptype );
            xSET_BIT( ch->in_room->progtypes, mptype );
            mpedit( ch, mprg, mptype, argument );
            mprg->next = mprog;
            ch->in_room->mudprogs = mprg;
            return;
        }

        cnt = 1;

        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value && mprg->next )
            {
                CREATE( mprg_next, MPROG_DATA, 1 );
                // ch->in_room->progtypes |= ( 1 << mptype );
                xSET_BIT( ch->in_room->progtypes, mptype );
                mpedit( ch, mprg_next, mptype, argument );
                mprg_next->next = mprg->next;
                mprg->next  = mprg_next;
                return;
            }
        }

        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "add" ) )
    {
        mptype = get_mpflag( arg2 );

        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type.\n\r", ch );
            return;
        }

        if ( mprog )
            for ( ; mprog->next; mprog = mprog->next );

        CREATE( mprg, MPROG_DATA, 1 );

        if ( mprog )
            mprog->next       = mprg;
        else
            ch->in_room->mudprogs = mprg;

        // ch->in_room->progtypes |= ( 1 << mptype );
        xSET_BIT( ch->in_room->progtypes, mptype );
        mpedit( ch, mprg, mptype, argument );
        mprg->next = NULL;
        return;
    }

    do_rpedit( ch, "" );
}

void do_rdelete( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* location;
    argument = one_argument( argument, arg );

    /*  Temporarily disable this command.
        return; */

    if ( arg[0] == '\0' )
    {
        send_to_char( "Delete which room?\n\r", ch );
        return;
    }

    /* Find the room. */
    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    /* Does the player have the right to delete this room? */
    if ( get_trust( ch ) < sysdata.level_modify_proto
            && ( location->vnum < ch->pcdata->r_range_lo ||
                 location->vnum > ch->pcdata->r_range_hi ) )
    {
        send_to_char( "That room is not in your assigned range.\n\r", ch );
        return;
    }

    /* We could go to the trouble of clearing out the room, but why? */
    /* Delete_room does that anyway, but this is probably safer */
    if ( location->first_person || location->first_content )
    {
        send_to_char( "The room must be empty first.\n\r", ch );
        return;
    }

    /*  Ok, we've determined that the room exists, it is empty and the
        player has the authority to delete it, so let's dump the thing.
        The function to do it is in db.c so it can access the top-room
        variable. */
    delete_room( location );
    send_to_char( "Room deleted.\n\r", ch );
    return;
}
void do_odelete( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA* obj;
    OBJ_DATA* temp;
    argument = one_argument( argument, arg );

    /* Temporarily disable this command. */
    /*    return;*/

    if ( arg[0] == '\0' )
    {
        send_to_char( "Delete which object?\n\r", ch );
        return;
    }

    /* Find the object. */
    if ( !( obj = get_obj_index( atoi( arg ) ) ) )
    {
        if ( !( temp = get_obj_here( ch, arg ) ) )
        {
            send_to_char( "No such object.\n\r", ch );
            return;
        }

        obj = temp->pIndexData;
    }

    /* Does the player have the right to delete this room? */
    if ( get_trust( ch ) < sysdata.level_modify_proto
            && ( obj->vnum < ch->pcdata->o_range_lo ||
                 obj->vnum > ch->pcdata->o_range_hi ) )
    {
        send_to_char( "That object is not in your assigned range.\n\r", ch );
        return;
    }

    /*  Ok, we've determined that the room exists, it is empty and the
        player has the authority to delete it, so let's dump the thing.
        The function to do it is in db.c so it can access the top-room
        variable. */
    delete_obj( obj );
    send_to_char( "Object deleted.\n\r", ch );
    return;
}
void do_mdelete( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA* mob;
    CHAR_DATA* temp;
    argument = one_argument( argument, arg );

    /* Temporarily disable this command. */
    /*    return;*/

    if ( arg[0] == '\0' )
    {
        send_to_char( "Delete which mob?\n\r", ch );
        return;
    }

    /* Find the mob. */
    if ( !( mob = get_mob_index( atoi( arg ) ) ) )
    {
        if ( !( temp = get_char_room( ch, arg ) ) || !IS_NPC( temp ) )
        {
            send_to_char( "No such mob.\n\r", ch );
            return;
        }

        mob = temp->pIndexData;
    }

    /* Does the player have the right to delete this room? */
    if ( get_trust( ch ) < sysdata.level_modify_proto
            && ( mob->vnum < ch->pcdata->m_range_lo ||
                 mob->vnum > ch->pcdata->m_range_hi ) )
    {
        send_to_char( "That mob is not in your assigned range.\n\r", ch );
        return;
    }

    /*  Ok, we've determined that the mob exists and the player has the
        authority to delete it, so let's dump the thing.
        The function to do it is in db.c so it can access the top_mob_index
        variable. */
    delete_mob( mob );
    send_to_char( "Mob deleted.\n\r", ch );
    return;
}

/* rdig command by Dracones */

void do_rdig( CHAR_DATA* ch, char* argument )
{
    char arg [MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    ROOM_INDEX_DATA* location, *ch_location;
    AREA_DATA*           pArea;
    int         vnum, edir;
    char tmpcmd[MAX_INPUT_LENGTH];
    EXIT_DATA*       xit;
    set_char_color( AT_PLAIN, ch );
    ch_location = ch->in_room;
    argument = one_argument( argument, arg );

    if ( !arg || arg[0] == '\0' )
    {
        send_to_char( "Dig out a new room or dig into an existing room.\n\r", ch );
        send_to_char( "Usage: rdig <dir>\n\r", ch );
        return;
    }

    if ( !ch->pcdata->area )
    {
        send_to_char( "It might help if you assigned your self an area, or you might crash the mud.\n\r", ch );
        return;
    }

    edir = get_dir( arg );
    xit = get_exit( ch_location, edir );

    if ( !xit )
    {
        pArea = ch->pcdata->area;
        vnum = pArea->low_r_vnum;

        while ( vnum <= pArea->hi_r_vnum && get_room_index( vnum ) != NULL )
        {
            vnum++;
        }

        if ( vnum > pArea->hi_r_vnum )
        {
            send_to_char( "No empty upper rooms could be found.\r\n", ch );
            return;
        }

        sprintf( buf, "Digging out room %d to the %s.\r\n", vnum, arg );
        send_to_char( buf, ch );
        location = make_room( vnum );

        if ( !location )
        {
            bug( "rdig: make_room failed", 0 );
            return;
        }

        location->area = ch->pcdata->area;
        sprintf( tmpcmd, "bexit %s %d", arg, vnum );
        do_redit( ch, tmpcmd );
    }
    else
    {
        vnum = xit->vnum;
        location = get_room_index( vnum );
        sprintf( buf, "Digging into room %d to the %s.\r\n", vnum, arg );
        send_to_char( buf, ch );
    }

    location->name = STRALLOC( ch_location->name );
    location->description = STRALLOC( ch_location->description );
    location->hdescription = STRALLOC( ch_location->hdescription );
    location->sector_type = ch_location->sector_type;
    location->room_flags = ch_location->room_flags;
    location->hroom_flags = ch_location->hroom_flags;
    /*
        Below here you may add anything else you wish to be
        copied into the rdug rooms.

        NiteDesc is specific my mud, do not add if not needed -- Drac
    */
    /*    location->nitedesc = ch_location->nitedesc; */
    /* Move while rdigging -- Dracones */
    sprintf( buf, "%d", vnum );
    do_goto( ch, buf );
    return;
}

/*
    Makes a exact copy of a room to another room vnum.
    Syntax: RCOPY <from vnum> <to vnum>
    -Ghost
*/
void do_rcopy( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* room;
    ROOM_INDEX_DATA* target;
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Copy which room? RCOPY <from> <to>\n\r", ch );
        return;
    }

    /* Find the room. */
    if ( ( room = find_location( ch, arg ) ) == NULL )
    {
        send_to_char( "Invalid room for <FROM>. RCOPY <from> <to>\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        sprintf( arg2, "%d", ch->in_room->vnum );
        // send_to_char( "Copy to what room? RCOPY <from> <to>\n\r", ch );
        // return;
    }

    /* Find the room. */
    if ( ( target = find_location( ch, arg2 ) ) == NULL )
    {
        send_to_char( "Invalid room for <TO>. RCOPY <from> <to>\n\r", ch );
        return;
    }

    if ( target->name )
        STRFREE( target->name );

    if ( target->description )
        STRFREE( target->description );

    if ( target->hdescription )
        STRFREE( target->hdescription );

    target->name = STRALLOC( room->name );
    target->description = STRALLOC( room->description );
    target->hdescription = STRALLOC( room->hdescription );
    target->sector_type = room->sector_type;
    target->room_flags = room->room_flags;
    target->hroom_flags = room->hroom_flags;
}

/*
    For in-code rcopys. Example under morespace.c
*/
void rcopy( ROOM_INDEX_DATA* target, ROOM_INDEX_DATA* room )
{
    if ( !target || target == NULL )
        bug( "*ERROR* rcopy: NULL TARGET POINTER" );
    else if ( !room || room == NULL )
        bug( "*ERROR* rcopy: NULL ROOM POINTER" );
    else
    {
        if ( target->name )
            STRFREE( target->name );

        if ( target->description )
            STRFREE( target->description );

        if ( target->hdescription )
            STRFREE( target->hdescription );

        target->name = STRALLOC( room->name );
        target->description = STRALLOC( room->description );
        target->hdescription = STRALLOC( room->hdescription );
        target->tunnel      = room->tunnel;
        target->sector_type = room->sector_type;
        target->room_flags  = room->room_flags;
        target->hroom_flags = room->hroom_flags;
    }

    return;
}

/*
    Mobile and Object Program Copying
    Last modified Feb. 24 1999
    Mystaric
*/

/*  void mpcopy( MPROG_DATA *source, MPROG_DATA *destination)
    {
    destination->type=source->type;
    destination->triggered=source->triggered;
    destination->resetdelay=source->resetdelay;
    destination->arglist = STRALLOC( source->arglist);
    destination->comlist = STRALLOC( source->comlist);
    destination->next = NULL;
    }

    void do_opcopy( CHAR_DATA *ch, char *argument )
    {
    char sobj [MAX_INPUT_LENGTH];
    char prog [MAX_INPUT_LENGTH];
    char num  [MAX_INPUT_LENGTH];
    char dobj [MAX_INPUT_LENGTH];
    OBJ_DATA  *source=NULL, *destination=NULL;
    MPROG_DATA *source_oprog=NULL, *dest_oprog=NULL, *source_oprg=NULL, *dest_oprg=NULL;
    int value = -1, optype = -1, cnt=0;
    bool COPY = FALSE;

    if ( IS_NPC( ch ) )
    {
    send_to_char( "Mob's can't opcopy\n\r", ch );
    return;
    }

    if ( !ch->desc )
    {
    send_to_char( "You have no descriptor\n\r", ch );
    return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, sobj );
    argument = one_argument( argument, prog );

    if ( sobj[0] == '\0' || prog[0] == '\0' )
    {
        send_to_char( "Syntax: opcopy <source object> <program> [number] <destination object>\n\r", ch );
        send_to_char( "        opcopy <source object> all <destination object>\n\r", ch );
        send_to_char( "        opcopy <source object> all <destination object> <program>\n\r", ch );
    send_to_char( "\n\r",                       ch );
    send_to_char( "Program being one of:\n\r",          ch );
    send_to_char( "  act speech rand wear remove sac zap get\n\r",  ch );
    send_to_char( "  drop damage repair greet exa use\n\r",ch );
        send_to_char( "  useon useoff (for USE toggling)\n\r",ch );
    send_to_char( "\n\r", ch);
    send_to_char( "Object should be in your inventory to edit.\n\r",ch);
    return;
    }

    if (!strcmp( prog, "all") )
    {
        argument = one_argument( argument, dobj );
        argument = one_argument( argument, prog );
        optype = get_mpflag(prog);
        COPY = TRUE;
    }
    else
    {
       argument = one_argument( argument, num );
       argument = one_argument( argument, dobj );
       value = atoi( num );
    }

    if ( get_trust( ch ) < LEVEL_GOD )
    {
        if ( ( source = get_obj_carry( ch, sobj ) ) == NULL )
        {
       send_to_char( "You aren't carrying source object.\n\r", ch );
       return;
        }

        if ( ( destination = get_obj_carry( ch, dobj ) ) == NULL )
        {
       send_to_char( "You aren't carrying destination object.\n\r", ch );
       return;
        }
    }
    else
    {
        if ( ( source = get_obj_world( ch, sobj ) ) == NULL )
        {
        send_to_char( "Can't find source object in all the realms.\n\r", ch );
        return;
        }

        if ( ( destination = get_obj_world( ch, dobj ) ) == NULL )
        {
        send_to_char( "Can't find destination object in all the realms.\n\r", ch );
        return;
        }
    }

    if ( source == destination )
    {
        send_to_char( "Source and destination objects cannot be the same\n\r", ch);
        return;
    }


    if ( !can_omodify( ch, destination ) )
    {
        send_to_char( "You cannot modify destination object.\n\r", ch);
    return;
    }

    if ( !IS_OBJ_STAT(destination, ITEM_PROTOTYPE) )
    {
    send_to_char( "Destination object must have prototype flag.\n\r", ch );
    return;
    }

    set_char_color( AT_PLAIN, ch );

    source_oprog = source->pIndexData->mudprogs;
    dest_oprog = destination->pIndexData->mudprogs;



    set_char_color( AT_GREEN, ch );

    if ( !source_oprog )
    {
        send_to_char( "Source object has no mob programs.\n\r", ch );
    return;
    }

    if(COPY)
    {
        for ( source_oprg = source_oprog; source_oprg; source_oprg = source_oprg->next )
        {
        if ( optype==source_oprg->type || optype == -1 )
        {
            if ( dest_oprog != NULL )
            for ( ; dest_oprog->next; dest_oprog = dest_oprog->next );
            CREATE( dest_oprg, MPROG_DATA, 1 );
            if (dest_oprog )
               dest_oprog->next= dest_oprg;
            else
                {
                   destination->pIndexData->mudprogs = dest_oprg;
                   dest_oprog = dest_oprg;
                }
                mpcopy( source_oprg, dest_oprg );
                xSET_BIT(destination->pIndexData->progtypes, dest_oprg->type);
                cnt++;
            }
        }

    if (cnt == 0)
    {
       ch_printf(ch, "No such program in source object\n\r");
       return;
    }
    ch_printf(ch, "%d programs successfully copied from %s to %s.\n\r", cnt, sobj, dobj);
    return;
    }

     if(value < 1 )
     {
         send_to_char( "No such program in source object.\n\r", ch );
         return;
     }

     optype = get_mpflag(prog);

     for ( source_oprg = source_oprog; source_oprg; source_oprg = source_oprg->next )
     {
         if(++cnt == value && source_oprg->type == optype )
     {
         if ( dest_oprog != NULL )
             for ( ; dest_oprog->next; dest_oprog = dest_oprog->next );
         CREATE( dest_oprg, MPROG_DATA, 1 );
         if (dest_oprog )
             dest_oprog->next= dest_oprg;
         else
                 destination->pIndexData->mudprogs = dest_oprg;
         mpcopy( source_oprg, dest_oprg );
             xSET_BIT(destination->pIndexData->progtypes, dest_oprg->type);
             ch_printf(ch, "%s program %d from %s successfully copied to %s.\n\r",
             prog, value, sobj, dobj);
             return;
         }
     }
     if (!source_oprg)
     {
         send_to_char( "No such program in source object.\n\r", ch );
         return;
      }
      do_opcopy( ch, "" );
    }

    void do_mpcopy( CHAR_DATA *ch, char *argument )
    {
    char smob [MAX_INPUT_LENGTH];
    char prog [MAX_INPUT_LENGTH];
    char num  [MAX_INPUT_LENGTH];
    char dmob [MAX_INPUT_LENGTH];
    CHAR_DATA  *source=NULL, *destination=NULL;
    MPROG_DATA *source_mprog=NULL, *dest_mprog=NULL, *source_mprg=NULL, *dest_mprg=NULL;
    int value = -1, mptype = -1, cnt = 0;
    bool COPY = FALSE;

    set_char_color( AT_PLAIN, ch );

    if ( IS_NPC( ch ) )
    {
    send_to_char( "Mob's can't opcop\n\r", ch );
    return;
    }

    if ( !ch->desc )
    {
    send_to_char( "You have no descriptor\n\r", ch );
    return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, smob );
    argument = one_argument( argument, prog );

    if ( smob[0] == '\0' || prog[0] == '\0' )
    {
        send_to_char( "Syntax: mpcopy <source mobile> <program> [number] <destination mobile>\n\r", ch );
        send_to_char( "        mpcopy <source mobile> all <destination mobile>\n\r", ch );
        send_to_char( "        mpcopy <source mobile> all <destination mobile> <program>\n\r", ch );
    send_to_char( "\n\r",                       ch );
    send_to_char( "Program being one of:\n\r",          ch );
    send_to_char( "  act speech rand fight hitprcnt greet allgreet\n\r", ch );
    send_to_char( "  entry give bribe death time hour script\n\r",  ch );
    return;
    }

    if (!strcmp( prog, "all") )
    {
        argument = one_argument( argument, dmob );
        argument = one_argument( argument, prog );
        mptype = get_mpflag(prog);
        COPY = TRUE;
    }
    else
    {
       argument = one_argument( argument, num );
       argument = one_argument( argument, dmob );
       value = atoi( num );
    }

    if ( get_trust( ch ) < LEVEL_GOD )
    {
        if ( ( source = get_char_room( ch, smob ) ) == NULL )
        {
            send_to_char( "Source mobile is not present.\n\r", ch );
            return;
        }

        if ( ( destination = get_char_room( ch, dmob ) ) == NULL )
        {
            send_to_char( "Destination mobile is not present.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( ( source = get_char_world_full( ch, smob ) ) == NULL )
        {
            send_to_char( "Can't find source mobile\n\r", ch );
            return;
        }

        if ( ( destination = get_char_world_full( ch, dmob ) ) == NULL )
        {
            send_to_char( "Can't find destination mobile\n\r", ch );
            return;
        }
    }
    if ( source == destination )
    {
        send_to_char( "Source and destination mobiles cannot be the same\n\r", ch);
        return;
    }

    if ( get_trust( ch ) < source->toplevel || !IS_NPC(source) ||
         get_trust( ch) < destination->toplevel || !IS_NPC(destination) )
    {
    send_to_char( "You can't do that!\n\r", ch );
    return;
    }

    if ( !can_mmodify( ch, destination ) )
    {
        send_to_char( "You cannot modify destination mobile.\n\r", ch);
    return;
    }

    if ( !xIS_SET(destination->act, ACT_PROTOTYPE) )
    {
    send_to_char( "Destination mobile must have a prototype flag to mpcopy.\n\r", ch );
    return;
    }

    source_mprog = source->pIndexData->mudprogs;
    dest_mprog = destination->pIndexData->mudprogs;

    set_char_color( AT_GREEN, ch );

    if ( !source_mprog )
    {
        send_to_char( "Source mobile has no mob programs.\n\r", ch );
    return;
    }


    if(COPY)
    {
        for ( source_mprg = source_mprog; source_mprg; source_mprg = source_mprg->next )
        {
        if ( mptype==source_mprg->type || mptype == -1 )
        {
            if ( dest_mprog != NULL )
                for ( ; dest_mprog->next; dest_mprog = dest_mprog->next );
            CREATE( dest_mprg, MPROG_DATA, 1 );

            if (dest_mprog )
                dest_mprog->next= dest_mprg;
            else
                {
                    destination->pIndexData->mudprogs = dest_mprg;
                    dest_mprog = dest_mprg;
                }
                mpcopy( source_mprg, dest_mprg );
                xSET_BIT(destination->pIndexData->progtypes, dest_mprg->type);
                cnt++;
            }
        }

    if (cnt == 0)
    {
       ch_printf(ch, "No such program in source mobile\n\r");
       return;
    }
    ch_printf(ch, "%d programs successfully copied from %s to %s.\n\r", cnt, smob, dmob);
    return;
    }

     if(value < 1 )
     {
         send_to_char( "No such program in source mobile.\n\r", ch );
         return;
     }

     mptype = get_mpflag(prog);

     for ( source_mprg = source_mprog; source_mprg; source_mprg = source_mprg->next )
     {
         if(++cnt == value && source_mprg->type == mptype )
     {
         if ( dest_mprog != NULL )
             for ( ; dest_mprog->next; dest_mprog = dest_mprog->next );
         CREATE( dest_mprg, MPROG_DATA, 1 );
         if (dest_mprog )
             dest_mprog->next= dest_mprg;
         else
                 destination->pIndexData->mudprogs = dest_mprg;
         mpcopy( source_mprg, dest_mprg );
             xSET_BIT(destination->pIndexData->progtypes, dest_mprg->type);
             ch_printf(ch, "%s program %d from %s successfully copied to %s.\n\r",
             prog, value, smob, dmob);
             return;
         }
     }

     if (!source_mprg)
     {
         send_to_char( "No such program in source mobile.\n\r", ch );
         return;
      }
      do_mpcopy( ch, "" );
    } */

void do_rpcopy( CHAR_DATA* ch, char* argument )
{
    char buf  [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA* from = NULL, *to = NULL;
    MPROG_DATA* mprog, *mprg, *mprg_next;
    int value, mptype = 0, cnt = 0, fault = 0;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "RPCOPY is restricted to real builders, Sorry.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || atoi( arg1 ) <= 0 )
    {
        send_to_char( "Syntax: RPCOPY <from room> [Optional - To Room]\n\r", ch );
        return;
    }

    from = get_room_index( atoi( arg1 ) );

    if ( atoi( arg2 ) <= 0 )
    {
        to = ch->in_room;
    }
    else
    {
        to = get_room_index( atoi( arg2 ) );
    }

    if ( from == NULL )
    {
        send_to_char( "Invalid First Argument. FROM Room does not exist.\n\r", ch );
        return;
    }

    if ( to == NULL )
    {
        send_to_char( "Invalid Second Argument. TO Room does not exist.\n\r", ch );
        return;
    }

    if ( !can_rmodify( ch, to ) )
    {
        send_to_char( "&RYeah right! I'm not thinking so!\n\r", ch );
        return;
    }

    mprog = from->mudprogs;
    cnt = 0;

    if ( !mprog )
    {
        send_to_char( "Specified room has no room programs set.\n\r", ch );
        return;
    }

    for ( mprg = mprog; mprg; mprg = mprg->next )
    {
        ++cnt;

        if ( !lock_rprog( to, mprg->comlist, mprg->arglist, mprg->type ) )
        {
            send_to_char( "&R[RPCOPY Fault]: Xerox is out of ink. Notify Ghost.\n\r", ch );
            return;
        }
    }

    if ( fault > 0 )
        ch_printf( ch, "&R%d Program(s) failed to copy to the target location.\n\r", fault );

    ch_printf( ch, "&Y%d Program(s) were successfully copied to the target location.\n\r", cnt );
    ch_printf( ch, "&GRPCOPY Command complete. Have a nice day.\n\r" );
    return;
}

int free_area_vnum( CHAR_DATA* ch )
{
    int first = 0;
    int last = 0;
    int i = 0;

    if ( !ch->pcdata )
        return -1;

    if ( !ch->pcdata->area )
        return -1;

    first = ch->pcdata->area->low_r_vnum;
    last = ch->pcdata->area->hi_r_vnum;

    for ( i = first; i < last; i++ )
    {
        if ( get_room_index( i ) == NULL )
            return i;
    }

    return -1;
}

ROOM_INDEX_DATA* bw_create( CHAR_DATA* ch )
{
    ROOM_INDEX_DATA* room = NULL;
    AREA_DATA* pArea = NULL;
    int vnum;
    vnum = free_area_vnum( ch );

    if ( vnum == -1 )
    {
        send_to_char( "No more rooms left in your range, buddy.\n\r", ch );
        return NULL;
    }

    if ( get_trust( ch ) < sysdata.level_modify_proto )
    {
        if ( !ch->pcdata || !( pArea = ch->pcdata->area ) )
        {
            send_to_char( "You must have an assigned area to create rooms.\n\r", ch );
            return NULL;
        }

        if ( vnum < pArea->low_r_vnum || vnum > pArea->hi_r_vnum )
        {
            send_to_char( "That room is not within your assigned range.\n\r", ch );
            return NULL;
        }
    }

    room = make_room( vnum );

    if ( !room )
    {
        bug( "Buildwalk: make_room failed", 0 );
        return NULL;
    }

    room->area = ch->pcdata->area;
    return room;
}

bool lock_rprog( ROOM_INDEX_DATA* room, char* prog, char* argument, int mptype )
{
    MPROG_DATA* mprog = NULL, *mprg = NULL;
    /* MPROG_DATA *mprg_next; */
    mprog = room->mudprogs;

    if ( mprog )
    {
        for ( ; mprog->next; mprog = mprog->next ) { }
    }

    CREATE( mprg, MPROG_DATA, 1 );

    if ( mprog )
    {
        mprog->next    = mprg;
    }
    else
    {
        room->mudprogs = mprg;
    }

    if ( mprg == NULL )
        return FALSE;

    // room->progtypes |= mptype ;
    xSET_BIT( room->progtypes, mptype );
    mprg->type = mptype;

    if ( mprg->arglist )
        STRFREE( mprg->arglist );

    mprg->arglist = STRALLOC( argument );
    mprg->comlist = STRALLOC( prog );
    mprg->next = NULL;
    return TRUE;
}

