/* tour.c
* To learn the basic signal handling in c
* Can send the interrupt signal via Ctrl-c in a terminal.
*
* Complete TODO items for this question
*/

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#define NUM_FRIEND 5 // Number of friends visiting the Dinosaurs Park

int friend_count = 0; // Current count of friends completed the tour

// TODO: Add a signal handling function which handle SIGINT and SIGCHLD signals

void SigHandler(int sig_num) {
    if(sig_num == SIGINT) {
        printf("You have interrupted the tour.\n");

        if(friend_count==NUM_FRIEND) {
            printf("All %d friends have completed the tours.\n", NUM_FRIEND);
            printf("Thank you for visiting the Dinosaurs park\n");
            
            // make SIGINT behavior to default, and then call kill() --
            signal(SIGINT, SIG_DFL);        // default (terminate)
            kill(getpid(), SIGINT);
            // or use SIGKILL

        }
        else {
            printf("Oh Sorry! Only %d out of the %d friends have completed the tour.\n", friend_count, NUM_FRIEND);
        }
    }
    else if(sig_num == SIGCHLD) {
        friend_count++;             // also, this example: https://stackoverflow.com/a/2708512
                                    // to determine pid of child, use waitpid: https://stackoverflow.com/questions/2595503/determine-pid-of-terminated-process
        
        // if not wait, then this child turns into zombie (bcoz not reaped), and reaped on parent's exit...
        // could also do while(wait(NULL)>0) in main()
        waitpid(-1, NULL, WNOHANG);         // WNOHANG, since we know that a child has terminated bcoz of SIGCHLD
            // return value is the child's pid
    }
}

// END TODO
int main(int argc, char *argv[])
{

    printf("Welcome to the Dinosaurs Park.\n");

    /* 
    * TODO: add struct/calls to sigaction() to handle SIGINT and SIGCHLD. 
    * Use sigaction() and associated structs.
    * Ensure that the flag SA_RESTART is set as well to ensure system calls are automatically restarted.
    */
    
    // https://stackoverflow.com/a/5113641

    struct sigaction sgac;
    sgac.sa_handler = SigHandler;
    sgac.sa_flags = SA_RESTART;             // <-- IMPORTANT
    sigaction(SIGINT, &sgac, NULL);
    sigaction(SIGCHLD, &sgac, NULL);

    // END TODO
    printf("The Process ID of Dinosaurs Park: %d \n", getpid());
    for (size_t friend = 1; friend <= NUM_FRIEND; friend++)
    {
        if (fork() == 0)
        {
            //TODO Note that, you must carefully place the various children in different process groups 
            // that is different from the default process group of its parent
            //say, using the setpgid system call.

            setpgid(getpid(), 0);           // IMPORTANT -- Ctrl+C sent to all processes in the process group
                                            // ** see what happens on commenting this.. [all child processes stopped abruptly]
                                            // can also just do: signal(SIGINT,SIG_IGN) in all children...
            /*
            https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-setpgid-set-process-group-id-job-control
            If pgid is zero, the system uses the PID of the process indicated by pid as the ID for the *new* process group.
            The new group is created in the caller's session.
            */

            sleep(5 * friend);             // sleep emulates "touring" time
            printf("Friend #%zu with process ID - %d has completed the tour.\n", friend, getpid());
            
            // friend_count = friend_count + 1;     // won't work: https://stackoverflow.com/a/59553695
                                                    // Processes must use inter-process communication (IPC) mechanisms to talk to each
                                                    // other, such as: pipes, signals, ... [here SIGCHLD used]

            exit(0);
            //END TODO
        }
    }

    for(;;)pause(); // wait for all friends to complete tour
    return 0;
}
