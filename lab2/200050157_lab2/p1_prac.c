#include <stdio.h>
#include <stdlib.h>		// for exit(...)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>	// wait

int main() {

	pid_t pid = fork();

	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) {
		printf("Child : My process ID is:%d\n", getpid());
		printf("Child : The parent process ID is:%d\n", getppid());
		exit(7);
	}
	else {
		printf("Parent : My process ID is:%d\n", getpid());
		printf("Parent : The child process ID is:%d\n", pid);
	}

	int status = 0;
	pid_t wpid;
	// wpid = wait(&status);		// https://stackoverflow.com/a/23872806
	
	// printf("Exit = %d, child = %d\n", WEXITSTATUS(status), wpid);

	// ** nice wait example: https://man7.org/linux/man-pages/man2/wait.2.html
	do {
		wpid = waitpid(pid, &status, WUNTRACED | WCONTINUED);
		if (wpid == -1) {
			perror("waitpid");
			exit(EXIT_FAILURE);
		}

		if (WIFEXITED(status)) {
			printf("exited, status=%d\n", WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			printf("killed by signal %d\n", WTERMSIG(status));
		} else if (WIFSTOPPED(status)) {
			printf("stopped by signal %d\n", WSTOPSIG(status));
		} else if (WIFCONTINUED(status)) {
			printf("continued\n");
		}
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));

}
