#include "shmADT.h"

typedef struct shmCDT {
    char *name;
    int flag;
    int offset_read;
    int offset_write;
    sem_t semaphore;
    char virtual_address[SHARED_MEM_SIZE];
} shmCDT;

shmADT create_shared_mem(char *name) {

    if (name == NULL || name[0] != '/') {
        perror("Invalid name for shared memory\n");
        return NULL;
    }

    int fd = shm_open(name, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Shared memory could not be created\n");
        return NULL;
    }

    if (ftruncate(fd, sizeof(shmCDT)) == -1) {
        perror("Shared memory size is unavailable\n");
        close(fd);
        return NULL;
    }

    shmADT shm = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("Shared memory could not be mapped\n");
        shm_unlink(name);
        close(fd);
        return NULL;
    }

    if(sem_init(&shm->semaphore, 1, 0)==-1){
        perror("Failed to create semaphore\n");
        munmap(shm, SHARED_MEM_SIZE);
        shm_unlink(name);
        close(fd);
        return NULL;
    }

    shm->offset_read = 0;
    shm->offset_write = 0;
    close(fd); 

    shm->name = name;

    return shm;
}

shmADT open_shared_mem(char *name) {
    if (name == NULL || name[0] != '/') {
        perror("Invalid name for shared memory\n");
        return NULL;
    }

    int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Shared memory could not be opened\n");
        return NULL;
    }

    shmADT shm = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        close(fd);
        perror("Shared memory could not be mapped\n");
        shm_unlink(name);
        return NULL;
    }

    close(fd);

    return shm;
}


int close_and_delete_shared_mem(shmADT shm ){
    if(shm==NULL){
        perror(INVALID_ARGS);
        return EXIT_FAILURE;
    }

    int return_value;

    return_value= shm_unlink(shm->name);
    if(return_value == EXIT_FAIL){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void raise_finish_reading(shmADT shm){ 
    shm->flag=READ_FINISHED;
    sem_post(&shm->semaphore);
}


int read_shared_mem(shmADT shm, char *message_buffer, int size){

    if(shm==NULL || message_buffer==NULL || size<=0){
        perror(INVALID_ARGS);
        return EXIT_FAILURE;
    }

    if (sem_wait(&shm->semaphore) != 0) {
        perror("Error waiting on semaphore");
        return EXIT_FAILURE;
    }

    if(shm->flag==READ_FINISHED){
        return READ_FINISHED;
    }

    size--;
    int i;
    for(i=0; shm->virtual_address[shm->offset_read]!=0 && i<size; i++){
        message_buffer[i]=shm->virtual_address[shm->offset_read++];
    }
    message_buffer[i]=0;

    if (i > 0) {
        shm->offset_read++;
    }

    return EXIT_SUCCESS;
}


int write_shared_mem(shmADT shm, const char *message_buffer){
    if(shm==NULL || message_buffer==NULL){ 
        perror(INVALID_ARGS);
        return EXIT_FAILURE;
    }
    int i;
   
    for(i=0; message_buffer[i]!=0; i++){
        shm->virtual_address[shm->offset_write++]=message_buffer[i];
    }

    shm->virtual_address[shm->offset_write++]=message_buffer[i];
    sem_post(&shm->semaphore);

    return EXIT_SUCCESS;
}