#################################################
# A simple Makefile that you should modify for your system.
# This makefile assumes that you have compiled scourge already.
#################################################

#################################################
# Where is sdl? (Get these by running sdl-config --cflags and sdl-config --libs)
#################################################
SDL_CFLAGS=-I/usr/include/SDL -D_REENTRANT
SDL_LIBS=-L/usr/lib -lSDL -lpthread

#################################################
# Where is opengl?
#################################################
GL_CFLAGS=
GL_LIBS=-lpthread -lSM -lICE -L/usr/X11R6/lib -lX11 -lXi -lXext -lXmu -lXt -ldl -lm -lz -lGL -lGLU -lz

#################################################
# Where are the scourge libs? I use the relative path in the hope of it working
# on other people systems. ;-)
# Beware, the order in SCOURGE_LIBS matters!!
#################################################
SCOURGE_SRC_DIR=../src
SCOURGE_CFLAGS = -I${SCOURGE_SRC_DIR}
SCOURGE_LIBS = \
	${SCOURGE_SRC_DIR}/constants.o\
	${SCOURGE_SRC_DIR}/persist.o\
	${SCOURGE_SRC_DIR}/render/libScourgeRender.a\
	${SCOURGE_SRC_DIR}/io/libScourgeIo.a

#################################################
# Shouldn't need to change this below this line.
#################################################

ifdef DEPLOY
EXTRA_CFLAGS = -O2
else
EXTRA_CFLAGS = -g
endif

CC=g++
CXX=g++
CXXFLAGS=${SCOURGE_CFLAGS} ${SDL_CFLAGS} ${GL_CFLAGS} ${EXTRA_CFLAGS} -Wall
LDFLAGS=-Wall ${SDL_LIBS} ${GL_LIBS}
# Executable name must match first .o file's base-name.
TARGETS=main

all: ${TARGETS}
main: main.o game.o graphics.o
	$(CC) $(CXXFLAGS) $(LDFLAGS) $^ -o main $(SCOURGE_LIBS)
main.o: main.cpp examplepreferences.h
game.o: game.cpp game.h
graphics.o: graphics.cpp graphics.h

clean:
	-rm -f *.o ${TARGETS}

