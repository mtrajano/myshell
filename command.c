#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

#include "command.h"

command_t *create_command() {
	command_t *temp = malloc(sizeof(command_t));
	char **args = malloc(MAX_ARGS);

	temp->argv = args;
	temp->argc = 0;
	temp->next = NULL;

	return temp;
}

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

command_t *build_commands(token_t *tokens, int *num_progs) {
	command_t *head = NULL;

	while(tokens != NULL && tokens->token != NULL) {

		command_t *new_comm = create_command();
		(*num_progs)++;

		/*Add args for a single command*/
		while(tokens != NULL && tokens->token != NULL && strcmp(tokens->token, "|") != 0 && new_comm->argc < MAX_ARGS) {
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

void free_args(char **args, int argc) {
	if(args != NULL) {
		for(int i=0; i<argc; i++) {
			free(args[i]);
		}

		free(args);
	}
}

void destroy_commands(command_t *list) {
	while(list != NULL) {
		command_t *temp = list;
		list = list->next;

		free_args(temp->argv, temp->argc);
		free(temp);
	}
}
