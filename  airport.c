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

struct Plane{
    int arrival_ariport, departure_airport;
    int plane_id, total_weight;
    int plane_type, passengers; 
};




int main(){
    int airport_number, runways;
    int *loadCapacity = NULL;
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

    loadCapacity = (int*) malloc (runways *sizeof(int));
    for(int i=0; i<runways; i++){
        scanf("%d", &loadCapacity[i]);
    }


    char semaphore_name[25];  
    sprintf(semaphore_name, "airport_semaphore_%d", airport_number); 
    sem_unlink(semaphore_name);
    sem_t *ss = sem_open(semaphore_name, O_CREAT | O_EXCL, 0666, 1);
    if (ss == SEM_FAILED) {
        perror("sem_open");
    }






    free(loadCapacity);

    return 0;
}