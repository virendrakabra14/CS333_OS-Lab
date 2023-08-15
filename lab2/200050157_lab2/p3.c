#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

    // fork and exec
    // Copy-on-write optimization

    char exec_name[51];        // limited by 50 chars [last is null-char]

    pid_t pid;

    // execv vs execvp
    // https://stackoverflow.com/questions/55743496/difference-between-exec-execvp-execl-execv

    // l for list, v for vector(array) -- both must end with NULL
    // p for: look in PATH... used in Lab3 (ls, etc. commands)

    while(1) {
        while(wait(NULL)>0);        // wait for any child process(es) [in particular, the last executed one...]
        printf(">>> ");
        scanf("%s", exec_name);
        pid = fork();
        if(pid==0) {

            char* args[] = {exec_name, NULL};      // ** man exec: 'v' => The array of pointers must be terminated by a null pointer.
            execv(exec_name, args);     // alternative: system(exec_name)
                                            // 'v' for vector (array of args; must end with NULL)
            // execl(exec_name, exec_name, NULL);         // 'l' for list of args... must end with NULL
            exit(0);
        }
    }

    /*
    man:
        The exec() functions return only if an error has occurred. The return value is -1, and errno is set to indicate the error.

    */

}