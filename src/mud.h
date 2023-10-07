/***************************************************************************
                            STAR WARS REALITY 1.0
    --------------------------------------------------------------------------
    Star Wars Reality Code Additions and changes from the Smaug Code
    copyright (c) 1997, 1998 by Sean Cooper
    -------------------------------------------------------------------------
    Starwars and Starwars Names copyright (c) Lucasfilm Ltd.
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
                Main mud header file
****************************************************************************/

#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

/* Added for extended bitvectors  -Ghost */
#include <stdarg.h>
#include <stddef.h>

#include <unistd.h>
#include <sys/cdefs.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdio.h>

typedef int             ch_ret;
typedef int             obj_ret;

/*
    Accommodate old non-Ansi compilers.
*/
#if defined(TRADITIONAL)
    #define const
    #define args( list )            ( )
    #define DECLARE_DO_FUN( fun )       void fun( )
    #define DECLARE_SPEC_FUN( fun )     bool fun( )
#else
    #define args( list )            list
    #define DECLARE_DO_FUN( fun )       DO_FUN    fun
    #define DECLARE_SPEC_FUN( fun )     SPEC_FUN  fun
#endif

/*
    Short scalar types.
    Diavolo reports AIX compiler has bugs with short types.
*/

#if !defined(FALSE)
    #define FALSE    0
#endif

#if !defined(TRUE)
    #define TRUE     1
#endif

#if !defined(BERR)
    #define BERR     255
#endif

#if defined(_AIX)
    #if !defined(const)
        #define const
    #endif
    typedef int             sh_int;
    typedef int             bool;
    #define unix
#else
    typedef  short  int      sh_int;
    #include <stdbool.h>
#endif

#define NULLSTR( str )  ( str == NULL || str[0] == '\0' )

/*
    Structure types.
*/
typedef struct  affect_data         AFFECT_DATA;
typedef struct  area_data           AREA_DATA;
typedef struct  var_data            VAR_DATA;
typedef struct  ban_data            BAN_DATA;
typedef struct  extracted_char_data EXTRACT_CHAR_DATA;
typedef struct  char_data           CHAR_DATA;
typedef struct  hunt_hate_fear      HHF_DATA;
typedef struct  descriptor_data     DESCRIPTOR_DATA;
typedef struct  exit_data           EXIT_DATA;
typedef struct  extra_descr_data    EXTRA_DESCR_DATA;
typedef struct  help_data           HELP_DATA;
typedef struct  menu_data           MENU_DATA;
typedef struct  mob_index_data      MOB_INDEX_DATA;
typedef struct  note_data           NOTE_DATA;
typedef struct  comment_data        COMMENT_DATA;
typedef struct  board_data          BOARD_DATA;
typedef struct  obj_data            OBJ_DATA;
typedef struct  obj_index_data      OBJ_INDEX_DATA;
typedef struct  pc_data             PC_DATA;
typedef struct  reset_data          RESET_DATA;
typedef struct  room_index_data     ROOM_INDEX_DATA;
typedef struct  shop_data           SHOP_DATA;
typedef struct  time_info_data      TIME_INFO_DATA;
typedef struct  hour_min_sec        HOUR_MIN_SEC;
typedef struct  weather_data        WEATHER_DATA;
typedef struct  bank_data           BANK_DATA;
typedef struct  clan_data           CLAN_DATA;
typedef struct  tourney_data        TOURNEY_DATA;
typedef struct  mob_prog_data       MPROG_DATA;
typedef struct  mpsleep_data        MPSLEEP_DATA;
typedef struct  mob_prog_act_list   MPROG_ACT_LIST;
typedef struct  editor_data         EDITOR_DATA;
typedef struct  teleport_data       TELEPORT_DATA;
typedef struct  timer_data          TIMER;
typedef struct  godlist_data        GOD_DATA;
typedef struct  system_data         SYSTEM_DATA;
typedef struct  smaug_affect        SMAUG_AFF;
typedef struct  who_data            WHO_DATA;
typedef struct  skill_type          SKILLTYPE;
typedef struct  social_type         SOCIALTYPE;
typedef struct  cmd_type            CMDTYPE;
typedef struct  wizent              WIZENT;
typedef struct  tractor_data        TRACTOR_DATA;
typedef struct  extended_bitvector  EXT_BV;
typedef struct  kill_data           KILL_DATA;
typedef struct  tns_data            TNS_DATA;
typedef struct  arena_data          ARENA_DATA;
typedef struct  bot_data            BOT_DATA;
typedef struct  wp_data             WP_DATA;
typedef struct  sentry_data         SENTRY_DATA;
typedef struct  vote_data           VOTE_DATA;
typedef struct  voter_data          VOTER_DATA;

/*
    Function types.
*/
typedef void    DO_FUN      args( ( CHAR_DATA* ch, char* argument ) );
typedef bool    SPEC_FUN    args( ( CHAR_DATA* ch ) );

#define DUR_CONV    23.333333333333333333333333
#define HIDDEN_TILDE    '*'

#define BV00        (1 <<  0)
#define BV01        (1 <<  1)
#define BV02        (1 <<  2)
#define BV03        (1 <<  3)
#define BV04        (1 <<  4)
#define BV05        (1 <<  5)
#define BV06        (1 <<  6)
#define BV07        (1 <<  7)
#define BV08        (1 <<  8)
#define BV09        (1 <<  9)
#define BV10        (1 << 10)
#define BV11        (1 << 11)
#define BV12        (1 << 12)
#define BV13        (1 << 13)
#define BV14        (1 << 14)
#define BV15        (1 << 15)
#define BV16        (1 << 16)
#define BV17        (1 << 17)
#define BV18        (1 << 18)
#define BV19        (1 << 19)
#define BV20        (1 << 20)
#define BV21        (1 << 21)
#define BV22        (1 << 22)
#define BV23        (1 << 23)
#define BV24        (1 << 24)
#define BV25            (1 << 25)
#define BV26        (1 << 26)
#define BV27        (1 << 27)
#define BV28        (1 << 28)
#define BV29        (1 << 29)
#define BV30        (1 << 30)
#define BV31        (1 << 31)
/* 32 USED! DO NOT ADD MORE! SB */

/*
    String and memory management parameters.
*/
#define MAX_KEY_HASH         2048
#define MAX_STRING_LENGTH    24576  /* buf */
#define MAX_INPUT_LENGTH     2048  /* arg */
#define MAX_INBUF_SIZE       MAX_STRING_LENGTH

#define MSL  MAX_STRING_LENGTH
#define MIL                  MAX_INPUT_LENGTH
#define SUB_MIL              1536
#define SUPER_MIL            4096
#define SUB_MSL              22528
#define SUPER_MSL            26624

#define HASHSTR          /* use string hashing */

#define MAX_LAYERS       8  /* maximum clothing layers */
#define MAX_NEST           100  /* maximum container nesting */

/*
    Game parameters.
    Increase the max'es if you add more of something.
    Adjust the pulse numbers to suit yourself.
*/
#define MAX_EXP_WORTH          500000
#define MIN_EXP_WORTH          25

/*
    Time before getting booted automatically.
*/
#define IDLE_TIMEOUT            2400   /* 10 minutes */

#define MAX_REXITS           20   /* Maximum exits allowed in 1 room */
#define MAX_SKILL           276
#define MAX_RACE              3
#define MAX_NPC_RACE          3
#define MAX_VNUMS    2000000000
#define MAX_LEVEL           105
#define MAX_PLANET          100
#define MAX_GOV             255
#define MAX_COM_CHANNELS   9999  /* How many com channels are there? */
#define MAX_COM_CODES     99999  /* How many encryption codes are there? */
#define MAX_QUOTES           34  /* The current number of quotes */
#define MAX_HERB             20
#define MAX_BANK           7500  /* Maximum number of player bank accounts */
#define MAX_DISEASE          20

#define STAT_BASE            12  /* Base stats in character creation */
#define STAT_FREE            20  /* Points to spend on stats in CC */
#define STAT_MAX             18  /* Basis for the max stat */

#define LEVEL_HERO             (MAX_LEVEL - 5)
#define LEVEL_IMMORTAL         (MAX_LEVEL - 4)
#define LEVEL_SUPREME          MAX_LEVEL
#define LEVEL_INFINITE         (MAX_LEVEL - 1)
#define LEVEL_ETERNAL          (MAX_LEVEL - 1)
#define LEVEL_IMPLEMENTOR      (MAX_LEVEL - 1)
#define LEVEL_SUB_IMPLEM       (MAX_LEVEL - 1)
#define LEVEL_ASCENDANT        (MAX_LEVEL - 2)
#define LEVEL_GREATER          (MAX_LEVEL - 2)
#define LEVEL_GOD              (MAX_LEVEL - 2)
#define LEVEL_LESSER           (MAX_LEVEL - 3)
#define LEVEL_TRUEIMM          (MAX_LEVEL - 3)
#define LEVEL_DEMI             (MAX_LEVEL - 3)
#define LEVEL_SAVIOR           (MAX_LEVEL - 3)
#define LEVEL_CREATOR          (MAX_LEVEL - 3)
#define LEVEL_ACOLYTE          (MAX_LEVEL - 4)
#define LEVEL_NEOPHYTE         (MAX_LEVEL - 4)
#define LEVEL_AVATAR           (MAX_LEVEL - 5)

#define LEVEL_LOG           LEVEL_LESSER
#define LEVEL_HIGOD         LEVEL_GOD
#define LEVEL_NEWBIE                    5

#define PULSE_PER_SECOND          4
#define PULSE_MINUTE              ( 60 * PULSE_PER_SECOND)
#define PULSE_SINGLE              (  1 * PULSE_PER_SECOND)
#define PULSE_VIOLENCE            (  3 * PULSE_PER_SECOND)
#define PULSE_BOT                 (  2 * PULSE_PER_SECOND)
#define PULSE_MOBILE              (  4 * PULSE_PER_SECOND)
#define PULSE_TICK                ( 70 * PULSE_PER_SECOND)
#define PULSE_AREA                ( 60 * PULSE_PER_SECOND)
#define PULSE_SPACE               ( 10 * PULSE_PER_SECOND)
#define PULSE_LOGALERT            ( 5 * PULSE_MINUTE )
#define PULSE_SAVE                ( 15 * PULSE_MINUTE)
#define PULSE_TAXES               ( 60 * PULSE_MINUTE)
#define PULSE_XNAME               ( 120 * PULSE_MINUTE)
#define PULSE_EMERGENCY           ( 45 * PULSE_PER_SECOND )
#define PULSE_BACTA               ( 15 * PULSE_PER_SECOND )

/*
    SWR Version
*/
#define SWR_VERSION_MAJOR "1"
#define SWR_VERSION_MINOR "0"
#define SWR_VERSION_REVISION "6"

/*
    Command logging types.
*/
typedef enum
{
    LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM, LOG_ALL
} log_types;

/*
    Return types for move_char, damage, greet_trigger, etc, etc
    Added by Thoric to get rid of bugs
*/
typedef enum
{
    rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
    rBOTH_QUIT, rOBJ_SCRAPPED, rOBJ_EATEN, rOBJ_EXPIRED,
    rOBJ_TIMER, rOBJ_SACCED, rOBJ_QUAFFED, rOBJ_USED, rOBJ_EXTRACTED,
    rOBJ_DRUNK, rCHAR_IMMUNE, rVICT_IMMUNE, rCHAR_AND_OBJ_EXTRACTED = 128,
    rERROR = 255
} ret_types;

/* Echo types for echo_to_all */
#define ECHOTAR_ALL 0
#define ECHOTAR_PC  1
#define ECHOTAR_IMM 2

/* defines for new do_who */
#define WT_ALIEN 0
#define WT_PREDATOR 1
#define WT_MARINE 2

/*
    do_who output structure -- Narn
*/
struct who_data
{
    WHO_DATA* prev;
    WHO_DATA* next;
    char* text;
    int  type;
};

struct allowmp_data
{
    char  name[1024], host[1024], by[1024];
    struct allowmp_data* next;
};

struct xname_data
{
    char name[MAX_STRING_LENGTH];
    time_t time;
    struct xname_data* next;
};

extern struct xname_data* xnames;

/*
    Site ban structure.
*/
struct  ban_data
{
    BAN_DATA*   next;
    BAN_DATA*   prev;
    char*   name;
    int     level;
    char*   ban_time;
};

/*
    Defines for extended bitvectors
*/
#ifndef INTBITS
    #define INTBITS   32
#endif
#define XBM     31  /* extended bitmask   ( INTBITS - 1 )   */
#define RSV     5   /* right-shift value  ( sqrt(XBM+1) )   */
#define XBI     4   /* integers in an extended bitvector    */
#define MAX_BITS    XBI * INTBITS

/*
    Structure for extended bitvectors -- Thoric
*/
struct extended_bitvector
{
    int     bits[XBI];
};

#include "dns.h"

/*
    Time and weather stuff.
*/
typedef enum
{
    SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET
} sun_positions;

typedef enum
{
    SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
} sky_conditions;

struct  time_info_data
{
    int     hour;
    int     day;
    int     month;
    int     year;
};

/*
    Random quote system    -Ghost
*/
struct quote_type
{
    char*  text;
    char*  by;
};

struct  arena_data
{
    ARENA_DATA*        next;
    ARENA_DATA*        prev;
    char*              name;        /* Arena Name */
    char*              desc;        /* Arena Desc */
    char*              filename;    /* Arena Filename */
    int              firstroom;
    int              lastroom;
    int              kills[3];      /* PKs by race */
    int              support[3];    /* Support by race */
    int              maxsupport[3]; /* Max Support by race */
    int              timer;         /* Arena lifespan */
    int              ctimer;        /* Current time remaining */
    int              hive;          /* Hive Progress */
    int              max_hive;      /* Max Hive Progress */
    int              min_p;         /* Minimum number of players */
    int              max_p;         /* Maximum number of players */
    int              id;            /* Auto-ID System */
};

/*
    TNS (Tunnel Networking System)
     -Ghost
*/
struct tns_data
{
    TNS_DATA* next;
    TNS_DATA* prev;
    TNS_DATA* link[6];
    ROOM_INDEX_DATA* flink;
    bool       done;
    bool       main;
    bool       lock;
    int        x, y, z;
    int        vnum;
};

struct hour_min_sec
{
    int hour;
    int min;
    int sec;
    int manual;
};

struct  weather_data
{
    int     mmhg;
    int     change;
    int     sky;
    int     sunlight;
};

/*
    Kill Data Tracking
*/
struct  kill_data
{
    int  pc[3];
    int  bot[3];
    int  npc[3];
};

/*
    Structure used to build wizlist
*/
struct  wizent
{
    WIZENT*         next;
    WIZENT*         last;
    char*       name;
    sh_int      level;
};


/*
    Connected state for a channel.
*/
typedef enum
{
    CON_PLAYING,          CON_GET_NAME,         CON_GET_OLD_PASSWORD,
    CON_CONFIRM_NEW_NAME, CON_GET_NEW_PASSWORD, CON_CONFIRM_NEW_PASSWORD,
    CON_GET_NEW_SEX,      CON_READ_MOTD,        CON_GET_FULL_NAME,
    CON_GET_NEW_RACE,     CON_GET_EMULATION,    CON_EDITING,
    CON_GET_WANT_RIPANSI, CON_TITLE,            CON_PRESS_ENTER,
    CON_WAIT_1,           CON_WAIT_2,           CON_WAIT_3,
    CON_ACCEPTED,         CON_READ_IMOTD,       CON_GET_NEW_EMAIL,
    CON_GET_MSP,          CON_COPYOVER_RECOVER, CON_IMM_INVIS,
    CON_IMM_LOADA,        CON_DISCLAIMER,       CON_GET_CENSOR,
    CON_GET_FULL_NAME_OK, CON_ENTER_GAME,       CON_MENU_BASE,
    CON_MENU_ENTER,       CON_MENU_NEWPASS1,    CON_MENU_NEWPASS2,
    CON_MENU_NEWPASS3,    CON_MENU_DELETE,      CON_MENU_LEVEL,
    CON_MENU_SKILL1,      CON_MENU_SKILL2,      CON_MENU_STAT1,
    CON_MENU_STAT2,       CON_GET_EMAIL
} connection_types;

/*
    Character substates
*/
typedef enum
{
    SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_OBJ_SHORT, SUB_OBJ_LONG,
    SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_ROOM_DESC, SUB_ROOM_EXTRA,
    SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT, SUB_HELP_EDIT,
    SUB_WRITING_MAP, SUB_PERSONAL_BIO, SUB_REPEATCMD, SUB_RESTRICTED,
    SUB_DEITYDESC, SUB_ROOM_DESC2, SUB_ARENA_DESC,
    /* timer types ONLY below this point */
    SUB_TIMER_DO_ABORT = 128, SUB_TIMER_CANT_ABORT
} char_substates;

/*
    Descriptor (channel) structure.
*/
struct  descriptor_data
{
    DESCRIPTOR_DATA*    next;
    DESCRIPTOR_DATA*    prev;
    DESCRIPTOR_DATA*    snoop_by[6];
    CHAR_DATA*      character;
    CHAR_DATA*      original;
    char*       host;
    char*               hostip;
    int         port;
    int         descriptor;
    sh_int      connected;
    sh_int      idle;
    sh_int      lines;
    sh_int      scrlen;
    bool        fcommand;
    char        inbuf       [MAX_INBUF_SIZE];
    char        incomm      [MAX_INPUT_LENGTH];
    char        inlast      [MAX_INPUT_LENGTH];
    int         repeat;
    char*       outbuf;
    unsigned long   outsize;
    int         outtop;
    char*       pagebuf;
    unsigned long   pagesize;
    int         pagetop;
    char*       pagepoint;
    char        pagecmd;
    char        pagecolor;
    int         auth_inc;
    int         auth_state;
    char        abuf[ 256 ];
    int         auth_fd;
    char*       user;
    int         atimes;
    int         newstate;
    unsigned char   prevcolor;
    int                 ifd;
    pid_t               ipid;
};

/* Race charts */
#define RACE_MARINE             0
#define RACE_ALIEN              1
#define RACE_PREDATOR           2

/* Languages (Modifyed for Extended Bitvectors) */
typedef enum
{
    LANG_MARINE, LANG_ALIEN, LANG_PREDATOR, LANG_UNKNOWN, MAX_LANG_TYPE
} languages_types;

/*
    TO types for act.
*/
#define TO_ROOM         0
#define TO_NOTVICT      1
#define TO_VICT         2
#define TO_CHAR         3

/*
    Real action "TYPES" for act.
*/
#define AT_BLACK        0
#define AT_BLOOD        1
#define AT_DGREEN           2
#define AT_ORANGE       3
#define AT_DBLUE        4
#define AT_PURPLE       5
#define AT_CYAN         6
#define AT_GREY         7
#define AT_DGREY        8
#define AT_RED          9
#define AT_GREEN       10
#define AT_YELLOW      11
#define AT_BLUE        12
#define AT_PINK        13
#define AT_LBLUE       14
#define AT_WHITE       15
#define AT_BLINK       16
#define AT_PLAIN       AT_GREY
#define AT_ACTION      AT_GREY
#define AT_SAY         AT_LBLUE
#define AT_GOSSIP      AT_LBLUE
#define AT_YELL            AT_WHITE
#define AT_TELL        AT_BLUE
#define AT_HIT         AT_WHITE
#define AT_HITME       AT_YELLOW
#define AT_OOC             AT_YELLOW
#define AT_IMMORT      AT_WHITE
#define AT_AVATAR      AT_GREEN
#define AT_HURT        AT_RED
#define AT_FALLING     AT_WHITE + AT_BLINK
#define AT_DANGER      AT_RED + AT_BLINK
#define AT_MAGIC       AT_BLUE
#define AT_CONSIDER    AT_GREY
#define AT_REPORT      AT_GREY
#define AT_POISON      AT_GREEN
#define AT_SOCIAL      AT_CYAN
#define AT_DYING       AT_YELLOW
#define AT_DEAD        AT_RED
#define AT_SKILL       AT_GREEN
#define AT_CARNAGE     AT_BLOOD
#define AT_DAMAGE      AT_WHITE
#define AT_FLEE        AT_YELLOW
#define AT_RMNAME      AT_WHITE
#define AT_RMDESC          AT_YELLOW
#define AT_OBJECT      AT_GREEN
#define AT_PERSON      AT_PINK
#define AT_LIST        AT_BLUE
#define AT_BYE         AT_GREEN
#define AT_GTELL       AT_BLUE
#define AT_NOTE        AT_GREEN
#define AT_HUNGRY      AT_ORANGE
#define AT_THIRSTY     AT_BLUE
#define AT_FIRE        AT_RED
#define AT_SOBER       AT_WHITE
#define AT_WEAROFF     AT_YELLOW
#define AT_EXITS       AT_WHITE
#define AT_SCORE           AT_DGREEN
#define AT_RESET       AT_DGREEN
#define AT_LOG         AT_PURPLE
#define AT_DIEMSG      AT_WHITE
#define AT_WARTALK         AT_RED
#define AT_SHIP            AT_PINK
#define AT_CLAN            AT_PINK
#define AT_INFO            AT_RED
#define AT_WHISPER         AT_LBLUE

#define MAX_ITEM_IMPACT      30

/*
    Help table types.
*/
struct  help_data
{
    HELP_DATA*  next;
    HELP_DATA* prev;
    sh_int  level;
    char*   keyword;
    char*   text;
};

