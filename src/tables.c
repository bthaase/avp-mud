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
* 			Table load/save Module				   *
****************************************************************************/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


/* global variables */
int top_sn;
int top_herb;

SKILLTYPE *     skill_table    [MAX_SKILL];
SKILLTYPE *     herb_table     [MAX_HERB];
SKILLTYPE *     disease_table  [MAX_DISEASE];

char * const skill_tname[] = { "unknown", "Spell", "Skill", "Weapon", "Tongue", "Herb", "Disease" };

DO_FUN *skill_function( char *name )
{
    switch( name[3] )
    {
    case 'a':
	if ( !str_cmp( name, "do_aassign" ))		return do_aassign;
	if ( !str_cmp( name, "do_advance" ))		return do_advance;
	if ( !str_cmp( name, "do_affected" ))		return do_affected;
	if ( !str_cmp( name, "do_afk" ))		return do_afk;
        if ( !str_cmp( name, "do_allarenas" ))          return do_allarenas;
        if ( !str_cmp( name, "do_allbots" ))            return do_allbots;
	if ( !str_cmp( name, "do_allow" ))		return do_allow;
	if ( !str_cmp( name, "do_allowmp" ))		return do_allowmp;
	if ( !str_cmp( name, "do_allowpk" ))		return do_allowpk;
	if ( !str_cmp( name, "do_ansi" ))		return do_ansi;
	if ( !str_cmp( name, "do_answer" ))		return do_answer;
	if ( !str_cmp( name, "do_apply" ))		return do_apply;
	if ( !str_cmp( name, "do_areas" ))		return do_areas;
        if ( !str_cmp( name, "do_arena" ))              return do_arena;
	if ( !str_cmp( name, "do_arm" ))		return do_arm;
        if ( !str_cmp( name, "do_armor" ))              return do_armor;
        if ( !str_cmp( name, "do_abackup" ))            return do_abackup;
	if ( !str_cmp( name, "do_aset" ))		return do_aset;
	if ( !str_cmp( name, "do_ask" ))		return do_ask;
	if ( !str_cmp( name, "do_astat" ))		return do_astat;
	if ( !str_cmp( name, "do_at" ))			return do_at;
        if ( !str_cmp( name, "do_attach" ))             return do_attach;
	if ( !str_cmp( name, "do_authorize" ))		return do_authorize;
	if ( !str_cmp( name, "do_avtalk" ))		return do_avtalk;
	break;
    case 'b':
        if ( !str_cmp( name, "do_block" ))              return do_block;
        if ( !str_cmp( name, "do_buildwalk" ))          return do_buildwalk;
	if ( !str_cmp( name, "do_balzhur" ))		return do_balzhur;
	if ( !str_cmp( name, "do_bamfin" ))		return do_bamfin;
	if ( !str_cmp( name, "do_bamfout" ))		return do_bamfout;
	if ( !str_cmp( name, "do_ban" ))		return do_ban;
	if ( !str_cmp( name, "do_bashdoor" ))		return do_bashdoor;
	if ( !str_cmp( name, "do_beep" ))               return do_beep;
	if ( !str_cmp( name, "do_bestow" ))		return do_bestow;
	if ( !str_cmp( name, "do_bestowarea" ))		return do_bestowarea;
	if ( !str_cmp( name, "do_bio" ))		return do_bio;
	if ( !str_cmp( name, "do_boards" ))		return do_boards;
	if ( !str_cmp( name, "do_bodybag" ))		return do_bodybag;
	if ( !str_cmp( name, "do_bset" ))		return do_bset;
	if ( !str_cmp( name, "do_bstat" ))		return do_bstat;
	if ( !str_cmp( name, "do_bug" ))		return do_bug;
	if ( !str_cmp( name, "do_bury" ))		return do_bury;
	if ( !str_cmp( name, "do_buy" ))		return do_buy;
	break;
    case 'c':
        if ( !str_cmp( name, "do_carry" ))              return do_carry;
        if ( !str_cmp( name, "do_cache" ))              return do_cache;
        if ( !str_cmp( name, "do_cover" ))              return do_cover;
        if ( !str_cmp( name, "do_complain" ))           return do_complain;
        if ( !str_cmp( name, "do_compute" ))            return do_compute;
	if ( !str_cmp( name, "do_cedit" ))		return do_cedit;
        if ( !str_cmp( name, "do_censor" ))             return do_censor;
	if ( !str_cmp( name, "do_channels" ))		return do_channels;
        if ( !str_cmp( name, "do_changes" ))            return do_changes;
	if ( !str_cmp( name, "do_chat" ))		return do_chat;
	if ( !str_cmp( name, "do_check_vnums" ))	return do_check_vnums;
	if ( !str_cmp( name, "do_climb" ))		return do_climb;
        if ( !str_cmp( name, "do_cloak" ))              return do_cloak;
	if ( !str_cmp( name, "do_close" ))		return do_close;
        if ( !str_cmp( name, "do_copyover" ))           return do_copyover;
	if ( !str_cmp( name, "do_cmdtable" ))		return do_cmdtable;
	if ( !str_cmp( name, "do_commands" ))		return do_commands;
	if ( !str_cmp( name, "do_comment" ))		return do_comment;
	if ( !str_cmp( name, "do_compare" ))		return do_compare;
	if ( !str_cmp( name, "do_config" ))		return do_config;
        if ( !str_cmp( name, "do_construct" ))          return do_construct;
        if ( !str_cmp( name, "do_confuse" ))            return do_confuse;
	if ( !str_cmp( name, "do_credits" ))		return do_credits;
	if ( !str_cmp( name, "do_cset" ))		return do_cset;
	break;
    case 'd':
        if ( !str_cmp( name, "do_delay" ))              return do_delay;
        if ( !str_cmp( name, "do_dehive" ))             return do_dehive;
        if ( !str_cmp( name, "do_detach" ))             return do_detach;
        if ( !str_cmp( name, "do_deploy" ))             return do_deploy;
	if ( !str_cmp( name, "do_deny" ))		return do_deny;
	if ( !str_cmp( name, "do_description" ))	return do_description;
	if ( !str_cmp( name, "do_destro" ))		return do_destro;
	if ( !str_cmp( name, "do_destroy" ))		return do_destroy;
	if ( !str_cmp( name, "do_dig" ))		return do_dig;
	if ( !str_cmp( name, "do_disconnect" ))		return do_disconnect;
	if ( !str_cmp( name, "do_dmesg" ))		return do_dmesg;
	if ( !str_cmp( name, "do_dnd" ))	       	return do_dnd;
	if ( !str_cmp( name, "do_down" ))		return do_down;
	if ( !str_cmp( name, "do_drag" ))		return do_drag;
	if ( !str_cmp( name, "do_drop" ))		return do_drop;
	break;
    case 'e':
	if ( !str_cmp( name, "do_east" ))		return do_east;
	if ( !str_cmp( name, "do_eat" ))		return do_eat;
	if ( !str_cmp( name, "do_echo" ))		return do_echo;
	if ( !str_cmp( name, "do_emote" ))		return do_emote;
	if ( !str_cmp( name, "do_empty" ))		return do_empty;
        if ( !str_cmp( name, "do_eject" ))              return do_eject;
	if ( !str_cmp( name, "do_enter" ))		return do_enter;
	if ( !str_cmp( name, "do_equipment" ))		return do_equipment;
	if ( !str_cmp( name, "do_examine" ))		return do_examine;
	if ( !str_cmp( name, "do_exits" ))		return do_exits;
	break;
    case 'f':
	if ( !str_cmp( name, "do_fixchar" ))		return do_fixchar;
        if ( !str_cmp( name, "do_fire" ))               return do_fire;
	if ( !str_cmp( name, "do_foldarea" ))		return do_foldarea;
	if ( !str_cmp( name, "do_follow" ))		return do_follow;
	if ( !str_cmp( name, "do_for" ))		return do_for;
	if ( !str_cmp( name, "do_force" ))		return do_force;
	if ( !str_cmp( name, "do_forceclose" ))		return do_forceclose;
	if ( !str_cmp( name, "do_form_password" ))	return do_form_password;
	if ( !str_cmp( name, "do_fquit" ))		return do_fquit;
	if ( !str_cmp( name, "do_freeze" ))		return do_freeze;
        if ( !str_cmp( name, "do_freevnums" ))          return do_freevnums;
	break;
    case 'g':
	if ( !str_cmp( name, "do_get" ))		return do_get;
	if ( !str_cmp( name, "do_give" ))		return do_give;
        if ( !str_cmp( name, "do_glance" ))             return do_glance;
	if ( !str_cmp( name, "do_goto" ))		return do_goto;
	break;
    case 'h':
	if ( !str_cmp( name, "do_hedit" ))		return do_hedit;
	if ( !str_cmp( name, "do_hell" ))		return do_hell;
	if ( !str_cmp( name, "do_help" ))		return do_help;
	if ( !str_cmp( name, "do_hide" ))		return do_hide;
        if ( !str_cmp( name, "do_hive" ))               return do_hive;
	if ( !str_cmp( name, "do_hlist" ))		return do_hlist;
	if ( !str_cmp( name, "do_holylight" ))		return do_holylight;
        if ( !str_cmp( name, "do_html" ))               return do_html;
	if ( !str_cmp( name, "do_homepage" ))		return do_homepage;
	if ( !str_cmp( name, "do_hset" ))		return do_hset;
	break;
    case 'i':
        if ( !str_cmp( name, "do_ignore" ))             return do_ignore;
	if ( !str_cmp( name, "do_i103" ))		return do_i103;
	if ( !str_cmp( name, "do_i104" ))		return do_i104;
	if ( !str_cmp( name, "do_i105" ))		return do_i105;
        if ( !str_cmp( name, "do_i200" ))               return do_i200;
	if ( !str_cmp( name, "do_ide" ))		return do_ide;
	if ( !str_cmp( name, "do_idea" ))		return do_idea;
	if ( !str_cmp( name, "do_immortalize" ))	return do_immortalize;
	if ( !str_cmp( name, "do_immtalk" ))		return do_immtalk;
	if ( !str_cmp( name, "do_installarea" ))	return do_installarea;
	if ( !str_cmp( name, "do_instaroom" ))		return do_instaroom;
	if ( !str_cmp( name, "do_instazone" ))		return do_instazone;
	if ( !str_cmp( name, "do_inventory" ))		return do_inventory;
	if ( !str_cmp( name, "do_invis" ))		return do_invis;
	break;
    case 'j':
        if ( !str_cmp( name, "do_junk" ))               return do_junk;
        break;
    case 'k':
	if ( !str_cmp( name, "do_kill" ))		return do_kill;
        if ( !str_cmp( name, "do_kneel" ))              return do_kneel;
	break;
    case 'l':
        if ( !str_cmp( name, "do_lob" ))                return do_lob;
        if ( !str_cmp( name, "do_lunge" ) )             return do_lunge;
        if ( !str_cmp( name, "do_lstat" ))              return do_lstat;
	if ( !str_cmp( name, "do_last" ))		return do_last;
	if ( !str_cmp( name, "do_leave" ))		return do_leave;
        if ( !str_cmp( name, "do_leap" ))               return do_leap;
	if ( !str_cmp( name, "do_level" ))		return do_level;
	if ( !str_cmp( name, "do_list" ))		return do_list;
	if ( !str_cmp( name, "do_loadarea" ))		return do_loadarea;
	if ( !str_cmp( name, "do_loadup" ))		return do_loadup;
	if ( !str_cmp( name, "do_lock" ))		return do_lock;
	if ( !str_cmp( name, "do_log" ))		return do_log;
	if ( !str_cmp( name, "do_look" ))		return do_look;
	if ( !str_cmp( name, "do_low_purge" ))		return do_low_purge;
	break;
    case 'm':
	if ( !str_cmp( name, "do_mailroom" ))		return do_mailroom;
        if ( !str_cmp( name, "do_makearena" ))          return do_makearena;
	if ( !str_cmp( name, "do_makeboard" ))		return do_makeboard;
        if ( !str_cmp( name, "do_makebot" ))            return do_makebot;
	if ( !str_cmp( name, "do_makeshop" ))		return do_makeshop;
	if ( !str_cmp( name, "do_makewizlist" ))	return do_makewizlist;
	if ( !str_cmp( name, "do_massign" ))		return do_massign;
	if ( !str_cmp( name, "do_mcreate" ))		return do_mcreate;
	if ( !str_cmp( name, "do_mdelete" ))		return do_mdelete;
	if ( !str_cmp( name, "do_memory" ))		return do_memory;
        if ( !str_cmp( name, "do_menu" ))               return do_menu;
	if ( !str_cmp( name, "do_mfind" ))		return do_mfind;
	if ( !str_cmp( name, "do_minvoke" ))		return do_minvoke;
	if ( !str_cmp( name, "do_mlist" ))		return do_mlist;
        if ( !str_cmp( name, "do_mpvar" ))              return do_mpvar;
        if ( !str_cmp( name, "do_mpmonitor" ))          return do_mpmonitor;
        if ( !str_cmp( name, "do_mpblast" ))            return do_mpblast;
        if ( !str_cmp( name, "do_mpcycle" ))            return do_mpcycle;
        if ( !str_cmp( name, "do_mpteamgain" ))         return do_mpteamgain;
	if ( !str_cmp( name, "do_mp_close_passage" ))	return do_mp_close_passage;
        if ( !str_cmp( name, "do_mpwait" ))             return do_mpwait;
        if ( !str_cmp( name, "do_mpadd" ))              return do_mpadd;
        if ( !str_cmp( name, "do_mpsub" ))              return do_mpsub;
        if ( !str_cmp( name, "do_mpset" ))              return do_mpset;
        if ( !str_cmp( name, "do_mprmflag" ))           return do_mprmflag;
        if ( !str_cmp( name, "do_mpreset" ))            return do_mpreset;
        if ( !str_cmp( name, "do_mprat" ))              return do_mprat;
	if ( !str_cmp( name, "do_mp_damage" ))		return do_mp_damage;
	if ( !str_cmp( name, "do_mp_open_passage" ))	return do_mp_open_passage;
	if ( !str_cmp( name, "do_mp_practice" ))	return do_mp_practice;
	if ( !str_cmp( name, "do_mp_restore" ))		return do_mp_restore;
	if ( !str_cmp( name, "do_mp_slay" ))		return do_mp_slay;
	if ( !str_cmp( name, "do_mpadvance" ))		return do_mpadvance;
	if ( !str_cmp( name, "do_mpapply" ))		return do_mpapply;
	if ( !str_cmp( name, "do_mpapplyb" ))		return do_mpapplyb;
	if ( !str_cmp( name, "do_mpasound" ))		return do_mpasound;
	if ( !str_cmp( name, "do_mpat" ))		return do_mpat;
	if ( !str_cmp( name, "do_mpdream" ))		return do_mpdream;
	if ( !str_cmp( name, "do_mpecho" ))		return do_mpecho;
        if ( !str_cmp( name, "do_mprunspec" ))          return do_mprunspec;
	if ( !str_cmp( name, "do_mpechoaround" ))	return do_mpechoaround;
	if ( !str_cmp( name, "do_mpechoat" ))		return do_mpechoat;
	if ( !str_cmp( name, "do_mpedit" ))		return do_mpedit;
	if ( !str_cmp( name, "do_mpgain" ))		return do_mpgain;
	if ( !str_cmp( name, "do_mpforce" ))		return do_mpforce;
	if ( !str_cmp( name, "do_mpgoto" ))		return do_mpgoto;
	if ( !str_cmp( name, "do_mpinvis" ))		return do_mpinvis;
	if ( !str_cmp( name, "do_mpjunk" ))		return do_mpjunk;
	if ( !str_cmp( name, "do_mpkill" ))		return do_mpkill;
	if ( !str_cmp( name, "do_mpmload" ))		return do_mpmload;
	if ( !str_cmp( name, "do_mpnothing" ))		return do_mpnothing;
	if ( !str_cmp( name, "do_mpoload" ))		return do_mpoload;
	if ( !str_cmp( name, "do_mppkset" ))		return do_mppkset;
	if ( !str_cmp( name, "do_mppurge" ))		return do_mppurge;
	if ( !str_cmp( name, "do_mpstat" ))		return do_mpstat;
	if ( !str_cmp( name, "do_mptransfer" ))		return do_mptransfer;
	if ( !str_cmp( name, "do_mrange" ))		return do_mrange;
	if ( !str_cmp( name, "do_mset" ))		return do_mset;
	if ( !str_cmp( name, "do_mstat" ))		return do_mstat;
	if ( !str_cmp( name, "do_mwhere" ))		return do_mwhere;
	break;
    case 'n':
	if ( !str_cmp( name, "do_name" ))		return do_name;
        if ( !str_cmp( name, "do_nothing" ))            return do_nothing;
	if ( !str_cmp( name, "do_newbiechat" ))		return do_newbiechat;
	if ( !str_cmp( name, "do_newbieset" ))		return do_newbieset;
	if ( !str_cmp( name, "do_news" ))		return do_news;
	if ( !str_cmp( name, "do_newzones" ))		return do_newzones;
	if ( !str_cmp( name, "do_noemote" ))		return do_noemote;
	if ( !str_cmp( name, "do_noresolve" ))		return do_noresolve;
	if ( !str_cmp( name, "do_north" ))		return do_north;
	if ( !str_cmp( name, "do_northeast" ))		return do_northeast;
	if ( !str_cmp( name, "do_northwest" ))		return do_northwest;
	if ( !str_cmp( name, "do_notell" ))		return do_notell;
	if ( !str_cmp( name, "do_noteroom" ))		return do_noteroom;
	break;
    case 'o':
	if ( !str_cmp( name, "do_oassign" ))		return do_oassign;
	if ( !str_cmp( name, "do_ocreate" ))		return do_ocreate;
	if ( !str_cmp( name, "do_odelete" ))		return do_odelete;
	if ( !str_cmp( name, "do_ofind" ))		return do_ofind;
/*	if ( !str_cmp( name, "do_ogrub" ))		return do_ogrub;
*/	if ( !str_cmp( name, "do_oinvoke" ))		return do_oinvoke;
	if ( !str_cmp( name, "do_oldscore" ))		return do_oldscore;
	if ( !str_cmp( name, "do_olist" ))		return do_olist;
	if ( !str_cmp( name, "do_ooc" ))		return do_ooc;
	if ( !str_cmp( name, "do_opedit" ))		return do_opedit;
	if ( !str_cmp( name, "do_open" ))		return do_open;
/*	if ( !str_cmp( name, "do_opentourney" ))	return do_opentourney; */
	if ( !str_cmp( name, "do_opstat" ))		return do_opstat;
	if ( !str_cmp( name, "do_orange" ))		return do_orange;
	if ( !str_cmp( name, "do_oset" ))		return do_oset;
	if ( !str_cmp( name, "do_ostat" ))		return do_ostat;
	if ( !str_cmp( name, "do_owhere" ))		return do_owhere;
	break;
    case 'p':
	if ( !str_cmp( name, "do_pager" ))		return do_pager;
        if ( !str_cmp( name, "do_pfiles" ))             return do_pfiles;
	if ( !str_cmp( name, "do_password" ))		return do_password;
	if ( !str_cmp( name, "do_practice" ))		return do_practice;
        if ( !str_cmp( name, "do_prestore" ))           return do_prestore;
	if ( !str_cmp( name, "do_prompt" ))		return do_prompt;
        if ( !str_cmp( name, "do_prone" ))              return do_prone;
	if ( !str_cmp( name, "do_purge" ))		return do_purge;
	if ( !str_cmp( name, "do_put" ))		return do_put;
	break;
    case 'q':
	if ( !str_cmp( name, "do_qui" ))		return do_qui;
	if ( !str_cmp( name, "do_quit" ))		return do_quit;
	break;
    case 'r':
        if ( !str_cmp( name, "do_request" ))            return do_request;
        if ( !str_cmp( name, "do_respawn" ))            return do_respawn;
        if ( !str_cmp( name, "do_rcreate" ))            return do_rcreate;
        if ( !str_cmp( name, "do_rage" ))               return do_rage;
        if ( !str_cmp( name, "do_radio" ))              return do_radio;
        if ( !str_cmp( name, "do_radio1" ))             return do_radio1;
        if ( !str_cmp( name, "do_radio2" ))             return do_radio2;
        if ( !str_cmp( name, "do_radio3" ))             return do_radio3;
        if ( !str_cmp( name, "do_radio4" ))             return do_radio4;
	if ( !str_cmp( name, "do_rassign" ))		return do_rassign;
	if ( !str_cmp( name, "do_rat" ))		return do_rat;
        if ( !str_cmp( name, "do_rdelete" ))            return do_rdelete;
	if ( !str_cmp( name, "do_reboo" ))		return do_reboo;
	if ( !str_cmp( name, "do_reboot" ))		return do_reboot;
	if ( !str_cmp( name, "do_recho" ))		return do_recho;
	if ( !str_cmp( name, "do_recall" ))             return do_recall;
        if ( !str_cmp( name, "do_rcopy" ))              return do_rcopy;
        if ( !str_cmp( name, "do_rpcopy" ))             return do_rpcopy;
        if ( !str_cmp( name, "do_redit" ))              return do_redit;
	if ( !str_cmp( name, "do_regoto" ))		return do_regoto;
	if ( !str_cmp( name, "do_reload" ))  		return do_reload;
	if ( !str_cmp( name, "do_remove" ))		return do_remove;
        if ( !str_cmp( name, "do_release" ))            return do_release;
        if ( !str_cmp( name, "do_rescue" ))             return do_rescue;
	if ( !str_cmp( name, "do_reply" ))		return do_reply;
	if ( !str_cmp( name, "do_rescue" ))		return do_rescue;
	if ( !str_cmp( name, "do_reset" ))		return do_reset;
	if ( !str_cmp( name, "do_restore" ))		return do_restore;
	if ( !str_cmp( name, "do_restoretime" ))	return do_restoretime;
	if ( !str_cmp( name, "do_restrict" ))		return do_restrict;
	if ( !str_cmp( name, "do_retran" ))		return do_retran;
	if ( !str_cmp( name, "do_return" ))		return do_return;
        if ( !str_cmp( name, "do_rdig" ))               return do_rdig;
	if ( !str_cmp( name, "do_rip" ))		return do_rip;
	if ( !str_cmp( name, "do_rlist" ))		return do_rlist;
        if ( !str_cmp( name, "do_rfill" ))              return do_rfill;
	if ( !str_cmp( name, "do_rpedit" ))		return do_rpedit;
	if ( !str_cmp( name, "do_rpstat" ))		return do_rpstat;
	if ( !str_cmp( name, "do_rreset" ))		return do_rreset;
	if ( !str_cmp( name, "do_rstat" ))		return do_rstat;
	break;
    case 's':
        if ( !str_cmp( name, "do_spit" ))               return do_spit;
        if ( !str_cmp( name, "do_smash" ))              return do_smash;
	if ( !str_cmp( name, "do_save" ))		return do_save;
	if ( !str_cmp( name, "do_saveall" ))            return do_saveall;
	if ( !str_cmp( name, "do_savearea" ))		return do_savearea;
	if ( !str_cmp( name, "do_say" ))		return do_say;
	if ( !str_cmp( name, "do_score" ))		return do_score;
        if ( !str_cmp( name, "do_sc" ))                 return do_sc;
        if ( !str_cmp( name, "do_scan" ))               return do_scan;
	if ( !str_cmp( name, "do_sedit" ))		return do_sedit;
	if ( !str_cmp( name, "do_sell" ))		return do_sell;
        if ( !str_cmp( name, "do_setmode" ))            return do_setmode;
        if ( !str_cmp( name, "do_setbot" ))             return do_setbot;
        if ( !str_cmp( name, "do_showbot" ))            return do_showbot;
        if ( !str_cmp( name, "do_setarena" ))           return do_setarena;
        if ( !str_cmp( name, "do_setlock" ))            return do_setlock;
        if ( !str_cmp( name, "do_setooc" ))             return do_setooc;
	if ( !str_cmp( name, "do_set_boot_time" ))	return do_set_boot_time;
        if ( !str_cmp( name, "do_showarena" ))          return do_showarena;
	if ( !str_cmp( name, "do_shops" ))		return do_shops;
	if ( !str_cmp( name, "do_shopset" ))		return do_shopset;
	if ( !str_cmp( name, "do_shopstat" ))		return do_shopstat;
	if ( !str_cmp( name, "do_shout" ))		return do_shout;
	if ( !str_cmp( name, "do_shove" ))		return do_shove;
        if ( !str_cmp( name, "do_show" ))               return do_show;
	if ( !str_cmp( name, "do_shutdow" ))		return do_shutdow;
	if ( !str_cmp( name, "do_shutdown" ))		return do_shutdown;
	if ( !str_cmp( name, "do_silence" ))		return do_silence;
	if ( !str_cmp( name, "do_sit" ))		return do_sit;
	if ( !str_cmp( name, "do_sla" ))		return do_sla;
	if ( !str_cmp( name, "do_slay" ))		return do_slay;
	if ( !str_cmp( name, "do_slist" ))		return do_slist;
	if ( !str_cmp( name, "do_slookup" ))		return do_slookup;
	if ( !str_cmp( name, "do_snipe" ))		return do_snipe;
	if ( !str_cmp( name, "do_snoop" ))		return do_snoop;
	if ( !str_cmp( name, "do_sober" ))		return do_sober;
	if ( !str_cmp( name, "do_socials" ))		return do_socials;
	if ( !str_cmp( name, "do_south" ))		return do_south;
	if ( !str_cmp( name, "do_sound" ))		return do_sound;
	if ( !str_cmp( name, "do_southeast" ))		return do_southeast;
	if ( !str_cmp( name, "do_southwest" ))		return do_southwest;
	if ( !str_cmp( name, "do_speak" ))		return do_speak;
	if ( !str_cmp( name, "do_sset" ))		return do_sset;
        if ( !str_cmp( name, "do_steacher" ))           return do_steacher;
        if ( !str_cmp( name, "do_stack" ))              return do_stack;
	if ( !str_cmp( name, "do_stand" ))		return do_stand;
        if ( !str_cmp( name, "do_spray" ))              return do_spray;
	if ( !str_cmp( name, "do_switch" ))		return do_switch;
	break;
    case 't':
        if ( !str_cmp( name, "do_treat" ))              return do_treat;
        if ( !str_cmp( name, "do_tackle" ))             return do_tackle;
	if ( !str_cmp( name, "do_tell" ))		return do_tell;
	if ( !str_cmp( name, "do_throw" ))		return do_throw;
	if ( !str_cmp( name, "do_time" ))		return do_time;
	if ( !str_cmp( name, "do_timecmd" ))		return do_timecmd;
	if ( !str_cmp( name, "do_track" ))		return do_track;
	if ( !str_cmp( name, "do_transfer" ))		return do_transfer;
	if ( !str_cmp( name, "do_trust" ))		return do_trust;
	if ( !str_cmp( name, "do_typo" ))		return do_typo;
	break;
    case 'u':
	if ( !str_cmp( name, "do_unfoldarea" ))		return do_unfoldarea;
        if ( !str_cmp( name, "do_unload" ))             return do_unload;
	if ( !str_cmp( name, "do_unhell" ))		return do_unhell;
	if ( !str_cmp( name, "do_unlock" ))		return do_unlock;
        if ( !str_cmp( name, "do_unsilence" ))          return do_unsilence;
	if ( !str_cmp( name, "do_up" ))			return do_up;
	if ( !str_cmp( name, "do_uptime" ))		return do_uptime;
	if ( !str_cmp( name, "do_use" ))		return do_use;
	if ( !str_cmp( name, "do_users" ))		return do_users;
	break;
    case 'v':
        if ( !str_cmp( name, "do_version" ))            return do_version;
        if ( !str_cmp( name, "do_vote" ))               return do_vote;
	if ( !str_cmp( name, "do_vassign" ))            return do_vassign;
	if ( !str_cmp( name, "do_vnums" ))		return do_vnums;
        if ( !str_cmp( name, "do_vsearch" ))            return do_vsearch;
	break;
    case 'w':
	if ( !str_cmp( name, "do_wear" ))		return do_wear;
	if ( !str_cmp( name, "do_weather" ))		return do_weather;
        if ( !str_cmp( name, "do_webserve" ))           return do_webserve;
	if ( !str_cmp( name, "do_west" ))		return do_west;
	if ( !str_cmp( name, "do_where" ))		return do_where;
	if ( !str_cmp( name, "do_whisper" ))		return do_whisper;
	if ( !str_cmp( name, "do_who" ))		return do_who;
	if ( !str_cmp( name, "do_whois" ))		return do_whois;
	if ( !str_cmp( name, "do_wizhelp" ))		return do_wizhelp;
	if ( !str_cmp( name, "do_wizlist" ))		return do_wizlist;
	if ( !str_cmp( name, "do_wizlock" ))		return do_wizlock;
	break;
    case 'x':
	if ( !str_cmp( name, "do_xname" ))		return do_xname;
	break;
    case 'y':
	if ( !str_cmp( name, "do_yell" ))		return do_yell;
	break;
    case 'z':
	if ( !str_cmp( name, "do_zones" ))		return do_zones;
    }
    return skill_notfound;
}

