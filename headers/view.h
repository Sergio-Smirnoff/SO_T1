#ifndef SO_TP1_VIEW_H
#define SO_TP1_VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h> 
#include <shm.h>

// DEFINES
#define STD_IN 0
#define STD_OUT 1
#define SIZE_OF_BUFF 256
#define AMOUNT_ARGS 2
#define EXIT_SUCCESS 0

void close_view(shmADT shm);

void read_view(shmADT shm, char *buff);

#endif //SO_TP1_VIEW_H