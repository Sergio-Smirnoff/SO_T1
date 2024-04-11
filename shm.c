#include "shm.h"

typedef struct shmCDT{
    char *name;             //nombre de la memoria compartida
    int offset_read;             //desplazamiento actual en la memoria compartida
    int offset_write;             //desplazamiento actual en la memoria compartida
    sem_t *semaphore;       //semáforo para la sincronización de lectura
    char virtual_address[SHARED_MEM_SIZE];  //dirección virtual de la memoria compartida
} shmCDT;

/*
static int map_shared_mem(shmADT shm, int prot, int fd){
    //Mapea la memoria compartida en el espacio de direcciones del proceso
    shm=mmap(NULL, sizeof(shmCDT), prot, MAP_SHARED, fd, 0);

    if(shm == MAP_FAILED){       //Chequea si el mapeo salió bien
        printf("shared memory could not be addresed");
        return EXIT_FAIL;
    }
    shm->offset_read = 0;      //inicializa el offset de la memoria compartida
    shm->offset_write = 0;
    close(fd);      //Cierra el descriptor del archivo
    return 0;
}
*/

shmADT create_shared_mem(char *name) {
    
    if (name == NULL || name[0] != '/') {
        printf("Invalid name for shared memory\n");
        return NULL;
    }

    // Crea nuevo objeto de memoria compartida
    int fd = shm_open(name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf("Shared memory could not be created\n");
        return NULL;
    }
    
    // Establece el tamaño de la memoria compartida
    if (ftruncate(fd, sizeof(shmCDT)) == -1) {
        printf("Shared memory size is unavailable\n");
        close(fd);
        return NULL;
    }

    // Mapea la memoria compartida en el espacio de direcciones del proceso
    shmADT shm = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        printf("Shared memory could not be mapped\n");
        shm_unlink(shm->name);
        close(fd);
        return NULL;
    }

    shm->offset_read = 0;      //inicializa el offset de la memoria compartida
    shm->offset_write = 0; // Inicializa el offset de la memoria compartida
    close(fd); // Cierra el descriptor del archivo

    shm->name = name; // Guarda el nombre de la memoria en el struct

    // Abre el semáforo asociado con la memoria compartida
    shm->semaphore = sem_open(name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR, 0);
    if (shm->semaphore == SEM_FAILED) {
        munmap(shm->virtual_address, SHARED_MEM_SIZE);
        printf("Could not create semaphore\n");
        shm_unlink(shm->name);
        return NULL;
    }

    return shm;
}

shmADT open_shared_mem(char *name) {
    if (name == NULL || name[0] != '/') {
        printf("Invalid name for shared memory\n");
        return NULL;
    }
    printf("%s", name);
    int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR); // Abre un objeto de memoria compartida para leerlo
    if (fd == -1) {
        printf("Shared memory could not be opened\n");
        return NULL;
    }

    // Mapea la shm para su lectura
    shmADT shm = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        close(fd);
        printf("Shared memory could not be mapped\n");
        shm_unlink(shm->name);
        return NULL;
    }

    shm->offset_read = 0;      //inicializa el offset de la memoria compartida
    shm->offset_write = 0;// Inicializa el offset de la memoria compartida
    close(fd); // Cierra el descriptor del archivo

    shm->name = name; // Guarda el nombre de la shm en el struct

    // Abre el semáforo asociado con la memoria compartida
    shm->semaphore = sem_open(name, O_RDWR);
    if (shm->semaphore == SEM_FAILED) {
        munmap(shm, SHARED_MEM_SIZE); // Desmapea la memoria compartida
        printf("Could not open semaphore\n");
        shm_unlink(shm->name);
        return NULL;
    }
    return shm;
}


int close_and_delete_shared_mem(shmADT shm ){
    if(shm==NULL){        //Chequea que el puntero de la shm no sea NULL
        printf(INVALID_ARGS);
        return EXIT_FAILURE;
    }

    int return_value;

    return_value = sem_close(shm->semaphore);      //Cierra el semáforo asociado con la shm
    if(return_value == EXIT_FAIL){      //Chequea que no haya fallado el cierre
        printf("Could not close semaphore\n");
        return EXIT_FAILURE;
    }
    
    return_value = sem_unlink(shm->name);      //Elimina el semáforo asociado a la memoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que se haya eliminado correctamente
        return EXIT_FAILURE;
    }

    return_value= shm_unlink(shm->name);      //Elimina la memoria compartida
    if(return_value == EXIT_FAIL){      //Chequea que se haya eliminado correctamente
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int read_shared_mem(shmADT shm, char *message_buffer, int size){
    if(shm==NULL || message_buffer==NULL || size<=0){     //Chequea que los argumentos sean válidos
        printf(INVALID_ARGS);
        return EXIT_FAILURE;
    }
    sem_wait(shm->semaphore);     //Espera a que haya algo para leer
    size--;                             //Hace lugar para el caracter nulo final
    int i;
    for(i=0; shm->virtual_address[shm->offset_read]!=0 && i<size; i++){      //Lee el mensaje desde la shm
        message_buffer[i]=shm->virtual_address[shm->offset_read++];
    }
    message_buffer[i]=0;        //Agrega el caracter nulo final
    shm->offset_read++;        //Actualiza desplazamiento de la shm
    return 0;
}

int write_shared_mem(shmADT shm, const char *message_buffer){
    if(shm==NULL || message_buffer==NULL){        //Chequea que los argumentos no sean inválidos
        printf(INVALID_ARGS);
        return EXIT_FAILURE;
    }
    int i;
    for(i=0; message_buffer[i]!=0; i++){        //Escribe el mensaje en la memoria compartida
        shm->virtual_address[shm->offset_write++]=message_buffer[i];
    }

    shm->virtual_address[shm->offset_write++]=message_buffer[i];  //incluye el caracter nulo final
    sem_post(shm->semaphore);     //Notifica que hay un mensaje disponible

    return 0;
}
