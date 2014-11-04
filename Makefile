OBJS = src/obj/main.o src/obj/cp.o
CC = g++
CFLAGS = -Wall -Werror -ansi -pedantic

all: rshell cp

rshell: $(OBJS) | bin
	$(CC) $(CFLAGS) $(OBJS) -o bin/rshell

cp: $(OBJS) | bin
	$(CC) $(CFLAGS) $(OBJS) -o bin/cp

src/obj/cp.o : src/cp.cpp | src/obj
	$(CC) $(CFLGAS) -c src/cp.cpp -o src/obj/cp.o

src/obj/main.o : src/main.cpp | src/obj
	$(CC) $(CFLAGS) -c src/main.cpp -o src/obj/main.o

src/obj:
	mkdir src/obj

bin:
	mkdir bin

clean:
	\rm -r bin src/obj
