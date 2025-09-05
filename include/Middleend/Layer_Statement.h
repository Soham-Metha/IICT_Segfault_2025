#ifndef STMT_LAYER_MIDDLEEND
#define STMT_LAYER_MIDDLEEND

#include <Utils/strings.h>

typedef struct Var_IR {
	String name;
	String type;
	int mem_addr;
} Var_IR;

void push_var_def(String name, String type, int id);

int get_var_id(String name);

void clear_var_defs();

int get_type_from_name(String name);

int get_size_of_type(int type);

int check_var_mutability(int id);

bool check_var_type(int id, String type);

#endif