#include <slave.h>

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
    char* hash;
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

    fgets(hash, SIZE_OF_BUFF, pipe);
    pclose(pipe);

    sprintf( to_return, "File: %s - Md5: %s - Slave Pid: %d\n", file, hash, pid);
    return to_return;

}