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
#include <errno.h>

#define MESSAGE_TYPE 1

/*
int message_sender:
1 = plane
2 = ATC
3 = airport
4 = clean up 
*/
int message_sender_id = 2;



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



struct Message reply_to_plane(int CONFORMATION_plane){
    struct Message currentMessage;
    currentMessage.sender = 2;
    currentMessage.TERMINATION = 0;
    currentMessage.CONFORMATION_plane = CONFORMATION_plane;
    return currentMessage;
}
struct Message departure(struct Plane r){
    struct Message currentMessage;
    currentMessage.sender = 2;
    currentMessage.TERMINATION = 0;
    currentMessage.FOR_DEPARTURE = 1;
    currentMessage.r = r;
    return currentMessage;
}

struct Message arrival(struct Plane r){
    struct Message currentMessage;
    currentMessage.sender = 2;
    currentMessage.TERMINATION = 0;
    currentMessage.FOR_DEPARTURE = 0;
    currentMessage.r = r;
    return currentMessage;
}

struct Message termination(){
    struct Message currentMessage;
    currentMessage.sender = 2;
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
    int airports;
    printf("Enter the number of airports to be handled/managed: ");
    fflush(stdout);
    scanf("%d", &airports);
    printf("\n");
    fflush(stdout);




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


    int terminated = 0;
    sem_wait(semATC);

    while (1){
        

        if (msgrcv(msgid, &msgbuf, sizeof(struct Message), MESSAGE_TYPE, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        struct Message received_msg;
        msgbuf.mtype = MESSAGE_TYPE;
        memcpy(&received_msg, msgbuf.mtext, sizeof(struct Message));


        
        if(received_msg.sender == 1)
        {
            char departure_semaphore_name[30];  
            sprintf(departure_semaphore_name, "airport_semaphore_%d", received_msg.r.departure_airport); 
            sem_t *departure_semaphore = sem_open(departure_semaphore_name, 0);
            struct Message msg = departure(received_msg.r);
            msgbuf.mtype = MESSAGE_TYPE;
            memcpy(msgbuf.mtext, &msg, sizeof(struct Message));    // Copy the struct Message into the message buffer
            sem_post(departure_semaphore);
            if (msgsnd(msgid, &msgbuf, sizeof(struct Message), IPC_NOWAIT) == -1) {
                perror("msgsnd");
                exit(1);
            }
        }
        else if(received_msg.sender == 3)
        {
            if(received_msg.FOR_DEPARTURE == 1){
                if(received_msg.DEPARTURE_case == 1){
                    ;
                }
                else if(received_msg.DEPARTURE_case == 2){
                    char arrival_semaphore_name[30];  
                    sprintf(arrival_semaphore_name, "airport_semaphore_%d", received_msg.r.arrival_airport); 
                    sem_t *arrival_semaphore = sem_open(arrival_semaphore_name, 0);
                    struct Message msg = arrival(received_msg.r);
                    msgbuf.mtype = MESSAGE_TYPE;
                    memcpy(msgbuf.mtext, &msg, sizeof(struct Message));    // Copy the struct Message into the message buffer
                    sem_post(arrival_semaphore);
                    if (msgsnd(msgid, &msgbuf, sizeof(struct Message), IPC_NOWAIT) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }
                }
                else{
                    printf("404_2\n");
                }
            }
            else if(received_msg.FOR_DEPARTURE == 0){
                if(received_msg.ARRIVAL_case == 1){
                    ;
                }
                else if (received_msg.ARRIVAL_case == 2){
                    struct Message msg = reply_to_plane(1);
                    char plane_semaphore_name[30];  
                    sprintf(plane_semaphore_name, "plane_semaphore_%d", received_msg.r.plane_id);
                    sem_t *plane_semaphore = sem_open(plane_semaphore_name, 0);
                    sem_post(plane_semaphore);
                    msgbuf.mtype = MESSAGE_TYPE;
                    memcpy(msgbuf.mtext, &msg, sizeof(struct Message));    // Copy the struct Message into the message buffer
                    if (msgsnd(msgid, &msgbuf, sizeof(struct Message), IPC_NOWAIT) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }
                    FILE *fp;
                    char filename[] = "AirTrafficController.txt";
                    char text_to_append[80] ;
                    sprintf(text_to_append, "Plane %d has departed from Airport %d and will land at Airport %d.", 
                                    received_msg.r.plane_id, received_msg.r.departure_airport, received_msg.r.arrival_airport); 
                    fp = fopen(filename, "a");
                    if (fp == NULL) {
                        fp = fopen(filename, "w");
                        if (fp == NULL) {
                            perror("Error creating file");
                            return EXIT_FAILURE;
                        }
                    }
                    fprintf(fp, "%s", text_to_append);
                    fclose(fp);

                }
                else{
                    printf("404_3\n");
                }
            }
            else{
                printf("404_4\n");
            }

        }
        else if(received_msg.sender == 4)
        {
            for(int i=1; i<= airports + 10; i++){
                char airport_name[30];  
                sprintf(airport_name, "airport_semaphore_%d", i);
                sem_t *airport_semaphore = sem_open(airport_name, 0);
                if (airport_semaphore == SEM_FAILED) {
                    ;
                }
                else{
                    sem_post(airport_semaphore);
                    sem_post(airport_semaphore);
                    struct Message msg = termination();
                    msgbuf.mtype = MESSAGE_TYPE;
                    memcpy(msgbuf.mtext, &msg, sizeof(struct Message));    // Copy the struct Message into the message buffer
                    if (msgsnd(msgid, &msgbuf, sizeof(struct Message), IPC_NOWAIT) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }
                }
            }
            for(int i=1; i<=  20; i++){
                char plane_semaphore_name[30];  
                sprintf(plane_semaphore_name, "plane_semaphore_%d", i);
                sem_t *plane_semaphore = sem_open(plane_semaphore_name, 0);
                if (plane_semaphore == SEM_FAILED) {
                    ;
                }
                else{
                    sem_post(plane_semaphore);
                    sem_post(plane_semaphore);
                    struct Message msg = reply_to_plane(0);
                    msgbuf.mtype = MESSAGE_TYPE;
                    memcpy(msgbuf.mtext, &msg, sizeof(struct Message));    // Copy the struct Message into the message buffer
                    if (msgsnd(msgid, &msgbuf, sizeof(struct Message), IPC_NOWAIT) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }
                }
            }
            break;
        }
        else{
            printf("404\n");
        }



        sem_wait(semATC);
    }
    return 0;
}