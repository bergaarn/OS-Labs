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

// Three gigantic globally declared matrices
static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];

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

// Parallel matrix initialization
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


// Thread Function
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

static void print_matrix(void)
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
            printf(" %7.2f", c[i][j]);
        printf("\n");
        //fflush(stdout);
    }
}

int main(int argc, char **argv)
{
    //init_matrix();
    init_matrix_para();
    matmul_para();
    //print_matrix();
}