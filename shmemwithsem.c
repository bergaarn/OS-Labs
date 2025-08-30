#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>		// Needed for usleep()
#include <stdlib.h>		// Needed for srand()
#include <wait.h>		// Needed for waitpid()
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> /* For O_* constants */

#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

const char* sem_id1 = "/semEmpty";
const char* sem_id2 = "/semFull";	// Forward slash since its portable with more general POSIX versions
const char* sem_id3 = "/semMutex";

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
	srand(time(NULL)); 	// Create a seed that randomly generates the sleep duration between timespans

	sem_t* semEmpty = sem_open(sem_id1, O_CREAT, O_RDWR, 10);	// 10 Empty Slots to start with
	sem_t* semFull = sem_open(sem_id2, O_CREAT, O_RDWR, 0);    // 0 Full Slots to start with
	sem_t* semMutex = sem_open(sem_id3, O_CREAT, O_RDWR, 1);  // 1 to open the mutex for the first process to access critical region

	/*
		Empty Semaphore keeps producer from writing when the buffer is full
		Full Semaphore keeps consumer from reading when the buffer is empty
		Mutex Semaphore keeps producer and consumer from accessing buffer at the same time

	*/

	/* allocate a chunk of shared memory */
	shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
	shmp = (struct shm_struct *) shmat(shmid, addr, 0);
	shmp->in = 0, shmp->out = 0;				// Initalize starting position for write and reading in buffer
	
	pid = fork();
	if (pid != 0) // Parent Process (Producer)
	{
		while (var1 < 100) 
		{
			var1++;

			sem_wait(semEmpty);	// Wait for an empty slot to be available to write to then decrement empty slot in buffer
			sem_wait(semMutex);	// Wait until mutex is 1 then decrement to enter critical region and keep other processes from accessing the buffer
			
			printf("Sending %d\n", var1); fflush(stdout);
			shmp->buffer[shmp->in] = var1; 		// Write the producer variable into the current writing position in the array
			shmp->in = (shmp->in + 1) % 10;		// Increment the writing position and modulo by 10 to wrap back around
			
			sem_post(semMutex); // Increment the mutex to allow other processes to enter critical region
			sem_post(semFull);	// Increment the amount of full slots in the buffer 
			
			/*

			Sleep turned off since semaphores allows for synchronization with protection for critical region
			(Faster execution but OS can allow more timeslots for one process in a row for less fair printing)
			
			The count variable could be altered by both processes at the same time which can
			cause race conditions

			Busy waiting is inefficient since it can still consume CPU cycles while waiting

			Semaphores also avoid under and over buffering which can skip and duplicate data in the buffer
			by reading the same number twice or not at all
			
			*/

			//usleep(100000 + rand() % 400000); 	// Sleep a random time between 0.1 sec and 0.5 sec
		}

		waitpid(pid, NULL, 0);					// Used to wait for the child process to finish before removing shared memory, removing semaphore and returning control to terminal
		shmdt(addr);							// Deattach from shared memory 
		shmctl(shmid, IPC_RMID, shm_buf);		// Remove shared memory when all attachted processes has deattatched

		sem_close(semEmpty);
		sem_close(semFull);		// Close process access to semaphore
		sem_close(semMutex);
		sem_unlink(sem_id1);
		sem_unlink(sem_id2);	// Remove semaphore from system when all processes have closed access (done by creator or last process to close)
		sem_unlink(sem_id3);
		
	}
	else // Child Process (Consumer)
	{
		while (var2 < 100) 
		{
			sem_wait(semFull);	// Wait for a full slot to be available then decrement amount of full slots
			sem_wait(semMutex); // Enter critical region preventing other processes from accesing buffer by decrementing mutex to 0

			var2 = shmp->buffer[shmp->out];		// Read the current reading position in the array into the consumer variable
			shmp->out = (shmp->out + 1) % 10; 	// Increment the reading position and modulo by 10 to wrap back around
			printf("Received %d\n", var2); fflush(stdout);
			
			sem_post(semMutex); // Increment mutex to leave critical region
			sem_post(semEmpty);	// Increment the amount of empty slots in the buffer

			//usleep(200000 + rand() % 1800000); 	// Sleep a random time between 0.2 and 2.0 sec
		}
		shmdt(addr);	// Deattach from shared memory

		sem_close(semEmpty);
		sem_close(semFull);		// Close process access to semaphore
		sem_close(semMutex);
	}
}
