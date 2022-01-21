#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "parser.h"
#include "tokenizer.h"

#define DEBUG
#include "parser.c"
#include "tokenizer.c"
#include "compiler.c"

char *slurp_file(char *path) {
	char *buffer;
	FILE *fp = fopen(path, "rb");
	if (!fp) {
		fprintf(stderr, "Error opening `%s` file: %s\n", path, strerror(errno));
		exit(1);
	}
	fseek(fp, 0L, SEEK_END);
	size_t s = ftell(fp);
	rewind(fp);
	buffer = malloc(sizeof(char) * (s + 1));
	if (buffer != NULL) {
		fread(buffer, s, 1, fp);
		buffer[s] = '\0';
	} else {
		fprintf(stderr, "Error allocating memory for file `%s`: %s\n", path, strerror(errno));
		exit(1);
	}
	fclose(fp);
	return buffer;
}

int main(int argc, char **argv) {
	char *source_file = *argv++;
	(void)argc;
	(void)source_file;
	
	char *parameter = *argv++;
	while (parameter != NULL) {
		char *file_data = slurp_file(parameter);

		parse_result_pool *pool = parse_string(file_data);
		token_pool *tokens = tokenize(pool);
		compile_x86_64_linux(tokens);
		
		parameter = *argv++;
	}
	
	return 0;
}