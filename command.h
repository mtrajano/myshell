#ifndef COMMAND_H
#define COMMAND_H

#include "token.h"

#define MAX_ARGS 50

typedef struct command command_t; 
struct command {
	int argc;
	char **argv;
	command_t *next;
};

/**
** @returns a new empty command node
**/

command_t *create_command();

/**
** @param {head} list of commands
** @param {new_comm} new command to be added to list of commands
**/

void add_command(command_t **head, command_t *new_comm);

/**
** @param {tokens} list of tokens
** @param {num_progs} reference to int will be set to number of programs in list returned
** @returns a list of commands to be executed
**/

command_t *build_commands(token_t *tokens, int *num_progs);

/**
** @param {args} array of arguments to be freed
** @param {argc} number of arguments in array
**/

void free_args(char **args, int argc);

/**
** @param {line} list of commands to be freed
**/

void destroy_commands(command_t *list);

#endif