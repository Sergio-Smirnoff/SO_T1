#ifndef SO_TP1_SHM_H
#define SO_TP1_SHM_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define SHARED_MEM_SIZE 1048576 //1MB=
#define EXIT_FAIL (-1)
#define INVALID_ARGS "Invalid arguments"
#define READ_FINISHED 2

typedef struct shmCDT * shmADT;

shmADT create_shared_mem(char *name);
shmADT open_shared_mem(char *name);
int close_and_delete_shared_mem(shmADT shm );
int read_shared_mem(shmADT shm, char *message_buffer, int size);
int write_shared_mem(shmADT shm, const char *message_buffer);
void raise_finish_reading(shmADT shm);
//int free_shared_mem(shmADT shm);

#endif //SO_TP1_SHM_H