#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

int account_balance = 0;
pthread_mutex_t lock ;

void* increment(void* nm) {
    int n = *((int *) nm) ;
    for(int j = 0 ; j < n ; j++){
        pthread_mutex_lock(&lock) ;
        for (int i = 0; i < 1000000; i++) {
          account_balance++;
        }
        pthread_mutex_unlock(&lock) ;
    }
    
}

int main(int argc, char* argv[]) {
    clock_t t = clock() ;
    int threadNum = atoi(argv[1]) ;
    pthread_t th[threadNum];
    int nf = 2048/threadNum ;
    int i;
    for (i = 0; i < threadNum; i++) {
        if (pthread_create(th + i, NULL, &increment, &nf) != 0) {
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
    t = clock()-t ;
    printf("Account Balance is : %d\n", account_balance);
    printf("Time spent: %f ms\n",1000*(((double)t)/CLOCKS_PER_SEC)) ;
    return 0;
}