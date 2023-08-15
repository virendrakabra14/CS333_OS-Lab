#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    // ~ obv pt: keep the below 2 lines BEFORE while(1)...
    // otherwise, never reaches there
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    printf("Process Id is: %d\n", getpid());

    while(1) {
        sleep(3);
        printf("Waiting...\n");
    }

}