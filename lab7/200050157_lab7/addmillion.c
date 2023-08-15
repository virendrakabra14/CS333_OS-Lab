#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

int account_balance = 0;
pthread_mutex_t m;

void* increment(void* num_millions) {
    for(int j=0; j<*(int*)num_millions; j++) {  // each deposit is of a million
        pthread_mutex_lock(&m);         // lock outside this loop means no parallelism!
        for (int i = 0; i < 1000000; i++) {
            account_balance++;
        }
        pthread_mutex_unlock(&m);
    }
}

int main(int argc, char* argv[]) {

    clock_t t;
    t = clock();
    
    if (argc<2) {
        printf("Enter number of parameters as cmd-line arg.\n");
        exit(100);
    }
    
    // int threadNum = 10;
    int threadNum = atoi(argv[1]);
    int num_millions = (2048)/threadNum;


    pthread_t th[threadNum];
    int i;
    for (i = 0; i < threadNum; i++) {
        if (pthread_create(th + i, NULL, &increment, &num_millions) != 0) {
            perror("Failed to create thread");
            return 1;
        }
        printf("Transaction %d has started\n", i);
    }
    for (i = 0; i < threadNum; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            return 2;
        }
        printf("Transaction %d has finished\n", i);
    }
    printf("Account Balance is : %d\n", account_balance);

    t = clock() - t;
    double time_taken = 1000*((double)t)/CLOCKS_PER_SEC;     // milliseconds
    printf("Time spent: %f ms\n", time_taken);

    return 0;
}