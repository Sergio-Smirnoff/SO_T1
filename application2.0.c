#include "application.h"

//BUFFER
char buff[SIZE_OF_BUFF];



int main(int argc, char *argv[]){

    int amount_files;
    
    if(argc < 2){
        perror("NO Files were given");
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
    printf("%d\n", amount_files);
    // Get amount of slaves
    int slaves_amount = get_amount_of_slaves(amount_files);
    printf("%d\n", slaves_amount);
    struct processCDT processes[slaves_amount];
    /*
    sleep(2);
    todo lo de shm
    */
    create_slaves(slaves_amount, processes);
    
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
        create_slave(processes[slave]);
    }
}

void create_slave(struct processCDT process)
{
    printf("Asigno\n");
    int p_master_slave[2];
    int p_slave_master[2];

    char *const extargv[] = {"./slave", NULL};
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
        

        execve("./slave", extargv, extenv);
        perror("execve");
        exit(EXIT_SUCCESS);
    }
    //closing the fd that we are not going to use
    close(p_master_slave[0]);
    close(p_slave_master[1]);

    // agregar al struct los fd
    process.pid = pid;
    process.fd_read = p_slave_master[0];
    process.fd_write = p_master_slave[1];
    
    close(process.fd_read);
    close(process.fd_write);
    //free(process);
    fsync(1);

    return;
}
