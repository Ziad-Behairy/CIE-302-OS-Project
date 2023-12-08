#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
int msg_up, msg_down;
int clk = 0;

struct msg_buffer 
{
    long msg_type; 
    char msg_text[64];
};
void requestDiskStatus(pid_t disk_pid) {
    kill(disk_pid, SIGUSR1);
}
int main() {
    //******************** disk kernel ****************************//
    key_t up_key = ftok("Diskkernel", 'U');
    key_t down_key = ftok("Diskkernel", 'D');
    int up_queue = msgget(up_key, IPC_CREAT | 0666);
    int down_queue = msgget(down_key, IPC_CREAT | 0666);
    //******************** process kernel ****************************//
    key_t up_key_PK = ftok("processkernel", 'U');
    key_t down_key_PK = ftok("processkernel", 'D');
    int up_queue_PK = msgget(up_key_PK, IPC_CREAT | 0666);
    int down_queue_PK = msgget(down_key_PK, IPC_CREAT | 0666);
    //********************************************************//

    if (up_queue == -1 || down_queue == -1) {
        perror("Error in creating message queue");
        exit(EXIT_FAILURE);
    }

  
    while (1)
    {
    
        
        
    
     

  
      
    }

    msgctl(up_queue, IPC_RMID, NULL);
    msgctl(down_queue, IPC_RMID, NULL);
   return 0 ;
}
