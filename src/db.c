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
            Database management module
****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "mud.h"

#define BFS_MARK         BV01

extern  int _filbuf     args( ( FILE* ) );

#if defined(KEY)
    #undef KEY
#endif

void init_supermob();

#define KEY( literal, field, value )                    \
    if ( !str_cmp( word, literal ) )    \
    {                   \
        field  = value;         \
        fMatch = TRUE;          \
        break;              \
    }


/*
    Globals.
*/

WIZENT*     first_wiz;
WIZENT*     last_wiz;

time_t                  last_restore_all_time = 0;

HELP_DATA*      first_help;
HELP_DATA*      last_help;

SHOP_DATA*      first_shop;
SHOP_DATA*      last_shop;

TELEPORT_DATA*      first_teleport;
TELEPORT_DATA*      last_teleport;

OBJ_DATA*       extracted_obj_queue;
EXTRACT_CHAR_DATA*  extracted_char_queue;

char            bug_buf     [MSL];
CHAR_DATA*      first_char;
CHAR_DATA*      last_char;
char*           help_greeting;
char            log_buf     [MSL];

OBJ_DATA*       first_object;
OBJ_DATA*       last_object;
TIME_INFO_DATA      time_info;
WEATHER_DATA        weather_info;

int         cur_qobjs;
int         cur_qchars;
int         nummobsloaded;
int         numobjsloaded;
int         physicalobjects;

FILE*           fpLOG;

/* Marines */
sh_int   gsn_lightarmor;
sh_int   gsn_handweapons;
sh_int   gsn_basic_medical;
sh_int   gsn_eagle_eye;
sh_int   gsn_sharpshooting;
sh_int   gsn_friendly_fire;
sh_int   gsn_alertness;
sh_int   gsn_awareness;
sh_int   gsn_mediumarmor;
sh_int   gsn_electronics;
sh_int   gsn_advanced_medical;
sh_int   gsn_near_miss;
sh_int   gsn_demolitions;
sh_int   gsn_heavyarmor;
sh_int   gsn_cartography;
sh_int   gsn_logistics;
sh_int   gsn_requestassist;
sh_int   gsn_moraleboost;

/* Aliens */
sh_int   gsn_dodge;
sh_int   gsn_evasive;
sh_int   gsn_tailslam;
sh_int   gsn_battle_method;
sh_int   gsn_alien_rage;
sh_int   gsn_critical_strike;
sh_int   gsn_acidspit;
sh_int   gsn_leap;
sh_int   gsn_breach;
sh_int   gsn_stealth;
sh_int   gsn_lunge;
sh_int   gsn_acid_potency;
sh_int   gsn_headbite;
sh_int   gsn_confuse;
sh_int   gsn_acute_hearing;
sh_int   gsn_pursuit;
sh_int   gsn_pheromones;
sh_int   gsn_impale;
sh_int   gsn_layeggs;

/* Predator */
sh_int   gsn_battle_focus;
sh_int   gsn_track;
sh_int   gsn_vision_modes;
sh_int   gsn_nightvision;
sh_int   gsn_shoulder_cannon;
sh_int   gsn_medical;
sh_int   gsn_cr_combat;
sh_int   gsn_tumble;
sh_int   gsn_trap_setting;
sh_int   gsn_double_strike;
sh_int   gsn_venom_cloud;
sh_int   gsn_selfdestruct;
sh_int   gsn_rage;
sh_int   gsn_ranged_combat;
sh_int   gsn_primal_instinct;
sh_int   gsn_sixth_sense;
sh_int   gsn_battlecry;
sh_int   gsn_human_tech;

/* Languages */
sh_int                  gsn_marine;
sh_int                  gsn_alien;
sh_int                  gsn_predator;

/* For searching */
sh_int          gsn_first_skill;
sh_int          gsn_first_weapon;
sh_int          gsn_first_tongue;
sh_int          gsn_top_sn;


/*
    Locals.
*/
MOB_INDEX_DATA*     mob_index_hash      [MAX_KEY_HASH];
OBJ_INDEX_DATA*     obj_index_hash      [MAX_KEY_HASH];
ROOM_INDEX_DATA*    room_index_hash     [MAX_KEY_HASH];

AREA_DATA*      first_area;
AREA_DATA*      last_area;
AREA_DATA*      first_build;
AREA_DATA*      last_build;
AREA_DATA*      first_asort;
AREA_DATA*      last_asort;
AREA_DATA*      first_bsort;
AREA_DATA*      last_bsort;

SYSTEM_DATA     sysdata;

int         top_affect;
int         top_area;
int         top_ed;
int         top_exit;
int         top_help;
int         top_mob_index;
int         top_obj_index;
int         top_reset;
int         top_room;
int         top_shop;
int         top_vroom;

/*
    Semi-locals.
*/
bool            fBootDb;
FILE*           fpArea;
char            strArea[MAX_INPUT_LENGTH];



/*
    Local booting procedures.
*/
void    init_mm     args( ( void ) );

void    boot_log    args( ( const char* str, ... ) );
void    load_area   args( ( FILE* fp ) );
void    load_author     args( ( AREA_DATA* tarea, FILE* fp ) );
void    load_resetmsg   args( ( AREA_DATA* tarea, FILE* fp ) ); /* Rennard */
void    load_flags      args( ( AREA_DATA* tarea, FILE* fp ) );
void    load_helps  args( ( AREA_DATA* tarea, FILE* fp ) );
void    load_mobiles    args( ( AREA_DATA* tarea, FILE* fp ) );
void    load_objects    args( ( AREA_DATA* tarea, FILE* fp ) );
void    load_resets args( ( AREA_DATA* tarea, FILE* fp ) );
void    load_rooms  args( ( AREA_DATA* tarea, FILE* fp ) );
void    load_shops  args( ( AREA_DATA* tarea, FILE* fp ) );
void    load_specials   args( ( AREA_DATA* tarea, FILE* fp ) );
void    load_ranges args( ( AREA_DATA* tarea, FILE* fp ) );
void    load_buildlist  args( ( void ) );
bool    load_systemdata args( ( SYSTEM_DATA* sys ) );
void    load_banlist    args( ( void ) );
void    load_mplist args( ( void ) );
ROOM_INDEX_DATA* make_vent_room( int x, int y, int z );

void    fix_exits   args( ( void ) );

/*
    External booting function
*/
void    load_corpses    args( ( void ) );
void    renumber_put_resets args( ( AREA_DATA* pArea ) );

/*
    MUDprogram locals
*/

int         mprog_name_to_type  args ( ( char* name ) );
MPROG_DATA*     mprog_file_read     args ( ( char* f, MPROG_DATA* mprg,
        MOB_INDEX_DATA* pMobIndex ) );
/* int      oprog_name_to_type  args ( ( char* name ) ); */
MPROG_DATA*     oprog_file_read     args ( ( char* f, MPROG_DATA* mprg,
        OBJ_INDEX_DATA* pObjIndex ) );
/* int      rprog_name_to_type  args ( ( char* name ) ); */
MPROG_DATA*     rprog_file_read     args ( ( char* f, MPROG_DATA* mprg,
        ROOM_INDEX_DATA* pRoomIndex ) );
void        load_mudprogs           args ( ( AREA_DATA* tarea, FILE* fp ) );
void        load_objprogs           args ( ( AREA_DATA* tarea, FILE* fp ) );
void        load_roomprogs          args ( ( AREA_DATA* tarea, FILE* fp ) );
void        mprog_read_programs     args ( ( FILE* fp,
        MOB_INDEX_DATA* pMobIndex ) );
void        oprog_read_programs     args ( ( FILE* fp,
        OBJ_INDEX_DATA* pObjIndex ) );
void        rprog_read_programs     args ( ( FILE* fp,
        ROOM_INDEX_DATA* pRoomIndex ) );


void shutdown_mud( char* reason )
{
    FILE* fp;

    if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
    {
        fprintf( fp, "%s\n", reason );
        fclose( fp );
    }
}


/*
    Big mama top level function.
*/
void boot_db( bool fCpyOver )
{
    sh_int wear, x;
    show_hash( 32 );
    unlink( BOOTLOG_FILE );
    boot_log( "---------------------[ Boot Log ]--------------------" );

    if ( fCpyOver )
        boot_log( "COPYOVER STATE: *-=# ACTIVE #=-*" );
    else
        boot_log( "COPYOVER STATE: Offline " );

    log_string( "Loading commands" );
    load_commands();
    log_string( "Loading sysdata configuration..." );
    /* default values */
    sysdata.read_all_mail       = LEVEL_DEMI;
    sysdata.read_mail_free      = LEVEL_IMMORTAL;
    sysdata.write_mail_free         = LEVEL_IMMORTAL;
    sysdata.take_others_mail        = LEVEL_DEMI;
    sysdata.muse_level          = LEVEL_DEMI;
    sysdata.think_level         = LEVEL_HIGOD;
    sysdata.build_level         = LEVEL_DEMI;
    sysdata.log_level           = LEVEL_LOG;
    sysdata.level_modify_proto      = LEVEL_LESSER;
    sysdata.level_override_private  = LEVEL_GREATER;
    sysdata.level_mset_player       = LEVEL_LESSER;
    sysdata.stun_plr_vs_plr     = 15;
    sysdata.stun_regular        = 15;
    sysdata.currid                      = 0;
    sysdata.dam_plr_vs_plr      = 100;
    sysdata.dam_plr_vs_mob      = 100;
    sysdata.dam_mob_vs_plr      = 100;
    sysdata.dam_mob_vs_mob      = 100;
    sysdata.level_getobjnotake      = LEVEL_GREATER;
    sysdata.save_frequency      = 20;   /* minutes */
    sysdata.save_flags          = SV_DEATH | SV_PASSCHG | SV_AUTO
                                  | SV_PUT | SV_DROP | SV_GIVE
                                  | SV_ZAPDROP | SV_IDLE;

    if ( !load_systemdata( &sysdata ) )
    {
        log_string( "Not found.  Creating new configuration." );
        sysdata.alltimemax = 0;
    }

    log_string( "Loading socials" );
    load_socials();
    log_string( "Loading skill table" );
    load_skill_table();
    sort_skill_table();
    gsn_first_skill  = 0;
    gsn_first_weapon = 0;
    gsn_first_tongue = 0;
    gsn_top_sn       = top_sn;

    for ( x = 0; x < top_sn; x++ )
        if ( !gsn_first_skill && skill_table[x]->type == SKILL_SKILL )
            gsn_first_skill = x;
        else if ( !gsn_first_weapon && skill_table[x]->type == SKILL_WEAPON )
            gsn_first_weapon = x;
        else if ( !gsn_first_tongue && skill_table[x]->type == SKILL_TONGUE )
            gsn_first_tongue = x;

    log_string( "Loading herb table" );
    load_herb_table();
    log_string( "Making wizlist" );
    make_wizlist();
    log_string( "Initializing request pipe" );
    init_request_pipe();
    fBootDb     = TRUE;
    nummobsloaded   = 0;
    numobjsloaded   = 0;
    physicalobjects = 0;
    sysdata.maxplayers  = 0;
    first_object    = NULL;
    last_object     = NULL;
    first_char      = NULL;
    last_char       = NULL;
    first_area      = NULL;
    last_area       = NULL;
    first_build     = NULL;
    last_area       = NULL;
    first_shop      = NULL;
    last_shop       = NULL;
    first_teleport  = NULL;
    last_teleport   = NULL;
    first_asort     = NULL;
    last_asort      = NULL;
    extracted_obj_queue = NULL;
    extracted_char_queue = NULL;
    cur_qobjs       = 0;
    cur_qchars      = 0;
    cur_char        = NULL;
    cur_obj     = 0;
    cur_obj_serial  = 0;
    cur_char_died   = FALSE;
    cur_obj_extracted   = FALSE;
    cur_room        = NULL;
    quitting_char   = NULL;
    loading_char    = NULL;
    saving_char     = NULL;

    for ( wear = 0; wear < MAX_WEAR; wear++ )
        for ( x = 0; x < MAX_LAYERS; x++ )
        {
            save_equipment[0][wear][x] = NULL;
            save_equipment[1][wear][x] = NULL;
        }

    /*
        Init random number generator.
    */
    log_string( "Initializing random number generator" );
    init_mm( );
    /*
        Set time and weather.
    */
    {
        long lhour, lday, lmonth;
        log_string( "Setting time and weather" );
        lhour       = ( current_time - 650336715 )
                      / ( PULSE_TICK / PULSE_PER_SECOND );
        time_info.hour  = lhour  % 24;
        lday        = lhour  / 24;
        time_info.day   = lday   % 35;
        lmonth      = lday   / 35;
        time_info.month = lmonth % 12;
        time_info.year  = lmonth / 12;

        if ( time_info.hour <  5 )
            weather_info.sunlight = SUN_DARK;
        else if ( time_info.hour <  6 )
            weather_info.sunlight = SUN_RISE;
        else if ( time_info.hour < 19 )
            weather_info.sunlight = SUN_LIGHT;
        else if ( time_info.hour < 20 )
            weather_info.sunlight = SUN_SET;
        else
            weather_info.sunlight = SUN_DARK;

        weather_info.change = 0;
        weather_info.mmhg   = 960;

        if ( time_info.month >= 7 && time_info.month <= 12 )
            weather_info.mmhg += number_range( 1, 50 );
        else
            weather_info.mmhg += number_range( 1, 80 );

        if ( weather_info.mmhg <=  980 )
            weather_info.sky = SKY_LIGHTNING;
        else if ( weather_info.mmhg <= 1000 )
            weather_info.sky = SKY_RAINING;
        else if ( weather_info.mmhg <= 1020 )
            weather_info.sky = SKY_CLOUDY;
        else
            weather_info.sky = SKY_CLOUDLESS;
    }
    /*
        Assign gsn's for skills which need them.
    */
    {
        log_string( "Assigning gsn's" );
        ASSIGN_GSN( gsn_lightarmor,        "light armor" );
        ASSIGN_GSN( gsn_friendly_fire,     "friendly fire" );
        ASSIGN_GSN( gsn_basic_medical,     "basic medical" );
        ASSIGN_GSN( gsn_eagle_eye,         "eagle eye" );
        ASSIGN_GSN( gsn_sharpshooting,     "sharpshooting" );
        ASSIGN_GSN( gsn_handweapons,       "hand weapons" );
        ASSIGN_GSN( gsn_alertness,         "alertness" );
        ASSIGN_GSN( gsn_awareness,         "awareness" );
        ASSIGN_GSN( gsn_mediumarmor,       "armor (medium)" );
        ASSIGN_GSN( gsn_electronics,       "electronics" );
        ASSIGN_GSN( gsn_advanced_medical,  "advanced medical" );
        ASSIGN_GSN( gsn_near_miss,         "near miss" );
        ASSIGN_GSN( gsn_demolitions,       "demolitions" );
        ASSIGN_GSN( gsn_heavyarmor,        "armor (heavy)" );
        ASSIGN_GSN( gsn_cartography,       "cartography" );
        ASSIGN_GSN( gsn_logistics,         "logistics" );
        ASSIGN_GSN( gsn_requestassist,     "request assistance" );
        ASSIGN_GSN( gsn_moraleboost,       "morale boost" );
        ASSIGN_GSN( gsn_dodge,             "dodge" );
        ASSIGN_GSN( gsn_evasive,           "evasive" );
        ASSIGN_GSN( gsn_tailslam,          "tail slam" );
        ASSIGN_GSN( gsn_battle_method,     "battle method" );
        ASSIGN_GSN( gsn_alien_rage,        "alien rage" );
        ASSIGN_GSN( gsn_critical_strike,   "critical strike" );
        ASSIGN_GSN( gsn_acidspit,          "acid spit" );
        ASSIGN_GSN( gsn_leap,              "leap" );
        ASSIGN_GSN( gsn_breach,            "breach" );
        ASSIGN_GSN( gsn_stealth,           "stealth" );
        ASSIGN_GSN( gsn_lunge,             "lunge" );
        ASSIGN_GSN( gsn_acid_potency,      "acid potency" );
        ASSIGN_GSN( gsn_headbite,          "head bite" );
        ASSIGN_GSN( gsn_confuse,           "confuse" );
        ASSIGN_GSN( gsn_acute_hearing,     "acute hearing" );
        ASSIGN_GSN( gsn_pursuit,           "pursuit" );
        ASSIGN_GSN( gsn_pheromones,        "pheromones" );
        ASSIGN_GSN( gsn_impale,            "impale" );
        ASSIGN_GSN( gsn_layeggs,           "lay eggs" );
        ASSIGN_GSN( gsn_battle_focus,      "battle focus" );
        ASSIGN_GSN( gsn_track,             "track" );
        ASSIGN_GSN( gsn_vision_modes,      "vision modes" );
        ASSIGN_GSN( gsn_nightvision,       "nightvision" );
        ASSIGN_GSN( gsn_shoulder_cannon,   "shoulder cannon" );
        ASSIGN_GSN( gsn_medical,           "medical" );
        ASSIGN_GSN( gsn_cr_combat,         "close-range combat" );
        ASSIGN_GSN( gsn_tumble,            "tumble" );
        ASSIGN_GSN( gsn_trap_setting,      "trap setting" );
        ASSIGN_GSN( gsn_double_strike,     "double strike" );
        ASSIGN_GSN( gsn_venom_cloud,       "venom cloud module" );
        ASSIGN_GSN( gsn_selfdestruct,      "selfdestruct" );
        ASSIGN_GSN( gsn_rage,              "rage" );
        ASSIGN_GSN( gsn_ranged_combat,     "ranged combat" );
        ASSIGN_GSN( gsn_primal_instinct,   "primal instinct" );
        ASSIGN_GSN( gsn_sixth_sense,       "sixth sense" );
        ASSIGN_GSN( gsn_battlecry,         "battlecry" );
        ASSIGN_GSN( gsn_human_tech,        "human technology" );
        ASSIGN_GSN( gsn_marine,         "marine" );
        ASSIGN_GSN( gsn_alien,          "alien" );
        ASSIGN_GSN( gsn_predator,       "predator" );
    }
    /*
        Read in all the area files.
    */
    {
        FILE* fpList;
        log_string( "Reading in area files..." );

        if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
        {
            log_string( "<-----> AREA-REGENERATION RUNNING <----->" );

            if ( ( fpList = fopen( AREA_LIST, "w" ) ) == NULL )
            {
                log_string( "Auto-Regeneration Failed... Shutting Down" );
                exit( 1 );
            }

            fprintf( fpList, "help.are\n" );
            fprintf( fpList, "limbo.are\n" );
            fprintf( fpList, "$\n" );
            regenerate_limbo();
            log_string( "Regeneration Complete. Please reboot mud for settings to take effect." );
            exit( 1 );
            /*
                shutdown_mud( "Unable to open area list" );
                exit( 1 );
            */
        }
        else
        {
            for ( ; ; )
            {
                strcpy( strArea, fread_word( fpList ) );

                if ( strArea[0] == '$' )
                    break;

                // sprintf( tmp, "***[ALERT]: Loading %s", strArea );
                // log_string( tmp );
                load_area_file( last_area, strArea );
            }

            fclose( fpList );
        }
    }
    /*
         initialize supermob.
          must be done before reset_area!

    */
    init_supermob();
    /*
        Fix up exits.
        Declare db booting over.
        Reset all areas once.
        Load up the notes file.
    */
    {
        log_string( "Fixing exits" );
        fix_exits( );
        fBootDb = FALSE;
        log_string( "Loading buildlist" );
        load_buildlist( );
        log_string( "Loading boards" );
        load_boards( );
        log_string( "Loading bans" );
        load_banlist( );
        log_string( "Loading corpses" );
        load_corpses( );
        log_string ( "Loading multiplayers" );
        load_mplist( );
        log_string( "Loading arenas" );
        load_arenas( );
        log_string( "Loading bots" );
        load_bots( );
        log_string( "Deploying area maps" );
        deploy_map( );
        // log_string( "Booting TNS" );
        // boot_tns( );
        log_string( "Resetting areas" );
        area_update( );
        log_string( "Loading DNS cache..." ); /* Samson 1-30-02 */
        load_dns();
        /* Load Equipment rooms on reboot - Ghost */
        load_equip_rooms ( );
        MOBtrigger = TRUE;
    }

    /* init_maps ( ); */

    if ( fCpyOver )
        copyover_recover();

    return;
}



/*
    Load an 'area' header line.
*/
void load_area( FILE* fp )
{
    AREA_DATA* pArea;
    CREATE( pArea, AREA_DATA, 1 );
    pArea->first_reset  = NULL;
    pArea->last_reset   = NULL;
    pArea->first_var    = NULL;
    pArea->last_var     = NULL;
    pArea->name     = fread_string_nohash( fp );
    pArea->author       = STRALLOC( "unknown" );
    pArea->filename = str_dup( strArea );
    pArea->ambience     = 0;
    pArea->version      = 1;
    pArea->age      = 15;
    pArea->nplayer  = 0;
    pArea->low_r_vnum   = 0;
    pArea->low_o_vnum   = 0;
    pArea->low_m_vnum   = 0;
    pArea->hi_r_vnum    = 0;
    pArea->hi_o_vnum    = 0;
    pArea->hi_m_vnum    = 0;
    pArea->low_soft_range = 0;
    pArea->hi_soft_range  = MAX_LEVEL;
    pArea->low_hard_range = 0;
    pArea->hi_hard_range  = MAX_LEVEL;
    LINK( pArea, first_area, last_area, next, prev );
    top_area++;
    return;
}


