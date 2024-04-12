
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    
    printf("hola mundo %d\n\0",getpid());

    return 0;
}