char *skill_name( DO_FUN *skill )
{   
    if ( skill == do_carry )    return "do_carry";
    if ( skill == do_cache )    return "do_cache";
    if ( skill == do_cover )    return "do_cover";
    if ( skill == do_release )  return "do_release";
    if ( skill == do_rage )     return "do_rage";
    if ( skill == do_request )  return "do_request";
    if ( skill == do_rescue )   return "do_rescue";
    if ( skill == do_respawn )  return "do_respawn";
    if ( skill == do_setooc ) return "do_setooc";
    if ( skill == do_smash ) return "do_smash";
    if ( skill == do_rcreate ) return "do_rcreate";
    if ( skill == do_spit ) return "do_spit";
    if ( skill == do_buildwalk )  return "do_buildwalk";
    if ( skill == do_tackle )       return "do_tackle";
    if ( skill == do_pfiles ) return "do_pfiles";    
    if ( skill == do_block )    return "do_block";
    if ( skill == do_ignore )   return "do_ignore";
    if ( skill == do_i103 )     return "do_i103";
    if ( skill == do_i104 )     return "do_i104";
    if ( skill == do_i105 )     return "do_i105";
    if ( skill == do_i200 )     return "do_i200";
    if ( skill == do_junk )     return "do_junk";
    if ( skill == do_attach )        return "do_attach";
    if ( skill == do_detach )        return "do_detach";
    if ( skill == do_allarenas )     return "do_allarenas";
    if ( skill == do_allbots )       return "do_allbots";
    if ( skill == do_makearena )     return "do_makearena";
    if ( skill == do_showarena )     return "do_showarena";
    if ( skill == do_setarena )      return "do_setarena";
    if ( skill == do_setbot )        return "do_setbot";
    if ( skill == do_showbot )       return "do_showbot";
    if ( skill == do_spray )        return "do_spray";
    if ( skill == do_setlock )  return "do_setlock";
    if ( skill == do_html )     return "do_html";
    if ( skill == do_hive )     return "do_hive";
    if ( skill == do_dehive )   return "do_dehive";
    if ( skill == do_nothing )     return "do_nothing";
    if ( skill == do_arm )     return "do_arm";
    if ( skill == do_armor )     return "do_armor";
    if ( skill == do_setmode )  return "do_setmode";
    if ( skill == do_use )     return "do_use";
    if ( skill == do_throw )     return "do_throw";
    if ( skill == do_snipe )     return "do_snipe";
    if ( skill == do_reload )     return "do_reload";
    if ( skill == do_radio )     return "do_radio";
    if ( skill == do_radio1 )    return "do_radio1";
    if ( skill == do_radio2 )    return "do_radio2";
    if ( skill == do_radio3 )    return "do_radio3";
    if ( skill == do_radio4 )    return "do_radio4";
    if ( skill == do_version )   return "do_version";
    if ( skill == do_vote )      return "do_vote";
    if ( skill == do_recall )    return "do_recall";
    if ( skill == do_compute )          return "do_compute";
    if ( skill == do_aassign )		return "do_aassign";
    if ( skill == do_advance )		return "do_advance";
    if ( skill == do_affected )		return "do_affected";
    if ( skill == do_afk )		return "do_afk";
    if ( skill == do_allow )		return "do_allow";
    if ( skill == do_allowmp )		return "do_allowmp";
    if ( skill == do_allowpk )		return "do_allowpk";
    if ( skill == do_ansi )		return "do_ansi";
    if ( skill == do_sound )		return "do_sound";
    if ( skill == do_answer )		return "do_answer";
    if ( skill == do_apply )		return "do_apply";
    if ( skill == do_areas )		return "do_areas";
    if ( skill == do_arena )            return "do_arena";
    if ( skill == do_aset )		return "do_aset";
    if ( skill == do_abackup )  return "do_abackup";
    if ( skill == do_ask )		return "do_ask";
    if ( skill == do_astat )		return "do_astat";
    if ( skill == do_at )		return "do_at";
    if ( skill == do_authorize )	return "do_authorize";
    if ( skill == do_avtalk )		return "do_avtalk";
    if ( skill == do_balzhur )		return "do_balzhur";
    if ( skill == do_bamfin )		return "do_bamfin";
    if ( skill == do_bamfout )		return "do_bamfout";
    if ( skill == do_ban )		return "do_ban";
    if ( skill == do_bashdoor )		return "do_bashdoor";
    if ( skill == do_beep )             return "do_beep";
    if ( skill == do_bestow )		return "do_bestow";
    if ( skill == do_bestowarea )	return "do_bestowarea";
    if ( skill == do_bio )		return "do_bio";
    if ( skill == do_boards )		return "do_boards";
    if ( skill == do_bodybag )		return "do_bodybag";
    if ( skill == do_bset )		return "do_bset";
    if ( skill == do_bstat )		return "do_bstat";
    if ( skill == do_bug )		return "do_bug";
    if ( skill == do_bury )		return "do_bury";
    if ( skill == do_buy )		return "do_buy";
    if ( skill == do_cedit )		return "do_cedit";
    if ( skill == do_censor )           return "do_censor";
    if ( skill == do_channels )		return "do_channels";
    if ( skill == do_changes )          return "do_changes";
    if ( skill == do_chat )		return "do_chat";
    if ( skill == do_ooc )		return "do_ooc";
    if ( skill == do_check_vnums )	return "do_check_vnums";
    if ( skill == do_climb )		return "do_climb";
    if ( skill == do_cloak )            return "do_cloak";
    if ( skill == do_close )		return "do_close";
    if ( skill == do_cmdtable )		return "do_cmdtable";
    if ( skill == do_commands )		return "do_commands";
    if ( skill == do_comment )		return "do_comment";
    if ( skill == do_compare )		return "do_compare";
    if ( skill == do_config )		return "do_config";
    if ( skill == do_construct )        return "do_construct";
    if ( skill == do_confuse )          return "do_confuse";
    if ( skill == do_copyover )     return "do_copyover";
    if ( skill == do_credits )		return "do_credits";
    if ( skill == do_cset )		return "do_cset";
    if ( skill == do_deny )		return "do_deny";
    if ( skill == do_deploy )		return "do_deploy";
    if ( skill == do_description )	return "do_description";
    if ( skill == do_destro )		return "do_destro";
    if ( skill == do_destroy )		return "do_destroy";
    if ( skill == do_dig )		return "do_dig";
    if ( skill == do_disconnect )	return "do_disconnect";
    if ( skill == do_dmesg )		return "do_dmesg";
    if ( skill == do_dnd )		return "do_dnd";
    if ( skill == do_down )		return "do_down";
    if ( skill == do_drag )		return "do_drag";
    if ( skill == do_drop )		return "do_drop";
    if ( skill == do_east )           return "do_east";
    if ( skill == do_eat )		return "do_eat";
    if ( skill == do_echo )		return "do_echo";
    if ( skill == do_emote )		return "do_emote";
    if ( skill == do_eject )        return "do_eject";
    if ( skill == do_empty )		return "do_empty";
    if ( skill == do_enter )		return "do_enter";
    if ( skill == do_equipment )	return "do_equipment";
    if ( skill == do_examine )		return "do_examine";
    if ( skill == do_exits )		return "do_exits";
    if ( skill == do_fixchar )		return "do_fixchar";
    if ( skill == do_fire )             return "do_fire";
    if ( skill == do_foldarea )		return "do_foldarea";
    if ( skill == do_follow )		return "do_follow";
    if ( skill == do_for )		return "do_for";
    if ( skill == do_force )		return "do_force";
    if ( skill == do_forceclose )	return "do_forceclose";
    if ( skill == do_form_password )	return "do_form_password";
    if ( skill == do_fquit )		return "do_fquit";
    if ( skill == do_freeze )		return "do_freeze";
    if ( skill == do_get )		return "do_get";
    if ( skill == do_give )		return "do_give";
    if ( skill == do_show )     return "do_show";
    if ( skill == do_glance )		return "do_glance";
    if ( skill == do_goto )		return "do_goto";
/*    if ( skill == do_grub )           return "do_grub"; */
    if ( skill == do_hedit )		return "do_hedit";
    if ( skill == do_hell )		return "do_hell";
    if ( skill == do_help )		return "do_help";
    if ( skill == do_freevnums )        return "do_freevnums";
    if ( skill == do_hide )		return "do_hide";
    if ( skill == do_hlist )		return "do_hlist";
    if ( skill == do_holylight )	return "do_holylight";
    if ( skill == do_homepage )		return "do_homepage";
    if ( skill == do_hset )		return "do_hset";
    if ( skill == do_ide )		return "do_ide";
    if ( skill == do_complain )         return "do_complain";
    if ( skill == do_idea )		return "do_idea";
    if ( skill == do_immortalize )	return "do_immortalize";  
    if ( skill == do_immtalk )		return "do_immtalk";
    if ( skill == do_installarea )	return "do_installarea";
    if ( skill == do_instaroom )	return "do_instaroom";
    if ( skill == do_instazone )	return "do_instazone";
    if ( skill == do_inventory )	return "do_inventory";
    if ( skill == do_invis )		return "do_invis";
    if ( skill == do_kill )		return "do_kill";
    if ( skill == do_kneel )            return "do_kneel";
    if ( skill == do_lob )              return "do_lob";
    if ( skill == do_lunge )            return "do_lunge";
    if ( skill == do_lstat )    return "do_lstat";
    if ( skill == do_last )		return "do_last";
    if ( skill == do_leave )		return "do_leave";
    if ( skill == do_leap )             return "do_leap";
    if ( skill == do_level )		return "do_level";
    if ( skill == do_list )		return "do_list";
    if ( skill == do_loadarea )		return "do_loadarea";
    if ( skill == do_loadup )		return "do_loadup";
    if ( skill == do_lock )		return "do_lock";
    if ( skill == do_log )		return "do_log";
    if ( skill == do_look )		return "do_look";
    if ( skill == do_low_purge )	return "do_low_purge";
    if ( skill == do_mailroom )		return "do_mailroom";
    if ( skill == do_menu )             return "do_menu";
    if ( skill == do_delay )            return "do_delay";
    if ( skill == do_makeboard )	return "do_makeboard";
    if ( skill == do_makebot )          return "do_makebot";
    if ( skill == do_makeshop )		return "do_makeshop";
    if ( skill == do_makewizlist )	return "do_makewizlist";
    if ( skill == do_massign )		return "do_massign";
    if ( skill == do_mcreate )		return "do_mcreate";
    if ( skill == do_mdelete )		return "do_mdelete";
    if ( skill == do_memory )		return "do_memory";
    if ( skill == do_mfind )        return "do_mfind";
    if ( skill == do_minvoke )		return "do_minvoke";
    if ( skill == do_mlist )		return "do_mlist";
    if ( skill == do_mp_close_passage )	return "do_mp_close_passage";
    if ( skill == do_mpvar )            return "do_mpvar";
    if ( skill == do_mpblast )          return "do_mpblast";
    if ( skill == do_mpcycle )          return "do_mpcycle";
    if ( skill == do_mpwait )           return "do_mpwait";
    if ( skill == do_mpadd )            return "do_mpadd";
    if ( skill == do_mpsub )            return "do_mpsub";
    if ( skill == do_mpmonitor )        return "do_mpmonitor";
    if ( skill == do_mpteamgain )       return "do_mpteamgain";
    if ( skill == do_mprmflag )         return "do_mprmflag";
    if ( skill == do_mpreset )          return "do_mpreset";
    if ( skill == do_mprat )            return "do_mprat";
    if ( skill == do_mp_damage )	return "do_mp_damage";
    if ( skill == do_mp_open_passage )	return "do_mp_open_passage";
    if ( skill == do_mp_practice )	return "do_mp_practice";
    if ( skill == do_mp_restore )	return "do_mp_restore";
    if ( skill == do_mp_slay )		return "do_mp_slay";
    if ( skill == do_mpadvance )	return "do_mpadvance";
    if ( skill == do_mpapply )		return "do_mpapply";
    if ( skill == do_mpapplyb )		return "do_mpapplyb";
    if ( skill == do_mpasound )		return "do_mpasound";
    if ( skill == do_mpat )		return "do_mpat";
    if ( skill == do_mpdream )		return "do_mpdream";
    if ( skill == do_mpecho )		return "do_mpecho";
    if ( skill == do_mprunspec )        return "do_mprunspec";
    if ( skill == do_mpechoaround )	return "do_mpechoaround";
    if ( skill == do_mpechoat )		return "do_mpechoat";
    if ( skill == do_mpedit )		return "do_mpedit";
    if ( skill == do_mpgain )		return "do_mpgain";
    if ( skill == do_mpforce )		return "do_mpforce";
    if ( skill == do_mpgoto )		return "do_mpgoto";
    if ( skill == do_mpinvis )		return "do_mpinvis";
    if ( skill == do_mpjunk )		return "do_mpjunk";
    if ( skill == do_mpkill )		return "do_mpkill";
    if ( skill == do_mpmload )		return "do_mpmload";
    if ( skill == do_mpnothing )	return "do_mpnothing";
    if ( skill == do_mpoload )		return "do_mpoload";
    if ( skill == do_mppkset )		return "do_mppkset";
    if ( skill == do_mppurge )		return "do_mppurge";
    if ( skill == do_mpstat )		return "do_mpstat";
    if ( skill == do_mptransfer )	return "do_mptransfer";
    if ( skill == do_mrange )		return "do_mrange";
    if ( skill == do_mset )		return "do_mset";
    if ( skill == do_mstat )		return "do_mstat";
    if ( skill == do_mwhere )		return "do_mwhere";
    if ( skill == do_name )		return "do_name";
    if ( skill == do_newbiechat )	return "do_newbiechat";
    if ( skill == do_newbieset )	return "do_newbieset";
    if ( skill == do_news )		return "do_news";
    if ( skill == do_newzones )		return "do_newzones";
    if ( skill == do_noemote )		return "do_noemote";
    if ( skill == do_noresolve )	return "do_noresolve";
    if ( skill == do_north )		return "do_north";
    if ( skill == do_northeast )	return "do_northeast";
    if ( skill == do_northwest )	return "do_northwest";
    if ( skill == do_notell )		return "do_notell";
    if ( skill == do_noteroom )		return "do_noteroom";
    if ( skill == do_oassign )		return "do_oassign";
    if ( skill == do_ocreate )		return "do_ocreate";
    if ( skill == do_odelete )		return "do_odelete";
    if ( skill == do_ofind )		return "do_ofind";
/*    if ( skill == do_ogrub )		return "do_ogrub";
*/    if ( skill == do_oinvoke )		return "do_oinvoke";
    if ( skill == do_oldscore )		return "do_oldscore";
    if ( skill == do_olist )		return "do_olist";
    if ( skill == do_opedit )		return "do_opedit";
    if ( skill == do_open )		return "do_open";
/*  if ( skill == do_opentourney )	return "do_opentourney"; */
    if ( skill == do_opstat )		return "do_opstat";
    if ( skill == do_orange )		return "do_orange";
    if ( skill == do_oset )		return "do_oset";
    if ( skill == do_ostat )		return "do_ostat";
    if ( skill == do_owhere )		return "do_owhere";
    if ( skill == do_pager )		return "do_pager";
    if ( skill == do_password )		return "do_password";
    if ( skill == do_practice )		return "do_practice";
    if ( skill == do_prompt )		return "do_prompt";
    if ( skill == do_prone )            return "do_prone";
    if ( skill == do_purge )		return "do_purge";
    if ( skill == do_put )		return "do_put";
    if ( skill == do_qui )		return "do_qui";
    if ( skill == do_quit )		return "do_quit";
    if ( skill == do_rassign )		return "do_rassign";
    if ( skill == do_rat )		return "do_rat";
    if ( skill == do_rdelete )		return "do_rdelete";
    if ( skill == do_reboo )		return "do_reboo";
    if ( skill == do_reboot )		return "do_reboot";
    if ( skill == do_recho )		return "do_recho";
    if ( skill == do_redit )		return "do_redit";
    if ( skill == do_rcopy )        return "do_rcopy";
    if ( skill == do_rpcopy )           return "do_rpcopy";
    if ( skill == do_regoto )		return "do_regoto";
    if ( skill == do_remove )		return "do_remove";
    if ( skill == do_reply )		return "do_reply";
    if ( skill == do_rescue )		return "do_rescue";
    if ( skill == do_reset )		return "do_reset";
    if ( skill == do_restore )		return "do_restore";
    if ( skill == do_prestore )         return "do_prestore";
    if ( skill == do_restoretime )	return "do_restoretime";
    if ( skill == do_restrict )		return "do_restrict";
    if ( skill == do_retran )		return "do_retran";
    if ( skill == do_return )		return "do_return";
    if ( skill == do_rdig )             return "do_rdig";
    if ( skill == do_rip )		return "do_rip";
    if ( skill == do_rlist )		return "do_rlist";
    if ( skill == do_rfill )            return "do_rfill";
    if ( skill == do_rpedit )		return "do_rpedit";
    if ( skill == do_rpstat )		return "do_rpstat";
    if ( skill == do_rreset )		return "do_rreset";
    if ( skill == do_rstat )		return "do_rstat";
    if ( skill == do_save )		return "do_save";
    if ( skill == do_saveall )          return "do_saveall";
    if ( skill == do_savearea )		return "do_savearea";
    if ( skill == do_say )		return "do_say";
    if ( skill == do_score )		return "do_score";
    if ( skill == do_sc )               return "do_sc";
    if ( skill == do_sedit )		return "do_sedit";
    if ( skill == do_sell )		return "do_sell";
    if ( skill == do_set_boot_time )	return "do_set_boot_time";
    if ( skill == do_shops )		return "do_shops";
    if ( skill == do_shopset )		return "do_shopset";
    if ( skill == do_shopstat )		return "do_shopstat";
    if ( skill == do_shout )		return "do_shout";
    if ( skill == do_shove )		return "do_shove";
    if ( skill == do_shutdow )		return "do_shutdow";
    if ( skill == do_shutdown )		return "do_shutdown";
    if ( skill == do_silence )		return "do_silence";
    if ( skill == do_sit )		return "do_sit";
    if ( skill == do_sla )		return "do_sla";
    if ( skill == do_slay )		return "do_slay";
    if ( skill == do_slist )		return "do_slist";
    if ( skill == do_slookup )		return "do_slookup";
    if ( skill == do_snoop )		return "do_snoop";
    if ( skill == do_sober )		return "do_sober";
    if ( skill == do_socials )		return "do_socials";
    if ( skill == do_south )		return "do_south";
    if ( skill == do_southeast )	return "do_southeast";
    if ( skill == do_southwest )	return "do_southwest";
    if ( skill == do_speak )		return "do_speak";
    if ( skill == do_sset )		return "do_sset";
    if ( skill == do_steacher ) return "do_steacher";
    if ( skill == do_stack )            return "do_stack";
    if ( skill == do_mpset )            return "do_mpset";
    if ( skill == do_stand )		return "do_stand";
    if ( skill == do_scan )       return "do_scan";
    if ( skill == do_switch )		return "do_switch";
    if ( skill == do_tell )		return "do_tell";
    if ( skill == do_treat )            return "do_treat";
    if ( skill == do_time )		return "do_time";
    if ( skill == do_timecmd )		return "do_timecmd";
    if ( skill == do_track )		return "do_track";
    if ( skill == do_transfer )		return "do_transfer";
    if ( skill == do_trust )		return "do_trust";
    if ( skill == do_typo )		return "do_typo";
    if ( skill == do_unfoldarea )	return "do_unfoldarea";
    if ( skill == do_unload )           return "do_unload";
    if ( skill == do_unhell )		return "do_unhell";
    if ( skill == do_unlock )		return "do_unlock";
    if ( skill == do_unsilence )        return "do_unsilence";
    if ( skill == do_up )		return "do_up";
    if ( skill == do_uptime )		return "do_uptime";
    if ( skill == do_users )		return "do_users";
    if ( skill == do_vassign )          return "do_vassign";
    if ( skill == do_vnums )		return "do_vnums";
    if ( skill == do_vsearch )		return "do_vsearch";
    if ( skill == do_wear )		return "do_wear";
    if ( skill == do_weather )		return "do_weather";
    if ( skill == do_webserve )         return "do_webserve";
    if ( skill == do_west )		return "do_west";
    if ( skill == do_where )		return "do_where";
    if ( skill == do_whisper )		return "do_whisper";
    if ( skill == do_who )		return "do_who";
    if ( skill == do_whois )		return "do_whois";
    if ( skill == do_wizhelp )		return "do_wizhelp";
    if ( skill == do_wizlist )		return "do_wizlist";
    if ( skill == do_wizlock )		return "do_wizlock";
    if ( skill == do_yell )		return "do_yell";
    if ( skill == do_zones )		return "do_zones";
    return "reserved";
}

