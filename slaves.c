#include "slave.h"
#include <libgen.h>

int main(){

    //recibo por pipe un archivo
    //lo leo

    char buff[SIZE_OF_BUFF];
    char hash[256];
    char* str = malloc(sizeof(char)*300);
    char* bname;
    int read_bytes;
    pid_t pid = getpid();
/*
    while(!feof(stdin)){
        read_bytes = read(0,buff,sizeof(buff)); 
        if (read_bytes == 0)
            continue;
    
        buff[read_bytes]='\0'; // -1 por testeo de entrada estandar despues sacar . Xq -1?

        int status = hashing(buff, hash);
        if ( status != 0 ){
            perror("Hashing error");
            exit(EXIT_FAILURE);
        }
        bname = basename(buff);
        sprintf( str, "File: %s - Md5: %s - Slave Pid: %d\n", bname, hash, pid);

        write(1,str,strlen(str));
        fsync(1);
    }
    */
    
    do{
        read_bytes = read(0,buff,sizeof(buff)); 
        if (read_bytes <= 0)
            continue;
    
        buff[read_bytes]='\0'; // -1 por testeo de entrada estandar despues sacar . Xq -1?

        int status = hashing(buff, hash);
        if ( status != 0 ){
            perror("Hashing error");
            exit(EXIT_FAILURE);
        }
        bname = basename(buff);
        sprintf( str, "File: %s - Md5: %s - Slave Pid: %d\n", bname, hash, pid);

        write(1,str,strlen(str));
        fsync(1);
    }while(read_bytes != 0);
    

    free(str);
    exit(EXIT_SUCCESS);

}


int hashing( char* file, char* hash){

    char* command = malloc(300*sizeof(char));
    if ( command == NULL ){
        return EXIT_FAILURE;
    }

    //formatting the command
    strcpy(command,"md5sum \"");
    strcat(command,file);
    strcat(command,"\" 2>/dev/null");

    FILE* pipe = popen(command, "r");
    if ( pipe == NULL ){
        perror("Error");
        return EXIT_FAILURE;
    }

    char buff[33];
    fgets(buff, sizeof(buff), pipe);

    strcpy(hash,buff);

    printf("%s\n", hash);

    pclose(pipe);
    free(command);

    return EXIT_SUCCESS;
}