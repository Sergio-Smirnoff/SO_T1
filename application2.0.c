#include "application.h"
#include <string.h>

//buff_readER

char buff_read[2*SIZE_OF_BUFF] = {0};
char buff_write[SIZE_OF_BUFF] = {0};



int main(int argc, char *argv[]){

    setvbuf(stdin,NULL,_IONBF,0);
    setvbuf(stdout,NULL,_IONBF,0);

    int amount_files;
    
    if(argc < 2){
        perror("No files were given");
        exit(1);
    }else {
        amount_files = argc - 1;
    }
    
    if (amount_files == 0)
    {
        printf("Usage: %s <files>", argv[0]);
        exit(EXIT_FAILURE);
    }
    /*
    int a = amount_files;

    while ( a > 0){
        printf("%s\n", argv[a--]);
    }
    */
    
    char *shm_name="/shm_app";
    shmADT shm=create_shared_mem(shm_name);
    if (shm == NULL) {
        printf("Failed to create shared memory\n");
        return EXIT_FAILURE;
    }
    printf("%s",shm_name);
/*
    // Abre la memoria compartida (revisar)
    printf("Parent process: Opening shared memory...\n");
    shmADT parent_shm = open_shared_mem(shm_name);
    if (parent_shm == NULL) {
        printf("Parent process: Failed to open shared memory\n");
        close_and_delete_shared_mem(shm);
        return EXIT_FAILURE;
    }*/
    
    //printf("%d\n", amount_files);
    // Get amount of slaves
    int slaves_amount = get_amount_of_slaves(amount_files);
    //printf("%d\n", slaves_amount);
    struct processCDT processes[slaves_amount];
    /*
    sleep(2);
    todo lo de shm
    */
    create_slaves(slaves_amount, processes);

    work_distributor(argv, amount_files, slaves_amount, processes, shm);

    close_selected_fd(processes,slaves_amount, 0);
    
    // mandar para finalizar vista
    //write_shared_mem(shm,EOF);

    raise_finish_reading(shm);
    
    sleep(2);
    close_and_delete_shared_mem(shm);
    
    pid_t pid;
    int status;
    while((pid=waitpid(-1,&status,0)) > 0 ){
        if (WIFEXITED(status)) {
            printf("Proceso hijo %d terminado con estado de salida %d\n", pid, WEXITSTATUS(status));
        } else {
            printf("Proceso hijo %d terminado de forma anormal\n", pid);
        }
    }

   return 0;

}

int get_amount_of_slaves(int amount_of_files)
{
    // if amount_of_files < 100 then 5 slaves, else floor(amount_of_files / 100) * 5
    // e.g. amount_of_files = 250 then floor(2.5) * 5 = 10

    if (amount_of_files < FILES_LIMIT)
    {
        return SLAVES_COUNT_INIT;
    }
    return floor(amount_of_files / FILES_LIMIT) * SLAVES_COUNT_INIT;
}

void create_slaves(int slave_amount, struct processCDT processes[])
{
    for (int slave = 0; slave < slave_amount; slave++)
    {
        create_slave(&processes[slave]);
    }
}

void create_slave(struct processCDT* process)
{
    int p_master_slave[2];
    int p_slave_master[2];

    char *const extargv[] = {"./slaves", NULL};
    char *const *extenv  = {NULL};

    if (pipe(p_master_slave) != 0)
        exit(EXIT_FAILURE);
    if (pipe(p_slave_master) != 0)
        exit(EXIT_FAILURE);

    pid_t pid = fork();
    
    if(pid < 0){
        perror("Error in creation of slave");
        exit(EXIT_FAILURE);
    }else if (pid == 0)
    {
        close(0);
        close(1);
        dup(p_master_slave[0]);
        dup(p_slave_master[1]);

        close(p_master_slave[0]);
        close(p_master_slave[1]);
        close(p_slave_master[0]);
        close(p_slave_master[1]);
        
        execve("./slaves", extargv, extenv);
        perror("execve");
        exit(EXIT_SUCCESS);
    }
    //closing the fd that we are not going to use
    close(p_master_slave[0]);
    close(p_slave_master[1]);

    // agregar al struct los fd
    process->pid = pid;
    process->fd_read = p_slave_master[0];
    process->fd_write = p_master_slave[1];
    
    fsync(1);

    return;
}

