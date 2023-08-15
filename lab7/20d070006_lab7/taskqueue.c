#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// Global variables
long sum = 0;
long odd = 0;
long even = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false;
int tasks_ongoing = 0 ;
int tasks_read = 0 ;
int tasks_completed = 0 ;
int exit_threads = 0 ;
int end = 0;

// Declaration of thread condition variable
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_com = PTHREAD_COND_INITIALIZER; 
pthread_cond_t cond_exit = PTHREAD_COND_INITIALIZER; 
 
// declaring mutex
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_com = PTHREAD_MUTEX_INITIALIZER ;
pthread_mutex_t lock_exit = PTHREAD_MUTEX_INITIALIZER ;
pthread_mutex_t lock_sum = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_odd_even = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_min = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_max = PTHREAD_MUTEX_INITIALIZER;

long * tasks ;

void processtask(long num);

void *task_complete(void *arg){
    // pthread_mutex_lock(&lock_com) ;
    printf("%d, %d\n", tasks_read, tasks_completed);
    while(tasks_read != tasks_completed){
        pthread_cond_wait(&cond_com,&lock_com) ;
    }
    pthread_cond_broadcast(&cond) ;
    // pthread_mutex_unlock(&lock_com) ;
}

void* do_task(void * arg){
    while(!end || tasks_ongoing < tasks_read){
        pthread_mutex_lock(&lock1) ;
        while(tasks_ongoing >= tasks_read){
            pthread_cond_wait(&cond,&lock1) ;
            pthread_mutex_lock(&lock_com) ;
            if(end && tasks_completed == tasks_read){
                pthread_mutex_unlock(&lock_com) ;
                pthread_mutex_unlock(&lock1) ;
                return NULL ;
            }
            pthread_mutex_unlock(&lock_com) ;
        }
        pthread_mutex_lock(&lock_com) ;
        if(end && (tasks_completed == tasks_read)){
            pthread_mutex_unlock(&lock_com) ;
            pthread_mutex_unlock(&lock1) ;
            break ;
        }
        pthread_mutex_unlock(&lock_com) ;
        int task_num = tasks_ongoing ;
        tasks_ongoing++ ;
        pthread_mutex_unlock(&lock1) ;
        processtask(tasks[task_num]) ;    
    }
    // printf("Thread Returned     1\n") ;
    // pthread_mutex_lock(&lock_exit) ;
    // while(!exit_threads){
    //     pthread_cond_wait(&cond_exit,&lock_exit) ;
    // }
    // pthread_mutex_unlock(&lock_exit) ;
    printf("Thread Returned     2\n") ;
}

void processtask(long number)
{
    // simulate burst time
    sleep(number);
    // update global variables
    pthread_mutex_lock(&lock_sum) ;
    sum += number;
    pthread_mutex_unlock(&lock_sum) ;
    if (number % 2 == 1)
    {
        pthread_mutex_lock(&lock_odd_even) ;
        odd++;
        pthread_mutex_unlock(&lock_odd_even) ;
    }
    else
    {
        pthread_mutex_lock(&lock_odd_even) ;
        even++;
        pthread_mutex_unlock(&lock_odd_even) ;
    }
    if (number < min)
    {
        pthread_mutex_lock(&lock_min) ;
        min = number;
        pthread_mutex_unlock(&lock_min) ;
    }
    if (number > max)
    {
        pthread_mutex_lock(&lock_max) ;
        max = number;
        pthread_mutex_unlock(&lock_max) ;
    }
    pthread_mutex_lock(&lock_com) ;
    tasks_completed++;
    pthread_cond_signal(&cond_com) ;
    pthread_mutex_unlock(&lock_com) ;
    printf("Task completed\n") ;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: sum <infile> <number of worker threads>\n");
        exit(EXIT_FAILURE);
    }
    int threadNum = atoi(argv[2]) ;
    char *fn = argv[1];
    // Read from file
    FILE *fin = fopen(fn, "r");
    long t;
    fscanf(fin, "%ld\n", &t);
    printf("The number of tasks are : %ld \n", t);
    tasks = malloc(t*(sizeof(long))) ;
    char type;
    long num;
    pthread_t * th = (pthread_t *)calloc(threadNum, sizeof(pthread_t));
    for(int k = 0 ; k < threadNum ; k++){
        if (pthread_create(th + k, NULL, &do_task,NULL) != 0) {
                perror("Failed to create thread");
                return 1;
        }
        printf("Thread created\n") ;
    }
    while (fscanf(fin, "%c %ld\n", &type, &num) == 2)
    {
        if (type == 'p')
        { // processing task
            printf("Reading task \n") ;
            //pthread_mutex_lock(&lock1) ;
            tasks[tasks_read] = num ;
            tasks_read++ ;
            pthread_cond_signal(&cond) ;
            //pthread_mutex_unlock(&lock1) ;
            printf("Task added to queue\n");
        }
        else if (type == 'w')
        { // waiting period
            sleep(num);
            printf("Wait Over\n");
        }
        else
        {
            printf("ERROR: Type Unrecognizable: '%c'\n", type);
            exit(EXIT_FAILURE);
        }
    }
    fclose(fin);
    end = 1 ;
    pthread_t comp ;
    if (pthread_create(&comp, NULL, &task_complete,NULL) != 0) {
                perror("Failed to create thread");
                return 1;
    }
    if (pthread_join(comp, NULL) != 0) {
            return 2;
    }
    // pthread_mutex_lock(&lock_exit) ;
    // exit_threads = 1 ;
    // pthread_cond_broadcast(&cond_exit) ;
    // pthread_mutex_unlock(&lock_exit) ;
    for(int k = 0 ; k < threadNum ; k++){
        printf("Thread %d - %lu\n",k,th[k]) ;
        if (pthread_join(th[k], NULL) != 0) {
            return 2;
        }
        printf("Thread %d has joined\n",k) ;
    }
    // Print global variables
    printf("%ld %ld %ld %ld %ld\n", sum, odd, even, min, max);

    return (EXIT_SUCCESS);
}