/*
    Shop types.
*/
#define MAX_TRADE    5

struct  shop_data
{
    SHOP_DATA*  next;           /* Next shop in list        */
    SHOP_DATA* prev;            /* Previous shop in list    */
    int     keeper;         /* Vnum of shop keeper mob  */
};

/*
    Mob program structures and defines
*/
#define MAX_IFS 20
#define IN_IF 0
#define IN_ELSE 1
#define DO_IF 2
#define DO_ELSE 3

#define MAX_PROG_NEST 20

struct  act_prog_data
{
    struct act_prog_data* next;
    void* vo;
};

struct  mob_prog_act_list
{
    MPROG_ACT_LIST* next;
    char*        buf;
    CHAR_DATA*       ch;
    OBJ_DATA*        obj;
    void*        vo;
};

struct  mob_prog_data
{
    MPROG_DATA* next;
    // int          type;
    int          type;
    bool     triggered;
    int      resetdelay;
    char*    arglist;
    char*    comlist;
};

/* Used to store sleeping mobprogs */
typedef enum {MP_MOB, MP_ROOM, MP_OBJ} mp_types;

struct mpsleep_data
{
    MPSLEEP_DATA* next;
    MPSLEEP_DATA* prev;
    int timer;
    mp_types type;
    ROOM_INDEX_DATA* room;
    int ignorelevel;
    int iflevel;
    bool ifstate[MAX_IFS][DO_ELSE];
    char* com_list;
    CHAR_DATA* mob;
    CHAR_DATA* actor;
    OBJ_DATA* obj;
    void* vo;
    bool single_step;
};

extern bool MOBtrigger;

/* Race Dedicated stuff */
struct  race_type
{
    char    race_name   [16];   /* Race name            */
    char        real_name       [16];   /* Real name                    */
    sh_int  str_plus;       /* Str bonus/penalty        */
    sh_int      sta_plus;               /* Sta      "                   */
    sh_int      int_plus;               /* Int      "                   */
    sh_int      rec_plus;               /* Rec      "                   */
    sh_int      bra_plus;               /* Bra      "                   */
    sh_int      per_plus;               /* Per      "                   */
    sh_int      hit;
    sh_int      move;
    int         language;               /* Default racial language      */
    char*       desc;                   /* Racial Description Pointer   */
};

struct tourney_data
{
    int    open;
    int    low_level;
    int    hi_level;
};

struct vote_data
{
    CHAR_DATA*   post;        /* Who posted the vote */
    int          timer;       /* Time left on the vote */
    int          mode;        /* What are we voting for */
    char         data[MSL];   /* Related vote data */
    int          yea;
    int          nay;
    int          votes;
    VOTER_DATA* last_vote;
    VOTER_DATA* first_vote;
};

struct voter_data
{
    VOTER_DATA* next;
    VOTER_DATA* prev;
    CHAR_DATA*   ch;
};

/*
    Data structure for notes.
*/
struct  note_data
{
    NOTE_DATA*  next;
    NOTE_DATA* prev;
    char*   sender;
    char*   date;
    char*   to_list;
    char*   subject;
    int         expire;
    int         voting;
    char*   yesvotes;
    char*   novotes;
    char*   abstentions;
    char*   text;
};

struct  board_data
{
    BOARD_DATA* next;           /* Next board in list          */
    BOARD_DATA* prev;           /* Previous board in list      */
    NOTE_DATA*   first_note;        /* First note on board         */
    NOTE_DATA*   last_note;     /* Last note on board          */
    char*    note_file;     /* Filename to save notes to       */
    char*    read_group;        /* Can restrict a board to a       */
    char*    post_group;        /* council, clan, guild etc        */
    char*    extra_readers;     /* Can give read rights to players */
    char*        extra_removers;        /* Can give remove rights to players */
    int      board_obj;     /* Vnum of board object        */
    sh_int   num_posts;     /* Number of notes on this board   */
    sh_int   min_read_level;    /* Minimum level to read a note    */
    sh_int   min_post_level;    /* Minimum level to post a note    */
    sh_int   min_remove_level;  /* Minimum level to remove a note  */
    sh_int   max_posts;     /* Maximum amount of notes allowed */
    int          type;                  /* Normal board or mail board?     */
};


/*
    An affect.
*/
struct  affect_data
{
    AFFECT_DATA*    next;
    AFFECT_DATA*    prev;
    sh_int      type;
    sh_int      duration;
    sh_int      location;
    int         modifier;
    EXT_BV      bitvector;
};


/*
    A SMAUG spell
*/
struct  smaug_affect
{
    SMAUG_AFF*      next;
    char*       duration;
    sh_int      location;
    char*       modifier;
    int         bitvector;
};


/***************************************************************************
 *                                                                         *
                     VALUES OF INTEREST TO AREA BUILDERS
                     (Start of section ... start here)
 *                                                                         *
 ***************************************************************************/

/*
    Well known mob virtual numbers.
    Defined in #MOBILES.
*/
#define MOB_VNUM_BOT            80
#define MOB_VNUM_TEMPLATE       97

/*
    ACT bits for mobs.
    Used in #MOBILES.
    Revamped For EXTENDED BITVECTORS by Ghost
*/
typedef enum
{
    ACT_IS_NPC, ACT_SENTINEL, ACT_SCAVENGER, ACT_AGGRESSIVE,
    ACT_STAY_AREA, ACT_PET, ACT_TRAIN, ACT_PRACTICE,
    ACT_IMMORTAL, ACT_DEADLY, ACT_POLYSELF, ACT_META_AGGR,
    ACT_GUARDIAN, ACT_RUNNING, ACT_NOWANDER, ACT_MOUNTABLE,
    ACT_MOUNTED, ACT_SCHOLAR, ACT_SECRETIVE, ACT_POLYMORPHED,
    ACT_MOBINVIS, ACT_NOASSIST, ACT_NOKILL, ACT_DROID, ACT_NOCORPSE,
    ACT_SURGEON, ACT_PROTOTYPE, ACT_ENLIST, ACT_HOSTAGE, MAX_ACT_FLAGS
} npc_act_types;

/*
    MUDH Quick defines    - Ghost
*/
#define MUDH_OOC_LIMIT    6        /* In SAVE.C                             */

/*
    Number of PC Aliens required (minus yourself) to create a swarm.
*/
#define SWARM_CNT   0
#define MAX_SWARM   3

/*
    Bits for 'affected_by'.
    REWRITTEN BY GHOST FOR USE WITH EXTENDED BITVECTORS
    Used in #MOBILES.
*/
typedef enum
{
    AFF_BLIND, AFF_INVISIBLE, AFF_DETECT_EVIL, AFF_DETECT_INVIS,
    AFF_DETECT_MAGIC, AFF_DETECT_HIDDEN, AFF_WEAKEN, AFF_SANCTUARY,
    AFF_FAERIE_FIRE, AFF_INFRARED, AFF_CURSE, AFF_FLAMING, AFF_POISON,
    AFF_PROTECT, AFF_PARALYSIS, AFF_SNEAK, AFF_HIDE, AFF_SLEEP, AFF_CHARM,
    AFF_FLYING, AFF_PASS_DOOR, AFF_FLOATING, AFF_TRUESIGHT, AFF_DETECTTRAPS,
    AFF_SCRYING, AFF_FIRESHIELD, AFF_SHOCKSHIELD, AFF_HAUS1, AFF_SHORTAGE,
    AFF_POSSESS, AFF_AQUA_BREATH, AFF_NAPALM, AFF_CLOAK, MAX_AFFECTED_BY
} affected_by_types;

/*
    Resistant Immune Susceptible flags
*/
typedef enum
{
    RIS_FIRE, RIS_ENERGY, RIS_IMPACT, RIS_PIERCE, RIS_ACID, MAX_RIS_FLAGS
} resistant_types;

/*
    Autosave flags
*/
#define SV_DEATH                  BV00
#define SV_KILL                   BV01
#define SV_PASSCHG                BV02
#define SV_DROP                   BV03
#define SV_PUT                    BV04
#define SV_GIVE                   BV05
#define SV_AUTO                   BV06
#define SV_ZAPDROP                BV07
#define SV_GET                    BV09
#define SV_RECEIVE                BV10
#define SV_IDLE                   BV11
#define SV_BACKUP                 BV12

/*
    Sex.
    Used in #MOBILES.
*/
typedef enum { SEX_NEUTRAL, SEX_MALE, SEX_FEMALE } sex_types;

/*
    Well known object virtual numbers.
    Defined in #OBJECTS.
*/
#define OBJ_VNUM_CORPSE_NPC      10
#define OBJ_VNUM_CORPSE_PC       11
#define OBJ_VNUM_BLOODSTAIN      18
#define OBJ_VNUM_SCRAPS          19
#define OBJ_VNUM_FIRE            30
#define OBJ_VNUM_NOTE            36
#define OBJ_VNUM_BLANK               40   /* A Blank, default object */

#define OBJ_VNUM_PULSE_RIFLE         80
#define OBJ_VNUM_M4A3_PISTOL         82

#define OBJ_VNUM_PULSE_AMMO          81
#define OBJ_VNUM_M4A3_AMMO           83

#define OBJ_VNUM_CLOAKING_DEVICE     96
#define OBJ_VNUM_ENERGY_GRENADE      97

#define OBJ_VNUM_MARINE_GRENADE      150
#define OBJ_VNUM_MARINE_RADIO        682

#define OBJ_VNUM_DATAPAD          111
#define OBJ_VNUM_FORCEPIKE        624

/* Academy eq */
#define OBJ_VNUM_SCHOOL_MACE      10315
#define OBJ_VNUM_SCHOOL_DAGGER    10312
#define OBJ_VNUM_SCHOOL_SWORD     10313
#define OBJ_VNUM_SCHOOL_VEST      10308
#define OBJ_VNUM_SCHOOL_SHIELD    10310
#define OBJ_VNUM_SCHOOL_BANNER    10311
#define OBJ_VNUM_SCHOOL_DIPLOMA   10321

#define OBJ_VNUM_DIAMOND_RING     32600
#define OBJ_VNUM_WEDDING_BAND     32601

#define OBJ_VNUM_EJECTION_SEAT    35
#define OBJ_VNUM_ESCAPE_POD       35 /* For now*/
#define OBJ_VNUM_MICROPHONE       0
#define OBJ_VNUM_CAMERA           0
#define OBJ_VNUM_MONITOR          0
#define OBJ_VNUM_BEACON           0
#define OBJ_VNUM_TRACKER          0

/*
    Item types.
    Used in #OBJECTS.
*/
typedef enum
{
    ITEM_NONE, ITEM_LIGHT, ITEM_WEAPON, ITEM_ARMOR,
    ITEM_FURNITURE, ITEM_TRASH, ITEM_CONTAINER, ITEM_PAPER,
    ITEM_DRINK_CON, ITEM_KEY, ITEM_FOOD, ITEM_CORPSE_NPC,
    ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_BLOODSTAIN, ITEM_SCRAPS,
    ITEM_FIRE, ITEM_AMMO, ITEM_GRENADE, ITEM_RADIO, ITEM_MEDIKIT,
    ITEM_MOTION, ITEM_CLOAK, ITEM_BINOCULAR, ITEM_GPS, ITEM_COVER,
    ITEM_BANDAGE, ITEM_C4, ITEM_LAPTOP, ITEM_TOOLKIT, ITEM_STERIL,
    ITEM_MEDICOMP, ITEM_SPAWNER, ITEM_LANDMINE, ITEM_ATTACHMENT,
    ITEM_MEDSTATION, ITEM_MOTIONB, ITEM_AMMOBOX, ITEM_TOOLCHEST,
    ITEM_MAPCONSOLE, ITEM_TRAP, ITEM_MSPAWNER, ITEM_DEPLOYER,
    ITEM_REMOTE, ITEM_SIFT, ITEM_FLARE, ITEM_REGENERATOR,
    ITEM_SENTRY, ITEM_SCANNON, ITEM_NVGOGGLE
} item_types;


#define MAX_ITEM_TYPE            ITEM_NVGOGGLE

/*
    Extra flags.
    Used in #OBJECTS.
*/
typedef enum
{
    ITEM_GLOW, ITEM_HUM, ITEM_INVIS, ITEM_NODROP, ITEM_NOREMOVE,
    ITEM_INVENTORY, ITEM_HIDDEN, ITEM_COVERING, ITEM_DEATHROT,
    ITEM_BURRIED, ITEM_PROTOTYPE, ITEM_NODUAL, ITEM_MARINE,
    ITEM_PREDATOR, ITEM_ALIEN, ITEM_RESPAWN, ITEM_BURSTFIRE,
    ITEM_AUTOFIRE, ITEM_SEMIAUTO, ITEM_SINGLEFIRE, ITEM_USEON,
    ITEM_USEOFF, ITEM_QUIET, ITEM_MODERATE, ITEM_LOUD, ITEM_RANGEDONLY,
    ITEM_INDESTRUCTABLE, ITEM_NOHOLD, ITEM_ALIENINVIS, ITEM_NOTACKLE,
    ITEM_FULLHIT, MAX_ITEM_FLAG
} item_extra_flags;

/* Weapon settings */
#define MODE_SINGLE           0
#define MODE_BURST            1
#define MODE_AUTOMATIC        2
#define MODE_SEMIAUTO         3

/* Weapon Types */
#define WEAPON_NONE           0
#define WEAPON_KNIFE          1   /* Marine */
#define WEAPON_PISTOL         2   /* Marine */
#define WEAPON_RIFLE          3   /* Marine */
#define WEAPON_SHOTGUN        4   /* Marine */
#define WEAPON_AUTOMATIC      5   /* Marine - Minigun */
#define WEAPON_FLAMETHROWER   6   /* Marine */
#define WEAPON_ROCKETFIRED    7   /* Marine - Grenade Launcher + Sadar */
#define WEAPON_BLADE          10  /* Predator */
#define WEAPON_SPEAR          11  /* Predator */
#define WEAPON_ERANGED        12  /* Predator - Ranged Energy Weapons */
#define WEAPON_RANGED         13  /* Predator - Dart Gun + Spear gun */
#define WEAPON_DISC           14  /* Predator - Smart Disc */
#define WEAPON_NATURAL        20  /* Alien - Melee */

#define MAX_WEAPON_TYPES      25

#define TELE_SHOWDESC       BV00
#define TELE_TRANSALL       BV01
#define TELE_TRANSALLPLUS   BV02

/*
    Wear flags.
    Used in #OBJECTS.
*/
typedef enum
{
    ITEM_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_NECK, ITEM_WEAR_BODY,
    ITEM_WEAR_HEAD, ITEM_WEAR_LEGS, ITEM_WEAR_FEET, ITEM_WEAR_HANDS,
    ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD, ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST,
    ITEM_WEAR_WRIST, ITEM_WIELD, ITEM_HOLD, ITEM_DUAL_WIELD, ITEM_WEAR_EARS,
    ITEM_WEAR_EYES, ITEM_MISSILE_WIELD, ITEM_WEAR_OVER, ITEM_WEAR_SHOULDER,
    MAX_WEAR_FLAGS
} wear_types;

/*
    Apply types (for affects).
    Used in #OBJECTS.
*/
typedef enum
{
    APPLY_NONE, APPLY_STR, APPLY_STA, APPLY_REC, APPLY_INT, APPLY_BRA, APPLY_PER,
    APPLY_NULL, APPLY_HIT, APPLY_MOVE, APPLY_AFFECT,
    APPLY_FIRE, APPLY_ENERGY, APPLY_IMPACT, APPLY_PIERCE, APPLY_ACID, MAX_APPLY_TYPE
} apply_types;

/*
    Values for containers (value[1]).
    Used in #OBJECTS.
*/
#define CONT_CLOSEABLE            1
#define CONT_PICKPROOF            2
#define CONT_CLOSED           4
#define CONT_LOCKED           8

/*
    Well known room virtual numbers.
    Defined in #ROOMS.
*/
#define ROOM_VNUM_LIMBO           2
#define ROOM_VNUM_POLY            3
#define ROOM_VNUM_CHAT        32144
#define ROOM_VNUM_TEMPLE          32500
#define ROOM_VNUM_ALTAR       32144
#define ROOM_VNUM_SCHOOL          69900
#define ROOM_AUTH_START       10300
#define ROOM_START_IMMORTAL         100
#define ROOM_LIMBO_SHIPYARD          45
#define ROOM_BLAST_LIMBO             46
#define ROOM_VENT_TEMPLATE           50
#define ROOM_DEFAULT_CRASH        28025

#define ARENA_RECOVER                 0      /* Arena Recovery Room */

/*
    Room flags.
    Used in #ROOMS.

    R1 is RESERVED! *LEAVE IT ALONE!* Used in Track.c
    CP stands for Control Point.
*/
typedef enum
{
    ROOM_DARK, ROOM_R1, ROOM_NO_MOB, ROOM_INDOORS, ROOM_CAN_LAND,
    ROOM_CAN_FLY, ROOM_CAN_DRIVE, ROOM_SHIPYARD, ROOM_NODROPALL,
    ROOM_LOGSPEECH, ROOM_TELEPORT, ROOM_HOTEL, ROOM_NOFLOOR, ROOM_NODROP,
    ROOM_VIEWPORT, ROOM_SPACECRAFT, ROOM_PROTOTYPE, ROOM_UNDERGROUND,
    ROOM_SAVEEQ, ROOM_BACTA, ROOM_FALLCATCH, ROOM_LIT, ROOM_VENTED_A,
    ROOM_HIVED, ROOM_MAPSTART, ROOM_NOHIVE, ROOM_SAFE, ROOM_DEPLOY_P,
    ROOM_DEPLOY_M, ROOM_DEPLOY_A, ROOM_NOCOORD, ROOM_VENTED_B,
    ROOM_VENTED_C, ROOM_VENTED_D, ROOM_DEPLOY_FROM, ROOM_RESCUE,
    ROOM_CP, ROOM_STATIC, ROOM_NOHIDE, MAX_ROOM_FLAGS
} room_flag_types;

/*
    Directions.
    Used in #ROOMS.
*/
typedef enum
{
    DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
    DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_SOMEWHERE
} dir_types;

#define MAX_DIR         DIR_SOUTHWEST   /* max for normal walking */
#define DIR_PORTAL      DIR_SOMEWHERE   /* portal direction   */


/*
    Exit flags.
    Used in #ROOMS.
*/
typedef enum
{
    EX_ISDOOR, EX_CLOSED, EX_LOCKED, EX_SECRET, EX_SWIM, EX_PICKPROOF,
    EX_FLY, EX_CLIMB, EX_DIG, EX_RES1, EX_NOPASSDOOR, EX_HIDDEN, EX_PASSAGE,
    EX_PORTAL, EX_RES2, EX_RES3, EX_xCLIMB, EX_xENTER, EX_xLEAVE, EX_xAUTO,
    EX_RES4, EX_xSEARCHABLE, EX_BASHED, EX_BASHPROOF, EX_NOMOB, EX_WINDOW,
    EX_xLOOK, EX_KEYPAD, EX_BARRED, EX_NOSPEC, EX_BLASTOPEN, EX_BLASTED,
    EX_ARMORED, EX_NOVENT, MAX_EXFLAG
} exit_types;

/*
    Sector types.
    Used in #ROOMS.
*/
typedef enum
{
    SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
    SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
    SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_MAX
} sector_types;

/*
    Equpiment wear locations.
    Used in #RESETS.
*/
typedef enum
{
    WEAR_NONE = -1, WEAR_LIGHT = 0, WEAR_FINGER_L, WEAR_FINGER_R, WEAR_NECK_1,
    WEAR_NECK_2, WEAR_BODY, WEAR_HEAD, WEAR_LEGS, WEAR_FEET, WEAR_HANDS,
    WEAR_ARMS, WEAR_SHIELD, WEAR_ABOUT, WEAR_WAIST, WEAR_WRIST_L, WEAR_WRIST_R,
    WEAR_WIELD, WEAR_HOLD, WEAR_DUAL_WIELD, WEAR_EARS, WEAR_EYES,
    WEAR_MISSILE_WIELD, WEAR_OVER, WEAR_BACK, WEAR_SHOULDER, MAX_WEAR
} wear_locations;

/* Board Types */
typedef enum { BOARD_NOTE, BOARD_MAIL } board_types;

/* Auth Flags */
#define FLAG_WRAUTH           1
#define FLAG_AUTH             2

/***************************************************************************
 *                                                                         *
                     VALUES OF INTEREST TO AREA BUILDERS
                     (End of this section ... stop here)
 *                                                                         *
 ***************************************************************************/

/*
    Positions.
*/
typedef enum
{
    POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_PRONE, POS_KNEELING,
    POS_SITTING, POS_STANDING, POS_MOUNTED, POS_SHOVE, POS_DRAG
} positions;