/*
    Load an author section. Scryn 2/1/96
*/
void load_author( AREA_DATA* tarea, FILE* fp )
{
    if ( !tarea )
    {
        bug( "Load_author: no #AREA seen yet." );

        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    if ( tarea->author )
        STRFREE( tarea->author );

    tarea->author   = fread_string( fp );
    return;
}

/*
    Load an ambience section. Ghost
*/
void load_ambience( AREA_DATA* tarea, FILE* fp )
{
    if ( !tarea )
    {
        bug( "Load_ambience: no #AREA seen yet." );

        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    tarea->ambience = fread_number( fp );
    return;
}

/*
    Load an version section. Ghost
*/
void load_version( AREA_DATA* tarea, FILE* fp )
{
    if ( !tarea )
    {
        bug( "Load_version: no #AREA seen yet." );

        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    tarea->version = fread_number( fp );
    return;
}

/* Reset Message Load, Rennard */
void load_resetmsg( AREA_DATA* tarea, FILE* fp )
{
    if ( !tarea )
    {
        bug( "Load_resetmsg: no #AREA seen yet." );

        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    if ( tarea->resetmsg )
        DISPOSE( tarea->resetmsg );

    tarea->resetmsg = fread_string_nohash( fp );
    return;
}

/*
    Load area flags. Narn, Mar/96
*/
void load_flags( AREA_DATA* tarea, FILE* fp )
{
    char* ln;
    int x1, x2;

    if ( !tarea )
    {
        bug( "Load_flags: no #AREA seen yet." );

        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    tarea->flags = fread_bitvector( fp );
    ln = fread_line( fp );
    x1 = x2 = 0;
    sscanf( ln, "%d",
            &x1 );
    tarea->reset_frequency = x1;

    if ( x1 )
        tarea->age = x1;

    return;
}

/*
    Adds a help page to the list if it is not a duplicate of an existing page.
    Page is insert-sorted by keyword.            -Thoric
    (The reason for sorting is to keep do_hlist looking nice)
*/
void add_help( HELP_DATA* pHelp )
{
    HELP_DATA* tHelp;
    int match;

    for ( tHelp = first_help; tHelp; tHelp = tHelp->next )
        if ( pHelp->level == tHelp->level
                &&   strcmp( pHelp->keyword, tHelp->keyword ) == 0 )
        {
            bug( "add_help: duplicate: %s.  Deleting.", pHelp->keyword );
            STRFREE( pHelp->text );
            STRFREE( pHelp->keyword );
            DISPOSE( pHelp );
            return;
        }
        else if ( ( match = strcmp( pHelp->keyword[0] == '\'' ? pHelp->keyword + 1 : pHelp->keyword,
                                    tHelp->keyword[0] == '\'' ? tHelp->keyword + 1 : tHelp->keyword ) ) < 0
                  ||   ( match == 0 && pHelp->level > tHelp->level ) )
        {
            if ( !tHelp->prev )
                first_help    = pHelp;
            else
                tHelp->prev->next = pHelp;

            pHelp->prev       = tHelp->prev;
            pHelp->next       = tHelp;
            tHelp->prev       = pHelp;
            break;
        }

    if ( !tHelp )
        LINK( pHelp, first_help, last_help, next, prev );

    top_help++;
}

/*
    Load a help section.
*/
void load_helps( AREA_DATA* tarea, FILE* fp )
{
    HELP_DATA* pHelp;

    for ( ; ; )
    {
        CREATE( pHelp, HELP_DATA, 1 );
        pHelp->level    = fread_number( fp );
        pHelp->keyword  = fread_string( fp );

        if ( pHelp->keyword[0] == '$' )
            break;

        pHelp->text = fread_string( fp );

        if ( pHelp->keyword[0] == '\0' )
        {
            STRFREE( pHelp->text );
            STRFREE( pHelp->keyword );
            DISPOSE( pHelp );
            continue;
        }

        if ( !str_cmp( pHelp->keyword, "greeting" ) )
            help_greeting = pHelp->text;

        add_help( pHelp );
    }

    return;
}


/*
    Add a character to the list of all characters
*/
void add_char( CHAR_DATA* ch )
{
    LINK( ch, first_char, last_char, next, prev );
}

/*
    Remove a character from the list of all characters
*/
void remove_char( CHAR_DATA* ch )
{
    UNLINK( ch, first_char, last_char, next, prev );
}


/*
    Load a mob section.
*/
void load_mobiles( AREA_DATA* tarea, FILE* fp )
{
    MOB_INDEX_DATA* pMobIndex;
    char* ln;
    int x1, x2, x3, x4, x5, x6, x7, x8;

    if ( !tarea )
    {
        bug( "Load_mobiles: no #AREA seen yet." );

        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    for ( ; ; )
    {
        char buf[MAX_STRING_LENGTH];
        int vnum;
        char letter;
        int iHash;
        bool oldmob;
        bool tmpBootDb;
        letter              = fread_letter( fp );

        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found." );

            if ( fBootDb )
            {
                shutdown_mud( "# not found" );
                exit( 1 );
            }
            else
                return;
        }

        vnum                = fread_number( fp );

        if ( vnum == 0 )
            break;

        tmpBootDb = fBootDb;
        fBootDb = FALSE;

        if ( get_mob_index( vnum ) )
        {
            if ( tmpBootDb )
            {
                bug( "Load_mobiles: vnum %d duplicated.", vnum );
                shutdown_mud( "duplicate vnum" );
                exit( 1 );
            }
            else
            {
                pMobIndex = get_mob_index( vnum );
                sprintf( buf, "Cleaning mobile: %d", vnum );
                log_string_plus( buf, LOG_BUILD, sysdata.log_level );
                clean_mob( pMobIndex );
                oldmob = TRUE;
            }
        }
        else
        {
            oldmob = FALSE;
            CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
        }

        fBootDb = tmpBootDb;
        pMobIndex->vnum         = vnum;

        if ( fBootDb )
        {
            if ( !tarea->low_m_vnum )
                tarea->low_m_vnum   = vnum;

            if ( vnum > tarea->hi_m_vnum )
                tarea->hi_m_vnum    = vnum;
        }

        pMobIndex->player_name      = fread_string( fp );
        pMobIndex->short_descr      = fread_string( fp );
        pMobIndex->long_descr       = fread_string( fp );
        pMobIndex->description      = fread_string( fp );
        pMobIndex->long_descr[0]    = UPPER( pMobIndex->long_descr[0] );
        pMobIndex->description[0]   = UPPER( pMobIndex->description[0] );
        pMobIndex->affected_by  = fread_bitvector( fp );
        pMobIndex->act          = fread_bitvector( fp );
        pMobIndex->pShop        = NULL;
        letter              = fread_letter( fp );
        pMobIndex->level        = fread_number( fp );

        if ( tarea->version <= 1 )
        {
            // Damnumdice and Hitnumdice Tables.
            fread_number( fp );
            fread_number( fp );
            fread_number( fp );
            fread_letter( fp );
            fread_number( fp );
            fread_letter( fp );
            fread_number( fp );
            fread_number( fp );
            fread_letter( fp );
            fread_number( fp );
            fread_letter( fp );
            fread_number( fp );
        }

        pMobIndex->position     = fread_number( fp );
        pMobIndex->defposition      = fread_number( fp );
        /*
            Back to meaningful values.
        */
        pMobIndex->sex          = fread_number( fp );

        if ( letter != 'S' && letter != 'C' && letter != 'Z' )
        {
            bug( "Load_mobiles: vnum %d: letter '%c' not Z, S or C.", vnum,
                 letter );
            shutdown_mud( "bad mob data" );
            exit( 1 );
        }

        if ( letter == 'C' || letter == 'Z' ) /* complex mob     -Ghost  */
        {
            pMobIndex->perm_str         = fread_number( fp );
            pMobIndex->perm_sta                 = fread_number( fp );
            pMobIndex->perm_rec                 = fread_number( fp );
            pMobIndex->perm_int                 = fread_number( fp );
            pMobIndex->perm_bra                 = fread_number( fp );
            pMobIndex->perm_per                 = fread_number( fp );
            ln = fread_line( fp );
            x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
            sscanf( ln, "%d %d %d %d",
                    &x1, &x2, &x3, &x4 );
            pMobIndex->race     = x1;
            pMobIndex->height       = x3;
            pMobIndex->weight       = x4;
            pMobIndex->speaks = fread_bitvector( fp );
            pMobIndex->speaking = fread_bitvector( fp );

            if ( tarea->version <= 1 )
            {
                fread_line( fp ); // Dispose of Damroll
            }
        }
        else
        {
            pMobIndex->perm_str     = 10;
            pMobIndex->perm_sta         = 10;
            pMobIndex->perm_rec         = 10;
            pMobIndex->perm_int         = 10;
            pMobIndex->perm_bra         = 10;
            pMobIndex->perm_per         = 10;
            pMobIndex->race     = 0;
        }

        if ( letter == 'Z' ) /*  Star Wars Reality Complex Mob  */
        {
            fread_bitvector( fp );
            ln = fread_line( fp );
            x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
            sscanf( ln, "%d %d %d %d %d %d",
                    &x1, &x2, &x3, &x4, &x5, &x6 );
        }

        letter = fread_letter( fp );

        if ( letter == '>' )
        {
            ungetc( letter, fp );
            mprog_read_programs( fp, pMobIndex );
        }
        else
            ungetc( letter, fp );

        if ( !oldmob )
        {
            iHash           = vnum % MAX_KEY_HASH;
            pMobIndex->next     = mob_index_hash[iHash];
            mob_index_hash[iHash]   = pMobIndex;
            top_mob_index++;
        }
    }

    return;
}



/*
    Load an obj section.
*/
void load_objects( AREA_DATA* tarea, FILE* fp )
{
    OBJ_INDEX_DATA* pObjIndex;
    char letter;
    char* ln;
    int x1, x2, x3, x4, x5, x6, x7;

    if ( !tarea )
    {
        bug( "Load_objects: no #AREA seen yet." );

        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    for ( ; ; )
    {
        char buf[MAX_STRING_LENGTH];
        int vnum;
        int iHash;
        bool tmpBootDb;
        bool oldobj;
        letter              = fread_letter( fp );

        if ( letter != '#' )
        {
            bug( "Load_objects: # not found." );

            if ( fBootDb )
            {
                shutdown_mud( "# not found" );
                exit( 1 );
            }
            else
                return;
        }

        vnum                = fread_number( fp );

        if ( vnum == 0 )
            break;

        tmpBootDb = fBootDb;
        fBootDb = FALSE;

        if ( get_obj_index( vnum ) )
        {
            if ( tmpBootDb )
            {
                bug( "Load_objects: vnum %d duplicated.", vnum );
                shutdown_mud( "duplicate vnum" );
                exit( 1 );
            }
            else
            {
                pObjIndex = get_obj_index( vnum );
                sprintf( buf, "Cleaning object: %d", vnum );
                log_string_plus( buf, LOG_BUILD, sysdata.log_level );
                clean_obj( pObjIndex );
                oldobj = TRUE;
            }
        }
        else
        {
            oldobj = FALSE;
            CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
        }

        fBootDb = tmpBootDb;
        pObjIndex->vnum         = vnum;

        if ( fBootDb )
        {
            if ( !tarea->low_o_vnum )
                tarea->low_o_vnum       = vnum;

            if ( vnum > tarea->hi_o_vnum )
                tarea->hi_o_vnum        = vnum;
        }

        pObjIndex->name         = fread_string( fp );
        pObjIndex->short_descr      = fread_string( fp );
        pObjIndex->description      = fread_string( fp );
        pObjIndex->action_desc      = fread_string( fp );
        /*  Commented out by Narn, Apr/96 to allow item short descs like
            Bonecrusher and Oblivion */
        /*pObjIndex->short_descr[0] = LOWER(pObjIndex->short_descr[0]);*/
        pObjIndex->description[0]   = UPPER( pObjIndex->description[0] );
        ln = fread_line( fp );
        x1 = x2 = x3 = x4 = x5 = 0;
        sscanf( ln, "%d %d", &x1, &x2 );
        pObjIndex->item_type        = x1;
        pObjIndex->layers           = x2;
        pObjIndex->extra_flags      = fread_bitvector( fp );
        pObjIndex->wear_flags       = fread_bitvector( fp );
        ln = fread_line( fp );
        x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
        sscanf( ln, "%d %d %d %d %d %d",
                &x1, &x2, &x3, &x4, &x5, &x6 );
        pObjIndex->value[0]     = x1;
        pObjIndex->value[1]     = x2;
        pObjIndex->value[2]     = x3;
        pObjIndex->value[3]     = x4;
        pObjIndex->value[4]     = x5;
        pObjIndex->value[5]     = x6;
        pObjIndex->weight       = fread_number( fp );
        pObjIndex->weight = UMAX( 1, pObjIndex->weight );
        pObjIndex->cost         = fread_number( fp );
        pObjIndex->rent         = fread_number( fp ); /* unused */

        for ( ; ; )
        {
            letter = fread_letter( fp );

            if ( letter == 'A' )
            {
                AFFECT_DATA* paf;
                CREATE( paf, AFFECT_DATA, 1 );
                paf->type       = -1;
                paf->duration       = -1;
                paf->location       = fread_number( fp );
                paf->modifier         = fread_number( fp );
                xCLEAR_BITS( paf->bitvector );
                LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
                top_affect++;
            }
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA* ed;
                CREATE( ed, EXTRA_DESCR_DATA, 1 );
                ed->keyword     = fread_string( fp );
                ed->description     = fread_string( fp );
                LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
                      next, prev );
                top_ed++;
            }
            else if ( letter == '>' )
            {
                ungetc( letter, fp );
                oprog_read_programs( fp, pObjIndex );
            }
            else
            {
                ungetc( letter, fp );
                break;
            }
        }

        if ( !oldobj )
        {
            iHash         = vnum % MAX_KEY_HASH;
            pObjIndex->next   = obj_index_hash[iHash];
            obj_index_hash[iHash] = pObjIndex;
            top_obj_index++;
        }
    }

    return;
}



/*
    Load a reset section.
*/
void load_resets( AREA_DATA* tarea, FILE* fp )
{
    char buf[MAX_STRING_LENGTH];
    bool not01 = FALSE;
    int count = 0;

    if ( !tarea )
    {
        bug( "Load_resets: no #AREA seen yet." );

        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    if ( tarea->first_reset )
    {
        if ( fBootDb )
        {
            RESET_DATA* rtmp;
            bug( "load_resets: WARNING: resets already exist for this area." );

            for ( rtmp = tarea->first_reset; rtmp; rtmp = rtmp->next )
                ++count;
        }
        else
        {
            /*
                Clean out the old resets
            */
            sprintf( buf, "Cleaning resets: %s", tarea->name );
            log_string_plus( buf, LOG_BUILD, sysdata.log_level );
            clean_resets( tarea );
        }
    }

    for ( ; ; )
    {
        /* ROOM_INDEX_DATA *pRoomIndex; */
        /* EXIT_DATA *pexit; */
        char letter;
        int extra, arg1, arg2, arg3;

        if ( ( letter = fread_letter( fp ) ) == 'S' )
            break;

        if ( letter == '*' )
        {
            fread_to_eol( fp );
            continue;
        }

        extra   = fread_number( fp );
        arg1    = fread_number( fp );
        arg2    = fread_number( fp );
        arg3    = ( letter == 'G' || letter == 'R' )
                  ? 0 : fread_number( fp );
        fread_to_eol( fp );
        ++count;
        /*
            Validate parameters.
            We're calling the index functions for the side effect.
                ----
               Due to issues with this feature, it had to be
               disabled temporarly. A inate-tendancy to freeze-up
               was deemed VERY annoying.  -Ghost
        */
        /*
            switch ( letter )
            {
            default:
            bug( "Load_resets: bad command '%c'.", letter );
            if ( fBootDb )
              boot_log( "Load_resets: %s (%d) bad command '%c'.", tarea->filename, count, letter );
            return;

            case 'M':
            if ( get_mob_index( arg1 ) == NULL && fBootDb )
            boot_log( "Load_resets: %s (%d) 'M': mobile %d doesn't exist.",
               tarea->filename, count, arg1 );
            if ( get_room_index( arg3 ) == NULL && fBootDb )
            boot_log( "Load_resets: %s (%d) 'M': room %d doesn't exist.",
               tarea->filename, count, arg3 );
            break;

            case 'O':
            if ( get_obj_index(arg1) == NULL && fBootDb )
            boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
               tarea->filename, count, letter, arg1 );
            if ( get_room_index(arg3) == NULL && fBootDb )
            boot_log( "Load_resets: %s (%d) '%c': room %d doesn't exist.",
               tarea->filename, count, letter, arg3 );
            break;

            case 'P':
            if ( get_obj_index(arg1) == NULL && fBootDb )
            boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
               tarea->filename, count, letter, arg1 );
            if ( arg3 > 0 ) {
            if ( get_obj_index(arg3) == NULL && fBootDb )
               boot_log( "Load_resets: %s (%d) 'P': destination object %d doesn't exist.",
               tarea->filename, count, arg3 );
            }
            else if ( extra > 1 )
              not01 = TRUE;
            break;

            case 'G':
            case 'E':
            if ( get_obj_index(arg1) == NULL && fBootDb )
            boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
               tarea->filename, count, letter, arg1 );
            break;

            case 'T':
            break;

            case 'H':
            if ( arg1 > 0 )
            if ( get_obj_index(arg1) == NULL && fBootDb )
               boot_log( "Load_resets: %s (%d) 'H': object %d doesn't exist.",
               tarea->filename, count, arg1 );
            break;

            case 'D':
            pRoomIndex = get_room_index( arg1 );
            if ( !pRoomIndex )
            {
            bug( "Load_resets: 'D': room %d doesn't exist.", arg1 );
            bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
               arg3 );
            if ( fBootDb )
              boot_log( "Load_resets: %s (%d) 'D': room %d doesn't exist.",
               tarea->filename, count, arg1 );
            break;
            }

            if ( arg2 < 0
            ||   arg2 > MAX_DIR+1
            || ( pexit = get_exit(pRoomIndex, arg2)) == NULL
               || !xIS_SET( pexit->exit_info, EX_ISDOOR ) )
            {
            bug( "Load_resets: 'D': exit %d not door.", arg2 );
            bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
               arg3 );
            if ( fBootDb )
              boot_log( "Load_resets: %s (%d) 'D': exit %d not door.",
               tarea->filename, count, arg2 );
            }

            if ( arg3 < 0 || arg3 > 2 )
            {
            bug( "Load_resets: 'D': bad 'locks': %d.", arg3 );
            if ( fBootDb )
             boot_log( "Load_resets: %s (%d) 'D': bad 'locks': %d.",
               tarea->filename, count, arg3 );
            }
            break;

            case 'R':
            pRoomIndex = get_room_index( arg1 );
            if ( !pRoomIndex && fBootDb )
            boot_log( "Load_resets: %s (%d) 'R': room %d doesn't exist.",
               tarea->filename, count, arg1 );

            if ( arg2 < 0 || arg2 > 6 )
            {
            bug( "Load_resets: 'R': bad exit %d.", arg2 );
            if ( fBootDb )
             boot_log( "Load_resets: %s (%d) 'R': bad exit %d.",
               tarea->filename, count, arg2 );
            break;
            }

            break;
            }
        */
        /* finally, add the reset */
        add_reset( tarea, letter, extra, arg1, arg2, arg3 );
    }

    if ( !not01 )
        renumber_put_resets( tarea );

    return;
}



/*
    Load a room section.
*/
void load_rooms( AREA_DATA* tarea, FILE* fp )
{
    ROOM_INDEX_DATA* pRoomIndex;
    char buf[MAX_STRING_LENGTH];
    char* ln;

    if ( !tarea )
    {
        bug( "Load_rooms: no #AREA seen yet." );
        shutdown_mud( "No #AREA" );
        exit( 1 );
    }

    for ( ; ; )
    {
        int vnum;
        char letter;
        int door;
        int iHash;
        bool tmpBootDb;
        bool oldroom;
        int x1, x2, x3, x4, x5, x6;
        letter              = fread_letter( fp );

        if ( letter != '#' )
        {
            bug( "Load_rooms: # not found." );

            if ( fBootDb )
            {
                shutdown_mud( "# not found" );
                exit( 1 );
            }
            else
                return;
        }

        vnum                = fread_number( fp );

        if ( vnum == 0 )
            break;

        tmpBootDb = fBootDb;
        fBootDb = FALSE;

        if ( get_room_index( vnum ) != NULL )
        {
            if ( tmpBootDb )
            {
                bug( "Load_rooms: vnum %d duplicated.", vnum );
                shutdown_mud( "duplicate vnum" );
                exit( 1 );
            }
            else
            {
                pRoomIndex = get_room_index( vnum );
                sprintf( buf, "Cleaning room: %d", vnum );
                log_string_plus( buf, LOG_BUILD, sysdata.log_level );
                clean_room( pRoomIndex );
                oldroom = TRUE;
            }
        }
        else
        {
            oldroom = FALSE;
            CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
            pRoomIndex->first_person  = NULL;
            pRoomIndex->last_person   = NULL;
            pRoomIndex->first_content = NULL;
            pRoomIndex->last_content  = NULL;
            pRoomIndex->link              = NULL;
        }

        fBootDb = tmpBootDb;
        pRoomIndex->area        = tarea;
        pRoomIndex->vnum        = vnum;
        pRoomIndex->first_extradesc = NULL;
        pRoomIndex->last_extradesc  = NULL;

        if ( fBootDb )
        {
            if ( !tarea->low_r_vnum )
                tarea->low_r_vnum       = vnum;

            if ( vnum > tarea->hi_r_vnum )
                tarea->hi_r_vnum        = vnum;
        }

        pRoomIndex->name        = fread_string( fp );
        pRoomIndex->description     = fread_string( fp );

        if ( tarea->version >= 3 )
        {
            pRoomIndex->hdescription         = fread_string( fp );
        }
        else
        {
            pRoomIndex->hdescription         = STRALLOC( "" );
        }

        /* Area number                    fread_number( fp ); */
        pRoomIndex->room_flags      = fread_bitvector( fp );

        if ( tarea->version >= 4 )
        {
            pRoomIndex->hroom_flags = fread_bitvector( fp );
        }
        else
        {
            xCLEAR_BITS( pRoomIndex->hroom_flags );
        }

        ln = fread_line( fp );
        x1 = x2 = x3 = x4 = x5 = x6 = 0;
        sscanf( ln, "%d %d %d %d %d",
                &x1, &x2, &x3, &x4, &x5 );
        pRoomIndex->sector_type     = x2;
        pRoomIndex->tele_delay      = x3;
        pRoomIndex->tele_vnum       = x4;
        pRoomIndex->tunnel      = x5;

        if ( pRoomIndex->sector_type < 0 || pRoomIndex->sector_type == SECT_MAX )
        {
            bug( "Fread_rooms: vnum %d has bad sector_type %d.", vnum,
                 pRoomIndex->sector_type );
            pRoomIndex->sector_type = 1;
        }

        pRoomIndex->light       = 0;
        pRoomIndex->first_exit      = NULL;
        pRoomIndex->last_exit       = NULL;

        for ( ; ; )
        {
            letter = fread_letter( fp );

            if ( letter == 'S' )
                break;

            if ( letter == 'D' )
            {
                EXIT_DATA* pexit;
                /* EXT_BV locks; */
                door = fread_number( fp );

                if ( door < 0 || door > 10 )
                {
                    bug( "Fread_rooms: vnum %d has bad door number %d.", vnum,
                         door );

                    if ( fBootDb )
                        exit( 1 );
                }
                else
                {
                    pexit = make_exit( pRoomIndex, NULL, door );
                    pexit->description    = fread_string( fp );
                    pexit->keyword    = fread_string( fp );
                    xCLEAR_BITS( pexit->exit_info );
                    pexit->exit_info = fread_bitvector( fp );
                    ln = fread_line( fp );
                    x1 = x2 = x3 = x4 = 0;
                    sscanf( ln, "%d %d %d",
                            &x1, &x2, &x3 );
                    pexit->key        = x1;
                    pexit->vnum       = x2;
                    pexit->vdir       = door;
                    pexit->distance   = x3;
                }
            }
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA* ed;
                CREATE( ed, EXTRA_DESCR_DATA, 1 );
                ed->keyword     = fread_string( fp );
                ed->description     = fread_string( fp );
                LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc,
                      next, prev );
                top_ed++;
            }
            else if ( letter == '>' )
            {
                ungetc( letter, fp );
                rprog_read_programs( fp, pRoomIndex );
            }
            else
            {
                bug( "Load_rooms: vnum %d has flag '%c' not 'DES'.", vnum,
                     letter );
                shutdown_mud( "Room flag not DES" );
                exit( 1 );
            }
        }

        if ( !oldroom )
        {
            iHash          = vnum % MAX_KEY_HASH;
            pRoomIndex->next   = room_index_hash[iHash];
            room_index_hash[iHash] = pRoomIndex;
            top_room++;
        }
    }

    return;
}



/*
    Load a shop section.
*/
void load_shops( AREA_DATA* tarea, FILE* fp )
{
    SHOP_DATA* pShop;

    for ( ; ; )
    {
        MOB_INDEX_DATA* pMobIndex;
        CREATE( pShop, SHOP_DATA, 1 );
        pShop->keeper       = fread_number( fp );

        if ( pShop->keeper == 0 )
            break;

        fread_to_eol( fp );
        pMobIndex       = get_mob_index( pShop->keeper );
        pMobIndex->pShop    = pShop;

        if ( !first_shop )
            first_shop      = pShop;
        else
            last_shop->next = pShop;

        pShop->next     = NULL;
        pShop->prev     = last_shop;
        last_shop       = pShop;
        top_shop++;
    }

    return;
}

/*
    Load spec proc declarations.
*/
void load_specials( AREA_DATA* tarea, FILE* fp )
{
    for ( ; ; )
    {
        MOB_INDEX_DATA* pMobIndex;
        char letter;

        switch ( letter = fread_letter( fp ) )
        {
            default:
                bug( "Load_specials: letter '%c' not *MS.", letter );
                exit( 1 );

            case 'C':
                pMobIndex       = get_mob_index ( fread_number ( fp ) );
                break;

            case 'S':
                return;

            case '*':
                break;

            case 'M':
                pMobIndex       = get_mob_index ( fread_number ( fp ) );

                if ( !pMobIndex->spec_fun )
                {
                    pMobIndex->spec_fun  = spec_lookup   ( fread_word   ( fp ) );

                    if ( pMobIndex->spec_fun == 0 )
                    {
                        bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
                        exit( 1 );
                    }
                }
                else if ( !pMobIndex->spec_2 )
                {
                    pMobIndex->spec_2    = spec_lookup   ( fread_word   ( fp ) );

                    if ( pMobIndex->spec_2 == 0 )
                    {
                        bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
                        exit( 1 );
                    }
                }
                else if ( !pMobIndex->spec_3 )
                {
                    pMobIndex->spec_3    = spec_lookup   ( fread_word   ( fp ) );

                    if ( pMobIndex->spec_3 == 0 )
                    {
                        bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
                        exit( 1 );
                    }
                }
                else if ( !pMobIndex->spec_4 )
                {
                    pMobIndex->spec_4    = spec_lookup   ( fread_word   ( fp ) );

                    if ( pMobIndex->spec_4 == 0 )
                    {
                        bug( "Load_specials: 'M': vnum %d.", pMobIndex->vnum );
                        exit( 1 );
                    }
                }

                break;
        }

        fread_to_eol( fp );
    }
}


/*
    Load soft / hard area ranges.
*/
void load_ranges( AREA_DATA* tarea, FILE* fp )
{
    int x1, x2, x3, x4;
    char* ln;

    if ( !tarea )
    {
        bug( "Load_ranges: no #AREA seen yet." );
        shutdown_mud( "No #AREA" );
        exit( 1 );
    }

    for ( ; ; )
    {
        ln = fread_line( fp );

        if ( ln[0] == '$' )
            break;

        x1 = x2 = x3 = x4 = 0;
        sscanf( ln, "%d %d %d %d",
                &x1, &x2, &x3, &x4 );
        tarea->low_soft_range = x1;
        tarea->hi_soft_range = x2;
        tarea->low_hard_range = x3;
        tarea->hi_hard_range = x4;
    }

    return;
}

/*
    Translate all room exits from virtual to real.
    Has to be done after all rooms are read in.
    Check for bad reverse exits.
*/
void fix_exits( void )
{
    ROOM_INDEX_DATA* pRoomIndex;
    EXIT_DATA* pexit, *pexit_next, *rev_exit;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex  = room_index_hash[iHash];
                pRoomIndex;
                pRoomIndex  = pRoomIndex->next )
        {
            bool fexit;
            fexit = FALSE;

            for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit_next )
            {
                pexit_next = pexit->next;
                pexit->rvnum = pRoomIndex->vnum;

                if ( pexit->vnum <= 0
                        ||  ( pexit->to_room = get_room_index( pexit->vnum ) ) == NULL )
                {
                    if ( fBootDb )
                        boot_log( "Fix_exits: room %d, exit %s leads to bad vnum (%d)",
                                  pRoomIndex->vnum, dir_name[pexit->vdir], pexit->vnum );

                    bug( "Deleting %s exit in room %d", dir_name[pexit->vdir],
                         pRoomIndex->vnum );
                    extract_exit( pRoomIndex, pexit );
                }
                else
                    fexit = TRUE;
            }

            if ( !fexit )
                xSET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
        }
    }

    /* Set all the rexit pointers   -Thoric */
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex  = room_index_hash[iHash];
                pRoomIndex;
                pRoomIndex  = pRoomIndex->next )
        {
            for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
            {
                if ( pexit->to_room && !pexit->rexit )
                {
                    rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );

                    if ( rev_exit )
                    {
                        pexit->rexit    = rev_exit;
                        rev_exit->rexit = pexit;
                    }
                }
            }
        }
    }

    return;
}


