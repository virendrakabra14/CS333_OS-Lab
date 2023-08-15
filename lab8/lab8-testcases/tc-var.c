#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int VAR = 0;

void increment(int *thread_rank) {
    VAR++;
    printf(1, "Thread Rank: %d, VAR: %d\n", *thread_rank, VAR);
    exit();
}

int main(int argc, char *argv[]) {

    printf(1, "Calling Process Print VAR value: %d\n", VAR);
    int N = 5;

    for(int i=0;i<N;i++) {
        create_thread(increment, &i);
        sleep(100);
    }

    for(int i=0;i<N;i++) {
        join();
    }

    printf(1, "All threads joined, VAR value: %d\n", VAR);
    exit();
}