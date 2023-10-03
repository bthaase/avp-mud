# Alien vs. Predator: Legends

## Introduction
AVP: Legends was a MUD I created back around 2003, I think I was around 16-17 at the time, so don't hold the code quality against me, haha. It descends from SWR, a descendant of SMAUG. 

## Disclaimer
This code is provided as-is. I haven't touched it in two decades now, and remember very little about how to setup or operate the MUD. I also made only a quick pass to scrub personal email addresses and old accounts, so I don't know what else you'll find in these files. 

## Running the Server
- Install the required packages
    - `apt-get install make g++ libz-dev`
- Optional install for debugging
    - `apt-get install gdb`
- Compile the source code (Expect a lot of warnings)
    - `cd src`
    - `make`
- Start the server
    - Make sure you are in the root directory.
    - Execute `./run.sh` - Edit this file if you want to change the port. (Default is 7000)

I do NOT recommend using the old `src/avpscript`, it has not be updated.

# Troubleshooting
Some other things I tried when getting this up and running, unsure what all is required.
- Most directories besides `src` should have both read and write permissions.
- The compiled binary needs to have it's working directory in area, so for a manual start:
    - `cd area`
    - `../src/avp [port]`
- There is a `./debug.sh` script in the root, that will start the server inside GDB.
