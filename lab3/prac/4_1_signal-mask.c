#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// https://stackoverflow.com/a/25281

int main() {

    sigset_t set, oldset;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    sigprocmask(SIG_BLOCK, &set, &oldset);          // blocks signals given in 'set'
                                                    // old mask is stored in oldset

    printf("set: %8.8ld, oldset: %8.8ld\n", set, oldset);

    // while(1);

    sigprocmask(SIG_UNBLOCK, &set, &oldset);

    printf("set: %8.8ld, oldset: %8.8ld\n", set, oldset);

    while(1);

}

// ** signal-mask pdf S5 too..