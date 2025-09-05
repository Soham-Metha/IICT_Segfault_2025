#ifndef STMT_LAYER_MIDDLEEND
#define STMT_LAYER_MIDDLEEND

#include <Utils/strings.h>

typedef struct Var_IR {
	String name;
	unsigned int mem_addr;
} Var_IR;

void push_var_def(String name, int id);

int get_var_id(String name);

void clear_var_defs();

#endif