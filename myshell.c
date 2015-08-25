#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 50
#define MAX_ARG_LENGTH 1000

#define MISSMATCHED_SINGLE_QUOTE (-1)
#define MISSMATCHED_DOUBLE_QUOTE (-2)

#define TRACEMSG(f) printf(f)
#define TRACE(f, x...) printf(f, x)

typedef struct tokens tokens_t; 
typedef struct command command_t; 

tokens_t *create_token(char *line, int num_chars);
void add_token(tokens_t **head, char *line, int num_chars);
void destroy_tokens(tokens_t *list);
tokens_t *build_tokens(char *line);
command_t *create_command();
void add_command(command_t **head, command_t *new_comm);
command_t *build_commands(tokens_t *tokens, int *num_progs);
void destroy_commands(command_t *list);
int get_arg_len(char *line);
void free_args(char **args, int argc);
void remove_trailing_space(char *line);
void execute_comm_list(command_t *comm_list, int num_progs);

/*Implementation of special commands not in /bin*/
void exit_with_sig(int sig, command_t *comm_list);
void change_dir(char *path);


struct tokens {
	char *token;
	tokens_t *next;
};

struct command {
	int argc;
	char **argv;
	command_t *next;
};


void debug_argv(char **argv, int argc) {
	for(int i=0; i<argc; i++) {
		TRACE("arg %d: %s, len: %lu\n", i, argv[i], strlen(argv[i]));
	}
	TRACE("argc: %d\n", argc);
}

void debug_commands(command_t *list) {
	int i=0;
	while(list != NULL) {
		TRACE("command %d:\n", i);
		for(int j=0; j<list->argc; j++) {
			TRACE("\targ %d: %s\n", j, list->argv[j]);
		}
		list = list->next;
		i++;
	}
}

/**
*** ------------- BEGINNING OF SPECIAL COMMANDS -----------
***/

/**
** @param {sig} signal to be exited with
** @param {comm_list} list to be cleaned up on exit
**/

void exit_with_sig(int sig, command_t *comm_list) {
	/*Clean up before*/
	destroy_commands(comm_list);

	printf("Shell exited with signal: %d\n", sig);
	exit(sig);
}

/**
** @param {path} path to cd into
**/

