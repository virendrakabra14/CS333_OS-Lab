#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char** argv) {

    if(argc<2) {
        printf(1, "Usage: cmd COMMAND [ARGS]\n");
        exit();
    }

    int pid = fork();

    if(pid==0) {
        exec(argv[1], argv+1);
        exit();     // won't reach here
    }
    else {
        wait();
    }

    exit();         // IMPORTANT


}