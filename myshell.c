#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "command.h"

#include "myshell.h"


/**
*** ------------- BEGINNING OF SPECIAL COMMANDS -----------
***/

void exit_with_sig(int sig, command_t *comm_list) {
	/*Clean up before*/
	destroy_commands(comm_list);

	printf("Shell exited with signal: %d\n", sig);
	exit(sig);
}

/**
** If path is NULL or ~ go to home
** If path is - go to last working directory
** *Else go to path
**/

void change_dir(char *path) {
	char *homedir = getenv("HOME");
	char *lastdir = getenv("OLDPWD");

	
	if(path == NULL) {
		chdir(homedir);
	}
	else if(strcmp("~", path) == 0) {
		chdir(homedir);
	}
	else if(strcmp("-", path) == 0) {
		chdir(lastdir);
	}
	else{
		chdir(path);
	}
}

/**
*** --------------- END OF SPECIAL COMMANDS -------------
***/


void remove_trailing_space(char *line) {
	char *end = line + strlen(line) - 1;

	while(end > line && *end == ' ') {
		end--;
	}

	*(end+1) = '\0';
}

void execute_comm_list(command_t *comm_list, int num_progs) {
	pid_t wpid, procid;
	int status;
	int num_pipes = num_progs - 1; /*ex. foo | bar <- 1 pipe, 2 progs*/
	int fd[2*num_pipes];
	int executed_count = 0;

	/*Copy list in case exit is called, so exit can cleanup*/
	command_t *list_copy = comm_list;

	for(int i = 0; i < num_pipes; i++) {
		pipe(fd + 2*i);
	}

	while(comm_list != NULL) {
		if(strcmp("exit", comm_list->argv[0]) == 0) {
			int sig = comm_list->argv[1] ? atoi(comm_list->argv[1]) : 0;
			exit_with_sig(sig, list_copy);
		}
		else if(strcmp("cd", comm_list->argv[0]) == 0) {
			change_dir(comm_list->argv[1]);
		}
		else {
			if((procid = fork()) < 0) {
				printf("Unexpected error occurred\n");
				exit(EXIT_FAILURE);
			}
			else if(procid == 0) {
				/*If in child*/

				/*Skip duping stdin on first program*/
				if(executed_count > 0) {
					dup2(fd[2*executed_count - 2], STDIN_FILENO);
				}

				/*Skip duping stdout on last program*/
				if(executed_count < num_progs - 1) {
					dup2(fd[2*executed_count + 1], STDOUT_FILENO);
				}

				/*Child closes all copies of their fd's*/
				for(int i = 0; i < 2*num_pipes; i++){
					close(fd[i]);
	            }

				if(execvp(comm_list->argv[0], comm_list->argv) < 0) {
				    printf("Error: %s program could not be found in your path\n", comm_list->argv[0]);
				    _exit(EXIT_FAILURE);
				}
			}
		}

		comm_list = comm_list->next;
		executed_count++;
	}

	/*Parent closes all copies of their fd's*/
	for(int i=0; i< 2*num_pipes; i++) {
		close(fd[i]);
	}

	/*Wait for all children*/
	while((wpid = wait(&status)) != -1);
}

int main( void ) {
	char *input = NULL;
	size_t read_count = 0;
	token_t *tokens;
	command_t *commands;
	int num_commands;

	/*Prompt user until interrupt, error or quit*/
	while(1) {
		num_commands = 0;

		printf("> ");
		getline(&input, &read_count, stdin);
		input[strlen(input) - 1] = '\0'; /*Strip newline char*/

		if(feof(stdin)) {
			exit_with_sig(0, NULL);
		}

		tokens = build_tokens(input);
		commands = build_commands(tokens, &num_commands);

		execute_comm_list(commands, num_commands);
		destroy_commands(commands);
	}
}
