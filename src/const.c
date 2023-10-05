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
                 Mud constants module
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"

/* undef these at EOF */
#define AM 95
#define AC 95
#define AT 85
#define AW 85
#define AV 95
#define AD 95
#define AR 90
#define AA 95

/*
    Race table.
*/
const   struct  race_type      race_table     [MAX_RACE]  =
{
    /* RACE NAME    MENU NAME    str sta int rec bra per  hit  mvm  LANGUAGE      */
    { "Marine",     "Human",     10, 10, 12, 10, 12, 10,  150, 100, LANG_MARINE, "Defensive race with incredible firepower." },
    { "Alien",      "Xenomorph", 12, 10, 10, 12, 10, 10,  150, 200, LANG_ALIEN, "Fastest, most potent close-range attacks." },
    { "Predator",   "Yautja",    10, 12, 10, 10, 10, 12,  300, 150, LANG_PREDATOR, "Deadly one-man armies. Very specialized." }
};

/*
    Curse words
*/
char*   const   curse_table[] =
{
    "fuck", "shit", "bitch", "bastard", "faggot", "pussy", "cock", "asshole",
    "tits", "cunt", "penis", "piss", "crap", "ass", "whore", "slut", "twat", "jackass",
    "anal", "dick", "damn", "hell", "prick", "balls", "dildo", "boner", "skank",
    "cum", "sex", "$"
};

/*
    Ignore words
    [Checks for these words first, so as to avoid filtering them]
*/
char*   const   ignore_table[] =
{
    "hello", "mass", "assault", "damnumroll", "assess", "assign", "pussycat",
    "pass", "scrap", "cockp", "lass", "cocker", "rass", "assa", "asso",
    "assi", "sass", "assu", "cockt", "cockr", "shell", "shite", "encumb",
    "asse", "$"
};

/*
    Quick Radio Functions
*/
char*   const   radio_set1[] =
{
    "Standard Messages", // Title of Menu
    "What the fuck are we gonna do now?",
    "Whatever your going to do, do it fast!",
    "Hey top, what's the op?",
    "I need a Medic!",
    "Friendly Fire! Friendly Fire!",
    "Game over, man! Game over!",
    "Fire in the hole!",
    "$"
};
char*   const   radio_set2[] =
{
    "Command Messages", // Title of Menu
    "Fall Back!",
    "Need Backup!",
    "Hold your ground!",
    "Nobody touch nothin'",
    "Box 'em in!",
    "Report in, Marines.",
    "Use short, controlled bursts!",
    "Marines, we are LEAVING!",
    "$"
};
char*   const   radio_set3[] =
{
    "Confirmation Messages", // Title of Menu
    "Acknowledged.",
    "Affirmitive!",
    "Negative.",
    "On my way!",
    "Be right there!",
    "Are you crazy?",
    "$"
};
char*   const   radio_set4[] =
{
    "Report Messages", // Title of Menu
    "Incoming Predator!",
    "Incoming Alien!",
    "Enemy Spotted!",
    "We have incoming!",
    "I'm surrounded!",
    "There above us!",
    "They cut the power!",
    "All Clear.",
    "$"
};

/*
    Ship classes
*/
char*   const   class_types[] =
{
    "Starfighter", "Transport", "Freighter", "Gunboat", "Corvette", "Frigate",
    "Cruiser", "Battleship", "Battlestation", "Ship Platform", "Cloud Car",
    "Ocean Ship", "Land Speeder", "Wheeled", "Land Crawler", "Walker"
};

/*
    Vehicle classes
*/
char*   const   vclass_types[] =
{
    "Cloud Car", "Ocean Ship", "Land Speeder", "Wheeled", "Land Crawler", "Walker"
};

char*   const   npc_race    [MAX_NPC_RACE] =
{
    "Marine", "Alien", "Predator"
};

