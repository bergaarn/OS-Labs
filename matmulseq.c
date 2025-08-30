    /***************************************************************************
     *
     * Sequential version of Matrix-Matrix multiplication
     *
     ***************************************************************************/

    #include <stdio.h>
    #include <stdlib.h>

    #define SIZE 1024

    static double a[SIZE][SIZE];
    static double b[SIZE][SIZE];
    static double c[SIZE][SIZE];

    static void init_matrix(void)
    {
        for (int i = 0; i < SIZE; i++)
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

    static void matmul_seq()
    {
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++) 
            {
                c[i][j] = 0.0;

                for (int k = 0; k < SIZE; k++)
                {
                    c[i][j] = c[i][j] + a[i][k] * b[k][j];
                }               
            }
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
        init_matrix();
        matmul_seq();
        //print_matrix();
    }