/*
    ACT bits for players.
*/
typedef enum
{
    PLR_IS_NPC, PLR_BOUGHT_DROID, PLR_SHOVEDRAG, PLR_AUTOEXIT,
    PLR_AUTOLOOT, PLR_AUTOSAC, PLR_BLANK, PLR_OUTCAST, PLR_BRIEF,
    PLR_COMBINE, PLR_PROMPT, PLR_TELNET_GA, PLR_HOLYLIGHT,
    PLR_WIZINVIS, PLR_ROOMVNUM, PLR_SILENCE, PLR_NO_EMOTE,
    PLR_ATTACKER, PLR_NO_TELL, PLR_LOG, PLR_DENY, PLR_FREEZE,
    PLR_KILLER, PLR_QUESTOR, PLR_LITTERBUG, PLR_ANSI, PLR_SOUND,
    PLR_NICE, PLR_FLEE, PLR_AUTOMAP, PLR_AFK, PLR_NCOUNCIL,
    PLR_CENSOR, PLR_WHOIS, PLR_COMPACT, PLR_BUILDWALK, MAX_PLR_FLAGS
} plr_types;

/* Bits for pc_data->flags. */
typedef enum
{
    PCFLAG_R1, PCFLAG_xx, PCFLAG_UNAUTHED, PCFLAG_NORECALL,
    PCFLAG_NOINTRO, PCFLAG_GAG, PCFLAG_RETIRED, PCFLAG_GUEST,
    PCFLAG_NOSUMMON, PCFLAG_PAGERON, PCFLAG_NOTITLE, PCFLAG_ROOM,
    PCFLAG_HELPSTART, PCFLAG_DND, PCFLAG_ITRACE, PCFLAG_NOWHO,
    PCFLAG_SHOWRESET, PCFLAG_SHOWAMMO, MAX_PC_FLAGS
} pcflag_type;

typedef enum
{
    TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN,
    TIMER_APPLIED, TIMER_PKILLED
} timer_types;

struct timer_data
{
    TIMER*      prev;
    TIMER*      next;
    DO_FUN*     do_fun;
    int     value;
    sh_int  type;
    sh_int  count;
};


/*
    Channel bits.
*/
typedef enum
{
    CHANNEL_CHAT, CHANNEL_QUEST, CHANNEL_IMMTALK,
    CHANNEL_MUSIC, CHANNEL_ASK, CHANNEL_SHOUT, CHANNEL_YELL,
    CHANNEL_MONITOR, CHANNEL_LOG, CHANNEL_104,
    CHANNEL_BUILD, CHANNEL_105, CHANNEL_AVTALK, CHANNEL_PRAY,
    CHANNEL_COUNCIL, CHANNEL_C17, CHANNEL_COMM, CHANNEL_TELLS,
    CHANNEL_C20, CHANNEL_NEWBIE, CHANNEL_WARTALK, CHANNEL_OOC,
    CANNNEL_C24, CHANNEL_C25, CHANNEL_C26, CHANNEL_WHISPER,
    CHANNEL_INFO, MAX_CHANNEL
} channel_type;

/*  Area defines - Scryn 8/11

*/
#define AREA_DELETED   BV00
#define AREA_LOADED    BV01

#define AREA_VERSION   4       /* Current file version */

typedef enum
{
    AFLAG_NOPKILL, AFLAG_NORESET, MAX_AREA_FLAG
} aflag_type;

/*
    Prototype for a mob.
    This is the in-memory version of #MOBILES.
*/
struct  mob_index_data
{
    MOB_INDEX_DATA*     next;
    MOB_INDEX_DATA*     next_sort;
    SPEC_FUN*       spec_fun;
    SPEC_FUN*           spec_2;
    SPEC_FUN*           spec_3;    /*  Added by Ghost  */
    SPEC_FUN*           spec_4;    /*  Added by Ghost  */
    SHOP_DATA*      pShop;
    MPROG_DATA*     mudprogs;
    // int                 progtypes;
    EXT_BV              progtypes;
    char*       player_name;
    char*       short_descr;
    char*       long_descr;
    char*       description;
    int         vnum;
    sh_int      count;
    sh_int      killed;
    sh_int      sex;
    sh_int      level;
    EXT_BV      act;
    int                 currexp;       /* New XP System */
    int                 maxexp;        /* New XP System */
    int         exp;
    EXT_BV              speaks;
    EXT_BV              speaking;
    EXT_BV              affected_by;
    sh_int      position;
    sh_int      defposition;
    sh_int      height;
    sh_int      weight;
    sh_int      race;
    sh_int      damroll;
    sh_int      perm_str;
    sh_int              perm_sta;
    sh_int              perm_int;
    sh_int              perm_rec;
    sh_int              perm_bra;
    sh_int              perm_per;
};


struct hunt_hate_fear
{
    char*       name;
    CHAR_DATA*      who;
};

struct  editor_data
{
    sh_int      numlines;
    sh_int      on_line;
    sh_int      size;
    char        line[49][81];
};

struct  extracted_char_data
{
    EXTRACT_CHAR_DATA*  next;
    CHAR_DATA*      ch;
    ROOM_INDEX_DATA*    room;
    ch_ret      retcode;
    bool        extract;
};

struct  bot_data
{
    BOT_DATA*           next;
    BOT_DATA*           prev;
    CHAR_DATA*            ch;  /* Pointer to the bot's body */
    ROOM_INDEX_DATA* target;   /* Destination room */
    char*               name;  /* Name of the bot */
    char*           filename;  /* File location of bot's pfile */
    bool              loaded;  /* Loaded up and running */
    bool               arena;  /* In arena? */
    int              respawn;  /* Obvious */
    int                 race;  /* Bot Race */
    int                level;  /* Current level */
    int                state;  /* Current action */
    int              favweap;  /* Favorite Weapon */
    int                 fear;  /* Odds of fleeing */
    int               attack;  /* Chance of lashing out */
    int              camping;  /* Chance of camping */
    int             blasting;  /* Chance of grenading */
    int             botspeak;  /* Message delay */
    int                   id;  /* Bot Reference ID */
};

/*
    Support for high-speed pulse Sentry Guns.
*/
struct  sentry_data
{
    SENTRY_DATA*          next;
    SENTRY_DATA*          prev;
    CHAR_DATA*            owner;
    OBJ_DATA*             gun;
    int                   arc;     // Main Firing Direction
    int                   temp;    // Gun Tempeture
    int                   wait;    // Sentry Waitstate
};

/*
    One character (PC or NPC).
    (Shouldn't most of that build interface stuff use substate, dest_buf,
    spare_ptr and tempnum?  Seems a little redundant)
*/
struct  char_data
{
    CHAR_DATA*      next;
    CHAR_DATA*      prev;
    CHAR_DATA*      next_in_room;
    CHAR_DATA*      prev_in_room;
    CHAR_DATA*      master;
    CHAR_DATA*      leader;
    CHAR_DATA*      reply;
    CHAR_DATA*      switched;
    CHAR_DATA*      mount;
    CHAR_DATA*          carrying;
    CHAR_DATA*          carried;
    HHF_DATA*       hunting;
    HHF_DATA*       fearing;
    HHF_DATA*       hating;
    SPEC_FUN*       spec_fun;
    SPEC_FUN*       spec_2;
    SPEC_FUN*           spec_3;
    SPEC_FUN*           spec_4;
    MPROG_ACT_LIST*     mpact;
    int         mpactnum;
    sh_int      mpscriptpos;
    MOB_INDEX_DATA*     pIndexData;
    DESCRIPTOR_DATA*    desc;
    AFFECT_DATA*    first_affect;
    AFFECT_DATA*    last_affect;
    NOTE_DATA*      pnote;
    NOTE_DATA*      comments;
    OBJ_DATA*       first_carrying;
    OBJ_DATA*       last_carrying;
    ROOM_INDEX_DATA*    in_room;
    ROOM_INDEX_DATA*    was_in_room;
    ROOM_INDEX_DATA*    was_sentinel;
    PC_DATA*        pcdata;
    DO_FUN*         last_cmd;
    DO_FUN*         prev_cmd;   /* mapping */
    void*       dest_buf;
    void*       dest_buf_2;
    void*       spare_ptr;
    int         tempnum;
    int         fbonus;        /* For Level bonus */
    int         busy;          /* For ordership special  */
    int         warned;        /* For auth_mob special   */
    EDITOR_DATA*    editor;
    TIMER*      first_timer;
    TIMER*      last_timer;
    char*       name;
    char*       short_descr;
    char*       long_descr;
    char*       description;
    sh_int      substate;
    sh_int      sex;
    sh_int      race;
    sh_int              top_level;
    sh_int      trust;
    int         played;
    int         playedweek;
    time_t      logon;
    time_t      save_time;
    sh_int      timer;
    sh_int      wait;
    sh_int      hit;
    sh_int      max_hit;
    sh_int      move;
    sh_int      max_move;
    sh_int              field;
    sh_int              resin;
    sh_int              max_field;
    int                 currexp;       /* New XP System */
    int                 maxexp;        /* New XP System */
    EXT_BV              act;
    EXT_BV              affected_by;
    int         carry_weight;
    int         carry_number;
    int                 protect[6];            /* Resistance system */
    EXT_BV              speaks;
    EXT_BV              speaking;
    sh_int      position;
    sh_int      defposition;
    sh_int      height;
    sh_int      weight;
    sh_int              ap;                   /* Combat AP */
    sh_int              mp;                   /* Movement AP */
    sh_int              morale;               /* Marine Morale */
    sh_int              teamkill;             /* Team Kills */
    int                 w_wound;              /* Wounding system ->       */
    int                 w_injury;             /*                 >- Ghost */
    EXT_BV              deaf;
    sh_int      perm_str;
    sh_int              perm_sta;
    sh_int              perm_int;
    sh_int              perm_rec;
    sh_int              perm_bra;
    sh_int              perm_per;
    sh_int      mod_str;
    sh_int              mod_sta;
    sh_int              mod_int;
    sh_int              mod_rec;
    sh_int              mod_bra;
    sh_int              mod_per;
    sh_int      mental_state;       /* simplified */
    sh_int      emotional_state;    /* simplified */
    int         pagelen;                        /* BUILD INTERFACE */
    sh_int      inter_page;                     /* BUILD INTERFACE */
    sh_int      inter_type;                     /* BUILD INTERFACE */
    char*        inter_editing;                 /* BUILD INTERFACE */
    int         inter_editing_vnum;             /* BUILD INTERFACE */
    sh_int      inter_substate;                 /* BUILD INTERFACE */
    int         retran;
    int         regoto;
    int                 streak;
    sh_int      mobinvis;   /* Mobinvis level SB */
    sh_int              was_stunned;
    sh_int              main_ability;
    sh_int              vision;        /* Current vision mode */
    EXIT_DATA*          block;         /* When the character blocks a door */
    OBJ_DATA*           cover;         /* Taking cover */
    BOT_DATA*           bot;           /* This is a Bot-controlled character */
    bool                vent;
    bool                was_home;
    sh_int              swarm;
};

/*
    Data which only PC's have.
*/
struct  pc_data
{
    int                 id;              /* ID Code, generated on creation */
    char*               gname;
    char*               fname;           /* First name */
    char*               lname;           /* Last name */
    AREA_DATA*      area;
    char*       homepage;
    char*               council_name;
    char*       pwd;
    char*       email;
    char*       bamfin;
    char*       bamfout;
    char*               rank;
    char*       title;
    char*       bestowments;    /* Special bestowed commands       */
    EXT_BV              flags;          /* Whether the player is deadly and whatever else we add.      */
    KILL_DATA           kills;          /* Your record of kills */
    KILL_DATA           killed;         /* Your record of deaths */
    long int            outcast_time;   /* The time at which the char was outcast */
    long int            restore_time;   /* The last time the char did a restore all */
    int         r_range_lo; /* room range */
    int                 r_range_hi;
    int             m_range_lo; /* mob range  */
    int             m_range_hi;
    int             o_range_lo; /* obj range  */
    int         o_range_hi;
    sh_int      wizinvis;   /* wizinvis level */
    sh_int      min_snoop;  /* minimum snoop level */
    sh_int      learned     [MAX_SKILL];
    sh_int              prepared        [MAX_SKILL];
    sh_int      quest_number;   /* current *QUEST BEING DONE* DON'T REMOVE! */
    sh_int      quest_curr; /* current number of quest points */
    int         quest_accum;    /* quest points accumulated in players life */
    int                 auth_state;
    time_t      release_date;   /* Auto-helling.. Altrag */
    time_t      last_quit;
    char*       helled_by;
    char*       bio;        /* Personal Bio */
    char*       authed_by;  /* what crazy imm authed this name ;) */
    char*       prompt;     /* User config prompts */
    char*       subprompt;  /* Substate prompt */
    sh_int      pagerlen;   /* For pager (NOT menus) */
    int                 respawn;        /* Time-till-respawn */
    ROOM_INDEX_DATA*    respawn_loc;    /* Respawn location */
    bool                spectator;      /* Are we spectating? */
    int                 ooclimit;       /* For OOC Limit-Break */
    bool                oocbreak;       /* For OOC Limit-Break */
    bool        openedtourney;
    char*               ignore;         /* Ignored players */
};



/*
    Extra description data for a room or object.
*/
struct  extra_descr_data
{
    EXTRA_DESCR_DATA* next; /* Next in list                     */
    EXTRA_DESCR_DATA* prev; /* Previous in list                 */
    char* keyword;              /* Keyword in look/examine          */
    char* description;          /* What to see                      */
};



/*
    Prototype for an object.
*/
struct  obj_index_data
{
    OBJ_INDEX_DATA*     next;
    OBJ_INDEX_DATA*     next_sort;
    EXTRA_DESCR_DATA*   first_extradesc;
    EXTRA_DESCR_DATA*   last_extradesc;
    AFFECT_DATA*    first_affect;
    AFFECT_DATA*    last_affect;
    MPROG_DATA*     mudprogs;               /* objprogs */
    // int                 progtypes;              /* objprogs */
    EXT_BV              progtypes;
    char*       name;
    char*       short_descr;
    char*       description;
    char*       action_desc;
    int         vnum;
    sh_int              level;
    sh_int      item_type;
    EXT_BV              extra_flags;
    EXT_BV              wear_flags;
    sh_int      count;
    sh_int      weight;
    int         cost;
    int         value   [6];
    int         serial;
    sh_int      layers;
    int         rent;           /* Unused */
};


/*
    One object.
*/
struct  obj_data
{
    OBJ_DATA*       next;
    OBJ_DATA*       prev;
    OBJ_DATA*       next_content;
    OBJ_DATA*       prev_content;
    OBJ_DATA*       first_content;
    OBJ_DATA*       last_content;
    OBJ_DATA*       in_obj;
    CHAR_DATA*      carried_by;
    CHAR_DATA*          parent;           /* Parent - Who controls it */
    EXTRA_DESCR_DATA*   first_extradesc;
    EXTRA_DESCR_DATA*   last_extradesc;
    AFFECT_DATA*    first_affect;
    AFFECT_DATA*    last_affect;
    OBJ_INDEX_DATA*     pIndexData;
    ROOM_INDEX_DATA*    in_room;
    char*       armed_by;
    char*       name;
    char*       short_descr;
    char*       description;
    char*       action_desc;
    char*               killed_by;
    sh_int      item_type;
    sh_int      mpscriptpos;
    EXT_BV              extra_flags;
    EXT_BV              wear_flags;
    OBJ_DATA*           ammo;           /* Current ammo cartridge */
    OBJ_DATA*           attach;         /* Current attachment */
    int                 weapon_mode;
    MPROG_ACT_LIST*     mpact;      /* mudprogs */
    int         mpactnum;   /* mudprogs */
    sh_int      wear_loc;
    sh_int      weight;
    int         cost;
    sh_int      level;
    sh_int      timer;
    int         value   [6];
    sh_int      count;      /* support for object grouping */
    int         serial;     /* serial number           */
};


/*
    Exit data.
*/
struct  exit_data
{
    EXIT_DATA*      prev;       /* previous exit in linked list */
    EXIT_DATA*      next;       /* next exit in linked list */
    EXIT_DATA*      rexit;      /* Reverse exit pointer     */
    ROOM_INDEX_DATA*    to_room;    /* Pointer to destination room  */
    char*       keyword;    /* Keywords for exit or door    */
    char*       description;    /* Description of exit      */
    int         vnum;       /* Vnum of room exit leads to   */
    int         rvnum;      /* Vnum of room in opposite dir */
    EXT_BV      exit_info;  /* door states & other flags    */
    int         key;        /* Key vnum         */
    sh_int      vdir;       /* Physical "direction"     */
    sh_int      distance;   /* how far to the next room */
};



/*
    Reset commands:
     '*': comment
     'M': read a mobile
     'O': read an object
     'P': put object in object
     'G': give object to mobile
     'E': equip object to mobile
     'H': hide an object
     'B': set a bitvector
     'T': trap an object
     'D': set state of door
     'R': randomize room exits
     'S': stop (end of list)
*/

/*
    Area-reset definition.
*/
struct  reset_data
{
    RESET_DATA*     next;
    RESET_DATA*     prev;
    char        command;
    int         extra;
    int         arg1;
    int         arg2;
    int         arg3;
};

/* Constants for arg2 of 'B' resets. */
#define BIT_RESET_DOOR          0
#define BIT_RESET_OBJECT        1
#define BIT_RESET_MOBILE        2
#define BIT_RESET_ROOM          3
#define BIT_RESET_TYPE_MASK     0xFF    /* 256 should be enough */
#define BIT_RESET_DOOR_THRESHOLD    8
#define BIT_RESET_DOOR_MASK     0xFF00  /* 256 should be enough */
#define BIT_RESET_SET           BV30
#define BIT_RESET_TOGGLE        BV31
#define BIT_RESET_FREEBITS    0x3FFF0000    /* For reference */


/*
    Bot Waypoint Data.
*/
struct  wp_data
{
    WP_DATA*            next;
    WP_DATA*            prev;
    int                 vnum;  // Where is this Waypoint.
    int                 race;  // Who is it for.
    int                 type;  // What kind is it.
    int                 rate;  // Waypoint Rating.
};

#define WP_STORE       0
#define WP_DEPLOY      1
#define WP_HOSTAGE     2
#define WP_SNIPE       3
#define WP_CONTROL     4
#define WP_DYING       5

/*
    Variable Stack System.
*/
struct  var_data
{
    VAR_DATA*           next;
    VAR_DATA*           prev;
    char*               name;
    int                 value;
};

/*
    Area definition.
*/
struct  area_data
{
    AREA_DATA*      next;
    AREA_DATA*      prev;
    AREA_DATA*      next_sort;
    AREA_DATA*      prev_sort;
    RESET_DATA*     first_reset;
    RESET_DATA*     last_reset;
    AREA_DATA*      next_on_planet;
    AREA_DATA*      prev_on_planet;
    VAR_DATA*           first_var;
    VAR_DATA*           last_var;
    WP_DATA*            first_wp;
    WP_DATA*            last_wp;
    char*       name;
    char*       filename;
    EXT_BV              flags;
    sh_int              status;  /* h, 8/11 */
    sh_int      age;
    sh_int      nplayer;
    sh_int      reset_frequency;
    int                 ambience;
    int                 version;
    int         low_r_vnum;
    int                 hi_r_vnum;
    int         low_o_vnum;
    int         hi_o_vnum;
    int         low_m_vnum;
    int         hi_m_vnum;
    int         low_soft_range;
    int         hi_soft_range;
    int         low_hard_range;
    int         hi_hard_range;
    char*       author; /* Scryn */
    char*               resetmsg; /* Rennard */
    RESET_DATA*     last_mob_reset;
    RESET_DATA*     last_obj_reset;
    sh_int      max_players;
    int         mkills;
    int         mdeaths;
    int         pkills;
    int         pdeaths;
    int         illegal_pk;
};

/*
    Load in the gods building data. -- Altrag
*/
struct  godlist_data
{
    GOD_DATA*       next;
    GOD_DATA*       prev;
    int         level;
    int         low_r_vnum;
    int         hi_r_vnum;
    int         low_o_vnum;
    int         hi_o_vnum;
    int         low_m_vnum;
    int         hi_m_vnum;
};


/*
    Used to keep track of system settings and statistics     -Thoric
*/
struct  system_data
{
    int         currid;                 /* Used for tracking player IDs */
    bool        ALLOW_OOC;              /* OOC is allowed */
    bool        SET_WIZLOCK;            /* Wizlock is set */
    int     maxplayers;     /* Maximum players this boot   */
    int     alltimemax;     /* Maximum players ever   */
    char*   time_of_max;        /* Time of max ever */
    bool    NO_NAME_RESOLVING;  /* Hostnames are not resolved  */
    bool        DENY_NEW_PLAYERS;   /* New players cannot connect  */
    bool    WAIT_FOR_AUTH;      /* New players must be auth'ed */
    sh_int  read_all_mail;      /* Read all player mail(was 54)*/
    sh_int  read_mail_free;     /* Read mail for free (was 51) */
    sh_int  write_mail_free;    /* Write mail for free(was 51) */
    sh_int  take_others_mail;   /* Take others mail (was 54)   */
    sh_int  muse_level;     /* Level of muse channel */
    sh_int  think_level;        /* Level of think channel LEVEL_HIGOD*/
    sh_int  build_level;        /* Level of build channel LEVEL_BUILD*/
    sh_int  log_level;      /* Level of log channel LEVEL LOG*/
    sh_int  level_modify_proto; /* Level to modify prototype stuff LEVEL_LESSER */
    sh_int  level_override_private; /* override private flag */
    sh_int  level_mset_player;  /* Level to mset a player */
    sh_int  stun_plr_vs_plr;    /* Stun mod player vs. player */
    sh_int  stun_regular;       /* Stun difficult */
    sh_int  dam_plr_vs_plr;     /* Damage mod player vs. player */
    sh_int  dam_plr_vs_mob;     /* Damage mod player vs. mobile */
    sh_int  dam_mob_vs_plr;     /* Damage mod mobile vs. player */
    sh_int  dam_mob_vs_mob;     /* Damage mod mobile vs. mobile */
    sh_int  level_getobjnotake;     /* Get objects without take flag */
    sh_int      level_forcepc;          /* The level at which you can use force on players. */
    sh_int  max_sn;         /* Max skills */
    char*       guild_overseer;         /* Pointer to char containing the name of the */
    char*       guild_advisor;      /* guild overseer and advisor. */
    int     save_flags;     /* Toggles for saving conditions */
    sh_int  save_frequency;     /* How old to autosave someone */
    bool        pkallowed;              /* Is PK Allowed right now? */
    sh_int      ident_retries;
    sh_int  newbie_purge; /* Level to auto-purge newbies at - Samson 12-27-98 */
    sh_int  regular_purge; /* Level to purge normal players at - Samson 12-27-98 */
    bool CLEANPFILES; /* Should the mud clean up pfiles daily? - Samson 12-27-98 */
    bool TMCBLOCK;    /* Block connections from the TMC */
    char* exe_file;
    char* mqtt_host;
    int   mqtt_port;
    bool  mqtt_enabled;
};


