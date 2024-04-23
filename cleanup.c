#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>


/*
int message_sender:
1 = plane
2 = ATC
3 = airport
4 = clean up 
*/
int message_sender_id = 4;


struct Plane{
    int arrival_ariport, departure_airport;
    int plane_id, total_weight;
    int plane_type, passengers; 
};

struct Message{
    int sender;
    int CONFORMATION_plane;
    int TERMINATION;
    int FOR_DEPARTURE;
    int DEPARTURE_case, ARRIVAL_case;
    int CONFORMATION_boarding, CONFORMATION_takeoff; 
    int CONFORMATION_landing, CONFORMATION_deboarded;
    struct Plane r;
};

struct Message clean(){
    struct Message currentMessage;
    currentMessage.sender = 4;
    currentMessage.TERMINATION = 1;
    return currentMessage;
}




int main(){
    char cc;
    while(cc != 'Y'){
        printf("Do you want the Air Traffic Control System to terminate?(Y for Yes and N for No) ");
        fflush(stdout);
        scanf("%c", &cc);
        printf("\n");
        fflush(stdout);
        if(cc != 'N' && cc != 'Y'){
            printf("INVALID CHAR ENTERED\n");
        }
    }
    key_t key = ftok("progfile", 65);
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    int msgid = msgget(key, IPC_CREAT | 0666); // Get the message queue ID
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    sem_unlink("ATC");
    sem_t *semATC = sem_open("ATC", O_CREAT | O_EXCL, 0666, 0);
    if (semATC == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    sem_post(semATC);
    if (msgsnd(msgid, &message_sender_id, sizeof(int), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    return 0;
}