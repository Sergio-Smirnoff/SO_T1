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
    printf("Paso write\n");
    read_shared_mem(shm, buffer, sizeof(char)*11);
    printf("Paso read\n");
    printf("%s", buffer);
    close_and_delete_shared_mem(shm);
    printf("\nSuccess!!!\n");
/*
    pid_t pid = fork();
    char *av[] = {"./prueba2.c", "MEMORIA", NULL};
    char *ev[] = {NULL};

    if (pid == 0){
        execve("prueba2.c", av, ev);

    }
*/
    return 0;


}