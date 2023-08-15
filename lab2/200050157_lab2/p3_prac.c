#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

    // just exec

    char exec_name[51];

    printf(">>> ");
    scanf("%s", exec_name);

    char* args[] = {exec_name, NULL};

    execv(exec_name, args);     // this process will get replaced...
    
    printf("done\n");           // NOT printed (if no error)

}