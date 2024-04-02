#include "shm.h"

typedef struct shmCDT{
    char *name;             //nombre de la memoria compartida
    int offset;             //desplazamiento actual en la memoria compartida
    char *virtual_address;  //dirección virtua de la memoria compartida
    sem_t *semaphore;       //semáforo para la sincronización de lectura
} shmCDT;

typedef shmCDT * shmADT;

static int map_shared_mem(shmADT shm, int prot, int fd){
    //Mapea la memoria compartida en el espacio de direcciones del proceso
    shm->virtual_address=mmap(NULL, sizeof(shm), prot, MAP_SHARED, fd, 0);

    if(shm->virtual_address == MAP_FAILED){       //Chequea si el mapeo salió bien
        perror("shared memory could not be addresed");
        return EXIT_FAIL;
    }
    shm->offset = 0;      //inicializa el offset de la memoria compartida
    close(fd);      //Cierra el descriptor del archivo
    return 0;
}

shmADT create_shared_mem(char *name){
    shmADT shm = malloc(sizeof(shmCDT));
    
    if(name == NULL || name[0]!='/'){       //Chequea que el nombre sea valido
        perror("Invalid name for shared memory");
        return EXIT_FAIL;
    }
    //Crea nuevo objeto de memoria compartida
    int fd= shm_open(name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR); 
    if(fd == EXIT_FAIL){        //Chequea que se haya creado bien
        perror("Shared memory could not be created");
        return EXIT_FAIL;
    }
    shm->name=name;       //Guardo el nombre de la memoria en el struct
    int return_value = ftruncate(fd, SHARED_MEM_SIZE);      //Establece el tamaño de la memoria compartida
    if (return_value == EXIT_FAIL){     //Chequea si falla el establecimiento
        perror("Shared memory size is unavailable");
        return return_value;
    }

    //Mapea la memoria compartida
    return_value = map_shared_mem(shm, PROT_READ | PROT_WRITE, fd);
    if(return_value == EXIT_FAIL){      //Chequea si el mapeo falló
        return return_value;
    }

    shm->semaphore = sem_open(name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR, 0);
    if(shm->semaphore == SEM_FAILED){
        shm_unlink(shm->name);
        perror("Could not create semaphore");
        return EXIT_FAIL;
    }
    return shm;
}

shmADT open_shared_mem(char *name){
    if(name == NULL || name[0]!='/'){       //Chequea que el nombre sea válido
        perror("Invalid name for shared memory");
        return EXIT_FAIL;
    }
    shmADT shm = malloc(sizeof(shmCDT));
    shm->name=name;       //Guarda el nombre de la shm en el struct
    int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);        //Abre un objeto de memoria compartida para leerlo
    if( fd == EXIT_FAIL){       //Chequea que la apertura haya salido bien
        perror("Shared memory could not be opened");
        return EXIT_FAIL;
    }

    int return_value= map_shared_mem(shm, PROT_WRITE, fd);     //Mapea la shm para su lectura
    if(return_value == EXIT_FAIL){      //Chequea que el mapeo no falle
        return return_value;
    }
    shm->semaphore = sem_open(name, O_RDWR);      //Abre el semáforo asociado con la memoria compartida
    if(shm->semaphore == SEM_FAILED){     //Chequea que la apertura no haya fallado
        perror("Could not open semaphore");
        munmap(name, SHARED_MEM_SIZE);      //Desmapea la memoria compartida
        return EXIT_FAIL;
    }

    return shm;
}
int delete_shared_mem(shmADT shm){
    if(shm==NULL){        //Chequea que el puntero de la shm no sea NULL
        perror(INVALID_ARGS);
        return EXIT_FAIL;
    }

    int return_value= sem_unlink(shm->name);      //Elimina el semáforo asociado a la memoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que se haya eliminado correctamente
        return return_value;
    }
    return_value= shm_unlink(shm->name);      //Elimina la memoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que se haya eliminado correctamente
        return return_value;
    }

    // free

    return 0;
}

int close_shared_mem(shmADT shm ){
    if(shm==NULL){        //Chequea que el puntero de la shm no sea NULL
        perror(INVALID_ARGS);
        return EXIT_FAIL;
    }

    int return_value= munmap(shm->virtual_address, SHARED_MEM_SIZE);      //Desmapea la memoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que no haya fallado el desmapeo
        perror("Could not unmap shared memory");
        return return_value;
    }
    return_value= sem_close(shm->semaphore);      //Cierra el semáforo asociado con la shm
    if(return_value == EXIT_FAIL){      //Chequea que no haya fallado el cierre
        perror("Could not close semaphore");
        return return_value;
    }

    return 0;
}
int read_shared_mem(shmADT shm, char *message_buffer, int size){
    if(shm==NULL || message_buffer==NULL || size>=0){     //Chequea que los argumentos sean válidos
        perror(INVALID_ARGS);
        return EXIT_FAIL;
    }
    sem_wait(shm->semaphore);     //Espera a que haya algo para leer
    size--;                             //Hace lugar para el caracter nulo final
    int i;
    for(i=0; shm->virtual_address[shm->offset]!=0 && i<size; i++){      //Lee el mensaje desde la shm
        message_buffer[i]=shm->virtual_address[shm->offset++];
    }
    message_buffer[i]=0;        //Agrega el caracter nulo final
    shm->offset++;        //Actualiza desplazamiento de la shm
    return 0;
}

int write_shared_mem(shmADT shm, const char *message_buffer){
    if(shm==NULL || message_buffer==NULL){        //Chequea que los argumentos no sean inválidos
        perror(INVALID_ARGS);
        return EXIT_FAIL;
    }
    int i;
    for(i=0; message_buffer[i]!=0; i++){        //Escribe el mensaje en la memoria compartida
        shm->virtual_address[shm->offset++]=message_buffer[i];
    }

    shm->virtual_address[shm->offset++]=message_buffer[i];  //incluye el caracter nulo final
    sem_post(shm->semaphore);     //Notifica que hay un mensaje disponible

    return 0;
}

int free_shared_mem(shmADT shm){
    if(shm==NULL){        // Chequea que el puntero de la shm no sea NULL
        perror(INVALID_ARGS);
        return EXIT_FAIL;
    }
    free(shm);      // Libera la memoria de la shm
    return 0;
}