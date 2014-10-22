OBJS = src/obj/main.o
CC = g++
CFLAGS = -Wall -Werror -ansi -pedantic

all: rshell

rshell: $(OBJS) | bin
	$(CC) $(CFLAGS) $(OBJS) -o bin/rshell

src/obj/main.o : src/main.cpp | src/obj
	$(CC) $(CFLAGS) -c src/main.cpp -o src/obj/main.o

src/obj:
	mkdir src/obj

bin:
	mkdir bin

clean:
	\rm -r bin src/obj
