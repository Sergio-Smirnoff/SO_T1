
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int main()
{
    pid_t pid = getpid();
    printf("Hola mundo %d\n", pid);
    /*
    char* str = malloc(sizeof(char)*300);
    char buff[256];

    int read_bytes = read(0,buff,sizeof(buff)); 
    if (read_bytes <= 0){
        errno = 1;
        printf("%s\n",strerror(errno));
        exit(1);
    }else if (read_bytes == 0 || buff[0] == EOF)
    {
        exit(0);
    }
        
    buff[read_bytes]='\0';
    strcpy(str,buff);
    write(1,str,strlen(str));

    free(str);
    */

    
    exit(0);
}
