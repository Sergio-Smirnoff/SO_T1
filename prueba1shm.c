#include "shm.h"


int main(int argc, char *argv[]) {
    shmADT my_shm;
    shmADT shm = create_shared_mem("/ABKSL", NULL);
    if (shm == NULL) {
        printf("Failed to create shared memory\n");
        return EXIT_FAILURE;
    }
   char buffer[15];
    write_shared_mem(shm, "Hola mundo");
    read_shared_mem(shm, buffer, sizeof("Hola mundo"));
    printf("%s", buffer);
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