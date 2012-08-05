
CFLAGS= -I./ `sdl-config --cflags` -Wall -ggdb
CC = cc
INSTALL= install
AR = ar
RANLIB = ranlib

OBJS = draw.o font.o gui.o link.o std_dialog.o drop.o timer.o load_save.o menu.o

libgui.a: ${OBJS}
	rm -rf libgui.a
	${AR} -q -v libgui.a ${OBJS} 
	${RANLIB} libgui.a

timer.o: timer.c ../config.h gui_types.h link.h gui.h std_dialog.h draw.h
	${CC} -c timer.c ${CFLAGS}

draw.o: draw.c ../config.h gui_types.h link.h gui.h draw.h
	${CC} -c draw.c ${CFLAGS}

font.o: font.c ../config.h gui_types.h link.h gui.h draw.h std_dialog.h font.h
	${CC} -c font.c ${CFLAGS}

gui.o: gui.c ../config.h gui_types.h link.h gui.h draw.h std_dialog.h
	${CC} -c gui.c ${CFLAGS}

link.o: link.c ../config.h link.h
	${CC} -c link.c ${CFLAGS}

std_dialog.o: std_dialog.c ../config.h gui_types.h link.h gui.h draw.h std_dialog.h font.h
	${CC} -c std_dialog.c ${CFLAGS}

drop.o: drop.c ../config.h drop.h
	${CC} -c drop.c ${CFLAGS}

load_save.o: load_save.c ../config.h load_save.h
	${CC} -c load_save.c ${CFLAGS}

menu.o: menu.c ../config.h menu.h

clean:
	rm -rf ${OBJS} realpath.o libgui.a example

