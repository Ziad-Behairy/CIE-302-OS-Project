#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdbool.h>  // Added for bool type
#include <pthread.h>

int msg_up, msg_down, msg_up_PK, msg_down_PK;
int clk = 0;
pid_t disk_id;

struct msg_buffer {
    long msg_type;
    char msg_text[64];
};

struct pid_msg {
    long msg_type;
    pid_t disk_pid;
};

typedef struct {
    long msg_type;
    bool status;
} status_msg;

void requestDiskStatus() {
    // Send SIGUSR1 signal to disk process to request status
    kill(disk_id, SIGUSR1);
}

// Signal handler for updating the clock
void sig_handler(int signum) {
    clk++;  // Increment the clock
}

void handleAddRequest(struct msg_buffer* request, FILE* log_file) {
    struct msg_buffer availableslots;
    requestDiskStatus();

    // Receive available slots message from disk process
    int a = msgrcv(msg_up, &availableslots, sizeof(availableslots.msg_text), 42, 0);
    if (a == -1) {
        perror("Error receiving Disk status");
        exit(EXIT_FAILURE);
    }

    if (availableslots.msg_text[0] != '0') {  // Check if there are available slots
        // Send the data to the disk to be added
        msgsnd(msg_up, request, sizeof(request->msg_text), 0);
        // Receive message from disk process indicating the result of addition
        struct msg_buffer DiskAdd; // received ADD from disk
         if (msgrcv(msg_up, &DiskAdd, sizeof(DiskAdd.msg_text), 0, 0) == -1) 
        {
            perror("msgrcv");
            exit(1);
        }

        // Send a message to the process indicating successful or unable to ADD
 
            msgsnd(msg_down_PK, &DiskAdd, sizeof(response.msg_text), 0);
        
        //open log file
        // Open the log file
        FILE* log_file = fopen("log.txt", "a");
        if (log_file == NULL) {
            perror("Error opening log file");
            exit(EXIT_FAILURE);
        }
        // Log the operation in the log file
        fprintf(log_file, "Time: %d, Action: %s, Data: %s\n", clk, DiskAdd.msg_text ,request->msg_text);
    } 
    else {
        // Send a message to the process indicating that the request can't be handled
        struct msg_buffer response;
        response.msg_type = 0;
        strcpy(response.msg_text, "The request can't be handled.");
        msgsnd(msg_down_PK, &response, sizeof(response.msg_text), 0);
    }
}

void handleDelRequest(struct msg_buffer* request, FILE* log_file) {
    // Send a message to the Disk Process indicating data deletion
    struct msg_buffer msg_send;
    msg_send.msg_type = 2;  

    // Send the message to the Disk Process (down_queue)
    if (msgsnd(msg_down, &msg_send, sizeof(msg_send.msg_text), 0) == -1) {
        perror("Error sending message to Disk Process");
    }

    // Receive message from disk process indicating the result of deletion
    struct msg_buffer DiskDelete;
    int a = msgrcv(msg_up, &DiskDelete, sizeof(DiskDelete.msg_text), 0, 0);
    if (a == -1) {
        perror("Error receiving Disk status");
        exit(EXIT_FAILURE);
    }

    //send message to the process to indicate the result of deletion (msg_down_PK)
   
     msgsnd(msg_down_PK, &DiskDelete, sizeof(response.msg_text), 0);

        // Log the operation delete  in the log file
        fprintf(log_file, "Time: %d, Action: %s, Data: %s\n", clk, DiskDelete.msg_text,request->msg_text);
  
}

// Thread function for sending SIGUSR2 every second
void* periodic_signal_sender(void* arg) {
    while (1) {
        sleep(1);
        kill(getpid(), SIGUSR2);
    }
    return NULL;
}

int main() {
    // Signal setup for clock updates
    signal(SIGUSR2, sig_handler);

    //******************** Disk Kernel Message Queues ****************************//
    key_t up_key = ftok("Diskkernel", 'U');
    key_t down_key = ftok("Diskkernel", 'D');
    int up_queue = msgget(up_key, IPC_CREAT | 0666);
    int down_queue = msgget(down_key, IPC_CREAT | 0666);
    msg_up = up_queue;
    msg_down = down_queue;

    //******************** Process Kernel Message Queues ****************************//
    key_t up_key_PK = ftok("processkernel", 'U');
    key_t down_key_PK = ftok("processkernel", 'D');
    int up_queue_PK = msgget(up_key_PK, IPC_CREAT | 0666);
    int down_queue_PK = msgget(down_key_PK, IPC_CREAT | 0666);
    msg_up_PK = up_queue_PK;
    msg_down_PK = down_queue_PK;

    if (up_queue == -1 || down_queue == -1 || up_queue_PK == -1 || down_queue_PK == -1) {
        perror("Error in creating message queues");
        exit(EXIT_FAILURE);
    }

    // Compile the disk process
    char command[256];
    snprintf(command, sizeof(command), "gcc diskprocess.c -o diskprocess");

    printf("Executing command: %s\n", command);
    int systemResult = system(command);
    printf("System result: %d\n", systemResult);

    if (systemResult == -1) {
        perror("Error executing Disk Process");
        exit(EXIT_FAILURE);
    }

    // Receive the Disk PID from the Disk Kernel
    struct pid_msg received_msg;
    int b = msgrcv(down_queue, &received_msg, sizeof(received_msg.disk_pid), 4, 0);
    if (b == -1) {
        perror("Error receiving Disk PID");
        exit(EXIT_FAILURE);
    }
    disk_id = received_msg.disk_pid;

    // Rest of your kernel process code...
    printf("Kernel Process: Received Disk PID: %d\n", disk_id);

    while (1)
    {
        // Receive message from process
        struct msg_buffer received_msg; // received message from process
        if (msgrcv(up_queue_PK, &received_msg, sizeof(received_msg.msg_text), 0, 0) == -1) 
        {
            perror("Error receiving message from process");
            exit(1);
        }
        // Check the operation type
        if (received_msg.msg_text[0] == 'A') {  // ADD
            handleAddRequest(&received_msg, log_file);
        } else if (received_msg.msg_text[0] == 'D') {  // DEL
            handleDelRequest(&received_msg, log_file);
        } else {
            // Send a message to the process indicating that the request can't be handled
            struct msg_buffer response;
            response.msg_type = 2;
            strcpy(response.msg_text, "The request can't be handled.");
            msgsnd(msg_down_PK, &response, sizeof(response.msg_text), 0);
        }

        // Close the log file
        fclose(log_file);
    }
    

    return 0;
}
