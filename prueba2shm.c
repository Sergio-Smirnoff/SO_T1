#include "shm.h"

int main(int argc, char *argv[]){


    shmADT shm = open_shared_mem(argv[1]);
    char buff[256];

    read_shared_mem(shm,buff,256);

    //printf("%s", buff);
    close_and_delete_shared_mem(shm);

    return 0;

}