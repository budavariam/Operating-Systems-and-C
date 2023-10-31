#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

/*
 SIGTERM -> 15
 SIGKILL -> 9

 pl.
 kill -15 "pidje a processnek amit ps -aux al kérdezel le a felhasználódra""

*/

void handler(int signumber)
{
    printf("My signal handler %i\n", signumber);
}

void handler_sigkill(int signumber)
{
    printf("My signal handler %i\n", signumber);
}

int main()
{

    int i = 0;
    signal(SIGTERM, handler);
    signal(SIGKILL, handler_sigkill);

    while (1)
    {
        printf("i = %i\n", i);
        i++;
        sleep(1);
    }
    return 0;
}