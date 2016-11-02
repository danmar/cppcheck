ifndef CC
    CC=gcc
endif


all:	controlflow.o	data.o	functions.o	uninit.o

controlflow.o:	controlflow.c
	$(CC) -c controlflow.c

data.o:	data.c
	$(CC) -c data.c

functions.o:	functions.c
	$(CC) -c functions.c

uninit.o:	uninit.c
	$(CC) -std=gnu99 -c uninit.c

clean:
	rm -rf controlflow.o data.o functions.o uninit.o
