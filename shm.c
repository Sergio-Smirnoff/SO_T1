#include "shm.h"

static int map_shared_mem(shared_mem *share_mem, int prot, int fd){
    //Mapea la memoria compartida en el espacio de direcciones del proceso
    share_mem->virtual_address=mmap(NULL, SHARED_MEM_SIZE, prot, MAP_SHARED, fd, 0);

    if(share_mem->virtual_address == MAP_FAILED){       //Chequea si el mapeo salió bien
        perror("shared memory could not be addresed");
        return EXIT_FAIL;
    }
    share_mem->offset = 0;      //inicializa el offset de la memoria compartida
    close(fd);      //Cierra el descriptor del archivo
    return 0;
}

int create_shared_mem(shared_mem *share_mem, char *name){
    if(name == NULL || name[0]!='/'){       //Chequea que el nombre sea valido
        perror("Invalid name for shared memory");
        return EXIT_FAIL;
    }
    //Crea nuevo objeto de memoria compartida
    int fd= open_shared_mem(name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if(fd == EXIT_FAIL){        //Chequea que se haya creado bien
        perror("Shared memory could not be created");
        return EXIT_FAIL;
    }
    share_mem->name=name;       //Guardo el nombre de la memoria en el struct
    int return_value = ftruncate(fd, SHARED_MEM_SIZE);      //Establece el tamaño de la memoria compartida
    if (return_value == EXIT_FAIL){     //Chequea si falla el establecimiento
        perror("Shared memory size is unavailable");
        return return_value;
    }

    //Mapea la memoria compartida
    return_value = map_shared_mem(share_mem, PROT_READ | PROT_WRITE, fd);
    if(return_value == EXIT_FAIL){      //Chequea si el mapeo falló
        return return_value;
    }

    share_mem->semaphore = sem_open(name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR, 0);
    if(share_mem->semaphore == SEM_FAILED){
        perror("Could not create semaphore");
        return EXIT_FAIL;
    }

    return 0;
}
int open_shared_mem(shared_mem *share_mem, char *name){
    if(name == NULL || name[0]!='/'){       //Chequea que el nombre sea válido
        perror("Invalid name for shared memory");
        return EXIT_FAIL;
    }
    share_mem->name=name;       //Guarda el nombre de la shm en el struct
    int fd = sham_open(name, O_RDONLY, S_IRUSR);        //Abre un objeto de memoria compartida para leerlo
    if( fd == EXIT_FAIL){       //Chequea que la apertura haya salido bien
        perror("Shared memory could not be opened");
        return EXIT_FAIL;
    }

    int return_value= map_shared_mem(share_mem, PROT_READ, fd);     //Mapea la shm para su lectura
    if(return_value == EXIT_FAIL){      //Chequea que el mapeo no falle
        return return_value;
    }
    share_mem->semaphore = sem_open(name, O_RDWR);      //Abre el semáforo asociado con la memoria compartida
    if(share_mem->semaphore == SEM_FAILED){     //Chequea que la apertura no haya fallado
        perror("Could not open semaphore");
        munmap(name, SHARED_MEM_SIZE);      //Desmapea la memoria compartida
        return EXIT_FAIL;
    }

    return 0;
}
int delete_shared_mem(shared_mem *share_mem){
    if(share_mem==NULL){        //Chequea que el puntero de la shm no sea NULL
        perror(INVALID_ARGS);
        return EXIT_FAIL;
    }

    int return_value= sem_unlink(share_mem->name);      //Elimina el semáforo asociado a la memoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que se haya eliminado correctamente
        return return_value;
    }
    return_value= shm_unlink(share_mem->name);      //Elimina la memoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que se haya eliminado correctamente
        return return_value;
    }

    return 0;

}
int close_shared_mem(shared_mem *share_mem){
    if(share_mem==NULL){        //Chequea que el puntero de la shm no sea NULL
        perror(INVALID_ARGS);
        return EXIT_FAIL;
    }


    int return_value= munmap(share_mem->virtual_address, SHARED_MEM_SIZE);      //Desmapea la emmoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que no haya fallado el desmapeo
        perror("Could not unmap shared memory");
        return return_value;
    }
    return_value= sem_close(share_mem->semaphore);      //Cierra el semáforo asociado con la shm
    if(return_value == EXIT_FAIL){      //Chequea que no haya fallado el cierre
        perror("Could not close semaphore");
        return return_value;
    }

    return 0;
}
int read_shared_mem(shared_mem *share_mem, char *message_buffer, int size){
    if(share_mem==NULL || message_buffer==NULL || size>=0){     //Chequea que los argumentos sean válidos
        perror(INVALID_ARGS);
        return EXIT_FAIL;
    }
    sem_wait(share_mem->semaphore);     //Espera a que haya algo para leer
    size--;                             //Hace lugar para el caracter nulo final
    int i;
    for(i=0; share_mem->virtual_address[share_mem->offset]!=0 && i<size; i++){      //Lee el mensaje desde la shm
        message_buffer[i]=share_mem->virtual_address[share_mem->offset++];
    }
    message_buffer[i]=0;        //Agrega el caracter nulo final
    share_mem->offset++;        //Actualiza desplazamiento de la shm

    return 0;
}

int write_shared_mem(shared_mem *share_mem, const char *message_buffer){
    if(share_mem==NULL || message_buffer==NULL){        //Chequea que los argumentos no sean inválidos
        perror(INVALID_ARGS);
        return EXIT_FAIL;
    }
    int i;
    for(i=0; message_buffer[i]!=0; i++){        //Escribe el mensaje en la memoria compartida
        share_mem->virtual_address[share_mem->offset++]=message_buffer[i];
    }

    share_mem->virtual_address[share_mem->offset++]=message_buffer[i];  //incluye el caracter nulo final
    sem_post(share_mem->semaphore);     //Notifica que hay un mensaje disponible

    return 0;
}
