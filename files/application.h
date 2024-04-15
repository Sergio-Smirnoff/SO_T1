#ifndef SO_TP1_APPLICATION_H
#define SO_TP1_APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>
#include "shmADT.h"

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
void create_slave(struct processCDT* process);
int set_fds(struct processCDT processes[], int num_slaves, fd_set *set_in, fd_set *set_out);
void close_selected_fd(struct processCDT processes[], int slave_amount, int in_out);
void work_distributor(char* files[], int amount_files,int slaves_amount, struct processCDT processes[], fd_set *set_in, fd_set *set_out,shmADT shm );
void files_distributor( char* files[], int amount_files,int slaves_amount, struct processCDT processes[], fd_set *set_in, fd_set *set_out,shmADT shm  );
void finish_hearing(int slaves_amount, struct processCDT processes[], fd_set *set_out,shmADT shm );
int are_all_fd_close(struct processCDT processes[], int amount_slaves);
void clear_buff();

#endif