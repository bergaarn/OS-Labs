#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>		// Needed for usleep()
#include <stdlib.h>		// Needed for srand()
#include <wait.h>		// Needed for waitpid()

#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

int main(int argc, char** argv)
{
	struct shm_struct
	{
		int buffer[10];
		unsigned in;
		unsigned out;
		unsigned count;
	};

	volatile struct shm_struct* shmp = NULL;
	char* addr = NULL;
	pid_t pid = -1;
	int var1 = 0, var2 = 0, shmid = -1;
	struct shmid_ds* shm_buf;
	srand(time(NULL)); 							// Create a seed that randomly generates the sleep duration between timespans

	/* allocate a chunk of shared memory */
	shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
	shmp = (struct shm_struct *) shmat(shmid, addr, 0);
	shmp->in = 0, shmp->out = 0;				// Initalize starting position for write and reading in buffer
	shmp->count = 0;							// Initalize count to current items in the buffer

	pid = fork();
	if (pid != 0) // Parent Process (Producer)
	{
		while (var1 < 100) 
		{
			var1++;
			while (shmp->count == 10); 			// Busy wait if the buffer is full to avoid overwriting and race conditions
			printf("Sending %d\n", var1);
			fflush(stdout);
			shmp->buffer[shmp->in] = var1; 		// Write the producer variable into the current writing position in the array
			shmp->in = (shmp->in + 1) % 10;		// Increment the writing position and modulo by 10 to wrap back around
			shmp->count++; 						// Increment the count to keep busy waiting consistent
			usleep(100000 + rand() % 400000); 	// Sleep a random time between 0.1 sec and 0.5 sec
		}

		waitpid(pid, NULL, 0);					// Used to wait for the child process to finish before returning control to terminal
												// to avoid interleaving with remaining printf's from the child and the prompt
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
		
	}
	else // Child Process (Consumer)
	{
		while (var2 < 100) 
		{
			while (shmp->count == 0); 			// Busy wait if there is nothing to read in the buffer
			var2 = shmp->buffer[shmp->out];		// Read the current reading position in the array into the consumer variable
			shmp->out = (shmp->out + 1) % 10; 	// Increment the reading position and modulo by 10 to wrap back around
			shmp->count--; 						// Decrement the count to keep busy waiting consistent
			printf("Received %d\n", var2);
			fflush(stdout);
			usleep(200000 + rand() % 1800000); 	// Sleep a random time between 0.2 and 2.0 sec
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	}
}
