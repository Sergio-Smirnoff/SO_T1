#include "shm.h"


int main(int argc, char *argv[]){


    printf("%d", (unsigned int) 1234);
    shmADT shm = create_shared_mem("/1234");
    printf("%d", (unsigned int) 1234);
    printf("%x", (unsigned int) shm);
    

    write_shared_mem(shm, "Hola mundo");
/*
    pid_t pid = fork();
    char *av[] = {"./prueba2.c", "MEMORIA", NULL};
    char *ev[] = {NULL};

    if (pid == 0){
        execve("prueba2.c", av, ev);

    }
*/
    //close_and_delete_shared_mem(shm);


    //free_shared_mem(shm);
    return 0;


}