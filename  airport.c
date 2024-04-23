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
const int message_sender_id = 3;


int airport_number, runways;
int *loadCapacity = NULL;
int *busy = NULL;
int back_up_BUSY = 0;


struct Plane{
    int arrival_ariport, departure_airport;
    int plane_id, total_weight;
    int plane_type, passengers; 
};




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

    loadCapacity = (int*) malloc (runways * sizeof(int));
    busy         = (int*) malloc (runways * sizeof(int));

    for(int i=0; i<runways; i++){
        scanf("%d", &loadCapacity[i]);
        busy[i] = 0;
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
    char semaphore_name[25];  
    sprintf(semaphore_name, "airport_semaphore_%d", airport_number); 
    sem_unlink(semaphore_name);
    sem_t *ss = sem_open(semaphore_name, O_CREAT | O_EXCL, 0666, 1);
    if (ss == SEM_FAILED) {
        perror("sem_open");
    }
    sem_wait(ss);
    while(1){
        int sender, FOR_DEPARTURE = -1;
        struct Plane r;
        if (msgsnd(msgid, &sender, sizeof(int), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        if(sender == 2){
            if (msgsnd(msgid, &FOR_DEPARTURE, sizeof(int), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            if (msgsnd(msgid, &r, sizeof(struct Plane), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            int use_backup_check = 1;
            for(int i=0; i<runways; i++){
                if(r.total_weight <= loadCapacity[i]){
                    use_backup_check =0;
                }
            }
            if(FOR_DEPARTURE == 1){
                if(use_backup_check){}
                else{
                    int idx=-1;
                    while(idx == -1){
                        for(int i=0; i<runways; i++){
                            if(loadCapacity[i]>= r.total_weight || !busy[i]){
                                idx = i;
                                break;
                            }
                        }
                    }
                    
                }
            }
            else if(FOR_DEPARTURE == 0){
                if(use_backup_check){}
                else{

                }
            }
            else{
                printf("404_2\n");
                fflush(stdout);
            }
        }
        else{
            printf("404\n");
            fflush(stdout);
        }
        sem_wait(ss);
    }






    free(loadCapacity);
    free(busy);

    return 0;
}