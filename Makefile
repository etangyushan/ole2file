LD = -lm -lpcap -pthread -ldl -ljansson -luuid -lzlog
DEB = -Wall -g
CC = gcc 
TARGET = all
PKG_CFLAGS = `pkg-config --cflags glib-2.0`
PKG_LIBS = `pkg-config --libs glib-2.0`

LIBS = $(LD) $(PKG_LIBS)
CFLAGS = $(PKG_CFLAGS)

SRC = main.o ole2.o

all:parser
parser:$(SRC)
	$(CC) -o parser $(SRC) $(LIBS) $(DEB) 
%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(DEB)
clean:
	rm -f parser *.o *.tmp
