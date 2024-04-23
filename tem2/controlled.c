#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>     // For O_CREAT, O_EXCL, O_RDWR
#include <sys/stat.h>  // For mode constants
// gcc -o controlled controlled.c -lrt
#define SEM_NAME "/my_named_semaphore"

int main() {
    sem_t *sem;

    // Open the named semaphore
    sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    printf("Controlled Process: Signaling the controller...\n");

    // Signal the controller process
    sem_post(sem);

    // Clean up: close the semaphore
    sem_close(sem);

    printf("Controlled Process: Semaphore closed\n");

    return EXIT_SUCCESS;
}
