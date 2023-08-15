#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

    int n; scanf("%d", &n);

    pid_t pid = fork();

    if(pid<0) {
        exit(0);
    }
    else if(pid==0) {       // 0 for child
        for(int i=1; i<=n; i++) {
            printf("C %d %d\n", getpid(), i);
        }
    }
    else {
        while(wait(NULL)>0);        // wait until all child processes finish
                                    // https://stackoverflow.com/questions/42426816/how-does-waitnull-exactly-work
                                    // https://stackoverflow.com/a/20670389
                                    // If the current process have no child processes, wait(NULL) returns -1

                                    // can also wait for this child using waitpid(pid, &status, WUNTRACED | WCONTINUED)

        for(int i=n+1; i<=2*n; i++) {
            printf("P %d %d\n", getpid(), i);
        }
    }

}