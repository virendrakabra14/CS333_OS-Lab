#include <stdio.h>
#include <stdlib.h>		// for exit(...)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>	// wait

int main() {

	pid_t pid = fork();

	// fork() returns a zero to the newly created child process
	// fork() returns a positive value, the process ID of the child process, to the parent

	// After a new child process is created, both processes will execute the next instruction following the fork() system call.

	if (pid < 0) {		// fork unsuccessful
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) {
		printf("Child : My process ID is:%d\n", getpid());		// int is ok, since "cat /proc/sys/kernel/pid_max" ~ 4 million
		printf("Child : The parent process ID is:%d\n", getppid());
	}
	else {
		printf("Parent : My process ID is:%d\n", getpid());
		printf("Parent : The child process ID is:%d\n", pid);
		// sleep(1);		// otherwise, parent exits before child, 
	}

	// instead of sleep, make parent wait until child exits:

	int status = 0;
	pid_t wpid;
	while ((wpid = wait(&status)) > 0);		// https://stackoverflow.com/a/23872806

	// or, can use: while(wait(NULL) > 0)
	// https://stackoverflow.com/questions/42426816/how-does-waitnull-exactly-work

	// ** p1_prac.c

}
