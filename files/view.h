#ifndef SO_TP1_VIEW_H
#define SO_TP1_VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h> 
#include <string.h>
#include "shmADT.h"

// DEFINES
#define STD_IN 0
#define STD_OUT 1
#define SIZE_OF_BUFF 256
#define AMOUNT_ARGS 2
#define EXIT_SUCCESS 0

int close_view(shmADT shm);
int read_view(shmADT shm, char *buff);
void clean_buff();

#endif //SO_TP1_VIEW_H