// setea los fds y retorna el fd mas alto para usarlo en el select
int set_fds(struct processCDT processes[], int num_slaves, fd_set *set_in, fd_set *set_out)
{
    if ( set_in == NULL && set_out == NULL )
        return -1;
    int max_fd = 0;
    if(set_in != NULL){
        FD_ZERO(set_in);
        for (int i = 0; i < num_slaves; i++)
        {
            int fd_in = processes[i].fd_read;
            FD_SET(fd_in, set_in);
            max_fd = (fd_in > max_fd) ? fd_in : max_fd;
        }
    }
    
    if(set_out != NULL){
        FD_ZERO(set_out);
        for (int i = 0; i < num_slaves; i++)
        {
            int fd_out = processes[i].fd_write;
            if ( fd_out != -1){
                FD_SET(fd_out, set_out);
            }
            
            max_fd = (fd_out > max_fd) ? fd_out : max_fd;
        }
    }
    
    
    return max_fd;
}

void close_selected_fd(struct processCDT processes[], int slave_amount, int in_out){
    for ( int i = 0; i < slave_amount; i++ ){
        if ( in_out == 2 ){
            close(processes[i].fd_read);
            if (close(processes[i].fd_write) == -1){
            perror("Error when closing1");
            exit EXIT_FAIL;
            }
        }else if(in_out == 1){
            if (close(processes[i].fd_write) == -1){
            perror("Error when closing2");
            exit EXIT_FAIL;
            }
        }
        else if(in_out == 0){
            if (close(processes[i].fd_read) == -1){
            perror("Error when closing3");
            exit EXIT_FAIL;
            }
        }
    }
}


void work_distributor(char* files[], int amount_files,int slaves_amount, struct processCDT processes[],shmADT shm ){
    
    /*
    for (int slave= 0; slave < slaves_amount && amount_files > 0; slave++)
        {
                // le paso un nuevo archivo
                sprintf(buff_write,"%s",files[amount_files--]);
                int wr = write( processes[slave].fd_write, buff_write, sizeof(buff_write));
                if (wr < 0){
                    perror("write");
                    exit(1);
                }
                clear_buff(1);
        }
    */
    
    files_distributor2(files, amount_files,slaves_amount,processes, shm);

    // chequear que todos los pipes ya hayan terminado
    // cierro desde este lado los pipes para mandar eof a los slaves
    close_selected_fd( processes, slaves_amount, 1);
    // seteo de lectura

    finish_hearing(slaves_amount, processes,shm);
    
}
/*
void files_distributor( char* files[], int amount_files,int slaves_amount, struct processCDT processes[], fd_set *set_in, fd_set *set_out,shmADT shm  ){
    while (amount_files > 0)
    {  
        int max_fd = set_fds(processes, slaves_amount, set_in, set_out);
        // stat de pipes
        int status = select(max_fd + 1, set_in, set_out, NULL, NULL);
        if (status < 0)
        {
            exit(EXIT_FAILURE);
        }else if (status == 0){
            continue;
        }
        // por la cantidad de modificados, cantidad por leer

        for (int slave= 0; slave < slaves_amount && status > 0; slave++)
        {
            
            if (FD_ISSET(processes[slave].fd_read, set_in)){
                //escribo en la share memory
                ssize_t read_bytes = read( processes[slave].fd_read, buff_read, sizeof(buff_read) );
                if (read_bytes < 0){
                    perror("read");
                    exit(1);
                }
                buff_read[read_bytes] = '\0';
                //write_shared_mem(shm, buff_read);
                printf("%s\n",buff_read);
                printf("Leyendo\n");
                clear_buff(0);
                status--;
            }


            if (FD_ISSET(processes[slave].fd_write , set_out)){
                // le paso un nuevo archivo

                sprintf(buff_write,"%s",files[amount_files--]);
                int wr = write( processes[slave].fd_write, buff_write, strlen(buff_write));
                if (wr < 0){
                    perror("write");
                    exit(1);
                }
                clear_buff(1);
                status--;
            }
            
        }
    }
    return;
}*/
/*
void finish_hearing(int slaves_amount, struct processCDT processes[], fd_set *set_out,shmADT shm ){
    int max_fd = set_fds(processes, slaves_amount, NULL, set_out);
        int status = select(max_fd + 1, NULL, set_out, NULL, NULL);
        if (status < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        for (int slave= 0; slave < slaves_amount && status > 0; slave++)
        {
            if (FD_ISSET(processes[slave].fd_write, set_out) != 0){
                //escribo en la share memory
                ssize_t read_bytes = read( processes[slave].fd_read, buff_read, sizeof(buff_read) );
                if (  read_bytes < 0){
                    perror("read");
                    exit(1);
                }else if ( read_bytes == 0 )
                {
                    if (close(processes[slave].fd_write) == -1)
                    {
                        perror("Error when closing");
                        exit EXIT_FAIL;
                    }
                    processes[slave].fd_write = -1;
                    clear_buff(0);
                    continue;
                }
                buff_read[read_bytes] = '\0';
                //write_shared_mem(shm, buff_read);
                printf("%s\n",buff_read);
                clear_buff(0);
                status--;
            }
        }
}*/

