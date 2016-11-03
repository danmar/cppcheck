ifndef CC
    CC=gcc
endif

all:	controlflow.o	data.o	functions.o	ub.o

controlflow.o:	controlflow.c
	$(CC) -c controlflow.c

data.o:	data.c
	$(CC) -c data.c

functions.o:	functions.c
	$(CC) -c functions.c

ub.o:	ub.c
	$(CC) -c ub.c

clean:
	rm -rf controlflow.o data.o functions.o ub.o
