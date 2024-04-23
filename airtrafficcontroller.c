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
    // Get the message queue ID
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    sem_unlink("ATC");
    sem_t *sem = sem_open("ATC", O_CREAT | O_EXCL, 0666, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    int terminated =0;
    while (1){
        sem_wait(sem);
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
            
            if(terminated){
                int conformation = 0;
                //not: ending sender id as it is obviour ATC is sending the conformation
                sem_post(plane_semaphore);
                if (msgsnd(msgid, &conformation, sizeof(int), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
            }
            else{

            }
        }
        else if (sender == 3){

        }
        else{
            //sender == 4
            terminated =1;
        }
    
    }
    return 0;
}