CC := gcc
CFLAGS := -Wall -Wextra -std=c99 -D_XOPEN_SOURCE=500 -lpthread -pedantic -g -lc  -fsanitize=address

all: application view slaves

application:
	$(CC) $(CFLAGS) application2.0.c shmADT -o application

slaves:
	$(CC) $(CFLAGS) slaves.c -o slaves
shm:
	$(CC) -Wall -Wextra -std=c99 -D_XOPEN_SOURCE=500 -lpthread -pedantic -g -c  -fsanitize=address shmADT.c -o shmADT
view:
	$(CC) $(CFLAGS) view.c shmADT -o view
clean:
	rm view slaves application