/*
    Get diku-compatable exit by number               -Thoric
*/
EXIT_DATA* get_exit_number( ROOM_INDEX_DATA* room, int xit )
{
    EXIT_DATA* pexit;
    int count;
    count = 0;

    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        if ( ++count == xit )
            return pexit;

    return NULL;
}

/*
    (prelude...) This is going to be fun... NOT!
    (conclusion) QSort is f*cked!
*/
int exit_comp( EXIT_DATA** xit1, EXIT_DATA** xit2 )
{
    int d1, d2;
    d1 = ( *xit1 )->vdir;
    d2 = ( *xit2 )->vdir;

    if ( d1 < d2 )
        return -1;

    if ( d1 > d2 )
        return 1;

    return 0;
}

void sort_exits( ROOM_INDEX_DATA* room )
{
    EXIT_DATA* pexit; /* *texit */ /* Unused */
    EXIT_DATA* exits[MAX_REXITS];
    int x, nexits;
    nexits = 0;

    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
    {
        exits[nexits++] = pexit;

        if ( nexits > MAX_REXITS )
        {
            bug( "sort_exits: more than %d exits in room... fatal", nexits );
            return;
        }
    }

    qsort( &exits[0], nexits, sizeof( EXIT_DATA* ),
           ( int( * )( const void*, const void* ) ) exit_comp );

    for ( x = 0; x < nexits; x++ )
    {
        if ( x > 0 )
            exits[x]->prev    = exits[x - 1];
        else
        {
            exits[x]->prev    = NULL;
            room->first_exit  = exits[x];
        }

        if ( x >= ( nexits - 1 ) )
        {
            exits[x]->next    = NULL;
            room->last_exit   = exits[x];
        }
        else
            exits[x]->next    = exits[x + 1];
    }
}

void randomize_exits( ROOM_INDEX_DATA* room, sh_int maxdir )
{
    EXIT_DATA* pexit;
    int nexits, /* maxd, */ d0, d1, count, door; /* Maxd unused */
    int vdirs[MAX_REXITS];
    nexits = 0;

    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        vdirs[nexits++] = pexit->vdir;

    for ( d0 = 0; d0 < nexits; d0++ )
    {
        if ( vdirs[d0] > maxdir )
            continue;

        count = 0;

        while ( vdirs[( d1 = number_range( d0, nexits - 1 ) )] > maxdir
                ||      ++count > 5 );

        if ( vdirs[d1] > maxdir )
            continue;

        door        = vdirs[d0];
        vdirs[d0]   = vdirs[d1];
        vdirs[d1]   = door;
    }

    count = 0;

    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        pexit->vdir = vdirs[count++];

    sort_exits( room );
}


/*
    Repopulate areas periodically.
*/
void area_update( void )
{
    AREA_DATA* pArea;

    for ( pArea = first_area; pArea; pArea = pArea->next )
    {
        CHAR_DATA* pch;
        int reset_age = pArea->reset_frequency ? pArea->reset_frequency : 15;

        if ( ( reset_age == -1 && pArea->age == -1 )
                ||    ++pArea->age < ( reset_age - 1 ) )
            continue;

        /*
            Check for PC's.
        */
        if ( pArea->nplayer > 0 && pArea->age == ( reset_age - 1 ) )
        {
            char buf[MAX_STRING_LENGTH];

            /* Rennard */
            if ( pArea->resetmsg )
                sprintf( buf, "&g%s\n\r", pArea->resetmsg );
            else
                strcpy( buf, "&gYou hear some squeaking sounds...\n\r" );

            for ( pch = first_char; pch; pch = pch->next )
            {
                if ( !IS_NPC( pch )
                        &&   IS_AWAKE( pch )
                        &&   pch->in_room
                        &&   pch->in_room->area == pArea )
                {
                    // set_char_color( AT_RESET, pch );
                    // send_to_char( buf, pch );
                }
            }
        }

        /*
            Check age and reset.
            Note: Mud Academy resets every 3 minutes (not 15).
        */
        if ( pArea->nplayer == 0 || pArea->age >= reset_age )
        {
            ROOM_INDEX_DATA* pRoomIndex;

            /* fprintf( stderr, "Resetting: %s\n", pArea->filename ); */
            if ( !xIS_SET( pArea->flags, AFLAG_NORESET ) )
                reset_area( pArea );

            if ( reset_age == -1 )
                pArea->age = -1;
            else
                pArea->age = number_range( 0, reset_age / 5 );

            pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );

            if ( pRoomIndex != NULL && pArea == pRoomIndex->area
                    &&   pArea->reset_frequency == 0 )
                pArea->age = 15 - 3;
        }
    }

    return;
}


/*
    Create an instance of a mobile.
*/
CHAR_DATA* create_mobile( MOB_INDEX_DATA* pMobIndex )
{
    CHAR_DATA* mob;

    if ( !pMobIndex )
    {
        bug( "Create_mobile: NULL pMobIndex." );
        exit( 1 );
    }

    CREATE( mob, CHAR_DATA, 1 );
    clear_char( mob );
    mob->pIndexData     = pMobIndex;
    mob->editor         = NULL;
    mob->name           = QUICKLINK( pMobIndex->player_name );
    mob->short_descr        = QUICKLINK( pMobIndex->short_descr );
    mob->long_descr     = QUICKLINK( pMobIndex->long_descr  );
    mob->description        = QUICKLINK( pMobIndex->description );
    mob->spec_fun       = pMobIndex->spec_fun;
    mob->spec_2     = pMobIndex->spec_2;
    mob->spec_3     = pMobIndex->spec_3;
    mob->spec_4     = pMobIndex->spec_4;
    mob->mpscriptpos        = 0;
    mob->top_level      = number_fuzzy( pMobIndex->level );
    mob->act            = pMobIndex->act;
    mob->affected_by    = pMobIndex->affected_by;
    mob->sex            = pMobIndex->sex;
    mob->main_ability       = 0;
    mob->vision                 = -1;
    mob->vent                   = FALSE;
    mob->was_home               = FALSE;
    mob->block                  = NULL;
    mob->streak                 = 0;
    mob->swarm                  = 0;
    mob->bot                    = NULL;
    mob->was_sentinel           = NULL;
    mob->hit            = mob->max_hit;
    mob->field                  = mob->max_field;
    mob->resin                  = get_max_resin( mob );
    /* lets put things back the way they used to be! -Thoric */
    mob->position       = pMobIndex->position;
    mob->defposition        = pMobIndex->defposition;
    mob->perm_str       = pMobIndex->perm_str;
    mob->perm_sta               = pMobIndex->perm_sta;
    mob->perm_rec               = pMobIndex->perm_rec;
    mob->perm_int               = pMobIndex->perm_int;
    mob->perm_bra               = pMobIndex->perm_bra;
    mob->perm_per               = pMobIndex->perm_per;
    mob->race           = pMobIndex->race;
    mob->height         = pMobIndex->height;
    mob->weight         = pMobIndex->weight;
    mob->speaks         = pMobIndex->speaks;
    mob->speaking       = pMobIndex->speaking;
    /*
        Insert in list.
    */
    add_char( mob );
    pMobIndex->count++;
    nummobsloaded++;
    return mob;
}



/*
    Create an instance of an object.
*/
OBJ_DATA* create_object( OBJ_INDEX_DATA* pObjIndex, int level )
{
    OBJ_DATA* obj;

    if ( !pObjIndex )
    {
        bug( "Create_object: NULL pObjIndex." );
        exit( 1 );
    }

    CREATE( obj, OBJ_DATA, 1 );
    obj->pIndexData = pObjIndex;
    obj->in_room    = NULL;
    obj->parent         = NULL;
    obj->level      = level;
    obj->wear_loc   = -1;
    obj->count      = 1;
    cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
    obj->serial = obj->pIndexData->serial = cur_obj_serial;
    obj->armed_by       = STRALLOC( "" );
    obj->name       = QUICKLINK( pObjIndex->name     );
    obj->short_descr    = QUICKLINK( pObjIndex->short_descr );
    obj->description    = QUICKLINK( pObjIndex->description );
    obj->action_desc    = QUICKLINK( pObjIndex->action_desc );
    obj->ammo           = NULL;
    obj->attach         = NULL;
    obj->item_type  = pObjIndex->item_type;
    obj->extra_flags    = pObjIndex->extra_flags;
    obj->wear_flags = pObjIndex->wear_flags;
    obj->weapon_mode    = default_weapon_mode( obj );
    obj->value[0]   = pObjIndex->value[0];
    obj->value[1]   = pObjIndex->value[1];
    obj->value[2]   = pObjIndex->value[2];
    obj->value[3]   = pObjIndex->value[3];
    obj->value[4]   = pObjIndex->value[4];
    obj->value[5]   = pObjIndex->value[5];
    obj->weight     = pObjIndex->weight;
    obj->cost       = pObjIndex->cost;
    /*
        obj->cost       = number_fuzzy( 10 )
              number_fuzzy( level ) * number_fuzzy( level );
    */

    /*
        Mess with object properties.
    */
    switch ( obj->item_type )
    {
        default:
            bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
            bug( "------------------------>     ", obj->item_type );
            break;

        case ITEM_RADIO:
        case ITEM_BINOCULAR:
        case ITEM_GPS:
        case ITEM_LIGHT:
        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_COVER:
        case ITEM_WEAPON:
        case ITEM_ATTACHMENT:
        case ITEM_BANDAGE:
        case ITEM_C4:
        case ITEM_LAPTOP:
        case ITEM_TOOLKIT:
        case ITEM_STERIL:
        case ITEM_MEDICOMP:
        case ITEM_MEDIKIT:
        case ITEM_FOOD:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_SCRAPS:
        case ITEM_GRENADE:
        case ITEM_FIRE:
        case ITEM_PAPER:
        case ITEM_AMMO:
        case ITEM_SPAWNER:
        case ITEM_MEDSTATION:
        case ITEM_MOTIONB:
        case ITEM_AMMOBOX:
        case ITEM_TOOLCHEST:
        case ITEM_MAPCONSOLE:
        case ITEM_TRAP:
        case ITEM_MSPAWNER:
        case ITEM_DEPLOYER:
        case ITEM_REMOTE:
        case ITEM_SIFT:
        case ITEM_FLARE:
        case ITEM_REGENERATOR:
        case ITEM_SENTRY:
        case ITEM_SCANNON:
        case ITEM_NVGOGGLE:
        case ITEM_LANDMINE:
        case ITEM_MOTION:
        case ITEM_CLOAK:
            break;

        case ITEM_ARMOR:
            if ( obj->value[0] == 0 )
                obj->value[0] = obj->value[1];

            // obj->timer = obj->value[3];
            break;
    }

    LINK( obj, first_object, last_object, next, prev );
    ++pObjIndex->count;
    ++numobjsloaded;
    ++physicalobjects;
    return obj;
}


/*
    Clear a new character.
*/
void clear_char( CHAR_DATA* ch )
{
    ch->editor          = NULL;
    ch->hunting         = NULL;
    ch->fearing         = NULL;
    ch->hating          = NULL;
    ch->name            = NULL;
    ch->short_descr     = NULL;
    ch->long_descr      = NULL;
    ch->description     = NULL;
    ch->next            = NULL;
    ch->prev            = NULL;
    ch->first_carrying      = NULL;
    ch->last_carrying       = NULL;
    ch->next_in_room        = NULL;
    ch->prev_in_room        = NULL;
    ch->switched        = NULL;
    ch->first_affect        = NULL;
    ch->last_affect     = NULL;
    ch->prev_cmd        = NULL;    /* maps */
    ch->last_cmd        = NULL;
    ch->dest_buf        = NULL;
    ch->dest_buf_2      = NULL;
    ch->spare_ptr       = NULL;
    ch->mount           = NULL;
    xCLEAR_BITS( ch->affected_by );
    ch->logon           = current_time;
    ch->ap                      = 0;
    ch->teamkill                = 0;
    ch->morale                  = 0;
    ch->position        = POS_STANDING;
    ch->hit                     = 100;
    ch->max_hit                 = 100;
    ch->move                    = 100;
    ch->max_move                = 100;
    ch->field                   = 100;
    ch->resin                   = 100;
    ch->max_field               = 100;
    ch->height          = 72;
    ch->weight          = 180;
    ch->race            = 0;
    ch->vision                  = -1;
    ch->substate        = 0;
    ch->tempnum         = 0;
    ch->perm_str        = 10;
    ch->perm_sta                = 10;
    ch->perm_rec                = 10;
    ch->perm_int                = 10;
    ch->perm_bra                = 10;
    ch->perm_per                = 10;
    ch->mod_str         = 0;
    ch->mod_sta                 = 0;
    ch->mod_rec                 = 0;
    ch->mod_int                 = 0;
    ch->mod_bra                 = 0;
    ch->mod_per                 = 0;
    ch->pagelen                 = 24;            /* BUILD INTERFACE */
    ch->inter_page      = NO_PAGE;           /* BUILD INTERFACE */
    ch->inter_type      = NO_TYPE;           /* BUILD INTERFACE */
    ch->inter_editing       = NULL;              /* BUILD INTERFACE */
    ch->inter_editing_vnum  = -1;                /* BUILD INTERFACE */
    ch->inter_substate      = SUB_NORTH;         /* BUILD INTERFACE */
    return;
}



/*
    Free a character.
*/
void free_char( CHAR_DATA* ch )
{
    OBJ_DATA* obj;
    AFFECT_DATA* paf;
    TIMER* timer;
    MPROG_ACT_LIST* mpact, *mpact_next;
    NOTE_DATA* comments, *comments_next;

    if ( !ch )
    {
        bug( "Free_char: null ch!" );
        return;
    }

    if ( ch->desc )
        bug( "Free_char: char still has descriptor." );

    while ( ( obj = ch->last_carrying ) != NULL )
        extract_obj( obj );

    while ( ( paf = ch->last_affect ) != NULL )
        affect_remove( ch, paf );

    while ( ( timer = ch->first_timer ) != NULL )
        extract_timer( ch, timer );

    // Shutdown the outstanding MPSLEEP progs
    mpsleep_inspect( ch );
    STRFREE( ch->name       );
    STRFREE( ch->short_descr    );
    STRFREE( ch->long_descr );
    STRFREE( ch->description    );

    if ( ch->editor )
        stop_editing( ch );

    if ( ch->inter_editing )
        DISPOSE( ch->inter_editing );

    stop_hunting( ch );
    stop_hating ( ch );
    stop_fearing( ch );

    if ( ch->pnote )
        free_note( ch->pnote );

    if ( ch->pcdata )
    {
        DISPOSE( ch->pcdata->pwd        );  /* no hash */
        DISPOSE( ch->pcdata->email  );  /* no hash */
        DISPOSE( ch->pcdata->bamfin );  /* no hash */
        DISPOSE( ch->pcdata->bamfout    );  /* no hash */
        DISPOSE( ch->pcdata->rank   );
        STRFREE( ch->pcdata->title  );
        STRFREE( ch->pcdata->bio    );

        if ( ch->pcdata->gname )
            DISPOSE( ch->pcdata->gname );  /* no hash */

        if ( ch->pcdata->fname )
            DISPOSE( ch->pcdata->fname );  /* no hash */

        if ( ch->pcdata->lname )
            DISPOSE( ch->pcdata->lname );  /* no hash */

        DISPOSE( ch->pcdata->bestowments ); /* no hash */
        DISPOSE( ch->pcdata->homepage   );  /* no hash */
        STRFREE( ch->pcdata->authed_by  );
        STRFREE( ch->pcdata->prompt );
        DISPOSE( ch->pcdata->ignore ); /* no hash */

        if ( ch->pcdata->subprompt )
            STRFREE( ch->pcdata->subprompt );

        DISPOSE( ch->pcdata );
    }

    for ( mpact = ch->mpact; mpact; mpact = mpact_next )
    {
        mpact_next = mpact->next;
        DISPOSE( mpact->buf );
        DISPOSE( mpact      );
    }

    for ( comments = ch->comments; comments; comments = comments_next )
    {
        comments_next = comments->next;
        STRFREE( comments->text    );
        STRFREE( comments->to_list );
        STRFREE( comments->subject );
        STRFREE( comments->sender  );
        STRFREE( comments->date    );
        DISPOSE( comments          );
    }

    DISPOSE( ch );
    return;
}



/*
    Get an extra description from a list.
*/
char* get_extra_descr( const char* name, EXTRA_DESCR_DATA* ed )
{
    char token[MAX_STRING_LENGTH];

    if ( ed == NULL || ed->keyword == NULL )
        return NULL;

    name = one_argument( name, token );

    while ( token[0] != '\0' )
    {
        for ( ; ed; ed = ed->next )
        {
            // Match on either a prefix or an exact match with the name we're looking for, in case it's multi-word
            if ( is_name_prefix( token, ed->keyword ) || !strcasecmp( token, ed->keyword ) )
            {
                return ed->description;
            }
        }

        name = one_argument( name, token );
    }

    return NULL;
}

/*
    Translates mob virtual number to its mob index struct.
    Hash table lookup.
*/
MOB_INDEX_DATA* get_mob_index( int vnum )
{
    MOB_INDEX_DATA* pMobIndex;

    if ( vnum < 0 )
        vnum = 0;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
            pMobIndex;
            pMobIndex  = pMobIndex->next )
        if ( pMobIndex->vnum == vnum )
            return pMobIndex;

    if ( fBootDb )
        bug( "Get_mob_index: bad vnum %d.", vnum );

    return NULL;
}



/*
    Translates obj virtual number to its obj index struct.
    Hash table lookup.
*/
OBJ_INDEX_DATA* get_obj_index( int vnum )
{
    OBJ_INDEX_DATA* pObjIndex;

    if ( vnum < 0 )
        vnum = 0;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
            pObjIndex;
            pObjIndex  = pObjIndex->next )
        if ( pObjIndex->vnum == vnum )
            return pObjIndex;

    if ( fBootDb )
        bug( "Get_obj_index: bad vnum %d.", vnum );

    return NULL;
}



/*
    Translates room virtual number to its room index struct.
    Hash table lookup.
*/
ROOM_INDEX_DATA* get_room_index( int vnum )
{
    ROOM_INDEX_DATA* pRoomIndex;

    if ( vnum < 0 )
        vnum = 0;

    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
            pRoomIndex;
            pRoomIndex  = pRoomIndex->next )
        if ( pRoomIndex->vnum == vnum )
            return pRoomIndex;

    if ( fBootDb )
        bug( "Get_room_index: bad vnum %d.", vnum );

    return NULL;
}



/*
    Added lots of EOF checks, as most of the file crashes are based on them.
    If an area file encounters EOF, the fread_* functions will shutdown the
    MUD, as all area files should be read in in full or bad things will
    happen during the game.  Any files loaded in without fBootDb which
    encounter EOF will return what they have read so far.   These files
    should include player files, and in-progress areas that are not loaded
    upon bootup.
    -- Altrag
*/


/*
    Read a letter from a file.
*/
char fread_letter( FILE* fp )
{
    char c;

    do
    {
        if ( feof( fp ) )
        {
            bug( "fread_letter: EOF encountered on read.\n\r" );

            if ( fBootDb )
                exit( 1 );

            return '\0';
        }

        c = getc( fp );
    } while ( isspace( c ) );

    return c;
}



/*
    Read a number from a file.
*/
int fread_number( FILE* fp )
{
    int number;
    bool sign;
    char c;

    do
    {
        if ( feof( fp ) )
        {
            bug( "fread_number: EOF encountered on read.\n\r" );

            if ( fBootDb )
                exit( 1 );

            return 0;
        }

        c = getc( fp );
    } while ( isspace( c ) );

    number = 0;
    sign   = FALSE;

    if ( c == '+' )
    {
        c = getc( fp );
    }
    else if ( c == '-' )
    {
        sign = TRUE;
        c = getc( fp );
    }

    if ( !isdigit( c ) )
    {
        char buf[MAX_STRING_LENGTH];
        sprintf( buf, "Fread_number: bad format. (%c)", c );
        bug( buf );

        if ( fBootDb )
            exit( 1 );

        return 0;
    }

    while ( isdigit( c ) )
    {
        if ( feof( fp ) )
        {
            bug( "fread_number: EOF encountered on read.\n\r" );

            if ( fBootDb )
                exit( 1 );

            return number;
        }

        number = number * 10 + c - '0';
        c      = getc( fp );
    }

    if ( sign )
        number = 0 - number;

    if ( c == '|' )
        number += fread_number( fp );
    else if ( c != ' ' )
        ungetc( c, fp );

    return number;
}


