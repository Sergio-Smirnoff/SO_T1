CC := gcc
CFLAGS := -Wall -std=c99 -D_XOPEN_SOURCE=500 -pedantic -g -lc  -fsanitize=address -lpthread

all: shm application view slaves 

application:
	$(CC) $(CFLAGS) application.c shmADT -o application

slaves:
	$(CC) $(CFLAGS) slaves.c -o slaves
shm:
	$(CC) -Wall -std=c99 -D_XOPEN_SOURCE=500 -lpthread -pedantic -g -c  -fsanitize=address shmADT.c -o shmADT
view:
	$(CC) $(CFLAGS) view.c shmADT -o view
clean:
	rm view slaves application shmADT