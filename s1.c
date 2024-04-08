
#include "slave.h"
int main(int argc, char *argv[]){

    pid_t pid = fork();
    char *av[] = {"./slave.c", "shm.c","shm.h", NULL};
    char *ev[] = {NULL};
    int p[2];
    pipe(p);
    
/*
    if (pid == 0){
        close(STD_IN);
        dup(p[STD_IN]);
        close(p[STD_IN]);

        execve("./slave", av, ev);
    }

    close(0);
*/
    return 0;


}