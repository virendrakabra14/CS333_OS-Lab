#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void main() {
    sigset_t set, oset, pset;

    sigemptyset( &set );
    sigaddset( &set, SIGINT );
    sigprocmask( SIG_BLOCK, &set, &oset );
    printf( "Old set was %8.8ld.\n", oset );

    sigpending( &pset );
    printf( "Pending set is %8.8ld.\n", pset );

    kill( getpid(), SIGINT );

    sigpending( &pset );
    printf( "Pending set is %8.8ld.\n", pset );

    sigprocmask( SIG_UNBLOCK, &set, &oset );

    /* The program terminates with a SIGINT */
}

// *** ---> https://stackoverflow.com/a/25281
// https://stackoverflow.com/questions/25261/set-and-oldset-in-sigprocmask

// ** 4_1.c