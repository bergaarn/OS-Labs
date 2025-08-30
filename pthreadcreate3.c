#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct threadArgs {
	unsigned int id;
	unsigned int numThreads;
	unsigned int squaredID; // New variable in struct for children to change
};

void* child(void* params) {
	struct threadArgs* args = (struct threadArgs*) params;
	unsigned int childID = args->id;
	unsigned int numThreads = args->numThreads;
	args->squaredID = childID * childID; // Squared childID stored in args pointer which is passed by value to child
	printf("Greetings from child #%u of %u\n", childID, numThreads);
}

int main(int argc, char** argv) {
	pthread_t* children; // dynamic array of child threads
	struct threadArgs* args; // argument buffer
	unsigned int numThreads = 0;
	// get desired # of threads
	if (argc > 1)
		numThreads = atoi(argv[1]);

	printf("I am the parent (main) thread.\n"); // Moved up for clear output
	children = malloc(numThreads * sizeof(pthread_t)); // allocate array of handles
	args = malloc(numThreads * sizeof(struct threadArgs)); // args vector
	for (unsigned int id = 0; id < numThreads; id++) {
		// create threads
		args[id].id = id;
		args[id].numThreads = numThreads;
		pthread_create(&(children[id]), // our handle for the child
			NULL, // attributes of the child
			child, // the function it should run
			(void*)&args[id]); // args to that function passed by value to make sure changes main thread can read changes done by child
	}

	for (unsigned int id = 0; id < numThreads; id++) { // Wait for all child threads to return
		pthread_join(children[id], NULL );
	}

	printf("Squared ID's from child threads\n");
	for (unsigned int id = 0; id < numThreads; id++)
	{
		printf("Child #%u: Squared ID: %u\n", args[id].id, args[id].squaredID); // Print new squaredID values stored by children
	}

	free(args); // deallocate args vector
	free(children); // deallocate array
	return 0;
}
