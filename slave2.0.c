
#include "slave.h"

int main(){

    //recibo por pipe un archivo
    //lo leo

    char buff[256];
    char* str;
    char* command = malloc(300*sizeof(char));
    int read_bytes = read(0,buff,sizeof(buff));
    if (read_bytes == 0)
        exit(0);
    
    buff[read_bytes]='\0';

    write(1,buff,read_bytes);

    strcpy(command,"md5sum ");
    strcat(command,"\"");
    strcat(command,buff);
    strcat(command,"\"");
    strcat(command," 2&> /dev/null");
    

    printf("%s", command);

    FILE* pipe = popen(command, "r");
    if ( pipe == NULL ){
        perror("Error");
        exit(1);
    }

char extbuff[300];
    read_bytes = read(pipe, extbuff, sizeof(extbuff));
    //extbuff[read_bytes] = '\n';
    
    
    //buff[read_bytes]="\n";

    write(1,extbuff,read_bytes);
    exit(0);

}