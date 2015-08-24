#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_ARGS 50
#define MAX_ARG_LENGTH 1000

#define MISSMATCHED_SINGLE_QUOTE (-1)
#define MISSMATCHED_DOUBLE_QUOTE (-2)

#define TRACEMSG(f) printf(f)
#define TRACE(f, x...) printf(f, x)

typedef struct tokens tokens_t; 

tokens_t *create_token(char *line, int num_chars);
void add_token(tokens_t **head, char *line, int num_chars);
tokens_t *build_tokens(char *line);
void exit_with_sig(int sig, char **argv, int argc);
int get_arg_len(char *line);
int parse_input(char *line, char **argv);
void free_args(char **args, int argc);
void remove_trailing_space(char *line);
int execute_prog(char **argv, int argc);


struct tokens {
	char *token;
	tokens_t *next;
};

void debug_argv(char **argv, int argc) {
	for(int i=0; i<argc; i++) {
		TRACE("arg %d: %s, len: %lu\n", i, argv[i], strlen(argv[i]));
	}
	TRACE("argc: %d\n", argc);
}

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
** @param {sig} signal to be exited with, NULL defaults to 0
**/

void exit_with_sig(int sig, char **argv, int argc) {
	/*Clean up before*/
	free_args(argv, argc);

	printf("Shell exited with signal: %d\n", sig);
	exit(sig);
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
** @param {line} line to be parsed
** @param {argv} array of arguments to be filled
** @returns number of arguments read
**/

int parse_input(char *line, char **argv) {
	int argc = 0;
	argv[0] = NULL;

	/*So no extra arg at the end*/
	remove_trailing_space(line);

	while(*line && argc < MAX_ARGS) {
		int arg_len = 0;

		while(*line == ' ') {
			line++;
		}

		/*If error return to main and don't execute program*/
		if((arg_len = get_arg_len(line)) < 0) {
			printf("Error parsing input\n");
			return arg_len;
		}

		/*Exclude first quotes from arg*/
		if(*line == '\'' || *line == '\"') {
			line++;
		}

		char *arg = malloc(arg_len);
		strncpy(arg, line, arg_len);

		line += arg_len;

		/*Exclude other side of quotes*/
		if(*line == '\'' || *line == '\"') {
			*line = '\0';
			line++;
		}

		argv[argc++] = arg;
		argv[argc] = NULL;
	}

	return argc;
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
** @param {argv} array of arguments for program, argv[0] is name of program
** @param {argc} argument count
** @returns 0 on success, <0 on error
**/

int execute_prog(char **argv, int argc) {
	pid_t procid, wpid;
	int status = 0;

	if(strcmp(argv[0], "exit") == 0) {
		int sig = argv[1] ? atoi(argv[1]) : 0;
		exit_with_sig(sig, argv, argc);
	}
	else {
		procid = fork();

		if(procid < 0) {
		    printf("Error: Something unexpected happened\n");
		}
		else if(procid == 0) {
		    if(execvp(argv[0], argv) < 0) {
		        printf("Error: %s program could not be found in your path\n", argv[0]);
		        _exit(EXIT_FAILURE);
		    }
		    else {
				_exit(EXIT_SUCCESS);
		    }
		}
		else {
		    wpid = wait(&status);
		    free_args(argv, argc);
		}
	}

	return status;
}

int main( void ) {
	int argc = 0;
	char **argv = NULL;
	char *input;
	size_t read_count;
	tokens_t *tokens;

	/*Prompt user until interrupt, error or quit*/
	while(1) {
		printf("> ");
		getline(&input, &read_count, stdin);
		input[strlen(input) - 1] = '\0'; /*Strip newline char*/

		if(feof(stdin)) {
			exit_with_sig(0, argv, argc);
		}

		tokens = build_tokens(input);

		destroy_tokens(tokens);
	}
}