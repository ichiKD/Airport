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
int message_sender_id = 3;
sem_t *semATC;
int msgid;

int airport_number, runways;
pthread_t *threads = NULL;
int *loadCapacity  = NULL;
int *busy          = NULL;
int back_up_BUSY   = 0;


struct Plane{
    int arrival_airport, departure_airport;
    int plane_id, total_weight;
    int plane_type, passengers; 
};
struct ThreadArgs {
    int idx;
    int FOR_DEPARTURE;
    struct Plane plane;
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


struct Message boarding(struct Plane r){
    struct Message currentMessage;
    currentMessage.sender = 3;
    currentMessage.FOR_DEPARTURE = 1;
    currentMessage.DEPARTURE_case = 1;
    currentMessage.CONFORMATION_boarding = 1;
    currentMessage.r = r;
    return currentMessage;
}


struct Message takeoff(struct Plane r){
    struct Message currentMessage;
    currentMessage.sender = 3;
    currentMessage.FOR_DEPARTURE = 1;
    currentMessage.DEPARTURE_case = 2;
    currentMessage.CONFORMATION_takeoff = 1;
    currentMessage.r = r;
    return currentMessage;
}

struct Message landing(struct Plane r){
    struct Message currentMessage;
    currentMessage.sender = 3;
    currentMessage.FOR_DEPARTURE = 0;
    currentMessage.ARRIVAL_case = 1;
    currentMessage.CONFORMATION_landing = 1;
    currentMessage.r = r;
    return currentMessage;
}
struct Message deboarded(struct Plane r){
    struct Message currentMessage;
    currentMessage.sender = 3;
    currentMessage.FOR_DEPARTURE = 0;
    currentMessage.ARRIVAL_case = 2;
    currentMessage.CONFORMATION_deboarded = 1;
    currentMessage.r = r;
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




void * useRunways(void *args){
    struct ThreadArgs *threadArg = (struct ThreadArgs *) args;
    int idx                      = threadArg->idx;
    int FOR_DEPARTURE            = threadArg->FOR_DEPARTURE;
    struct Plane r               = threadArg->plane;

    if(idx == runways){
        back_up_BUSY = 1;
    }
    else{
        busy[idx]    = 1;
    }
    if(FOR_DEPARTURE == 1){
        sleep(3);
        printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n",
                r.plane_id, idx + 1, airport_number);
        struct Message msg = boarding(r);
        sem_post(semATC);
        msgbuf.mtype = MESSAGE_TYPE;
        memcpy(msgbuf.mtext, &msg, sizeof(struct Message));    // Copy the struct Message into the message buffer
        if (msgsnd(msgid, &msgbuf, sizeof(struct Message), IPC_NOWAIT) == -1) {
            perror("msgsnd");
            exit(1);
        }

        sleep(2);
        msg = takeoff(r);
        sem_post(semATC);
        msgbuf.mtype = MESSAGE_TYPE;
        memcpy(msgbuf.mtext, &msg, sizeof(struct Message));    // Copy the struct Message into the message buffer
        if (msgsnd(msgid, &msgbuf, sizeof(struct Message), IPC_NOWAIT) == -1) {
            perror("msgsnd");
            exit(1);
        }
    }
    else if(FOR_DEPARTURE == 0){
        sleep(2);
        struct Message msg = landing(r);
        sem_post(semATC);
        msgbuf.mtype = MESSAGE_TYPE;
        memcpy(msgbuf.mtext, &msg, sizeof(struct Message));    // Copy the struct Message into the message buffer
        if (msgsnd(msgid, &msgbuf, sizeof(struct Message), IPC_NOWAIT) == -1) {
            perror("msgsnd");
            exit(1);
        }

        sleep(3);
        printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n",
                r.plane_id, idx + 1, airport_number);
        // SEND CONFORMATION TO ATC about deboarded
        msg = deboarded(r);
        sem_post(semATC);
        msgbuf.mtype = MESSAGE_TYPE;
        memcpy(msgbuf.mtext, &msg, sizeof(struct Message));    // Copy the struct Message into the message buffer
        if (msgsnd(msgid, &msgbuf, sizeof(struct Message), IPC_NOWAIT) == -1) {
            perror("msgsnd");
            exit(1);
        }

    }
    if(idx == runways){
        back_up_BUSY = 0;
    }
    else{
        busy[idx]    = 0;
    }
    pthread_exit(NULL);
}


