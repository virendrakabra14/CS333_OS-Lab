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

        printf("Child : Check child process state\n");
        printf("Child : Press Enter to continue\n");
        getchar();                                                  // STAT: S+

        printf("Child : Check child process state again\n");        // STAT: Z+
        /* Reason in lab pdf:
            A process that has completed its execution or terminated but still has some
            state (pid, memory allocation, stack, etc.) in the memory and has not been cleaned-up (reaped) is called
            a zombie process.
            A zombie process is reaped when the parent process executes the wait system call or
            when the parent process exits.
        */

    }
    else {
        printf("Parent : My process ID is: %d\n", getpid());
        printf("Parent : The child process ID is: %d\n", pid);
        sleep(60);
        while(wait(NULL)>0);
    }

}