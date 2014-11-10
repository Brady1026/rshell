RSHELLOBJS = src/obj/main.o
CPOBJS = src/obj/cp.o
CC = g++
CFLAGS = -Wall -Werror -ansi -pedantic

all: rshell cp

rshell: $(RSHELLOBJS) | bin
	$(CC) $(CFLAGS) $(RSHELLOBJS) -o bin/rshell

cp: $(CPOBJS) | bin
	$(CC) $(CFLAGS) $(CPOBJS) -o bin/cp

src/obj/cp.o : src/cp.cpp | src/obj
	$(CC) $(CFLAGS) -c src/cp.cpp -o src/obj/cp.o

src/obj/main.o : src/main.cpp | src/obj
	$(CC) $(CFLAGS) -c src/main.cpp -o src/obj/main.o

debug:	CFLAGS += -ggdb
debug: all

src/obj:
	mkdir src/obj

bin:
	mkdir bin

clean:
	\rm -r bin src/obj
