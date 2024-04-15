#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>

#define SHARED_MEM_SIZE 1048576
#define EXIT_FAIL -1
#define EXIT_SUCCESS 0
#define INVALID_ARGS "Invalid arguments\n"
#define READ_FINISHED 2

typedef struct shmCDT {
    char *name;             //nombre de la memoria compartida
    int offset_read;             //desplazamiento actual en la memoria compartida
    int offset_write;             //desplazamiento actual en la memoria compartida
    sem_t * semaphore;       //semáforo para la sincronización de lectura
    char virtual_address[SHARED_MEM_SIZE];  //dirección virtual de la memoria compartida
    int flag;
} shmCDT;

typedef shmCDT* shmADT;

shmADT create_shared_mem(char *name) {

    if (name == NULL || name[0] != '/') {
        
        perror("Invalid name for shared memory\n");
        return NULL;
    }

    // Crea nuevo objeto de memoria compartida
    int fd = shm_open(name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Shared memory could not be created\n");
        return NULL;
    }

    // Establece el tamaño de la memoria compartida
    if (ftruncate(fd, sizeof(shmCDT)) == -1) {
        perror("Shared memory size is unavailable\n");
        close(fd);
        return NULL;
    }

    // Mapea la memoria compartida en el espacio de direcciones del proceso
    shmADT shm = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("Shared memory could not be mapped\n");
        shm_unlink(name);
        close(fd);
        return NULL;
    }

    // Inicializa el semáforo asociado con la memoria compartida
    shm->semaphore = sem_open(name,O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR,0);
    if (shm->semaphore == SEM_FAILED) {
        perror("Failed to create semaphore\n");
        munmap(shm, SHARED_MEM_SIZE);
        shm_unlink(name);
        close(fd);
        return NULL;
    }

    shm->offset_read = 0;      // Inicializa el offset de la memoria compartida
    shm->offset_write = 0; // Inicializa el offset de la memoria compartida
    close(fd); // Cierra el descriptor del archivo

    shm->name = name; // Guarda el nombre de la memoria en el struct

    return shm;
}

shmADT open_shared_mem(char *name) {
    if (name == NULL || name[0] != '/') {
        perror("Invalid name for shared memory\n");
        return NULL;
    }

    // Abre la memoria compartida
    int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Shared memory could not be opened\n");
        return NULL;
    }

    // Mapea la memoria compartida
    shmADT shm = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        close(fd);
        perror("Shared memory could not be mapped\n");
        shm_unlink(name);
        return NULL;
    }

    shm->semaphore = sem_open(name,O_RDWR);
    if ( shm->semaphore == SEM_FAILED){
        perror("Could not open semaphore");
        munmap(name,SHARED_MEM_SIZE);
        return NULL;
    }

    close(fd); // Cierra el descriptor del archivo

    // El semáforo ya debería haber sido inicializado en create_shared_mem()

    shm->offset_read = 0;
    shm->offset_write = 0;
    shm->name = name;

    return shm;
}


int close_and_delete_shared_mem(shmADT shm ){
    if(shm==NULL){
        perror(INVALID_ARGS);
        return EXIT_FAILURE;
    }

    int return_value;

    // Cierra el semáforo asociado con la memoria compartida
    return_value = sem_close(shm->semaphore);
    if(return_value == EXIT_FAIL){
        perror("Could not close semaphore\n");
        return EXIT_FAILURE;
    }

    // Elimina el semáforo asociado a la memoria compartida
    return_value = sem_unlink(shm->name);
    if(return_value == EXIT_FAIL){
        return EXIT_FAILURE;
    }

    // Elimina la memoria compartida
    return_value= shm_unlink(shm->name);
    if(return_value == EXIT_FAIL){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void raise_finish_reading(shmADT shm){ //recomendacion:esperar dos segundos y ahi cerrar todo
    shm->flag=READ_FINISHED;
    sem_post(shm->semaphore);
}


int read_shared_mem(shmADT shm, char *message_buffer, int size){

    if(shm==NULL || message_buffer==NULL || size<=0){
        perror(INVALID_ARGS);
        return EXIT_FAILURE;
    }

    // Verifica si el semáforo se abrió correctamente
    if (sem_wait(shm->semaphore) != 0) {
        perror("Error waiting on semaphore");
        return EXIT_FAILURE;
    }

    if(shm->flag==READ_FINISHED && shm->offset_read>=shm->offset_write){
        return READ_FINISHED;
    }

    size--;
    int i;
    for(i=0; shm->virtual_address[shm->offset_read]!=0 && i<size; i++){
        message_buffer[i]=shm->virtual_address[shm->offset_read++];
    }
    message_buffer[i]=0;

    // Actualiza el desplazamiento solo si se leyó algo
    if (i > 0) {
        shm->offset_read++;
    }

    return EXIT_SUCCESS;
}


int write_shared_mem(shmADT shm, const char *message_buffer){
    if(shm==NULL || message_buffer==NULL){        //Chequea que los argumentos no sean inválidos
        perror(INVALID_ARGS);
        return EXIT_FAILURE;
    }
    int i;
    //sem_wait(shm->semaphore); // Espera a que se libere el semáforo
    for(i=0; message_buffer[i]!=0; i++){        //Escribe el mensaje en la memoria compartida
        shm->virtual_address[shm->offset_write++]=message_buffer[i];
    }

    shm->virtual_address[shm->offset_write++]=message_buffer[i];  //incluye el caracter nulo final
    sem_post(shm->semaphore);     //Notifica que hay un mensaje disponible

    return EXIT_SUCCESS;
}