all : ush

ush : ush.o parse.o
	gcc -o ush ush.o parse.o

parse.o: parse.c
	gcc -w -o parse.o -c parse.c
ush.o : ushell.c  
	gcc -w -o ush.o -c ushell.c

clean :
	rm ush.o parse.o
