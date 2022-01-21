#if !defined(PARSER_H_)
#define PARSER_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct prase_result {
	char *ptr_start;
	size_t length;
	bool is_end;
} parse_result;

typedef struct parse_result_pool {
	parse_result **results;
	size_t length;
	size_t allocated;
} parse_result_pool;

int get_split_data(char character);
void add_parse_result(parse_result_pool *pool, parse_result *to_add);

size_t get_line(char *string, size_t index);
char *get_value(parse_result *parse);
parse_result_pool *parse_string(char *string);

#endif // PARSER_H_