/*
    custom str_dup using create                  -Thoric
*/
char* str_dup( char const* str )
{
    static char* ret;
    int len;

    if ( !str )
        return NULL;

    len = strlen( str ) + 1;
    CREATE( ret, char, len );
    strcpy( ret, str );
    return ret;
}

/*
    Read a string from file fp
*/
char* fread_string( FILE* fp )
{
    char buf[MAX_STRING_LENGTH];
    char* plast;
    char c;
    int ln;
    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
        Skip blanks.
        Read first char.
    */
    do
    {
        if ( feof( fp ) )
        {
            bug( "fread_string: EOF encountered on read.\n\r" );

            if ( fBootDb )
                exit( 1 );

            return STRALLOC( "" );
        }

        c = getc( fp );
    } while ( isspace( c ) );

    if ( ( *plast++ = c ) == '~' )
        return STRALLOC( "" );

    for ( ;; )
    {
        if ( ln >= ( MAX_STRING_LENGTH - 1 ) )
        {
            bug( "fread_string: string too long" );
            *plast = '\0';
            return STRALLOC( buf );
        }

        switch ( *plast = getc( fp ) )
        {
            default:
                plast++;
                ln++;
                break;

            case EOF:
                bug( "Fread_string: EOF" );

                if ( fBootDb )
                    exit( 1 );

                *plast = '\0';
                return STRALLOC( buf );
                break;

            case '\n':
                plast++;
                ln++;
                *plast++ = '\r';
                ln++;
                break;

            case '\r':
                break;

            case '~':
                *plast = '\0';
                return STRALLOC( buf );
        }
    }
}

/*
    Read a string from file fp using str_dup (ie: no string hashing)
*/
char* fread_string_nohash( FILE* fp )
{
    char buf[MAX_STRING_LENGTH];
    char* plast;
    char c;
    int ln;
    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
        Skip blanks.
        Read first char.
    */
    do
    {
        if ( feof( fp ) )
        {
            bug( "fread_string_no_hash: EOF encountered on read.\n\r" );

            if ( fBootDb )
                exit( 1 );

            return str_dup( "" );
        }

        c = getc( fp );
    } while ( isspace( c ) );

    if ( ( *plast++ = c ) == '~' )
        return str_dup( "" );

    for ( ;; )
    {
        if ( ln >= ( MAX_STRING_LENGTH - 1 ) )
        {
            bug( "fread_string_no_hash: string too long" );
            *plast = '\0';
            return str_dup( buf );
        }

        switch ( *plast = getc( fp ) )
        {
            default:
                plast++;
                ln++;
                break;

            case EOF:
                bug( "Fread_string_no_hash: EOF" );

                if ( fBootDb )
                    exit( 1 );

                *plast = '\0';
                return str_dup( buf );
                break;

            case '\n':
                plast++;
                ln++;
                *plast++ = '\r';
                ln++;
                break;

            case '\r':
                break;

            case '~':
                *plast = '\0';
                return str_dup( buf );
        }
    }
}



/*
    Read to end of line (for comments).
*/
void fread_to_eol( FILE* fp )
{
    char c;

    do
    {
        if ( feof( fp ) )
        {
            bug( "fread_to_eol: EOF encountered on read.\n\r" );

            if ( fBootDb )
                exit( 1 );

            return;
        }

        c = getc( fp );
    } while ( c != '\n' && c != '\r' );

    do
    {
        c = getc( fp );
    } while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

/*
    Read to end of line into static buffer           -Thoric
*/
char* fread_line( FILE* fp )
{
    static char line[MAX_STRING_LENGTH];
    char* pline;
    char c;
    int ln;
    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
        Skip blanks.
        Read first char.
    */
    do
    {
        if ( feof( fp ) )
        {
            bug( "fread_line: EOF encountered on read.\n\r" );

            if ( fBootDb )
                exit( 1 );

            strcpy( line, "" );
            return line;
        }

        c = getc( fp );
    } while ( isspace( c ) );

    ungetc( c, fp );

    do
    {
        if ( feof( fp ) )
        {
            bug( "fread_line: EOF encountered on read.\n\r" );

            if ( fBootDb )
                exit( 1 );

            *pline = '\0';
            return line;
        }

        c = getc( fp );
        *pline++ = c;
        ln++;

        if ( ln >= ( MAX_STRING_LENGTH - 1 ) )
        {
            bug( "fread_line: line too long" );
            break;
        }
    } while ( c != '\n' && c != '\r' );

    do
    {
        c = getc( fp );
    } while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    *pline = '\0';
    return line;
}



/*
    Read one word (into static buffer).
*/
char* fread_word( FILE* fp )
{
    static char word[MAX_INPUT_LENGTH];
    char* pword;
    char cEnd;

    do
    {
        if ( feof( fp ) )
        {
            bug( "fread_word: EOF encountered on read.\n\r" );

            if ( fBootDb )
                exit( 1 );

            word[0] = '\0';
            return word;
        }

        cEnd = getc( fp );
    } while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
        pword   = word;
    }
    else
    {
        word[0] = cEnd;
        pword   = word + 1;
        cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
        if ( feof( fp ) )
        {
            bug( "fread_word: EOF encountered on read.\n\r" );

            if ( fBootDb )
                exit( 1 );

            *pword = '\0';
            return word;
        }

        *pword = getc( fp );

        if ( cEnd == ' ' ? isspace( *pword ) : *pword == cEnd )
        {
            if ( cEnd == ' ' )
                ungetc( *pword, fp );

            *pword = '\0';
            return word;
        }
    }

    bug( "Fread_word: word too long" );
    return NULL;
}


void do_memory( CHAR_DATA* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    int hash;
    argument = one_argument( argument, arg );
    ch_printf( ch, "&zAffects &C%5d    &zAreas        &C%5d\n\r",  top_affect, top_area   );
    ch_printf( ch, "&zExtDes  &C%5d    &zExits        &C%5d\n\r", top_ed,     top_exit   );
    ch_printf( ch, "&zHelps   &C%5d    &zResets       &C%5d\n\r", top_help,   top_reset  );
    ch_printf( ch, "&zIdxMobs &C%5d    &zMobs         &C%5d\n\r", top_mob_index, nummobsloaded );
    ch_printf( ch, "&zIdxObjs &C%5d    &zObjs         &C%5d (%d)\n\r", top_obj_index, numobjsloaded, physicalobjects );
    ch_printf( ch, "&zRooms   &C%5d    &zVRooms       &C%5d\n\r", top_room,   top_vroom   );
    ch_printf( ch, "&zShops   &C%5d    \n\r", top_shop );
    ch_printf( ch, "&zCurOq's &C%5d    &zCurCq's      &C%5d\n\r", cur_qobjs,  cur_qchars );
    ch_printf( ch, "&zPlayers &C%5d    &zMax Players  &C%5d\n\r", num_descriptors, sysdata.maxplayers );
    ch_printf( ch, "&zTopsn   &C%5d    &zMax Skill    &C%5d\n\r", top_sn, MAX_SKILL );
    ch_printf( ch, "\n\r" );
    ch_printf( ch, "&zUsed Player IDs: &C%5d\n\r", sysdata.currid );
    ch_printf( ch, "&zMaximum Players: &C%5d\n\r", sysdata.alltimemax );
    ch_printf( ch, "&zMaximum players recorded at: &C%s\n\r", sysdata.time_of_max );

    if ( !str_cmp( arg, "check" ) )
    {
#ifdef HASHSTR
        send_to_char( check_hash( argument ), ch );
#else
        send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
        return;
    }

    if ( !str_cmp( arg, "showhigh" ) )
    {
#ifdef HASHSTR
        show_high_hash( atoi( argument ) );
#else
        send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
        return;
    }

    if ( argument[0] != '\0' )
        hash = atoi( argument );
    else
        hash = -1;

    if ( !str_cmp( arg, "hash" ) )
    {
#ifdef HASHSTR
        ch_printf( ch, "Hash statistics:\n\r%s", hash_stats() );

        if ( hash != -1 )
            hash_dump( hash );

#else
        send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
    }

    return;
}



/*
    Stick a little fuzz on a number.
*/
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
        case 0:
            number -= 1;
            break;

        case 3:
            number += 1;
            break;
    }

    return UMAX( 1, number );
}



/*
    Generate a random number.
*/
int number_range( int from, int to )
{
    /*    int power;
        int number;*/
    if ( ( to = to - from + 1 ) <= 1 )
        return from;

    /*    for ( power = 2; power < to; power <<= 1 )
        ;

        while ( ( number = number_mm( ) & (power - 1) ) >= to )
        ;

        return from + number;*/
    return ( number_mm() % to ) + from;
}

/*
    Generate a percentile roll.
*/
int number_percent( void )
{
    return number_mm() % 100;
}



/*
    Generate a random door.
*/
int number_door( void )
{
    int door;

    while ( ( door = number_mm( ) & ( 16 - 1 ) ) > 9 )
        ;

    return door;
    /*    return number_mm() & 10; */
}



int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}



/*
    I've gotten too many bad reports on OS-supplied random number generators.
    This is the Mitchell-Moore algorithm from Knuth Volume II.
    Best to leave the constants alone unless you've read Knuth.
    -- Furey
*/
static  int rgiState[2 + 55];

void init_mm( )
{
    int* piState;
    int iState;
    piState = &rgiState[2];
    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;
    piState[0]  = ( ( int ) current_time ) & ( ( 1 << 30 ) - 1 );
    piState[1]  = 1;

    for ( iState = 2; iState < 55; iState++ )
    {
        piState[iState] = ( piState[iState - 1] + piState[iState - 2] )
                          & ( ( 1 << 30 ) - 1 );
    }

    return;
}



int number_mm( void )
{
    int* piState;
    int iState1;
    int iState2;
    int iRand;
    piState     = &rgiState[2];
    iState1     = piState[-2];
    iState2     = piState[-1];
    iRand       = ( piState[iState1] + piState[iState2] )
                  & ( ( 1 << 30 ) - 1 );
    piState[iState1]    = iRand;

    if ( ++iState1 == 55 )
        iState1 = 0;

    if ( ++iState2 == 55 )
        iState2 = 0;

    piState[-2]     = iState1;
    piState[-1]     = iState2;
    return iRand >> 6;
}



/*
    Roll some dice.                      -Thoric
*/
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
        case 0:
            return 0;

        case 1:
            return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
        sum += number_range( 1, size );

    return sum;
}


/*
    Removes the tildes from a string.
    Used for player-entered strings that go into disk files.
*/
void smash_tilde( char* str )
{
    for ( ; *str != '\0'; str++ )
        if ( *str == '~' )
            *str = '-';

    return;
}

/*
    Encodes the tildes in a string.              -Thoric
    Used for player-entered strings that go into disk files.
*/
void hide_tilde( char* str )
{
    for ( ; *str != '\0'; str++ )
        if ( *str == '~' )
            *str = HIDDEN_TILDE;

    return;
}

char* show_tilde( char* str )
{
    static char buf[MAX_STRING_LENGTH];
    char* bufptr;
    bufptr = buf;

    for ( ; *str != '\0'; str++, bufptr++ )
    {
        if ( *str == HIDDEN_TILDE )
            *bufptr = '~';
        else
            *bufptr = *str;
    }

    *bufptr = '\0';
    return buf;
}



/*
    Compare strings, case insensitive.
    Return TRUE if different
     (compatibility with historical functions).
*/
bool str_cmp( const char* astr, const char* bstr )
{
    if ( !astr )
    {
        bug( "Str_cmp: null astr." );

        if ( bstr )
            fprintf( stderr, "str_cmp: astr: (null)  bstr: %s\n", bstr );

        return TRUE;
    }

    if ( !bstr )
    {
        bug( "Str_cmp: null bstr." );

        if ( astr )
            fprintf( stderr, "str_cmp: astr: %s  bstr: (null)\n", astr );

        return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
        if ( LOWER( *astr ) != LOWER( *bstr ) )
            return TRUE;
    }

    return FALSE;
}



/*
    Compare strings, case insensitive, for prefix matching.
    Return TRUE if astr not a prefix of bstr
     (compatibility with historical functions).
*/
bool str_prefix( const char* astr, const char* bstr )
{
    if ( !astr )
    {
        bug( "Strn_cmp: null astr." );
        return TRUE;
    }

    if ( !bstr )
    {
        bug( "Strn_cmp: null bstr." );
        return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
        if ( LOWER( *astr ) != LOWER( *bstr ) )
            return TRUE;
    }

    return FALSE;
}



/*
    Compare strings, case insensitive, for match anywhere.
    Returns TRUE is astr not part of bstr.
     (compatibility with historical functions).
*/
bool str_infix( const char* astr, const char* bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = LOWER( astr[0] ) ) == '\0' )
        return FALSE;

    sstr1 = strlen( astr );
    sstr2 = strlen( bstr );

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
        if ( c0 == LOWER( bstr[ichar] ) && !str_prefix( astr, bstr + ichar ) )
            return FALSE;

    return TRUE;
}



/*
    Compare strings, case insensitive, for suffix matching.
    Return TRUE if astr not a suffix of bstr
     (compatibility with historical functions).
*/
bool str_suffix( const char* astr, const char* bstr )
{
    int sstr1;
    int sstr2;
    sstr1 = strlen( astr );
    sstr2 = strlen( bstr );

    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
        return FALSE;
    else
        return TRUE;
}



/*
    Returns an initial-capped string.
*/
char* capitalize( const char* str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strcap[i] = LOWER( str[i] );

    strcap[i] = '\0';
    strcap[0] = UPPER( strcap[0] );
    return strcap;
}