void change_dir(char *path) {
	char *homedir = getenv("HOME");
	char *lastdir = getenv("OLDPWD");

	/*If path is NULL or ~ go to home*/
	/*If path is - go to last working directory*/
	/*Else go to path*/

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

/**
** @param {line} line which space at the end is to be removed
**/

void remove_trailing_space(char *line) {
	char *end = line + strlen(line) - 1;

	while(end > line && *end == ' ') {
		end--;
	}

	*(end+1) = '\0';
}

/**
** @param {line} line of argument
** returns how long the argument is
**/

int get_arg_len(char *line) {
	int len = 0;

	if(*line == '\"') {
		line++;
		while(*line != '\0' && *line != '\"' && len < MAX_ARG_LENGTH) {
			line++;
			len++;
		}

		if(*line != '\"') {
			len = MISSMATCHED_DOUBLE_QUOTE;
		}
	}
	else if(*line == '\'') {
		line++;
		while(*line != '\0' && *line != '\'' && len < MAX_ARG_LENGTH) {
			line++;
			len++;
		}

		if(*line != '\'') {
			len = MISSMATCHED_SINGLE_QUOTE;
		}
	}
	else{
		while(*line != '\0' && *line != ' ' && len < MAX_ARG_LENGTH) {
			line++;
			len++;
		}
	}

	return len;
}

/**
** @param {line} line of arguments
** @param {num_chars} number of characters to be copied to token
** returns a new token
**/

tokens_t *create_token(char *line, int num_chars) {
	tokens_t *temp = malloc(sizeof(tokens_t));
	char *token = malloc(num_chars+1);

	temp->token = token;
	temp->next = NULL;

	strncpy(temp->token, line, num_chars);
	temp->token[num_chars] = '\0';

	return temp;
}

/**
** @param {head} reference to the beginning of the token list
** @param {line} line of arguments
** @param {num_chars} number of characters to be copied to token
**/

void add_token(tokens_t **head, char *line, int num_chars) {
	tokens_t *temp = create_token(line, num_chars);

	if(*head == NULL) {
		*head = temp;
	}
	else{
		tokens_t *tail = *head;
		while(tail->next != NULL) {
			tail = tail->next;
		}
		(tail)->next = temp;
	}
}

/**
** @param {line} line to be parsed for tokens
** returns a list of tokens
**/

tokens_t *build_tokens(char *line) {
	tokens_t *head = NULL;

	/*Skip leading spaces*/
	while(*line == ' ') {
		line++;
	}

	while(*line != '\0') {
		int arg_len = 0;

		/*If error return to main and don't execute program*/
		if((arg_len = get_arg_len(line)) < 0) {
			printf("Error parsing input around '%c'\n", *line);
			return NULL;
		}

		/*Exclude first quotes from arg*/
		if(*line == '\'' || *line == '\"') {
			add_token(&head, line, 1);
			line++;
		}

		add_token(&head, line, arg_len);
		line += arg_len;

		/*Exclude other side of quotes*/
		if(*line == '\'' || *line == '\"') {
			add_token(&head, line, 1);
			line++;
		}

		/*Skip trailing spaces*/
		while(*line == ' ') {
			line++;
		}
	}

	return head;
}

/**
** @param {line} list of tokens to be freed
**/

void destroy_tokens(tokens_t *list) {
	while(list != NULL) {
		tokens_t *temp = list;
		list = list->next;

		free(temp->token);
		free(temp);
	}
}

/**
** @returns a new empty command node
**/

command_t *create_command() {
	command_t *temp = malloc(sizeof(command_t));
	char **args = malloc(MAX_ARGS);

	temp->argv = args;
	temp->argc = 0;
	temp->next = NULL;

	return temp;
}

/**
** @param {head} list of commands
** @param {new_comm} new command to be added to list of commands
**/

void add_command(command_t **head, command_t *new_comm) {
	if(*head == NULL) {
		*head = new_comm;
	}
	else{
		command_t *tail = *head;
		while(tail->next != NULL) {
			tail = tail->next;
		}

		tail->next = new_comm;
	}
}

/**
** @param {tokens} list of tokens
** @param {num_progs} reference to int will be set to number of programs in list returned
** @returns a list of commands to be executed
**/

command_t *build_commands(tokens_t *tokens, int *num_progs) {
	command_t *head = NULL;

	while(tokens != NULL && tokens->token != NULL) {

		command_t *new_comm = create_command();
		(*num_progs)++;

		/*Add args for a single command*/
		while(tokens != NULL && tokens->token != NULL && strcmp(tokens->token, "|") != 0 && new_comm->argc < MAX_ARGS) {
			/*Don't add quotes to args*/
			if(strcmp(tokens->token, "\"") == 0 || strcmp(tokens->token, "\'") == 0) {
				tokens = tokens->next;
				continue;
			}

			new_comm->argv[new_comm->argc++] = tokens->token;
			tokens = tokens->next;
		}

		if(new_comm->argc == MAX_ARGS) {
			printf("Too many arguments given\n");
		}
		else if(tokens !=NULL && tokens->token != NULL) {
			tokens = tokens->next;
		}

		/*End of argument list must be appended by NULL as required by execvp*/
		new_comm->argv[new_comm->argc] = NULL;
		add_command(&head, new_comm);
	}

	return head;
}

/**
** @param {line} list of commands to be freed
**/

void destroy_commands(command_t *list) {
	while(list != NULL) {
		command_t *temp = list;
		list = list->next;

		free_args(temp->argv, temp->argc);
		free(temp);
	}
}

/**
** @param {args} array of arguments to be freed
** @param {argc} number of arguments in array
**/

void free_args(char **args, int argc) {
	if(args != NULL) {
		for(int i=0; i<argc; i++) {
			free(args[i]);
		}

		free(args);
	}
}

/**
** @param {comm_list} list of commands
** @param {num_progs} number of programs in comm_list
**/

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
	tokens_t *tokens;
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