#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>


//gcc -fno-stack-protector -o receive receive.c

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
    struct Plane r;

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

    // Receive the struct from message queue
    if (msgrcv(msgid, &r, sizeof(struct Plane), 0, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    // Print the received data
    printf("Received Plane Details:\n");
    printf("Arrival Airport: %d\n", r.arrival_airport);
    printf("Departure Airport: %d\n", r.departure_airport);
    printf("Plane ID: %d\n", r.plane_id);
    printf("Total Weight: %d\n", r.total_weight);
    printf("Plane Type: %d\n", r.plane_type);
    printf("Passengers: %d\n", r.passengers);

    // Destroy the message queue
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}
