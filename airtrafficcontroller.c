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
    currentMessage.CONFORMATION_plane = CONFORMATION_plane;
    return currentMessage;
}
struct Message departure(struct Plane r){
    struct Message currentMessage;
    currentMessage.sender = 2;
    currentMessage.FOR_DEPARTURE = 1;
    currentMessage.r = r;
    return currentMessage;
}

struct Message arrival(struct Plane r){
    struct Message currentMessage;
    currentMessage.sender = 2;
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
        
        int sender;
        if (msgrcv(msgid, &sender, sizeof(int), 0, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        sem_wait(semATC);
        if(sender == 1){
            struct Plane r;
            if (msgrcv(msgid, &r, sizeof(struct Plane), 0, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }

            char plane_semaphore_name[20];  
            sprintf(plane_semaphore_name, "plane_semaphore_%d", r.plane_id); 
            sem_t *plane_semaphore = sem_open(plane_semaphore_name, 0);

            char arrival_semaphore_name[25];  
            sprintf(arrival_semaphore_name, "airport_semaphore_%d", r.arrival_airport); 
            sem_t *arrival_semaphore = sem_open(arrival_semaphore_name, 0);

            char departure_semaphore_name[25];  
            sprintf(departure_semaphore_name, "airport_semaphore_%d", r.departure_airport); 
            sem_t *departure_semaphore = sem_open(departure_semaphore_name, 0);

            if(terminated){
                int conformation = 0;
                //NOTE: not ending sender id as it is obviour ATC is sending the conformation
                sem_post(plane_semaphore);
                if (msgsnd(msgid, &conformation, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
                
            }
            else{
                sem_post(departure_semaphore);
                int FOR_DEPARTURE =1;
                terminated = 0;
                if (msgsnd(msgid, &terminated, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
                sem_post(departure_semaphore);
                if (msgsnd(msgid, &FOR_DEPARTURE, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
                sem_post(departure_semaphore);
                if (msgsnd(msgid, &r, sizeof(struct Plane), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else if (sender == 3){
            int departure =-1; // 0 = arrival, 1 = departure
            if (msgrcv(msgid, &departure, sizeof(int), 0, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
            sem_wait(semATC);
            struct Plane r;
            if (msgrcv(msgid, &r, sizeof(struct Plane), 0, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
            sem_wait(semATC);
            if(departure){
                int CONFORMATION_boarding, CONFORMATION_takeoff;
                if (msgrcv(msgid, &CONFORMATION_boarding, sizeof(int), 0, 0) == -1) {
                    perror("msgrcv");
                    exit(EXIT_FAILURE);
                }
                sem_wait(semATC);
                if (msgrcv(msgid, &CONFORMATION_takeoff, sizeof(int), 0, 0) == -1) {
                    perror("msgrcv");
                    exit(EXIT_FAILURE);
                }
                sem_wait(semATC);
                char arrival_semaphore_name[25];  
                sprintf(arrival_semaphore_name, "airport_semaphore_%d", r.arrival_airport); 
                sem_t *arrival_semaphore = sem_open(arrival_semaphore_name, 0);
                int FOR_DEPARTURE = 0;
                sem_post(arrival_semaphore);
                if (msgsnd(msgid, &FOR_DEPARTURE, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
                if (msgsnd(msgid, &r, sizeof(struct Plane), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
            }
            else{
                char plane_semaphore_name[20];  
                sprintf(plane_semaphore_name, "plane_semaphore_%d", r.plane_id); 
                sem_t *plane_semaphore = sem_open(plane_semaphore_name, 0);
                int CONFORMATION_landing, CONFORMATION_deboarded;
                if (msgrcv(msgid, &CONFORMATION_landing, sizeof(int), 0, 0) == -1) {
                    perror("msgrcv");
                    exit(EXIT_FAILURE);
                }
                sem_wait(semATC);
                if (msgrcv(msgid, &CONFORMATION_deboarded, sizeof(int), 0, 0) == -1) {
                    perror("msgrcv");
                    exit(EXIT_FAILURE);
                }
                int plane_conformation = 1;
                //NOTE: not ending sender id as it is obviour ATC is sending the conformation
                sem_post(plane_semaphore);
                if (msgsnd(msgid, &plane_conformation, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
                FILE *fp;
                char filename[] = "AirTrafficController.txt";
                char text_to_append[60] ;
                sprintf(text_to_append, "Plane %d has departed from Airport %d and will land at Airport %d.", 
                                r.plane_id, r.departure_airport, r.arrival_airport); 
                fp = fopen(filename, "a");
                if (fp == NULL) {
                    fp = fopen(filename, "w");
                    if (fp == NULL) {
                        perror("Error creating file");
                        return EXIT_FAILURE;
                    }
                    printf("File created: %s\n", filename);
                }
                fprintf(fp, "%s", text_to_append);
                fclose(fp);
            }
        }
        else{
            //sender == 4
            terminated = 1;
            for(int i=1; i<airports+1; i++){
                char airport_name[25];  
                sprintf(airport_name, "airport_semaphore_%d", i);
                sem_t *airport_semaphore = sem_open(airport_name, 0);
                sem_post(airport_semaphore);
                if (msgsnd(msgid, &terminated, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
            }
            for(int i=1; i<11; i++){
                char plane_semaphore_name[20];  
                sprintf(plane_semaphore_name, "plane_semaphore_%d", i); 
                sem_t *plane_semaphore = sem_open(plane_semaphore_name, 0);
                sem_post(plane_semaphore);
                int conformation =0;
                if (msgsnd(msgid, &conformation, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
            }
            break;
        }
        sem_wait(semATC);
    }
    return 0;
}