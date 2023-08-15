#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void recur(int curr, int max_num) {

    if(curr >= max_num) {
        return;
    }

    pid_t pid = fork();

    if(pid<0) {
        exit(0);
    }
    else if(pid==0) {        // child
        printf("Child %d is created\n", getpid());
        recur(curr+1, max_num);     // note that recur() called here, so dry-run execution from the start again...
        printf("Child %d of parent %d exited\n", getpid(), getppid());
        exit(0);
    }
    else {
        while(wait(NULL)>0);
    }

}

void recur2(int curr, int max_num) {

    if(curr >= max_num) {
        return;
    }

    pid_t pid = fork();

    if(pid<0) {
        exit(0);
    }
    else if(pid==0) {
        // just call recur2() here...
        // printing done when the process is parent. [or, printing done by parent...]
        recur2(curr+1, max_num);
    }
    else {
        printf("Child %d is created\n", pid);       // pid is of child
        while(wait(NULL)>0);
        printf("Child %d of parent %d exited\n", pid, getpid());
        if(curr==0) {
            printf("Parent exited\n");
        }
    }


}

int main(int argc, char** argv) {

    if(argc<2) {
        printf("Provide n as command line argument\n");
        exit(0);
    }

    int n = atoi(argv[1]);

    printf("Parent is : %d\n", getpid());
    printf("Number of children: %d\n", n);

    /* earlier:

    recur(0,n);
    printf("Parent exited\n");

    // */

    // can just call recur2()
    recur2(0,n);

}