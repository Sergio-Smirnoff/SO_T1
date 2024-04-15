#include "application.h"
#include <string.h>

//buff_readER

char buff_read[2*SIZE_OF_BUFF] = {0};
char buff_write[SIZE_OF_BUFF] = {0};



int main(int argc, char *argv[]){

    int amount_files;
    
    if(argc < 2){
        perror("No files were given");
        exit(1);
    }else if (argc == 2){
        amount_files = argc - 1;
    }else{
        if ( strcmp(argv[argc-2],"|") == 0 )
            amount_files = argc-3;
        else
        // case: ./application(0) files/* (1 a n)
            amount_files = argc - 1;
    }
    
    if (amount_files == 0)
    {
        printf("Usage: %s <files>", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    char *shm_name="/shm_app";
    shmADT shm=create_shared_mem(shm_name);
    
    printf("%d\n", amount_files);
    // Get amount of slaves
    int slaves_amount = get_amount_of_slaves(amount_files);
    printf("%d\n", slaves_amount);
    struct processCDT processes[slaves_amount];
    /*
    sleep(2);
    todo lo de shm
    */
    fd_set setin;
    fd_set setout;

    
    create_slaves(slaves_amount, processes);

    work_distributor(argv, amount_files, slaves_amount, processes, &setin, &setout, shm);

    //close_all_fd(processes,slaves_amount);
    
    // mandar para finalizar vista
    //write_shared_mem(shm,EOF);

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

    printf("Termina el TPPPPPPPPPP...");

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
    printf("Asigno\n");
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
        printf("desasigno\n");
        if ( in_out == 2 ){
            close(processes[i].fd_read);
            if (close(processes[i].fd_write) == -1){
            perror("Error when closing");
            exit EXIT_FAIL;
            }
        }else if(in_out == 1){
            if (close(processes[i].fd_write) == -1){
            perror("Error when closing");
            exit EXIT_FAIL;
            }
        }
        else if(in_out == 0){
            if (close(processes[i].fd_read) == -1){
            perror("Error when closing");
            exit EXIT_FAIL;
            }
        }
    }
}


void work_distributor(char* files[], int amount_files,int slaves_amount, struct processCDT processes[], fd_set *set_in, fd_set *set_out,shmADT shm ){
    
    files_distributor(files, amount_files,slaves_amount,processes, set_in, set_out , shm);

    // chequear que todos los pipes ya hayan terminado
    // cierro desde este lado los pipes para mandar eof a los slaves
    close_selected_fd( processes, slaves_amount, 0);
    // seteo de lectura

    while(!are_all_fd_close(processes,slaves_amount)) // 1
    {
        finish_hearing(slaves_amount, processes, set_out, shm);
    }
    
}

void files_distributor( char* files[], int amount_files,int slaves_amount, struct processCDT processes[], fd_set *set_in, fd_set *set_out,shmADT shm  ){
    while (amount_files > 0)
    {
        int max_fd = set_fds(processes, slaves_amount, set_in, set_out);
        // stat de pipes
        int status = select(max_fd + 1, set_in, set_out, NULL, NULL);
        if (status < 0)
        {
            exit(EXIT_FAILURE);
        }
        // por la cantidad de modificados, cantidad por leer

        for (int slave= 0; slave < slaves_amount && status > 0; slave++)
        {
            if (FD_ISSET(processes[slave].fd_read , set_in) != 0){
                // le paso un nuevo archivo

                
                sprintf(buff_write,"%s",files[amount_files--]);
                int wr = write( processes[slave].fd_write, buff_write, sizeof(buff_write));
                if (wr < 0){
                    perror("write");
                    exit(1);
                }
                clear_buff(1);
                status--;
            }

            if (FD_ISSET(processes[slave].fd_write, set_out) != 0){
                //escribo en la share memory
                ssize_t read_bytes = read( processes[slave].fd_read, buff_read, sizeof(buff_read) );
                //write_shared_mem(shm, buff_read);
                printf("%s\n",buff_read);
                clear_buff(0);
                status--;
            }
        }
    }
    return;
}

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
                //write_shared_mem(shm, buff_read);
                printf("%s\n",buff_read);
                clear_buff(0);
                status--;
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
/*
void work_distributor(char* files[], int amount_files,int slaves_amount, p_process processes[], fd_set *set_in, fd_set *set_out,shmADT shm ){
    while (amount_files > 0)
    {
        int max_fd = set_fds(processes, slaves_amount, set_in, set_out);
        // stat de pipes
        int status = select(max_fd + 1, set_in, set_out, NULL, NULL);
        if (status < 0)
        {
            exit(EXIT_FAILURE);
        }
        // por la cantidad de modificados, cantidad por leer

        for (int slave= 0; slave < slaves_amount && status > 0; slave++)
        {
            if (FD_ISSET(processes[slave]->fd_read , set_in) == 0)
                continue;
            else
            {
                // le paso un nuevo archivo
                sprintf(processes[slave]->fd_read,"%s",files[amount_files--]);
                status--;
            }

            if (FD_ISSET(processes[slave]->fd_write, set_out) == 0)
                continue;
            else{
                //escribo en la share memory
                ssize_t read_bytes = read( processes[slave]->fd_write, buff_read, SIZE_OF_BUFF );
                write_shared_mem(shm, buff_read);
                clear_buff_read();
                status--;
            }
        }
    }
    // chequear que todos los pipes ya hayan terminado
    // cierro desde este lado los pipes para mandar eof a los slaves
    for( int j=0; j < slaves_amount; j++ ){
        if (close(processes[j]->fd_read) == -1){
            perror("Error when closing");
            exit EXIT_FAIL;
        }
    }
    // seteo de lectura

    //optimizar porque no hace falta inicializar el setin
    while(are_all_fd_close(processes,slaves_amount))
    {
        int max_fd = set_fds(processes, slaves_amount, NULL, set_out);
        int status = select(max_fd + 1, NULL, set_out, NULL, NULL);
        if (status < 0)
        {
            exit(EXIT_FAILURE);
        }
        for (int slave= 0; slave < slaves_amount && status > 0; slave++)
        {

            if (FD_ISSET(processes[slave]->fd_write, set_out) == 0)
                continue;
            else{
                //escribo en la share memory
                ssize_t read_bytes = read( processes[slave]->fd_write, buff_read, SIZE_OF_BUFF );
                if ( read_bytes == 0 )
                {
                    if (close(processes[slave]->fd_write) == -1)
                    {
                        perror("Error when closing");
                        exit EXIT_FAIL;
                    }
                    processes[slave]->fd_write = 0;
                    clear_buff_read();
                    continue;
                }
                write_shared_mem(shm, buff_read);
                clear_buff_read();
                status--;
            }
        }
    }
    // mandar un EOF para que finalice el vista
    write_shared_mem(shm,EOF);
}

// setea los fds y retorna el fd mas alto para usarlo en el select
int set_fds(p_process slaves[], int num_slaves, fd_set *set_in, fd_set *set_out)
{
    int max_fd = 0;
    if(set_in != NULL){
        FD_ZERO(set_in);
        for (int i = 0; i < num_slaves; i++)
        {
            int fd_in = slaves[i]->fd_read;
            FD_SET(fd_in, set_in);
            max_fd = (fd_in > max_fd) ? fd_in : max_fd;
        }
    }

    FD_ZERO(set_out);
    for (int i = 0; i < num_slaves; i++)
    {
        int fd_out = slaves[i]->fd_write;
        FD_SET(fd_out, set_out);
        max_fd = (fd_out > max_fd) ? fd_out : max_fd;
    }
    return max_fd;
}

int are_all_fd_close(p_process processes[], int amount_slaves){

    for(int slave=0; slave < amount_slaves; slave++){
        if ( processes[slave]->fd_write != 0 ){
            return 1;
        }
    }
    return 0;
}
*/