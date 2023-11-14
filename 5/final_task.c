#include "sys/types.h"
#include "unistd.h"
#include "stdlib.h"
#include "signal.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "wait.h"
#include "sys/ipc.h"
#include "sys/msg.h"
#include "sys/shm.h"
#include "sys/sem.h"
#include "sys/stat.h"

// Sajtótájékoztató kezdődik, ahol a moderátor, (szülő folyamat) a nyilatkozó (gyerek) és a szakértő (gyerek) vesznek részt és válaszolnak a sajtó kérdéseire. 
// A moderátor irányítja a teljes sajtótájékoztatót és megvárja amig megválaszolásra kerülnek a kérdések. (Szülő bevárja a gyerek folyamat végét, és végig fenntartja 
// a kapcsolatot)
// 1, A Sajtótájékoztató kezdetekor a moderátor (szülő) megvárja amig mind két további résztvevő (gyerekek) elfoglalja a helyét, 
// és jelzést küldenek ( signal ), hogy készen állnak a kezdésre. (Konzolra kiír) Ezt követően a moderátor csövön (pipe) keresztül feltesz egy kérdést a szakértőnek, 
// megerősítésre várva – „Megvan minden dokumentuma?”. Amire csövön keresztül válaszol, hogy „Igen” (A kommunikáció konzolon kiírásra kerül)
// 2, Ezt követően a nyilatkozó üzenetsoron keresztül (message queue) hozzászól szintén a kérdéshez, (moderátorhoz) hogy 
// „Igen méréseink vannak, amiket publikálni fogunk.” A moderátor megjeleníti ezt szintén a konzolon.
// 3, A nyilatkozó emellett egy mérési számot is megemlít. 
// (felvett tetszőleges változó) amit osztott memóriába ír. Ahonnan a moderátor kiolvassa és megjeleníti.
// 4, Védje szemaforral az osztott memória használatát.

pid_t mainProcessValue = 0;
int ready = 0;
int messageQueue;
int semid;
struct sharedData *s;

struct message 
{
    long mtype;
    char mtext[1024];
};

struct sharedData
{
    char text[1024];
};

int semaphoreCreation(const char *pathname, int semaphoreValue)
{
    int semid;
    key_t key;

    key = ftok(pathname, 1);
    if((semid = semget(key, 1, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
        perror("semget");
    if(semctl(semid, 0, SETVAL, semaphoreValue) < 0)
        perror("semctl");
    return semid;
}

void semapthoreOperation(int semid, int op) 
{
    struct sembuf opreration;

    opreration.sem_num = 0;
    opreration.sem_op = op;
    opreration.sem_flg = 0;

    if(semop(semid, &opreration, 1) < 0)
        perror("semop");
}

void semaphoreDelete(int semid) 
{
    semctl(semid, 0, IPC_RMID);
}

void readyHandler(int sig) {
    if(sig == SIGUSR1) {
        ready++;
    }
}

pid_t Expert(int pipe_id_rec, int pipe_id_send) 
{
    pid_t process = fork();
    if(process == -1)
        exit(-1);
    if(process > 0)
    {
        return process;
    }

    kill(mainProcessValue, SIGUSR1);

    char puffer[50];
    read(pipe_id_rec, puffer, sizeof(puffer));
    printf("Kapott kérdés: %s\n", puffer);
    char answer[] = "Igen";
    write(pipe_id_send, answer, sizeof(answer));

    exit(0);
}

pid_t Statement() 
{
    pid_t process = fork();
    if(process == -1)
        exit(-1);
    if(process > 0)
    {
        return process;
    }

    kill(mainProcessValue, SIGUSR1);

    int status;
    struct message ms = {5, "Igen méréseink vannak, amiket publikálni fogunk."};
    status = msgsnd(messageQueue, &ms, strlen(ms.mtext) + 1, 0);
    if(status < 0)
    {
        perror("msgsnd");
    }

    char newData[50] = "16 százalék erre az esély";
    semapthoreOperation(semid, -1);
    strcpy(s->text, newData);
    semapthoreOperation(semid, 1);
    shmdt(s);
    exit(0);
}


int main(int argc, char **argv)
{
    mainProcessValue = getpid();
    signal(SIGUSR1, readyHandler);

    int status;
    key_t mainKey;

    mainKey = ftok(argv[0], 1);
    messageQueue = msgget(mainKey, 0600 | IPC_CREAT);
    if(messageQueue < 0)
    {
        perror("msgget");
        return 1;
    }

    int sh_mem_id;
    sh_mem_id = shmget(mainKey, sizeof(s), IPC_CREAT | S_IRUSR | S_IWUSR);
    s = shmat(sh_mem_id, NULL, 0);

    semid = semaphoreCreation(argv[0], 1);

    int io_pipes[2];
    int succ = pipe(io_pipes);
    if(succ == -1)
        exit(-1);

    int io_pipes1[2];
    int succ1 = pipe(io_pipes1);
    if(succ1 == -1)
        exit(-1);

    Expert(io_pipes1[0], io_pipes[1]);
    Statement();

    while(ready < 1)
        ;
    puts("Nyilatkozó készen van");
    while(ready < 2)
        ;
    puts("Szakértő készen van");

    char puffer[50];
    char question[] = "Megvan minden dokumentuma?";
    write(io_pipes1[1], question, sizeof(question));
    read(io_pipes[0], puffer, sizeof(puffer));
    printf("Szakértő válasza: %s\n", puffer);

    struct message ms;
    status = msgrcv(messageQueue, &ms, 1024, 5, 0);
    if(status < 0)
    {
        perror("msgrcv");
    }
    else 
    {
        printf("A kapott üzenet a nyilatkozótól kódja: %ld, szövege: %s \n", ms.mtype, ms.mtext);
    }

    semapthoreOperation(semid, -1);
    printf("Közölt mérési szám: %s\n", s->text);
    semapthoreOperation(semid, 1);

    shmdt(s);
    wait(NULL);
    wait(NULL);
    semaphoreDelete(semid);
    close(io_pipes[0]);
    close(io_pipes1[0]);
    close(io_pipes[1]);
    close(io_pipes1[1]);
    status = msgctl(messageQueue, IPC_RMID, NULL);
    if(status < 0)
    {
        perror("msgctl");
    }
    return 0;
}