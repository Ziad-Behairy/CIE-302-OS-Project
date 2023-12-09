#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#define MAX_SLOTS 10 
#define SLOT_SIZE 64
int msg_up, msg_down;
int clk = 0;
int available_slots = 10;
typedef struct {
    char data[MAX_SLOTS][SLOT_SIZE];
    int used_slots;
} Storage;
void initializeStorage(Storage *storage) {
    for (int i = 0; i < MAX_SLOTS; ++i) {
        storage->data[i][0] = '\0'; // Initialize all slots as empty
    }
    storage->used_slots = 0;
}
bool addMessage(Storage *storage, const char *message) {
    if (storage->used_slots >= MAX_SLOTS) {
        return false; // Storage is full
    }

    for (int i = 0; i < MAX_SLOTS; ++i) {
        if (storage->data[i][0] == '\0') { // Check for an empty slot
            strncpy(storage->data[i], message, SLOT_SIZE - 1);
            storage->data[i][SLOT_SIZE - 1] = '\0'; // Ensure null termination
            storage->used_slots++;
            return true;
        }
    }
    return false;
}
bool deleteMessage(Storage *storage, int slotIndex) {
    if (slotIndex < 0 || slotIndex >= MAX_SLOTS) {
        return false; // Slot index out of range
    }

    if (storage->data[slotIndex][0] != '\0') { // Check if the slot is not already empty
        // Clear the entire slot by setting each character to '\0'
        for (int i = 0; i < SLOT_SIZE; ++i) {
            storage->data[slotIndex][i] = '\0';
        }
        storage->used_slots--;
        return true;
    }
    return false; // Slot was already empty
}

int availableSlots(const Storage *storage) {
    return MAX_SLOTS - storage->used_slots;
}



typedef  struct 
{
    long msg_type; 
    char msg_text[64];
} msg_buffer ;

struct pid_msg {
    long msg_type;
    pid_t disk_pid;
};
  msg_buffer msg_handlersend;
  Storage storage;

void sigusr1_handler(int signum) 
{
    int availableslots = availableSlots(&storage); 
    msg_buffer msg_handlersend;

    // Set msg_type to 42 (long)
    msg_handlersend.msg_type = 42;

    // Convert availableslots integer to string and copy to msg_text
    snprintf(msg_handlersend.msg_text, sizeof(msg_handlersend.msg_text), "%d", availableslots);

    // Send the message to the UP queue
    msgsnd(msg_up, &msg_handlersend, sizeof(msg_handlersend.msg_text), 0) ;

 }

int main() 
{


     signal(SIGUSR1, sigusr1_handler);


    initializeStorage(&storage);
    //----------------
    key_t up_key = ftok("Diskkernel", 'U');
    key_t down_key = ftok("Diskkernel", 'D');
    int up_queue = msgget(up_key, IPC_CREAT | 0666);
    int down_queue = msgget(down_key, IPC_CREAT | 0666);
    msg_up = up_queue;
    msg_down = down_queue;
    //--------------------
    pid_t disk_pid = getpid();

    struct pid_msg pid_message;
    pid_message.msg_type = 4; // Message type for PID communication
    pid_message.disk_pid = disk_pid;


    msg_buffer msg_recv; // message got from the kernel to down queue 
    msg_buffer msg_send; // message send for success or failure 

// Send the PID to the kernel process
    if (msgsnd(down_queue, &pid_message, sizeof(pid_message.disk_pid), 0) == -1) {
        perror("Error sending Disk PID");
        exit(EXIT_FAILURE);
    }

    // while (1) {
    //     if (msgrcv(msg_down, &msg_recv, sizeof(msg_recv.msg_text), 0, 0) == -1) 
    //     {
    //         perror("msgrcv");
    //         exit(1);
    //     }
    //     if(msg_recv.msg_type==1) // recived add from the kernel 
    //     {
            
    //         if(addMessage(&storage,msg_recv.msg_text)==true) 
    //         {
    //          msg_send.msg_type =="0";
    //          strcpy(msg_send.msg_text, "adding operation done successfully");
    //          msgsnd(msg_down, &msg_send, sizeof(msg_send.msg_text), 0);
    //         }
    //         else 
    //         {
    //         msg_send.msg_type =="2";
    //          strcpy(msg_send.msg_text, "adding operation failure");
    //          msgsnd(msg_down, &msg_send, sizeof(msg_send.msg_text), 0);
    //         }

    //     }
    //     else if(msg_recv.msg_type==2) // recived deletion from the kernel 
    //     {
            
    //         if(deleteMessage(&storage,msg_recv.msg_text)==true) 
    //         {
    //            msg_send.msg_type =="1";
         
    //          strcpy(msg_send.msg_text, "deleting operation done successfully");
    //          msgsnd(msg_down, &msg_send, sizeof(msg_send.msg_text), 0);
    //         }
    //         else 
    //         {
    //         msg_send.msg_type =="3";
    //          strcpy(msg_send.msg_text, "deleting operation failure");
    //          msgsnd(msg_down, &msg_send, sizeof(msg_send.msg_text), 0);
    //         }
    //     }
       
    //     // Process request...
    //     // Implement logic for ADD, DELETE, STATUS operations
    // }

    return 0;
}