#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

    int n; scanf("%d", &n);

    pid_t pid = fork();

    if(pid<0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(pid==0) {       // 0 for child
        for(int i=1; i<=n; i++) {
            printf("C %d %d\n", getpid(), i);
        }
    }
    else {
        for(int i=n+1; i<=2*n; i++) {
            printf("P %d %d\n", getpid(), i);
        }
    }

}