/*
    Returns a lowercase string.
*/
char* strlower( const char* str )
{
    static char strlow[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strlow[i] = LOWER( str[i] );

    strlow[i] = '\0';
    return strlow;
}

/*
    Returns an uppercase string.
*/
char* strupper( const char* str )
{
    static char strup[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strup[i] = UPPER( str[i] );

    strup[i] = '\0';
    return strup;
}

/*
    Returns TRUE or FALSE if a letter is a vowel         -Thoric
*/
bool isavowel( char letter )
{
    char c;
    c = tolower( letter );

    if ( c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' )
        return TRUE;
    else
        return FALSE;
}

/*
    Shove either "a " or "an " onto the beginning of a string    -Thoric
*/
char* aoran( const char* str )
{
    static char temp[MAX_STRING_LENGTH];

    if ( !str )
    {
        bug( "Aoran(): NULL str" );
        return "";
    }

    if ( isavowel( str[0] )
            || ( strlen( str ) > 1 && tolower( str[0] ) == 'y' && !isavowel( str[1] ) ) )
        strcpy( temp, "an " );
    else
        strcpy( temp, "a " );

    strcat( temp, str );
    return temp;
}

char* aoran_cap( const char* str )
{
    static char temp[MAX_STRING_LENGTH];

    if ( !str )
    {
        bug( "Aoran_cap(): NULL str" );
        return "";
    }

    if ( isavowel( str[0] )
            || ( strlen( str ) > 1 && tolower( str[0] ) == 'y' && !isavowel( str[1] ) ) )
        strcpy( temp, "An " );
    else
        strcpy( temp, "A " );

    strcat( temp, str );
    return temp;
}


/*
    Append a string to a file.
*/
void append_file( CHAR_DATA* ch, char* file, char* str )
{
    FILE* fp;

    if ( IS_NPC( ch ) || str[0] == '\0' )
        return;

    fclose( fpLOG );

    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
        send_to_char( "Could not open the file!\n\r", ch );
    }
    else
    {
        fprintf( fp, "[%5d] %s: %s\n",
                 ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
        fclose( fp );
    }

    fpLOG = fopen( NULL_FILE, "r" );
    return;
}

/*
    Append a string to a file.
*/
void append_to_file( char* file, char* str )
{
    FILE* fp;

    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {}
    else
    {
        fprintf( fp, "%s\n", str );
        fclose( fp );
    }

    return;
}


/*
    Reports a bug.
*/
void bug( const char* str, ... )
{
    char buf[MAX_STRING_LENGTH];
    FILE* fp;
    struct stat fst;

    if ( fpArea != NULL )
    {
        int iLine;
        long iChar;

        if ( fpArea == stdin )
        {
            iLine = 0;
        }
        else
        {
            iChar = ftell( fpArea );
            fseek( fpArea, 0, 0 );

            for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
            {
                while ( getc( fpArea ) != '\n' )
                    ;
            }

            fseek( fpArea, iChar, 0 );
        }

        sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
        log_string( buf );

        if ( stat( SHUTDOWN_FILE, &fst ) != -1 )    /* file exists */
        {
            if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
            {
                fprintf( fp, "[*****] %s\n", buf );
                fclose( fp );
            }
        }
    }

    strcpy( buf, "[*****] BUG: " );
    {
        va_list param;
        va_start( param, str );
        vsprintf( buf + strlen( buf ), str, param );
        va_end( param );
    }
    log_string( buf );
    fclose( fpLOG );

    if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
    {
        fprintf( fp, "%s\n", buf );
        fclose( fp );
    }

    fpLOG = fopen( NULL_FILE, "r" );
    return;
}

/*
    Add a string to the boot-up log              -Thoric
*/
void boot_log( const char* str, ... )
{
    char buf[MAX_STRING_LENGTH];
    FILE* fp;
    va_list param;
    strcpy( buf, "[*****] BOOT: " );
    va_start( param, str );
    vsprintf( buf + strlen( buf ), str, param );
    va_end( param );
    log_string( buf );
    fclose( fpLOG );

    if ( ( fp = fopen( BOOTLOG_FILE, "a" ) ) != NULL )
    {
        fprintf( fp, "%s\n", buf );
        fclose( fp );
    }

    fpLOG = fopen( NULL_FILE, "r" );
    return;
}

/*
    Dump a text file to a player, a line at a time       -Thoric
*/
void show_file( CHAR_DATA* ch, char* filename )
{
    FILE* fp;
    char buf[MAX_STRING_LENGTH];
    int c;
    int num = 0;

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        while ( !feof( fp ) )
        {
            while ( ( buf[num] = fgetc( fp ) ) != EOF
                    &&      buf[num] != '\n'
                    &&      buf[num] != '\r'
                    &&      num < ( MAX_STRING_LENGTH - 2 ) )
                num++;

            c = fgetc( fp );

            if ( ( c != '\n' && c != '\r' ) || c == buf[num] )
                ungetc( c, fp );

            buf[num++] = '\n';
            buf[num++] = '\r';
            buf[num  ] = '\0';
            send_to_pager( buf, ch );
            num = 0;
        }
    }
}

/*
    Show the boot log file                   -Thoric
*/
void do_dmesg( CHAR_DATA* ch, char* argument )
{
    set_pager_color( AT_LOG, ch );
    show_file( ch, BOOTLOG_FILE );
}

/*
    Writes a string to the log, extended version         -Thoric
*/
void log_string_plus( const char* str, sh_int log_type, sh_int level )
{
    char* strtime;
    int offset;
    strtime                    = ctime( &current_time );
    strtime[strlen( strtime ) - 1] = '\0';
    fprintf( stderr, "%s :: %s\n", strtime, str );

    if ( strncmp( str, "Log ", 4 ) == 0 )
        offset = 4;
    else
        offset = 0;

    switch ( log_type )
    {
        default:
            to_channel( str + offset, CHANNEL_LOG, "Log", level );
            break;

        case LOG_BUILD:
            to_channel( str + offset, CHANNEL_BUILD, "Build", level );
            break;

        case LOG_COMM:
            to_channel( str + offset, CHANNEL_COMM, "Comm", level );
            break;

        case LOG_ALL:
            break;
    }

    return;
}

/*
    wizlist builder!                     -Thoric
*/

void towizfile( const char* line )
{
    int filler, xx;
    char outline[MAX_STRING_LENGTH];
    FILE* wfp;
    outline[0] = '\0';

    if ( line && line[0] != '\0' )
    {
        filler = ( 78 - strlen( line ) );

        if ( filler < 1 )
            filler = 1;

        filler /= 2;

        for ( xx = 0; xx < filler; xx++ )
            strcat( outline, " " );

        strcat( outline, line );
    }

    strcat( outline, "\n\r" );
    wfp = fopen( WIZLIST_FILE, "a" );

    if ( wfp )
    {
        fputs( outline, wfp );
        fclose( wfp );
    }
}

void add_to_wizlist( char* name, int level )
{
    WIZENT* wiz, *tmp;
    CREATE( wiz, WIZENT, 1 );
    wiz->name = str_dup( name );
    wiz->level    = level;

    if ( !first_wiz )
    {
        wiz->last   = NULL;
        wiz->next   = NULL;
        first_wiz   = wiz;
        last_wiz    = wiz;
        return;
    }

    /* insert sort, of sorts */
    for ( tmp = first_wiz; tmp; tmp = tmp->next )
        if ( level > tmp->level )
        {
            if ( !tmp->last )
                first_wiz   = wiz;
            else
                tmp->last->next = wiz;

            wiz->last = tmp->last;
            wiz->next = tmp;
            tmp->last = wiz;
            return;
        }

    wiz->last     = last_wiz;
    wiz->next     = NULL;
    last_wiz->next    = wiz;
    last_wiz      = wiz;
    return;
}

/*
    Wizlist builder                      -Thoric
*/
#define MWIZ_LEVEL    200

void make_wizlist( )
{
    DIR* dp;
    struct dirent* dentry;
    FILE* gfp;
    char* word;
    int ilevel;
    EXT_BV iflags;
    WIZENT* wiz, *wiznext;
    char buf[MAX_STRING_LENGTH];
    first_wiz = NULL;
    last_wiz  = NULL;
    dp = opendir( GOD_DIR );
    ilevel = 0;
    dentry = readdir( dp );

    while ( dentry )
    {
        if ( dentry->d_name[0] != '.' )
        {
            sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
            gfp = fopen( buf, "r" );

            if ( gfp )
            {
                word = feof( gfp ) ? "End" : fread_word( gfp );
                ilevel = fread_number( gfp );
                fread_to_eol( gfp );
                word = feof( gfp ) ? "End" : fread_word( gfp );

                if ( !str_cmp( word, "Pcflags" ) )
                    iflags = fread_bitvector( gfp );
                else
                    xCLEAR_BITS( iflags );

                fclose( gfp );

                if ( xIS_SET( iflags, PCFLAG_RETIRED ) )
                    ilevel = MWIZ_LEVEL - 94;

                if ( xIS_SET( iflags, PCFLAG_GUEST ) )
                    ilevel = MWIZ_LEVEL - 94;

                if ( ilevel <= MWIZ_LEVEL )
                    add_to_wizlist( dentry->d_name, ilevel );
            }
        }

        dentry = readdir( dp );
    }

    closedir( dp );
    buf[0] = '\0';
    unlink( WIZLIST_FILE );
    towizfile( " " );
    towizfile( " Masters of AVP Legend!" );
    ilevel = 65535;

    for ( wiz = first_wiz; wiz; wiz = wiz->next )
    {
        if ( wiz->level > LEVEL_AVATAR )
        {
            if ( wiz->level < ilevel )
            {
                if ( buf[0] )
                {
                    towizfile( buf );
                    buf[0] = '\0';
                }

                towizfile( "" );
                ilevel = wiz->level;

                switch ( ilevel )
                {
                    case MWIZ_LEVEL -  0:
                        towizfile( "     &GThe Chief Executives&W" );
                        break;

                    case MAX_LEVEL -  0:
                        towizfile( "     &cThe Chairman of the Board&W" );
                        break;

                    case MAX_LEVEL -  1:
                        towizfile( "     &CThe Defense Attorney&W" );
                        break;

                    case MAX_LEVEL -  2:
                        towizfile( " The Employees" );
                        break;

                    case MAX_LEVEL -  3:
                        towizfile( " The Consultants" );
                        break;

                    default:
                        towizfile( " The Stockholders" );
                        break;
                }
            }

            if ( strlen( buf ) + strlen( wiz->name ) > 76 )
            {
                towizfile( buf );
                buf[0] = '\0';
            }

            strcat( buf, " " );
            strcat( buf, wiz->name );

            if ( strlen( buf ) > 70 )
            {
                towizfile( buf );
                buf[0] = '\0';
            }
        }
    }

    if ( buf[0] )
        towizfile( buf );

    for ( wiz = first_wiz; wiz; wiz = wiznext )
    {
        wiznext = wiz->next;
        DISPOSE( wiz->name );
        DISPOSE( wiz );
    }

    first_wiz = NULL;
    last_wiz = NULL;
}


void do_makewizlist( CHAR_DATA* ch, char* argument )
{
    make_wizlist();
}


/* mud prog functions */

/* This routine reads in scripts of MUDprograms from a file */

int mprog_name_to_type ( char* name )
{
    if ( !str_cmp( name, "in_file_prog"   ) )
        return IN_FILE_PROG;

    if ( !str_cmp( name, "act_prog"       ) )
        return ACT_PROG;

    if ( !str_cmp( name, "speech_prog"    ) )
        return SPEECH_PROG;

    if ( !str_cmp( name, "rand_prog"      ) )
        return RAND_PROG;

    if ( !str_cmp( name, "fight_prog"     ) )
        return FIGHT_PROG;

    if ( !str_cmp( name, "hitprcnt_prog"  ) )
        return HITPRCNT_PROG;

    if ( !str_cmp( name, "death_prog"     ) )
        return DEATH_PROG;

    if ( !str_cmp( name, "entry_prog"     ) )
        return ENTRY_PROG;

    if ( !str_cmp( name, "greet_prog"     ) )
        return GREET_PROG;

    if ( !str_cmp( name, "all_greet_prog" ) )
        return ALL_GREET_PROG;

    if ( !str_cmp( name, "give_prog"      ) )
        return GIVE_PROG;

    if ( !str_cmp( name, "bribe_prog"     ) )
        return BRIBE_PROG;

    if ( !str_cmp( name, "time_prog"     ) )
        return TIME_PROG;

    if ( !str_cmp( name, "hour_prog"     ) )
        return HOUR_PROG;

    if ( !str_cmp( name, "wear_prog"     ) )
        return WEAR_PROG;

    if ( !str_cmp( name, "remove_prog"   ) )
        return REMOVE_PROG;

    if ( !str_cmp( name, "sac_prog"      ) )
        return SAC_PROG;

    if ( !str_cmp( name, "look_prog"     ) )
        return LOOK_PROG;

    if ( !str_cmp( name, "exa_prog"      ) )
        return EXA_PROG;

    if ( !str_cmp( name, "zap_prog"      ) )
        return ZAP_PROG;

    if ( !str_cmp( name, "get_prog"      ) )
        return GET_PROG;

    if ( !str_cmp( name, "drop_prog"     ) )
        return DROP_PROG;

    if ( !str_cmp( name, "damage_prog"   ) )
        return DAMAGE_PROG;

    if ( !str_cmp( name, "repair_prog"   ) )
        return REPAIR_PROG;

    if ( !str_cmp( name, "greet_prog"    ) )
        return GREET_PROG;

    if ( !str_cmp( name, "randiw_prog"   ) )
        return RANDIW_PROG;

    if ( !str_cmp( name, "speechiw_prog" ) )
        return SPEECHIW_PROG;

    if ( !str_cmp( name, "useon_prog"     ) )
        return USEON_PROG;

    if ( !str_cmp( name, "useoff_prog"     ) )
        return USEOFF_PROG;

    if ( !str_cmp( name, "sleep_prog"    ) )
        return SLEEP_PROG;

    if ( !str_cmp( name, "rest_prog" ) )
        return REST_PROG;

    if ( !str_cmp( name, "rfight_prog"   ) )
        return FIGHT_PROG;

    if ( !str_cmp( name, "enter_prog"    ) )
        return ENTRY_PROG;

    if ( !str_cmp( name, "leave_prog"    ) )
        return LEAVE_PROG;

    if ( !str_cmp( name, "rdeath_prog"   ) )
        return DEATH_PROG;

    if ( !str_cmp( name, "script_prog"   ) )
        return SCRIPT_PROG;

    if ( !str_cmp( name, "use_prog"  ) )
        return USE_PROG;

    if ( !str_cmp( name, "pulse_prog"    ) )
        return PULSE_PROG;

    return ( ERROR_PROG );
}

MPROG_DATA* mprog_file_read( char* f, MPROG_DATA* mprg, MOB_INDEX_DATA* pMobIndex )
{
    char        MUDProgfile[ MAX_INPUT_LENGTH ];
    FILE*       progfile;
    char        letter;
    MPROG_DATA* mprg_next, *mprg2;
    bool        done = FALSE;
    sprintf( MUDProgfile, "%s%s", PROG_DIR, f );
    progfile = fopen( MUDProgfile, "r" );

    if ( !progfile )
    {
        bug( "Mob: %d couldn't open mudprog file", pMobIndex->vnum );
        exit( 1 );
    }

    mprg2 = mprg;

    switch ( letter = fread_letter( progfile ) )
    {
        case '>':
            break;

        case '|':
            bug( "empty mudprog file." );
            exit( 1 );
            break;

        default:
            bug( "in mudprog file syntax error." );
            exit( 1 );
            break;
    }

    while ( !done )
    {
        mprg2->type = mprog_name_to_type( fread_word( progfile ) );

        switch ( mprg2->type )
        {
            case ERROR_PROG:
                bug( "mudprog file type error" );
                exit( 1 );
                break;

            case IN_FILE_PROG:
                bug( "mprog file contains a call to file." );
                exit( 1 );
                break;

            default:
                // pMobIndex->progtypes = pMobIndex->progtypes | mprg2->type;
                xSET_BIT( pMobIndex->progtypes, mprg2->type );
                mprg2->arglist       = fread_string( progfile );
                mprg2->comlist       = fread_string( progfile );

                switch ( letter = fread_letter( progfile ) )
                {
                    case '>':
                        CREATE( mprg_next, MPROG_DATA, 1 );
                        mprg_next->next = mprg2;
                        mprg2 = mprg_next;
                        break;

                    case '|':
                        done = TRUE;
                        break;

                    default:
                        bug( "in mudprog file syntax error." );
                        exit( 1 );
                        break;
                }

                break;
        }
    }

    fclose( progfile );
    return mprg2;
}

/*  Load a MUDprogram section from the area file.
*/
void load_mudprogs( AREA_DATA* tarea, FILE* fp )
{
    MOB_INDEX_DATA* iMob;
    MPROG_DATA*     original;
    MPROG_DATA*     working;
    char            letter;
    int             value;

    for ( ; ; )
        switch ( letter = fread_letter( fp ) )
        {
            default:
                bug( "Load_mudprogs: bad command '%c'.", letter );
                exit( 1 );
                break;

            case 'S':
            case 's':
                fread_to_eol( fp );
                return;

            case '*':
                fread_to_eol( fp );
                break;

            case 'M':
            case 'm':
                value = fread_number( fp );

                if ( ( iMob = get_mob_index( value ) ) == NULL )
                {
                    bug( "Load_mudprogs: vnum %d doesnt exist", value );
                    exit( 1 );
                }

                /*  Go to the end of the prog command list if other commands
                    exist */

                if ( ( original = iMob->mudprogs ) != NULL )
                    for ( ; original->next; original = original->next );

                CREATE( working, MPROG_DATA, 1 );

                if ( original )
                    original->next = working;
                else
                    iMob->mudprogs = working;

                working = mprog_file_read( fread_word( fp ), working, iMob );
                working->next = NULL;
                fread_to_eol( fp );
                break;
        }

    return;
}

/*  This procedure is responsible for reading any in_file MUDprograms.
*/

void mprog_read_programs( FILE* fp, MOB_INDEX_DATA* pMobIndex )
{
    MPROG_DATA* mprg;
    char        letter;
    bool        done = FALSE;

    if ( ( letter = fread_letter( fp ) ) != '>' )
    {
        bug( "Load_mobiles: vnum %d MUDPROG char", pMobIndex->vnum );
        exit( 1 );
    }

    CREATE( mprg, MPROG_DATA, 1 );
    pMobIndex->mudprogs = mprg;

    while ( !done )
    {
        mprg->type = mprog_name_to_type( fread_word( fp ) );

        switch ( mprg->type )
        {
            case ERROR_PROG:
                bug( "Load_mobiles: vnum %d MUDPROG type.", pMobIndex->vnum );
                exit( 1 );
                break;

            case IN_FILE_PROG:
                mprg = mprog_file_read( fread_string( fp ), mprg, pMobIndex );
                fread_to_eol( fp );

                switch ( letter = fread_letter( fp ) )
                {
                    case '>':
                        CREATE( mprg->next, MPROG_DATA, 1 );
                        mprg = mprg->next;
                        break;

                    case '|':
                        mprg->next = NULL;
                        fread_to_eol( fp );
                        done = TRUE;
                        break;

                    default:
                        bug( "Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->vnum );
                        exit( 1 );
                        break;
                }

                break;

            default:
                // pMobIndex->progtypes = pMobIndex->progtypes | mprg->type;
                xSET_BIT( pMobIndex->progtypes, mprg->type );
                mprg->arglist        = fread_string( fp );
                fread_to_eol( fp );
                mprg->comlist        = fread_string( fp );
                fread_to_eol( fp );

                switch ( letter = fread_letter( fp ) )
                {
                    case '>':
                        CREATE( mprg->next, MPROG_DATA, 1 );
                        mprg = mprg->next;
                        break;

                    case '|':
                        mprg->next = NULL;
                        fread_to_eol( fp );
                        done = TRUE;
                        break;

                    default:
                        bug( "Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->vnum );
                        exit( 1 );
                        break;
                }

                break;
        }
    }

    return;
}



/*************************************************************/
/* obj prog functions */
/*  This routine transfers between alpha and numeric forms of the
    mob_prog bitvector types. This allows the use of the words in the
    mob/script files.
*/

/* This routine reads in scripts of OBJprograms from a file */


MPROG_DATA* oprog_file_read( char* f, MPROG_DATA* mprg,
                             OBJ_INDEX_DATA* pObjIndex )
{
    char        MUDProgfile[ MAX_INPUT_LENGTH ];
    FILE*       progfile;
    char        letter;
    MPROG_DATA* mprg_next, *mprg2;
    bool        done = FALSE;
    sprintf( MUDProgfile, "%s%s", PROG_DIR, f );
    progfile = fopen( MUDProgfile, "r" );

    if ( !progfile )
    {
        bug( "Obj: %d couldnt open mudprog file", pObjIndex->vnum );
        exit( 1 );
    }

    mprg2 = mprg;

    switch ( letter = fread_letter( progfile ) )
    {
        case '>':
            break;

        case '|':
            bug( "empty objprog file." );
            exit( 1 );
            break;

        default:
            bug( "in objprog file syntax error." );
            exit( 1 );
            break;
    }

    while ( !done )
    {
        mprg2->type = mprog_name_to_type( fread_word( progfile ) );

        switch ( mprg2->type )
        {
            case ERROR_PROG:
                bug( "objprog file type error" );
                exit( 1 );
                break;

            case IN_FILE_PROG:
                bug( "objprog file contains a call to file." );
                exit( 1 );
                break;

            default:
                // pObjIndex->progtypes = pObjIndex->progtypes | mprg2->type;
                xSET_BIT( pObjIndex->progtypes, mprg2->type );
                mprg2->arglist       = fread_string( progfile );
                mprg2->comlist       = fread_string( progfile );

                switch ( letter = fread_letter( progfile ) )
                {
                    case '>':
                        CREATE( mprg_next, MPROG_DATA, 1 );
                        mprg_next->next = mprg2;
                        mprg2 = mprg_next;
                        break;

                    case '|':
                        done = TRUE;
                        break;

                    default:
                        bug( "in objprog file syntax error." );
                        exit( 1 );
                        break;
                }

                break;
        }
    }

    fclose( progfile );
    return mprg2;
}

/*  Load a MUDprogram section from the area file.
*/
void load_objprogs( AREA_DATA* tarea, FILE* fp )
{
    OBJ_INDEX_DATA* iObj;
    MPROG_DATA*     original;
    MPROG_DATA*     working;
    char            letter;
    int             value;

    for ( ; ; )
        switch ( letter = fread_letter( fp ) )
        {
            default:
                bug( "Load_objprogs: bad command '%c'.", letter );
                exit( 1 );
                break;

            case 'S':
            case 's':
                fread_to_eol( fp );
                return;

            case '*':
                fread_to_eol( fp );
                break;

            case 'M':
            case 'm':
                value = fread_number( fp );

                if ( ( iObj = get_obj_index( value ) ) == NULL )
                {
                    bug( "Load_objprogs: vnum %d doesnt exist", value );
                    exit( 1 );
                }

                /*  Go to the end of the prog command list if other commands
                    exist */

                if ( ( original = iObj->mudprogs ) != NULL )
                    for ( ; original->next; original = original->next );

                CREATE( working, MPROG_DATA, 1 );

                if ( original )
                    original->next = working;
                else
                    iObj->mudprogs = working;

                working = oprog_file_read( fread_word( fp ), working, iObj );
                working->next = NULL;
                fread_to_eol( fp );
                break;
        }

    return;
}

/*  This procedure is responsible for reading any in_file OBJprograms.
*/

void oprog_read_programs( FILE* fp, OBJ_INDEX_DATA* pObjIndex )
{
    MPROG_DATA* mprg;
    char        letter;
    bool        done = FALSE;

    if ( ( letter = fread_letter( fp ) ) != '>' )
    {
        bug( "Load_objects: vnum %d OBJPROG char", pObjIndex->vnum );
        exit( 1 );
    }

    CREATE( mprg, MPROG_DATA, 1 );
    pObjIndex->mudprogs = mprg;

    while ( !done )
    {
        mprg->type = mprog_name_to_type( fread_word( fp ) );

        switch ( mprg->type )
        {
            case ERROR_PROG:
                bug( "Load_objects: vnum %d OBJPROG type.", pObjIndex->vnum );
                exit( 1 );
                break;

            case IN_FILE_PROG:
                mprg = oprog_file_read( fread_string( fp ), mprg, pObjIndex );
                fread_to_eol( fp );

                switch ( letter = fread_letter( fp ) )
                {
                    case '>':
                        CREATE( mprg->next, MPROG_DATA, 1 );
                        mprg = mprg->next;
                        break;

                    case '|':
                        mprg->next = NULL;
                        fread_to_eol( fp );
                        done = TRUE;
                        break;

                    default:
                        bug( "Load_objects: vnum %d bad OBJPROG.", pObjIndex->vnum );
                        exit( 1 );
                        break;
                }

                break;

            default:
                // pObjIndex->progtypes = pObjIndex->progtypes | mprg->type;
                xSET_BIT( pObjIndex->progtypes, mprg->type );
                mprg->arglist        = fread_string( fp );
                fread_to_eol( fp );
                mprg->comlist        = fread_string( fp );
                fread_to_eol( fp );

                switch ( letter = fread_letter( fp ) )
                {
                    case '>':
                        CREATE( mprg->next, MPROG_DATA, 1 );
                        mprg = mprg->next;
                        break;

                    case '|':
                        mprg->next = NULL;
                        fread_to_eol( fp );
                        done = TRUE;
                        break;

                    default:
                        bug( "Load_objects: vnum %d bad OBJPROG.", pObjIndex->vnum );
                        exit( 1 );
                        break;
                }

                break;
        }
    }

    return;
}


/*************************************************************/
/* room prog functions */
/*  This routine transfers between alpha and numeric forms of the
    mob_prog bitvector types. This allows the use of the words in the
    mob/script files.
*/

/* This routine reads in scripts of OBJprograms from a file */
MPROG_DATA* rprog_file_read( char* f, MPROG_DATA* mprg,
                             ROOM_INDEX_DATA* RoomIndex )
{
    char        MUDProgfile[ MAX_INPUT_LENGTH ];
    FILE*       progfile;
    char        letter;
    MPROG_DATA* mprg_next, *mprg2;
    bool        done = FALSE;
    sprintf( MUDProgfile, "%s%s", PROG_DIR, f );
    progfile = fopen( MUDProgfile, "r" );

    if ( !progfile )
    {
        bug( "Room: %d couldnt open roomprog file", RoomIndex->vnum );
        exit( 1 );
    }

    mprg2 = mprg;

    switch ( letter = fread_letter( progfile ) )
    {
        case '>':
            break;

        case '|':
            bug( "empty roomprog file." );
            exit( 1 );
            break;

        default:
            bug( "in roomprog file syntax error." );
            exit( 1 );
            break;
    }

    while ( !done )
    {
        mprg2->type = mprog_name_to_type( fread_word( progfile ) );

        switch ( mprg2->type )
        {
            case ERROR_PROG:
                bug( "roomprog file type error" );
                exit( 1 );
                break;

            case IN_FILE_PROG:
                bug( "roomprog file contains a call to file." );
                exit( 1 );
                break;

            default:
                // RoomIndex->progtypes = RoomIndex->progtypes | mprg2->type;
                xSET_BIT( RoomIndex->progtypes, mprg2->type );
                mprg2->arglist       = fread_string( progfile );
                mprg2->comlist       = fread_string( progfile );

                switch ( letter = fread_letter( progfile ) )
                {
                    case '>':
                        CREATE( mprg_next, MPROG_DATA, 1 );
                        mprg_next->next = mprg2;
                        mprg2 = mprg_next;
                        break;

                    case '|':
                        done = TRUE;
                        break;

                    default:
                        bug( "in roomprog file syntax error." );
                        exit( 1 );
                        break;
                }

                break;
        }
    }

    fclose( progfile );
    return mprg2;
}

/*  Load a ROOMprogram section from the area file.
*/
void load_roomprogs( AREA_DATA* tarea, FILE* fp )
{
    ROOM_INDEX_DATA* iRoom;
    MPROG_DATA*     original;
    MPROG_DATA*     working;
    char            letter;
    int             value;

    for ( ; ; )
        switch ( letter = fread_letter( fp ) )
        {
            default:
                bug( "Load_objprogs: bad command '%c'.", letter );
                exit( 1 );
                break;

            case 'S':
            case 's':
                fread_to_eol( fp );
                return;

            case '*':
                fread_to_eol( fp );
                break;

            case 'M':
            case 'm':
                value = fread_number( fp );

                if ( ( iRoom = get_room_index( value ) ) == NULL )
                {
                    bug( "Load_roomprogs: vnum %d doesnt exist", value );
                    exit( 1 );
                }

                /*  Go to the end of the prog command list if other commands
                    exist */

                if ( ( original = iRoom->mudprogs ) != NULL )
                    for ( ; original->next; original = original->next );

                CREATE( working, MPROG_DATA, 1 );

                if ( original )
                    original->next = working;
                else
                    iRoom->mudprogs = working;

                working = rprog_file_read( fread_word( fp ), working, iRoom );
                working->next = NULL;
                fread_to_eol( fp );
                break;
        }

    return;
}

/*  This procedure is responsible for reading any in_file ROOMprograms.
*/

void rprog_read_programs( FILE* fp, ROOM_INDEX_DATA* pRoomIndex )
{
    MPROG_DATA* mprg;
    char        letter;
    bool        done = FALSE;

    if ( ( letter = fread_letter( fp ) ) != '>' )
    {
        bug( "Load_rooms: vnum %d ROOMPROG char", pRoomIndex->vnum );
        exit( 1 );
    }

    CREATE( mprg, MPROG_DATA, 1 );
    pRoomIndex->mudprogs = mprg;

    while ( !done )
    {
        mprg->type = mprog_name_to_type( fread_word( fp ) );

        switch ( mprg->type )
        {
            case ERROR_PROG:
                bug( "Load_rooms: vnum %d ROOMPROG type.", pRoomIndex->vnum );
                exit( 1 );
                break;

            case IN_FILE_PROG:
                mprg = rprog_file_read( fread_string( fp ), mprg, pRoomIndex );
                fread_to_eol( fp );

                switch ( letter = fread_letter( fp ) )
                {
                    case '>':
                        CREATE( mprg->next, MPROG_DATA, 1 );
                        mprg = mprg->next;
                        break;

                    case '|':
                        mprg->next = NULL;
                        fread_to_eol( fp );
                        done = TRUE;
                        break;

                    default:
                        bug( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum );
                        exit( 1 );
                        break;
                }

                break;

            default:
                // pRoomIndex->progtypes = pRoomIndex->progtypes | mprg->type;
                xSET_BIT( pRoomIndex->progtypes, mprg->type );
                mprg->arglist        = fread_string( fp );
                fread_to_eol( fp );
                mprg->comlist        = fread_string( fp );
                fread_to_eol( fp );

                switch ( letter = fread_letter( fp ) )
                {
                    case '>':
                        CREATE( mprg->next, MPROG_DATA, 1 );
                        mprg = mprg->next;
                        break;

                    case '|':
                        mprg->next = NULL;
                        fread_to_eol( fp );
                        done = TRUE;
                        break;

                    default:
                        bug( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum );
                        exit( 1 );
                        break;
                }

                break;
        }
    }

    return;
}


/*************************************************************/
/*  Function to delete a room index.  Called from do_rdelete in build.c
    Narn, May/96
*/
bool delete_room( ROOM_INDEX_DATA* room )
{
    char buf[MAX_STRING_LENGTH];
    int hash;
    ROOM_INDEX_DATA* prev, *limbo = get_room_index( ROOM_VNUM_LIMBO );
    OBJ_DATA* o;
    CHAR_DATA* ch;
    EXTRA_DESCR_DATA* ed;
    EXIT_DATA* ex;
    MPROG_ACT_LIST* mpact;
    MPROG_DATA* mp;
    sprintf( buf, "%s%d", EQ_DIR, room->vnum );

    if ( file_exist( buf ) )
    {
        remove( buf );
        bug( "Destroyed EQ_SAVE room at %d.", room->vnum );
    }

    while ( ( ch = room->first_person ) != NULL )
    {
        if ( !IS_NPC( ch ) )
        {
            char_from_room( ch );
            char_to_room( ch, limbo );
        }
        else
            extract_char( ch, TRUE, FALSE );
    }

    while ( ( o = room->first_content ) != NULL )
        extract_obj( o );

    while ( ( ed = room->first_extradesc ) != NULL )
    {
        room->first_extradesc = ed->next;
        STRFREE( ed->keyword );
        STRFREE( ed->description );
        DISPOSE( ed );
        --top_ed;
    }

    while ( ( ex = room->first_exit ) != NULL )
        extract_exit( room, ex );

    while ( ( mpact = room->mpact ) != NULL )
    {
        room->mpact = mpact->next;
        DISPOSE( mpact->buf );
        DISPOSE( mpact );
    }

    while ( ( mp = room->mudprogs ) != NULL )
    {
        room->mudprogs = mp->next;
        STRFREE( mp->arglist );
        STRFREE( mp->comlist );
        DISPOSE( mp );
    }

    STRFREE( room->name );
    STRFREE( room->description );
    STRFREE( room->hdescription );
    hash = room->vnum % MAX_KEY_HASH;

    if ( room == room_index_hash[hash] )
        room_index_hash[hash] = room->next;
    else
    {
        for ( prev = room_index_hash[hash]; prev; prev = prev->next )
            if ( prev->next == room )
                break;

        if ( prev )
            prev->next = room->next;
        else
            bug( "delete_room: room %d not in hash bucket %d.", room->vnum, hash );
    }

    DISPOSE( room );
    --top_room;
    return TRUE;
}

/* See comment on delete_room. */
bool delete_obj( OBJ_INDEX_DATA* obj )
{
    int hash;
    OBJ_INDEX_DATA* prev;
    OBJ_DATA* o, *o_next;
    EXTRA_DESCR_DATA* ed;
    AFFECT_DATA* af;
    MPROG_DATA* mp;
    /* int auc; */

    /* Remove references to object index */
    for ( o = first_object; o; o = o_next )
    {
        o_next = o->next;

        if ( o->pIndexData == obj )
            extract_obj( o );
    }

    while ( ( ed = obj->first_extradesc ) != NULL )
    {
        obj->first_extradesc = ed->next;

        if ( ed->keyword )
            STRFREE( ed->keyword );

        if ( ed->description )
            STRFREE( ed->description );

        DISPOSE( ed );
        --top_ed;
    }

    while ( ( af = obj->first_affect ) != NULL )
    {
        obj->first_affect = af->next;
        DISPOSE( af );
        --top_affect;
    }

    while ( ( mp = obj->mudprogs ) != NULL )
    {
        obj->mudprogs = mp->next;
        STRFREE( mp->arglist );
        STRFREE( mp->comlist );
        DISPOSE( mp );
    }

    if ( obj->name )
        STRFREE( obj->name );

    if ( obj->short_descr )
        STRFREE( obj->short_descr );

    if ( obj->description )
        STRFREE( obj->description );

    if ( obj->action_desc )
        STRFREE( obj->action_desc );

    hash = obj->vnum % MAX_KEY_HASH;

    if ( obj == obj_index_hash[hash] )
        obj_index_hash[hash] = obj->next;
    else
    {
        for ( prev = obj_index_hash[hash]; prev; prev = prev->next )
            if ( prev->next == obj )
                break;

        if ( prev )
            prev->next = obj->next;
        else
            bug( "delete_obj: object %d not in hash bucket %d.", obj->vnum, hash );
    }

    DISPOSE( obj );
    --top_obj_index;
    return TRUE;
}

/* See comment on delete_room. */
bool delete_mob( MOB_INDEX_DATA* mob )
{
    int hash;
    MOB_INDEX_DATA* prev;
    CHAR_DATA* ch, *ch_next;
    MPROG_DATA* mp;

    for ( ch = first_char; ch; ch = ch_next )
    {
        ch_next = ch->next;

        if ( ch->pIndexData == mob )
            extract_char( ch, TRUE, FALSE );
    }

    while ( ( mp = mob->mudprogs ) != NULL )
    {
        mob->mudprogs = mp->next;
        STRFREE( mp->arglist );
        STRFREE( mp->comlist );
        DISPOSE( mp );
    }

    if ( mob->pShop )
    {
        UNLINK( mob->pShop, first_shop, last_shop, next, prev );
        DISPOSE( mob->pShop );
        --top_shop;
    }

    STRFREE( mob->player_name );
    STRFREE( mob->short_descr );
    STRFREE( mob->long_descr );
    STRFREE( mob->description );
    hash = mob->vnum % MAX_KEY_HASH;

    if ( mob == mob_index_hash[hash] )
        mob_index_hash[hash] = mob->next;
    else
    {
        for ( prev = mob_index_hash[hash]; prev; prev = prev->next )
            if ( prev->next == mob )
                break;

        if ( prev )
            prev->next = mob->next;
        else
            bug( "delete_mob: mobile %d not in hash bucket %d.", mob->vnum, hash );
    }

    DISPOSE( mob );
    --top_mob_index;
    return TRUE;
}

/*
    Creat a new room (for online building)           -Thoric
*/
ROOM_INDEX_DATA* make_room( int vnum )
{
    ROOM_INDEX_DATA* pRoomIndex;
    int iHash;
    CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
    pRoomIndex->first_person    = NULL;
    pRoomIndex->last_person     = NULL;
    pRoomIndex->first_content   = NULL;
    pRoomIndex->last_content    = NULL;
    pRoomIndex->first_extradesc = NULL;
    pRoomIndex->last_extradesc  = NULL;
    pRoomIndex->link                = NULL;
    pRoomIndex->area            = NULL;
    pRoomIndex->vnum            = vnum;
    pRoomIndex->name            = STRALLOC( "Floating in a void" );
    pRoomIndex->description     = STRALLOC( "" );
    pRoomIndex->hdescription        = STRALLOC( "" );
    xSET_BIT( pRoomIndex->room_flags, ROOM_PROTOTYPE );
    xCLEAR_BITS( pRoomIndex->hroom_flags );
    pRoomIndex->x                   = -1;
    pRoomIndex->y                   = -1;
    pRoomIndex->z                   = -1;
    xSET_BIT( pRoomIndex->room_flags, ROOM_NOCOORD );
    pRoomIndex->sector_type         = 1;
    pRoomIndex->light           = 0;
    pRoomIndex->first_exit      = NULL;
    pRoomIndex->last_exit       = NULL;
    iHash           = vnum % MAX_KEY_HASH;
    pRoomIndex->next    = room_index_hash[iHash];
    room_index_hash[iHash]  = pRoomIndex;
    top_room++;
    return pRoomIndex;
}

/*
    Create a new INDEX object (for online building)      -Thoric
    Option to clone an existing index object.
*/
OBJ_INDEX_DATA* make_object( int vnum, int cvnum, char* name )
{
    OBJ_INDEX_DATA* pObjIndex, *cObjIndex;
    char buf[MAX_STRING_LENGTH];
    int iHash;

    if ( cvnum > 0 )
        cObjIndex = get_obj_index( cvnum );
    else
        cObjIndex = NULL;

    CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
    pObjIndex->vnum         = vnum;
    pObjIndex->name         = STRALLOC( name );
    pObjIndex->first_affect     = NULL;
    pObjIndex->last_affect      = NULL;
    pObjIndex->first_extradesc  = NULL;
    pObjIndex->last_extradesc   = NULL;

    if ( !cObjIndex )
    {
        sprintf( buf, "A %s", name );
        pObjIndex->short_descr    = STRALLOC( buf  );
        sprintf( buf, "A %s is here.", name );
        pObjIndex->description    = STRALLOC( buf );
        pObjIndex->action_desc    = STRALLOC( "" );
        pObjIndex->short_descr[0] = LOWER( pObjIndex->short_descr[0] );
        pObjIndex->description[0] = UPPER( pObjIndex->description[0] );
        pObjIndex->item_type      = ITEM_TRASH;
        xSET_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
        xCLEAR_BITS( pObjIndex->wear_flags );
        pObjIndex->value[0]       = 0;
        pObjIndex->value[1]       = 0;
        pObjIndex->value[2]       = 0;
        pObjIndex->value[3]       = 0;
        pObjIndex->value[4]           = 0;
        pObjIndex->value[5]           = 0;
        pObjIndex->weight     = 1;
        pObjIndex->cost       = 0;
    }
    else
    {
        EXTRA_DESCR_DATA* ed,  *ced;
        AFFECT_DATA*      paf, *cpaf;
        pObjIndex->short_descr    = QUICKLINK( cObjIndex->short_descr );
        pObjIndex->description    = QUICKLINK( cObjIndex->description );
        pObjIndex->action_desc    = QUICKLINK( cObjIndex->action_desc );
        pObjIndex->item_type      = cObjIndex->item_type;
        pObjIndex->extra_flags    = cObjIndex->extra_flags;
        xSET_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
        pObjIndex->wear_flags     = cObjIndex->wear_flags;
        pObjIndex->value[0]       = cObjIndex->value[0];
        pObjIndex->value[1]       = cObjIndex->value[1];
        pObjIndex->value[2]       = cObjIndex->value[2];
        pObjIndex->value[3]       = cObjIndex->value[3];
        pObjIndex->value[4]           = cObjIndex->value[4];
        pObjIndex->value[5]           = cObjIndex->value[5];
        pObjIndex->weight     = cObjIndex->weight;
        pObjIndex->cost       = cObjIndex->cost;

        for ( ced = cObjIndex->first_extradesc; ced; ced = ced->next )
        {
            CREATE( ed, EXTRA_DESCR_DATA, 1 );
            ed->keyword     = QUICKLINK( ced->keyword );
            ed->description     = QUICKLINK( ced->description );
            LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
                  next, prev );
            top_ed++;
        }

        for ( cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next )
        {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type       = cpaf->type;
            paf->duration       = cpaf->duration;
            paf->location       = cpaf->location;
            paf->modifier       = cpaf->modifier;
            paf->bitvector      = cpaf->bitvector;
            LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
                  next, prev );
            top_affect++;
        }
    }

    pObjIndex->count        = 0;
    iHash               = vnum % MAX_KEY_HASH;
    pObjIndex->next         = obj_index_hash[iHash];
    obj_index_hash[iHash]       = pObjIndex;
    top_obj_index++;
    return pObjIndex;
}

/*
    Create a new INDEX mobile (for online building)      -Thoric
    Option to clone an existing index mobile.
*/
MOB_INDEX_DATA* make_mobile( int vnum, int cvnum, char* name )
{
    MOB_INDEX_DATA* pMobIndex, *cMobIndex;
    char buf[MAX_STRING_LENGTH];
    int iHash;

    if ( cvnum > 0 )
        cMobIndex = get_mob_index( cvnum );
    else
        cMobIndex = NULL;

    CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
    pMobIndex->vnum         = vnum;
    pMobIndex->count        = 0;
    pMobIndex->killed       = 0;
    pMobIndex->player_name      = STRALLOC( name );

    if ( !cMobIndex )
    {
        sprintf( buf, "A newly created %s", name );
        pMobIndex->short_descr    = STRALLOC( buf  );
        sprintf( buf, "Some god abandoned a newly created %s here.\n\r", name );
        pMobIndex->long_descr     = STRALLOC( buf );
        pMobIndex->description    = STRALLOC( "" );
        pMobIndex->short_descr[0] = LOWER( pMobIndex->short_descr[0] );
        pMobIndex->long_descr[0]  = UPPER( pMobIndex->long_descr[0] );
        pMobIndex->description[0] = UPPER( pMobIndex->description[0] );
        xSET_BIT( pMobIndex->act, ACT_IS_NPC );
        xSET_BIT( pMobIndex->act, ACT_PROTOTYPE );
        xCLEAR_BITS( pMobIndex->affected_by );
        pMobIndex->pShop      = NULL;
        pMobIndex->spec_fun       = NULL;
        pMobIndex->spec_2     = NULL;
        pMobIndex->spec_3     = NULL;
        pMobIndex->spec_4     = NULL;
        pMobIndex->mudprogs       = NULL;
        xCLEAR_BITS( pMobIndex->progtypes );
        pMobIndex->level      = 1;
        pMobIndex->exp        = 0;
        pMobIndex->position       = 8;
        pMobIndex->defposition    = 8;
        pMobIndex->sex        = 0;
        pMobIndex->perm_str       = 10;
        pMobIndex->perm_sta           = 10;
        pMobIndex->perm_rec           = 10;
        pMobIndex->perm_int           = 10;
        pMobIndex->perm_bra           = 10;
        pMobIndex->perm_per           = 10;
        pMobIndex->race       = 0;
    }
    else
    {
        pMobIndex->short_descr    = QUICKLINK( cMobIndex->short_descr );
        pMobIndex->long_descr     = QUICKLINK( cMobIndex->long_descr  );
        pMobIndex->description    = QUICKLINK( cMobIndex->description );
        pMobIndex->act            = cMobIndex->act;
        xSET_BIT( pMobIndex->act,  ACT_PROTOTYPE );
        pMobIndex->affected_by    = cMobIndex->affected_by;
        pMobIndex->pShop      = NULL;
        pMobIndex->spec_fun       = cMobIndex->spec_fun;
        pMobIndex->spec_2     = cMobIndex->spec_2;
        pMobIndex->spec_3     = cMobIndex->spec_3;
        pMobIndex->spec_4     = cMobIndex->spec_4;
        pMobIndex->mudprogs       = NULL;
        xCLEAR_BITS( pMobIndex->progtypes );
        pMobIndex->level      = cMobIndex->level;
        pMobIndex->exp        = cMobIndex->exp;
        pMobIndex->position       = cMobIndex->position;
        pMobIndex->defposition    = cMobIndex->defposition;
        pMobIndex->sex        = cMobIndex->sex;
        pMobIndex->perm_str       = cMobIndex->perm_str;
        pMobIndex->perm_sta           = cMobIndex->perm_sta;
        pMobIndex->perm_rec           = cMobIndex->perm_rec;
        pMobIndex->perm_int           = cMobIndex->perm_int;
        pMobIndex->perm_bra           = cMobIndex->perm_bra;
        pMobIndex->perm_per           = cMobIndex->perm_per;
        pMobIndex->race       = cMobIndex->race;
    }

    iHash               = vnum % MAX_KEY_HASH;
    pMobIndex->next         = mob_index_hash[iHash];
    mob_index_hash[iHash]       = pMobIndex;
    top_mob_index++;
    return pMobIndex;
}

/*
    Creates a simple exit with no fields filled but rvnum and optionally
    to_room and vnum.                        -Thoric
    Exits are inserted into the linked list based on vdir.
*/
EXIT_DATA* make_exit( ROOM_INDEX_DATA* pRoomIndex, ROOM_INDEX_DATA* to_room, sh_int door )
{
    EXIT_DATA* pexit, *texit;
    bool broke;
    CREATE( pexit, EXIT_DATA, 1 );
    pexit->vdir     = door;
    pexit->rvnum        = pRoomIndex->vnum;
    pexit->to_room      = to_room;
    pexit->distance     = 1;

    if ( to_room )
    {
        pexit->vnum = to_room->vnum;
        texit = get_exit_to( to_room, rev_dir[door], pRoomIndex->vnum );

        if ( texit )    /* assign reverse exit pointers */
        {
            texit->rexit = pexit;
            pexit->rexit = texit;
        }
    }

    broke = FALSE;

    for ( texit = pRoomIndex->first_exit; texit; texit = texit->next )
        if ( door < texit->vdir )
        {
            broke = TRUE;
            break;
        }

    if ( !pRoomIndex->first_exit )
        pRoomIndex->first_exit    = pexit;
    else
    {
        /* keep exits in incremental order - insert exit into list */
        if ( broke && texit )
        {
            if ( !texit->prev )
                pRoomIndex->first_exit    = pexit;
            else
                texit->prev->next     = pexit;

            pexit->prev         = texit->prev;
            pexit->next         = texit;
            texit->prev         = pexit;
            top_exit++;
            return pexit;
        }

        pRoomIndex->last_exit->next   = pexit;
    }

    pexit->next         = NULL;
    pexit->prev         = pRoomIndex->last_exit;
    pRoomIndex->last_exit       = pexit;
    top_exit++;
    return pexit;
}

void fix_area_exits( AREA_DATA* tarea )
{
    ROOM_INDEX_DATA* pRoomIndex;
    EXIT_DATA* pexit, *rev_exit;
    int rnum;
    bool fexit;

    for ( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
    {
        if ( ( pRoomIndex = get_room_index( rnum ) ) == NULL )
            continue;

        fexit = FALSE;

        for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
        {
            fexit = TRUE;
            pexit->rvnum = pRoomIndex->vnum;

            if ( pexit->vnum <= 0 )
                pexit->to_room = NULL;
            else
                pexit->to_room = get_room_index( pexit->vnum );
        }

        if ( !fexit )
            xSET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
    }

    for ( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
    {
        if ( ( pRoomIndex = get_room_index( rnum ) ) == NULL )
            continue;

        for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
        {
            if ( pexit->to_room && !pexit->rexit )
            {
                rev_exit = get_exit_to( pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum );

                if ( rev_exit )
                {
                    pexit->rexit    = rev_exit;
                    rev_exit->rexit = pexit;
                }
            }
        }
    }
}

void load_area_file( AREA_DATA* tarea, char* filename )
{
    int safty = 0;

    /*    FILE *fpin;
        what intelligent person stopped using fpArea?????
        if fpArea isn't being used, then no filename or linenumber
        is printed when an error occurs during loading the area..
        (bug uses fpArea)
          --TRI  */

    if ( fBootDb )
        tarea = last_area;

    if ( !fBootDb && !tarea )
    {
        bug( "Load_area: null area!" );
        return;
    }

    if ( ( fpArea = fopen( filename, "r" ) ) == NULL )
    {
        bug( "load_area: error loading file (can't open)" );
        bug( filename );
        return;
    }

    for ( ; ; )
    {
        char* word;

        if ( ++safty > 99999 )
        {
            bug( "load_area: Infinite loop override, could not load area." );
            exit( 1 );
        }

        if ( fread_letter( fpArea ) != '#' )
        {
            bug( tarea->filename );
            bug( "load_area: # not found." );
            exit( 1 );
        }

        word = fread_word( fpArea );

        // log_string( word );

        if ( word[0] == '$'               )
            break;
        else if ( !str_cmp( word, "AREA"     ) )
        {
            if ( fBootDb )
            {
                load_area    ( fpArea );
                tarea = last_area;
            }
            else
            {
                DISPOSE( tarea->name );
                tarea->name = fread_string_nohash( fpArea );
            }
        }
        else if ( !str_cmp( word, "AUTHOR"   ) )
            load_author  ( tarea, fpArea );
        else if ( !str_cmp( word, "AMBIENCE" ) )
            load_ambience( tarea, fpArea );
        else if ( !str_cmp( word, "VERSION"  ) )
            load_version ( tarea, fpArea );
        else if ( !str_cmp( word, "RANGES"   ) )
            load_ranges  ( tarea, fpArea );
        else if ( !str_cmp( word, "RESETMSG" ) )
            load_resetmsg( tarea, fpArea );
        else if ( !str_cmp( word, "HELPS"    ) )
            load_helps   ( tarea, fpArea );
        else if ( !str_cmp( word, "MOBILES"  ) )
            load_mobiles ( tarea, fpArea );
        else if ( !str_cmp( word, "MUDPROGS" ) )
            load_mudprogs( tarea, fpArea );
        else if ( !str_cmp( word, "OBJECTS"  ) )
            load_objects ( tarea, fpArea );
        else if ( !str_cmp( word, "OBJPROGS" ) )
            load_objprogs( tarea, fpArea );
        else if ( !str_cmp( word, "RESETS"   ) )
            load_resets  ( tarea, fpArea );
        else if ( !str_cmp( word, "ROOMS"    ) )
            load_rooms   ( tarea, fpArea );
        else if ( !str_cmp( word, "SHOPS"    ) )
            load_shops   ( tarea, fpArea );
        else if ( !str_cmp( word, "SPECIALS" ) )
            load_specials( tarea, fpArea );
        else
        {
            bug( tarea->filename );
            bug( "load_area: bad section name '%s'.", word );

            if ( fBootDb )
                exit( 1 );
            else
            {
                fclose( fpArea );
                fpArea = NULL;
                return;
            }
        }
    }

    fclose( fpArea );
    fpArea = NULL;

    if ( tarea )
    {
        if ( fBootDb )
            sort_area( tarea, FALSE );

        fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d Mobs: %5d - %d\n",
                 tarea->filename,
                 tarea->low_r_vnum, tarea->hi_r_vnum,
                 tarea->low_o_vnum, tarea->hi_o_vnum,
                 tarea->low_m_vnum, tarea->hi_m_vnum );

        if ( !tarea->author )
            tarea->author = STRALLOC( "" );

        SET_BIT( tarea->status, AREA_LOADED );
    }

    /*
        else
        fprintf( stderr, "(%s)\n", filename );
    */
}



/*  Build list of in_progress areas.  Do not load areas.
    define AREA_READ if you want it to build area names rather than reading
    them out of the area files. -- Altrag */
void load_buildlist( void )
{
    DIR* dp;
    struct dirent* dentry;
    FILE* fp;
    char buf[MAX_STRING_LENGTH];
    AREA_DATA* pArea;
    char line[81];
    char word[81];
    int low, hi;
    int mlow, mhi, olow, ohi, rlow, rhi;
    bool badfile = FALSE;
    char temp;
    dp = opendir( GOD_DIR );
    dentry = readdir( dp );

    while ( dentry )
    {
        if ( dentry->d_name[0] != '.' )
        {
            sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );

            if ( !( fp = fopen( buf, "r" ) ) )
            {
                bug( "Load_buildlist: invalid file" );
                dentry = readdir( dp );
                continue;
            }

            // log_string( buf );
            badfile = FALSE;
            rlow = rhi = olow = ohi = mlow = mhi = 0;

            while ( !feof( fp ) && !ferror( fp ) )
            {
                low = 0;
                hi = 0;
                word[0] = 0;
                line[0] = 0;

                if ( ( temp = fgetc( fp ) ) != EOF )
                    ungetc( temp, fp );
                else
                    break;

                fgets( line, 80, fp );
                sscanf( line, "%s %d %d", word, &low, &hi );

                if ( !strcmp( word, "Level" ) )
                {
                    if ( low < LEVEL_AVATAR )
                    {
                        sprintf( buf, "%s: God file with level %d < %d",
                                 dentry->d_name, low, LEVEL_AVATAR );
                        badfile = TRUE;
                    }
                }

                if ( !strcmp( word, "RoomRange" ) )
                    rlow = low, rhi = hi;
                else if ( !strcmp( word, "MobRange" ) )
                    mlow = low, mhi = hi;
                else if ( !strcmp( word, "ObjRange" ) )
                    olow = low, ohi = hi;
            }

            fclose( fp );

            if ( rlow && rhi && !badfile )
            {
                sprintf( buf, "%s%s.are", BUILD_DIR, dentry->d_name );

                if ( !( fp = fopen( buf, "r" ) ) )
                {
                    fpArea = NULL;
                    bug( "Load_buildlist: cannot open area file for read" );
                    dentry = readdir( dp );
                    continue;
                }

#if !defined(READ_AREA)  /* Dont always want to read stuff.. dunno.. shrug */
                strcpy( word, fread_word( fp ) );

                if ( word[0] != '#' || strcmp( &word[1], "AREA" ) )
                {
                    sprintf( buf, "Make_buildlist: %s.are: no #AREA found.",
                             dentry->d_name );
                    fclose( fp );
                    dentry = readdir( dp );
                    continue;
                }

#endif
                CREATE( pArea, AREA_DATA, 1 );
                sprintf( buf, "%s.are", dentry->d_name );
                pArea->author = STRALLOC( dentry->d_name );
                pArea->filename = str_dup( buf );
#if !defined(READ_AREA)
                pArea->name = fread_string_nohash( fp );
#else
                sprintf( buf, "{PROTO} %s's area in progress", dentry->d_name );
                pArea->name = str_dup( buf );
#endif
                fclose( fp );
                pArea->low_r_vnum = rlow;
                pArea->hi_r_vnum = rhi;
                pArea->low_m_vnum = mlow;
                pArea->hi_m_vnum = mhi;
                pArea->low_o_vnum = olow;
                pArea->hi_o_vnum = ohi;
                pArea->low_soft_range = -1;
                pArea->hi_soft_range = -1;
                pArea->low_hard_range = -1;
                pArea->hi_hard_range = -1;
                pArea->first_reset = NULL;
                pArea->last_reset = NULL;
                LINK( pArea, first_build, last_build, next, prev );
                fprintf( stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d "
                         "Mobs: %5d - %-5d\n",
                         pArea->filename,
                         pArea->low_r_vnum, pArea->hi_r_vnum,
                         pArea->low_o_vnum, pArea->hi_o_vnum,
                         pArea->low_m_vnum, pArea->hi_m_vnum );
                sort_area( pArea, TRUE );
            }
        }

        dentry = readdir( dp );
    }

    closedir( dp );
}


/*
    Sort by room vnums                   -Altrag & Thoric
*/
void sort_area( AREA_DATA* pArea, bool proto )
{
    AREA_DATA* area = NULL;
    AREA_DATA* first_sort, *last_sort;
    bool found;

    if ( !pArea )
    {
        bug( "Sort_area: NULL pArea" );
        return;
    }

    if ( proto )
    {
        first_sort = first_bsort;
        last_sort  = last_bsort;
    }
    else
    {
        first_sort = first_asort;
        last_sort  = last_asort;
    }

    found = FALSE;
    pArea->next_sort = NULL;
    pArea->prev_sort = NULL;

    if ( !first_sort )
    {
        pArea->prev_sort = NULL;
        pArea->next_sort = NULL;
        first_sort   = pArea;
        last_sort    = pArea;
        found = TRUE;
    }
    else
        for ( area = first_sort; area; area = area->next_sort )
            if ( pArea->low_r_vnum < area->low_r_vnum )
            {
                if ( !area->prev_sort )
                    first_sort    = pArea;
                else
                    area->prev_sort->next_sort = pArea;

                pArea->prev_sort = area->prev_sort;
                pArea->next_sort = area;
                area->prev_sort  = pArea;
                found = TRUE;
                break;
            }

    if ( !found )
    {
        pArea->prev_sort     = last_sort;
        pArea->next_sort     = NULL;
        last_sort->next_sort = pArea;
        last_sort        = pArea;
    }

    if ( proto )
    {
        first_bsort = first_sort;
        last_bsort  = last_sort;
    }
    else
    {
        first_asort = first_sort;
        last_asort  = last_sort;
    }
}


/*
    Display vnums currently assigned to areas        -Altrag & Thoric
    Sorted, and flagged if loaded.
*/
void show_vnums( CHAR_DATA* ch, int low, int high, bool proto, bool shownl,
                 char* loadst, char* notloadst )
{
    AREA_DATA* pArea, *first_sort;
    int count, loaded;
    count = 0;
    loaded = 0;

    if ( proto )
        first_sort = first_bsort;
    else
        first_sort = first_asort;

    for ( pArea = first_sort; pArea; pArea = pArea->next_sort )
    {
        if ( IS_SET( pArea->status, AREA_DELETED ) )
            continue;

        if ( pArea->low_r_vnum < low )
            continue;

        if ( pArea->hi_r_vnum > high )
            break;

        if ( IS_SET( pArea->status, AREA_LOADED ) )
            loaded++;
        else if ( !shownl )
            continue;

        ch_printf( ch, "&B%-14s &z| V: &Y%-2d &zR: &C%5d - %-5d"
                   " &zO: &C%5d - %-5d &zM: &C%5d - %-5d%s\n\r",
                   ( pArea->filename ? pArea->filename : "&R(invalid)" ),
                   pArea->version,
                   pArea->low_r_vnum, pArea->hi_r_vnum,
                   pArea->low_o_vnum, pArea->hi_o_vnum,
                   pArea->low_m_vnum, pArea->hi_m_vnum,
                   IS_SET( pArea->status, AREA_LOADED ) ? loadst : notloadst );
        count++;
    }

    ch_printf( ch, "&zAreas listed: &C%d  &zLoaded: &G%d &z(Current version: &Y%d&z)\n\r", count, loaded, AREA_VERSION );
    return;
}

/*
    Shows prototype vnums ranges, and if loaded
*/

void do_vnums( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;
    high = MAX_VNUMS;

    if ( arg1[0] != '\0' )
    {
        low = atoi( arg1 );

        if ( arg2[0] != '\0' )
            high = atoi( arg2 );
    }

    show_vnums( ch, low, high, TRUE, TRUE, " &G*", "" );
}

/*
    Shows installed areas, sorted.  Mark unloaded areas with an X
*/
void do_zones( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;
    do_vnums( ch, argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;
    high = MAX_VNUMS;

    if ( arg1[0] != '\0' )
    {
        low = atoi( arg1 );

        if ( arg2[0] != '\0' )
            high = atoi( arg2 );
    }

    show_vnums( ch, low, high, FALSE, TRUE, "", " &RX" );
}

/*
    Show prototype areas, sorted.  Only show loaded areas
*/
void do_newzones( CHAR_DATA* ch, char* argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;
    high = MAX_VNUMS;

    if ( arg1[0] != '\0' )
    {
        low = atoi( arg1 );

        if ( arg2[0] != '\0' )
            high = atoi( arg2 );
    }

    show_vnums( ch, low, high, TRUE, FALSE, "", " X" );
}

/*
    Save system info to data file
*/
void save_sysdata( SYSTEM_DATA sys )
{
    FILE* fp;
    char filename[MAX_INPUT_LENGTH];
    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );
    fclose( fpReserve );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "save_sysdata: fopen" );
    }
    else
    {
        fprintf( fp, "#SYSTEM\n" );
        fprintf( fp, "Highplayers    %d\n", sys.alltimemax      );
        fprintf( fp, "Highplayertime %s~\n", sys.time_of_max        );
        fprintf( fp, "Nameresolving  %d\n", sys.NO_NAME_RESOLVING   );
        fprintf( fp, "Allowooc       %d\n", sys.ALLOW_OOC               );
        fprintf( fp, "Setwizlock     %d\n", sys.SET_WIZLOCK             );
        fprintf( fp, "Waitforauth    %d\n", sys.WAIT_FOR_AUTH       );
        fprintf( fp, "Readallmail    %d\n", sys.read_all_mail       );
        fprintf( fp, "Readmailfree   %d\n", sys.read_mail_free      );
        fprintf( fp, "Writemailfree  %d\n", sys.write_mail_free     );
        fprintf( fp, "Takeothersmail %d\n", sys.take_others_mail    );
        fprintf( fp, "Currid         %d\n", sys.currid                  );
        fprintf( fp, "Muse           %d\n", sys.muse_level      );
        fprintf( fp, "Think          %d\n", sys.think_level     );
        fprintf( fp, "Build          %d\n", sys.build_level     );
        fprintf( fp, "Log            %d\n", sys.log_level       );
        fprintf( fp, "Protoflag      %d\n", sys.level_modify_proto  );
        fprintf( fp, "Overridepriv   %d\n", sys.level_override_private  );
        fprintf( fp, "Msetplayer     %d\n", sys.level_mset_player   );
        fprintf( fp, "Stunplrvsplr   %d\n", sys.stun_plr_vs_plr     );
        fprintf( fp, "Stunregular    %d\n", sys.stun_regular        );
        fprintf( fp, "Damplrvsplr    %d\n", sys.dam_plr_vs_plr      );
        fprintf( fp, "Damplrvsmob    %d\n", sys.dam_plr_vs_mob      );
        fprintf( fp, "Dammobvsplr    %d\n", sys.dam_mob_vs_plr      );
        fprintf( fp, "Dammobvsmob    %d\n", sys.dam_mob_vs_mob      );
        fprintf( fp, "Forcepc        %d\n", sys.level_forcepc       );
        fprintf( fp, "Guildoverseer  %s~\n", sys.guild_overseer     );
        fprintf( fp, "Guildadvisor   %s~\n", sys.guild_advisor      );
        fprintf( fp, "Saveflags      %d\n", sys.save_flags      );
        fprintf( fp, "Savefreq       %d\n", sys.save_frequency      );
        fprintf( fp, "Newbie_purge   %d\n", sys.newbie_purge            );
        fprintf( fp, "Regular_purge  %d\n", sys.regular_purge           );
        fprintf( fp, "Autopurge      %d\n", sys.CLEANPFILES             );
        fprintf( fp, "Tmcblock       %d\n", sys.TMCBLOCK                );
        fprintf( fp, "Exe_file       %s~\n", sys.exe_file );
        fprintf( fp, "MQTT_Host      %s~\n", sys.mqtt_host );
        fprintf( fp, "MQTT_Port      %d\n", sys.mqtt_port );
        fprintf( fp, "MQTT_Enabled   %d\n", sys.mqtt_enabled );
        fprintf( fp, "End\n\n"                      );
        fprintf( fp, "#END\n"                       );
    }

    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}


