#include "shm.h"


int main(int argc, char *argv[]) {
    shmADT shm = create_shared_mem("/shmtest1");
    if (shm == NULL) {
        printf("Failed to create shared memory\n");
        return EXIT_FAILURE;
    }
    shm = open_shared_mem("/shmtest1");
    char buffer[256];
    write_shared_mem(shm, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    // printf("Paso write\n");
    read_shared_mem(shm, buffer, sizeof(char)*256);
    // printf("Paso read\n");
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