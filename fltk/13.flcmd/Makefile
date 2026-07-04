CC = g++
CFLAGS = $(shell fltk-config --cxxflags)
LDFLAGS = $(shell fltk-config --ldflags --use-images)
SRC = 'src/main.cxx'
PROG = flcmd

all: flcmd

flcmd: 
	${CC} -g $(SRC) -o $(PROG) ${CFLAGS} ${LDFLAGS} -no-pie

clean: 
	rm flcmd
