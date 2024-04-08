#include <view.h>

// BUFFER
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
        read(STD_IN, buff, sizeof(buff));
    }
    
    // open shared memory
    shmADT shm = open_shared_mem(buff);
    if (shm == NULL) {
        perror("Could not open shared memory");
        return EXIT_FAIL;
    }
    
    // read shared memory 
    read_view(shm, buff);

    // close shared memory
    close_view(shm);
}

void close_view(shmADT shm){
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

void read_view(shmADT shm, char *buff) {
    int read_status;
    do {
        clean_buff();
        read_status = read_shared_mem(shm, buff, SIZE_OF_BUFF);
        if (read_status == EXIT_FAIL) {
            perror("Error reading shared memory");
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