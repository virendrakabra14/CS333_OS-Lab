#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<fcntl.h>

/*

man read

On  success,  the  number  of bytes read is returned (zero indicates end of file), and the
file position is advanced by this number.  It is not an error if this  number  is  smaller
than  the  number  of bytes requested; this may happen for example because fewer bytes are
actually available right now (maybe because we were close to end-of-file,  or  because  we
are  reading from a pipe, or from a terminal), or because read() was interrupted by a sig‐
nal.

*/

int main() {

    char input_file[101];      // 100 chars limit
    char output_file[101];

    char buffer[101];

    read(STDIN_FILENO, input_file, sizeof(input_file));        // it's: #define STDIN_FILENO 0
    read(STDIN_FILENO, output_file, sizeof(output_file));

    // ** have to do this ↓

    int i = 0;
    while(input_file[i]!='\n' && input_file[i]!='\0') {
        i++;
    }
    input_file[i] = '\0';       // replace '\n' by '\0'

    i = 0;
    while(output_file[i]!='\n' && output_file[i]!='\0') {
        i++;
    }
    output_file[i] = '\0';

    /* man open:
        creat()
        A call to creat() is equivalent to calling open() with flags
        equal to O_CREAT|O_WRONLY|O_TRUNC.

        ---

        The argument flags must include one of the following access
        modes: O_RDONLY, O_WRONLY, or O_RDWR.  These request opening the
        file read-only, write-only, or read/write, respectively.
    */

    int f_in = open(input_file, O_RDONLY);        // f_in is file descriptor (fd)
    int f_out = open(output_file, O_CREAT|O_WRONLY|O_TRUNC);

    if(f_in==-1) {
        printf("not f_in\n");
        exit(1);
    }
    if(f_out==-1) {
        printf("not f_out\n");
        exit(1);
    }


    // NOTE: do read, write char-by-char [byte-by-byte]
    // otherwise, garbage in buffer can be written to f_out

    while(read(f_in, buffer, 1) > 0) {      // 0 return value from 'read' indicates eof [see above (man page)]
        // for read(..., 100): may read < 100 too... but advances the file position by number returned by 'read'
        
        write(f_out, buffer, 1);
    }

    close(f_in);
    close(f_out);

}