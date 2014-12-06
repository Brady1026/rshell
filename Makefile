RSHELLOBJS = src/obj/main.o
LSOBJS = src/obj/ls.o
CPOBJS = src/obj/cp.o
CC = g++
CFLAGS = -Wall -Werror -ansi -pedantic

all: rshell ls cp

rshell: $(RSHELLOBJS) | bin
	$(CC) $(CFLAGS) $(RSHELLOBJS) -o bin/rshell

ls: $(LSOBJS) | bin
	$(CC) $(CFLAGS) $(LSOBJS) -o bin/ls

cp: $(CPOBJS) | bin
	$(CC) $(CFLAGS) $(CPOBJS) -o bin/cp

src/obj/main.o : src/main.cpp | src/obj
	$(CC) $(CFLAGS) -c src/main.cpp -o src/obj/main.o

src/obj/ls.o : src/ls.cpp | src/obj
	$(CC) $(CFLAGS) -c src/ls.cpp -o src/obj/ls.o

src/obj/cp.o : src/cp.cpp | src/obj
	$(CC) $(CFLAGS) -c src/cp.cpp -o src/obj/cp.o

debug:	CFLAGS += -ggdb
debug: all

src/obj:
	mkdir src/obj

bin:
	mkdir bin

clean:
	\rm -r bin src/obj
