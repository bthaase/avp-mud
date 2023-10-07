CC      = gcc
PROF    = 
NOCRYPT =

#Uncomment the next line if you want request support
#DBUGFLG = -DREQUESTS
DEBUG   = -DDEBUG -DMC_MEM_INTRO -DMEMORY_HASH=101 -ggdb
C_FLAGS = -fsigned-char -g3 -Wall $(PROF) $(NOCRYPT) $(DBUGFLG) $(DEBUG)
L_FLAGS = $(PROF) -lcrypt -lm -lpthread -lbacktrace -lcrypt -lsystemd -lmosquitto
#D_FLAGS : For the DNS Slave process. No need in linking all the extra libs for this.
D_FLAGS = -g3 -O $(PROF) $(SOLARIS_LINK)

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o arena.o \
          boards.o bots.o build.o comm.o comments.o const.o db.o dns.o \
      fight.o greet.o handler.o hashstr.o id.o mapout.o makeobjs.o save.o interp.o tables.o \
      misc.o mud_comm.o mud_prog.o player.o requests.o pfiles.o \
      reset.o shops.o skills.o special.o track.o update.o upgrades.o \
      swskills.o websrv.o backtrace.o mqtt.o

C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c arena.c \
          boards.c bots.c build.c comm.c comments.c const.c db.c dns.c \
      fight.c greet.c handler.c hashstr.c id.c mapout.c makeobjs.c save.c interp.c tables.c \
      misc.c mud_comm.c mud_prog.c player.c requests.c pfiles.c \
      reset.c shops.c skills.c special.c track.c update.c upgrades.c \
      swskills.c websrv.c backtrace.c mqtt.c

H_FILES = mud.h bet.h pfiles.h mqtt.h

all:
	make avp
#       rm -f $(H_FILES)

avp: $(O_FILES)
	rm -f avp
	$(CC) -o avp $(O_FILES) $(L_FLAGS)
	chmod g+w avp
	chmod g+w $(O_FILES)

dns: resolver.o
	rm -f resolver
	$(CC) $(D_FLAGS) -o resolver resolver.o
	chmod g+w resolver
	chmod a+x resolver
	chmod g+w resolver.o

.c.o: mud.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f $(O_FILES)
