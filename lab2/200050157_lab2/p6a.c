#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

    pid_t pid = fork();

    if(pid<0) {
        exit(0);
    }
    else if (pid==0) {
        printf("Child : My process ID is: %d\n", getpid());
        printf("Child : The parent process ID is: %d\n", getppid());
        
        sleep(5);

        printf("Child : After sleeping for 5 seconds\n");
        printf("Child : My process ID is: %d\n", getpid());
        printf("Child : The parent process ID is: %d\n", getppid());

        exit(0);

    }
    else {
        printf("Parent : My process ID is: %d\n", getpid());
        printf("Parent : The child process ID is: %d\n", pid);
        sleep(1);       // so that child's first output is correct...

        exit(0);

    }

}