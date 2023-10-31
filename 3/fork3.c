#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   //fork
#include <sys/wait.h> //waitpid
#include <errno.h>

pid_t mainProcessValue = 0;
int commonValue = 1;

pid_t child()
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
   commonValue = 5;
   printf("The value is %i before forking (child)\n", commonValue);

   exit(0);
}

pid_t child2()
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
   commonValue = 15;
   printf("The value is %i after forking (child2)\n", commonValue);

   exit(0);
}

int main()
{
   commonValue = 2;
   printf("The value is %i after forking\n", commonValue);
   mainProcessValue = getpid();
   child();
   child2();
   wait(NULL);
   wait(NULL);

   printf("The value is %i after forking (parent)\n", commonValue);
   return 0;
}
