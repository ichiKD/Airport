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
#include <string.h>
#define MESSAGE_TYPE 1

/*
int message_sender:
1 = plane
2 = ATC
3 = airport
4 = clean up 
*/
int message_sender_id = 4;


struct Plane{
    int arrival_airport, departure_airport;
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


void print_message(struct Message msg) {
    printf("Message Details:\n");
    printf("Sender: %d\n", msg.sender);
    printf("Confirmation Plane: %d\n", msg.CONFORMATION_plane);
    printf("Termination: %d\n", msg.TERMINATION);
    printf("For Departure: %d\n", msg.FOR_DEPARTURE);
    printf("Departure Case: %d\n", msg.DEPARTURE_case);
    printf("Arrival Case: %d\n", msg.ARRIVAL_case);
    printf("Confirmation Boarding: %d\n", msg.CONFORMATION_boarding);
    printf("Confirmation Takeoff: %d\n", msg.CONFORMATION_takeoff);
    printf("Confirmation Landing: %d\n", msg.CONFORMATION_landing);
    printf("Confirmation Deboarded: %d\n", msg.CONFORMATION_deboarded);
    printf("Plane Details:\n");
    printf("Arrival Airport: %d\n", msg.r.arrival_airport);
    printf("Departure Airport: %d\n", msg.r.departure_airport);
    printf("Plane ID: %d\n", msg.r.plane_id);
    printf("Total Weight: %d\n", msg.r.total_weight);
    printf("Plane Type: %d\n", msg.r.plane_type);
    printf("Passengers: %d\n", msg.r.passengers);
}

struct MessageBuffer {
    long mtype; 
    char mtext[sizeof(struct Message)]; 
};
struct MessageBuffer msgbuf;



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
    struct Message msg = clean();
    msgbuf.mtype = MESSAGE_TYPE;
    memcpy(msgbuf.mtext, &msg, sizeof(struct Message));    // Copy the struct Message into the message buffer
    if (msgsnd(msgid, &msgbuf, sizeof(struct Message), IPC_NOWAIT) == -1) {
        perror("msgsnd");
        exit(1);
    }
    printf("Message sent successfully\n");

    return 0;
}