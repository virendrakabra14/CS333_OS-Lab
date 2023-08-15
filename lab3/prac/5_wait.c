#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {

    int status;
    pid_t wpid;

    if(fork()==0) {
        sleep(5);
        printf("Child1 %d: after sleep\n", getpid());
        exit(7);
    }
    else {
        wpid = waitpid(-1, &status, WNOHANG);     // doesn't wait (hang)...
        printf("%d, %d\n", wpid, status);
    }

    if(fork()==0) {
        printf("Child2 %d: no sleep\n", getpid());
        exit(7);
    }
    else {
        sleep(1);
        wpid = waitpid(-1, &status, WNOHANG);       // child2 terminated due to sleep, so this works √
        printf("%d, %d\n", wpid, status);
    }

}