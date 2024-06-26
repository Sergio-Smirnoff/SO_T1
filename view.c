#include "view.h"

char buff[SIZE_OF_BUFF];

int main(int argc, char *argv[]){

    if (argc > AMOUNT_ARGS){
        printf("Too many arguments\n");
        return EXIT_FAILURE;
    }

    if (argc == 2){
        strcpy(buff, argv[1]);
    } else {
        read(STD_IN, buff, sizeof(buff));
    }
    
    shmADT shm = open_shared_mem(buff);
    if (shm == NULL) {
        printf("Could not open shared memory\n");
        return EXIT_FAILURE;
    }
     
    read_view(shm, buff);

    return EXIT_SUCCESS;
}

int close_view(shmADT shm){
    int close_shm_status = close_and_delete_shared_mem(shm);
    if (close_shm_status == EXIT_FAILURE){
        printf("Error closing shared memory\n");
        return EXIT_FAILURE;
    }
    else {
        return EXIT_SUCCESS;
    }
}

int read_view(shmADT shm, char *buff) {
    int read_status;
    do {
        clean_buff();
        read_status = read_shared_mem(shm, buff, SIZE_OF_BUFF);
        if (read_status == EXIT_FAILURE) {
            printf("Error reading shared memory\n");
            return EXIT_FAILURE;
        }

        printf("%s\n", buff);
        
    } while (read_status == 0);

    return 0;
}

void clean_buff(){
    for ( int i = 0; i < SIZE_OF_BUFF; i++ )
    {
        buff[i] = '\0';
    }
}