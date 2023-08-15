#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_Q_SIZE 10000
long* arr;

// Global variables
long sum = 0;
long odd = 0;
long even = 0;
long min = INT_MAX;
long max = INT_MIN;

// added

int write_index = 0;
bool still_writing = 1;
pthread_cond_t work_present = PTHREAD_COND_INITIALIZER;

int read_index = 0;
pthread_mutex_t read_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t data = PTHREAD_MUTEX_INITIALIZER;


void processtask(long number);

void processtask(long number)
{
    // simulate burst time
    sleep(number);

    // update global variables
    pthread_mutex_lock(&data);          // added lock
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
    pthread_mutex_unlock(&data);
    printf("Completed task for time %ld\n", number);
}

// added

void* do_work(void* arg) {
    int tmp_read_index;
    while(read_index <= write_index || still_writing) {
        pthread_mutex_lock(&read_lock);
        while(read_index == write_index && still_writing) {
            pthread_cond_wait(&work_present, &read_lock);
        }       // will proceed only after locking (after being woken up)
        if(read_index < write_index) {
            tmp_read_index = read_index;
            read_index++;
            pthread_mutex_unlock(&read_lock);
            processtask(arr[tmp_read_index]);       // using a tmp variable as it's possible that other threads
                                                    // have incremented read_index by now
                                                    // (e.g., if this thread is descheduled right after unlocking)

                                                    // ofc, processtask() should be called without lock...
                                                    // otherwise, no parallelism!!
        }
        else {      // threads will execute this if all work has already been taken up
                    // (i.e., read_index==write_index)
                    // AND, still_writing is false
            pthread_mutex_unlock(&read_lock);
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: sum <infile> <number of threads>\n");
        exit(EXIT_FAILURE);
    }
    
    arr = malloc(sizeof(long)*MAX_Q_SIZE);

    int num_threads = atoi(argv[2]);
    pthread_t pool[num_threads];
    for(int i=0; i<num_threads; i++) {
        pthread_create(&pool[i], NULL, &do_work, NULL);
    }

    char *fn = argv[1];
    // Read from file
    FILE *fin = fopen(fn, "r");
    long t;
    fscanf(fin, "%ld\n", &t);
    printf("The number of tasks are : %ld \n", t);

    char type;
    long num;
    while (fscanf(fin, "%c %ld\n", &type, &num) == 2)
    {
        if (type == 'p')
        {
            // assuming #tasks <= MAX_Q_SIZE...

            // main() is the only one modifying the variable write_index
            // so no lock required
            arr[write_index] = num;
            write_index++;
            pthread_cond_signal(&work_present);     // will be lost if no one's "waiting" for work
                                                    // but the while loop in do_work() ensures that
                                                    // work is performed √
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

    still_writing = 0;
    pthread_cond_broadcast(&work_present);      // broadcast,
                                                // since some threads might be waiting on work_present
                                                // (and aren't aware of the change in "still_writing")

    for(int i=0; i<num_threads; i++) {
        pthread_join(pool[i], NULL);
    }

    // Print global variables
    printf("%ld %ld %ld %ld %ld\n", sum, odd, even, min, max);


    return (EXIT_SUCCESS);
}
