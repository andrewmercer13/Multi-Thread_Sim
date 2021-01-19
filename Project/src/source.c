/*
 * File:	source.c
 * Author:	Andrew Merer
 * Date:	2020-07-05
 * Version:	1.0
 *
 * Purpose:	To create a variable number of threads and simulate differences 
 *          between synchronized and unsynchronized threading modeals.
 * 
 * Notes:   This should be compiled using:
 *          gcc -o A4 source.c -lpthread
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

/*Global variables*/
int sv; 
int thread_num;
int thread_sum = 0;

  
static pthread_mutex_t lock;


/*
 * Name:	*threadFunc()
 * Purpose:	    To set the global variable sv to 0 and print number of times
 *              sv changes when sv is set to 2 by main
 * Arguments:	argc, sv
 * Outputs:	    Outputs thread id and number of times thread has changed sv.
 * Modifies:	sv
 * Returns:	    NULL
 * Assumptions:	
 * Bugs:	
 * Notes:       This simulated the unsynchronized model. 
 */

void *threadFunc(void *arg)
{
    /*Converts thread id pointer from main into an int*/
    int thread = *(int *)arg;
    unsigned long thread_sv_count = 0;
    printf("I am thread %d, starting up now\n",thread);
    
    while(1)
    {   
        if(sv == 1)
        {
            thread_sv_count++;
            sv = 0;
            /*needed to avoid first thread doing all tasks*/
            usleep(10);
        }
        if(sv == 2)
        {
            printf("I am thread %d; I changed the value %ld times\n",thread, thread_sv_count);
            break;
        }
        
        
    }
    /*Sum of total number of times sv was changed in this thread, used in main*/
    thread_sum = thread_sum + thread_sv_count;
    /*Frees memory allocated for the thread_id*/
    free(arg);
    return NULL;
}
/*
 * Name:	*threadFunc2()
 * Purpose:	    To set the global variable sv to 0 and print number of times
 *              sv changes when sv is set to 2 by main
 * Arguments:	argc, sv
 * Outputs:	    Outputs thread id and number of times thread has changed sv.
 * Modifies:	sv
 * Returns:	    NULL
 * Assumptions:	
 * Bugs:	    I believe the code might be hanging at when very large numbers 
 *              of iterations are input, but it could be taking a long tme to 
 *              process.
 * Notes:       This simulated the synchronized model. 
 */
void *threadFunc2(void *arg)
{
    /*Converts thread id pointer from main into an int*/
    int thread = *(int *)arg;
    unsigned long thread_sv_count = 0;
    printf("I am thread %d, starting up now\n",thread);
    while(1)
    {   
        /*mutex lock used to avoid race condition on sv*/
        pthread_mutex_lock(&lock);
        if(sv == 1)
        {
            thread_sv_count++;
            sv = 0;
            usleep(10);
        }
        /*mutex unlock to avoid deadlock*/
        pthread_mutex_unlock(&lock);
        usleep(10);
        /*mutex lock used to avoid race condition on sv*/
        pthread_mutex_lock(&lock);
        if(sv == 2)
        {
            printf("I am thread %d; I changed the value %ld times\n",thread, thread_sv_count);
            /*mutex unlock to avoid deadlock*/
            pthread_mutex_unlock(&lock);
            break;
        }
        /*mutex unlock to avoid deadlock*/
        pthread_mutex_unlock(&lock);
        
    }
    /*Sum of total number of times sv was changed in this thread, used in main*/
    thread_sum = thread_sum + thread_sv_count;
    /*Frees memory allocated for the thread_id*/
    free(arg);
    return NULL;
}

/*
 * Name:	main();
 * Purpose:	    Parses command line arguments, sets sv to 1 until the threads have 
 *              set sv to 0 the desired number of times, and then will set sv to 2. 
 *              threads will then be joined. 
 * Arguments:	argc, argv sv
 * Outputs:	    Outputs terminal messages regarding sv change data
 * Modifies:	sv and sv_count
 * Returns:	    0
 * Assumptions:	Assume command line arguments are input in the correct order
 *              with no additional arguments contained.
 * Bugs:	   
 * Notes:       
 */
int main(int argc, char *argv[])
{   
    long int num_of_sv_increases;
    int sv_count = 0;
    sv = 0;
    int sync_bool = 0;
    int * ptr;


    char* temp = argv[1];
    /*Parses command line arguments if "-sync" is included*/
    if(strcmp("-sync",temp) == 0)
    {
        temp = argv[2];
        thread_num = strtol(temp,NULL,10);
        /*Avoids using inputing and number of threads below 1*/
        if(thread_num <= 0)
        {
            printf("Must have at least 1 thread\n");
            exit(1);
        }
        temp = argv[3];
        num_of_sv_increases = strtol(temp,NULL,10);
        /*Used later to call sync version of threadfunc*/
        sync_bool = 1;
    }
    else
    {
        temp = argv[1];  
        thread_num = strtol(temp,NULL,10);
        /*Avoids using inputing and number of threads below 1*/
        if(thread_num <= 0)
        {
            printf("Must have at least 1 thread\n");
            exit(1);
        }
        temp = argv[2];
        num_of_sv_increases = strtol(temp,NULL,10);
    }
    /*thread_num + 1 because main is a thread as well*/
    pthread_t thread_id[thread_num + 1];
    
    /*creates mutex locks only if sync version of threadfunc is going to be used*/
    if(sync_bool == 1)
    {
        pthread_mutex_init(&lock, NULL);
    }

    

    /*Creates threads, quantity based on command line arguments*/
    unsigned i = 0;
    for(i = 0; i < thread_num + 1; i++)
    {   
        if(0 == i)
        {
            /* stores main thread id, used for thread creation */
            thread_id[i] = pthread_self(); 
            

        }
        else
        {
            /*used to pass thread thread id variable to thread functions*/
            ptr = (int *)malloc(sizeof(int));
            *ptr = i;
            /*creates either sync of unsync threads based on sync_bool*/
            if(sync_bool == 0)
            {
                pthread_create(&thread_id[i], NULL, threadFunc, (void* )ptr);
            }
            else if (sync_bool == 1)
            {
                pthread_create(&thread_id[i], NULL, threadFunc2, (void* )ptr);
            }
            
        }
        
    }
    sv = 1;
    while(1)
    {
        if(sv == 0)
        {
            /*counts number of times sv is set to 0 by the threads*/
            sv_count++;
            if(sv_count == num_of_sv_increases)
            {
                printf("main(): setting sv to 2 and joining the threads\n");
                sv = 2;
                break;
            }
        
            sv = 1;
        }


    }
    
    
    /*Join threads*/
    unsigned j;
    for(j = 0; j < i; j++)
    {   
        pthread_join(thread_id[j], NULL);
    }
    printf("main(): all the threads have finished\n");
    printf("main(): I have set sv to 1 %d times, the threads reset it %d times.\n",sv_count,thread_sum);

    /*removes mutex locks*/
    if(sync_bool == 1)
    {
        pthread_mutex_destroy(&lock);
        pthread_exit(0);  
    }
    return 0;
}