#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void sig_handler(int signum) {
    printf("Received signal %d\n", signum);
}

int main() {

    /*
    signal()
    It accepts two arguments, first signal number (one of those SIGsomething) and second pointer to a signal handler function. Signal handler function returns void and accepts single integer argument that represents a signal number that has been sent. This way you can use the same signal handler function for several different signals.
    */

    signal(SIGINT, sig_handler);        // #define SIGINT 2
    
    sleep(100);                         // this won't be completed after interruption [ASK?]
    // while(1);

    printf("After sleep (and ^C)\n");

    return 0;

}