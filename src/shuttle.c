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
                               Shuttle Module
****************************************************************************/

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

SHIP_DATA* first_ship;
SHIP_DATA* last_ship;

MISSILE_DATA* first_missile;
MISSILE_DATA* last_missile;

SPACE_DATA* first_starsystem;
SPACE_DATA* last_starsystem;

/*
    Local Routines
*/
bool    land_bus         args( ( SHIP_DATA* ship, int destination ) );
void    launch_bus       args( ( SHIP_DATA* ship ) );

void land_express( SHIP_DATA* ship, int destination, char* argument )
{
    char       buf[MAX_STRING_LENGTH];

    if ( !land_bus( ship, destination ) )
    {
        sprintf( buf, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'", argument );
        echo_to_ship( AT_CYAN, ship, buf );
        echo_to_ship( AT_CYAN, ship, "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'" );
    }
    else
    {
        sprintf( buf,  "An electronic voice says, 'Welcome to %s'", argument );
        echo_to_ship( AT_CYAN, ship, buf );
        echo_to_ship( AT_CYAN, ship, "It continues, 'Please exit through the main ramp. Enjoy your stay.'" );
    }

    return;
}

void fx_ready_shuttle( SHIP_DATA* ship, char* argument )
{
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "%s prepares to launch, and refueling compartments close..", ship->name );
    echo_to_room( AT_YELLOW, get_room_index( ship->location ), buf );
    sprintf( buf, "A Annoucment echos from %s: 'Next stop: %s'", ship->name, argument );
    echo_to_room( AT_LBLUE, get_room_index( ship->location ), buf );
}

bool  land_bus( SHIP_DATA* ship, int destination )
{
    // char buf[MAX_STRING_LENGTH];
    /*
        if ( !ship_to_room( ship , destination ) )
        {
        return FALSE;
        }
        echo_to_ship( AT_YELLOW , ship , "You feel a slight thud as the ship sets down on the ground.");
        ship->location = destination;
        ship->lastdoc = ship->location;
        ship->shipstate = SHIP_DOCKED;
        if (ship->starsystem)
        ship_from_starsystem( ship, ship->starsystem );
        sprintf( buf, "%s lands on the platform.", ship->name );
        echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
        sprintf( buf , "The hatch on %s opens." , ship->name);
        echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
        echo_to_room( AT_YELLOW , get_room_index(ship->entrance) , "The hatch opens." );
        ship->hatchopen = TRUE;
        ship->hatchtimer = 0;
        sound_to_room( get_room_index(ship->entrance) , "!!SOUND(door)" );
        sound_to_room( get_room_index(ship->location) , "!!SOUND(door)" );
    */
    return TRUE;
}

void    launch_bus( SHIP_DATA* ship )
{
    // char buf[MAX_STRING_LENGTH];
    /*
        sound_to_room( get_room_index(ship->entrance) , "!!SOUND(door)" );
        sound_to_room( get_room_index(ship->location) , "!!SOUND(door)" );
        sprintf( buf , "The hatch on %s closes and it begins to launch." , ship->name);
        echo_to_room( AT_YELLOW , get_room_index(ship->location) , buf );
        echo_to_room( AT_YELLOW , get_room_index(ship->entrance) , "The hatch slides shut." );
        ship->hatchopen = FALSE;
        ship->hatchtimer = 0;
        extract_ship( ship );
        echo_to_ship( AT_YELLOW , ship , "The ship begins to launch.");
        ship->location = 0;
        ship->shipstate = SHIP_READY;
    */
}

void update_traffic( )
{
    // SHIP_DATA *shuttle, *senate;
    // SHIP_DATA *turbocar;
    // char       buf[MAX_STRING_LENGTH];
    /*
        shuttle = ship_from_cockpit( ROOM_CORUSCANT_SHUTTLE );
        senate = ship_from_cockpit( ROOM_SENATE_SHUTTLE );
        if ( senate != NULL && shuttle != NULL )
        {
        switch (corus_shuttle)
        {
             default:
                corus_shuttle++;
                break;

             case 0:
                land_bus( shuttle , STOP_PLANET );
                land_bus( senate , SENATEPAD );
                corus_shuttle++;
                echo_to_ship( AT_CYAN , shuttle , "Welcome to Menari Spaceport." );
                echo_to_ship( AT_CYAN , senate , "Welcome to The Senate Halls." );
                break;

             case 4:
                launch_bus( shuttle );
                launch_bus( senate );
                corus_shuttle++;
                break;

             case 5:
                land_bus( shuttle , STOP_SHIPYARD );
                land_bus( senate , OUTERPAD );
                echo_to_ship( AT_CYAN , shuttle , "Welcome to Coruscant Shipyard." );
                echo_to_ship( AT_CYAN , senate , "Welcome to The Outer System Landing Area." );
                corus_shuttle++;
                break;

             case 9:
                launch_bus( shuttle );
                launch_bus( senate );
                corus_shuttle++;
                break;

        }

        if ( corus_shuttle >= 10 )
              corus_shuttle = 0;

        }

        turbocar = ship_from_cockpit( ROOM_CORUSCANT_TURBOCAR );
        if ( turbocar != NULL )
        {
        sprintf( buf , "The turbocar doors close and it speeds out of the station.");
        echo_to_room( AT_YELLOW , get_room_index(turbocar->location) , buf );
        extract_ship( turbocar );
        turbocar->location = 0;
        ship_to_room( turbocar , station_vnum[turbocar_stop] );
        echo_to_ship( AT_YELLOW , turbocar , "The turbocar makes a quick journey to the next station.");
        turbocar->location = station_vnum[turbocar_stop];
        turbocar->lastdoc = turbocar->location;
        turbocar->shipstate = SHIP_DOCKED;
        if (turbocar->starsystem)
          ship_from_starsystem( turbocar, turbocar->starsystem );
        sprintf( buf, "A turbocar pulls into the platform and the doors slide open.");
        echo_to_room( AT_YELLOW , get_room_index(turbocar->location) , buf );
        sprintf( buf, "Welcome to %s." , station_name[turbocar_stop] );
        echo_to_ship( AT_CYAN , turbocar , buf );
        turbocar->hatchopen = TRUE;
        turbocar->hatchtimer = 0;

        turbocar_stop++;
        if ( turbocar_stop >= MAX_STATION )
           turbocar_stop = 0;
        }
    */
}

