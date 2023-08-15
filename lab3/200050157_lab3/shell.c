#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

// TODO: YOUR CODE HERE
// INITIALIZE DATA STRUCTURE TO STORE PIDS OF PROCESSES TO KILL LATER			// used wait below...

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line) {

    // tokenizer variables
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

    // loop on length of line
	for(i=0; i < strlen(line); i++){

		char readChar = line[i];

        // tokenize on any kind of space
		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0) {
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		}
		else {
			token[tokenIndex++] = readChar;
		}
	}
	
	free(token);
	tokens[tokenNo] = NULL ;
	return tokens;
}

// TODO
// MAKE FUNCTIONS AS REQUIRED TO AVOID REPETITIVE WORK

void signal_handler(int sig_num) {
	if(sig_num==SIGINT) return;
}


int main(int argc, char* argv[]) {

	signal(SIGINT, signal_handler);

	char  line[MAX_INPUT_SIZE];            
	char  **tokens;           

	// TODO: YOUR CODE HERE
	// INITIALIZE GLOBAL DATA STRUCTURE OF PIDS TO SOME DEFAULT VALUE  
	// INITIALIZE OTHER VARIABLES AS NEEDED

	while(1) {	

		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);		// e.g., "cd home" -> ["cd", "home"]

		// You can maintain boolean variables to check which type of command is given by user and then
        // conditionally execute depending on them

        // TODO: YOUR CODE HERE
        //

		if(tokens[0]==NULL) continue;				// single ENTER pressed

		int args_count = 0;
		while(tokens[args_count]!=NULL) args_count++;

		int cnt = 0, prev_cnt = -1;

		// printf("args cnt: %d\n", args_count);

		while(cnt < args_count) {

			// printf("prev, cnt: %d, %d\n", prev_cnt, cnt);

			// command to be executed in tokens[prev_cnt+1 ... cnt-1]

			if(strcmp(tokens[cnt],"&&")==0 || strcmp(tokens[cnt],"&&&")==0 || cnt==args_count-1) {

				if(cnt==args_count-1) cnt++;			// for uniformity in "[prev_cnt+1 ... cnt-1]"

				if(strcmp(tokens[prev_cnt+1],"exit")==0) {		// exit
					while(wait(NULL)>0);
					exit(0);
				}

				else if(strcmp(tokens[prev_cnt+1],"cd")==0) {
					if(cnt-prev_cnt-1 != 2) {
						printf("Number of arguments is incorrect\n");
						continue;
					}
					else {
						if(chdir(tokens[cnt-1])==-1) {						// man chdir
							printf("Command failed\n");
						}
					}
				}

				else {

					pid_t pid_child = fork();

					if(pid_child==0) {

						char** tokens_tmp = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
						for(int i=0; i<cnt-prev_cnt-1; i++) {
							tokens_tmp[i] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
							strcpy(tokens_tmp[i], tokens[prev_cnt+1+i]);					// ** strcpy
						}
						tokens_tmp[cnt-prev_cnt-1] = NULL;			// necessary to make last ptr NULL (for execv(p))

						if (execvp(tokens[prev_cnt+1], tokens_tmp) < 0) {
							printf("Command failed\n");
						}

						// free up memory
						for(int i=0; tokens_tmp[i]!=NULL; i++) {		// **M of condition
							free(tokens_tmp[i]);
						}
						free(tokens_tmp);

						exit(0);
					}

					else {

						// if &&&, then don't wait here...

						if(cnt>=args_count-1 || strcmp(tokens[cnt],"&&")==0) {		// "||" is short-circuit 'or'

							while(wait(NULL)>0);			// necessary, e.g. sleep 5 -- then need to wait for child to terminate
						}
					}

				}

				prev_cnt = cnt;

			}

			cnt++;

		}

		while(wait(NULL)>0);				// wait for &&& ones here

    
        // freeing the memory
		for(int i=0; tokens[i]!=NULL; i++){
			free(tokens[i]);
		}

		free(tokens);

	}
	return 0;
}


