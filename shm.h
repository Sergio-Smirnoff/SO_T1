
#ifndef SO_TP1_SHM_H
#define SO_TP1_SHM_H

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>

#define SHARED_MEM_SIZE 1048576 //1MB
#define EXIT_FAIL -1
#define INVALID_ARGS "Invalid arguments"

int create_shared_mem(shared_mem *share_mem, char *name);
int open_shared_mem(shared_mem *share_mem, char *name);
int delete_shared_mem(shared_mem *share_mem);
int close_shared_mem(shared_mem *share_mem);
int read_shared_mem(shared_mem *share_mem, char *message_buffer, int size);
int write_shared_mem(shared_mem *share_mem, const char *message_buffer);

#endif //SO_TP1_SHM_H
