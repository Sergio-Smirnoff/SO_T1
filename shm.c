#include "shm.h"

typedef struct shmCDT{
    char *name;             //nombre de la memoria compartida
    int offset;             //desplazamiento actual en la memoria compartida
    sem_t *semaphore;       //semáforo para la sincronización de lectura
    char virtual_address[SHARED_MEM_SIZE];  //dirección virtual de la memoria compartida
} shmCDT;

typedef shmCDT * shmADT;


static int map_shared_mem(shmADT shm, int prot, int fd){
    //Mapea la memoria compartida en el espacio de direcciones del proceso
    shm=mmap(NULL, sizeof(shmCDT), prot, MAP_SHARED, fd, 0);

    if(shm == MAP_FAILED){       //Chequea si el mapeo salió bien
        perror("shared memory could not be addresed");
        return EXIT_FAIL;
    }
    shm->offset = 0;      //inicializa el offset de la memoria compartida
    close(fd);      //Cierra el descriptor del archivo
    return 0;
}

shmADT create_shared_mem(char *name, shmADT shm) {
    if (name == NULL || name[0] != '/') {
        perror("Invalid name for shared memory");
        return NULL;
    }

    // Crea nuevo objeto de memoria compartida
    int fd = shm_open(name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Shared memory could not be created");
        return NULL;
    }
    
    // Establece el tamaño de la memoria compartida
    if (ftruncate(fd, sizeof(shmCDT)) == -1) {
        perror("Shared memory size is unavailable");
        close(fd);
        return NULL;
    }

    // Mapea la memoria compartida en el espacio de direcciones del proceso
    shm = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("Shared memory could not be mapped");
        close(fd);
        return NULL;
    }

    shm->offset = 0; // Inicializa el offset de la memoria compartida
    close(fd); // Cierra el descriptor del archivo

    shm->name = name; // Guarda el nombre de la memoria en el struct

    // Abre el semáforo asociado con la memoria compartida
    shm->semaphore = sem_open(name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR, 0);
    if (shm->semaphore == SEM_FAILED) {
        munmap(shm->virtual_address, SHARED_MEM_SIZE);
        perror("Could not create semaphore");
        return NULL;
    }

    return shm;
}

shmADT open_shared_mem(char *name, shmADT shm) {
    if (name == NULL || name[0] != '/') {
        perror("Invalid name for shared memory");
        return NULL;
    }

    int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR); // Abre un objeto de memoria compartida para leerlo
    if (fd == -1) {
        perror("Shared memory could not be opened");
        return NULL;
    }

    // Mapea la shm para su lectura
    shm = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        close(fd);
        perror("Shared memory could not be mapped");
        return NULL;
    }

    shm->offset = 0; // Inicializa el offset de la memoria compartida
    close(fd); // Cierra el descriptor del archivo

    shm->name = name; // Guarda el nombre de la shm en el struct

    // Abre el semáforo asociado con la memoria compartida
    shm->semaphore = sem_open(name, O_RDWR);
    if (shm->semaphore == SEM_FAILED) {
        munmap(shm, SHARED_MEM_SIZE); // Desmapea la memoria compartida
        perror("Could not open semaphore");
        return NULL;
    }

    return shm;
}


int close_and_delete_shared_mem(shmADT shm ){
    if(shm==NULL){        //Chequea que el puntero de la shm no sea NULL
        perror(INVALID_ARGS);
        return EXIT_FAIL;
    }

    int return_value;

    return_value= munmap(shm, SHARED_MEM_SIZE);      //Desmapea la memoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que no haya fallado el desmapeo
        perror("Could not unmap shared memory");
        return return_value;
    }

    return_value= sem_close(shm->semaphore);      //Cierra el semáforo asociado con la shm
    if(return_value == EXIT_FAIL){      //Chequea que no haya fallado el cierre
        perror("Could not close semaphore");
        return return_value;
    }
    
    return_value= sem_unlink(shm->name);      //Elimina el semáforo asociado a la memoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que se haya eliminado correctamente
        return return_value;
    }

    return_value= shm_unlink(shm->name);      //Elimina la memoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que se haya eliminado correctamente
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
    for(i=sizeof(shmCDT)-SHARED_MEM_SIZE; shm->virtual_address[shm->offset]!=0 && i<size; i++){      //Lee el mensaje desde la shm
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