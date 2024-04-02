#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h> 
#include <shm.h>

// DEFINES
#define STD_IN 0
#define STD_OUT 1
#define SIZE_OF_BUFF 256
#define AMOUNT_ARGS 2
#define EXIT_SUCCESS 0

//BUFFER
char buff[SIZE_OF_BUFF];

int main(int argc, char *argv[]){

    // validar si hay mas de un argumento
    if (argc > AMOUNT_ARGS){
        perror("Too many arguments");
        return EXIT_FAILURE;
    }

    char* name;
    if ( argc == 2 ){
        name = argv[1];
    }else{
        name = read(STD_IN, buff, sizeof(buff));
    }
    
    // open shared memory
    shmADT shm = open_shared_mem(name);

    if (shm == NULL) {
        perror("Could not open shared memory");
        return EXIT_FAIL;
    }
    
    // read shared memory 
    int read_status;
    do {
        read_status = read_shared_mem(shm, buff, sizeof(buff));
        if (read_status == EXIT_FAIL) {
            perror("Error reading shared memory");
        }
        
        if ( buff[0] != EOF ) {
            // printear
        }
        
    } while (read_status == 0);

    // close shared memory
    close(shm);
}


void close(shmADT shm){
    int close_shm_status = close_shared_mem(shm);
    if (close_shm_status != EXIT_SUCCESS){
        perror("Error closing shared memory");
        return EXIT_FAIL;
    }
    else{
        return EXIT_SUCCESS;
    }  
    return;
}