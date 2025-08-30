#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv)
{
    pid_t pid1, pid2;
    unsigned i;
    unsigned niterations = 100;
    pid1 = fork();
    if (pid1 == 0) 
    {
        for (i = 0; i < niterations; ++i)
        {
            printf("A = %d, ", i);
        }
    } 
    else
    {
        pid2 = fork();
        if (pid2 == 0) 
        {
            for (i = 0; i < niterations; ++i)
            {
                printf("C = %d, ", i);
            }
                
        } 
        else 
        {
            for (i = 0; i < niterations; ++i)
            {
                printf("B = %d, ", i);
            }

            printf("Child A pid: %d", pid1);
            printf(" | Child C pid: %d", pid2);
        }
    }
    printf("\n");
}