void update_bus( )
{
    // SHIP_DATA *ship;
    // SHIP_DATA *ship2;
    // SHIP_DATA *shuttle1;
    // SHIP_DATA *target;
    // int        destination;
    // char       buf[MAX_STRING_LENGTH];
    /*
        ship = ship_from_cockpit( ROOM_SHUTTLE_BUS );
        ship2 = ship_from_cockpit( ROOM_SHUTTLE_BUS_2 );

        shuttle1 = ship_from_cockpit( ROOM_SHUTTLE1 );

        if ( ship == NULL || ship2 == NULL )
        {
        // bug("WARNING: Tocca/Plugous Malfunction. Does not exist.");
        return;
        }
        if ( shuttle1 == NULL )
        {
        // bug("WARNING: Crossrunner Express EX00 Does not EXIST!");
        return;
        }
    */
    /*
        Tocca/Plugous will be disabled while being rebuilt.
        STAGE NOTES:  0) Check Landing Pad
                     1) Land Bus
                     5) 'Next Stop' FX
                     6) Launch Bus
                     7) Hyperdrive FX
                     9) Exit Hyperdrive FX
    */
    /*
        switch (bus_pos)
        {
        case 0:
            target = ship_from_hangar( bus_vnum[bus_planet] );
            if ( target != NULL && !target->starsystem )
            {
               sprintf( buf,  "An electronic voice says, 'Cannot land at %s ... it seems to have dissapeared.'", bus_stop[bus_planet] );
               echo_to_ship( AT_CYAN , ship , buf );
               bus_pos = 5;
            }

            target = ship_from_hangar( bus_vnum[bus2_planet] );
            if ( target != NULL && !target->starsystem )
            {
               sprintf( buf,  "An electronic voice says, 'Cannot land at %s ... it seems to have dissapeared.'", bus_stop[bus_planet] );
               echo_to_ship( AT_CYAN , ship2 , buf );
               bus_pos = 5;
            }

            bus_pos++;
            break;

        case 6:
            launch_bus ( ship );
            launch_bus ( ship2 );
            bus_pos++;
            break;

        case 7:
            echo_to_ship( AT_YELLOW , ship , "The ship lurches slightly as it makes the jump to lightspeed.");
            echo_to_ship( AT_YELLOW , ship2 , "The ship lurches slightly as it makes the jump to lightspeed.");
            bus_pos++;
            break;

        case 9:

            echo_to_ship( AT_YELLOW , ship , "The ship lurches slightly as it comes out of hyperspace..");
            echo_to_ship( AT_YELLOW , ship2 , "The ship lurches slightly as it comes out of hyperspace..");
            bus_pos++;
            break;

        case 1:
            destination = bus_vnum[bus_planet];
            if ( !land_bus( ship, destination ) )
            {
               sprintf( buf, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'" , bus_stop[bus_planet] );
               echo_to_ship( AT_CYAN , ship , buf );
               echo_to_ship( AT_CYAN , ship , "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'");
            }
            else
            {
               sprintf( buf,  "An electronic voice says, 'Welcome to %s'" , bus_stop[bus_planet] );
               echo_to_ship( AT_CYAN , ship , buf);
               echo_to_ship( AT_CYAN , ship , "It continues, 'Please exit through the main ramp. Enjoy your stay.'");
            }
            destination = bus_vnum[bus2_planet];
            if ( !land_bus( ship2, destination ) )
            {
               sprintf( buf, "An electronic voice says, 'Oh My, %s seems to have dissapeared.'" , bus_stop[bus_planet] );
               echo_to_ship( AT_CYAN , ship2 , buf );
               echo_to_ship( AT_CYAN , ship2 , "An electronic voice says, 'I do hope it wasn't a superlaser. Landing aborted.'");
            }
            else
            {
               sprintf( buf,  "An electronic voice says, 'Welcome to %s'" , bus_stop[bus2_planet] );
               echo_to_ship( AT_CYAN , ship2 , buf);
               echo_to_ship( AT_CYAN , ship2 , "It continues, 'Please exit through the main ramp. Enjoy your stay.'");
            }

            bus_pos++;
            break;

        case 5:
            sprintf( buf, "It continues, 'Next stop, %s'" , bus_stop[bus_planet+1] );
            echo_to_ship( AT_CYAN , ship , "An electronic voice says, 'Preparing for launch.'");
            echo_to_ship( AT_CYAN , ship , buf);
            sprintf( buf, "It continues, 'Next stop, %s'" , bus_stop[bus2_planet+1] );
            echo_to_ship( AT_CYAN , ship2 , "An electronic voice says, 'Preparing for launch.'");
            echo_to_ship( AT_CYAN , ship2 , buf);
            bus_pos++;
            break;

        default:
            bus_pos++;
            break;
        }

        if ( bus_pos >= 10 )
        {
        bus_pos = 0;
        bus_planet++;
        bus2_planet++;
        }

        if ( bus_planet >= MAX_BUS_STOP )    bus_planet = 0;
        if ( bus2_planet >= MAX_BUS_STOP )   bus2_planet = 0;
    */
}

