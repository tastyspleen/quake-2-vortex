#
# Quake2 gamei386.real.so Makefile for Linux
#
# Jan '98 by Zoid <zoid@idsoftware.com>
#
# ELF only
#
# Edited December 28, 2019 by QwazyWabbit
#
# Requires GNU make
#

# this nice line comes from the linux kernel makefile
ARCH := $(shell uname -m | sed -e s/i.86/i386/ \
	-e s/sun4u/sparc64/ -e s/arm.*/arm/ \
	-e s/sa110/arm/ -e s/alpha/axp/)

# On 64-bit OS use the command: setarch i386 make all
# to obtain the 32-bit binary DLL on 64-bit Linux.

CC = gcc -std=c11 -Wall

# on x64 machines do this preparation:
# sudo apt-get install ia32-libs
# sudo apt-get install libc6-dev-i386
# On Ubuntu 16.x use sudo apt install libc6-dev-i386
# this will let you build 32-bits on ia64 systems
#
# This is for native build
CFLAGS=-O3 -DARCH="$(ARCH)" -DSTDC_HEADERS
# This is for 32-bit build on 64-bit host
ifeq ($(ARCH),i386)
CFLAGS =-m32 -O3 -DARCH="$(ARCH)" -DSTDC_HEADERS -I/usr/include
endif

# use this when debugging
#CFLAGS=-g -Og -DDEBUG -DARCH="$(ARCH)" -Wall -pedantic

# flavors of Linux
ifeq ($(shell uname),Linux)
#SVNDEV := -D'SVN_REV="$(shell svnversion -n .)"'
#CFLAGS += $(SVNDEV)
CFLAGS += -DLINUX
LIBTOOL = ldd
endif

# OS X wants to be Linux and FreeBSD too.
ifeq ($(shell uname),Darwin)
#SVNDEV := -D'SVN_REV="$(shell svnversion -n .)"'
#CFLAGS += $(SVNDEV)
CFLAGS += -DLINUX
LIBTOOL = otool
endif

SHLIBEXT=so
#set position independent code
SHLIBCFLAGS=-fPIC

DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

.c.o:
	$(DO_SHLIB_CC)

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

GAME_OBJS = \
	ally.o armory.o auras.o backpack.o bombspell.o boss_general.o \
		boss_makron.o boss_tank.o class_brain.o class_demon.o cloak.o \
		ctf.o damage.o domination.o drone_ai.o drone_berserk.o drone_bitch.o \
		drone_brain.o drone_decoy.o drone_gladiator.o drone_gunner.o \
		drone_infantry.o drone_jorg.o drone_makron.o drone_medic.o \
		drone_misc.o drone_move.o drone_mutant.o drone_parasite.o \
		drone_soldier.o drone_supertank.o drone_tank.o ents.o \
		file_output.o flying_skull.o forcewall.o \
		g_chase.o g_cmds.o g_combat.o g_ctf.o g_flame.o g_freeze.o \
		g_func.o g_items.o g_main.o g_misc.o g_monster.o \
		g_phys.o g_save.o g_spawn.o g_svcmds.o g_sword.o g_target.o \
		g_trigger.o g_utils.o g_weapon.o \
		gds.o grid.o help.o invasion.o item_menu.o jetpack.o lasers.o \
		lasersight.o laserstuff.o m_flash.o magic.o maplist.o \
		menu.o minisentry.o misc_stuff.o \
		p_client.o p_hook.o p_hud.o p_menu.o p_parasite.o p_trail.o p_view.o \
		p_weapon.o player.o player_points.o playertoberserk.o playertoflyer.o \
		playertomedic.o playertomutant.o playertoparasite.o playertotank.o \
		pvb.o q_shared.o repairstation.o runes.o scanner.o \
		sentrygun2.o shaman.o special_items.o spirit.o sqlstuff.o \
		supplystation.o talents.o teamplay.o totems.o trade.o upgrades.o \
		v_file_io.o v_items.o v_maplist.o v_utils.o vote.o weapon_upgrades.o \
		weapons.o

game$(ARCH).real.$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(GAME_OBJS) -ldl -lm
	$(LIBTOOL) -r $@


#############################################################################
# MISC
#############################################################################

clean:
	/bin/rm -f $(GAME_OBJS)

depend:
	$(CC) -MM $(GAME_OBJS:.o=.c)

depends:
	$(CC) $(CFLAGS) -MM *.c > dependencies

all:
	make clean
	make depends
	make

-include dependencies

