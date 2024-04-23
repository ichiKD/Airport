#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>     // For O_CREAT, O_EXCL, O_RDWR
#include <sys/stat.h>  // For mode constants
// gcc -o controller controller.c -lrt
#define SEM_NAME "/my_named_semaphore"
#define SEM_INITIAL_VALUE 1  // Initial value of the semaphore

int main() {
    sem_t *sem;

    // Create or open the named semaphore
    sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    printf("Controller: Semaphore created\n");

    // Wait for the controlled process to signal
    printf("Controller: Waiting for the controlled process...\n");
    sem_wait(sem);

    printf("Controller: Controlled process has signaled\n");

    // Clean up: close and unlink the semaphore
    sem_close(sem);
    sem_unlink(SEM_NAME);

    printf("Controller: Semaphore closed and unlinked\n");

    return EXIT_SUCCESS;
}
