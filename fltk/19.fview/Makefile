CC = g++
CFLAGS = `fltk-config --cxxflags --use-images`
LDFLAGS = `fltk-config --ldflags --use-images`
PROG = build/linux/fview
SRC = src/main.cxx
WINFLAGS = -I/usr/local/include -mwindows -DWIN32 -DUSE_OPENGL32 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -mwindows /usr/local/lib/libfltk_images.a -lpng -lz -ljpeg /usr/local/lib/libfltk.a -lole32 -luuid -lcomctl32

all: fview

fview: 
	${CC} $(SRC) -o $(PROG) ${CFLAGS} ${LDFLAGS} 
	cp data/*.png build/linux

clean: 
	rm -f build/linux/*

win: 
	${CC} -o 'build/windows/fview.exe' $(SRC) ${WINFLAGS}
	cp lib/*.dll build/windows/
