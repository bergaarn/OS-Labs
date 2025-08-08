#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PERMS 0644
struct buff_int 
{
   long msgType;
   int msgInt;
};

int main(void) 
{
   struct buff_int buf;
   int msqid;
   key_t key;

   if ((key = ftok("msgq.txt", 'B')) == -1) 
   {
      perror("ftok");
      exit(1);
   }

   if ((msqid = msgget(key, PERMS)) == -1) 
   { /* connect to the queue */
      perror("msgget");
      exit(1);
   }
   printf("message queue: ready to receive messages.\n");

   while (1) // Loop forever until break condition is achieved
   {
      // Read integer from queue
      if (msgrcv(msqid, &buf, sizeof(int), 0, 0) == -1)
      {
         perror("msgrcv");
         exit(1);
      }

      // If message is 0, break out of the loop
      if (buf.msgInt == 0)
      {
         printf("Recieved 0. Closing down.\n");
         break;
      }
      printf("Received: %d\n", buf.msgInt);
   }

   // Reciever is responsible for removing message queue
   // to make sure all messages reaches the reciever
   if (msgctl(msqid, IPC_RMID, NULL) == -1)
   {
      perror("msgctl");
      exit(1);
   }
   printf("message queue: done receiving messages.\n");
   system("rm msgq.txt");
   return 0;
}
