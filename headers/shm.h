#ifndef SO_TP1_SHM_H
#define SO_TP1_SHM_H

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/stat.h>

#define SHARED_MEM_SIZE 1048576 //1MB
#define EXIT_FAIL -1
#define INVALID_ARGS "Invalid arguments"

typedef struct shmCDT * shmADT;

shmADT create_shared_mem(char *name);
shmADT open_shared_mem(char *name);
int delete_shared_mem(shmADT shm);
int close_shared_mem(shmADT shm);
int read_shared_mem(shmADT shm, char *message_buffer, int size);
int write_shared_mem(shmADT shm, const char *message_buffer);
//int free_shared_mem(shmADT shm);

#endif //SO_TP1_SHM_H