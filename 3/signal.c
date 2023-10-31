#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include<unistd.h>

void handler(int sigNumber){
    printf("My signal number is %i\n", sigNumber);
};
void handlerSigKill(int sigNumber){
    printf("My signal number is %i\n", sigNumber);
};

int main()
{
    int i = 0;
    signal(SIGTERM, handler);
    signal(SIGKILL, handlerSigKill);
    while (1)
    {
        printf("i = %i\n", i);
        i++;
        sleep(1);
    }
    return 0;

}