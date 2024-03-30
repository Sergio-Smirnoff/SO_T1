#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// DEFINES
#define STD_IN 0
#define STD_OUT 1
#define SIZE_OF_BUFF 256

//BUFFER
char buff[SIZE_OF_BUFF];

int main(int argc, char *argv[])
{
    char* file;
    char* final;
    pid_t pid = getpid();
    // hashes the given files by argument
    for( int i=1; i < 3; i++ ){
        file = argv[i];
        final = hashing( file, pid );

        write( STD_OUT,final,strlen(final) );
    }
    
    
    char* pointer;
    // hashes the given files by pipe
    while((pointer = fgets(file,SIZE_OF_BUFF,STD_IN )) == NULL){
        final = hashing ( file, pid );

        write( STD_OUT,final,strlen(final) );
    }

    //TODO: chequear que funcione esto
    if ( feof(STD_IN) )
        exit(EXIT_SUCCESS);
    else
        exit(EXIT_FAILURE);

    
}

char* hashing(char* file, pid_t pid){
    char* to_return;
    FILE *pipe=popen("md5sum", "r");
    if(!pipe){
        perror("popen");
        exit(EXIT_FAILURE);
    }

    fprintf(pipe, "%s", file);
    fclose(pipe);

    pipe = popen("md5sum", "r");
    if(!pipe){
        perror("popen");
        exit(EXIT_FAILURE);
    }

    fgets(file, SIZE_OF_BUFF, pipe);
    pclose(pipe);

    sprintf( to_return, "%s -> slave pid: %d", file, pid );
    return to_return;

}