hackass : main.o parser.o decoder.o dynpoolalloc.o symbollib.o poolalloc.o
	gcc -o hackass main.o parser.o decoder.o dynpoolalloc.o poolalloc.o symbollib.o

main.o : main.c
	gcc -c main.c

parser.o : parser.c
	gcc -c parser.c

decoder.o : decoder.c
	gcc -c decoder.c

symbollib.o : symbollib.c dynpoolalloc.h
	gcc -c symbollib.c

dynpoolalloc.o : dynpoolalloc.c poolalloc.h
	gcc -c dynpoolalloc.c

poolalloc.o : poolalloc.c
	gcc -c poolalloc.c

clean : 
	rm main.o parser.o decoder.o symbollib.o dynpoolalloc.o poolalloc.o
