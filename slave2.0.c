
#include "slave.h"

int main(){
    auxForMd5sum();
}

int auxForMd5sum(){
     char* command = malloc(300*sizeof(char));
    strcpy(command,"md5sum \"");
    strcat(command,"./shm.c"); //Aca deberia entrar un path en vez de ./shm.c
    strcat(command,"\" 2>/dev/null");

    FILE* pipe = popen(command, "r");
    if ( pipe == NULL ){
        perror("Error");
        return 1;
    }

    char extbuff[33]; // son 32 bytes de hash + el '\0'
   
    fgets(extbuff, sizeof(extbuff), pipe);

    printf("%s\n",extbuff);
    //Faltaria agregar un buffer de salida para extraer el hash obtenido aca
    //Podemos usar la funcion basename(path) --> Te retorna el nombre de archivo
    pclose(pipe);
    free(command);
    
    return 0;
}