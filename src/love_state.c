#ifndef LOVE_STATE_C_
#define LOVE_STATE_C_

#include "tokenizer.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct love_var {
    token_type data_type;
    char *name;
    void *value;
    bool is_ref;
    bool is_constant;
} love_var;

// int love_var_assign(love_var *var, void *value) {
//     var->value = value;
// }

typedef struct love_state {
    size_t variable_count;
    size_t variables_allocated;
    love_var **variables;
} love_state;

bool ls_does_var_exist(love_state *state, char *name, token_type data_type) { // TODO(deter): Make faster
    for (size_t i = 0; i < state->variable_count; i++) {
        if (strcmp(state->variables[i]->name, name) == 0 && state->variables[i]->data_type == data_type) {
            return true;
        }
    }
    return false;
}

void ls_add_var(love_state *l_state, love_var *var) {
    if (l_state->variables_allocated >= l_state->variable_count + 1) {
        l_state->variables_allocated = l_state->variables_allocated * 1.5 + 50;
        l_state->variables = (love_var **)realloc(l_state->variables, sizeof(love_var *) * l_state->variables_allocated);
    }
    l_state->variables[l_state->variable_count] = var;
    l_state->variable_count++;
}

#endif // LOVE_STATE_C