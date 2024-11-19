
CFLAGS= -I./ `sdl2-config --cflags`  -Wall -ggdb
CC = cc
INSTALL= install
AR = ar
RANLIB = ranlib

OBJS = draw.o font.o gui.o link.o std_dialog.o drop.o timer.o load_save.o menu.o

libgui.a: ${OBJS}
	rm -rf libgui.a
	${AR} -q -v libgui.a ${OBJS} 
	${RANLIB} libgui.a

timer.o: timer.c gui_types.h link.h gui.h std_dialog.h draw.h
	${CC} -c timer.c ${CFLAGS}

draw.o: draw.c gui_types.h link.h gui.h draw.h
	${CC} -c draw.c ${CFLAGS}

font.o: font.c gui_types.h link.h gui.h draw.h std_dialog.h font.h
	${CC} -c font.c ${CFLAGS}

gui.o: gui.c gui_types.h link.h gui.h draw.h std_dialog.h
	${CC} -c gui.c ${CFLAGS}

link.o: link.c link.h
	${CC} -c link.c ${CFLAGS}

std_dialog.o: std_dialog.c gui_types.h link.h gui.h draw.h std_dialog.h font.h
	${CC} -c std_dialog.c ${CFLAGS}

drop.o: drop.c drop.h
	${CC} -c drop.c ${CFLAGS}

load_save.o: load_save.c load_save.h
	${CC} -c load_save.c ${CFLAGS}

menu.o: menu.c menu.h

clean:
	rm -rf ${OBJS} realpath.o libgui.a example