/*
    Room type.
*/
struct  room_index_data
{
    ROOM_INDEX_DATA*    next;
    ROOM_INDEX_DATA*    next_sort;
    ROOM_INDEX_DATA*    link;            /* TNS Module */
    CHAR_DATA*      first_person;
    CHAR_DATA*      last_person;
    OBJ_DATA*       first_content;
    OBJ_DATA*       last_content;
    EXTRA_DESCR_DATA*   first_extradesc;
    EXTRA_DESCR_DATA*   last_extradesc;
    AREA_DATA*      area;
    EXIT_DATA*      first_exit;
    EXIT_DATA*      last_exit;
    char*       name;
    char*       description;
    char*               hdescription;
    int         vnum;
    int                 x, y, z;
    EXT_BV              room_flags;
    EXT_BV              hroom_flags;
    MPROG_ACT_LIST*     mpact;               /* mudprogs */
    int         mpactnum;            /* mudprogs */
    MPROG_DATA*     mudprogs;            /* mudprogs */
    sh_int      mpscriptpos;
    // int                 progtypes;           /* mudprogs */
    EXT_BV              progtypes;
    sh_int      light;
    sh_int      sector_type;
    int         tele_vnum;
    sh_int      tele_delay;
    sh_int              tunnel;              /* max people that will fit */
};

/*
    Delayed teleport type.
*/
struct  teleport_data
{
    TELEPORT_DATA*  next;
    TELEPORT_DATA*  prev;
    ROOM_INDEX_DATA*    room;
    sh_int      timer;
};


/*
    Types of skill numbers.  Used to keep separate lists of sn's
    Must be non-overlapping with spell/skill types,
    but may be arbitrary beyond that.
*/
#define TYPE_UNDEFINED       -1
#define TYPE_HIT             1000  /* allows for 1000 skills/spells   */
#define TYPE_HERB            2000  /* allows for 1000 attack types    */
#define TYPE_PERSONAL        3000  /* allows for 1000 herb types      */
#define TYPE_GENERIC         4000  /* allows for 1000 personal types  */

/*
    Target types.
*/
typedef enum
{
    TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF,
    TAR_OBJ_INV
} target_types;

typedef enum
{
    SKILL_UNKNOWN, SKILL_SPELL, SKILL_SKILL, SKILL_WEAPON, SKILL_TONGUE,
    SKILL_HERB, SKILL_RACIAL, SKILL_DISEASE
} skill_types;



struct timerset
{
    int num_uses;
    struct timeval total_time;
    struct timeval min_time;
    struct timeval max_time;
};



/*
    Skills include spells as a particular case.
*/
struct  skill_type
{
    char*         name;            /* Name of skill        */
    DO_FUN*       skill_fun;       /* Skill pointer (for skills)   */
    sh_int      minimum_position;  /* Position for caster / user   */
    sh_int      slot;              /* Slot for #OBJECT loading */
    sh_int      reset;             /* Rounds required to reset after usage */
    sh_int      race;              /* Race the skill belongs to */
    sh_int      min_level;         /* Minimum level to learn */
    sh_int      type;              /* Skill/Weapon/Tongue    */
    char*       msg_off;           /* Wear off message     */
    struct      timerset   userec; /* Usage record         */
};


/*
    These are skill_lookup return values for common skills and spells.
*/
extern sh_int   gsn_lightarmor;
extern sh_int   gsn_friendly_fire;
extern sh_int   gsn_basic_medical;
extern sh_int   gsn_eagle_eye;
extern sh_int   gsn_sharpshooting;
extern sh_int   gsn_handweapons;
extern sh_int   gsn_alertness;
extern sh_int   gsn_awareness;
extern sh_int   gsn_mediumarmor;
extern sh_int   gsn_electronics;
extern sh_int   gsn_advanced_medical;
extern sh_int   gsn_near_miss;
extern sh_int   gsn_demolitions;
extern sh_int   gsn_heavyarmor;
extern sh_int   gsn_cartography;
extern sh_int   gsn_logistics;
extern sh_int   gsn_requestassist;
extern sh_int   gsn_moraleboost;
extern sh_int   gsn_airsupport;

extern sh_int   gsn_dodge;
extern sh_int   gsn_evasive;
extern sh_int   gsn_alien_rage;
extern sh_int   gsn_battle_method;
extern sh_int   gsn_tailslam;
extern sh_int   gsn_critical_strike;
extern sh_int   gsn_acidspit;
extern sh_int   gsn_leap;
extern sh_int   gsn_breach;
extern sh_int   gsn_stealth;
extern sh_int   gsn_lunge;
extern sh_int   gsn_acid_potency;
extern sh_int   gsn_headbite;
extern sh_int   gsn_confuse;
extern sh_int   gsn_acute_hearing;
extern sh_int   gsn_pursuit;
extern sh_int   gsn_pheromones;
extern sh_int   gsn_impale;
extern sh_int   gsn_layeggs;

extern sh_int   gsn_battle_focus;
extern sh_int   gsn_track;
extern sh_int   gsn_vision_modes;
extern sh_int   gsn_nightvision;
extern sh_int   gsn_shoulder_cannon;
extern sh_int   gsn_medical;
extern sh_int   gsn_cr_combat;
extern sh_int   gsn_tumble;
extern sh_int   gsn_trap_setting;
extern sh_int   gsn_double_strike;
extern sh_int   gsn_venom_cloud;
extern sh_int   gsn_selfdestruct;
extern sh_int   gsn_rage;
extern sh_int   gsn_ranged_combat;
extern sh_int   gsn_primal_instinct;
extern sh_int   gsn_sixth_sense;
extern sh_int   gsn_battlecry;
extern sh_int   gsn_human_tech;

/* used to do specific lookups */
extern sh_int   gsn_first_skill;
extern sh_int   gsn_first_weapon;
extern sh_int   gsn_first_tongue;
extern sh_int   gsn_top_sn;

/* languages */
extern sh_int   gsn_marine;
extern sh_int   gsn_alien;
extern sh_int   gsn_predator;

/*
    Utility macros.
*/
#define UMIN(a, b)      ((a) < (b) ? (a) : (b))
#define UMAX(a, b)      ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)     ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)        ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)   ((flag) & (bit))
#define SET_BIT(var, bit)   ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

/*
    Macros for accessing virtually unlimited bitvectors.     -Thoric
    Added to AVP by Ghost

    Note that these macros use the bit number rather than the bit value
    itself -- which means that you can only access _one_ bit at a time

    This code uses an array of integers

    --------------------------------------------------------------------------
    --------------------------------------------------------------------------
    --------------------------------------------------------------------------
*/

/*
    The functions for these prototypes can be found in misc.c
    They are up here because they are used by the macros below
*/
bool    ext_is_empty        args( ( EXT_BV* bits ) );
void    ext_clear_bits      args( ( EXT_BV* bits ) );
int ext_has_bits        args( ( EXT_BV* var, EXT_BV* bits ) );
bool    ext_same_bits       args( ( EXT_BV* var, EXT_BV* bits ) );
void    ext_set_bits        args( ( EXT_BV* var, EXT_BV* bits ) );
void    ext_remove_bits     args( ( EXT_BV* var, EXT_BV* bits ) );
void    ext_toggle_bits     args( ( EXT_BV* var, EXT_BV* bits ) );

/*
    Here are the extended bitvector macros:
*/
#define xIS_SET(var, bit)   ((var).bits[(bit) >> RSV] & 1 << ((bit) & XBM))
#define xSET_BIT(var, bit)  ((var).bits[(bit) >> RSV] |= 1 << ((bit) & XBM))
#define xSET_BITS(var, bit) (ext_set_bits(&(var), &(bit)))
#define xREMOVE_BIT(var, bit)   ((var).bits[(bit) >> RSV] &= ~(1 << ((bit) & XBM)))
#define xREMOVE_BITS(var, bit)  (ext_remove_bits(&(var), &(bit)))
#define xTOGGLE_BIT(var, bit)   ((var).bits[(bit) >> RSV] ^= 1 << ((bit) & XBM))
#define xTOGGLE_BITS(var, bit)  (ext_toggle_bits(&(var), &(bit)))
#define xCLEAR_BITS(var)    (ext_clear_bits(&(var)))
#define xIS_EMPTY(var)      (ext_is_empty(&(var)))
#define xHAS_BITS(var, bit) (ext_has_bits(&(var), &(bit)))
#define xSAME_BITS(var, bit)    (ext_same_bits(&(var), &(bit)))
/*
    --------------------------------------------------------------------------
    --------------------------------------------------------------------------
    --------------------------------------------------------------------------
    --------------------------------------------------------------------------
*/

/*
    Memory allocation macros.
*/

/*
    Memory allocation macros.
*/
#define CREATE(result, type, number)                    \
    do                                          \
    {                                           \
        if (!((result) = (type *) calloc ((number), sizeof(type)))) \
        {                                           \
            perror("malloc failure");                       \
            fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
            abort();                                    \
        }       \
        memset((result),0,sizeof(type)*(number));                                   \
    } while(0)

#define RECREATE(result,type,number)                    \
    do                                          \
    {                                           \
        if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
        {                                           \
            perror("realloc failure");                      \
            fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
            abort();                                    \
        }                                           \
    } while(0)


#define DISPOSE(point)                              \
    do                                          \
    {                                           \
        if((point))                                  \
        {                                            \
            free((point));                              \
            (point) = NULL;                             \
        }                                            \
    } while(0)

#ifdef HASHSTR
#define STRALLOC(point)     str_alloc((point))
#define QUICKLINK(point)    quick_link((point))
#define QUICKMATCH(p1, p2)  (strcmp((p1), (p2)) == 0)
#define STRFREE(point)                              \
    do                                          \
    {                                           \
        if((point))                                  \
        {                                            \
            if( str_free((point)) == -1 )                       \
                fprintf(stderr, "STRFREEing bad pointer in %s, line %d\n", __FILE__, __LINE__ ); \
            (point) = NULL;                             \
        }                                            \
    } while(0)
#else
#define STRALLOC(point)     str_dup((point))
#define QUICKLINK(point)    str_dup((point))
#define QUICKMATCH(p1, p2)  strcmp((p1), (p2)) == 0
#define STRFREE(point)      DISPOSE((point))
#endif

/* double-linked list handling macros -Thoric */
/* Updated by Scion 8/6/1999 */
#define LINK(link, first, last, next, prev)                         \
    do                                                                  \
    {                                                                   \
        if ( !(first) )                              \
        {                                                        \
            (first) = (link);                                     \
            (last) = (link);                                  \
        }                                            \
        else                                                         \
            (last)->next = (link);                                    \
        (link)->next = NULL;                                     \
        if ((first) == (link))                               \
            (link)->prev = NULL;                          \
        else                                     \
            (link)->prev = (last);                                    \
        (last) = (link);                                     \
    } while(0)

#define INSERT(link, insert, first, next, prev)                 \
    do                                                              \
    {                                                               \
        (link)->prev = (insert)->prev;                           \
        if ( !(insert)->prev )                                       \
            (first) = (link);                                         \
        else                                                         \
            (insert)->prev->next = (link);                            \
        (insert)->prev = (link);                                     \
        (link)->next = (insert);                                     \
    } while(0)

#define UNLINK(link, first, last, next, prev)                       \
    do                                                                  \
    {                                                                   \
        if ( !(link)->prev )                            \
        {                                                   \
            (first) = (link)->next;                                \
            if ((first))                             \
                (first)->prev = NULL;                     \
        }                                       \
        else                                        \
        {                                                       \
            (link)->prev->next = (link)->next;                     \
        }                                       \
        if ( !(link)->next )                            \
        {                                                   \
            (last) = (link)->prev;                             \
            if ((last))                              \
                (last)->next = NULL;                      \
        }                                       \
        else                                        \
        {                                                           \
            (link)->next->prev = (link)->prev;                     \
        }                                       \
    } while(0)


#define CHECK_LINKS(first, last, next, prev, type)      \
    do {                                \
        type *ptr, *pptr = NULL;                  \
        if ( !(first) && !(last) )                    \
            break;                          \
        if ( !(first) )                       \
        {                             \
            bug( "CHECK_LINKS: last with NULL first!  %s.",     \
                 __STRING(first) );                  \
            for ( ptr = (last); ptr->prev; ptr = ptr->prev );       \
            (first) = ptr;                      \
        }                             \
        else if ( !(last) )                       \
        {                             \
            bug( "CHECK_LINKS: first with NULL last!  %s.",     \
                 __STRING(first) );                  \
            for ( ptr = (first); ptr->next; ptr = ptr->next );      \
            (last) = ptr;                       \
        }                             \
        if ( (first) )                        \
        {                             \
            for ( ptr = (first); ptr; ptr = ptr->next )         \
            {                               \
                if ( ptr->prev != pptr )                  \
                {                             \
                    bug( "CHECK_LINKS(%s): %p:->prev != %p.  Fixing.",  \
                         __STRING(first), ptr, pptr );           \
                    ptr->prev = pptr;                   \
                }                             \
                if ( ptr->prev && ptr->prev->next != ptr )        \
                {                             \
                    bug( "CHECK_LINKS(%s): %p:->prev->next != %p.  Fixing.",\
                         __STRING(first), ptr, ptr );            \
                    ptr->prev->next = ptr;                  \
                }                             \
                pptr = ptr;                       \
            }                               \
            pptr = NULL;                        \
        }                             \
        if ( (last) )                         \
        {                             \
            for ( ptr = (last); ptr; ptr = ptr->prev )          \
            {                               \
                if ( ptr->next != pptr )                  \
                {                             \
                    bug( "CHECK_LINKS (%s): %p:->next != %p.  Fixing.", \
                         __STRING(first), ptr, pptr );           \
                    ptr->next = pptr;                   \
                }                             \
                if ( ptr->next && ptr->next->prev != ptr )        \
                {                             \
                    bug( "CHECK_LINKS(%s): %p:->next->prev != %p.  Fixing.",\
                         __STRING(first), ptr, ptr );            \
                    ptr->next->prev = ptr;                  \
                }                             \
                pptr = ptr;                       \
            }                               \
        }                             \
    } while(0)


#define ASSIGN_GSN(gsn, skill)                  \
    do                              \
    {                               \
        if ( ((gsn) = skill_lookup((skill))) == -1 )        \
            fprintf( stderr, "ASSIGN_GSN: Skill %s not found.\n",   \
                     (skill) );                  \
    } while(0)

#define CHECK_SUBRESTRICTED(ch)                 \
    do                              \
    {                               \
        if ( (ch)->substate == SUB_RESTRICTED )         \
        {                               \
            send_to_char( "You cannot use this command from within another command.\n\r", ch ); \
            return;                         \
        }                               \
    } while(0)


/*
    Character macros.
*/
#define IS_NPC(ch)      (xIS_SET((ch)->act, ACT_IS_NPC))
#define IS_BOT(ch)              ((ch)->bot)
#define IN_VENT(ch)             ((ch)->vent)
#define IS_BUILDWALKING(ch)     (xIS_SET((ch)->act, PLR_BUILDWALK))
#define IS_IMMORTAL(ch)     (get_trust((ch)) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)     (get_trust((ch)) >= LEVEL_HERO)
#define IS_AFFECTED(ch, sn) (xIS_SET((ch)->affected_by, (sn)))

#define CH(descriptor)  ((descriptor)->original ? (descriptor)->original : (descriptor)->character)

#define IS_AWAKE(ch)            ((ch)->position > POS_STUNNED)

#define IS_INHIVE(ch)      (xIS_SET( (ch)->in_room->room_flags, ROOM_HIVED) )

#define IS_INSIDE(ch)      (xIS_SET( (ch)->in_room->room_flags, ROOM_INDOORS) \
                            || xIS_SET( (ch)->in_room->room_flags, ROOM_UNDERGROUND) )

#define IS_OUTSIDE(ch)      (!xIS_SET( (ch)->in_room->room_flags, ROOM_INDOORS) \
                             && !xIS_SET( (ch)->in_room->room_flags, ROOM_SPACECRAFT) \
                             && !xIS_SET( (ch)->in_room->room_flags, ROOM_UNDERGROUND) )

/*
    define WAIT_STATE(ch, npulse)  ((ch)->wait = UMAX((ch)->wait, (npulse)))
*/

#define WAIT_STATE(ch, npulse)  ((ch)->wait = (ch)->wait + (npulse))

#define EXIT(ch, door)      ( get_exit( (ch)->in_room, door ) )

#define CAN_GO(ch, door)    (EXIT((ch),(door))           \
                             && (EXIT((ch),(door))->to_room != NULL)  \
                             && !xIS_SET(EXIT((ch), (door))->exit_info, EX_CLOSED))

#define IS_VALID_SN(sn)     ( (sn) >=0 && (sn) < MAX_SKILL           \
                              && skill_table[(sn)]                 \
                              && skill_table[(sn)]->name )

#define IS_VALID_HERB(sn)   ( (sn) >=0 && (sn) < MAX_HERB            \
                              && herb_table[(sn)]              \
                              && herb_table[(sn)]->name )

#define IS_VALID_DISEASE(sn)   ( (sn) >=0 && (sn) < MAX_DISEASE       \
                                 && disease_table[(sn)]              \
                                 && disease_table[(sn)]->name )

#define IS_QUESTOR(ch)     (xIS_SET((ch)->act, PLR_QUESTOR))

/* Retired and guest imms. */
#define IS_RETIRED(ch) (ch->pcdata && xIS_SET(ch->pcdata->flags,PCFLAG_RETIRED))
#define IS_GUEST(ch) (ch->pcdata && xIS_SET(ch->pcdata->flags,PCFLAG_GUEST))

#define IS_IDLE(d) ( (d)->idle > (60 * 4) )
#define NOT_AUTHED(ch)      (!IS_NPC(ch) && ch->pcdata->auth_state <= 3  \
                             && xIS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

#define IS_WAITING_FOR_AUTH(ch) (!IS_NPC(ch) && ch->desc             \
                                 && ( ch->pcdata->auth_state == 1           \
                                      || ch->pcdata->auth_state == 2 )           \
                                 && xIS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

/*
    Object macros.
*/
#define CAN_WEAR(obj, part) (xIS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)  (xIS_SET((obj)->extra_flags, (stat)))

#define ANSISAFE( ansi, txt )   ( ansi ? (txt) : "" )

/*
    Description macros.
*/
#define PERS(ch, looker)    ( can_see( (looker), (ch) ) ?       \
                              ( IS_NPC(ch) ? (ch)->short_descr    \
                                : g_name((ch), (looker)) ) : ( IS_IMMORTAL(ch) ?    \
                                        "Immortal" : "Someone" ) )


#define SPERS(ch, looker)       ( can_see( (looker), (ch) ) ?           \
                                  ( IS_NPC(ch) ? (ch)->short_descr        \
                                    : (ch)->name ) : ( IS_IMMORTAL(ch) ?    \
                                            "Immortal" : "Someone" ) )

/*
    #define PERS(ch, looker)    ( can_see( (looker), (ch) ) ?       \
                                ( IS_NPC(ch) ? (ch)->short_descr        \
                : (ch)->name ) : "someone" )

*/

#define log_string( txt )   ( log_string_plus( (txt), LOG_NORMAL, LEVEL_LOG ) )


/*
    Structure for a command in the command lookup table.
*/
struct  cmd_type
{
    CMDTYPE*        next;
    char*       name;
    DO_FUN*         do_fun;
    sh_int      position;
    sh_int      level;
    sh_int      log;
    sh_int              ooc;
    struct      timerset    userec;
};



/*
    Structure for a social in the socials table.
*/
struct  social_type
{
    SOCIALTYPE*     next;
    char*       name;
    char*       char_no_arg;
    char*       others_no_arg;
    char*       char_found;
    char*       others_found;
    char*       vict_found;
    char*       char_auto;
    char*       others_auto;
};



/*
    Global constants.
*/
extern  time_t last_restore_all_time;
extern  time_t boot_time;  /* this should be moved down */
extern  HOUR_MIN_SEC* set_boot_time;
extern  struct  tm* new_boot_time;
extern  time_t new_boot_time_t;

