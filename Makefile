ifeq ($(PREFIX),)
    PREFIX := /usr/bin
endif

CC = cc
LEX = lex
CFLAGS = -I./gui/ `sdl2-config --cflags` -O2 -Wall -ggdb -DDOXRANDR
LIBS= `sdl2-config --libs` -O2 -ggdb -lSDL2_image ./gui/libgui.a -lm
PROGNAME=mega

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    LIBS += -lm
endif

OBJS = draw.o mega.o proc.o vdp.o bottom.o lex.yy.o uu.o bmp.o gui/libgui.a

mega all: ${OBJS}
	${CC} -o ${PROGNAME}  ${OBJS} ${LIBS}

lex.yy.o: lex.yy.c mega_file.h
	${CC} -c ${CFLAGS} lex.yy.c

lex.yy.c: mega_file.l mega_file.h
	${LEX} mega_file.l

uu.o: uu.c 
	${CC} -c ${CFLAGS} uu.c

draw.o: draw.c mega.h vdp.h draw.h
	${CC} -c ${CFLAGS} draw.c

mega.o: mega.c mega.h vdp.h proc.h
	${CC} -c ${CFLAGS} mega.c

proc.o: proc.c mega.h vdp.h draw.h proc.h
	${CC} -c ${CFLAGS} proc.c

vdp.o: vdp.c mega.h vdp.h
	${CC} -c ${CFLAGS} vdp.c

bottom.o: bottom.c bottom.h 
	${CC} -c ${CFLAGS} bottom.c

gui/libgui.a:
	cd gui && make


install: mega
	install mega $(DESTDIR)$(PREFIX)/mega

clean:
	cd gui && make clean
	rm -f mega mega.exe mega.core core mega.res  lex.yy.c ${OBJS}
