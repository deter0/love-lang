#if !defined(COMPILER_C)
#define COMPILER_C

#include "tokenizer.h"

#include "tokenizer.c"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

void assert_eof(token *current_token, token_type expected_token_type) {
	if (current_token->type == token_eof) {
		fprintf(stderr, "Unexpected %s did you mean token %s?\n",
			get_token_type_string(current_token->type), get_token_type_string(expected_token_type));
		exit(1);
	}
}

typedef struct token_pool_2d {
	size_t len;
	token_pool **split_pool;
} token_pool_2d;

token_pool_2d split_tokens(token_pool *pool, size_t from, size_t to, token_type delim) {
	size_t allocated = 5;
	size_t len = 0;
	token_pool **split_pool = (token_pool**)malloc(sizeof(token_pool*) * allocated);
	size_t prev = from;
	
	for (size_t k = from; k < to; k++) {
		// printf("\t\t\t%ld = %ld -> %ld\n", k, from, to);
		if (pool->tokens[k]->type == delim || k == to - 1) {
			// printf("%s splitting.\n", get_token_type_string(pool->tokens[k]->type));
			token_pool *split = (token_pool *)malloc(sizeof(token_pool));
			split->allocated = 5;
			split->tokens = (token **)malloc(sizeof(token*) * split->allocated);
			split->length = 0;
			for (size_t j = prev; j < k; j++) {
				add_token(split, pool->tokens[j]);
			}
			prev = k + 1;
			
			if (len + 1 >= allocated) {
				allocated += 15;
				split_pool = realloc(split_pool, sizeof(token_pool*) * allocated);
			}
			split_pool[len] = split;
			len++;
		}
	}
	
	token_pool_2d pool_2d = {
		.len = len,
		.split_pool = split_pool
	};
	
	return pool_2d;
}

void assert_tok_af(token_type type, token_type before, token_type expected) {
	if (type != expected) {
		fprintf(stderr, "ERROR: Expected %s after %s but got %s.\n",
			get_token_type_string(expected),
			get_token_type_string(before),
			get_token_type_string(type));
		exit(1);
	}
}

char* get_variable_val(token_pool *pool) {
	bool is_len = pool->tokens[0]->type == token_hashtag;
	if (is_len) {
		assert_tok_af(pool->tokens[1]->type, token_hashtag, token_word);
		char *var_name = get_value(pool->tokens[1]->value);
		char *len_name = strcat(var_name, "_len");
		return len_name;
	} else {
		assert_tok_af(pool->tokens[0]->type, token_comma, token_word);
		return get_value(pool->tokens[0]->value);
	}
}

void __mov_func(FILE *output_asm, token_pool_2d *params) {
	fprintf(output_asm, "\tmov %s, %s\n",
		get_variable_val(params->split_pool[0]) + 2,
		get_variable_val(params->split_pool[1])
	);
}

void __int_func(FILE *output_asm, token_pool_2d *params) {
	fprintf(output_asm,
		"\tint %s\n",
		get_variable_val(params->split_pool[0])
	);
}

void __sys_call_func(FILE *output_asm, token_pool_2d *params) {
	(void)params;
	fprintf(output_asm,
		"\tsyscall\n"
	);
}

