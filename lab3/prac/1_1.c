#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sig_handler(int signum) {
    printf("Received signal %d\n", signum);
}

int main() {

    signal(SIGINT, sig_handler);
    // signal(SIGINT, SIG_IGN);

    sleep(100);

}