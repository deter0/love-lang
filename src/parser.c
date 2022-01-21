#if !defined(PARSER_C)
#define PARSER_C

#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

int get_split_data(char character) {
	switch (character) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
		return 1;
		
		case '-':
		case '+':
		case '*':
		case '/':
		case '[':
		case ']':
		case '{':
		case '}':
		case ';':
		case '=':
		case '!':
		case '@':
		case ',':
		case '$':
		case '~':
		case '`':
		case '%':
		case '^':
		case '&':
		case '(':
		case ')':
		case '#':
		case '.':
		case '?':
		return 2;
		
		default:
			return -1;
	}
}

void add_parse_result(parse_result_pool *pool, parse_result *to_add) {
	if (pool->length + 1 >= pool->allocated) {
		pool->allocated = pool->allocated * 1.5 + 10;
		pool->results = (parse_result **)realloc((void*)pool->results, sizeof(parse_result*) * pool->allocated);
		if (pool->results == NULL) {
			fprintf(stderr, "Error reallocating pool memory.\n");
			exit(1);
		}
	}
	pool->results[pool->length] = to_add;
	pool->length++;
}

size_t get_line(char *string, size_t index) {
	size_t count = 0;
	for (size_t j = 0; j < index; j++) {
		if (string[j] == '\n') {
			count++;
		}
	}
	return count;
}

char *get_value(parse_result *parse) {
	char *buffer = (char *)malloc(sizeof(char) * (parse->length + 1));
	assert(buffer != NULL && "error allocating string.");
	if (buffer == NULL) {
		fprintf(stderr, "Error allocating buffer of size %s %ld\n", parse->ptr_start, parse->length + 1);
		exit(1);
	}
	size_t ptr_size = strlen(parse->ptr_start);
	for (size_t k = 0; k < parse->length && k < ptr_size; k++) {
		buffer[k] = parse->ptr_start[k];
	}
	buffer[parse->length] = '\0';
	return buffer;
}

parse_result_pool *parse_string(char *string) {
	// parse_result **things = (parse_result **)malloc(sizeof(parse_result*) * 20);
	// parse_result *thing = (parse_result *)malloc(sizeof(parse_result));
	// thing->length = 20;
	// things[0] = thing;
	// printf("%ld\n", things[0]->length);
	parse_result_pool *pool = (parse_result_pool *)calloc(1, sizeof(parse_result_pool));
	if (pool == NULL) {
		fprintf(stderr, "Error allocating parse result pool.\n");
		exit(1);
	}
	pool->allocated = 50;
	pool->results = (parse_result **)calloc(pool->allocated, sizeof(parse_result*));
	
	for (size_t n = 0; n < strlen(string); n++) {
		if (string[n] == '\n') {
			string[n] = ' ';
		}
	}
	
	size_t i = 0, t = 0;
	uint8_t locked_string = 0;
	
	while (1) {
		if (string[i] == '\0')
			break;
		if (string[i] == '"' || string[i] == '\'') {
			if (locked_string && (string[i] == '"' ? 1 : 2) == locked_string) {
				size_t escape_counter = 0;
				for (int j = i - 1; j > 0; j--) {
					if (string[j] == '\\') {
						escape_counter++;
					} else {
						break;
					}
				}
				if (escape_counter % 2 == 0 || escape_counter == 0) {
					locked_string = 0;
				} else {
					fprintf(stderr, "Invalid string line: %ld.\n", get_line(string, i));
					exit(1);
				}
			} else if (!locked_string) {
				t = i;
				locked_string = string[i] == '"' ? 1 : 2;
			}
		}
		if (locked_string == 0) {
			int split_data = get_split_data(string[i]);
			switch (split_data) {
			case 1:
				//parse_result *thing = (parse_result *)malloc(sizeof(parse_result));
				{
					parse_result *before_i = (parse_result *)malloc(sizeof(parse_result));
					assert(before_i != NULL && "error allocating memory (1).\n");
					before_i->ptr_start = string + t;
					before_i->length = i - t;
					before_i->is_end = false;
					add_parse_result(pool, before_i);
					t = i + 1;
				}
				break;
			
			case 2:
				{
					parse_result *before_i = (parse_result *)malloc(sizeof(parse_result));
					assert(before_i != NULL && "error allocating memory (2).\n");
					before_i->ptr_start = string + t;
					before_i->length = i - t;
					before_i->is_end = false;
					add_parse_result(pool, before_i);
					
					parse_result *character = (parse_result *)malloc(sizeof(parse_result));
					assert(character != NULL && "error allocating memory (3).\n");
					character->ptr_start = string + i;
					character->length = 1;
					character->is_end = false;
					add_parse_result(pool, character);
					t = i + 1;
				}
				break;
			
			default:
				break;
			}
		}
		i++;
	}
	
#ifdef DEBUG
	for (size_t k = 0; k < pool->length; k++) {
		printf("TOKEN: `%s`\n", get_value(pool->results[k]));
	}
	printf("%ld\n", pool->length);
#endif
	
	return pool;
}

#endif // PARSER_C
