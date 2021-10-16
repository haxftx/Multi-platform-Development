
all: build

build: so-multi.o
	gcc -Wall -Wextra -g so-multi.o -o so-cpp

so-multi.o: so-multi.c
	gcc -Wall -Wextra -g so-multi.c -c -o so-multi.o

clean:
	rm -f so-cpp so-multi.o
