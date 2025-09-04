#ifndef STMT_LAYER_MIDDLEEND
#define STMT_LAYER_MIDDLEEND

#include <Utils/strings.h>

typedef struct Var_IR {
	String name;
	unsigned int mem_addr;
} Var_IR;

#endif