#include "types.h"
#include "stat.h"
#include "user.h"

char buf[1];

void head(int fd, int cnt) {
    int n;
    while((n = read(fd, buf, 1)) > 0) {     // read 1 byte at a time
        if (cnt <= 0) break;
        if (buf[0] == '\n') cnt--;          // decrease cnt at newline
        if (write(1, buf, n) != n) {
            printf(1, "head: write error\n");
            exit();
        }
    }
    if(n < 0){
        printf(1, "head: read error\n");
        exit();
    }
}

int main(int argc, char** argv) {

    int fd, i;

    if(argc <= 2) {

        /*
            cat --help on linux:
                With no FILE, or when FILE is -, read standard input.
            Similar effect seen in cat.c here: cat(0) -- reads stdin (and prints the same) (fd=0)

            Here, we read stdin for 0 lines
        */

        head(0,0);
        // printf(1, "Usage: head COUNT FILE[S]\n");        // printf of xv6
        exit();
    }

    int cnt = atoi(argv[1]);
    for(i=2; i<argc; i++) {
        if((fd = open(argv[i], 0)) < 0){
            printf(1, "head: cannot open %s\n", argv[i]);
            exit();
        }
        printf(1, "--- %s ---\n", argv[i]);
        head(fd, cnt);
        close(fd);
    }

    exit();         // necessary to call exit()
}
