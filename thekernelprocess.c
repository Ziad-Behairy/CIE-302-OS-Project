#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdbool.h>

 

int msg_up, msg_down, msg_up_PK, msg_down_PK;
int clk;
pid_t disk_id;
pid_t processid;
FILE* log_file;

struct msg_buffer {
    long msg_type;
    char msg_text[64];
};

struct pid_msgdisk {
    long msg_type;
    pid_t disk_pid;
};
struct pid_msgprocess {
    long msg_type;
    pid_t process_pid;
};
typedef struct {
    long msg_type;
    bool status;
} status_msg;
struct ClkMessage {
    long msg_type;
    int clk;
};
void requestDiskStatus() {
    // Send SIGUSR1 signal to disk process to request status
    printf("before kill siguser1\n");
    kill(disk_id, SIGUSR1);
    printf("after kill siguser1");

}

// Signal handler for updating the clock
void sig_handler(int signum) {
    clk++;  // Increment the clock
}

void handleAddRequest(struct msg_buffer* request, FILE* log_file) {
       
     fprintf(log_file, "At time = %d, request to add \"%s\" from P1\n", clk,request->msg_text); // Log the operation request in the log file
    struct msg_buffer availableslots;
    requestDiskStatus();             // dont forget to remove the comment******awad
  
    fprintf(log_file, "At time = %d, sent status request to Disk\n", clk); //log the sending status of the request to the disk
  // Receive available slots message from disk process
    int a = msgrcv(msg_up, &availableslots, sizeof(availableslots.msg_text), 42, 0);
    if (a == -1) {
        perror("Error receiving Disk status");
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "At time = %d,  Disk status =  %s empty slots\n", clk, availableslots.msg_text); //log the number of available slots in the log file

    if (availableslots.msg_text[0] != '0') {  // Check if there are available slots
        // Send the data to the disk to be added
        request->msg_type=1;
        msgsnd(msg_down, request, sizeof(request->msg_text), 0);
        printf("kernel send the request to the disk/\n");
        // Receive message from disk process indicating the result of addition
        struct msg_buffer DiskAdd; 
        // received ADD status from disk
         int messagerecive = msgrcv(msg_down, &DiskAdd, sizeof(DiskAdd.msg_text),330, IPC_NOWAIT) ;
       
        // Send a message to the process indicating successful or unable to ADD
        DiskAdd.msg_type=502;
        msgsnd(msg_down_PK, &DiskAdd, sizeof(DiskAdd.msg_text), 0);  
        printf("send status to the process \n");
        //process need to recive it  --  ahmed
        //log the status of the request in the log file
       if(DiskAdd.msg_text[17]=='d')
        {
           fprintf(log_file, "At time = %d,send success to P1\n", clk);//log the success of the request in the log file
        }
        else
        {
            fprintf(log_file, "At time = %d,send failed to P1\n", clk); //log the failure of the request in the log file
        }
    
        fflush(log_file);

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
    // struct msg_buffer msg_send;  
    // msg_send.msg_type = 2;  
       //log the operation request in the log file
    fprintf(log_file, "At time = %d, request to delete slot\"%s\" from P1\n", clk,request->msg_text);
    request->msg_type=2;

    // Send the message to the Disk Process (down_queue)
    if (msgsnd(msg_down, request, sizeof(request->msg_text), 0) == -1) {
        perror("Error sending message to Disk Process");
    }

    // Receive message from disk process indicating the result of deletion
    struct msg_buffer DiskDelete;
    int a = msgrcv(msg_down, &DiskDelete, sizeof(DiskDelete.msg_text), 111, IPC_NOWAIT); //---------------------- edit ahmed -------------------
    if (a == -1) {
        perror("Error receiving Disk status");
        //exit(EXIT_FAILURE);
    }

    // Send message to the process to indicate the result of deletion (msg_down_PK)
    DiskDelete.msg_type=502;
    msgsnd(msg_down_PK, &DiskDelete, sizeof(DiskDelete.msg_text), 0);

    // Log the operation delete in the log file
    // Log the status of the delete request in the log file
    if(DiskDelete.msg_text[19]=='d')
    {
       fprintf(log_file, "At time = %d,send success to P1\n", clk);//log the success of the request in the log file
    }
    else
    {
        fprintf(log_file, "At time = %d,send failed to P1\n", clk); //log the failure of the request in the log file
    }
    fflush(log_file);//flush the log file (clear the buffer)

}

// // Thread function for sending SIGUSR2 every second
// void* periodic_signal_sender(void* arg) {
//     while (1) {
//         sleep(1);
//         kill(getpid(), SIGUSR2);
//     }
//     return NULL;
// }

int main() 
{

    //printf("recieved process id %d",process1id);
    clk=0;

    // Signal setup for clock updates
    signal(SIGUSR2, sig_handler);

    // Open the log file at the beginning of the program
    log_file = fopen("log.txt", "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    //******* Disk Kernel Message Queues ***********//
    key_t up_key = 102;
    key_t down_key = 103;
    int up_queue = msgget(up_key, IPC_CREAT | 0666);
    int down_queue = msgget(down_key, IPC_CREAT | 0666);
    msg_up = up_queue;
    msg_down = down_queue;

   // ******* Process Kernel Message Queues ***********//
    key_t up_key_PK = 77;
    key_t down_key_PK = 55;
    int up_queue_PK = msgget(up_key_PK, IPC_CREAT | 0666);
    int down_queue_PK = msgget(down_key_PK, IPC_CREAT | 0666);
    msg_up_PK = up_queue_PK;
    msg_down_PK = down_queue_PK;

    if (up_queue == -1 || down_queue == -1 ) 
    {
        perror("Error in creating message queues");
        exit(EXIT_FAILURE);
    }

    // Compile the disk process
   
    // Receive the Disk PID from the Disk Kernel
   // struct ClkMessage  sendclkToprocess;
    //struct ClkMessage  sendclkToDisk;
    struct pid_msgdisk received_diskid;
    struct pid_msgprocess received_processid;

    // //****************************....clk send *********************/
    // sendclkToprocess.msg_type = 7; // Message type for PID communication
    // sendclkToprocess.clk = clk;
    // sendclkToDisk.msg_type = 8; // Message type for PID communication
    // sendclkToDisk.clk = clk;

// // Send the clk to the process
//     if (msgsnd(up_queue_PK, &sendclkToprocess, sizeof(sendclkToprocess.clk), 0) == -1) {
//         perror("Error sending Disk PID");
//         exit(EXIT_FAILURE);
//     }
// // Send the clk to the process
//     if (msgsnd(up_queue, &sendclkToDisk, sizeof(sendclkToDisk.clk), 0) == -1) {
//         perror("Error sending Disk PID");
//         exit(EXIT_FAILURE);
//     }
    

  //**********************************get diskid *********************************//
    int diskid = msgrcv(up_queue, &received_diskid, sizeof(received_diskid.disk_pid), 4, 0);
   if (diskid== -1) {
        perror("Error receiving Disk PID");
        exit(EXIT_FAILURE);
    }
    disk_id = received_diskid.disk_pid;
    printf("Kernel Process: Received Disk PID: %d\n", disk_id);

    //**********************************get processid *********************************//
     int processid = msgrcv(up_queue_PK, &received_processid, sizeof(received_processid.process_pid), 15, !IPC_NOWAIT);
    if (processid== -1) {
        perror("Error receiving process id");
        exit(EXIT_FAILURE);
    }
 
    processid=received_processid.process_pid;
    // Rest of your kernel process code...
    printf("Kernel Process: Received process PID: %d\n", processid);


    //-------------------test omar -----------------------

    
















        // clk++;
        //kill(disk_id,SIGUSR2);
        //kill(processid,SIGUSR2);
        // Receive message from pro
    while (1)
    {
        sleep(1);

        kill(disk_id,SIGUSR2);
        kill(processid,SIGUSR2);
        clk++;

        printf("%d \n",clk); 

        // Receive message from process
        struct msg_buffer received_msg; // received message from process
        received_msg.msg_text[0]='N';

        if (msgrcv(up_queue_PK, &received_msg, sizeof(received_msg.msg_text), 0, IPC_NOWAIT) == -1) 
        {
            //printf("Error receiving message from process");
            //exit(1);
        }
        // Check the operation type
        if (received_msg.msg_text[0] == 'A') 
        {  // ADD
            printf("recieved add request from process");
            handleAddRequest(&received_msg, log_file);
        } else if (received_msg.msg_text[0] == 'D') 
        {  // DEL
            printf("recieved delete request from process\n");
            handleDelRequest(&received_msg, log_file);
        } else {
            // Send a message to the process indicating that the request can't be handled
            struct msg_buffer response;
            response.msg_type = 2;
            strcpy(response.msg_text, "The request can't be handled.");
            msgsnd(msg_down_PK, &response, sizeof(response.msg_text), 0);
            
        }
    }

    // Close the log file when done
    fclose(log_file);


    return 0;
}
