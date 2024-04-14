#ifndef SO_TP1_APPLICATION_H
#define SO_TP1_APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include "shm.h"

// DEFINES
#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2
#define SLAVES_COUNT_INIT 5
#define FILES_LIMIT 100
#define SIZE_OF_BUFF 256
#define MEM_NAME "pipe"

//Structure

typedef struct processCDT
{
    pid_t pid;
    int fd_read;
    int fd_write;
} process;

typedef process * p_process;

int get_amount_of_slaves(int amount_of_files);
void create_slaves(int slave_amount, struct processCDT processes[]);
void create_slave(struct processCDT process);
//int set_fds(p_process slaves[], int num_slaves, fd_set *set_in, fd_set *set_out);
//void work_distributor(char* files[], int amount_files,int slaves_amount, p_process processes[], fd_set *set_in, fd_set *set_out,shmADT shm );
//void clean_buff();
//int are_all_fd_close(p_process processes[], int amount_slaves);

#endif