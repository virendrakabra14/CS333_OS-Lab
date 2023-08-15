#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

    char* cmd = "ls";
    char* args[] = {"ls", "-l", NULL};             // "ls" necessary here too...

    pid_t pid = fork();

    if(pid==0) {
        execlp("ls", "ls", "-l", NULL);             // "ls" twice √
        exit(EXIT_SUCCESS);     // exit(0)
    }
    else {
        waitpid(pid, NULL, WUNTRACED|WCONTINUED);
        // execvp(cmd, args);
    }

    // https://stackoverflow.com/a/21559498
    // execl("/bin/ls", "ls", "-la", NULL);
    // execlp("ls", "ls", "-la", NULL);

}