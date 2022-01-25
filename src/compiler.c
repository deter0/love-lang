#include "parser.h"
#include <stddef.h>
#if !defined(COMPILER_C)
#define COMPILER_C

#include "tokenizer.h"
#include "tokenizer.c"
#include "love_state.c"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>

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

typedef struct t_state {
	token_pool *pool;
	size_t saved_index;
	size_t current_index;
	token *next_token;
	token *cur_token;
} t_state;

token *t_state_next(t_state *state) {
	state->current_index++;
	state->next_token = state->pool->tokens[state->current_index + 1];
	state->cur_token = state->pool->tokens[state->current_index];
	return state->cur_token;
}

token *t_state_previous(t_state *state) {
	state->current_index--;
	state->next_token = state->pool->tokens[state->current_index + 1];
	state->cur_token = state->pool->tokens[state->current_index];
	return state->cur_token;
}

void t_state_save(t_state *state) {
	state->saved_index = state->current_index;
}

void t_state_rollback(t_state *state) {
	state->current_index = state->saved_index;
    
	state->next_token = state->pool->tokens[state->current_index + 1];
	state->cur_token = state->pool->tokens[state->current_index - 1];
}

void type_ass_err(char *name, token_type expected, token_type got) { // Consistent errors
    fprintf(stderr, "Error assigning \"%s\" of type %s to type %s.\n", name, get_token_type_string(expected), get_token_type_string(got));
    exit(1);
}

void redec_err(char *name, token_type type) { // TODO(deter): Move errors to another file
    fprintf(stderr, "Variable \"%s\" with type %s already exists.\n", name, get_token_type_string(type));
    exit(1);
}

int match_variable(t_state *token_state, love_state *l_state) {
    token *type = token_state->cur_token;
	token *name = token_state->next_token;
	assert_eof(name, token_word);
    
	if (name->type == token_word) {
		t_state_next(token_state);
		token *eq_or_el = token_state->next_token;
		printf("Okay! %s\n", get_token_type_string(eq_or_el->type));
		assert_eof(eq_or_el, token_equals);
        
		if (eq_or_el->type == token_equals) {
			printf("Is Equal\n");
			t_state_next(token_state);
			token *value = token_state->next_token;
			assert_eof(value, token_word); // TODO(deter): Put variadic args on assert_eof func
			love_var *variable = (love_var *)malloc(sizeof(love_var));
			assert(variable != NULL);
			variable->name = get_value(name->value);
            // TODO(deter): Validate function name to see if it contains numbers
            // We should parse numbers in `parser.c` or `tokenizer.c`
            // Tokenizer would be better since we can just check if the first letter of 
            // a `token_word` is a number and try an atoi or atof. We can also throw an error if
            // that fails.
			variable->is_ref = false;
			variable->data_type = type->type;
            variable->is_constant = false;
            variable->value = (void*)get_value(value->value);
            
            (void)t_state_next(token_state);
            if (token_state->next_token->type == token_end_line) {
                (void)t_state_next(token_state);
                t_state_save(token_state);
            } else {
                fprintf(stderr, "Expected ';' after declaring variable got: %s.\n", get_token_type_string(token_state->next_token->type));
            }
            
			if (type->type != token_data_type_string && value->type == token_constant_string) {
                type_ass_err(variable->name, type->type, value->type);
			} else {
				//printf("Okay 2. %s = %s\n", get_token_type_string(type->type), get_token_type_string(value->type));
                bool exists = ls_does_var_exist(l_state, variable->name, type->type);
                if (exists) {
                    redec_err(variable->name, type->type);
                }
                ls_add_var(l_state, variable);
			}
		} else if (eq_or_el->type == token_end_line) {
			printf("This is a uninit'ed var.\n");
			t_state_save(token_state);
			return 1;
		}
	}
	return 0;
}

// * TODO(deter): Add a way to name the output assembly file as a parameter from main.c
void compile_x86_64_linux(token_pool *tokens) {
	fclose(fopen("out.asm", "w"));
	FILE *output_asm = fopen("out.asm", "w");
	
	love_state *l_state = (love_state *)malloc(sizeof(love_state));
	t_state *token_state = (t_state *)calloc(1, sizeof(t_state));
	assert(token_state != NULL && l_state != NULL);
    
    l_state->variable_count = 0;
    l_state->variables_allocated = 0;
    l_state->variables = malloc(0); 
    
	token_state->current_index = 0;
	token_state->saved_index = 0;
	token_state->pool = tokens;
	token_state->cur_token = token_state->pool->tokens[0];
	token_state->next_token = token_state->pool->tokens[1];
    
	while (1) {
		token *cur_token = token_state->cur_token;
		if (cur_token->type == token_eof) {
			break;
		}
		if (is_type(cur_token->type)) {
			int matched = match_variable(token_state, l_state);
			if (matched == 0) {
				// Other matches
			}
		}
		t_state_next(token_state);
		t_state_save(token_state);
	}
    
    printf("Var count: %ld\n\n\n\n", l_state->variable_count);
    for (size_t k = 0; k < l_state->variable_count; k++) {
        if (l_state->variables[k]->data_type == token_data_type_string) {
            printf("%s = %s\n", l_state->variables[k]->name, (char*)l_state->variables[k]->value);
        } else {
            fprintf(stderr, "Unsupported variable type: %s\n", get_token_type_string(l_state->variables[k]->data_type));
        }
    }
    printf("\n\n\n");
    exit(1);
}

#endif // COMPILER_C
