#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
#include <sys/wait.h>

// https://www.geeksforgeeks.org/pipe-system-call/

int main() {

    int p1[2];
    int p2[2];

    if(pipe(p1)<0) {        // pipe
        perror("pipe");
        exit(1);
    }
    if(pipe(p2)<0) {
        exit(1);
    }

    int x, y, x1, y1;

    pid_t pid_a = fork();

    if(pid_a<0) {
        exit(1);
    }
    else if(pid_a==0) {
        printf("Process A : Input value of x : ");
        scanf("%d", &x);
        
        close(p1[0]);           // man: close unused read end       // worked fine without this too...
        write(p1[1], &x, sizeof(x));

        exit(0);
    }

    while(wait(NULL)>0);        // ** necessary [otherwise all children ~ simultaneously]

    pid_t pid_b = fork();

    if(pid_b<0) {
        exit(1);
    }
    else if(pid_b==0) {
        printf("Process B : Input value of y : ");
        scanf("%d", &y);
        
        close(p2[0]);
        write(p2[1], &y, sizeof(y));
        
        exit(0);
    }

    while(wait(NULL)>0);

    pid_t pid_c = fork();

    if(pid_c<0) {
        exit(1);
    }
    else if(pid_c==0) {
        printf("Process C : Result after addition : ");

        close(p1[1]);
        read(p1[0], &x1, sizeof(x1));
        
        close(p2[1]);
        read(p2[0], &y1, sizeof(y1));

        // printf("%d %d\n", x1, y1);       // debugging

        printf("%d\n", x1+y1);
        exit(0);
    }

    while(wait(NULL)>0);

    /* done above:
        - close (unused) read end before writing
        - close (unused) write end before reading
     */
    
    // https://unix.stackexchange.com/questions/693780/closing-different-ends-in-a-pipe

}