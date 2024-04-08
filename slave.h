#ifndef SO_TP1_SLAVE_H
#define SO_TP1_SLAVE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// DEFINES
#define SIZE_OF_BUFF 256

char* hashing(pid_t pid);
void clean_buff();

#endif //SO_TP1_SLAVE_H