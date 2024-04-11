#include "view.h"

// BUFFER
char buff[SIZE_OF_BUFF];

int main(int argc, char *argv[]){
shmADT shm = create_shared_mem("/shmtest1");
    // validar si hay mas de un argumento
    if (argc > AMOUNT_ARGS){
        printf("Too many arguments\n");
        return EXIT_FAILURE;
    }

    if ( argc == 2 ){
        // buff = argv[1];
        strcpy(buff, argv[1]);
    }else{
        read(STD_IN, buff, sizeof(buff));
    }
    
    // open shared memory
    printf("%s", buff);
    shm = open_shared_mem(buff);
    if (shm == NULL) {
        printf("Could not open shared memory\n");
        return EXIT_FAILURE;
    }
    
    
    write_shared_mem(shm, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    // read shared memory 
    read_view(shm, buff);

    // close shared memory
    if (close_view(shm) == EXIT_FAIL){
        printf("Error closing shared memory\n");
        return EXIT_FAILURE;
    } 
    return EXIT_SUCCESS;
}

int close_view(shmADT shm){
    int close_shm_status = close_and_delete_shared_mem(shm);
    if (close_shm_status == NULL){
        printf("Error closing shared memory\n");
        return EXIT_FAILURE;
    }
    else {
        return EXIT_SUCCESS;
    }
}

void read_view(shmADT shm, char *buff) {
    int read_status;
    do {
        clean_buff();
        read_status = read_shared_mem(shm, buff, SIZE_OF_BUFF);
        if (read_status == NULL) {
            printf("Error reading shared memory\n");
            return;
        }
        
        if ( buff[0] != EOF ) {
            for( int i=0; buff[i] != '\0' && i < SIZE_OF_BUFF; i++ ){
                putchar(buff[i]);
            }
        }
        
    } while (read_status == 0);
}

void clean_buff(){
    for ( int i = 0; i < SIZE_OF_BUFF; i++ )
    {
        buff[i] = '\0';
    }
}