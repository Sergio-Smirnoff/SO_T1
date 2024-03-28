#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STD_IN 0
#define STD_OUT 1

int main(int argc, char *argv[])
{
    char* hfile;
    // hashes the given files by argument
    for( int i=1; i < 3; i++ ){
        hfile = argv[i];
        write( STD_OUT,hfile,strlen(hfile) );
    }
    
    char* file;
    scanf(file);
    // hashes the given files by pipe
    while(file != EOF){
        hfile = md5sum(file);
        printf("%s",hfile); // ver si modificar el formato
    }

    exit(0);
}