/*
 * Function used by qsort to sort skills
 */
int skill_comp( SKILLTYPE **sk1, SKILLTYPE **sk2 )
{
    SKILLTYPE *skill1 = (*sk1);
    SKILLTYPE *skill2 = (*sk2);

    if ( !skill1 && skill2 )
	return 1;
    if ( skill1 && !skill2 )
	return -1;
    if ( !skill1 && !skill2 )
	return 0;
    if ( skill1->type < skill2->type )
	return -1;
    if ( skill1->type > skill2->type )
	return 1;
    return strcmp( skill1->name, skill2->name );
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table()
{
    log_string( "Sorting skill table..." );
    qsort( &skill_table[1], top_sn-1, sizeof( SKILLTYPE * ),
		(int(*)(const void *, const void *)) skill_comp );
}


/*
 * Write skill data to a file
 */
void fwrite_skill( FILE *fpout, SKILLTYPE *skill )
{
	fprintf( fpout, "Name         %s~\n",	skill->name	);
	fprintf( fpout, "Type         %s\n",	skill_tname[skill->type]);
	if ( skill->minimum_position )
	  fprintf( fpout, "Minpos       %d\n",	skill->minimum_position );
	if ( skill->slot )
	  fprintf( fpout, "Slot         %d\n",	skill->slot	);
        if ( skill->reset )
          fprintf( fpout, "Rounds       %d\n",  skill->reset    );
        if ( skill->race != -1 )
          fprintf( fpout, "Race         %d\n",  skill->race     );
	if ( skill->skill_fun )
	  fprintf( fpout, "Code         %s\n",	skill_name(skill->skill_fun) );
	if ( skill->msg_off && skill->msg_off[0] != '\0' )
	  fprintf( fpout, "Wearoff      %s~\n",	skill->msg_off	);
	
	if ( skill->type != SKILL_HERB )
	{
	    fprintf( fpout, "Minlevel     %d\n",	skill->min_level	);
	}
	fprintf( fpout, "End\n\n" );
}

/*
 * Save the skill table to disk
 */
void save_skill_table()
{
    int x;
    FILE *fpout;

    if ( (fpout=fopen( SKILL_FILE, "w" )) == NULL )
    {
	bug( "Cannot open skills.dat for writting", 0 );
	perror( SKILL_FILE );
	return;
    }

    for ( x = 0; x < top_sn; x++ )
    {
	if ( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
	   break;
	fprintf( fpout, "#SKILL\n" );
	fwrite_skill( fpout, skill_table[x] );
    }
    fprintf( fpout, "#END\n" );
    fclose( fpout );
}

/*
 * Save the herb table to disk
 */
void save_herb_table()
{
    int x;
    FILE *fpout;

    if ( (fpout=fopen( HERB_FILE, "w" )) == NULL )
    {
	bug( "Cannot open herbs.dat for writting", 0 );
	perror( HERB_FILE );
	return;
    }

    for ( x = 0; x < top_herb; x++ )
    {
	if ( !herb_table[x]->name || herb_table[x]->name[0] == '\0' )
	   break;
	fprintf( fpout, "#HERB\n" );
	fwrite_skill( fpout, herb_table[x] );
    }
    fprintf( fpout, "#END\n" );
    fclose( fpout );
}

/*
 * Save the socials to disk
 */
void save_socials()
{
    FILE *fpout;
    SOCIALTYPE *social;
    int x;

    if ( (fpout=fopen( SOCIAL_FILE, "w" )) == NULL )
    {
	bug( "Cannot open socials.dat for writting", 0 );
	perror( SOCIAL_FILE );
	return;
    }

    for ( x = 0; x < 27; x++ )
    {
	for ( social = social_index[x]; social; social = social->next )
	{
	    if ( !social->name || social->name[0] == '\0' )
	    {
		bug( "Save_socials: blank social in hash bucket %d", x );
		continue;
	    }
	    fprintf( fpout, "#SOCIAL\n" );
	    fprintf( fpout, "Name        %s~\n",	social->name );
	    if ( social->char_no_arg )
		fprintf( fpout, "CharNoArg   %s~\n",	social->char_no_arg );
	    else
	        bug( "Save_socials: NULL char_no_arg in hash bucket %d", x );
	    if ( social->others_no_arg )
		fprintf( fpout, "OthersNoArg %s~\n",	social->others_no_arg );
	    if ( social->char_found )
		fprintf( fpout, "CharFound   %s~\n",	social->char_found );
	    if ( social->others_found )
		fprintf( fpout, "OthersFound %s~\n",	social->others_found );
	    if ( social->vict_found )
		fprintf( fpout, "VictFound   %s~\n",	social->vict_found );
	    if ( social->char_auto )
		fprintf( fpout, "CharAuto    %s~\n",	social->char_auto );
	    if ( social->others_auto )
		fprintf( fpout, "OthersAuto  %s~\n",	social->others_auto );
	    fprintf( fpout, "End\n\n" );
	}
    }
    fprintf( fpout, "#END\n" );
    fclose( fpout );
}

int get_skill( char *skilltype )
{
    if ( !str_cmp( skilltype, "Spell" ) )
      return SKILL_SPELL;
    if ( !str_cmp( skilltype, "Skill" ) )
      return SKILL_SKILL;
    if ( !str_cmp( skilltype, "Weapon" ) )
      return SKILL_WEAPON;
    if ( !str_cmp( skilltype, "Tongue" ) )
      return SKILL_TONGUE;
    if ( !str_cmp( skilltype, "Herb" ) )
      return SKILL_HERB;
    return SKILL_UNKNOWN;
}

/*
 * Save the commands to disk
 */
void save_commands()
{
    FILE *fpout;
    CMDTYPE *command;
    int x;

    if ( (fpout=fopen( COMMAND_FILE, "w" )) == NULL )
    {
	bug( "Cannot open commands.dat for writing", 0 );
	perror( COMMAND_FILE );
	return;
    }

    for ( x = 0; x < 126; x++ )
    {
	for ( command = command_hash[x]; command; command = command->next )
	{
	    if ( !command->name || command->name[0] == '\0' )
	    {
		bug( "Save_commands: blank command in hash bucket %d", x );
		continue;
	    }
	    fprintf( fpout, "#COMMAND\n" );
	    fprintf( fpout, "Name        %s~\n", command->name		);
	    fprintf( fpout, "Code        %s\n",	 skill_name(command->do_fun) );
	    fprintf( fpout, "Position    %d\n",	 command->position	);
	    fprintf( fpout, "Level       %d\n",	 command->level		);
	    fprintf( fpout, "Log         %d\n",	 command->log		);
            fprintf( fpout, "Ooc         %d\n",  command->ooc           );
	    fprintf( fpout, "End\n\n" );
	}
    }
    fprintf( fpout, "#END\n" );
    fclose( fpout );
}

SKILLTYPE *fread_skill( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    SKILLTYPE *skill;

    CREATE( skill, SKILLTYPE, 1 );
    
    skill->race = -1;

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

	case 'C':
	    if ( !str_cmp( word, "Code" ) )
	    {
		DO_FUN	  *dofun;
		char	  *w = fread_word(fp);
		
		fMatch = TRUE;
		if ( (dofun=skill_function(w)) != skill_notfound )
		   skill->skill_fun = dofun;
		else
		{
                   sprintf( buf, "fread_skill: unknown skill %s", w );
		   bug( buf, 0 );
		}
		break;
	    }
	    break;
 
	case 'E':
	    if ( !str_cmp( word, "End" ) )
		return skill;
	    break;
	    
	case 'M':
	    KEY( "Minlevel",	skill->min_level,	fread_number( fp ) );
	    KEY( "Minpos",	skill->minimum_position, fread_number( fp ) );
	    break;
	
	case 'N':
            KEY( "Name",	skill->name,		fread_string_nohash( fp ) );
	    break;

	case 'R':
            KEY( "Rounds",      skill->reset,           fread_number( fp ) );
            KEY( "Race",        skill->race,            fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Slot",	skill->slot,		fread_number( fp ) );
	    break;

	case 'T':
	    KEY( "Type",	skill->type,  get_skill(fread_word( fp ))  );
	    break;

	case 'W':
	    KEY( "Wearoff",	skill->msg_off,		fread_string_nohash( fp ) );
	    break;
	}
	
	if ( !fMatch )
	{
            sprintf( buf, "Fread_skill: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

void load_skill_table()
{
    FILE *fp;

    if ( ( fp = fopen( SKILL_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
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
                bug( "Load_skill_table: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "SKILL"      ) )
	    {
		if ( top_sn >= MAX_SKILL )
		{
		    bug( "load_skill_table: more skills than MAX_SKILL %d", MAX_SKILL );
		    fclose( fp );
		    return;
		}
		skill_table[top_sn++] = fread_skill( fp );
		continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_skill_table: bad section.", 0 );
		continue;
	    }
	}
	fclose( fp );
    }
    else
    {
	bug( "Cannot open skills.dat", 0 );
 	exit(0);
    }
}

void load_herb_table()
{
    FILE *fp;

    if ( ( fp = fopen( HERB_FILE, "r" ) ) != NULL )
    {
	top_herb = 0;
	for ( ;; )
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
		bug( "Load_herb_table: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "HERB"      ) )
	    {
		if ( top_herb >= MAX_HERB )
		{
		    bug( "load_herb_table: more herbs than MAX_HERB %d", MAX_HERB );
		    fclose( fp );
		    return;
		}
		herb_table[top_herb++] = fread_skill( fp );
		if ( herb_table[top_herb-1]->slot == 0 )
		    herb_table[top_herb-1]->slot = top_herb-1;
		continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_herb_table: bad section.", 0 );
		continue;
	    }
	}
	fclose( fp );
    }
    else
    {
	bug( "Cannot open herbs.dat", 0 );
 	exit(0);
    }
}

void fread_social( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    SOCIALTYPE *social;

    CREATE( social, SOCIALTYPE, 1 );

    for ( ;; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'C':
	    KEY( "CharNoArg",	social->char_no_arg,	fread_string_nohash(fp) );
	    KEY( "CharFound",	social->char_found,	fread_string_nohash(fp) );
	    KEY( "CharAuto",	social->char_auto,	fread_string_nohash(fp) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !social->name )
		{
		    bug( "Fread_social: Name not found", 0 );
		    free_social( social );
		    return;
		}
		if ( !social->char_no_arg )
		{
		    bug( "Fread_social: CharNoArg not found", 0 );
		    free_social( social );
		    return;
		}
		add_social( social );
		return;
	    }
	    break;

	case 'N':
	    KEY( "Name",	social->name,		fread_string_nohash(fp) );
	    break;

	case 'O':
	    KEY( "OthersNoArg",	social->others_no_arg,	fread_string_nohash(fp) );
	    KEY( "OthersFound",	social->others_found,	fread_string_nohash(fp) );
	    KEY( "OthersAuto",	social->others_auto,	fread_string_nohash(fp) );
	    break;

	case 'V':
	    KEY( "VictFound",	social->vict_found,	fread_string_nohash(fp) );
	    break;
	}
	
	if ( !fMatch )
	{
            sprintf( buf, "Fread_social: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

void load_socials()
{
    FILE *fp;

    if ( ( fp = fopen( SOCIAL_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
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
                bug( "Load_socials: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "SOCIAL"      ) )
	    {
                fread_social( fp );
	    	continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_socials: bad section.", 0 );
		continue;
	    }
	}
	fclose( fp );
    }
    else
    {
	bug( "Cannot open socials.dat", 0 );
 	exit(0);
    }
}

void fread_command( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    CMDTYPE *command;

    CREATE( command, CMDTYPE, 1 );

    for ( ;; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'C':
	    KEY( "Code",	command->do_fun,	skill_function(fread_word(fp)) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !command->name )
		{
		    bug( "Fread_command: Name not found", 0 );
		    free_command( command );
		    return;
		}
		if ( !command->do_fun )
		{
		    bug( "Fread_command: Function not found", 0 );
		    free_command( command );
		    return;
		}
                if ( !command->ooc ) command->ooc = 0;
		add_command( command );
		return;
	    }
	    break;

	case 'L':
	    KEY( "Level",	command->level,		fread_number(fp) );
	    KEY( "Log",		command->log,		fread_number(fp) );
	    break;

        case 'O':
            KEY( "Ooc",         command->ooc,           fread_number(fp) );
	    break;

	case 'N':
	    KEY( "Name",	command->name,		fread_string_nohash(fp) );
	    break;

	case 'P':
	    KEY( "Position",	command->position,	fread_number(fp) );
	    break;
	}
	
	if ( !fMatch )
	{
            sprintf( buf, "Fread_command: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

void load_commands()
{
    FILE *fp;

    if ( ( fp = fopen( COMMAND_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
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
                bug( "Load_commands: # not found.", 0 );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "COMMAND"      ) )
	    {
                fread_command( fp );
	    	continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_commands: bad section.", 0 );
		continue;
	    }
	}
	fclose( fp );
    }
    else
    {
	bug( "Cannot open commands.dat", 0 );
 	exit(0);
    }

}