void fread_sysdata( SYSTEM_DATA* sys, FILE* fp )
{
    char* word;
    bool fMatch;
    sys->time_of_max = NULL;

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER( word[0] ) )
        {
            case '*':
                fMatch = TRUE;
                fread_to_eol( fp );
                break;

            case 'A':
                KEY( "Allowooc",       sys->ALLOW_OOC,        fread_number( fp ) );
                KEY( "Autopurge",      sys->CLEANPFILES,      fread_number( fp ) );
                break;

            case 'B':
                KEY( "Build",      sys->build_level,      fread_number( fp ) );
                break;

            case 'C':
                KEY( "Currid",         sys->currid,           fread_number( fp ) );
                break;

            case 'D':
                KEY( "Damplrvsplr",    sys->dam_plr_vs_plr,   fread_number( fp ) );
                KEY( "Damplrvsmob",    sys->dam_plr_vs_mob,   fread_number( fp ) );
                KEY( "Dammobvsplr",    sys->dam_mob_vs_plr,   fread_number( fp ) );
                KEY( "Dammobvsmob",    sys->dam_mob_vs_mob,   fread_number( fp ) );
                break;

            case 'E':
                KEY( "Exe_file", sys->exe_file, fread_string( fp ) );
                if ( !str_cmp( word, "End" ) )
                {
                    if ( !sys->time_of_max )
                        sys->time_of_max = str_dup( "(not recorded)" );

                    return;
                }

                break;

            case 'F':
                KEY( "Forcepc",    sys->level_forcepc,    fread_number( fp ) );
                break;

            case 'G':
                KEY( "Guildoverseer",  sys->guild_overseer,  fread_string( fp ) );
                KEY( "Guildadvisor",   sys->guild_advisor,   fread_string( fp ) );
                break;

            case 'H':
                KEY( "Highplayers",    sys->alltimemax,   fread_number( fp ) );
                KEY( "Highplayertime", sys->time_of_max,      fread_string_nohash( fp ) );
                break;

            case 'L':
                KEY( "Log",        sys->log_level,    fread_number( fp ) );
                break;

            case 'M':
                KEY( "Msetplayer",     sys->level_mset_player, fread_number( fp ) );
                KEY( "Muse",       sys->muse_level,    fread_number( fp ) );
                KEY( "MQTT_Host",   sys->mqtt_host, fread_string(fp));
                KEY( "MQTT_Port",   sys->mqtt_port, fread_number(fp));
                KEY( "MQTT_Enabled",   sys->mqtt_enabled, fread_number(fp));
                break;

            case 'N':
                KEY( "Nameresolving",  sys->NO_NAME_RESOLVING, fread_number( fp ) );
                KEY( "Newbie_purge",      sys->newbie_purge, fread_number( fp ) );
                break;

            case 'O':
                KEY( "Overridepriv",   sys->level_override_private, fread_number( fp ) );
                break;

            case 'P':
                KEY( "Protoflag",      sys->level_modify_proto, fread_number( fp ) );
                break;

            case 'R':
                KEY( "Readallmail",    sys->read_all_mail,  fread_number( fp ) );
                KEY( "Readmailfree",   sys->read_mail_free, fread_number( fp ) );
                KEY( "Regular_purge", sys->regular_purge, fread_number( fp ) );
                break;

            case 'S':
                KEY( "Stunplrvsplr",   sys->stun_plr_vs_plr, fread_number( fp ) );
                KEY( "Stunregular",    sys->stun_regular,   fread_number( fp ) );
                KEY( "Saveflags",      sys->save_flags, fread_number( fp ) );
                KEY( "Savefreq",       sys->save_frequency, fread_number( fp ) );
                KEY( "Setwizlock",     sys->SET_WIZLOCK,    fread_number( fp ) );
                break;

            case 'T':
                KEY( "Takeothersmail", sys->take_others_mail, fread_number( fp ) );
                KEY( "Think",          sys->think_level,      fread_number( fp ) );
                KEY( "Tmcblock",       sys->TMCBLOCK,         fread_number( fp ) );
                break;

            case 'W':
                KEY( "Waitforauth",    sys->WAIT_FOR_AUTH,    fread_number( fp ) );
                KEY( "Writemailfree",  sys->write_mail_free,  fread_number( fp ) );
                break;
        }

        if ( !fMatch )
        {
            bug( "Fread_sysdata: no match: %s", word );
        }
    }
}



