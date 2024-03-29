#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define STD_IN 0
#define STD_OUT 1
#define SIZE_OF_BUFF 256

//buffer

char buff[SIZE_OF_BUFF];

int main(int argc, char *argv[])
{
    char* file;
    char* final;
    pid_t pid = getpid();
    // hashes the given files by argument
    for( int i=1; i < 3; i++ ){
        file = argv[i];

        sprintf( final, "%s %s -> slave pid: %d", file, md5sum(file), pid );
        write( STD_OUT,final,strlen(final) );
    }
    
    //read( STD_IN, buff, sizeof(buff));
    fgets(file,SIZE_OF_BUFF,STD_IN );
    ssize_t bytes_read;
    // hashes the given files by pipe
    while((bytes_read = read(STD_IN, buff, sizeof(buff))) > 0){

        int p[2];
        char *argv[] = {"./md5sum",file, NULL};
        char *env = {NULL};
        pipe(p);
        //TODO: cambiarlo como un popen de prueba .c 
        if ( fork() == 0 )
        {

            close(STD_IN);
            close(STD_OUT);
            dup(p[STD_IN]);
            dup(p[STD_OUT]);
            close(p[STD_IN]);
            close(p[STD_OUT]);

            execve("md5sum",argv,env);

        }

        sprintf( final, "%s %s -> slave pid: %d", file, md5sum(file), pid );
        write( STD_OUT,final,strlen(final) );
        free(file);
    }

    if(bytes_read == -1){
        perror("read");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
