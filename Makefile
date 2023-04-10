CC=gcc
CFLAGS=-I. -pthread

all: sislog cliente

sislog: cola.o util.o sislog.o
	gcc -c cola.c util.c -pthread
	gcc -c util.c -pthread
	gcc -c sislog.c cola.c util.c -pthread
	gcc -o sislog cola.o util.o sislog.o -pthread

cliente: cola.o util.o sislog.o
	gcc -c cola.c util.c -pthread
	gcc -c util.c -pthread
	gcc -c cliente.c cola.c util.c -pthread
	gcc -o cliente cola.o util.o cliente.o -pthread

limpia:
	-rm *.o

cleanall: limpia
	-rm sislog cliente *.dat