extern  time_t pfile_time;
extern  HOUR_MIN_SEC* set_pfile_time;
extern  struct  tm* new_pfile_time;
extern  time_t new_pfile_time_t;
extern  sh_int num_pfiles;

extern  const   struct  race_type     race_table  [MAX_RACE];
extern  const   struct  quote_type  quote_table [MAX_QUOTES];
extern  int     const                   weapon_accuracy [MAX_WEAPON_TYPES];

extern  char*   const   skill_tname [];
extern  sh_int  const   movement_loss   [SECT_MAX];
extern  char*   const   dir_name    [];
extern  char*   const   where_name  [];
extern  const   sh_int  rev_dir     [];
extern  char*   const   r_flags     [];
extern  char*   const   r_flags2    [];
extern  char*   const   w_flags     [];
extern  char*   const   o_flags     [];
extern  char*   const   o_flags2    [];
extern  char*   const   a_flags     [];
extern  char*   const   o_types     [];
extern  char*   const   a_types     [];
extern  char*   const   act_flags   [];
extern  char*   const   plr_flags   [];
extern  char*   const   pc_flags    [];
extern  char*   const   ris_flags   [];
extern  char*   const   trig_flags  [];
extern  char*   const   part_flags  [];
extern  char*   const   npc_race    [];
extern  char*   const   class_types     [];
extern  char*   const   curse_table     [];
extern  char*   const   ignore_table    [];
extern  char*   const   radio_set1      [];
extern  char*   const   radio_set2      [];
extern  char*   const   radio_set3      [];
extern  char*   const   radio_set4      [];
extern  char*   const   vclass_types    [];
extern  char*   const   area_flags  [];

extern  int const   lang_array      [];
extern  char*   const   lang_names      [];


/*
    Global variables.
*/
extern bool fCopyOver;   /* Copyover recover in progress */
extern bool pk_allow;

extern MPSLEEP_DATA* first_mpwait;
extern MPSLEEP_DATA* last_mpwait;
extern MPSLEEP_DATA* current_mpwait;

extern char*    bigregex;
extern char*    preg;

extern  int numobjsloaded;
extern  int nummobsloaded;
extern  int physicalobjects;
extern  int num_descriptors;
extern  int     pmPulse;              /* Monitor Slave Process */
extern  struct  system_data     sysdata;
extern  int top_sn;
extern  int top_vroom;
extern  int top_herb;

extern      CMDTYPE*        command_hash    [126];

extern      SKILLTYPE*      skill_table [MAX_SKILL];
extern      SOCIALTYPE*     social_index    [27];
extern      CHAR_DATA*      cur_char;
extern      ROOM_INDEX_DATA*    cur_room;
extern      bool            cur_char_died;
extern      ch_ret          global_retcode;
extern          SKILLTYPE*      herb_table    [MAX_HERB];
extern          SKILLTYPE*      disease_table [MAX_DISEASE];

extern      int         cur_obj;
extern      int         cur_obj_serial;
extern      bool            cur_obj_extracted;
extern      obj_ret         global_objcode;

extern      HELP_DATA*      first_help;
extern      HELP_DATA*      last_help;
extern      SHOP_DATA*      first_shop;
extern      SHOP_DATA*      last_shop;

extern          VOTE_DATA*              curr_vote;

extern      BAN_DATA*       first_ban;
extern      BAN_DATA*       last_ban;
extern struct       allowmp_data*           mplist;
extern      CHAR_DATA*      first_char;
extern      CHAR_DATA*      last_char;
extern          BOT_DATA*               first_bot;
extern          BOT_DATA*               last_bot;
extern          SENTRY_DATA*            first_sentry;
extern          SENTRY_DATA*            last_sentry;
extern      DESCRIPTOR_DATA*    first_descriptor;
extern      DESCRIPTOR_DATA*    last_descriptor;
extern      BOARD_DATA*     first_board;
extern      BOARD_DATA*     last_board;
extern      OBJ_DATA*       first_object;
extern      OBJ_DATA*       last_object;
extern          ARENA_DATA*             first_arena;
extern          ARENA_DATA*             last_arena;
extern          ARENA_DATA*             curr_arena;
extern      AREA_DATA*      first_area;
extern      AREA_DATA*      last_area;
extern      AREA_DATA*      first_build;
extern      AREA_DATA*      last_build;
extern      AREA_DATA*      first_asort;
extern      AREA_DATA*      last_asort;
extern      AREA_DATA*      first_bsort;
extern      AREA_DATA*      last_bsort;
/*
    extern      GOD_DATA      * first_imm;
    extern      GOD_DATA      * last_imm;
*/
extern      TELEPORT_DATA*      first_teleport;
extern      TELEPORT_DATA*      last_teleport;
extern      OBJ_DATA*       extracted_obj_queue;
extern      EXTRACT_CHAR_DATA*  extracted_char_queue;
extern          OBJ_DATA*               save_equipment[2][MAX_WEAR][MAX_LAYERS];
extern      CHAR_DATA*      quitting_char;
extern      CHAR_DATA*      loading_char;
extern      CHAR_DATA*      saving_char;
extern      OBJ_DATA*       all_obj;

extern      char            bug_buf     [MSL];
extern      time_t          current_time;
extern      bool            fLogAll;
extern      FILE*           fpReserve;
extern      FILE*           fpLOG;
extern          FILE*                   fpMatch;
extern      char            log_buf     [MSL];
extern      TIME_INFO_DATA      time_info;
extern      WEATHER_DATA        weather_info;

extern      struct act_prog_data*   mob_act_list;

/*
    Command functions.
    Defined in act_*.c (mostly).
*/
DECLARE_DO_FUN( do_carry );
DECLARE_DO_FUN( do_cache );
DECLARE_DO_FUN( do_cover );
DECLARE_DO_FUN( do_rescue );
DECLARE_DO_FUN( do_rage );
DECLARE_DO_FUN( do_request );
DECLARE_DO_FUN( do_release );
DECLARE_DO_FUN( do_respawn );
DECLARE_DO_FUN( do_rcreate );
DECLARE_DO_FUN( do_setmode );
DECLARE_DO_FUN( do_smash );
DECLARE_DO_FUN( do_spit );
DECLARE_DO_FUN( do_delay );
DECLARE_DO_FUN( do_deploy );
DECLARE_DO_FUN( do_setlock );
DECLARE_DO_FUN( do_makearena );
DECLARE_DO_FUN( do_showarena );
DECLARE_DO_FUN( do_allarenas );
DECLARE_DO_FUN( do_setarena );

DECLARE_DO_FUN( do_makebot );
DECLARE_DO_FUN( do_showbot );
DECLARE_DO_FUN( do_allbots );
DECLARE_DO_FUN( do_setbot );

DECLARE_DO_FUN( do_pfiles );
DECLARE_DO_FUN( do_treat );
DECLARE_DO_FUN( do_tackle );
DECLARE_DO_FUN( do_junk );
DECLARE_DO_FUN( do_block );
DECLARE_DO_FUN( do_sound  );
DECLARE_DO_FUN( do_spray );
DECLARE_DO_FUN( do_copyover );
DECLARE_DO_FUN( do_arm );
DECLARE_DO_FUN( do_armor );
DECLARE_DO_FUN( do_use );
DECLARE_DO_FUN( do_snipe );
DECLARE_DO_FUN( do_throw );
DECLARE_DO_FUN( do_reload );
DECLARE_DO_FUN( do_radio );
DECLARE_DO_FUN( do_radio1 );
DECLARE_DO_FUN( do_radio2 );
DECLARE_DO_FUN( do_radio3 );
DECLARE_DO_FUN( do_radio4 );
DECLARE_DO_FUN( do_recall );
DECLARE_DO_FUN( do_stack );
DECLARE_DO_FUN( do_sc );
DECLARE_DO_FUN( do_scan );
DECLARE_DO_FUN( do_compute );
DECLARE_DO_FUN( do_lob );
DECLARE_DO_FUN( do_lunge );
DECLARE_DO_FUN( do_lstat );
DECLARE_DO_FUN( skill_notfound  );

DECLARE_DO_FUN( do_aassign  );
DECLARE_DO_FUN( do_vassign      );
DECLARE_DO_FUN( do_rassign      );
DECLARE_DO_FUN( do_massign      );
DECLARE_DO_FUN( do_oassign      );
DECLARE_DO_FUN( do_advance  );
DECLARE_DO_FUN( do_affected     );
DECLARE_DO_FUN( do_afk          );
DECLARE_DO_FUN( do_aid      );
DECLARE_DO_FUN( do_allow    );
DECLARE_DO_FUN( do_allowmp      );
DECLARE_DO_FUN( do_allowpk      );
DECLARE_DO_FUN( do_ansi     );
DECLARE_DO_FUN( do_answer   );
DECLARE_DO_FUN( do_apply    );
DECLARE_DO_FUN( do_areas    );
DECLARE_DO_FUN( do_arena        );
DECLARE_DO_FUN( do_aset     );
DECLARE_DO_FUN( do_abackup      );
DECLARE_DO_FUN( do_ask      );
DECLARE_DO_FUN( do_astat    );
DECLARE_DO_FUN( do_at       );
DECLARE_DO_FUN( do_attach       );
DECLARE_DO_FUN( do_detach       );
DECLARE_DO_FUN( do_authorize    );
DECLARE_DO_FUN( do_avtalk   );
DECLARE_DO_FUN( do_balzhur  );
DECLARE_DO_FUN( do_bamfin   );
DECLARE_DO_FUN( do_bamfout  );
DECLARE_DO_FUN( do_ban      );
DECLARE_DO_FUN( do_bash     );
DECLARE_DO_FUN( do_bashdoor     );
DECLARE_DO_FUN( do_beep         );
DECLARE_DO_FUN( do_bestow   );
DECLARE_DO_FUN( do_bestowarea   );
DECLARE_DO_FUN( do_bio      );
DECLARE_DO_FUN( do_boards   );
DECLARE_DO_FUN( do_bodybag  );
DECLARE_DO_FUN( do_bset     );
DECLARE_DO_FUN( do_bstat    );
DECLARE_DO_FUN( do_bug      );
DECLARE_DO_FUN( do_bury     );
DECLARE_DO_FUN( do_buy      );
DECLARE_DO_FUN( do_cedit    );
DECLARE_DO_FUN( do_censor       );
DECLARE_DO_FUN( do_channels );
DECLARE_DO_FUN( do_changes      );
DECLARE_DO_FUN( do_chat     );
DECLARE_DO_FUN( do_ooc      );
DECLARE_DO_FUN( do_check_vnums  );
DECLARE_DO_FUN( do_freevnums    );
DECLARE_DO_FUN( do_climb    );
DECLARE_DO_FUN( do_cloak        );
DECLARE_DO_FUN( do_close    );
DECLARE_DO_FUN( do_cmdtable );
DECLARE_DO_FUN( do_cmenu    );
DECLARE_DO_FUN( do_commands );
DECLARE_DO_FUN( do_comment  );
DECLARE_DO_FUN( do_compare  );
DECLARE_DO_FUN( do_config   );
DECLARE_DO_FUN( do_construct    );
DECLARE_DO_FUN( do_confuse      );
DECLARE_DO_FUN( do_credits  );
DECLARE_DO_FUN( do_cset     );
DECLARE_DO_FUN( do_dehive       );
DECLARE_DO_FUN( do_deny     );
DECLARE_DO_FUN( do_description  );
DECLARE_DO_FUN( do_destro       );
DECLARE_DO_FUN( do_destroy      );
DECLARE_DO_FUN( do_dig      );
DECLARE_DO_FUN( do_disarm   );
DECLARE_DO_FUN( do_disconnect   );
DECLARE_DO_FUN( do_dmesg    );
DECLARE_DO_FUN( do_dnd          );
DECLARE_DO_FUN( do_down     );
DECLARE_DO_FUN( do_drag     );
DECLARE_DO_FUN( do_drop     );
DECLARE_DO_FUN( do_east     );
DECLARE_DO_FUN( do_eat      );
DECLARE_DO_FUN( do_echo     );
DECLARE_DO_FUN( do_emote    );
DECLARE_DO_FUN( do_empty    );
DECLARE_DO_FUN( do_eject        );
DECLARE_DO_FUN( do_enter    );
DECLARE_DO_FUN( do_equipment    );
DECLARE_DO_FUN( do_examine  );
DECLARE_DO_FUN( do_exits    );
DECLARE_DO_FUN( do_vote         );
DECLARE_DO_FUN( do_version      );
DECLARE_DO_FUN( do_fixchar  );
DECLARE_DO_FUN( do_fire         );
DECLARE_DO_FUN( do_foldarea );
DECLARE_DO_FUN( do_follow   );
DECLARE_DO_FUN( do_for          );
DECLARE_DO_FUN( do_force    );
DECLARE_DO_FUN( do_forceclose   );
DECLARE_DO_FUN( do_fquit    );     /* Gorog */
DECLARE_DO_FUN( do_form_password );
DECLARE_DO_FUN( do_freeze   );
DECLARE_DO_FUN( do_get      );
DECLARE_DO_FUN( do_show         );     /* Ghost */
DECLARE_DO_FUN( do_give         );
DECLARE_DO_FUN( do_glance   );
DECLARE_DO_FUN( do_goto     );
DECLARE_DO_FUN( do_grub     );
DECLARE_DO_FUN( do_hedit    );
DECLARE_DO_FUN( do_hive         );
DECLARE_DO_FUN( do_hell     );
DECLARE_DO_FUN( do_help     );
DECLARE_DO_FUN( do_hide     );
DECLARE_DO_FUN( do_hitall   );
DECLARE_DO_FUN( do_hlist    );
DECLARE_DO_FUN( do_holylight    );
DECLARE_DO_FUN( do_buildwalk    );
DECLARE_DO_FUN( do_homepage );
DECLARE_DO_FUN( do_hset     );
DECLARE_DO_FUN( do_ignore       );
DECLARE_DO_FUN( do_i103     );
DECLARE_DO_FUN( do_i104     );
DECLARE_DO_FUN( do_i105     );
DECLARE_DO_FUN( do_i200         );
DECLARE_DO_FUN( do_ide      );
DECLARE_DO_FUN( do_complain     );
DECLARE_DO_FUN( do_idea     );
DECLARE_DO_FUN( do_immortalize  );
DECLARE_DO_FUN( do_immtalk  );
DECLARE_DO_FUN( do_installarea  );
DECLARE_DO_FUN( do_instaroom    );
DECLARE_DO_FUN( do_instazone    );
DECLARE_DO_FUN( do_inventory    );
DECLARE_DO_FUN( do_invis    );
DECLARE_DO_FUN( do_kill     );
DECLARE_DO_FUN( do_kneel        );
DECLARE_DO_FUN( do_last     );
DECLARE_DO_FUN( do_leave    );
DECLARE_DO_FUN( do_leap         );
DECLARE_DO_FUN( do_level    );
DECLARE_DO_FUN( do_list     );
DECLARE_DO_FUN( do_loadarea );
DECLARE_DO_FUN( do_loadup   );
DECLARE_DO_FUN( do_lock     );
DECLARE_DO_FUN( do_log      );
DECLARE_DO_FUN( do_look     );
DECLARE_DO_FUN( do_low_purge    );
DECLARE_DO_FUN( do_mailroom );
DECLARE_DO_FUN( do_menu         );
DECLARE_DO_FUN( do_make     );
DECLARE_DO_FUN( do_makeboard    );
DECLARE_DO_FUN( do_makerepair   );
DECLARE_DO_FUN( do_makeshop );
DECLARE_DO_FUN( do_makewizlist  );
DECLARE_DO_FUN( do_memory   );
DECLARE_DO_FUN( do_mcreate  );
DECLARE_DO_FUN( do_mdelete  );
DECLARE_DO_FUN( medit_copy      );
DECLARE_DO_FUN( do_mfind    );
DECLARE_DO_FUN( do_minvoke  );
DECLARE_DO_FUN( do_mlist    );
DECLARE_DO_FUN( do_mset     );
DECLARE_DO_FUN( do_mstat    );
DECLARE_DO_FUN( do_mwhere   );
DECLARE_DO_FUN( do_name     );
DECLARE_DO_FUN( do_newbiechat   );
DECLARE_DO_FUN( do_newbieset    );
DECLARE_DO_FUN( do_news         );
DECLARE_DO_FUN( do_newzones );
DECLARE_DO_FUN( do_noemote  );
DECLARE_DO_FUN( do_noresolve    );
DECLARE_DO_FUN( do_setooc       );
DECLARE_DO_FUN( do_north    );
DECLARE_DO_FUN( do_northeast    );
DECLARE_DO_FUN( do_northwest    );
DECLARE_DO_FUN( do_notell   );
DECLARE_DO_FUN( do_noteroom );
DECLARE_DO_FUN( do_ocreate  );
DECLARE_DO_FUN( do_odelete  );
DECLARE_DO_FUN( oedit_copy      );
DECLARE_DO_FUN( do_ofind    );
DECLARE_DO_FUN( do_ogrub    );
DECLARE_DO_FUN( do_oinvoke  );
DECLARE_DO_FUN( do_oldscore );
DECLARE_DO_FUN( do_olist    );
DECLARE_DO_FUN( do_open     );
DECLARE_DO_FUN( do_opentourney  );
DECLARE_DO_FUN( do_oset     );
DECLARE_DO_FUN( do_ostat    );
DECLARE_DO_FUN( do_ot       );
DECLARE_DO_FUN( do_owhere   );
DECLARE_DO_FUN( do_prone        );
DECLARE_DO_FUN( do_pager    );
DECLARE_DO_FUN( do_password );
DECLARE_DO_FUN( do_practice );
DECLARE_DO_FUN( do_prompt   );
DECLARE_DO_FUN( do_purge    );
DECLARE_DO_FUN( do_put      );
DECLARE_DO_FUN( do_qpset    );
DECLARE_DO_FUN( do_qui      );
DECLARE_DO_FUN( do_quit     );
DECLARE_DO_FUN( do_rat      );
DECLARE_DO_FUN( do_rdelete  );
DECLARE_DO_FUN( do_rdig         );
DECLARE_DO_FUN( do_reboo    );
DECLARE_DO_FUN( do_reboot   );
DECLARE_DO_FUN( do_recall   );
DECLARE_DO_FUN( do_recho    );
DECLARE_DO_FUN( do_redit    );
DECLARE_DO_FUN( do_rcopy        );
DECLARE_DO_FUN( do_rpcopy       );
DECLARE_DO_FUN( redit_copy      );
DECLARE_DO_FUN( do_regoto       );
DECLARE_DO_FUN( do_remove   );
DECLARE_DO_FUN( do_reply    );
DECLARE_DO_FUN( do_rescue   );
DECLARE_DO_FUN( do_reset    );
DECLARE_DO_FUN( do_prestore     );
DECLARE_DO_FUN( do_restore  );
DECLARE_DO_FUN( do_restoretime  );
DECLARE_DO_FUN( do_restrict );
DECLARE_DO_FUN( do_retran       );
DECLARE_DO_FUN( do_return   );
DECLARE_DO_FUN( do_revert   );
DECLARE_DO_FUN( do_rip      );
DECLARE_DO_FUN( do_rlist    );
DECLARE_DO_FUN( do_rfill        );
DECLARE_DO_FUN( do_rreset   );
DECLARE_DO_FUN( do_rstat    );
DECLARE_DO_FUN( do_save     );
DECLARE_DO_FUN( do_saveall      );
DECLARE_DO_FUN( do_savearea );
DECLARE_DO_FUN( do_say      );
DECLARE_DO_FUN( do_html         );
DECLARE_DO_FUN( do_score    );
DECLARE_DO_FUN( do_nothing      );
DECLARE_DO_FUN( do_sedit    );
DECLARE_DO_FUN( do_sell     );
DECLARE_DO_FUN( do_set_boot_time );
DECLARE_DO_FUN( do_shops    );
DECLARE_DO_FUN( do_shopset  );
DECLARE_DO_FUN( do_shopstat );
DECLARE_DO_FUN( do_shout    );
DECLARE_DO_FUN( do_shove    );
DECLARE_DO_FUN( do_shutdow  );
DECLARE_DO_FUN( do_shutdown );
DECLARE_DO_FUN( do_silence  );
DECLARE_DO_FUN( do_sit      );
DECLARE_DO_FUN( do_sla      );
DECLARE_DO_FUN( do_slay     );
DECLARE_DO_FUN( do_slist        );
DECLARE_DO_FUN( do_slookup  );
DECLARE_DO_FUN( do_snoop    );
DECLARE_DO_FUN( do_sober    );
DECLARE_DO_FUN( do_socials  );
DECLARE_DO_FUN( do_south    );
DECLARE_DO_FUN( do_southeast    );
DECLARE_DO_FUN( do_southwest    );
DECLARE_DO_FUN( do_speak        );
DECLARE_DO_FUN( do_speaker      );
DECLARE_DO_FUN( do_sset         );
DECLARE_DO_FUN( do_steacher     );
DECLARE_DO_FUN( do_stand    );
DECLARE_DO_FUN( do_starttourney );
DECLARE_DO_FUN( do_switch   );
DECLARE_DO_FUN( do_tell     );
DECLARE_DO_FUN( do_time     );
DECLARE_DO_FUN( do_timecmd  );
DECLARE_DO_FUN( do_track    );
DECLARE_DO_FUN( do_transfer );
DECLARE_DO_FUN( do_trust    );
DECLARE_DO_FUN( do_typo     );
DECLARE_DO_FUN( do_unfoldarea   );
DECLARE_DO_FUN( do_unload       );
DECLARE_DO_FUN( do_unhell   );
DECLARE_DO_FUN( do_unlock   );
DECLARE_DO_FUN( do_unsilence    );
DECLARE_DO_FUN( do_up       );
DECLARE_DO_FUN( do_uptime   );
DECLARE_DO_FUN( do_users    );
DECLARE_DO_FUN( do_visible  );
DECLARE_DO_FUN( do_vnums    );
DECLARE_DO_FUN( do_vsearch  );
DECLARE_DO_FUN( do_wake     );
DECLARE_DO_FUN( do_wear     );
DECLARE_DO_FUN( do_weather  );
DECLARE_DO_FUN( do_webserve     );
DECLARE_DO_FUN( do_west     );
DECLARE_DO_FUN( do_where    );
DECLARE_DO_FUN( do_whisper      );
DECLARE_DO_FUN( do_who      );
DECLARE_DO_FUN( do_whois    );
DECLARE_DO_FUN( do_wizhelp  );
DECLARE_DO_FUN( do_wizlist  );
DECLARE_DO_FUN( do_wizlock  );
DECLARE_DO_FUN( do_yell     );
DECLARE_DO_FUN( do_xname        );
DECLARE_DO_FUN( do_zones    );

