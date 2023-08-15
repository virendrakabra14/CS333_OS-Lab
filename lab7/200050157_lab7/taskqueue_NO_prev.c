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

struct task{
    char type;
    long time;
    bool done;
};
struct task* arr_tasks;
long num_tasks;
long index_into_arr;
pthread_mutex_t m, tmp_mut;
pthread_cond_t wait_task, wait_done;
bool waiting_thread_started;

void processtask(long number);

void processtask(long number)
{
    // simulate burst time
    sleep(number);

    // update global variables
    sum += number;
    if (number % 2 == 1)
    {
        odd++;
    }
    else
    {
        even++;
    }
    if (number < min)
    {
        min = number;
    }
    if (number > max)
    {
        max = number;
    }
}

void* do_task() {
    int curr_task = -1;
    while(index_into_arr<num_tasks) {
        curr_task = -1;
        pthread_mutex_lock(&m);
        printf("enter do_task, %ld\n", index_into_arr);
        if(index_into_arr<num_tasks && arr_tasks[index_into_arr].type!='\0' && arr_tasks[index_into_arr].done==0) {
            // processtask(arr_tasks[index_into_arr].time);     // no parallelism!!
            curr_task = index_into_arr;
            arr_tasks[index_into_arr].done = 1;
            index_into_arr++;
        }
        else if(index_into_arr<num_tasks && arr_tasks[index_into_arr].type=='\0' /*&& arr_tasks[index_into_arr].done==0*/) {
            arr_tasks[index_into_arr].done = 1;
            pthread_cond_signal(&wait_task);
            pthread_cond_wait(&wait_done,&m);       // what if another worker starts running now??
        }
        pthread_mutex_unlock(&m);
        if(curr_task!=-1) {
            processtask(arr_tasks[curr_task].time);
        }
        printf("exit do_task, %ld\n", index_into_arr);
    }
    printf("reached here\n");
    pthread_cond_signal(&wait_task);
}

void* do_wait() {
    while(index_into_arr<num_tasks) {
        // printf("enter do_wait, %ld\n", index_into_arr);
        if(!waiting_thread_started) waiting_thread_started = 1;
        if(index_into_arr<num_tasks) {
            // printf("wait in wait\n");
            pthread_cond_wait(&wait_task, &tmp_mut);
        }
        pthread_mutex_lock(&m);
        if(index_into_arr<num_tasks && arr_tasks[index_into_arr].type=='\0' /*&& arr_tasks[index_into_arr].done==0*/) {
            sleep(arr_tasks[index_into_arr].time);
            arr_tasks[index_into_arr].done = 1;
            index_into_arr++;
            printf("exit do_wait, %ld\n", index_into_arr);
            pthread_mutex_unlock(&m);
            pthread_cond_signal(&wait_done);
        }
        else {
            pthread_mutex_unlock(&m);
        }
    }
    pthread_cond_broadcast(&wait_done);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: sum <infile> <num_threads>\n");
        exit(EXIT_FAILURE);
    }

    char *fn = argv[1];
    // Read from file
    FILE *fin = fopen(fn, "r");
    fscanf(fin, "%ld\n", &num_tasks);
    printf("The number of tasks are : %ld \n", num_tasks);

    // array of tasks
    arr_tasks = (struct task*)malloc(num_tasks*sizeof(struct task));

    char type;
    long num;
    index_into_arr = 0;
    struct task tmp_task;
    while (fscanf(fin, "%c %ld\n", &type, &num) == 2)
    {
        if (type == 'p') {
            arr_tasks[index_into_arr].type = type;
            arr_tasks[index_into_arr].time = num;
            arr_tasks[index_into_arr].done = 0;
        }
        else if (type == 'w') {
            arr_tasks[index_into_arr].type = '\0';
            arr_tasks[index_into_arr].time = num;
            arr_tasks[index_into_arr].done = 0;
        }
        else {
            printf("ERROR: Type Unrecognizable: '%c'\n", type);
            exit(EXIT_FAILURE);
        }
        index_into_arr++;
    }
    fclose(fin);

    num_tasks = index_into_arr;       // actual num_tasks
    index_into_arr = 0;

    for(int i=0; i<num_tasks; i++) {
        printf("%c, %ld, %d\n", arr_tasks[i].type, arr_tasks[i].time, arr_tasks[i].done);
    }

    // create all threads

    pthread_t waiting_thread;
    waiting_thread_started = 0;
    pthread_create(&waiting_thread, NULL, do_wait, NULL);
    while(!waiting_thread_started);

    int max_num_threads = atoi(argv[2]);
    pthread_t pool[max_num_threads];
    // bool is_aval[max_num_threads];
    // memset(is_aval, 1, sizeof(bool)*max_num_threads);       // memset: byte-by-byte basis
                                                        // since bool, so '1' works here

    for(int i=0; i<max_num_threads; i++) {
        pthread_create(pool+i, NULL, do_task, NULL);
    }

    printf("Created pool of threads\n");

    for(int i=0; i<max_num_threads; i++) {
        pthread_join(pool[i], NULL);
    }
    pthread_join(waiting_thread, NULL);

    // Print global variables
    printf("%ld %ld %ld %ld %ld\n", sum, odd, even, min, max);

    return (EXIT_SUCCESS);
}
