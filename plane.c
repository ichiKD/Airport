#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>


struct Plane{
    int arrival_ariport, departure_airport;
    int plane_id, total_weight;
    int plane_type, passengers; 
};


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
    if(plane_type == 1){
        // passenger plane
        printf("Enter Number of Occupied Seats: ");
        fflush(stdout);
        scanf("%d", &passengers);
        printf("\n");
        fflush(stdout);
        for(int i=0; i<passengers; i++){
            if(pipe(fd[i]) == -1){
                printf("Error in making a pipe\n");
                fflush(stdout);
            }
        }

        pid_t pid;
        int ID = passengers;
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
            // I can add a mutex here for every child and a parent control to make it more sequential
            int luggage_w;
            printf("Enter Weight of Your Luggage: ");
            fflush(stdout);
            scanf("%d", &luggage_w);
            printf("\n");
            fflush(stdout);
            write(fd[ID][1], &luggage_w, sizeof(int));

            int passenger_w;
            printf("Enter Your Body Weight: ");
            fflush(stdout);
            scanf("%d", &passenger_w);
            printf("\n");
            fflush(stdout);
            write(fd[ID][1], &passenger_w, sizeof(int));



        }
        else{
            for(int i=0; i<passengers; i++){
                read(fd[i][0], &luggage_weight[i], sizeof(int));
                total_weight+=luggage_weight[i] + passenger_weight[i];
            }
            for(int i=0; i<passengers; i++){
                read(fd[i][0], &passenger_weight[i], sizeof(int));
            }
            for(int i=0; i<passengers; i++){
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
        fflush(stdout);
        printf("Enter Average Weight of Cargo Items: ");
        fflush(stdout);
        scanf("%d", &avg_cargo_weight);
        printf("\n");
        fflush(stdout);
        total_weight = (avg_cargo_weight * num_cargo) + (75 * 2);
    }

    printf("Enter Airport Number for Departure: ");
    fflush(stdout);
    scanf("%d", &departure_airport);
    printf("\n");
    fflush(stdout);

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


    return 0;
}