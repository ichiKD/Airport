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
    char cc;
    while(cc != 'Y'){
        printf("Do you want the Air Traffic Control System to terminate?(Y for Yes and N for No) ");
        fflush(stdout);
        scanf("%c", &cc);
        printf("\n");
        fflush(stdout);
        if(cc != 'N' && cc != "Y"){
            printf("INVALID CHAR ENTERED\n");
        }
    }


    return 0;
}