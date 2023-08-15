#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <unistd.h>     // write, STDOUT_FILENO

void sig_handler(int signum) {
    // printf("Received signal %d\n", signum);          // man signal-safety
    
    char* msg = "Received signal\n";
    write(STDOUT_FILENO, msg, 17);
}

int main() {

    /*
    signal()
    It accepts two arguments, first signal number (one of those SIGsomething) and second pointer to a signal handler function. Signal handler function returns void and accepts single integer argument that represents a signal number that has been sent. This way you can use the same signal handler function for several different signals.
    */

    signal(SIGINT, sig_handler);        // #define SIGINT 2
    // signal(SIGINT, SIG_IGN);

    printf("Remaining time: %d\n", sleep(100));
    // while(1);

    /*
    man signal:
        The sleep(3) function is also never restarted if interrupted by a
        handler, but gives a success return: the number of seconds
        remaining to sleep.
    
    also, man 3 sleep
    */

    printf("Done\n");

    return 0;

}