int main(){
    printf("Enter Airport Number: ");
    fflush(stdout);
    scanf("%d", &airport_number);

    printf("\n");
    printf("Enter number of Runways: ");
    fflush(stdout);
    scanf("%d", &runways);

    printf("\n");
    printf("Enter loadCapacity of Runways (give as a space separated list in a single line):\n");
    fflush(stdout);
    threads      = (pthread_t *) malloc ((runways + 1) * sizeof(pthread_t));
    loadCapacity = (int*)        malloc (runways * sizeof(int));
    busy         = (int*)        malloc (runways * sizeof(int));

    for(int i=0; i<runways; i++){
        scanf("%d", &loadCapacity[i]);
        busy[i] = 0;
    }




    key_t key = ftok("progfile", 65);
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    msgid = msgget(key, IPC_CREAT | 0666); // Get the message queue ID
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    semATC = sem_open("ATC", 0);
    char semaphore_name[25];  
    sprintf(semaphore_name, "airport_semaphore_%d", airport_number); 
    sem_unlink(semaphore_name);
    sem_t *ss = sem_open(semaphore_name, O_CREAT | O_EXCL, 0666, 0);
    if (ss == SEM_FAILED) {
        perror("sem_open");
    }



    sem_wait(ss);
    while(1){

        if (msgrcv(msgid, &msgbuf, sizeof(struct Message), MESSAGE_TYPE, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        struct Message received_msg;
        msgbuf.mtype = MESSAGE_TYPE;
        memcpy(&received_msg, msgbuf.mtext, sizeof(struct Message));
        if(received_msg.sender == 2)
        {
            if(received_msg.TERMINATION == 1){
                break;
            }
            int use_backup_check = 1;
            for(int i=0; i<runways; i++){
                if(received_msg.r.total_weight <= loadCapacity[i]){
                    use_backup_check =0;
                }
            }

            if(received_msg.FOR_DEPARTURE == 1){
                if(use_backup_check){
                    pthread_join(threads[runways], NULL);
                    struct ThreadArgs threadArg = {runways, 1, received_msg.r};
                    pthread_create(&threads[runways], NULL, useRunways, (void *)&threadArg);
                }
                else{
                    int idx=-1;
                    while(idx == -1){
                        for(int i=0; i<runways; i++){
                            if(loadCapacity[i]>= received_msg.r.total_weight || !busy[i]){
                                idx = i;
                                break;
                            }
                        }
                    }
                    for(int i=0; i<runways; i++){
                        if(!busy[i] ){
                            if(loadCapacity[i] < loadCapacity[idx]){
                                if(loadCapacity[i] >= received_msg.r.total_weight){
                                    idx = i;
                                }
                            }
                        }
                    }
                    struct ThreadArgs threadArg = {idx, 1, received_msg.r};
                    pthread_create(&threads[idx], NULL, useRunways, (void *)&threadArg);
                }
            }
            else if(received_msg.FOR_DEPARTURE == 0){
                if(use_backup_check){
                    pthread_join(threads[runways], NULL);
                    struct ThreadArgs threadArg = {runways, 0, received_msg.r};
                    pthread_create(&threads[runways], NULL, useRunways, (void *)&threadArg);
                }
                else{
                    int idx=-1;
                    while(idx == -1){
                        for(int i=0; i<runways; i++){
                            if(loadCapacity[i]>= received_msg.r.total_weight || !busy[i]){
                                idx = i;
                                break;
                            }
                        }
                    }
                    for(int i=0; i<runways; i++){
                        if(!busy[i] ){
                            if(loadCapacity[i] < loadCapacity[idx]){
                                if(loadCapacity[i] >= received_msg.r.total_weight){
                                    idx = i;
                                }
                            }
                        }
                    }
                    struct ThreadArgs threadArg = {idx, 0, received_msg.r};
                    pthread_create(&threads[idx], NULL, useRunways, (void *)&threadArg);
                }
            }
            else{
                printf("404_2\n");
            }
        }
        else{
            printf("404\n");
        }
        sem_wait(ss);
    }


    free(threads);
    free(loadCapacity);
    free(busy);

    return 0;
}