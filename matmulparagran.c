/***************************************************************************
 *
 * Sequential version of Matrix-Matrix multiplication
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//#define SIZE 64
#define SIZE 1024
#define NUMBER_OF_THREADS 4 // Number of threads equal to number of cores

typedef struct // Determining row range for each thread/core to execute 
{
    int firstRow;
    int lastRow;    
} ThreadInfo;

// Three gigantic globally declared matrices
static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

// Sequential matrix initialization 
static void init_matrix(void)
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++) 
        {
            /* Simple initialization, which enables us to easy check
            * the correct answer. Each element in c will have the same
            * value as SIZE after the matmul operation.
            */
            a[i][j] = 1.0;
            b[i][j] = 1.0;
        }
    }
}

// Thread initiailization utility function
void* init_matrix_thread(void* arg) 
{
    // Extract row index from argument passed to thread function
    int i = *(int*)arg;

    for (int j = 0; j < SIZE; j++) 
    {
        a[i][j] = 1.0; // Assign 1.0 to each element in a and b matrices
        b[i][j] = 1.0;
    }
    pthread_exit(NULL);  // Terminate thread to join main thread
    return NULL;
}

// Parallelized initialization function
static void init_matrix_para() 
{
    pthread_t threads[SIZE]; // Threads Array
    int rowIndex[SIZE];  // Send unique index for each row as argument to thread function

    for (int i = 0; i < SIZE; i++)
    {
        rowIndex[i] = i;
        // Create one thread for each row 
        pthread_create(&threads[i], NULL, init_matrix_thread, &rowIndex[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < SIZE; i++) 
    {
        pthread_join(threads[i], NULL);
    }
}


// Thread matrix multiplication utility function
void* matmul_thread(void* arg)
{
    // Extract row index from argument passed to thread function
    int i = *(int*)arg;   

    for (int j = 0; j < SIZE; j++) 
    {
        // Initialize the values in c matrix before adding the dot product
        c[i][j] = 0.0; 

        for (int k = 0; k < SIZE; k++) 
        {
            c[i][j] += a[i][k] * b[k][j]; // Calculate the dot product of a and b and add sum to c
        }
    }
    pthread_exit(NULL); // Thread finished, send signal back to main thread to join

    return NULL;
}

// Parallelized matrix mulitplication function
static void matmul_para()
{
    pthread_t threads[SIZE]; // Thread Array
    int rowIndex[SIZE];   // Send unique index for each row as argument to thread function

    for (int i = 0; i < SIZE; i++) 
    {
        rowIndex[i] = i;
        pthread_create(&threads[i], NULL, matmul_thread, &rowIndex[i]);
        // Create thread in slot i with function call matmul_thread with row index argument per thread
    }

    // Wait for all threads to finish
    for (int i = 0; i < SIZE; i++) 
    {
        pthread_join(threads[i], NULL);
    }
}

// Chunked thread initialization function
void* init_matrix_thread_chunk(void* arg)
{
    ThreadInfo* thread = (ThreadInfo*)arg;

    for (int i = thread->firstRow; i < thread->lastRow; i++)
    {
        for (int j = 0; j < SIZE; j++) 
        {
            a[i][j] = 1.0;
            b[i][j] = 1.0;
        }
    }
    pthread_exit(NULL);
    return NULL;
}

// Chunked initialization function
void init_matrix_para_chunked()
{
    pthread_t threads[NUMBER_OF_THREADS];
    ThreadInfo threadInfo[NUMBER_OF_THREADS];

    // Number of rows each thread (core) is responsible for
    int rowsPerThread = SIZE / NUMBER_OF_THREADS;

    for (int i = 0; i < NUMBER_OF_THREADS; i++) 
    {
        // Last row is responsible for picking up any remainining rows if SIZE isnt divisble by number of cores
        // otherwise last row is determined by i+1 * the number of rows per each core. 
        // Core 0 example: First row = 0 * 256 = 0 | Last row = i+1 = 1 * 256 = 256 -> Row number 0 -> 256 for core 0
        threadInfo[i].firstRow = i * rowsPerThread;
        threadInfo[i].lastRow = (i == NUMBER_OF_THREADS - 1) ? SIZE : (i + 1) * rowsPerThread;
        pthread_create(&threads[i], NULL, init_matrix_thread_chunk, &threadInfo[i]);
    }

    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }    
}

// Chunked thread matrix multiplication function
void* matmul_thread_chunk(void* arg)
{
    ThreadInfo* threadInfo = (ThreadInfo*)arg;

    for (int i = threadInfo->firstRow; i < threadInfo->lastRow; i++)
    {
        for (int j = 0; j < SIZE; j++) 
        {
            // Initialize all elements in c to 0.0
            c[i][j] = 0.0;

            for (int k = 0; k < SIZE; k++)
            {
                // Add dot product of a and b to c
                c[i][j] += a[i][k] * b[k][j];
            }  
        }
    }  
    pthread_exit(NULL);
    return NULL;
}

// Chunked matrix multiplication function
void matmul_para_chunked()
{
    pthread_t threads[NUMBER_OF_THREADS];
    ThreadInfo threadInfo[NUMBER_OF_THREADS];

    // Number of rows each thread (core) is responsible for
    int rowsPerThread = SIZE / NUMBER_OF_THREADS;

    for (int i = 0; i < NUMBER_OF_THREADS; i++) 
    {
        // Last row is responsible for picking up any remainining rows if SIZE isnt divisble by number of cores
        // otherwise last row is determined by i+1 * the number of rows per each core. 
        // Core 0 example: First row = 0 * 256 = 0 | Last row = i+1 = 1 * 256 = 256 -> Row number 0 -> 256 for core 0
        threadInfo[i].firstRow = i * rowsPerThread;
        threadInfo[i].lastRow = (i == NUMBER_OF_THREADS - 1) ? SIZE : (i + 1) * rowsPerThread;
        pthread_create(&threads[i], NULL, matmul_thread_chunk, &threadInfo[i]);
    }

    for (int i = 0; i < NUMBER_OF_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }   
}

static void print_matrix(void)
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            printf(" %7.2f", c[i][j]);
        }
        printf("\n");
        //fflush(stdout);
    }
}

int main(int argc, char **argv)
{
    //init_matrix();        // sequential initialization
    //init_matrix_para();   // parallel initialization
    //matmul_para();        // parallel matrix multiplication
    init_matrix_para_chunked(); // chunked parallel matrix mulitplication initialization
    matmul_para_chunked();  // chunked parallel matrix multiplication
    //print_matrix();
}