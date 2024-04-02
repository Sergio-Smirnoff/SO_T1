#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
    char* file 
    char* final;
    pid_t pid = getpid();
    char* a = file;
    for( int i=0; buff[i] != EOF && i < 4; i++ ){
        (*a) = buff[i];
        a+=sizeof(char);
    }
    (*a) = '\0';
        
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

    fgets(final, BUFFER_SIZE, pipe);
    pclose(pipe);

    int esc = sprintf( final, "%s -> slave pid: %d", file, pid );
    write( 1,final,strlen(final) );
    free(file);
    return 0;
}
