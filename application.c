#include "application.h"

char buff_read[2*SIZE_OF_BUFF] = {0};
char buff_write[SIZE_OF_BUFF] = {0};

int main(int argc, char *argv[]){

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

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

    char *shm_name="/shm_app";

    shmADT shm=create_shared_mem(shm_name);
    if (shm == NULL) {
        printf("Failed to create shared memory\n");
        return EXIT_FAILURE;
    }

    printf("%s",shm_name);

    int slaves_amount = get_amount_of_slaves(amount_files);
    struct processCDT processes[slaves_amount];

    create_slaves(slaves_amount, processes);

    work_distributor(argv, amount_files, slaves_amount, processes, shm);

    close_selected_fd(processes, slaves_amount, 0);

    sleep(2);
    raise_finish_reading(shm);
    close_and_delete_shared_mem(shm);

    pid_t pid;
    int status;
    while((pid=waitpid(-1, &status, 0)) > 0 ){
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
        create_slave(&processes[slave], slave, processes);
    }
}

void create_slave(struct processCDT* process, int index, struct processCDT processes[])
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
        close(STD_IN);
        close(STD_OUT);
        dup(p_master_slave[STD_IN]);
        dup(p_slave_master[STD_OUT]);

        for(int j=0; j< index; j++){
            close(processes[j].fd_read);
            close(processes[j].fd_write);
        }

        close(p_master_slave[STD_IN]);
        close(p_master_slave[STD_OUT]);
        close(p_slave_master[STD_IN]);
        close(p_slave_master[STD_OUT]);

        execve("./slaves", extargv, extenv);
        perror("execve");
        exit(EXIT_SUCCESS);
    }

    close(p_master_slave[STD_IN]);
    close(p_slave_master[STD_OUT]);

    process->pid = pid;
    process->fd_read = p_slave_master[STD_IN];
    process->fd_write = p_master_slave[STD_OUT];

    fsync(1);

    return;
}

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
                exit (EXIT_FAILURE);
            }
        }else if(in_out == 1){
            if (close(processes[i].fd_write) == -1){
                perror("Error when closing2");
                exit (EXIT_FAILURE);
            }
        }
        else if(in_out == 0){
            if (close(processes[i].fd_read) == -1){
                perror("Error when closing3");
                exit (EXIT_FAILURE);
            }
        }
    }
}


void work_distributor(char* files[], int amount_files, int slaves_amount, struct processCDT processes[], shmADT shm){
    FILE* file = fopen("archivo.txt", "w");

    files_distributor2(files, amount_files, slaves_amount, processes, shm, file); //pasar por parametro el file

    close_selected_fd( processes, slaves_amount, 1);

    finish_hearing(slaves_amount, processes, shm, file); //Pasar por parametro el file

    fclose(file);

}

void finish_hearing(int slaves_amount, struct processCDT processes[], shmADT shm, FILE* file)
{
    int corte = slaves_amount;
    while( corte>0 ){
        fd_set read_fds;
        int max_fd = -1;
        FD_ZERO(&read_fds);

        for (int j = 0; j < slaves_amount; j++) {
            FD_SET(processes[j].fd_read, &read_fds);
            max_fd = (processes[j].fd_read > max_fd) ? processes[j].fd_read : max_fd;
        }

        for (int j = 0; j < slaves_amount; j++) {
            if (FD_ISSET(processes[j].fd_read, &read_fds)) {
                ssize_t len = read(processes[j].fd_read, buff_read, sizeof(buff_read));

                if (len < 0) {
                    perror("read");
                    return;
                }

                buff_read[len] = '\0';
                write_shared_mem(shm,buff_read);
                fprintf(file, "%s\n", buff_read);
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
    if (in_out == 1){
        for (int i = 0; i < SIZE_OF_BUFF; i++)
        {
            buff_write[i] = '\0';
        }
    }else{
        for (int i = 0; i < SIZE_OF_BUFF; i++)
        {
            buff_read[i] = '\0';
        }
    }

}

void files_distributor2(char* files[], int amount_files, int slaves_amount, struct processCDT processes[], shmADT shm, FILE* file){

    fd_set read_fds;
    int max_fd = -1;

    for (int slave= 0; slave < slaves_amount && amount_files > 0; slave++)
    {
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
            return;
        } else if (select_result == 0) {

            continue;
        } else {

            for (int j = 0; j < slaves_amount && amount_files > 0; j++) {
                if (FD_ISSET(processes[j].fd_read, &read_fds)) {
                    ssize_t len = read(processes[j].fd_read, buff_read, sizeof(buff_read));

                    if (len < 0) {
                        perror("read");
                        return;
                    }

                    buff_read[len] = '\0';
                    write_shared_mem(shm,buff_read);
                    fprintf(file, "%s\n", buff_read);
                    sprintf(buff_write,"%s",files[amount_files--]);
                    write(processes[j].fd_write, buff_write, sizeof(buff_write));
                    clear_buff(1);
                    clear_buff(0);
                }
            }
        }
    }
}
