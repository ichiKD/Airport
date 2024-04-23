#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>


//gcc -o send send.c


// Define the structure for the plane
struct Plane {
    int arrival_airport;
    int departure_airport;
    int plane_id;
    int total_weight;
    int plane_type;
    int passengers;
};

int main() {
    struct Plane p = {1, 2, 3, 4, 5, 6};
    printf("Sending Plane Details:\n");
    printf("Arrival Airport: %d\n", p.arrival_airport);
    printf("Departure Airport: %d\n", p.departure_airport);
    printf("Plane ID: %d\n", p.plane_id);
    printf("Total Weight: %d\n", p.total_weight);
    printf("Plane Type: %d\n", p.plane_type);
    printf("Passengers: %d\n", p.passengers);

    // Generate a key for the message queue
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

    // Send the struct to message queue
    if (msgsnd(msgid, &p, sizeof(struct Plane), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    return 0;
}