const int weapon_accuracy[MAX_WEAPON_TYPES] =
{
    0, 25, 10, 10, 30, 20, 0, -25, 0, 0, 25, -5, -10, 10, 25, 0, 0, 0, 0, 0, 25
};

const  struct    quote_type    quote_table [MAX_QUOTES] =
{
    { "All battles are fought at the junction of two or more map sheets.", "Murphy's Laws of Combat" },
    { "If the enemy is in range, so are you.", "Murphy's Laws of Combat" },
    { "Incoming fire has the right of way.", "Murphy's Laws of Combat" },
    { "Dont look conspicuous... It draws fire.", "Murphy's Laws of Combat" },
    { "There is always a way.\n\r  The easy way is always mined.", "Murphy's Laws of Combat" },
    { "Try to look unimportant, they may be low on ammo.", "Murphy's Laws of Combat" },
    { "Teamwork is essential... It gives them someone else to shoot at.", "Murphy's Laws of Combat" },
    { "The tank is a monument to the inaccuracy of indirect fire.", "Murphy's Laws of Combat" },
    { "Never reinforce failure... failure reinforces itself.", "Murphy's Laws of Combat" },
    { "Odd objects attact fire. You are odd.", "Murphy's Laws of Combat" },
    { "If they are shooting at you, it is a high-intensity conflict.", "Murphy's Laws of Combat" },
    { "War is the unfolding of miscalculations.", "Murphy's Laws of Combat" },
    { "A sucking chest wound is natures way of saying 'Slow down'", "Murphy's Laws of Combat" },
    { "Never draw fire.... It irritates everyone around you.", "Murphy's Laws of Combat" },
    { "Friendly fire... Is'nt.", "Murphy's Laws of Combat" },
    { "No matter which way you have to march... Its always uphill.", "Murphy's Laws of Combat" },
    { "For every action, there is a equal and opposite criticism.", "Murphy's Laws of Combat" },
    { "If its stupid but it works... Then its not stupid.", "Murphy's Laws of Combat" },
    { "When in doubt.... Empty the magazine.", "Murphy's Laws of Combat" },
    { "If it flies... It dies...", "Murphy's Laws of Combat" },
    { "Never worry about the bullet with your name on it...\n\r  Worry about the shrapnel addressed to occupant.", "Murphy's Laws of Combat" },
    { "Tracers work BOTH WAYS.", "Murphy's Laws of Combat" },
    { "Military intelligence is a contradiction of terms.", "Murphy's Laws of Combat" },
    { "Priorities are made by officers, not by God. There IS a diffrenence.", "Murphy's Laws of Combat" },
    { "If you need a officer in a hurry... Take a nap.", "Murphy's Laws of Combat" },
    { "The weapon that usally jams when you need it most is yours.", "Murphy's Laws of Combat" },
    { "Theres too much blood in my caffine system!", "Unknown" },
    { "We have enough youth, how about a fountain of smart?", "Unknown" },
    { "Yeah, Right... Like their going to give beggars weapons! Bwhahahaha!", "Famous last words of Imperial Spies" },
    { "Always walk at the end of the line. Your less likely to get hit by the claymores.", "Great words of Advice" },
    { "You have two choices: Kill, or be Killed. Or maybe a third I have'nt thought of...", "Unknown" },
    { "Hey, This Doormat kind of looks like a mine! Wait-a-minute...", "Famous last words of Imperial Spies" },
    { "Can you say terrorism? I though you could!", "Mister Rodgers, Black Sun Inductee" },
    { "The complexity of a weapon is inversely proportional to the IQ of its operator.", "Murphy's Laws of Combat" }
};

/*
    The skill and spell table.
    Slot numbers must never be changed as they appear in #OBJECTS sections.
*/
#define SLOT(n) n
#define LI LEVEL_IMMORTAL

#undef AM
#undef AC
#undef AT
#undef AW
#undef AV
#undef AD
#undef AR
#undef AA

#undef LI
