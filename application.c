#include "application.h"
//BUFFER
char buff[SIZE_OF_BUFF];

//Structure
typedef struct process
{
    pid_t pid;
    int fd_read;
    int fd_write;
} process;

int main(int argc, char *argv[])
{
    
    // case: ./application(0) files/*(1 a n) |(n+1) ./view(n+2)
    int amount_files;
    if ( argv[argc-2] == "|" )
        amount_files = argc-3;
    else
        // case: ./application(0) files/* (1 a n)
        amount_files = argc - 1;

    // Files validation
    if (amount_files == 0)
    {
        printf("Usage: %s <files>", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Get amount of slaves
    int slaves_amount = get_amount_of_slaves(amount_files);
    p_process processes[slaves_amount];

    fd_set setin;
    fd_set setout;
    p_process slaves;

    create_slaves(slaves_amount, argv, &amount_files, processes); // files are stored in argv

    //abro la memoria compartida y hago el while para acabar los archivos
    shmADT shm = create_shared_mem(MEM_NAME);
    
    work_distributor(argv, amount_files, slaves_amount, processes, &setin, &setout, shm);
    
    // waitpids

    close_and_delete_shared_mem(shm);

    pid_t pid;
    int status;
    while(pid=waitpid(-1,&status,0) ){
        if (WIFEXITED(status)) {
            printf("Proceso hijo %d terminado con estado de salida %d\n", pid, WEXITSTATUS(status));
        } else {
            printf("Proceso hijo %d terminado de forma anormal\n", pid);
        }
    }
    return EXIT_SUCCESS;
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

//chequear copia del array...

void create_slaves(int slave_amount, char *files[], int *amount_of_files, p_process processes[])
{
    for (int i = slave_amount; slave_amount > 0, *amount_of_files > 0; slave_amount--, (*amount_of_files) -= 2)
    {
        // in case there is only one file left, the slave only receives one file and NULL
        if (amount_of_files == 1)
        {
            create_slave(files[*amount_of_files], NULL, processes[i]);
        }
        create_slave(files[*amount_of_files], files[*amount_of_files - 1], processes[i]);
    }
}

void create_slave(char *file1, char *file2, p_process process)
{
    int p[2];
    char *argv[] = {"./slave", file1, file2, NULL};
    char *env = {NULL};
    pipe(p);

    pid_t pid = fork();
    if (pid == 0)
    {
        close(STD_IN);
        close(STD_OUT);
        dup(p[STD_IN]);
        dup(p[STD_OUT]);
        close(p[STD_IN]);
        close(p[STD_OUT]);

        execve(argv[1], argv, env);
    }
    else if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    // agregar al struct los fd
    process->pid = pid;
    process->fd_read = p[0];
    process->fd_write = p[1];

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

// modularizar work distributor

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
                ssize_t read_bytes = read( processes[slave]->fd_write, buff, SIZE_OF_BUFF );
                write_shared_mem(shm, buff);
                clear_buff();
                status--;
            }
        }
    }
    // chequear que todos los pipies ya hayan terminado
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
                ssize_t read_bytes = read( processes[slave]->fd_write, buff, SIZE_OF_BUFF );
                if ( read_bytes == 0 )
                {
                    if (close(processes[slave]->fd_write) == -1)
                    {
                        perror("Error when closing");
                        exit EXIT_FAIL;
                    }
                    processes[slave]->fd_write = 0;
                    clear_buff();
                    continue;
                }
                write_shared_mem(shm, buff);
                clear_buff();
                status--;
            }
        }
    }
    // mandar un EOF para que finalice el vista
    write_shared_mem(shm,EOF);
}

void clean_buff(){
    for ( int i = 0; i < SIZE_OF_BUFF; i++ )
    {
        buff[i] = '\0';
    }
}


int are_all_fd_close(p_process processes[], int amount_slaves){
    for(int slave=0; slave < amount_slaves; slave++){
        if ( processes[slave]->fd_write != 0 ){
            return 1;
        }
    }
    return 0;
}