void finish_hearing(int slaves_amount, struct processCDT processes[],shmADT shm )
{
    int corte = slaves_amount;
    while( corte ){
        fd_set read_fds;
        int max_fd = -1;
        FD_ZERO(&read_fds);

        //seteo de set
        for (int j = 0; j < slaves_amount; j++) {
            FD_SET(processes[j].fd_read, &read_fds);
            max_fd = (processes[j].fd_read > max_fd) ? processes[j].fd_read : max_fd;
        }

        //select
        int select_result = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        for (int j = 0; j < slaves_amount; j++) {
            if (FD_ISSET(processes[j].fd_read, &read_fds)) {
                ssize_t len = read(processes[j].fd_read, buff_read, sizeof(buff_read));

                if (len < 0) {
                    perror("read");
                    return -1;
                }

                buff_read[len] = '\0';
                write_shared_mem(shm,buff_read);
                corte--;
            }
        }
        
    } 

}


int are_all_fd_close(struct processCDT processes[], int amount_slaves){

    for(int slave=0; slave < amount_slaves; slave++){
        if ( processes[slave].fd_write != -1 ){
            return 0;
        }
    }
    return 1;
}

void clear_buff(int in_out){
    if ( in_out == 1){
        for ( int i = 0; i < SIZE_OF_BUFF; i++ )
    {
        buff_write[i] = '\0';
    }
    }else{
        for ( int i = 0; i < SIZE_OF_BUFF; i++ )
    {
        buff_read[i] = '\0';
    }
    }
    
}


//---------------------------------------------------------------------------------------------------------------------------------------------

void files_distributor2( char* files[], int amount_files,int slaves_amount, struct processCDT processes[],shmADT shm  ){
   
    fd_set read_fds;
    int max_fd = -1;
    
    for (int slave= 0; slave < slaves_amount && amount_files > 0; slave++)
        {
                // le paso un nuevo archivo
                sprintf(buff_write,"%s",files[amount_files--]);
                int wr = write( processes[slave].fd_write, buff_write, sizeof(buff_write));
                if (wr < 0){
                    perror("write");
                    exit(1);
                }
                clear_buff(1);
        }


    while (amount_files > 0)
    {  
       FD_ZERO(&read_fds);

       for (int j = 0; j < slaves_amount; j++) {
            FD_SET(processes[j].fd_read, &read_fds);
            max_fd = (processes[j].fd_read > max_fd) ? processes[j].fd_read : max_fd;
        }

        int select_result = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (select_result < 0) {
            perror("select");
            return -1;
        } else if (select_result == 0) {
            // No hay nada listo, sigo
            continue;
        } else {

            for (int j = 0; j < slaves_amount && amount_files > 0; j++) {
                if (FD_ISSET(processes[j].fd_read, &read_fds)) {
                    ssize_t len = read(processes[j].fd_read, buff_read, sizeof(buff_read));

                    if (len < 0) {
                        perror("read");
                        return -1;
                    }

                    buff_read[len] = '\0';
                    //printf("%s",buff_read);
                    write_shared_mem(shm,buff_read);

                    sprintf(buff_write,"%s",files[amount_files--]);
                    write(processes[j].fd_write, buff_write, sizeof(buff_write));
                    clear_buff(1);
                    clear_buff(0);
                }
            }
        }
    }
    
}