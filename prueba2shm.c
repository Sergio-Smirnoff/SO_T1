
#include "shm.h"
/*
int main(int argc, char argv[]){


    shmADT shm = create_shared_mem("/shmtest200");
    if (shm == NULL) {
        printf("Failed to create shared memory\n");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();

    char *extargv[] = {"./view","/shmtest200", NULL};
    char *extenv = {NULL};

    if (  pid < 0 ){
        perror("todo mal");
        exit(1);
    }else if ( pid == 0 ){


        execve("view", extargv, extenv);
        perror("execve");
        exit(0);

    }

    write_shared_mem(shm,"me gusta comer pastel\0");

    int state;
    waitpid(-1,&state,0);
    int close_shm_status = close_and_delete_shared_mem(shm);
    if (close_shm_status == EXIT_FAILURE){
        printf("Error closing shared memory\n");
        return EXIT_FAILURE;
    }
    else {
        return EXIT_SUCCESS;
    }

    printf("\nSuccess!!!\n");
    return 0;

}
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shm.h" // Incluye el archivo de encabezado que define las funciones para trabajar con la memoria compartida

#define BUFFER_SIZE 256

int main() {
    char *shm_name = "/test_shm"; // Nombre de la memoria compartida

    // Crea la memoria compartida
    printf("Creating shared memory...\n");
    shmADT shm = create_shared_mem(shm_name);
    if (shm == NULL) {
        printf("Failed to create shared memory\n");
        return EXIT_FAILURE;
    }
    printf("Shared memory created\n");

    // Crea un proceso hijo
    printf("Forking...\n");
    pid_t child_pid = fork();

    if (child_pid == -1) {
        printf("Failed to fork\n");
        close_and_delete_shared_mem(shm);
        return EXIT_FAILURE;
    } else if (child_pid == 0) {
        // Proceso hijo

        // Abre la memoria compartida
        printf("Child process: Opening shared memory...\n");
        shmADT child_shm = open_shared_mem(shm_name);
        if (child_shm == NULL) {
            printf("Child process: Failed to open shared memory\n");
            return EXIT_FAILURE;
        }

        printf("Child process: Shared memory opened\n");
        // Escribe en la memoria compartida
        char reply[] = "Hello from child!";
        printf("Child process: Writing reply...\n");
        write_shared_mem(child_shm, reply);
        printf("Child process: Reply written\n");

        // Lee desde la memoria compartida
        char message[BUFFER_SIZE];
        printf("Child process: Waiting for message...\n");
        read_shared_mem(child_shm, message, BUFFER_SIZE);
        printf("Child process: Received message: %s\n", message);
        // Cierra y elimina la memoria compartida
        //printf("Child process: Closing shared memory...\n");
        //close_and_delete_shared_mem(child_shm);
        //printf("Child process: Shared memory closed\n");

        return EXIT_SUCCESS;
    } else {
        // Proceso padre

        // Espera a que el hijo termine
        printf("Parent process: Waiting for child to finish...\n");
        wait(NULL);
        printf("Parent process: Child process finished\n");

        // Abre la memoria compartida
        printf("Parent process: Opening shared memory...\n");
        shmADT parent_shm = open_shared_mem(shm_name);
        if (parent_shm == NULL) {
            printf("Parent process: Failed to open shared memory\n");
            close_and_delete_shared_mem(shm);
            return EXIT_FAILURE;
        }
        printf("Parent process: Shared memory opened\n");

        // Escribe en la memoria compartida
        char message[] = "Hello from parent!";
        printf("Parent process: Writing message...\n");
        write_shared_mem(parent_shm, message);
        printf("Parent process: Message written\n");

        // Lee desde la memoria compartida
        char reply[BUFFER_SIZE];
        printf("Parent process: Waiting for reply...\n");
        read_shared_mem(parent_shm, reply, BUFFER_SIZE);
        printf("Parent process: Received reply: %s\n", reply);

        // Cierra y elimina la memoria compartida
        printf("Parent process: Closing shared memory...\n");
        close_and_delete_shared_mem(parent_shm);
        printf("Parent process: Shared memory closed\n");

        // Elimina la memoria compartida
        printf("Parent process: Deleting shared memory...\n");
        close_and_delete_shared_mem(shm);
        printf("Parent process: Shared memory deleted\n");

        return EXIT_SUCCESS;
    }
}