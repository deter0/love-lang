#if !defined(TOKENIZER_H)
#define TOKENIZER_H

#include "parser.h"

#include <stdlib.h>
#include <stdbool.h>

typedef enum {
	token_word,
	token_number,
	token_hashtag,
	
	// Punct
	token_paren_open,
	token_paren_close,
	
	token_scope_open, // {
	token_scope_close, // }
	
	token_end_line, // ;
	token_eof,
	
	token_equals,
	token_comma,
	
	// Types
	token_data_type_void,
	token_data_type_int,
	token_data_type_string,
	
	token_constant_string
} token_type;

bool is_type(token_type type) {
	return (int)type <= (int)token_data_type_string
		&& (int)type >= (int)token_data_type_void;
}

char *get_token_type_string(token_type type) {
	switch (type) {
		case token_word:
			return "<Word>";
		case token_number:
			return "<Number>";
		case token_data_type_void:
			return "<Data Type Void>";
		case token_data_type_int:
			return "<Data Type Int>";
		case token_data_type_string:
			return "<Data Type String>";
		case token_paren_open:
			return "<Parenthesis Open>";
		case token_paren_close:
			return "<Parenthesis Close>";
		case token_scope_open:
			return "<Scope Open>";
		case token_constant_string:
			return "<Constant String>";
		case token_end_line:
			return "<End of Line>";
		case token_eof:
			return "<End of File>";
		case token_scope_close:
			return "<Scope Close>";
		case token_equals:
			return "<Equal Sign>";
		case token_comma:
			return "<Comma>";
		case token_hashtag:
			return "<Hashtag>";
		default:
			return "<Unknown Token>";
	}
}

typedef struct {
	token_type type;
	parse_result *value;
} token;

typedef struct {
	token **tokens;

	size_t allocated;
	size_t length;
} token_pool;

token_pool *tokenize(parse_result_pool *parse_result);

#endif // TOKENIZER_H