void compile_x86_64_linux(token_pool *tokens) {
	fclose(fopen("out.asm", "w"));
	FILE *output_asm = fopen("out.asm", "w");
	if (!output_asm) {
		perror("Error opening assembly file for writing");
		exit(1);
	}
	
	fprintf(output_asm, "section .text\n\tglobal _start\n\n");
	
	for (size_t p = 0; p < tokens->length; p++) {
		token *current_token = tokens->tokens[p];
		if (is_type(current_token->type)) {
			token *function_name = tokens->tokens[p + 1]; //current_token + 1;
			assert_eof(function_name, token_word);
			
			token *parameters_start = tokens->tokens[p + 2]; //function_name + 1;
			assert_eof(parameters_start, token_paren_open);
			if (parameters_start->type == token_paren_open) {
				token *parameters_end = tokens->tokens[p + 3]; // parameters_start + 1;
				assert_eof(parameters_end, token_data_type_int);
				for (size_t k = 0; 1; k++) {
					if (parameters_end->type == token_paren_close)
						break;
					printf("Params: %s\n", get_token_type_string(parameters_end->type));
					assert_eof(parameters_end, token_paren_close);
					parameters_end = tokens->tokens[p + 3 + k];
				}
				char *function_name_str = get_value(function_name->value);
				fprintf(output_asm, "%s:\n", strcmp(function_name_str, "main") == 0 ? "_start" : function_name_str);
				free(function_name_str);
				size_t parameter_count = parameters_end - parameters_start;
				printf("Parameters count: %ld\n", parameter_count);
				printf("%s | %s\n", get_token_type_string(parameters_start->type), get_token_type_string(parameters_end->type));
				
				token *scope_open = tokens->tokens[p + 4]; //parameters_end + 1;
				assert_eof(scope_open, token_scope_open);
				
				if (
					parameters_start->type != token_paren_open
					|| parameters_end->type != token_paren_close
					|| scope_open->type != token_scope_open) {
					goto skip;
				}
				
				token *body_token = scope_open;
				size_t body_len = 0;
				printf("%s:\n", get_value(function_name->value));
				while (1) {
					assert_eof(body_token, token_word);
					if (body_token->type == token_scope_close)
						break;
					body_len++;
					
					printf("\t%s\n", get_token_type_string(body_token->type));
					if (body_token->type == token_word) {
						printf("\t\t>Is Word\n");
						token *open_paren = tokens->tokens[body_len + p + 3];
						if (open_paren->type == token_paren_open) {
							printf("\t\t\t>Is func call\n");
							for (size_t k = 0; 1; k++) {
								if (tokens->tokens[body_len + p + 3 + k]->type == token_paren_close) {
									size_t from = body_len + p + 4;
									size_t to = body_len + p + 4 + k;
									
									printf("\t\t\t%ld -> %ld\n", from, to);
									token_pool_2d params = split_tokens(tokens, from, to, token_comma);
									printf("\t\t\t%s Param Count: %ld\n", get_value(body_token->value), params.len);
									
									for (size_t pl = 0; pl < params.len; pl++) {
										for (size_t pli = 0; pli < params.split_pool[pl]->length; pli++) {
											printf("\t\t\t%s\n", get_value(params.split_pool[pl]->tokens[pli]->value));
										}
									}
									
									char *function_name = get_value(body_token->value);
									if (strcmp(function_name, "__mov") == 0) {
										printf("\t\t\t__mov function call.\n");
										__mov_func(output_asm, &params);
									} else if (strcmp(function_name, "__int") == 0) {
										__int_func(output_asm, &params);
									} else if (strcmp(function_name, "__syscall") == 0) {
										__sys_call_func(output_asm, &params);
									} else {
										fprintf(output_asm, "\tjmp %s\n", function_name);
									}
									
									break;
								}
								assert_eof(tokens->tokens[body_len + p + 3 + k], token_paren_close);
							}
						}
					}
					
					// if (body_token->type == token_word && strcmp(get_value(body_token->value), "__int") == 0) {
					// 	printf("Int.\n");
					// }
					
					body_token = tokens->tokens[body_len + p + 3];
				}
				fprintf(output_asm, "\n\n");
				// fprintf(output_asm, "end:\n\n");
				
				printf("function name: %s\n", get_value(function_name->value));
				
				current_token += 2 + parameter_count + body_len;
				skip:
			}
			if (is_type(current_token->type)) {
				token *variable_name = tokens->tokens[p + 1]; //current_token + 1;
				assert_eof(variable_name, token_word);
				if (variable_name->type == token_word) {
					printf("%s\n", get_token_type_string(function_name->type));
					char *variable_name_str = get_value(function_name->value);
					token *equals = tokens->tokens[p + 2]; //variable_name + 1;
					assert_eof(equals, token_equals);
					if (equals->type == token_equals) {
						token *value = tokens->tokens[p + 3];
						if (value->type == token_constant_string) {
							//!(FIXME): Data section
							printf("VARIABLE: %s\n", variable_name_str);
							fprintf(output_asm, "section .data\n");
							char *dup = malloc(sizeof(char) * strlen(variable_name_str));
							strcpy(dup, variable_name_str);
							fprintf(output_asm, "\t%s: db %s, 10\n", variable_name_str, get_value(value->value));
							fprintf(output_asm, "\t%s: equ $ - %s\n", strcat(dup, "_len"), variable_name_str);
							
							fprintf(output_asm, "\n");
						} else {
							assert(false && "Not implemented.");
						}
					}
				}
			}
		}
	}
	
	fclose(output_asm);
}

#endif // COMPILER_C
