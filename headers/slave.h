#ifndef SO_TP1_SLAVE_H
#define SO_TP1_SLAVE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// DEFINES
#define STD_IN 0
#define STD_OUT 1
#define SIZE_OF_BUFF 256

char* hashing(char* file, pid_t pid);

#endif //SO_TP1_SLAVE_H