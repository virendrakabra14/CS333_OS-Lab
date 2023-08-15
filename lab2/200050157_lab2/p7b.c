#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char** argv) {

    if(argc<2) {
        printf("Provide n as command line argument\n");
        exit(0);
    }

    int n = atoi(argv[1]);

    printf("Parent is : %d\n", getpid());
    printf("Number of children: %d\n", n);

    pid_t pid;

    for(int i=0; i<n; i++) {
        pid = fork();
        // child process starts executing after "fork()"
        if(pid==0) {
            sleep(1);
            printf("Child %d of parent %d exited\n", getpid(), getppid());
            exit(0);
        }
        else {
            printf("Child %d is created\n", pid);
        }
    }

    while(wait(NULL)>0);

    printf("Parent exited\n");

    /* can also use:
	while ((wpid = wait(NULL)) > 0) {
		fprintf(stdout, "Child %d of parent %d exited\n", wpid, getppid());
	}
    // */

}