/*
    Load the sysdata file
*/
bool load_systemdata( SYSTEM_DATA* sys )
{
    char filename[MAX_INPUT_LENGTH];
    FILE* fp;
    bool found;
    found = FALSE;
    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        found = TRUE;

        for ( ; ; )
        {
            char letter;
            char* word;
            letter = fread_letter( fp );

            if ( letter == '*' )
            {
                fread_to_eol( fp );
                continue;
            }

            if ( letter != '#' )
            {
                bug( "Load_sysdata_file: # not found." );
                break;
            }

            word = fread_word( fp );

            if ( !str_cmp( word, "SYSTEM" ) )
            {
                fread_sysdata( sys, fp );
                break;
            }
            else if ( !str_cmp( word, "END"  ) )
                break;
            else
            {
                bug( "Load_sysdata_file: bad section." );
                break;
            }
        }

        fclose( fp );
    }

    if ( !sysdata.guild_overseer )
        sysdata.guild_overseer = str_dup( "" );

    if ( !sysdata.guild_advisor  )
        sysdata.guild_advisor  = str_dup( "" );

    return found;
}

void load_xnamelist( void )
{
    FILE* fp;
    char* line;
    struct xname_data* xname;
    long a;
    xnames = NULL;

    if ( !( fp = fopen( SYSTEM_DIR XNAME_LIST, "r" ) ) )
        return;

    for ( ; ; )
    {
        if ( feof( fp ) )
        {
            perror( "Load_xnames: no -1 found." );
            fclose( fp );
            return;
        }

        if ( !( line = fread_line( fp ) ) )
            return ;

        if ( strstr( line, "###" ) )
            return;

        CREATE( xname, struct xname_data, 1 );
        sscanf( line, "%s %ld", xname->name, &a );
        xname->time = ( time_t )a;
        xname->next = xnames;
    }
}

void load_mplist( void )
{
    FILE* fp;
    char* line/*,line2[MAX_STRING_LENGTH]*/;
    struct allowmp_data* mpitem;

    if ( !( fp = fopen( SYSTEM_DIR MP_LIST, "r" ) ) )
        return;

    for ( ; ; )
    {
        if ( feof( fp ) )
        {
            perror( "Load_banlist: no -1 found." );
            fclose( fp );
            return;
        }

        CREATE( mpitem, struct allowmp_data, 1 );

        if ( !( line = fread_string( fp ) ) )
            return;

        // perror(line);
        if ( strstr( line, "###" ) )
            return;

        // perror("after read line");
        if ( *line == '\0' )
            return;

        line = one_argument( line, mpitem->by );
        line = one_argument( line, mpitem->name );
        one_argument( line, mpitem->host );

        // perror(" Got to after sscanf");
        if ( mplist )
            mpitem->next = mplist;

        mplist = mpitem;
    }

    fclose( fp );
}

void load_banlist( void )
{
    BAN_DATA* pban;
    FILE* fp;
    int number;
    char letter;

    if ( !( fp = fopen( SYSTEM_DIR BAN_LIST, "r" ) ) )
        return;

    for ( ; ; )
    {
        if ( feof( fp ) )
        {
            bug( "Load_banlist: no -1 found." );
            fclose( fp );
            return;
        }

        number = fread_number( fp );

        if ( number == -1 )
        {
            fclose( fp );
            return;
        }

        CREATE( pban, BAN_DATA, 1 );
        pban->level = number;
        pban->name = fread_string_nohash( fp );

        if ( ( letter = fread_letter( fp ) ) == '~' )
            pban->ban_time = fread_string_nohash( fp );
        else
        {
            ungetc( letter, fp );
            pban->ban_time = str_dup( "(unrecorded)" );
        }

        LINK( pban, first_ban, last_ban, next, prev );
    }
}

/* Check to make sure range of vnums is free - Scryn 2/27/96 */

