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


int main(){
    int plane_id, plane_type, passengers;
    int luggage_weight[20]={0};
    int passenger_weight[20]={0};
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
            }
            for(int i=0; i<passengers; i++){
                read(fd[i][0], &passenger_weight[i], sizeof(int));
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
        }
    }
    else{
        // cargo plane
    }




    return 0;
}