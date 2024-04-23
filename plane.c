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
const int message_sender_id = 1;


struct Plane{
    int arrival_ariport, departure_airport;
    int plane_id, total_weight;
    int plane_type, passengers; 
};

struct Plane plane_data;

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

struct Message message_to_ATC(){
    struct Message currentMessage;
    currentMessage.sender = 1;
    currentMessage.r = plane_data;
    return currentMessage;
}







int main(){



 
    int plane_id, plane_type, passengers=0;
    int total_weight=0;
    int num_cargo=0, avg_cargo_weight=0;
    int luggage_weight[20]={0};
    int passenger_weight[20]={0};
    int departure_airport, arrival_ariport;

    int fd[20][2]; //pipe


    printf("Enter Plane ID: ");
    fflush(stdout);
    scanf("%d", &plane_id);
    printf("\n");
    fflush(stdout);

    printf("Enter Type of Plane: ");
    fflush(stdout);
    scanf("%d", &plane_type);
    printf("\n");
    fflush(stdout);



    char semaphore_name[20];  
    sprintf(semaphore_name, "plane_semaphore_%d", plane_id); 
    sem_unlink(semaphore_name);
    sem_t *ss = sem_open(semaphore_name, O_CREAT | O_EXCL, 0666, 1);
    if (ss == SEM_FAILED) {
        perror("sem_open");
    }



    pid_t pid =-1;
    
    if(plane_type == 1){
        // passenger plane
        printf("Enter Number of Occupied Seats: ");
        fflush(stdout);
        scanf("%d", &passengers);
        printf("\n");
        fflush(stdout);
        int ID = passengers;
        for(int i=0; i<passengers; i++){
            if(pipe(fd[i]) == -1){
                printf("Error in making a pipe\n");
                fflush(stdout);
            }
        }
        for(int i=0; i<passengers; i++){
            pid = fork();
            if(pid == 0){
                close(fd[i][0]); //closeing the reading end for child
                ID = i;
                break;
            }
            else{
                close(fd[i][1]); //closeing the writing end for parent
            }
        }
        if(pid == 0){
            sem_wait(ss);
            int luggage_w, passenger_w;
            printf("Enter Weight of Your Luggage: ");
            fflush(stdout);
            scanf("%d", &luggage_w);
            printf("\n");
            printf("Enter Your Body Weight: ");
            fflush(stdout);
            scanf("%d", &passenger_w);
            printf("\n");
            fflush(stdout);

            write(fd[ID][1], &luggage_w, sizeof(int));
            write(fd[ID][1], &passenger_w, sizeof(int));
            sem_post(ss);

        }
        else{
            for(int i=0; i<passengers; i++){
                read(fd[i][0], &luggage_weight[i], sizeof(int));
                read(fd[i][0], &passenger_weight[i], sizeof(int));
                total_weight += luggage_weight[i] + passenger_weight[i];
            }
            for(int i=0; i<passengers; i++){
                printf("%d\n", luggage_weight[i]);
            }
            for(int i=0; i<passengers; i++){
                printf("%d\n", passenger_weight[i]);
            }
            while (wait(NULL) != -1){
                ;
            }
            total_weight += (75 *7);
        }
    }
    else{
        // cargo plane
        printf("Enter Number of Cargo Items: ");
        fflush(stdout);
        scanf("%d", &num_cargo);
        printf("\n");
        printf("Enter Average Weight of Cargo Items: ");
        fflush(stdout);
        scanf("%d", &avg_cargo_weight);
        printf("\n");
        fflush(stdout);

        total_weight = (avg_cargo_weight * num_cargo) + (75 * 2);
    }



    if(pid!=0){
        printf("Enter Airport Number for Departure: ");
        fflush(stdout);
        scanf("%d", &departure_airport);
        printf("\n");
        printf("Enter Airport Number for Arrival: ");
        fflush(stdout);
        scanf("%d", &arrival_ariport);
        printf("\n");
        fflush(stdout);


        struct Plane plane_data;
        plane_data.arrival_ariport   = arrival_ariport;
        plane_data.departure_airport = departure_airport;
        plane_data.plane_id          = plane_id;
        plane_data.total_weight      = total_weight;
        plane_data.plane_type        = plane_type;
        plane_data.passengers        = passengers;


        //send the plane_data to message queue
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

        sem_t *semATC = sem_open("ATC", 0);
        sem_post(semATC);
        if (msgsnd(msgid, &message_sender_id, sizeof(int), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        sem_post(semATC);
        if (msgsnd(msgid, &plane_data, sizeof(struct Plane), 0) == -1) {
            perror("msgsnd");
            exit(EXIT_FAILURE);
        }
        sem_wait(ss);
        int conformation;
        if (msgrcv(msgid, &conformation, sizeof(int), 0, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        if(conformation){
            printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n",
                             plane_id, departure_airport, arrival_ariport);
            fflush(stdout);
        }
        else{
            printf("Plain can not travel as cleanup happended already\n");
            fflush(stdout);
        }
    }

    return 0;
}