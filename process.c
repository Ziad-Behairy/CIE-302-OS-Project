#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define SLOT_SIZE 64

// Message structure
struct Message {
    long mtype;
    char data[SLOT_SIZE];
};

void formulateAndAddRequest(int upQueue, int time, char* operation, char* data);

int main() {
    key_t up_key_PK = ftok("processkernel", 'U');
    key_t down_key_PK = ftok("processkernel", 'D');
    int up_queue_PK = msgget(up_key_PK, IPC_CREAT | 0666);
    int down_queue_PK = msgget(down_key_PK, IPC_CREAT | 0666);

    FILE* file = fopen("test.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[100];
    while (fgets(line, sizeof(line), file) != NULL) {
        int time;
        char operation[5]; // "ADD" or "DEL"
        char data[SLOT_SIZE]; // Message content

        if (sscanf(line, "%d \"%[A-Z]\" \"%[^\"]\"", &time, operation, data) == 3) {
            formulateAndAddRequest(up_queue_PK, time, operation, data);
        } else {
            fprintf(stderr, "Error parsing line: %s\n", line);
        }
    }

    fclose(file);
    return 0;
}

void formulateAndAddRequest(int up_queue_PK, int time, char* operation, char* data) {
    struct Message request;
    // Assuming time is positive and can be used as mtype

    if (strcmp(operation, "ADD") == 0) 
    {
    snprintf(request.data, SLOT_SIZE, "A %s", data );
        request.mtype = 1; 
    } else if (strcmp(operation, "DEL") == 0) {
        snprintf(request.data, SLOT_SIZE, "D %s", data );
        request.mtype = 2; 
    } else {
        fprintf(stderr, "Invalid operation type: %s\n", operation);
        return;
    }

    if (msgsnd(up_queue_PK, &request, sizeof(request.data),!IPC_NOWAIT) == -1) {
        perror("Error adding request to UP Queue");
        exit(EXIT_FAILURE);
    }

    printf("Added request to UP Queue: %s\n", request.data);
}