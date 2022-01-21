#if !defined(TOKENIZER_C_)
#define TOKENIZER_C_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "parser.c"
#include "parser.h"
#include "tokenizer.h"

bool cmp_parse_res(parse_result *parse, char *str) {
	for (size_t k = 0; k < parse->length; k++) {
		if (parse->ptr_start[k] != str[k]) {
			return 1;
		}
	}
	return 0;
}

token_type get_keyword_type(parse_result *value) {
	if (cmp_parse_res(value, "void") == 0) {
		return token_data_type_void;
	} else if (cmp_parse_res(value, "int") == 0) {
		return token_data_type_int;
	} else if (cmp_parse_res(value, "string") == 0) {
		return token_data_type_string;
	} else {
		return -1;
	}
}

/* what are the ()[]!-=*;/ operators called idk just using punct short for punctuation */
token_type get_punct_type(parse_result *value) { // TODO(deter): Optimize
	if (cmp_parse_res(value, "(") == 0) {
		return token_paren_open;
	} else if (cmp_parse_res(value, ")") == 0) {
		return token_paren_close;
	} else if (cmp_parse_res(value, "{") == 0) {
		return token_scope_open;
	} else if (cmp_parse_res(value, "}") == 0) {
		return token_scope_close;
	} else if (cmp_parse_res(value, ";") == 0) {
		return token_end_line;
	} else if (cmp_parse_res(value, "=") == 0) {
		return token_equals;
	} else if (cmp_parse_res(value, ",") == 0) {
		return token_comma;
	} else if (cmp_parse_res(value, "#") == 0) {
		return token_hashtag;
	} else {
		return -1;
	}
}

void add_token(token_pool *tokens, token *token) {
	if (tokens->length + 1 > tokens->allocated) {
		tokens->allocated = tokens->allocated * 1.75 + 50;
		tokens->tokens = realloc(tokens->tokens, sizeof(*token) * tokens->allocated);
		if (tokens->tokens == NULL) {
			fprintf(stderr, "Error reallocating memory (T1).\n");
			exit(1);
		}
	}
	// printf("Added: %s\n", get_token_type_string(token->type));
	tokens->tokens[tokens->length] = token;
	tokens->length++;
}

void print_tokens(token_pool *pool) {
	for (size_t k = 0; k < pool->length; k++) {
		if (pool->tokens[k]->type == token_eof)
			break;
		printf("Token Type %s \x1b[1;31m%s\x1b[0m\n",
			get_token_type_string(pool->tokens[k]->type),
			get_value(pool->tokens[k]->value)
		);
	}
}

token_pool *tokenize(parse_result_pool *parsed_pool) {
	token_pool *tok_pool = (token_pool *)calloc(1, sizeof(token_pool));
	assert(tok_pool != NULL && "Error allocating token pool");
	tok_pool->allocated = 50;
	tok_pool->tokens = (token **)calloc(tok_pool->allocated, sizeof(token*));
	assert(tok_pool->tokens != NULL && "Error allocating token pool's tokens");

	printf("Okay %ld!\n", parsed_pool->length);
	size_t j;
	for (j = 0; j < parsed_pool->length; j++) {
		parse_result *res = parsed_pool->results[j];
		
		if (res->length <= 0)
			goto next;
		
		token *c_token = (token *)calloc(1, sizeof(c_token));
		c_token->value = res;
		
		token_type data_type = get_keyword_type(res);
		token_type punct_type = get_punct_type(res);
		
		printf("Token len: %ld\n", res->length);
		
		if ((int)data_type != -1) {
			c_token->type = data_type;
		} else if ((int)punct_type != -1) {
			c_token->type = punct_type;
		} else if (
			(res->ptr_start[0] == '"' && res->ptr_start[res->length - 1] == '"')
			|| (res->ptr_start[0] == '\'' && res->ptr_start[res->length - 1] == '\'')) {
			c_token->type = token_constant_string;
		} else {
			c_token->type = token_word;
		}
		add_token(tok_pool, c_token);
		next:
		if (res->is_end)
			break;
	}
	
	token *end_of_file = (token*)malloc(sizeof(token));
	end_of_file->type = token_eof;
	end_of_file->value = (parse_result*)calloc(0, sizeof(parse_result));
	add_token(tok_pool, end_of_file);

	print_tokens(tok_pool);

	return tok_pool;
}

#endif // TOKENIZER_C_
