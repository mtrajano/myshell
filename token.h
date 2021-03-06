#ifndef TOKEN_H
#define TOKEN_H

#define MAX_ARG_LENGTH 1000

#define MISSMATCHED_SINGLE_QUOTE (-1)
#define MISSMATCHED_DOUBLE_QUOTE (-2)

typedef struct token token_t;
struct token {
	char *token;
	token_t *next;
};

/**
** @param {line} line of arguments
** @param {num_chars} number of characters to be copied to token
** returns a new token
**/

token_t *create_token(char *line, int num_chars);

/**
** @param {head} reference to the beginning of the token list
** @param {line} line of arguments
** @param {num_chars} number of characters to be copied to token
**/

void add_token(token_t **head, char *line, int num_chars);

/**
** @param {line} line of argument
** returns how long the argument is
**/

int get_arg_len(char *line);

/**
** @param {line} line to be parsed for tokens
** returns a list of tokens
**/

token_t *build_tokens(char *line);

/**
** @param {line} list of tokens to be freed
**/

void destroy_tokens(token_t *list);

#endif
