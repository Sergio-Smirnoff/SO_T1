#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2

#define SLAVES_COUNT_INIT 5
#define FILES_LIMIT 100

int get_amount_of_slaves(int amount_of_files);

int main(int argc, char *argv[])
{

    int amount_files = argc - 1;

    // Files validation
    if (amount_files == 0)
    {
        printf("Usage: %s <files>", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Get amount of slaves 
    int slaves_amount = get_amount_of_slaves(amount_files);
    create_slaves(slaves_amount, argv, &amount_files); // files are stored in argv

    while (amount_files > 0)
    {
        // stat de pipes

        // escritura de los pipes 
    }

    // waitpids
}

int get_amount_of_slaves(int amount_of_files)
{
    /*
        if amount_of_files < 100 then 5 slaves
        else floor(amount_of_files / 100) * 5
            e.g. amount_of_files = 250 then floor(2.5) * 5 = 10
    */
    if (amount_of_files < FILES_LIMIT)
    {
        return SLAVES_COUNT_INIT;
    }
    return floor(amount_of_files / FILES_LIMIT) * SLAVES_COUNT_INIT;
}

void create_slaves(int slave_amount, char * files[], int * amount_of_files)
{
    
    for (int i = slave_amount; slave_amount > 0, *amount_of_files > 0; slave_amount--, (*amount_of_files) -= 2)
    {
        // in case there is only one file left, the slave only receives one file and NULL
        if (amount_of_files == 1) {
            create_slave(files[*amount_of_files], NULL);
        }
        create_slave(files[*amount_of_files], files[*amount_of_files-1]);
    }
    
}

void create_slave(char* file1, char* file2)
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
}