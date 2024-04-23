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
int message_sender_id = 3;
sem_t *semATC;
int msgid;

int airport_number, runways;
pthread_t *threads = NULL;
int *loadCapacity  = NULL;
int *busy          = NULL;
int back_up_BUSY   = 0;


struct Plane{
    int arrival_ariport, departure_airport;
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
struct Message takeoff(struct Plane r){
    struct Message currentMessage;
    currentMessage.sender = 3;
    currentMessage.FOR_DEPARTURE = 0;
    currentMessage.ARRIVAL_case = 2;
    currentMessage.CONFORMATION_deboarded = 1;
    currentMessage.r = r;
    return currentMessage;
}




void * useRunways(void *args){
    struct ThreadArgs *threadArg = (struct ThreadArgs *) args;
    int idx                      = threadArg->idx;
    int FOR_DEPARTURE            = threadArg->FOR_DEPARTURE;
    struct Plane r               = threadArg->plane;

    if(idx == runways){
        //backup runway
        back_up_BUSY = 1;
        if(FOR_DEPARTURE == 1){
            sleep(3);
            printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n",
                    r.plane_id, idx + 1, airport_number);
            sem_post(semATC);
            if (msgsnd(msgid, &message_sender_id, sizeof(int), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            sem_post(semATC);
            if (msgsnd(msgid, &FOR_DEPARTURE, sizeof(int), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            sem_post(semATC);
            if (msgsnd(msgid, &r, sizeof(struct Plane ), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            sem_post(semATC);
            int CONFORMATION_boarding = 1;
            if (msgsnd(msgid, &CONFORMATION_boarding, sizeof(int), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            sem_post(semATC);
            // SEND CONFORMATION TO ATC about boarding
            sleep(2);
            int CONFORMATION_takeoff= 1;
            if (msgsnd(msgid, &CONFORMATION_takeoff, sizeof(int), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
        }
        else if(FOR_DEPARTURE == 0){
            sleep(2);
            sem_post(semATC);
            if (msgsnd(msgid, &message_sender_id, sizeof(int), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            sem_post(semATC);
            if (msgsnd(msgid, &FOR_DEPARTURE, sizeof(int), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            sem_post(semATC);
            if (msgsnd(msgid, &r, sizeof(struct Plane ), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            sem_post(semATC);
            // SEND CONFORMATION TO ATC about landing
            int CONFORMATION_landing = 1;
            if (msgsnd(msgid, &CONFORMATION_landing, sizeof(int), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
            sem_post(semATC);
            sleep(3);
            printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n",
                    r.plane_id, idx + 1, airport_number);
            // SEND CONFORMATION TO ATC about deboarded
            int CONFORMATION_deboarded= 1;
            if (msgsnd(msgid, &CONFORMATION_deboarded, sizeof(int), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }  
        }
        back_up_BUSY = 0;
        pthread_exit(NULL);
        return (void *)NULL;
    }
    busy[idx] = 1;
    if(FOR_DEPARTURE == 1){
        sleep(3);
        printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d.\n",
                r.plane_id, idx + 1, airport_number);
        sem_post(semATC);
        if (msgsnd(msgid, &message_sender_id, sizeof(int), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        sem_post(semATC);
        if (msgsnd(msgid, &FOR_DEPARTURE, sizeof(int), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        sem_post(semATC);
        if (msgsnd(msgid, &r, sizeof(struct Plane ), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        sem_post(semATC);
        int CONFORMATION_boarding = 1;
        if (msgsnd(msgid, &CONFORMATION_boarding, sizeof(int), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        sem_post(semATC);
        // SEND CONFORMATION TO ATC about boarding
        sleep(2);
        int CONFORMATION_takeoff= 1;
        if (msgsnd(msgid, &CONFORMATION_takeoff, sizeof(int), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
    }
    else if(FOR_DEPARTURE == 0){
        sleep(2);
        sem_post(semATC);
        if (msgsnd(msgid, &message_sender_id, sizeof(int), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        sem_post(semATC);
        if (msgsnd(msgid, &FOR_DEPARTURE, sizeof(int), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        sem_post(semATC);
        if (msgsnd(msgid, &r, sizeof(struct Plane ), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        sem_post(semATC);
        // SEND CONFORMATION TO ATC about landing
        int CONFORMATION_landing = 1;
        if (msgsnd(msgid, &CONFORMATION_landing, sizeof(int), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        sem_post(semATC);
        sleep(3);
        printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n",
                r.plane_id, idx + 1, airport_number);
        // SEND CONFORMATION TO ATC about deboarded
        int CONFORMATION_deboarded= 1;
        if (msgsnd(msgid, &CONFORMATION_deboarded, sizeof(int), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }        
    }
    busy[idx] = 0;
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
    sem_t *ss = sem_open(semaphore_name, O_CREAT | O_EXCL, 0666, 1);
    if (ss == SEM_FAILED) {
        perror("sem_open");
    }
    sem_wait(ss);
    int terminated;
    while(1){
        terminated = 0;
        int FOR_DEPARTURE = -1;
        int flag =0;
        struct Plane r;
        if (msgrcv(msgid, &terminated, sizeof(int), 0, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        if(terminated == 0){
            sem_wait(ss);
            if (msgrcv(msgid, &FOR_DEPARTURE, sizeof(int), 0, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
            sem_wait(ss);
            if (msgrcv(msgid, &r, sizeof(struct Plane), 0, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }

            int use_backup_check = 1;
            for(int i=0; i<runways; i++){
                if(r.total_weight <= loadCapacity[i]){
                    use_backup_check =0;
                }
            }
            if(FOR_DEPARTURE == 1){
                if(use_backup_check){
                    pthread_join(threads[runways], NULL);
                    struct ThreadArgs threadArg = {runways, 1, r};
                    pthread_create(&threads[runways], NULL, useRunways, (void *)&threadArg);
                }
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
                    for(int i=0; i<runways; i++){
                        if(!busy[i] ){
                            if(loadCapacity[i] < loadCapacity[idx]){
                                if(loadCapacity[i] >= r.total_weight){
                                    idx = i;
                                }
                            }
                        }
                    }
                    struct ThreadArgs threadArg = {idx, 1, r};
                    pthread_create(&threads[idx], NULL, useRunways, (void *)&threadArg);
                }
            }
            else if(FOR_DEPARTURE == 0){
                if(use_backup_check){
                    pthread_join(threads[runways], NULL);
                    struct ThreadArgs threadArg = {runways, 0, r};
                    pthread_create(&threads[runways], NULL, useRunways, (void *)&threadArg);
                }
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
                    for(int i=0; i<runways; i++){
                        if(!busy[i] ){
                            if(loadCapacity[i] < loadCapacity[idx]){
                                if(loadCapacity[i] >= r.total_weight){
                                    idx = i;
                                }
                            }
                        }
                    }
                    struct ThreadArgs threadArg = {idx, 0, r};
                    pthread_create(&threads[idx], NULL, useRunways, (void *)&threadArg);
                }
            }
            else{
                printf("404_2\n");
                fflush(stdout);
            }
        }
        else{
            flag =1;
            int check =0;
            for(int i=0; i<runways; i++){
                check+=busy[i];
            }
            check +=back_up_BUSY;
            if(check == 0){
                break;
            }
            else{
                continue;
            }
        }
        if(flag){
             flag =1;
            int check =0;
            for(int i=0; i<runways; i++){
                check+=busy[i];
            }
            check +=back_up_BUSY;
            if(check == 0){
                break;
            }
            else{
                continue;
            }
        }
        sem_wait(ss);
    }


    free(threads);
    free(loadCapacity);
    free(busy);

    return 0;
}