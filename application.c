#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/select.h>

// DEFINES
#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2
#define SLAVES_COUNT_INIT 5
#define FILES_LIMIT 100
#define SIZE_OF_BUFF 256

//BUFFER
char buff[SIZE_OF_BUFF];

// STRUCTURES
typedef struct process
{
    pid_t pid;
    int fd_read;
    int fd_write;
} process;

typedef process *p_process;

// PROTOTYPES
int get_amount_of_slaves(int amount_of_files);
void create_slaves(int slave_amount, char *files[], int *amount_of_files);
void create_slave(char *file1, char *file2);
int set_fds(process *slaves, int num_slaves, fd_set *setin, fd_set *setout);

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

    fd_set setin;
    fd_set setout;
    p_process slaves;

    create_slaves(slaves_amount, argv, &amount_files); // files are stored in argv

    while (amount_files > 0)
    {
        int max_fd = set_fds(slaves, slaves_amount, &setin, &setout);
        // stat de pipes
        int status = select(max_fd + 1, &setin, &setout, NULL, NULL);
        if (status < 0)
        {
            exit(EXIT_FAILURE);
        }
        // por la cantidad de modificados, cantidad por leer
        // isSet
        for (int fd = 3; fd < max_fd && status > 0; fd++)
        {
            if (FD_ISSET(fd, &setin) == 0)
                continue;
            else
            {
                // le paso un nuevo archivo
                sprintf(fd,"%s",argv[amount_files--]);
                status--;
            }

            if (FD_ISSET(fd, &setout) == 0)
                continue;
            else{ // mandar un EOF para que finalice el vista
                //escribo en la share memory
                //chequear
                char* to_write;
                ssize_t read_bytes = read( fd, buff, SIZE_OF_BUFF );
                status--;
            }
        }

        // escritura de los pipes
    }
    

    // waitpids y cerrar todos los fd.
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

void create_slaves(int slave_amount, char *files[], int *amount_of_files)
{
    for (int i = slave_amount; slave_amount > 0, *amount_of_files > 0; slave_amount--, (*amount_of_files) -= 2)
    {
        // in case there is only one file left, the slave only receives one file and NULL
        if (amount_of_files == 1)
        {
            create_slave(files[*amount_of_files], NULL);
        }
        create_slave(files[*amount_of_files], files[*amount_of_files - 1]);
    }
}

void create_slave(char *file1, char *file2)
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
    // agregar al set los fd
}

// setea los fds y retorna el fd mas alto para usarlo en el select
int set_fds(process *slaves, int num_slaves, fd_set *set_in, fd_set *set_out)
{
    FD_ZERO(set_in);
    FD_ZERO(set_out);
    int max_fd = 0;
    for (int slave = 0; slave < num_slaves; slave++)
    {
        int fd_in = slaves->fd_read;
        int fd_out = slaves->fd_write;
        FD_SET(fd_in, set_in);
        FD_SET(fd_out, set_out);
        max_fd = (fd_in > max_fd) ? fd_in : ((fd_out > max_fd) ? fd_out : max_fd);
    }
    return max_fd;
}

/*
pid_t pid
int fd_read
int fd_write
*/