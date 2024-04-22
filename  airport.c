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


    free(loadCapacity);
    return 0;
}