/* mob prog stuff */
DECLARE_DO_FUN( do_mpmonitor    );
DECLARE_DO_FUN( do_mpvar        );
DECLARE_DO_FUN( do_mpwait       );
DECLARE_DO_FUN( do_mpadd        );
DECLARE_DO_FUN( do_mpsub        );
DECLARE_DO_FUN( do_mpblast      );
DECLARE_DO_FUN( do_mpcycle      );
DECLARE_DO_FUN( do_mpteamgain   );
DECLARE_DO_FUN( do_mprmflag     );
DECLARE_DO_FUN( do_mpreset      );
DECLARE_DO_FUN( do_mprat        );
DECLARE_DO_FUN( do_mp_damage    );
DECLARE_DO_FUN( do_mp_restore   );
DECLARE_DO_FUN( do_mp_open_passage );
DECLARE_DO_FUN( do_mp_close_passage );
DECLARE_DO_FUN( do_mp_practice  );
DECLARE_DO_FUN( do_mp_slay      );
DECLARE_DO_FUN( do_mpadvance    );
DECLARE_DO_FUN( do_mpasound     );
DECLARE_DO_FUN( do_mpat         );
DECLARE_DO_FUN( do_mpdream  );
DECLARE_DO_FUN( do_mprunspec    );
DECLARE_DO_FUN( do_mpecho       );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat     );
DECLARE_DO_FUN( do_mpedit       );
DECLARE_DO_FUN( do_mrange       );
DECLARE_DO_FUN( do_opedit       );
DECLARE_DO_FUN( do_orange       );
DECLARE_DO_FUN( do_rpedit       );
DECLARE_DO_FUN( do_mpforce      );
DECLARE_DO_FUN( do_mpinvis  );
DECLARE_DO_FUN( do_mpgoto       );
DECLARE_DO_FUN( do_mpjunk       );
DECLARE_DO_FUN( do_mpkill       );
DECLARE_DO_FUN( do_mpset        );
DECLARE_DO_FUN( do_mpmload      );
DECLARE_DO_FUN( do_mpmset   );
DECLARE_DO_FUN( do_mpnothing    );
DECLARE_DO_FUN( do_mpoload      );
DECLARE_DO_FUN( do_mposet   );
DECLARE_DO_FUN( do_mppurge      );
DECLARE_DO_FUN( do_mpstat       );
DECLARE_DO_FUN( do_opstat       );
DECLARE_DO_FUN( do_rpstat       );
DECLARE_DO_FUN( do_mptransfer   );
DECLARE_DO_FUN( do_mpapply  );
DECLARE_DO_FUN( do_mpapplyb     );
DECLARE_DO_FUN( do_mppkset  );
DECLARE_DO_FUN( do_mpgain   );

/*
    OS-dependent declarations.
    These are all very standard library functions,
     but some systems have incomplete or non-ansi header files.
*/
#if defined(_AIX)
    char*   crypt       args( ( const char* key, const char* salt ) );
#endif

#if defined(apollo)
    int atoi        args( ( const char* string ) );
    void*   calloc      args( ( unsigned nelem, size_t size ) );
    char*   crypt       args( ( const char* key, const char* salt ) );
#endif

#if defined(hpux)
    char*   crypt       args( ( const char* key, const char* salt ) );
#endif

#if defined(interactive)
#endif

#if defined(linux)
    char*   crypt       args( ( const char* key, const char* salt ) );
#endif

#if defined(MIPS_OS)
    char*   crypt       args( ( const char* key, const char* salt ) );
#endif

#if defined(NeXT)
    char*   crypt       args( ( const char* key, const char* salt ) );
#endif

#if defined(sequent)
    char*   crypt       args( ( const char* key, const char* salt ) );
    int fclose      args( ( FILE* stream ) );
    int fprintf     args( ( FILE* stream, const char* format, ... ) );
    int fread       args( ( void* ptr, int size, int n, FILE* stream ) );
    int fseek       args( ( FILE* stream, long offset, int ptrname ) );
    void    perror      args( ( const char* s ) );
    int ungetc      args( ( int c, FILE* stream ) );
#endif

#if defined(sun)
char*   crypt       args( ( const char* key, const char* salt ) );
int fclose      args( ( FILE* stream ) );
int fprintf     args( ( FILE* stream, const char* format, ... ) );
/*
    #if     defined(SYSV)
*/
size_t  fread       args( ( void* ptr, size_t size, size_t n,
                            /*              FILE *stream ) );
                                #else
                                int fread       args( ( void *ptr, int size, int n, FILE *stream ) );
                                #endif
                            */
                            int fseek       args( ( FILE* stream, long offset, int ptrname ) );
                            void    perror      args( ( const char* s ) );
                            int ungetc      args( ( int c, FILE* stream ) );
#endif

#if defined(ultrix)
    char*   crypt       args( ( const char* key, const char* salt ) );
#endif

/*
    The crypt(3) function is not available on some operating systems.
    In particular, the U.S. Government prohibits its export from the
        United States to foreign countries.
    Turn on NOCRYPT to keep passwords in plain text.
*/
#if defined(NOCRYPT)
    #define crypt(s1, s2)   (s1)
#endif


/*
    Data files used by the server.

    AREA_LIST contains a list of areas to boot.
    All files are read in completely at bootup.
    Most output files (bug, idea, typo, shutdown) are append-only.

    The NULL_FILE is held open so that we have a stream handle in reserve,
        so players can go ahead and telnet to all the other descriptors.
    Then we close it whenever we need to open a file (e.g. a save file).
*/
#define LOG_DIR         "../log/"       /* Log files                    */
#define PLAYER_DIR  "../player/"    /* Player files         */
#define BACKUP_DIR  "../backup/"    /* Backup Player files      */
#define GOD_DIR     "../gods/"  /* God Info Dir         */
#define BOARD_DIR   "../boards/"    /* Board data dir       */
#define EQ_DIR          "../save_eq/"   /* SAVE_EQ dir                  */
#define ARENA_DIR       "../arena/"     /* For Arena Templates          */
#define BOT_DIR         "../bots/"      /* Bot Files                    */
#define BUILD_DIR       "../building/"  /* Online building save dir     */
#define SYSTEM_DIR  "../system/"    /* Main system files        */
#define BACKTRACE_DIR "../backtraces/" /* Backtraces                   */
#define PROG_DIR    "mudprogs/" /* MUDProg files        */
#define CORPSE_DIR  "../corpses/"   /* Corpses          */
#ifdef WIN32
    #define NULL_FILE "nul"       /* To reserve one stream        */
#else
    #define NULL_FILE "/dev/null" /* To reserve one stream        */
#endif
#define UPTIME_PATH "/usr/bin/uptime" /* Uptime Program Location    */

#define COPYOVER_FILE  SYSTEM_DIR "copyover.dat"     /* for warm reboots     */
#define EXE_FILE       "../src/avp"            /* executable path  */
#define EXE2_FILE      "../src/avp"            /* executable path2  */

#define AREA_LIST   "area.lst"  /* List of areas        */
#define BAN_LIST        "ban.lst"       /* List of bans                 */
#define MP_LIST         "mp.list"
#define ARENA_LIST      "arena.lst"     /* For arenas                   */
#define BOT_LIST        "bots.lst"      /* For bots                     */
#define GOD_LIST    "gods.lst"  /* List of gods         */
#define XNAME_LIST  "xname.dat"


#define BOARD_FILE  "boards.txt"        /* For bulletin boards   */
#define SHUTDOWN_FILE   "shutdown.txt"      /* For 'shutdown'    */

#define RIPSCREEN_FILE  SYSTEM_DIR "mudrip.rip"
#define RIPTITLE_FILE   SYSTEM_DIR "mudtitle.rip"
#define ANSITITLE_FILE  SYSTEM_DIR "mudtitle.ans"
#define ASCTITLE_FILE   SYSTEM_DIR "mudtitle.asc"
#define BOOTLOG_FILE    SYSTEM_DIR "boot.txt"      /* Boot up error file  */
#define BUG_FILE        SYSTEM_DIR "bugs.txt"      /* For bug( )          */
#define COMPLAINT_FILE  SYSTEM_DIR "complaint.txt" /* For 'complain'      */
#define IDEA_FILE       SYSTEM_DIR "ideas.txt"     /* For 'idea'          */
#define TYPO_FILE       SYSTEM_DIR "typos.txt"     /* For 'typo'          */
#define PBUG_FILE       SYSTEM_DIR "pbugs.txt"     /* For 'bug' command   */
#define LOG_FILE        SYSTEM_DIR "log.txt"       /* For talking in logged rooms */
#define WIZLIST_FILE    SYSTEM_DIR "WIZLIST"       /* Wizlist             */
#define WHO_FILE        SYSTEM_DIR "WHO"           /* Who output file     */
#define WEBWHO_FILE     SYSTEM_DIR "WEBWHO"        /* WWW Who output file */
#define REQUEST_PIPE    SYSTEM_DIR "REQUESTS"      /* Request FIFO        */
#define SKILL_FILE      SYSTEM_DIR "skills.dat"    /* Skill table         */
#define HERB_FILE       SYSTEM_DIR "herbs.dat"     /* Herb table          */
#define SOCIAL_FILE     SYSTEM_DIR "socials.dat"   /* Socials             */
#define COMMAND_FILE    SYSTEM_DIR "commands.dat"  /* Commands            */
#define USAGE_FILE      SYSTEM_DIR "usage.txt"     /* How many people are on 
                                                      every half hour - trying to
                                                      determine best reboot time */

#define TEMP_FILE   SYSTEM_DIR "charsave.tmp" /* More char save protect */

                            /*
                                Our function prototypes.
                                One big lump ... this is every function in Merc.
                            */
#define CD  CHAR_DATA
#define MID MOB_INDEX_DATA
#define OD  OBJ_DATA
#define OID OBJ_INDEX_DATA
#define RID ROOM_INDEX_DATA
#define SF  SPEC_FUN
#define BD  BOARD_DATA
#define EDD EXTRA_DESCR_DATA
#define RD  RESET_DATA
#define ED  EXIT_DATA
#define ST  SOCIALTYPE
#define DE  DEITY_DATA
#define SK  SKILLTYPE

/* act_comm.c */
void    sound_to_room( ROOM_INDEX_DATA* room, char* argument );
bool    circle_follow   args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void    add_follower    args( ( CHAR_DATA* ch, CHAR_DATA* master ) );
void    stop_follower   args( ( CHAR_DATA* ch ) );
void    die_follower    args( ( CHAR_DATA* ch ) );
bool    is_same_group   args( ( CHAR_DATA* ach, CHAR_DATA* bch ) );
void    send_rip_screen args( ( CHAR_DATA* ch ) );
void    send_rip_title  args( ( CHAR_DATA* ch ) );
void    send_ansi_title args( ( CHAR_DATA* ch ) );
void    send_ascii_title args( ( CHAR_DATA* ch ) );
void    to_channel  args( ( const char* argument, int channel, const char* verb, sh_int level ) );
bool    knows_language  args( ( CHAR_DATA* ch, EXT_BV language, CHAR_DATA* cch ) );
bool    can_learn_lang  args( ( CHAR_DATA* ch, int language ) );
int     countlangs      args( ( int languages ) );
char*   translate       args( ( CHAR_DATA* ch, CHAR_DATA* victim,
        const char* argument ) );
void    send_to_buffer  args( ( const char* txt, DESCRIPTOR_DATA* d ) );
char*   obj_short   args( ( OBJ_DATA* obj ) );
void info               args( ( CHAR_DATA* ch, int level, char* message, ... ) );
char* act_string       args( ( const char* format, CHAR_DATA* to, CHAR_DATA* ch, const void* arg1, const void* arg2 ) );
void send_monitor       args( ( CHAR_DATA* ignore, char* msg ) );
void radio_broadcast    args( ( CHAR_DATA* ch, char* message ) );
void sound_radius       args( ( char* msg, ROOM_INDEX_DATA* room, int radius, CHAR_DATA* ignore ) );
void sound_radius_1     args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room, char* buf, int range, int dir ) );
void sound_radius_2     args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room, int range ) );
int count_friends       args( ( CHAR_DATA* ch ) );
int _count_friends      args( ( ROOM_INDEX_DATA* room, CHAR_DATA* ch ) );
void motion_ping        args( ( int x, int y, int z, AREA_DATA* area, CHAR_DATA* ignore ) );
bool block_profane      args( ( CHAR_DATA* ch ) );
void profanity_filter   args( ( char* arg, char* out ) );
bool profanity_check    args( ( char* arg ) );
void send_sound         args( ( char* buf, ROOM_INDEX_DATA* room, int volume, CHAR_DATA* ch ) );
void update_swarm       args( ( void ) );
int count_followers     args( ( CHAR_DATA* ch, int type, bool inroom ) );
void team_xpgain        args( ( int race, int exp ) );

/* act_info.c */
int     get_door            args( ( char* arg ) );
char*   format_obj_to_char  args( ( OBJ_DATA* obj, CHAR_DATA* ch, bool fShort ) );
void    show_list_to_char   args( ( OBJ_DATA* list, CHAR_DATA* ch, bool fShort, bool fShowNothing ) );
char*   conv_hcolor         args( ( char* msg ) );
void    HTML_Who            args( ( void ) );
void    HTML_Allrooms       args( ( void ) );
void    HTML_Objstats       args( ( void ) );
void    examine_obj         args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
void    HTML_Ammostats      args( ( void ) );
HELP_DATA* get_help         args( ( CHAR_DATA* ch, char* argument ) );
void    send_skill_store    args( ( CHAR_DATA* ch, DESCRIPTOR_DATA* d ) );
int     get_store_skill     args( ( CHAR_DATA* ch, int num ) );

/* act_move.c */
void    clear_vrooms    args( ( void ) );
ED*     find_door   args( ( CHAR_DATA* ch, char* arg, bool quiet ) );
ED*     get_exit    args( ( ROOM_INDEX_DATA* room, sh_int dir ) );
ED*     get_exit_to args( ( ROOM_INDEX_DATA* room, sh_int dir, int vnum ) );
ED*     get_exit_num    args( ( ROOM_INDEX_DATA* room, sh_int count ) );
ch_ret  move_char       args( ( CHAR_DATA* ch, EXIT_DATA* pexit, int edir, int fall ) );
void    teleport    args( ( CHAR_DATA* ch, int room, int flags ) );
sh_int  encumbrance args( ( CHAR_DATA* ch, sh_int move ) );
bool    will_fall   args( ( CHAR_DATA* ch, int fall ) );
int     wherehome       args( ( CHAR_DATA* ch ) );
char*   rev_exit        args( ( sh_int vdir ) );
void    abort_follow    args( ( CHAR_DATA* ch ) );
void    hive_message    args( ( CHAR_DATA* ch, char* msg ) );
void    cloak_message   args( ( CHAR_DATA* ch, char* msg ) );
char*   main_exit   args( ( sh_int vdir ) );
ROOM_INDEX_DATA* generate_exit  args( ( ROOM_INDEX_DATA* in_room, EXIT_DATA** pexit ) );
int     flip_dir    args( ( int dir ) );
void    set_bexit_flag  args( ( EXIT_DATA* pexit, int flag ) );
void    player_ping     args( ( CHAR_DATA* ch, CHAR_DATA* ignore ) );

/* act_obj.c */
obj_ret damage_obj      args( ( OBJ_DATA* obj, int damage ) );
sh_int  get_obj_resistance args( ( OBJ_DATA* obj ) );
void    obj_fall    args( ( OBJ_DATA* obj, bool through ) );
void    save_equip_room args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room ) );
void    detonate_traps args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room ) );
void    equip_lag args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
void    unequip_lag args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
void    load_equip_rooms args( ( void ) );
void    wear_obj    args( ( CHAR_DATA* ch, OBJ_DATA* obj, bool fReplace, sh_int wear_bit ) );
int     count_traps args( ( ROOM_INDEX_DATA* room ) );
void    clear_traps args( ( ROOM_INDEX_DATA* room ) );

/* act_wiz.c */
void              close_area    args( ( AREA_DATA* pArea ) );
RID*    find_location   args( ( CHAR_DATA* ch, char* arg ) );
void    echo_to_room    args( ( sh_int AT_COLOR, ROOM_INDEX_DATA* room, char* argument ) );
void    echo_to_all args( ( sh_int AT_COLOR, char* argument,
        sh_int tar ) );
void    get_reboot_string args( ( void ) );
struct tm* update_time  args( ( struct tm* old_time ) );
void    free_social args( ( SOCIALTYPE* social ) );
void    add_social  args( ( SOCIALTYPE* social ) );
void    free_command    args( ( CMDTYPE* command ) );
void    unlink_command  args( ( CMDTYPE* command ) );
void    add_command args( ( CMDTYPE* command ) );
void    autosave    args( ( void ) );
void    advance_all     args( ( CHAR_DATA* ch, CHAR_DATA* victim, int level ) );

/* arena.c */
void    reset_vote  args( ( void ) );
void    log_vote    args( ( CHAR_DATA* ch, bool yea ) );
void    process_vote args( ( void ) );
void    init_vote   args( ( void ) );
void    update_arena    args( ( void ) );
void    load_arenas args( ( void ) );
void    update_votes    args( ( void ) );

/* backtrace.c */
void capturebacktrace args( ( const char* type ) );

/* boards.c */
void    load_boards args( ( void ) );
BD*     get_board   args( ( OBJ_DATA* obj ) );
void    free_note   args( ( NOTE_DATA* pnote ) );

/* bots.c */
void    bot_load    args( ( BOT_DATA* bot ) );
void    bot_unload  args( ( BOT_DATA* bot ) );
void    load_bots   args( ( void ) );
void    bot_update  args( ( void ) );

/* build.c */
char*   conv_tag        args( ( char* str ) );
char*   convert_sp      args( ( char* str ) );
char*   flag_string args( ( EXT_BV* bitvector, char* const flagarray[], int range ) );
char*   old_flag_string args( ( int bitvector, char* const flagarray[] ) );
int get_mpflag  args( ( char* flag ) );
int get_dir     args( ( char* txt  ) );
char*   strip_cr    args( ( char* str  ) );
bool    lock_rprog      args( ( ROOM_INDEX_DATA* room, char* prog, char* argument, int mptype ) );
ROOM_INDEX_DATA* bw_create args( ( CHAR_DATA* ch ) );
int     get_rflag   args( ( char* flag ) );
void    hflag_toggle    args( ( ROOM_INDEX_DATA* room ) );

/* greet.c */
char*        make_greet_desc  args( ( CHAR_DATA* ch, CHAR_DATA* looker ) );
char*        g_name           args( ( CHAR_DATA* ch, CHAR_DATA* looker ) );
bool         knows_player     args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void         assign_gname     args( ( CHAR_DATA* ch ) );

/* comm.c */
void    close_socket    args( ( DESCRIPTOR_DATA* dclose, bool force ) );
void    write_to_buffer args( ( DESCRIPTOR_DATA* d, const char* txt,
        int length ) );
void    write_to_pager  args( ( DESCRIPTOR_DATA* d, const char* txt,
        int length ) );
void    send_to_char    args( ( const char* txt, CHAR_DATA* ch ) );
void    send_to_char_color  args( ( const char* txt, CHAR_DATA* ch ) );
void    send_to_pager   args( ( const char* txt, CHAR_DATA* ch ) );
void    send_to_pager_color args( ( const char* txt, CHAR_DATA* ch ) );
void    set_char_color  args( ( sh_int AType, CHAR_DATA* ch ) );
void    set_pager_color args( ( sh_int AType, CHAR_DATA* ch ) );
void    ch_printf   args( ( CHAR_DATA* ch, char* fmt, ... ) );
void    pager_printf    args( ( CHAR_DATA* ch, char* fmt, ... ) );
void    act     args( ( sh_int AType, const char* format, CHAR_DATA* ch,
        const void* arg1, const void* arg2, int type ) );
