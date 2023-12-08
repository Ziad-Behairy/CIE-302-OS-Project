#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>

#define SLOT_SIZE 64

// Message structure
struct Message {
    long mtype;
    char data[SLOT_SIZE];
};

// Function to formulate and add requests to the UP Queue
void formulateAndAddRequest(int upQueue, int time, char* operation, char* data);

int main() {
    // Open the file containing I/O requests

    //******************** process kernel ****************************//
    key_t up_key_PK = ftok("processkernel", 'U');
    key_t down_key_PK = ftok("processkernel", 'D');
    int up_queue_PK = msgget(up_key_PK, IPC_CREAT | 0666);
    int down_queue_PK = msgget(down_key_PK, IPC_CREAT | 0666);
    //********************************************************//
    FILE* file = fopen("io_requests.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Create or connect to the UP Queue
    // int upQueue = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    // if (upQueue == -1) {
    //     perror("Error creating or connecting to the UP Queue");
    //     exit(EXIT_FAILURE);
    // }

    // Read each line from the file and formulate requests
    char line[100]; // Assuming a maximum line length of 100 characters
    while (fgets(line, sizeof(line), file) != NULL) {
        // Parse the line to extract Time, Operation, and Data
        int time;
        char operation[5]; // Assuming "Add" or "Del" and null terminator
        char data[SLOT_SIZE]; // Assuming data has SLOT_SIZE characters or less
        sscanf(line, "from %d %s %s tp", &time, operation, data);

        // Formulate and add requests to the UP Queue
        formulateAndAddRequest(up_queue_PK, time, operation, data);
    }

    // Close the file
    fclose(file);

    return 0;
}

void formulateAndAddRequest(int up_queue_PK, int time, char* operation, char* data) {
    struct Message request;

    // Formulate the request based on the operation type
    if (strcmp(operation, "ADD") == 0) {
        // If operation is "Add", add "A" at the beginning of the message
        sprintf(request.data, "A from %d ADD %s tp A %s", time, data, data);
    } else if (strcmp(operation, "DEL") == 0) {
        // If operation is "Del", add "D" at the beginning of the message
        sprintf(request.data, "D from %d DEL %s tp D %s", time, data, data);
    } else {
        fprintf(stderr, "Invalid operation type: %s\n", operation);
        return;
    }
    // Set the message type based on time
    request.mtype = time;

    // Add the formulated request to the UP Queue
    if (msgsnd(up_queue_PK, &request, sizeof(struct Message) - sizeof(long), 0) == -1) {
        perror("Error adding request to UP Queue");
        exit(EXIT_FAILURE);
    }

    // Optionally, print a message indicating the added request
    printf("Added request to UP Queue: %s\n", request.data);
}