#ifndef SO_TP1_SLAVE_H
#define SO_TP1_SLAVE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <libgen.h>

// DEFINES
#define SIZE_OF_BUFF 256
#define STD_IN 0
#define STD_OUT 1

int hashing(char* file, char* hash);

#endif //SO_TP1_SLAVE_H