void    copyover_recover args( ( void ) );
bool    read_from_descriptor    args( ( DESCRIPTOR_DATA* d ) );
bool    write_to_descriptor     args( ( int desc, char* txt, int length ) );
bool    match_log       args( ( const char* str, ... ) );
bool    open_match      args( ( void ) );
bool    close_match     args( ( void ) );
void    emergency_copyover  args( ( void ) );
void    write_menu_to_desc  args( ( DESCRIPTOR_DATA* d ) );
void    emergency_arm   args( ( void ) );

/* reset.c */
RD*     make_reset  args( ( char letter, int extra, int arg1, int arg2, int arg3 ) );
RD*     add_reset   args( ( AREA_DATA* tarea, char letter, int extra, int arg1, int arg2, int arg3 ) );
RD*     place_reset args( ( AREA_DATA* tarea, char letter, int extra, int arg1, int arg2, int arg3 ) );
void    reset_area  args( ( AREA_DATA* pArea ) );

/* db.c */
void    regenerate_limbo args( ( void ) );
void    save_sysdata    args( ( SYSTEM_DATA sys ) );
void    show_file   args( ( CHAR_DATA* ch, char* filename ) );
char*   str_dup     args( ( char const* str ) );
void    boot_db     args( ( bool fCopyOver ) );
void    area_update args( ( void ) );
void    add_char    args( ( CHAR_DATA* ch ) );
CD*     create_mobile   args( ( MOB_INDEX_DATA* pMobIndex ) );
OD*     create_object   args( ( OBJ_INDEX_DATA* pObjIndex, int level ) );
void    clear_char  args( ( CHAR_DATA* ch ) );
void    free_char   args( ( CHAR_DATA* ch ) );
char*   get_extra_descr args( ( const char* name, EXTRA_DESCR_DATA* ed ) );
MID*    get_mob_index   args( ( int vnum ) );
OID*    get_obj_index   args( ( int vnum ) );
RID*    get_room_index  args( ( int vnum ) );
char    fread_letter    args( ( FILE* fp ) );
int fread_number    args( ( FILE* fp ) );
/* -------------------------------------------------------- */
EXT_BV  fread_bitvector args( ( FILE* fp ) );
void    fwrite_bitvector args( ( EXT_BV* bits, FILE* fp ) );
char*   print_bitvector args( ( EXT_BV* bits ) );
/* -------------------------------------------------------- */
char*   fread_string    args( ( FILE* fp ) );
char*   fread_string_nohash args( ( FILE* fp ) );
void    fread_to_eol    args( ( FILE* fp ) );
char*   fread_word  args( ( FILE* fp ) );
char*   fread_line  args( ( FILE* fp ) );
int number_fuzzy    args( ( int number ) );
int number_range    args( ( int from, int to ) );
int number_percent  args( ( void ) );
int number_door args( ( void ) );
int number_bits args( ( int width ) );
int number_mm   args( ( void ) );
int dice        args( ( int number, int size ) );
int interpolate args( ( int level, int value_00, int value_32 ) );
void    smash_tilde args( ( char* str ) );
void    hide_tilde  args( ( char* str ) );
char*   show_tilde  args( ( char* str ) );
bool    str_cmp     args( ( const char* astr, const char* bstr ) );
bool    str_prefix  args( ( const char* astr, const char* bstr ) );
bool    str_infix   args( ( const char* astr, const char* bstr ) );
bool    str_suffix  args( ( const char* astr, const char* bstr ) );
char*   capitalize  args( ( const char* str ) );
char*   strlower    args( ( const char* str ) );
char*   strupper    args( ( const char* str ) );
char*   aoran       args( ( const char* str ) );
void    append_file args( ( CHAR_DATA* ch, char* file, char* str ) );
void    append_to_file  args( ( char* file, char* str ) );
void    bug     args( ( const char* str, ... ) );
void    log_string_plus args( ( const char* str, sh_int log_type, sh_int level ) );
RID*    make_room   args( ( int vnum ) );
OID*    make_object args( ( int vnum, int cvnum, char* name ) );
MID*    make_mobile args( ( int vnum, int cvnum, char* name ) );
ED*     make_exit   args( ( ROOM_INDEX_DATA* pRoomIndex, ROOM_INDEX_DATA* to_room, sh_int door ) );
void    add_help    args( ( HELP_DATA* pHelp ) );
void    fix_area_exits  args( ( AREA_DATA* tarea ) );
void    load_area_file  args( ( AREA_DATA* tarea, char* filename ) );
void    randomize_exits args( ( ROOM_INDEX_DATA* room, sh_int maxdir ) );
void    make_wizlist    args( ( void ) );
void    tail_chain  args( ( void ) );
bool    delete_room     args( ( ROOM_INDEX_DATA* room ) );
bool    delete_obj      args( ( OBJ_INDEX_DATA* obj ) );
bool    delete_mob      args( ( MOB_INDEX_DATA* mob ) );
void    sort_area   args( ( AREA_DATA* pArea, bool proto ) );
int     file_size       args( ( char* buf ) );
void    deploy_map  args( ( void ) );
void     memory_cleanup args( ( void ) );
void    map_reset_all   args( ( void ) );
void    map_apply_coords    args( ( ROOM_INDEX_DATA* room, int x, int y, int z ) );
void    map_reset_alert args( ( void ) );
void    tns_cleanup args( ( void ) );
void    fold_module args( ( void ) );

/* build.c */
void    start_editing   args( ( CHAR_DATA* ch, char* data ) );
void    stop_editing    args( ( CHAR_DATA* ch ) );
void    edit_buffer args( ( CHAR_DATA* ch, char* argument ) );
char*   copy_buffer args( ( CHAR_DATA* ch ) );
bool    can_rmodify args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room ) );
bool    can_omodify args( ( CHAR_DATA* ch, OBJ_DATA* obj  ) );
bool    can_mmodify args( ( CHAR_DATA* ch, CHAR_DATA* mob ) );
bool    can_medit   args( ( CHAR_DATA* ch, MOB_INDEX_DATA* mob ) );
void    free_reset  args( ( AREA_DATA* are, RESET_DATA* res ) );
void    free_area   args( ( AREA_DATA* are ) );
void    assign_area args( ( CHAR_DATA* ch ) );
EDD*    SetRExtra   args( ( ROOM_INDEX_DATA* room, char* keywords ) );
bool    DelRExtra   args( ( ROOM_INDEX_DATA* room, char* keywords ) );
EDD*    SetOExtra   args( ( OBJ_DATA* obj, char* keywords ) );
bool    DelOExtra   args( ( OBJ_DATA* obj, char* keywords ) );
EDD*    SetOExtraProto  args( ( OBJ_INDEX_DATA* obj, char* keywords ) );
bool    DelOExtraProto  args( ( OBJ_INDEX_DATA* obj, char* keywords ) );
void    fold_area   args( ( AREA_DATA* tarea, char* filename, bool install ) );
int get_otype   args( ( char* type ) );
int get_atype   args( ( char* type ) );
int get_aflag   args( ( char* flag ) );
int get_oflag   args( ( char* flag ) );
int get_wflag   args( ( char* flag ) );

/* fight.c */
int     xp_compute      args( ( CHAR_DATA* gch, CHAR_DATA* victim ) );
int max_fight   args( ( CHAR_DATA* ch ) );
void    violence_update args( ( void ) );
ch_ret  multi_hit   args( ( CHAR_DATA* ch, CHAR_DATA* victim, int dt ) );
sh_int  ris_damage      args( ( CHAR_DATA* ch, sh_int dam, int ris, bool apply ) );
ch_ret  damage          args( ( CHAR_DATA* ch, CHAR_DATA* victim, int dam, int dt ) );
void    update_pos  args( ( CHAR_DATA* victim ) );
void    check_killer    args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void    check_attacker  args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void    death_cry   args( ( CHAR_DATA* ch ) );
void    stop_hunting    args( ( CHAR_DATA* ch ) );
void    stop_hating args( ( CHAR_DATA* ch ) );
void    stop_fearing    args( ( CHAR_DATA* ch ) );
void    start_hunting   args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void    start_hating    args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void    start_fearing   args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    is_hunting  args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    is_hating   args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    is_fearing  args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    is_safe     args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    is_safe_nm  args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    legal_loot  args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    check_illegal_pk args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void    raw_kill        args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    in_arena    args( ( CHAR_DATA* ch ) );
bool    is_ranged   args( ( int type ) );
void    remove_cover args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
bool    check_rescue args( ( CHAR_DATA* ch ) );
void    ignite_target args( ( CHAR_DATA* ch, CHAR_DATA* victim, int duration ) );
bool    delete_player args( ( char* name ) );
int     armor_status    args( ( CHAR_DATA* ch, int mode ) );
int     default_weapon_mode args( ( OBJ_DATA* obj ) );
int     snipe_direction args( ( CHAR_DATA* ch, CHAR_DATA* vic, char* arg, OBJ_DATA* wield, int dir, int dual ) );
bool    mob_reload      args( ( CHAR_DATA* ch, OBJ_DATA* weapon ) );
void    clear_effects   args( ( CHAR_DATA* ch ) );
void    weapon_echo     args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
void    auto_eject      args( ( CHAR_DATA* ch, OBJ_DATA* wield ) );
void    showammo_option args( ( CHAR_DATA* ch, OBJ_DATA* wield ) );
void    clear_shortage  args( ( CHAR_DATA* victim ) );
int     stamina_penalty args( ( CHAR_DATA* ch, int chance ) );
int     char_acc_modify args( ( CHAR_DATA* ch, int ac ) );
void    miss_message    args( ( CHAR_DATA* ch, CHAR_DATA* victim, int dt, int at ) );
int     apply_armor_skill   args( ( CHAR_DATA* ch, OBJ_DATA* obj, int odam ) );
void    short_target    args( ( CHAR_DATA* ch, CHAR_DATA* victim, int duration ) );
void    radius_gain     args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void    manage_streak   args( ( CHAR_DATA* ch, CHAR_DATA* rch, bool reset ) );
void    xp_radius_1     args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room, CHAR_DATA* victim, int xp, int range ) );
void    xp_radius_2     args( ( ROOM_INDEX_DATA* room, int range ) );
void    res_radius_1    args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room, CHAR_DATA* victim, int xp, int range ) );
void    res_radius_2    args( ( ROOM_INDEX_DATA* room, int range ) );
void    mob_weapon_set  args( ( CHAR_DATA* ch ) );
int     spray_direction args( ( CHAR_DATA* ch, OBJ_DATA* wield, int dir, int dual ) );
int     lob_direction   args( ( CHAR_DATA* ch, OBJ_DATA* obj, int dir, int range ) );
void    add_cover       args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
int     fire_weapon     args( ( CHAR_DATA* ch, OBJ_DATA* obj, int dir, int range ) );
bool    is_enemy        args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
int     total_pc_kills  args( ( CHAR_DATA* ch ) );
int     total_pc_killed args( ( CHAR_DATA* ch ) );
int     cdamage         args( ( CHAR_DATA* ch, CHAR_DATA* victim, int dam, bool silent ) );
bool    has_ammo        args( ( int type ) );

/* pfiles.c */
void    check_pfiles           args( ( time_t reset ) );
void    init_pfile_scan_time   args( ( void ) );

/* makeobjs.c */
void    make_corpse args( ( CHAR_DATA* ch, CHAR_DATA* killer ) );
void    make_blood  args( ( CHAR_DATA* ch ) );
void    make_bloodstain args( ( CHAR_DATA* ch ) );
OD*     make_scraps     args( ( OBJ_DATA* obj ) );
void    make_fire   args( ( ROOM_INDEX_DATA* in_room, sh_int timer ) );
OD*     create_money    args( ( int amount ) );
void    newbie_create   args( ( CHAR_DATA* ch, int type ) );

/* mapout.c */
void    Initilize_Map   args( ( int height ) );
void    Reset_Map       args( ( void ) );
void    Check_Exits_Ext args( ( ROOM_INDEX_DATA* room, int cX, int cY ) );
void    Check_Exits_Sim args( ( ROOM_INDEX_DATA* room, int cX, int cY ) );
int     get_room_type   args( ( ROOM_INDEX_DATA* room, int tmp ) );

/* misc.c */
void actiondesc args( ( CHAR_DATA* ch, OBJ_DATA* obj, void* vo ) );
void jedi_checks  args( ( CHAR_DATA* ch ) );
void jedi_bonus   args( ( CHAR_DATA* ch ) );
void sith_penalty args( ( CHAR_DATA* ch ) );
SENTRY_DATA* get_sentry args( ( OBJ_DATA* obj ) );
void update_sentry  args( ( void ) );
bool rem_sentry     args( ( OBJ_DATA* obj, CHAR_DATA* ch ) );
void rempc_sentry   args( ( CHAR_DATA* ch ) );
void reload_main    args( ( CHAR_DATA* ch, OBJ_DATA* obj, char* argument ) );
int  use_obj        args( ( CHAR_DATA* ch, char* argument, int mode ) );
void deploy_fire    args( ( SENTRY_DATA* gun, int dir ) );

/* mud_comm.c */
char*   mprog_type_to_name  args( ( int type ) );

/* mud_prog.c */
#ifdef DUNNO_STRSTR
char*   strstr                  args ( ( const char* s1, const char* s2 ) );
#endif

void    mprog_wordlist_check    args ( ( char* arg, CHAR_DATA* mob,
        CHAR_DATA* actor, OBJ_DATA* object,
        void* vo, int type ) );
void    mprog_percent_check     args ( ( CHAR_DATA* mob, CHAR_DATA* actor,
        OBJ_DATA* object, void* vo,
        int type ) );
void    mprog_act_trigger       args ( ( char* buf, CHAR_DATA* mob,
        CHAR_DATA* ch, OBJ_DATA* obj,
        void* vo ) );
void    mprog_bribe_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
        int amount ) );
void    mprog_entry_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_give_trigger      args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
        OBJ_DATA* obj ) );
void    mprog_greet_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_fight_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_hitprcnt_trigger  args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_death_trigger     args ( ( CHAR_DATA* killer, CHAR_DATA* mob ) );
void    mprog_random_trigger    args ( ( CHAR_DATA* mob ) );
void    mprog_speech_trigger    args ( ( char* txt, CHAR_DATA* mob ) );
void    mprog_script_trigger    args ( ( CHAR_DATA* mob ) );
void    mprog_hour_trigger      args ( ( CHAR_DATA* mob ) );
void    mprog_time_trigger      args ( ( CHAR_DATA* mob ) );
void    progbug                 args( ( char* str, CHAR_DATA* mob ) );
void    rset_supermob       args( ( ROOM_INDEX_DATA* room ) );
void    release_supermob    args( ( ) );
void    mpsleep_update          args( ( ) );
void    mpsleep_inspect     args( ( CHAR_DATA* ch ) );
void    mprog_pulse_trigger args( ( CHAR_DATA* mob ) );
void    rprog_pulse_trigger args( ( CHAR_DATA* ch ) );

/* player.c */
void    set_title   args( ( CHAR_DATA* ch, char* title ) );
char*   reduce_ratio    args( ( int a, int b ) );
char*   drawbar         args( ( int bars, int curr, int max, char* cA, char* cB ) );
int     find_percent    args( ( int a, int b ) );
char*   drawlevel       args( ( CHAR_DATA* ch ) );
char*   get_sex         args( ( CHAR_DATA* ch ) );
int     make_percent    args( ( int cur, int max ) );

/* skills.c */
bool    check_illegal_psteal    args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    check_skill     args( ( CHAR_DATA* ch, char* command, char* argument ) );
void    learn_from_success  args( ( CHAR_DATA* ch, int sn ) );
void    learn_from_failure  args( ( CHAR_DATA* ch, int sn ) );
bool    check_parry     args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    check_dodge     args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    check_grip      args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void    disarm          args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void    trip            args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
int     ris_save                args( ( CHAR_DATA* ch, int chance, int ris ) );
int     ch_slookup              args( ( CHAR_DATA* ch, const char* name ) );
int     find_skill              args( ( CHAR_DATA* ch, const char* name, bool know ) );
int     find_weapon             args( ( CHAR_DATA* ch, const char* name, bool know ) );
int     find_tongue             args( ( CHAR_DATA* ch, const char* name, bool know ) );
int     skill_lookup            args( ( const char* name ) );
int     herb_lookup             args( ( const char* name ) );
int     personal_lookup         args( ( CHAR_DATA* ch, const char* name ) );
int     bsearch_skill           args( ( const char* name, int first, int top ) );
int     bsearch_skill_exact     args( ( const char* name, int first, int top ) );
SK*     get_skilltype           args( ( int sn ) );
void run_awareness              args( ( CHAR_DATA* first, CHAR_DATA* ch ) );
void echo_to_room_ignore        args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room, char* argument ) );

/* handler.c */
void    explode         args( ( OBJ_DATA* obj ) );
int get_exp     args( ( CHAR_DATA* ch, int ability ) );
int get_exp_worth   args( ( CHAR_DATA* ch ) );
int exp_level   args( ( sh_int level ) );
int     get_new_ac      args( ( CHAR_DATA* ch ) );
sh_int  get_trust   args( ( CHAR_DATA* ch ) );
sh_int  get_age     args( ( CHAR_DATA* ch ) );
sh_int  get_curr_str    args( ( CHAR_DATA* ch ) );
sh_int  get_curr_sta    args( ( CHAR_DATA* ch ) );
sh_int  get_curr_int    args( ( CHAR_DATA* ch ) );
sh_int  get_curr_rec    args( ( CHAR_DATA* ch ) );
sh_int  get_curr_bra    args( ( CHAR_DATA* ch ) );
sh_int  get_curr_per    args( ( CHAR_DATA* ch ) );
bool    can_take_proto  args( ( CHAR_DATA* ch ) );
int can_carry_n args( ( CHAR_DATA* ch ) );
int can_carry_w args( ( CHAR_DATA* ch ) );
bool    is_home         args( ( CHAR_DATA* ch ) );
bool    is_spectator    args( ( CHAR_DATA* ch ) );
bool    is_name     args( ( const char* str, char* namelist ) );
bool    is_name_prefix  args( ( const char* str, char* namelist ) );
bool    nifty_is_name   args( ( char* str, char* namelist ) );
bool    nifty_is_name_prefix args( ( char* str, char* namelist ) );
void    affect_modify   args( ( CHAR_DATA* ch, AFFECT_DATA* paf, bool fAdd ) );
void    affect_to_char  args( ( CHAR_DATA* ch, AFFECT_DATA* paf ) );
void    affect_remove   args( ( CHAR_DATA* ch, AFFECT_DATA* paf ) );
void    affect_strip    args( ( CHAR_DATA* ch, int sn ) );
bool    is_affected args( ( CHAR_DATA* ch, int sn ) );
void    affect_join args( ( CHAR_DATA* ch, AFFECT_DATA* paf ) );
void    char_from_room  args( ( CHAR_DATA* ch ) );
void    char_to_room    args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* pRoomIndex ) );
OD*     obj_to_char args( ( OBJ_DATA* obj, CHAR_DATA* ch ) );
void    obj_from_char   args( ( OBJ_DATA* obj ) );
int apply_ac    args( ( OBJ_DATA* obj, int iWear ) );
OD*     get_eq_char args( ( CHAR_DATA* ch, int iWear ) );
void    equip_char  args( ( CHAR_DATA* ch, OBJ_DATA* obj, int iWear ) );
void    unequip_char    args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
int count_obj_list  args( ( OBJ_INDEX_DATA* obj, OBJ_DATA* list ) );
void    obj_from_room   args( ( OBJ_DATA* obj ) );
OD*     obj_to_room args( ( OBJ_DATA* obj, ROOM_INDEX_DATA* pRoomIndex ) );
OD*     obj_to_obj  args( ( OBJ_DATA* obj, OBJ_DATA* obj_to ) );
void    obj_from_obj    args( ( OBJ_DATA* obj ) );
void    extract_obj args( ( OBJ_DATA* obj ) );
void    extract_exit    args( ( ROOM_INDEX_DATA* room, EXIT_DATA* pexit ) );
void    extract_room    args( ( ROOM_INDEX_DATA* room ) );
void    clean_room  args( ( ROOM_INDEX_DATA* room ) );
void    clean_obj   args( ( OBJ_INDEX_DATA* obj ) );
void    clean_mob   args( ( MOB_INDEX_DATA* mob ) );
void    clean_resets    args( ( AREA_DATA* tarea ) );
void    extract_char    args( ( CHAR_DATA* ch, bool fPull, bool menu ) );
CD*     get_char_room   args( ( CHAR_DATA* ch, char* argument ) );
CD*     get_char_world  args( ( CHAR_DATA* ch, char* argument ) );
CD*     get_char_room_full   args( ( CHAR_DATA* ch, char* argument ) );
CD*     get_char_world_full  args( ( CHAR_DATA* ch, char* argument ) );
OD*     get_obj_type    args( ( OBJ_INDEX_DATA* pObjIndexData ) );
OD*     get_obj_list    args( ( CHAR_DATA* ch, char* argument, OBJ_DATA* list ) );
OD*     get_obj_list_rev args( ( CHAR_DATA* ch, char* argument,
        OBJ_DATA* list ) );
