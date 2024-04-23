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
const int message_sender_id = 2;


struct Plane{
    int arrival_ariport, departure_airport;
    int plane_id, total_weight;
    int plane_type, passengers; 
};


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
            sprintf(arrival_semaphore_name, "airport_semaphore_%d", r.arrival_ariport); 
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
                if (msgsnd(msgid, &message_sender_id, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
                if (msgsnd(msgid, &FOR_DEPARTURE, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
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
            if(departure){
                struct Plane r;
                if (msgrcv(msgid, &r, sizeof(struct Plane), 0, 0) == -1) {
                    perror("msgrcv");
                    exit(EXIT_FAILURE);
                }
                char arrival_semaphore_name[25];  
                sprintf(arrival_semaphore_name, "airport_semaphore_%d", r.arrival_ariport); 
                sem_t *arrival_semaphore = sem_open(arrival_semaphore_name, 0);
                int FOR_DEPARTURE = 0;
                sem_post(arrival_semaphore);
                if (msgsnd(msgid, &message_sender_id, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
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
                struct Plane r;
                if (msgrcv(msgid, &r, sizeof(struct Plane), 0, 0) == -1) {
                    perror("msgrcv");
                    exit(EXIT_FAILURE);
                }
                char plane_semaphore_name[20];  
                sprintf(plane_semaphore_name, "plane_semaphore_%d", r.plane_id); 
                sem_t *plane_semaphore = sem_open(plane_semaphore_name, 0);
                sem_post(plane_semaphore);
                int conformation = 1;
                //NOTE: not ending sender id as it is obviour ATC is sending the conformation
                sem_post(plane_semaphore);
                if (msgsnd(msgid, &conformation, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }

                //
                FILE *fp;
                char filename[] = "output.txt";
                char text_to_append[] = "This is new data to append.\n";
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
            terminated =1;
        }
        sem_wait(semATC);
    }
    return 0;
}