#include <stdio.h>

#include <sys/types.h>               // man 2 kill
#include <signal.h>

#include <stdlib.h>

#include <unistd.h>     // sleep


int main(int argc, char** argv) {

    if(argc<2) {
        printf("Give pid as command line argument\n");
        exit(0);
    }

    pid_t pid = atoi(argv[1]);

    // https://www.csl.mtu.edu/cs4411.ck/www/NOTES/signal/kill.html

    kill(pid, SIGINT);
    printf("SIGINT signal sent to PID: %d\n", pid);
    sleep(5);

    kill(pid, SIGTERM);
    printf("SIGTERM signal sent to PID: %d\n", pid);
    sleep(5);
    
    kill(pid, 9);       // SIGKILL: 9
    printf("SIGKILL (9) signal sent to PID: %d\n", pid);

}