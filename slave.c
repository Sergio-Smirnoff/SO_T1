#include "slave.h"

//BUFFER
char buff[SIZE_OF_BUFF];

int main(int argc, char *argv[])
{
    char* file;
    char* final;
    pid_t pid = getpid();
    // hashes the given files by argument
    for( int i=1; i < 3; i++ ){
        //sprintf(buff,"%s",argv[i]); // ver si hay forma mejor
        //final = hashing( pid );
        char* str = argv[i];

        int s= write( 1,"Hola mundo",sizeof("Hola mundo"));
        fsync(1);
        //printf("%d\n", s);
        //clean_buff();
        "Hola munHol"
    }
    exit(0);
    
    int bytes_read;
    // hashes the given files by pipe
    while((bytes_read = read(STD_IN,buff,SIZE_OF_BUFF )) == 0){
        final = hashing(pid);
        clean_buff();

        write( STD_OUT,final,strlen(final) );
    }
    //cerrar pipies
    close(STD_IN);
    close(STD_OUT);

    if ( bytes_read == 0 )
        exit(EXIT_SUCCESS);
    else
        exit(EXIT_FAILURE);

    
}

char* hashing(pid_t pid){
    char* to_return;
    char* hash;
    FILE *pipe=popen("md5sum", "r");
    if(!pipe){
        perror("popen");
        exit(EXIT_FAILURE);
    }

    fprintf(pipe, "\"%s\"", buff);
    fclose(pipe);

    pipe = popen("md5sum", "r");
    if(!pipe){
        perror("popen");
        exit(EXIT_FAILURE);
    }

    fgets(hash, SIZE_OF_BUFF, pipe);
    pclose(pipe);

    sprintf( to_return, "File: %s - Md5: %s - Slave Pid: %d\n", buff, hash, pid);
    return to_return;

}

void clean_buff(){
    for ( int i = 0; i < SIZE_OF_BUFF; i++ )
    {
        buff[i] = '\0';
    }
}