hackass : main.o dynpoolalloc.o symbollib.o poolalloc.o
	gcc -g -o hackass main.o dynpoolalloc.o poolalloc.o symbollib.o

main.o : main.c
	gcc -g -c main.c

symbollib.o : symbollib.c dynpoolalloc.h
	gcc -g -c symbollib.c

dynpoolalloc.o : dynpoolalloc.c poolalloc.h
	gcc -g -c dynpoolalloc.c

poolalloc.o : poolalloc.c
	gcc -g -c poolalloc.c

clean : 
	rm main.o symbollib.o dynpoolalloc.o poolalloc.o
