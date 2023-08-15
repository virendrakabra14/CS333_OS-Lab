#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"      // required for O_CREATE, etc. defines

#define print() printf(1, "Number of open files: %d\n", numOpenFiles())

int main(int argc,const char** argv){
    
    print();

    int num_files = 5;
    int* fds = malloc(num_files * sizeof(int));
    
    char* filename = "file#";
    char* created_text = "file# created\n";
    char* not_created_text = "file# not created\n";
    char* filetext = "File# : first line\n";

    for(int i=0; i<num_files; i++) {

        char x = i+'0';
        filename[4] = x;
        created_text[4] = x;
        not_created_text[4] = x;
        filetext[4] = x;

        if((fds[i] = open(filename, O_CREATE|O_RDWR)) >= 0) {
            printf(1, created_text);
            write(fds[i], filetext, strlen(filename));           // can do: "cat file1" afterwards
        }
        else {
            printf(1, not_created_text);
            exit();
        }

        print();
    
    }

    for(int i=0; i<num_files; i++) {
        close(fds[i]);

        // delete file as well [otherwise, stays on the disk]
        // The unlink system call (line 6001) removes a path from a file system
        // https://www.cse.iitb.ac.in/~mythili/os/notes/old-lecture-notes/10-xv6-filesystem.pdf
        char x = i+'0';
        filename[4] = x;
        unlink(filename);

        // also:
        // https://pdos.csail.mit.edu/6.828/2012/lec/l-fs.txt
        // https://www.ics.uci.edu/~aburtsev/cs5460/lectures/lecture02-xv6-unix/lecture02-xv6-unix.pdf
        // for filesystem

        // PDOS link:
        /*
            fd = open("x/y", O_CREATE);
            write(fd, "abc", 3);
            link("x/y", "x/z");
            unlink("x/y");
        */
        
        print();
    }
    exit();
}