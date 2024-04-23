#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
//gcc -fno-stack-protector -o airtrafficcontroller airtrafficcontroller.c



/*
int message_sender:
1 = plane
2 = ATC
3 = airport
4 = clean up 
*/
int message_sender_id = 2;

struct Plane {
    int arrival_airport;
    int departure_airport;
    int plane_id;
    int total_weight;
    int plane_type;
    int passengers;
};


int main(){


    struct Plane r;
    key_t key = ftok("progfile", 65);
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if (msgrcv(msgid, &r, sizeof(struct Plane), 0, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }

    printf("%d", r.plane_id);
    return 0;
}