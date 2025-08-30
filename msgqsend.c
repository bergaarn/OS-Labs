#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <limits.h>
#include <time.h>

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
   system("touch msgq.txt");

   if ((key = ftok("msgq.txt", 'B')) == -1) 
   {
      perror("ftok");
      exit(1);
   }

   if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1) 
   {
      perror("msgget");
      exit(1);
   }

   printf("message queue: ready to send messages.\n");
   printf("Start sending random integers between 0 and %d\n", INT_MAX);
   buf.msgType = 1;
   srand(time(NULL));

   for (int i = 0; i < 50; i++) 
   {
      buf.msgInt = (rand() % INT_MAX) + 1;
      if (msgsnd(msqid, &buf, sizeof(int), 0) == -1)
      {
         perror("msgsnd");
         exit(1);
      }
      printf("Sent: %d\n", buf.msgInt);
   }

   // Tell reciever to close down by sending 0
   buf.msgInt = 0;
   if (msgsnd(msqid, &buf, sizeof(int), 0) == -1) 
   {
      perror("msgsnd");
      exit(1);
   }
   printf("All numbers sent.\n");

   printf("message queue: done sending messages.\n");
   return 0;
}
