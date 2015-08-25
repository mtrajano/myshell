#include <stdio.h>
#include <stdlib.h>

#include "token.h"

token_t *create_token(char *line, int num_chars) {
	token_t *temp = malloc(sizeof(token_t));
	char *token = malloc(num_chars+1);

	temp->token = token;
	temp->next = NULL;

	strncpy(temp->token, line, num_chars);
	temp->token[num_chars] = '\0';

	return temp;
}

void add_token(token_t **head, char *line, int num_chars) {
	token_t *temp = create_token(line, num_chars);

	if(*head == NULL) {
		*head = temp;
	}
	else{
		token_t *tail = *head;
		while(tail->next != NULL) {
			tail = tail->next;
		}
		(tail)->next = temp;
	}
}

token_t *build_tokens(char *line) {
	token_t *head = NULL;

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

void destroy_tokens(token_t *list) {
	while(list != NULL) {
		token_t *temp = list;
		list = list->next;

		free(temp->token);
		free(temp);
	}
}