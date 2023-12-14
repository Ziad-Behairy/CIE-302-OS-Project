#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#define SLOT_SIZE 64

int clk;
int counter=0;
int msg_up, msg_down;
// Message structure
void waitForTime( int starttime) 
{
    while (1) 
    {
        if (clk>=starttime) 
        {
            break; // Break the loop when the desired time has passed
        }
    }
}

struct ClkMessage {
    long msg_type;
    int clk;
};
struct Message {
    long mtype;
    char data[SLOT_SIZE];
};
struct pid_msg {
    long msg_type;
    pid_t pid;
};

 void siguser2_handler(int signum)
 {
    clk++;
    printf("%d clk now is \n",clk); 
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
    if (clk==time || clk > time)
    {
        if (msgsnd(up_queue_PK, &request, sizeof(request.data),0) == -1) 
        {
        perror("Error adding request to UP Queue");
        exit(EXIT_FAILURE);
        }
        struct Message operationstatus; 
        int statusmessage=msgrcv(msg_down, &operationstatus, sizeof(operationstatus.data),502, IPC_NOWAIT)== -1;
        printf("recieved  status  %s \n",operationstatus.data);// delete 


      printf("Send request to the kernel");
      printf("Added request to UP Queue: %s\n", request.data);
    }
    else
    {
      
      if (msgsnd(up_queue_PK, &request, sizeof(request.data),!IPC_NOWAIT) == -1) 
        {
        perror("Error adding request to UP Queue");
        exit(EXIT_FAILURE);
        }
    }
    counter++;
    printf("the final clk after process %d %d/n",clk,counter);
    
}



int main() 
{
    signal(SIGUSR2, siguser2_handler);


    printf("at the start of the process\n");
    clk=0;
    key_t up_key_PK = 77;
    key_t down_key_PK = 55;
    int up_queue_PK = msgget(up_key_PK, IPC_CREAT | 0666);
    int down_queue_PK = msgget(down_key_PK, IPC_CREAT | 0666);
    msg_up = up_queue_PK;
    msg_down = down_queue_PK;

      //***********************************send pid to the kernel process ****************************/
    pid_t process_pid = getpid();

    struct pid_msg process__id;
    process__id.msg_type = 15; // Message type for PID communication
    process__id.pid = process_pid;

// Send the PID to the kernel process

    int test = msgsnd(up_queue_PK, &process__id, sizeof(process__id.pid), 0);
    printf ("the test send id %d \n",test);
    if (test == -1) {
        perror("Error sending Process PID");
        exit(EXIT_FAILURE);
    }

//-------------------------test-----------------------------------




    printf("Send the PID to the kernel process\n");
//        //**********************************get clk *********************************//
//     struct ClkMessage messageclk;
//        int message1 = msgrcv(up_queue_PK, &messageclk, sizeof(messageclk.clk), 7, 0);
//     if (message1== -1) {
//         perror("Error receiving clk ");
//         exit(EXIT_FAILURE);
//     }
//     clk=messageclk.clk;
//     printf("revieved clk from the kernel %d",clk);
// //**************************************************************************/
 

    FILE* file = fopen("test.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
        printf("epen the file\n");
    char line[100];
    int time;
    char operation[10]; // "ADD" or "DEL"
    char data[SLOT_SIZE]; // Message content
     
        while (fgets(line, sizeof(line), file) != NULL) 
        {

                printf("%d \n",clk); 

       

             if (sscanf(line, "%d \"%[A-Z]\" \"%[^\"]\"", &time, operation, data) == 3)
              {
                    waitForTime(time);
                    printf("inside formulate if \n");
                    formulateAndAddRequest(up_queue_PK, time, operation, data);
                    printf("end formulate \n");

              } 
              else 
               {
                   fprintf(stderr, "Error parsing line: %s\n", line);
               }
        }

    fclose(file);

 
    return 0;
}




// 13 "ADD" "lunch time"
// 20 "ADD" "nice to meet you"