void do_check_vnums( CHAR_DATA* ch, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    AREA_DATA* pArea;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    bool room, mob, obj, all, area_conflict;
    int low_range, high_range;
    room = FALSE;
    mob  = FALSE;
    obj  = FALSE;
    all  = FALSE;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Please specify room, mob, object, or all as your first argument.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "room" ) )
        room = TRUE;
    else if ( !str_cmp( arg1, "mob" ) )
        mob = TRUE;
    else if ( !str_cmp( arg1, "object" ) )
        obj = TRUE;
    else if ( !str_cmp( arg1, "all" ) )
        all = TRUE;
    else
    {
        send_to_char( "Please specify room, mob, or object as your first argument.\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_char( "Please specify the low end of the range to be searched.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Please specify the high end of the range to be searched.\n\r", ch );
        return;
    }

    low_range = atoi( arg2 );
    high_range = atoi( argument );

    if ( low_range < 1 || low_range > MAX_VNUMS )
    {
        send_to_char( "Invalid argument for bottom of range.\n\r", ch );
        return;
    }

    if ( high_range < 1 || high_range > MAX_VNUMS )
    {
        send_to_char( "Invalid argument for top of range.\n\r", ch );
        return;
    }

    if ( high_range < low_range )
    {
        send_to_char( "Bottom of range must be below top of range.\n\r", ch );
        return;
    }

    if ( all )
    {
        sprintf( buf, "room %d %d", low_range, high_range );
        do_check_vnums( ch, buf );
        sprintf( buf, "mob %d %d", low_range, high_range );
        do_check_vnums( ch, buf );
        sprintf( buf, "object %d %d", low_range, high_range );
        do_check_vnums( ch, buf );
        return;
    }

    set_char_color( AT_PLAIN, ch );

    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;

        if ( IS_SET( pArea->status, AREA_DELETED ) )
            continue;
        else if ( room )
        {
            if ( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
                area_conflict = TRUE;

            if ( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
                area_conflict = TRUE;

            if ( ( low_range >= pArea->low_r_vnum )
                    && ( low_range <= pArea->hi_r_vnum ) )
                area_conflict = TRUE;

            if ( ( high_range <= pArea->hi_r_vnum )
                    && ( high_range >= pArea->low_r_vnum ) )
                area_conflict = TRUE;
        }

        if ( mob )
        {
            if ( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
                area_conflict = TRUE;

            if ( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
                area_conflict = TRUE;

            if ( ( low_range >= pArea->low_m_vnum )
                    && ( low_range <= pArea->hi_m_vnum ) )
                area_conflict = TRUE;

            if ( ( high_range <= pArea->hi_m_vnum )
                    && ( high_range >= pArea->low_m_vnum ) )
                area_conflict = TRUE;
        }

        if ( obj )
        {
            if ( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
                area_conflict = TRUE;

            if ( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
                area_conflict = TRUE;

            if ( ( low_range >= pArea->low_o_vnum )
                    && ( low_range <= pArea->hi_o_vnum ) )
                area_conflict = TRUE;

            if ( ( high_range <= pArea->hi_o_vnum )
                    && ( high_range >= pArea->low_o_vnum ) )
                area_conflict = TRUE;
        }

        if ( area_conflict )
        {
            sprintf( buf, "Conflict:%-15s| ",
                     ( pArea->filename ? pArea->filename : "(invalid)" ) );

            if ( room )
                sprintf( buf2, "Rooms: %5d - %-5d\n\r", pArea->low_r_vnum,
                         pArea->hi_r_vnum );

            if ( mob )
                sprintf( buf2, "Mobs: %5d - %-5d\n\r", pArea->low_m_vnum,
                         pArea->hi_m_vnum );

            if ( obj )
                sprintf( buf2, "Objects: %5d - %-5d\n\r", pArea->low_o_vnum,
                         pArea->hi_o_vnum );

            strcat( buf, buf2 );
            send_to_char( buf, ch );
        }
    }

    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;

        if ( IS_SET( pArea->status, AREA_DELETED ) )
            continue;
        else if ( room )
        {
            if ( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
                area_conflict = TRUE;

            if ( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
                area_conflict = TRUE;

            if ( ( low_range >= pArea->low_r_vnum )
                    && ( low_range <= pArea->hi_r_vnum ) )
                area_conflict = TRUE;

            if ( ( high_range <= pArea->hi_r_vnum )
                    && ( high_range >= pArea->low_r_vnum ) )
                area_conflict = TRUE;
        }

        if ( mob )
        {
            if ( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
                area_conflict = TRUE;

            if ( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
                area_conflict = TRUE;

            if ( ( low_range >= pArea->low_m_vnum )
                    && ( low_range <= pArea->hi_m_vnum ) )
                area_conflict = TRUE;

            if ( ( high_range <= pArea->hi_m_vnum )
                    && ( high_range >= pArea->low_m_vnum ) )
                area_conflict = TRUE;
        }

        if ( obj )
        {
            if ( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
                area_conflict = TRUE;

            if ( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
                area_conflict = TRUE;

            if ( ( low_range >= pArea->low_o_vnum )
                    && ( low_range <= pArea->hi_o_vnum ) )
                area_conflict = TRUE;

            if ( ( high_range <= pArea->hi_o_vnum )
                    && ( high_range >= pArea->low_o_vnum ) )
                area_conflict = TRUE;
        }

        if ( area_conflict )
        {
            sprintf( buf, "Conflict:%-15s| ",
                     ( pArea->filename ? pArea->filename : "(invalid)" ) );

            if ( room )
                sprintf( buf2, "Rooms: %5d - %-5d\n\r", pArea->low_r_vnum,
                         pArea->hi_r_vnum );

            if ( mob )
                sprintf( buf2, "Mobs: %5d - %-5d\n\r", pArea->low_m_vnum,
                         pArea->hi_m_vnum );

            if ( obj )
                sprintf( buf2, "Objects: %5d - %-5d\n\r", pArea->low_o_vnum,
                         pArea->hi_o_vnum );

            strcat( buf, buf2 );
            send_to_char( buf, ch );
        }
    }

    /*
        for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
        {
            area_conflict = FALSE;
        if ( IS_SET( pArea->status, AREA_DELETED ) )
           continue;
        else
        if (room)
          if((pArea->low_r_vnum >= low_range)
          && (pArea->hi_r_vnum <= high_range))
            area_conflict = TRUE;

        if (mob)
          if((pArea->low_m_vnum >= low_range)
          && (pArea->hi_m_vnum <= high_range))
            area_conflict = TRUE;

        if (obj)
          if((pArea->low_o_vnum >= low_range)
          && (pArea->hi_o_vnum <= high_range))
            area_conflict = TRUE;

        if (area_conflict)
          ch_printf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
                 " Objs: %5d - %-5d Mobs: %5d - %-5d\n\r",
            (pArea->filename ? pArea->filename : "(invalid)"),
            pArea->low_r_vnum, pArea->hi_r_vnum,
            pArea->low_o_vnum, pArea->hi_o_vnum,
            pArea->low_m_vnum, pArea->hi_m_vnum );
        }

        for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
        {
            area_conflict = FALSE;
        if ( IS_SET( pArea->status, AREA_DELETED ) )
           continue;
        else
        if (room)
          if((pArea->low_r_vnum >= low_range)
          && (pArea->hi_r_vnum <= high_range))
            area_conflict = TRUE;

        if (mob)
          if((pArea->low_m_vnum >= low_range)
          && (pArea->hi_m_vnum <= high_range))
            area_conflict = TRUE;

        if (obj)
          if((pArea->low_o_vnum >= low_range)
          && (pArea->hi_o_vnum <= high_range))
            area_conflict = TRUE;

        if (area_conflict)
          sprintf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
                 " Objs: %5d - %-5d Mobs: %5d - %-5d\n\r",
            (pArea->filename ? pArea->filename : "(invalid)"),
            pArea->low_r_vnum, pArea->hi_r_vnum,
            pArea->low_o_vnum, pArea->hi_o_vnum,
            pArea->low_m_vnum, pArea->hi_m_vnum );
        }
    */
    return;
}

/*
    This function is here to aid in debugging.
    If the last expression in a function is another function call,
     gcc likes to generate a JMP instead of a CALL.
    This is called "tail chaining."
    It hoses the debugger call stack for that call.
    So I make this the last call in certain critical functions,
     where I really need the call stack to be right for debugging!

    If you don't understand this, then LEAVE IT ALONE.
    Don't remove any calls to tail_chain anywhere.

    -- Furey
*/
void tail_chain( void )
{
    return;
}

void save_xnames( void )
{
    FILE* fp;
    struct xname_data* xname;

    if ( !( fp = fopen( SYSTEM_DIR XNAME_LIST, "w" ) ) )
        return;

    for ( xname = xnames; xname; xname = xname->next )
        fprintf( fp, "%s %ld\n", xname->name, ( long )xname->time );

    fprintf( fp, "###" );
    fclose( fp );
}

/*
    file_size ( File Path )
    Returns the size of a specified file (by path).
*/
int file_size( char* buf )
{
    FILE* fp;
    int size = 0;

    if ( ( fp = fopen( buf, "rb" ) ) == NULL )
    {
        bug( "file_size: could not open file to retrive size (%s).", buf );
        return -1;
    }

    /* Seek to end of file */
    if ( fseek( fp, 0, SEEK_END ) != 0 )
    {
        bug( "file_size: failed seek-to-end operation." );
        fclose( fp );
        return -1;
    }

    /* Returns the number of characters from the beginning */
    size = ftell( fp );
    fclose( fp );
    return size;
}

/*
    EMERGENCY RECOVER FEATURE ---->
    Called for boot_db system, this function triggers
    if the area list cannot be located. It will generate
    a temporary area and a new area list. <-=Ghost=->
*/
void regenerate_limbo( void )
{
}

void memory_cleanup( void )
{
    int count = 0;
    printf( "<---Initating Memory Cleanup--->\n" );
    printf( "Stage 1: Removing Help files\n" );
    {
        HELP_DATA* tHelp, * nHelp;

        for ( tHelp = first_help; tHelp; tHelp = nHelp )
        {
            nHelp = tHelp->next;
            STRFREE( tHelp->text );
            STRFREE( tHelp->keyword );
            DISPOSE( tHelp );
            count++;
        }
    }
    printf( "Removed %d helpfiles from memory.\n", count );
    count = 0;
    return;
}

void deploy_map( void )
{
    ROOM_INDEX_DATA* room;
    int iHash;
    /* Reset the map */
    map_reset_all( );

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( room = room_index_hash[iHash]; room; room = room->next )
        {
            if ( xIS_SET( room->room_flags, ROOM_MAPSTART ) )
                map_apply_coords( room, 0, 0, 0 );
        }
    }

    /* Reset it again, with the alert */
    map_reset_alert(  );
    return;
}

void map_reset_all( void )
{
    ROOM_INDEX_DATA* pRoomIndex;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex  = room_index_hash[iHash]; pRoomIndex; pRoomIndex  = pRoomIndex->next )
        {
            xREMOVE_BIT( pRoomIndex->room_flags, BFS_MARK );
        }
    }

    return;
}

void map_reset_alert( void )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA* pRoomIndex;
    int iHash, cnt = 0;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex  = pRoomIndex->next )
        {
            if ( xIS_SET( pRoomIndex->room_flags, BFS_MARK ) )
            {
                xREMOVE_BIT( pRoomIndex->room_flags, BFS_MARK );
            }
            else if ( pRoomIndex->first_exit != NULL )
            {
                cnt++;
            }
        }
    }

    sprintf( buf, "[Alert]: Mapping system could not tag %d rooms.", cnt );
    log_string( buf );
    return;
}

void map_apply_coords( ROOM_INDEX_DATA* room, int x, int y, int z )
{
    /*
        Is this room 'clean' of the mapping?
    */
    if ( xIS_SET( room->room_flags, BFS_MARK ) )
        return;

    /* Mark the room as being hit by the Mapping */
    xSET_BIT( room->room_flags, BFS_MARK );
    xREMOVE_BIT( room->room_flags, ROOM_NOCOORD );
    room->x = x;
    room->y = y;
    room->z = z;
    /* Spread the explosion to nearby rooms */
    {
        EXIT_DATA* pexit;

        for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        {
            if ( pexit->to_room && pexit->to_room != room )
            {
                /* Mapping check */
                if ( xIS_SET( pexit->to_room->room_flags, BFS_MARK ) )
                {
                    bool safe = FALSE;

                    switch ( pexit->vdir )
                    {
                        default:
                            safe = TRUE;
                            break;

                        case 0:
                            if ( ( pexit->to_room->x == ( x + 0 ) ) && ( pexit->to_room->y == ( y + 1 ) ) && ( pexit->to_room->z == ( z + 0 ) ) )
                            {
                                safe = TRUE;
                            }

                            break;

                        case 1:
                            if ( ( pexit->to_room->x == ( x + 1 ) ) && ( pexit->to_room->y == ( y + 0 ) ) && ( pexit->to_room->z == ( z + 0 ) ) )
                            {
                                safe = TRUE;
                            }

                            break;

                        case 2:
                            if ( ( pexit->to_room->x == ( x + 0 ) ) && ( pexit->to_room->y == ( y - 1 ) ) && ( pexit->to_room->z == ( z + 0 ) ) )
                            {
                                safe = TRUE;
                            }

                            break;

                        case 3:
                            if ( ( pexit->to_room->x == ( x - 1 ) ) && ( pexit->to_room->y == ( y + 0 ) ) && ( pexit->to_room->z == ( z + 0 ) ) )
                            {
                                safe = TRUE;
                            }

                            break;

                        case 4:
                            if ( ( pexit->to_room->x == ( x + 0 ) ) && ( pexit->to_room->y == ( y + 0 ) ) && ( pexit->to_room->z == ( z + 1 ) ) )
                            {
                                safe = TRUE;
                            }

                            break;

                        case 5:
                            if ( ( pexit->to_room->x == ( x + 0 ) ) && ( pexit->to_room->y == ( y + 0 ) ) && ( pexit->to_room->z == ( z - 1 ) ) )
                            {
                                safe = TRUE;
                            }

                            break;

                        case 6:
                            if ( ( pexit->to_room->x == ( x + 1 ) ) && ( pexit->to_room->y == ( y + 1 ) ) && ( pexit->to_room->z == ( z + 0 ) ) )
                            {
                                safe = TRUE;
                            }

                            break;

                        case 7:
                            if ( ( pexit->to_room->x == ( x - 1 ) ) && ( pexit->to_room->y == ( y + 1 ) ) && ( pexit->to_room->z == ( z + 0 ) ) )
                            {
                                safe = TRUE;
                            }

                            break;

                        case 8:
                            if ( ( pexit->to_room->x == ( x + 1 ) ) && ( pexit->to_room->y == ( y - 1 ) ) && ( pexit->to_room->z == ( z + 0 ) ) )
                            {
                                safe = TRUE;
                            }

                            break;

                        case 9:
                            if ( ( pexit->to_room->x == ( x - 1 ) ) && ( pexit->to_room->y == ( y - 1 ) ) && ( pexit->to_room->z == ( z + 0 ) ) )
                            {
                                safe = TRUE;
                            }

                            break;
                    }

                    if ( !safe )
                    {
                        char buf[MAX_STRING_LENGTH];
                        sprintf( buf, "Overlap failure connecting rooms %d and %d.", room->vnum, pexit->to_room->vnum );
                        bug( buf );
                    }
                }

                /* Continue mapping */
                switch ( pexit->vdir )
                {
                    default:
                        break;

                    case 0:
                        map_apply_coords( pexit->to_room, x + 0, y + 1, z + 0 );
                        break;

                    case 1:
                        map_apply_coords( pexit->to_room, x + 1, y + 0, z + 0 );
                        break;

                    case 2:
                        map_apply_coords( pexit->to_room, x + 0, y - 1, z + 0 );
                        break;

                    case 3:
                        map_apply_coords( pexit->to_room, x - 1, y + 0, z + 0 );
                        break;

                    case 4:
                        map_apply_coords( pexit->to_room, x + 0, y + 0, z + 1 );
                        break;

                    case 5:
                        map_apply_coords( pexit->to_room, x + 0, y + 0, z - 1 );
                        break;

                    case 6:
                        map_apply_coords( pexit->to_room, x + 1, y + 1, z + 0 );
                        break;

                    case 7:
                        map_apply_coords( pexit->to_room, x - 1, y + 1, z + 0 );
                        break;

                    case 8:
                        map_apply_coords( pexit->to_room, x + 1, y - 1, z + 0 );
                        break;

                    case 9:
                        map_apply_coords( pexit->to_room, x - 1, y - 1, z + 0 );
                        break;
                }
            }
        }
    }
}

ROOM_INDEX_DATA* room_from_coord( int x, int y, int z )
{
    ROOM_INDEX_DATA* pRoomIndex;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
        {
            if ( xIS_SET( pRoomIndex->room_flags, ROOM_NOCOORD ) )
                continue;

            if ( pRoomIndex->x == x && pRoomIndex->y == y && pRoomIndex->z == z )
                return pRoomIndex;
        }
    }

    return NULL;
}

void deploy_tns( int first, int last, int flag )
{
    char buf[MAX_STRING_LENGTH];
    char desc[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA* room = NULL;
    TNS_DATA* best_tns = NULL;
    TNS_DATA* first_tns = NULL;
    TNS_DATA* last_tns = NULL;
    TNS_DATA* tnsA = NULL;
    TNS_DATA* tnsB = NULL;
    TNS_DATA* tmp = NULL;
    bool fpass = TRUE;
    int vnum = 0;
    int cnt = 0;

    if ( last - first <= 0 )
        return;

    /* Build TNS Map */
    for ( vnum = first; vnum < last; vnum++ )
    {
        if ( ( room = get_room_index( vnum ) ) == NULL )
            continue;

        if ( !xIS_SET( room->room_flags, flag ) )
            continue;

        /* Generate a point - Adds to end of list */
        CREATE( tmp, TNS_DATA, 1 );
        tmp->prev = last_tns;

        if ( last_tns )
            last_tns->next = tmp;

        if ( !first_tns )
            first_tns = tmp;

        last_tns = tmp;
        sprintf( buf, "Adding : %d, %d, %d", room->x, room->y, room->z );
        log_string( buf );
        tmp->x = room->x;
        tmp->y = room->y;
        tmp->z = room->z;
        tmp->done = FALSE;
        tmp->main = TRUE;
        tmp->lock = FALSE;
        tmp->link[0] = NULL;
        tmp->link[1] = NULL;
        tmp->link[2] = NULL;
        tmp->link[3] = NULL;
        tmp->link[4] = NULL;
        tmp->link[5] = NULL;
        tmp->flink = room;
    }

    /* Bubble Sort Pass */
    for ( ; ; )
    {
        TNS_DATA* tmpA, * tmpB;
        bool swap = FALSE;
        int dist, ldist = 0;
        int cnt = 0;

        for ( tnsA = first_tns; tnsA; tnsA = tnsA->next )
        {
            cnt++;
            dist = get_sprox( tnsA->x * 100, tnsA->y * 100, tnsA->z * 100, first_tns->x * 100, first_tns->y * 100, first_tns->z * 100 );

            if ( dist < ldist )
            {
                sprintf( buf, "Swapping %d with %d.", cnt, cnt - 1 );
                log_string( buf );

                if ( tnsA->prev )
                {
                    if ( tnsA->prev->prev )
                        tnsA->prev->prev->next = tnsA;
                }

                if ( tnsA->next )
                    tnsA->next->prev = tnsA->prev;

                tmpA = tnsA->next;
                tmpB = tnsA->prev->prev;
                tnsA->next = tnsA->prev;
                tnsA->prev->next = tmpA;
                tnsA->prev->prev = tnsA;
                tnsA->prev = tmpB;
                swap = TRUE;
            }

            ldist = dist;
        }

        if ( swap == FALSE )
            break;
    }

    for ( tnsA = first_tns; tnsA; tnsA = tnsA->next )
    {
        sprintf( buf, "Sorted : %d, %d, %d", tnsA->x, tnsA->y, tnsA->z );
        log_string( buf );
    }

    /* Create Filler Rooms */
    for ( tnsA = first_tns; tnsA; tnsA = tnsA->next )
    {
        int dist = -1;
        int best = -1;

        if ( tnsA->done )
            continue;

        best_tns = NULL;

        for ( tnsB = first_tns; tnsB; tnsB = tnsB->next )
        {
            if ( tnsB == tnsA )
                continue;

            if ( !fpass && !tnsB->done )
                continue;

            dist = get_sprox( tnsA->x, tnsA->y, tnsA->z, tnsB->x, tnsB->y, tnsB->z );

            if ( dist < best || best < 0 )
            {
                best_tns = tnsB;
                best = dist;
            }
        }

        fpass = FALSE;
        tnsB = best_tns;

        /* Start filling */
        if ( tnsB != NULL )
        {
            TNS_DATA* prev = tnsA;
            int dir, x, y, z;
            bool stay = TRUE;
            x = tnsA->x;
            y = tnsA->y;
            z = tnsA->z;
            sprintf( buf, "Linking : %d, %d, %d -> %d, %d, %d", x, y, z, tnsB->x, tnsB->y, tnsB->z );
            log_string( buf );

            for ( ; stay ; )
            {
                if ( z < tnsB->z && !prev->link[4] )
                {
                    z++;
                    dir = 4;
                }
                else if ( z > tnsB->z && !prev->link[5] )
                {
                    z--;
                    dir = 5;
                }
                else if ( x < tnsB->x && !prev->link[1] )
                {
                    x++;
                    dir = 1;
                }
                else if ( x > tnsB->x && !prev->link[3] )
                {
                    x--;
                    dir = 3;
                }
                else if ( y < tnsB->y && !prev->link[0] )
                {
                    y++;
                    dir = 0;
                }
                else if ( y > tnsB->y && !prev->link[2] )
                {
                    y--;
                    dir = 2;
                }
                else
                {
                    bug( "TNS Fault!" );
                    break;
                }

                if ( x == tnsB->x && y == tnsB->y && z == tnsB->z )
                {
                    prev->link[dir] = tnsB;
                    sprintf( buf, "Finishing : %d, %d, %d (%s)", x, y, z, dir_name[dir] );
                    log_string( buf );

                    if ( dir == 0 )
                        dir = 2;
                    else if ( dir == 1 )
                        dir = 3;
                    else if ( dir == 2 )
                        dir = 0;
                    else if ( dir == 3 )
                        dir = 1;
                    else if ( dir == 4 )
                        dir = 5;
                    else if ( dir == 5 )
                        dir = 4;

                    tnsB->link[dir] = prev;
                    tnsB->done = TRUE;
                    tnsA->done = TRUE;
                    stay = FALSE;
                }
                else
                {
                    sprintf( buf, "Creating : %d, %d, %d (%s)", x, y, z, dir_name[dir] );
                    log_string( buf );
                    tmp = NULL;
                    CREATE( tmp, TNS_DATA, 1 );
                    tmp->prev = last_tns;
                    last_tns->next = tmp;
                    last_tns = tmp;
                    tmp->x = x;
                    tmp->y = y;
                    tmp->z = z;
                    tmp->done = TRUE;
                    tmp->main = FALSE;
                    tmp->lock = FALSE;
                    tmp->link[0] = NULL;
                    tmp->link[1] = NULL;
                    tmp->link[2] = NULL;
                    tmp->link[3] = NULL;
                    tmp->link[4] = NULL;
                    tmp->link[5] = NULL;
                    prev->link[dir] = tmp;

                    if ( dir == 0 )
                        dir = 2;
                    else if ( dir == 1 )
                        dir = 3;
                    else if ( dir == 2 )
                        dir = 0;
                    else if ( dir == 3 )
                        dir = 1;
                    else if ( dir == 4 )
                        dir = 5;
                    else if ( dir == 5 )
                        dir = 4;

                    tmp->link[dir] = prev;
                    prev = tmp;
                }
            }
        }
        else
        {
            sprintf( buf, "Linking Fault : %d, %d, %d", tnsA->x, tnsA->y, tnsA->z );
            log_string( buf );
        }
    }

    cnt = 0;
    room = get_room_index( ROOM_VENT_TEMPLATE );

    if ( room == NULL )
    {
        strcpy( desc, "(Could not locate room description)" );
    }
    else
    {
        strcpy( desc, room->description );
    }

    for ( tnsA = first_tns; tnsA; tnsA = tnsA->next )
    {
        /* Build the room */
        room = make_vent_room( tnsA->x, tnsA->y, tnsA->z );

        if ( room->name )
            STRFREE( room->name );

        if ( room->description )
            STRFREE( room->description );

        room->description = STRALLOC( desc );

        if ( room == NULL )
        {
            cnt = -1;
            break;
        }

        tnsA->vnum = room->vnum;

        // Vent Access Points
        if ( tnsA->main )
        {
            // room->link = room_from_coord( tnsA->x, tnsA->y, tnsA->z );
            room->link = tnsA->flink;

            if ( room->link )
                room->link->link = room;

            sprintf( buf, "&w&zVent above '%s&z'", room->link->name );
            room->name = STRALLOC( buf );
        }
        else
        {
            room->name = STRALLOC( "&w&zInside a Ventilation Shaft" );
        }

        tnsA->lock = TRUE;
    }

    if ( cnt >= 0 )
    {
        cnt = 0;

        /* Link the exits now */
        for ( tnsA = first_tns; tnsA; tnsA = tnsA->next )
        {
            room = get_room_index( tnsA->vnum );

            for ( vnum = 0; vnum <= 5; vnum++ )
            {
                if ( tnsA->link[vnum] != NULL )
                {
                    char buf[MAX_STRING_LENGTH];
                    EXIT_DATA* pexit = NULL;
                    sprintf( buf, "Connecting: %d -> %d (%s)", room->vnum, tnsA->link[vnum]->vnum, dir_name[vnum] );
                    log_string( buf );
                    /* Build a single connecting exit */
                    pexit = make_exit( room, NULL, vnum );
                    pexit->vdir = vnum;
                    pexit->key = 0;
                    pexit->distance = 0;
                    pexit->vnum = tnsA->link[vnum]->vnum;
                    pexit->to_room = get_room_index( tnsA->link[vnum]->vnum );
                    pexit->description    = STRALLOC( "" );
                    pexit->keyword        = STRALLOC( "" );
                    xCLEAR_BITS( pexit->exit_info );
                }
            }

            cnt++;
        }
    }
    else
    {
        cnt = 0;
    }

    bug( "TNS Generated %d rooms in this area.", cnt );

    /*
        Cleanup routine
    */
    for ( tnsA = first_tns; tnsA; tnsA = tnsB )
    {
        tnsB = tnsA->next;

        for ( cnt = 0; cnt <= 5; cnt++ )
            tnsA->link[cnt] = NULL;

        DISPOSE( tnsA );
    }

    return;
}

/*
    Trigger all areas to deploy the TNS vent module
*/
void boot_tns( void )
{
    AREA_DATA* pArea;
    tns_cleanup();

    for ( pArea = first_area; pArea; pArea = pArea->next )
    {
        bug( "Booting TNS : %s", pArea->filename );
        deploy_tns( pArea->low_r_vnum, pArea->hi_r_vnum, ROOM_VENTED_A );
        deploy_tns( pArea->low_r_vnum, pArea->hi_r_vnum, ROOM_VENTED_B );
        deploy_tns( pArea->low_r_vnum, pArea->hi_r_vnum, ROOM_VENTED_C );
        deploy_tns( pArea->low_r_vnum, pArea->hi_r_vnum, ROOM_VENTED_D );
    }

    return;
}

void tns_cleanup( void )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA* room = NULL;
    AREA_DATA* sArea = NULL;
    int vnum, cnt = 0;
    bool aFound;
    // log_string("Room-Recycling Engaged --- Cleaning tns_module.are");
    aFound = FALSE;

    for ( sArea = first_area; sArea; sArea = sArea->next )
        if ( !str_cmp( "tns_module.are", sArea->filename ) )
        {
            aFound = TRUE;
            break;
        }

    if ( !aFound )
    {
        log_string( "Could not locate tns_module.are, skipping." );
        return;
    }

    for ( vnum = sArea->low_r_vnum + 1; vnum <= sArea->hi_r_vnum - 1; vnum++ )
    {
        if ( ( room = get_room_index( vnum ) ) != NULL )
        {
            CHAR_DATA* victim, *vnext;
            OBJ_DATA*  object, *onext;

            /* Attempt a Peusdo-PURGE on the room */
            for ( victim = room->first_person; victim; victim = vnext )
            {
                vnext = victim->next_in_room;

                if ( IS_NPC( victim ) && !xIS_SET( victim->act, ACT_POLYMORPHED ) )
                    extract_char( victim, TRUE, FALSE );
            }

            for ( object = room->first_content; object; object = onext )
            {
                onext = object->next_content;
                extract_obj( object );
            }

            /* RDELETE This room, if its empty */
            if ( !room->first_person && !room->first_content )
            {
                delete_room( room );
                cnt++;
            }
        }
    }

    sprintf( buf, "Killed %d rooms from tns_module.are", cnt );
    log_string( buf );
    /* Save any changes */
    fold_module();
    return;
}

void fold_module( void )
{
    AREA_DATA* area;
    bool aFound = FALSE;

    for ( area = first_area; area; area = area->next )
        if ( !str_cmp( "tns_module.are", area->filename ) )
        {
            aFound = TRUE;
            break;
        }

    if ( !aFound )
    {
        bug ( "----<> WARNING : CANNOT FIND TNS_MODULE.ARE : WARNING <>----" );
        return;
    }

    fold_area( area, area->filename, FALSE );
    return;
}

/*
    All *FASTSHIPS* are created in vships.are
*/
ROOM_INDEX_DATA* make_vent_room( int x, int y, int z )
{
    ROOM_INDEX_DATA* pRoom;
    AREA_DATA* sArea;
    int vnum;
    bool aFound, rFound;
    aFound = FALSE;
    rFound = FALSE;

    for ( sArea = first_area; sArea; sArea = sArea->next )
        if ( !str_cmp( "tns_module.are", sArea->filename ) )
        {
            aFound = TRUE;
            break;
        }

    if ( !aFound )
    {
        bug ( "----<> WARNING : CANNOT FIND TNS_MODULE.ARE : WARNING <>----" );
        return NULL;
    }

    for ( vnum = sArea->low_r_vnum ; vnum <= sArea->hi_r_vnum && !rFound; vnum++ )
    {
        if ( get_room_index( vnum ) == NULL )
        {
            rFound      = TRUE;
            pRoom       = make_room( vnum );
            pRoom->area = sArea;
            pRoom->x    = x;
            pRoom->y    = y;
            pRoom->z    = z;
            xSET_BIT( pRoom->room_flags, ROOM_DARK );
            xSET_BIT( pRoom->room_flags, ROOM_NOHIVE );
            xSET_BIT( pRoom->room_flags, ROOM_INDOORS );
            xSET_BIT( pRoom->room_flags, ROOM_NODROP );
            xREMOVE_BIT( pRoom->room_flags, ROOM_PROTOTYPE );
            xREMOVE_BIT( pRoom->room_flags, ROOM_NOCOORD );
        }
    }

    if ( !rFound )
    {
        bug( "CANNOT LOCATE ROOM-->ABORTING" );
        return NULL;
    }

    return pRoom;
}

