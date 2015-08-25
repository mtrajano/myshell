#ifndef MYSHELL_H
#define MYSHELL_H

#include "command.h"

#define MAX_ARG_LENGTH 1000

#define MISSMATCHED_SINGLE_QUOTE (-1)
#define MISSMATCHED_DOUBLE_QUOTE (-2)

#define LOG(fmt, args...) printf(fmt, ##args)

/**
** @param {line} line of argument
** returns how long the argument is
**/

int get_arg_len(char *line);


/**
** @param {line} line which space at the end is to be removed
**/

void remove_trailing_space(char *line);

/**
** @param {comm_list} list of commands
** @param {num_progs} number of programs in comm_list
**/

void execute_comm_list(command_t *comm_list, int num_progs);



/**
*** ------------- BEGINNING OF SPECIAL COMMANDS -----------
*** --------------- COMMANDS ARE NOT IN PATH --------------
***/

/**
** @param {sig} signal to be exited with
** @param {comm_list} list to be cleaned up on exit
**/

void exit_with_sig(int sig, command_t *comm_list);

/**
** @param {path} path to cd into
**/


void change_dir(char *path);

/**
*** ---------------- END OF SPECIAL COMMANDS --------------
***/



#endif