OD*     get_obj_carry   args( ( CHAR_DATA* ch, char* argument ) );
OD*     get_obj_wear    args( ( CHAR_DATA* ch, char* argument ) );
OD*     get_obj_here    args( ( CHAR_DATA* ch, char* argument ) );
OD*     get_obj_world   args( ( CHAR_DATA* ch, char* argument ) );
int get_obj_number  args( ( OBJ_DATA* obj ) );
int get_obj_weight  args( ( OBJ_DATA* obj ) );
bool    room_is_dark    args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* pRoomIndex ) );
bool    if_equip_room   args( ( ROOM_INDEX_DATA* room ) );
bool    room_is_private args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* pRoomIndex ) );
/*CD      *room_is_dnd    args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );*/
OBJ_DATA* has_comlink  args( ( CHAR_DATA* ch ) );
OBJ_DATA* has_datapad  args( ( CHAR_DATA* ch ) );
bool    check_permit    args( ( CHAR_DATA* ch, int type ) );
bool    can_see     args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
bool    can_see_obj args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
bool    can_drop_obj    args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
char*   stripclr        args( ( char* text ) );
char*   item_type_name  args( ( OBJ_DATA* obj ) );
char*   affect_loc_name args( ( int location ) );
char*   affect_bit_name args( ( EXT_BV* vector ) );
char*   extra_bit_name  args( ( EXT_BV* extra_flags ) );
char*   magic_bit_name  args( ( EXT_BV* magic_flags ) );
ch_ret  check_for_trap  args( ( CHAR_DATA* ch, OBJ_DATA* obj, int flag ) );
ch_ret  check_room_for_traps args( ( CHAR_DATA* ch, int flag ) );
bool    is_trapped  args( ( OBJ_DATA* obj ) );
OD*     get_trap    args( ( OBJ_DATA* obj ) );
ch_ret  spring_trap     args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
void    name_stamp_stats args( ( CHAR_DATA* ch ) );
void    fix_char    args( ( CHAR_DATA* ch ) );
void    showaffect  args( ( CHAR_DATA* ch, AFFECT_DATA* paf ) );
void    set_cur_obj args( ( OBJ_DATA* obj ) );
bool    obj_extracted   args( ( OBJ_DATA* obj ) );
void    queue_extracted_obj args( ( OBJ_DATA* obj ) );
void    clean_obj_queue args( ( void ) );
void    set_cur_char    args( ( CHAR_DATA* ch ) );
bool    char_died   args( ( CHAR_DATA* ch ) );
void    queue_extracted_char    args( ( CHAR_DATA* ch, bool extract ) );
void    clean_char_queue    args( ( void ) );
void    add_timer   args( ( CHAR_DATA* ch, sh_int type, sh_int count, DO_FUN* fun, int value ) );
TIMER* get_timerptr    args( ( CHAR_DATA* ch, sh_int type ) );
sh_int  get_timer   args( ( CHAR_DATA* ch, sh_int type ) );
void    extract_timer   args( ( CHAR_DATA* ch, TIMER* timer ) );
void    remove_timer    args( ( CHAR_DATA* ch, sh_int type ) );
bool    in_soft_range   args( ( CHAR_DATA* ch, AREA_DATA* tarea ) );
bool    in_hard_range   args( ( CHAR_DATA* ch, AREA_DATA* tarea ) );
bool    chance      args( ( CHAR_DATA* ch, sh_int percent ) );
bool    chance_attrib   args( ( CHAR_DATA* ch, sh_int percent, sh_int attrib ) );
OD*     clone_object    args( ( OBJ_DATA* obj ) );
void    split_obj   args( ( OBJ_DATA* obj, int num ) );
void    separate_obj    args( ( OBJ_DATA* obj ) );
bool    empty_obj   args( ( OBJ_DATA* obj, OBJ_DATA* destobj,
        ROOM_INDEX_DATA* destroom ) );
OD*     find_obj    args( ( CHAR_DATA* ch, char* argument,
        bool carryonly ) );
bool    ms_find_obj args( ( CHAR_DATA* ch ) );
void    worsen_mental_state args( ( CHAR_DATA* ch, int mod ) );
void    better_mental_state args( ( CHAR_DATA* ch, int mod ) );
void    modify_skill    args( ( CHAR_DATA* ch, int sn, int mod, bool fAdd ) );
char    LetterConversion ( char chLetter );
bool    player_exist    args( ( char* name ) );
int     get_sprox       args( ( int ax, int ay, int az, int bx, int by, int bz ) );
void  echo_to_room_dnr  args( ( int ecolor, ROOM_INDEX_DATA* room,  char* argument ) );
int     get_max_rounds  args( ( OBJ_DATA* obj ) );
void    skill_power     args( ( CHAR_DATA* ch, int gsn, int amount ) );
void    drop_morale     args( ( CHAR_DATA* ch, int amount ) );
bool    contains_explosive args( ( OBJ_DATA* obj, int antiloop ) );
bool    checkclr    args( ( char* text, char* token ) );
int     get_max_ap  args( ( CHAR_DATA* ch ) );
bool    file_exist  args( ( char* name ) );
void    clear_variables args( ( AREA_DATA* area ) );
bool    can_hive_room args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room ) );
int     count_players args( ( void ) );
int     get_max_morale  args( ( CHAR_DATA* ch ) );
int     get_max_mp  args( ( CHAR_DATA* ch ) );
int     get_max_resin   args( ( CHAR_DATA* ch ) );
char*   get_rank    args( ( int race, int level ) );
int     get_max_teamkill    args( ( CHAR_DATA* ch ) );
ROOM_INDEX_DATA* get_obj_room   args( ( OBJ_DATA* obj ) );
void    clear_timers    args( ( CHAR_DATA* ch ) );
int     get_dark_range  args( ( CHAR_DATA* ch ) );
CHAR_DATA* get_char_far_room    args( ( CHAR_DATA* ch, ROOM_INDEX_DATA* room, char* argument ) );
int     break_exit      args( ( ROOM_INDEX_DATA* room, EXIT_DATA* pexit, int roomblast ) );
void    set_variable    args( ( AREA_DATA* area, char* name, int value ) );
int     get_variable    args( ( AREA_DATA* area, char* name ) );
sh_int  get_mod_per     args( ( CHAR_DATA* ch ) );
size_t  strlcpy         args( (char * __restrict dst, const char * __restrict src, size_t dsize) );
size_t  strlcat         args( (char *dst, const char *src, size_t siz) );

/* interp.c */
bool    check_pos   args( ( CHAR_DATA* ch, sh_int position ) );
void    interpret   args( ( CHAR_DATA* ch, char* argument, bool is_order ) );
bool    is_number   args( ( char* arg ) );
bool    is_number_sym   args( ( char* arg ) );
int number_argument args( ( char* argument, char* arg ) );
char*   one_argument    args( ( const char* argument, char* arg_first ) );
char*   one_argument2   args( ( const char* argument, char* arg_first ) );
char*   one_argument_sc args( ( const char* argument, char* arg_first ) );
ST*     find_social args( ( char* command ) );
CMDTYPE* find_command   args( ( char* command ) );
void    hash_commands   args( ( ) );
bool    is_plain_text   args( ( char* text ) );
void    get_last_arg    args( ( char* arg, char* str ) );
void    start_timer args( ( struct timeval* stime ) );
time_t  end_timer   args( ( struct timeval* stime ) );
void    send_timer  args( ( struct timerset* vtime, CHAR_DATA* ch ) );
void    update_userec   args( ( struct timeval* time_used, struct timerset* userec ) );
bool    is_ignored  args( ( CHAR_DATA* ch, char* name ) );
int     count_args  args( ( char* argument ) );
int     cnt_arg     args( ( char* argument, int cnt ) );


/* request.c */
void    init_request_pipe   args( ( void ) );
void    check_requests      args( ( void ) );

/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY    0
#define OS_CORPSE   1
void    save_char_obj   args( ( CHAR_DATA* ch ) );
bool    load_char_obj   args( ( DESCRIPTOR_DATA* d, char* name, bool preload ) );
void    set_alarm   args( ( long seconds ) );
void    requip_char args( ( CHAR_DATA* ch ) );
void    fwrite_obj      args( ( CHAR_DATA* ch,  OBJ_DATA*  obj, FILE* fp,
        int iNest, sh_int os_type ) );
void    fread_obj   args( ( CHAR_DATA* ch,  FILE* fp, sh_int os_type ) );
void    de_equip_char   args( ( CHAR_DATA* ch ) );
void    re_equip_char   args( ( CHAR_DATA* ch ) );
void    save_home   args( ( CHAR_DATA* ch ) );
void    set_ident   args( ( CHAR_DATA* ch ) );
void    fwrite_mobile   args( ( FILE* fp, CHAR_DATA* mob ) );
void    write_stats     args( ( CHAR_DATA* ch ) );

/* shops.c */
CHAR_DATA* find_keeper args( ( CHAR_DATA* ch ) );

/* special.c */
SF*     spec_lookup args( ( const char* name ) );
char*   lookup_spec args( ( SPEC_FUN* special ) );

/* tables.c */
int get_skill   args( ( char* skilltype ) );
char*   skill_name  args( ( DO_FUN* skill ) );
void    load_skill_table args( ( void ) );
void    save_skill_table args( ( void ) );
void    sort_skill_table args( ( void ) );
void    load_socials    args( ( void ) );
void    save_socials    args( ( void ) );
void    load_commands   args( ( void ) );
void    save_commands   args( ( void ) );
DO_FUN* skill_function  args( ( char* name ) );
void    load_herb_table args( ( void ) );
void    save_herb_table args( ( void ) );

/* track.c */
void    found_prey  args( ( CHAR_DATA* ch, CHAR_DATA* victim ) );
void    hunt_victim args( ( CHAR_DATA* ch ) );
int     find_first_step args( ( ROOM_INDEX_DATA* src, ROOM_INDEX_DATA* target, int maxdist ) );
void    follow_victim   args( ( CHAR_DATA* ch ) );

/* update.c */
void    advance_level   args( ( CHAR_DATA* ch, bool silent ) );
void    gain_exp        args( ( CHAR_DATA* ch, int gain ) );
void    update_handler  args( ( void ) );
void    reboot_check    args( ( time_t reset ) );
#if 0
void    reboot_check    args( ( char* arg ) );
#endif
void    remove_portal   args( ( OBJ_DATA* portal ) );
void    write_serverstats args( ( void ) );
void    equipment_recharge  args( ( CHAR_DATA* ch ) );
void    obj_tick    args( ( OBJ_DATA* obj ) );
void    bacta_update    args( ( void ) );
void    update_movement args( ( void ) );
void    update_respawn  args( ( void ) );
void    update_ooc  args( ( void ) );
void    pulse_update    args( ( void ) );
void    recharge_drill  args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
void    recharge_item   args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );

/* upgrades.c */
void    send_attach_stats args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );
void    recharge_attachment args( ( CHAR_DATA* ch, OBJ_DATA* tmp ) );
int     check_light_modifier    args( ( OBJ_DATA* obj ) );
int     fire_attachment     args( ( CHAR_DATA* ch, OBJ_DATA* obj, int dir, int range ) );
int     attach_light_modifier   args( ( CHAR_DATA* ch ) );
bool    has_thermal     args( ( CHAR_DATA* ch ) );
bool    use_attachment  args( ( CHAR_DATA* ch, OBJ_DATA* tmp ) );
void    send_attach_note    args( ( CHAR_DATA* ch, OBJ_DATA* obj ) );

/* hashstr.c */
char*   str_alloc   args( ( const char* str ) );
char*   quick_link  args( ( const char* str ) );
int     str_free    args( ( const char* str ) );
void    show_hash   args( ( int count ) );
char*   hash_stats  args( ( void ) );
char*   check_hash  args( ( const char* str ) );
void    hash_dump   args( ( int hash ) );
void    show_high_hash  args( ( int top ) );

/* websrv.c */
void init_web( int wport );
void handle_web( void );
void shutdown_web( void );

/* newscore.c */
char*   get_race    args( ( CHAR_DATA* ch ) );

#undef  SK
#undef  CO
#undef  ST
#undef  CD
#undef  MID
#undef  OD
#undef  OID
#undef  RID
#undef  SF
#undef  BD
#undef  CL
#undef  EDD
#undef  RD
#undef  ED

/*

    New Build Interface Stuff Follows

*/


/*
    Data for a menu page
*/
struct  menu_data
{
    char*        sectionNum;
    char*        charChoice;
    int         x;
    int         y;
    char*        outFormat;
    void*        data;
    int         ptrType;
    int         cmdArgs;
    char*        cmdString;
};

DECLARE_DO_FUN( do_redraw_page  );
DECLARE_DO_FUN( do_refresh_page );
DECLARE_DO_FUN( do_pagelen  );
DECLARE_DO_FUN( do_omenu    );
DECLARE_DO_FUN( do_rmenu    );
DECLARE_DO_FUN( do_mmenu    );
DECLARE_DO_FUN( do_clear    );

extern      MENU_DATA       room_page_a_data[];
extern      MENU_DATA       room_page_b_data[];
extern      MENU_DATA       room_page_c_data[];
extern      MENU_DATA       room_help_page_data[];

extern      MENU_DATA       mob_page_a_data[];
extern      MENU_DATA       mob_page_b_data[];
extern      MENU_DATA       mob_page_c_data[];
extern      MENU_DATA       mob_page_d_data[];
extern      MENU_DATA       mob_page_e_data[];
extern      MENU_DATA       mob_page_f_data[];
extern      MENU_DATA       mob_help_page_data[];

extern      MENU_DATA       obj_page_a_data[];
extern      MENU_DATA       obj_page_b_data[];
extern      MENU_DATA       obj_page_c_data[];
extern      MENU_DATA       obj_page_d_data[];
extern      MENU_DATA       obj_page_e_data[];
extern      MENU_DATA       obj_help_page_data[];

extern      MENU_DATA       control_page_a_data[];
extern      MENU_DATA       control_help_page_data[];

extern  const   char    room_page_a[];
extern  const   char    room_page_b[];
extern  const   char    room_page_c[];
extern  const   char    room_help_page[];

extern  const   char    obj_page_a[];
extern  const   char    obj_page_b[];
extern  const   char    obj_page_c[];
extern  const   char    obj_page_d[];
extern  const   char    obj_page_e[];
extern  const   char    obj_help_page[];

extern  const   char    mob_page_a[];
extern  const   char    mob_page_b[];
extern  const   char    mob_page_c[];
extern  const   char    mob_page_d[];
extern  const   char    mob_page_e[];
extern  const   char    mob_page_f[];
extern  const   char    mob_help_page[];
extern  const   char*   npc_sex[3];
extern  const   char*   ris_strings[];

extern  const   char    control_page_a[];
extern  const   char    control_help_page[];

#define SH_INT 1
#define INT 2
#define CHAR 3
#define STRING 4
#define SPECIAL 5


#define NO_PAGE    0
#define MOB_PAGE_A 1
#define MOB_PAGE_B 2
#define MOB_PAGE_C 3
#define MOB_PAGE_D 4
#define MOB_PAGE_E 5
#define MOB_PAGE_F 17
#define MOB_HELP_PAGE 14
#define ROOM_PAGE_A 6
#define ROOM_PAGE_B 7
#define ROOM_PAGE_C 8
#define ROOM_HELP_PAGE 15
#define OBJ_PAGE_A 9
#define OBJ_PAGE_B 10
#define OBJ_PAGE_C 11
#define OBJ_PAGE_D 12
#define OBJ_PAGE_E 13
#define OBJ_HELP_PAGE 16
#define CONTROL_PAGE_A 18
#define CONTROL_HELP_PAGE 19

#define NO_TYPE   0
#define MOB_TYPE  1
#define OBJ_TYPE  2
#define ROOM_TYPE 3
#define CONTROL_TYPE 4

#define SUB_NORTH DIR_NORTH
#define SUB_EAST  DIR_EAST
#define SUB_SOUTH DIR_SOUTH
#define SUB_WEST  DIR_WEST
#define SUB_UP    DIR_UP
#define SUB_DOWN  DIR_DOWN
#define SUB_NE    DIR_NORTHEAST
#define SUB_NW    DIR_NORTHWEST
#define SUB_SE    DIR_SOUTHEAST
#define SUB_SW    DIR_SOUTHWEST

/*
    defines for use with this get_affect function
*/

#define RIS_000     BV00
#define RIS_R00     BV01
#define RIS_0I0     BV02
#define RIS_RI0     BV03
#define RIS_00S     BV04
#define RIS_R0S     BV05
#define RIS_0IS     BV06
#define RIS_RIS     BV07

#define GA_AFFECTED BV09
#define GA_RESISTANT    BV10
#define GA_IMMUNE   BV11
#define GA_SUSCEPTIBLE  BV12
#define GA_RIS          BV30

/*
    mudprograms stuff
*/
extern  CHAR_DATA* supermob;

void oprog_speech_trigger( char* txt, CHAR_DATA* ch );
void oprog_random_trigger( OBJ_DATA* obj );
void oprog_wear_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
bool oprog_use_trigger( CHAR_DATA* ch, OBJ_DATA* obj,
                        CHAR_DATA* vict, OBJ_DATA* targ, void* vo );
void oprog_remove_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_sac_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_damage_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_repair_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_drop_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_zap_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
char* oprog_type_to_name( int type );

/*
    MUD_PROGS START HERE
    (object stuff)
*/
void oprog_greet_trigger( CHAR_DATA* ch );
void oprog_speech_trigger( char* txt, CHAR_DATA* ch );
void oprog_random_trigger( OBJ_DATA* obj );
void oprog_random_trigger( OBJ_DATA* obj );
void oprog_remove_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_sac_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_get_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_damage_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_repair_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_drop_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_examine_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_zap_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_useon_trigger( CHAR_DATA* ch, OBJ_DATA* obj );
void oprog_useoff_trigger( CHAR_DATA* ch, OBJ_DATA* obj );


/* mud prog defines */

#define ERROR_PROG        -1
#define IN_FILE_PROG       0
#define ACT_PROG           1 // BV00
#define SPEECH_PROG        2 // BV01
#define RAND_PROG          3 // BV02
#define FIGHT_PROG         4 // BV03
#define RFIGHT_PROG        4 // BV03
#define DEATH_PROG         5 // BV04
#define RDEATH_PROG        5 // BV04
#define HITPRCNT_PROG      6 // BV05
#define ENTRY_PROG         7 // BV06
#define ENTER_PROG         7 // BV06
#define GREET_PROG         8 // BV07
#define RGREET_PROG        8 // BV07
#define OGREET_PROG        8 // BV07
#define ALL_GREET_PROG     9 // BV08
#define GIVE_PROG          10 // BV09
#define BRIBE_PROG         11 // BV10
#define HOUR_PROG          12 // BV11
#define TIME_PROG          13 // BV12
#define WEAR_PROG          14 // BV13  
#define REMOVE_PROG        15 // BV14  
#define SAC_PROG           16 // BV15  
#define LOOK_PROG          17 // BV16  
#define EXA_PROG           18 // BV17  
#define ZAP_PROG           19 // BV18  
#define GET_PROG           20 // BV19  
#define DROP_PROG          21 // BV20  
#define DAMAGE_PROG        22 // BV21  
#define REPAIR_PROG        23 // BV22  
#define RANDIW_PROG        24 // BV23  
#define SPEECHIW_PROG      25 // BV24  
#define USEON_PROG         26 // BV25  
#define USEOFF_PROG        27 // BV26  
#define SLEEP_PROG         28 // BV27  
#define REST_PROG          29 // BV28  
#define LEAVE_PROG         30 // BV29
#define SCRIPT_PROG        31 // BV30
#define USE_PROG           32 // BV31

// New 'Ghostified' Progtypes start here
#define PULSE_PROG         33

#define MAX_PROG           33

void rprog_leave_trigger( CHAR_DATA* ch );
void rprog_enter_trigger( CHAR_DATA* ch );
void rprog_sleep_trigger( CHAR_DATA* ch );
void rprog_rest_trigger( CHAR_DATA* ch );
void rprog_rfight_trigger( CHAR_DATA* ch );
void rprog_death_trigger( CHAR_DATA* killer, CHAR_DATA* ch );
void rprog_speech_trigger( char* txt, CHAR_DATA* ch );
void rprog_random_trigger( CHAR_DATA* ch );
void rprog_time_trigger( CHAR_DATA* ch );
void rprog_hour_trigger( CHAR_DATA* ch );
char* rprog_type_to_name( int type );

#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
void oprog_act_trigger( char* buf, OBJ_DATA* mobj, CHAR_DATA* ch,
                        OBJ_DATA* obj, void* vo );
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
void rprog_act_trigger( char* buf, ROOM_INDEX_DATA* room, CHAR_DATA* ch,
                        OBJ_DATA* obj, void* vo );
#endif


#define g_r_i         get_room_index
#define send_to_char  send_to_char_color
#define send_to_pager send_to_pager_color

void save_xnames( void );

#ifdef WIN32
void gettimeofday( struct timeval* tv, struct timezone* tz );
void kill_timer();

/* directory scanning stuff */

typedef struct dirent
{
    char*   d_name;
};

typedef struct
{
    HANDLE      hDirectory;
    WIN32_FIND_DATA Win32FindData;
    struct dirent   dirinfo;
    char        sDirName[MAX_PATH];
} DIR;


DIR* opendir( char* sDirName );
struct dirent* readdir ( DIR* dp );
void closedir( DIR* dp );

/* --------------- Stuff for Win32 services ------------------ */
/*

    NJG:

    When "exit" is called to handle an error condition, we really want to
    terminate the game thread, not the whole process.

*/

#define exit(arg) Win32_Exit(arg)
void Win32_Exit( int exit_code );

#endif
