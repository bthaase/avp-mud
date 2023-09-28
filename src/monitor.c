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
*                        Slaved Process Monitor                            *
****************************************************************************/

#include "mud.h"
#include <pthread.h>

#define MONITOR_TIMEOUT 10

int             pmPulse;          // toggle
pthread_t       pmChild;
int             pmRun;

extern bool mud_down;

int pmUpdate()
{
 struct timeval timewait;

 while(!mud_down)
 {
   timewait.tv_sec  = MONITOR_TIMEOUT;
   timewait.tv_usec = 0;

   /* select wait */
   select(0, NULL, NULL, NULL, &timewait);
  
   /* deadlock check */
   if( pmPulse == 1 )
   {
       bug("Monitor Slave is terminating this defunct program.");
       exit(-1);          // aggressive negotiations
   }
   else
   {
       pmPulse = 1;
   }
 }

 pmRun = 0;
 pthread_exit(0);		// Die, thread, die.
 return 0;			// just for the compiler.
}

int pmStart( bool force )
{
 if( pmRun == 1 && !force ) return -1;

 // if( pthread_create( pmChild, NULL, pmUpdate, NULL ) == EAGAIN ) return -1;
 pthread_create( &pmChild, NULL, pmUpdate, NULL );

 pmRun = 1;

 return 0;
}

int pmStop()
{
 if(pmRun == 0) return -1;

 // if( pthread_cancel( &pmChild ) == ESRCH ) return -1;
 pthread_cancel( &pmChild );

 pmRun = 0;

 return 0;
}

int pmReset()
{
 pmPulse = 0;

 return 0;
}
