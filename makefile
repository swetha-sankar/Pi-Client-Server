all: make
make: final
final: main.o
	gcc main.o -o final -lsense -lm
main.o: main.h main.c
	gcc -c main.c
clean: 
	rm -f final *.o
