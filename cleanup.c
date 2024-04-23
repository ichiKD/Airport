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


/*
For plane to ATC
Message.sender = 1;
Message.r = plane_data;

For ATC to plane
Message.sender = 2;
Message.CONFORMATION_plane = CONFORMATION_plane;

For ATC to airport for departure
Message.sender = 2;
Message.FOR_DEPARTURE = 1;
Message.r = r;

For airport to ATC (When boarding: DEPARTURE_case = 1)
Message.sender = 3;
Message.FOR_DEPARTURE = 1;
Message.DEPARTURE_case = 1;
Message.CONFORMATION_boarding = CONFORMATION_boarding;
Message.r = r;

For airport to ATC (When takeoff: DEPARTURE_case = 2)
Message.sender = 3;
Message.FOR_DEPARTURE = 1;
Message.DEPARTURE_case = 2;
Message.CONFORMATION_takeoff = CONFORMATION_takeoff;
Message.r = r;

For ATC to airport for arrival
Message.sender = 2;
Message.FOR_DEPARTURE = 0;
Message.r = r;


For airport to ATC (When landing: ARRIVAL_case = 1)
Message.sender = 3;
Message.FOR_DEPARTURE = 0;
Message.ARRIVAL_case = 1;
Message.CONFORMATION_landing = CONFORMATION_landing;
Message.r = r;

For airport to ATC (When deboarded: ARRIVAL_case = 2)
Message.sender = 3;
Message.FOR_DEPARTURE = 0;
Message.ARRIVAL_case = 2;
Message.CONFORMATION_deboarded = CONFORMATION_deboarded;
Message.r = r;

For ATC to airport
Message.sender = 2;
Message.TERMINATION = TERMINATION;

For cleanup to ATC
Message.sender = 4;
Message.TERMINATION = 1;




*/




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