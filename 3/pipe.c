#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   //fork
#include <sys/wait.h> //waitpid
#include <errno.h>
#include "time.h"

pid_t mainProcessValue = 0;
int commonValue = 1;

pid_t child(int pipe_id_rec, int pipe_id_send)
{
    pid_t process = fork();
    if (process == -1)
    {
        exit(-1);
    }
    if (process > 0)
    {
        return process;
    }
    int arrived;
    char str[] = "Child message to the parent... \n.";
    read(pipe_id_rec, &arrived, sizeof(int));
    printf("Parent message to the child: '%i'\n", arrived);
    printf("\n");
    write(pipe_id_send, str, sizeof(str));

    exit(0);
}

int main()
{
    mainProcessValue = getpid();
    int ioPipe[2];
    int succ = pipe(ioPipe);
    if (succ == -1)
        exit(-1);
    int ioPipe1[2];
    int succ1 = pipe(ioPipe1);
    if (succ1 == -1)
        exit(-1);

    char pufferChar[50];

    child(ioPipe1[0], ioPipe[1]); // 0 read, 1 write

    srand(time(NULL));
    int r = rand() % 100;
    printf("Random number %i\n", r);
    printf("\n");

    write(ioPipe1[1], &r, sizeof(r));
    printf("Parent send a random number\n");

    read(ioPipe[0], pufferChar, 50);
    printf("Message from child: '%s'\n", pufferChar);

    wait(NULL);
    close(ioPipe[0]);
    close(ioPipe[1]);
    close(ioPipe1[0]);
    close(ioPipe1[1]);
    return 0;
}
