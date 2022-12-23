#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Reader/Reader.h"


void* Reader(void* arg)
{
    printf("Hello from reader!\n");
    int fd = -1;
    
    for(;;)
    {
        /*Get raw data from /proc/stat */
        if((fd = open("/proc/stat", O_RDONLY)) < 0)
        {
            printf("Error during file open!\n");
            break;    
        }




        if(close(fd) != 0)
        {
            printf("Error during closing file!\n");
            break;
        }
    }

    return arg;
}


int main()
{
    pthread_t th[5];

    // Reader Task
    if(pthread_create(&th[0], NULL, Reader, NULL) != 0)
    {
        printf("Pthread_create error!\n");
        goto CleanUp;
    }



    // join
    // for(int t = 0; t < 5; t++)
    // {
    //     if(pthread_join(th[t], NULL) != 0)
    //     {
    //         printf("Pthread_create error!\n");
    //         return -1;
    //     }
    // }

    if(pthread_join(th[0], NULL) != 0)
    {
        printf("Pthread_create error!\n");
        return -1;
    }


    printf("Cpu Usage Tracker \n");
    return 0;

CleanUp:
    return -1;
}
