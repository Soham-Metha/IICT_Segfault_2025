#include <Middleend/Layer_Statement.h>
#include <assert.h>
#include <Utils/strings.h>

Var_IR var_defs[128];
int var_def_cnt = 0;

void push_var_def(String name, String type, int id)
{
	assert(var_def_cnt < 128);
	var_defs[var_def_cnt++] = (Var_IR) { .name = name, .type = type, .mem_addr = id };
}

int get_var_id(String name) {
    for (int i = var_def_cnt-1; i >= 0; i--)
    {
        if (compare_str(name,var_defs[i].name)){
            return var_defs[i].mem_addr;
        }
    }
    assert(0 && "VAR NOT IN SCOPE");
}

bool check_var_type(int id, String type) {
    for (int i = var_def_cnt-1; i >= 0; i--)
    {
        if (id==var_defs[i].mem_addr){
            return compare_str(type,var_defs[i].type);
        }
    }
    assert(0 && "VAR NOT IN SCOPE");
}

void clear_var_defs() {
    var_def_cnt = 0;
}

typedef enum {
    VAR_TYPE_STR,
    VAR_TYPE_I64,
    VAR_TYPE_F64,
    VAR_TYPE_CHAR,
    VAR_TYPE_FUNC,
    VAR_TYPE_STRUCT,
    VAR_TYPE_COUNT,
} varType;

int get_type_from_name(String name)
{
    if (compare_str(name,STR("func"))){
        return VAR_TYPE_FUNC;
    } else if (compare_str(name,STR("struct"))){
        return VAR_TYPE_STRUCT;
    } else if (compare_str(name,STR("char"))){
        return VAR_TYPE_CHAR;
    } else if (compare_str(name,STR("i64"))){
        return VAR_TYPE_I64;
    } else if (compare_str(name,STR("f64"))){
        return VAR_TYPE_F64;
    } else if (compare_str(name,STR("str"))){
        return VAR_TYPE_STR;
    }
    assert(0 && "INVALID TYPE!");
}

int get_size_of_type(int type) {
    switch(type)
    {
        case VAR_TYPE_I64:
            return 8;
        case VAR_TYPE_F64:
            return 8;
        case VAR_TYPE_CHAR:
            return 1;
        case VAR_TYPE_STR:
        case VAR_TYPE_FUNC:
        case VAR_TYPE_STRUCT:
        case VAR_TYPE_COUNT:
        default:
            return 0;
    }
}

int get_size_from_id(int id) {
    switch(get_type_from_name(var_defs[id].type))
    {
        case VAR_TYPE_I64:
            return 8;
        case VAR_TYPE_F64:
            return 8;
        case VAR_TYPE_CHAR:
            return 1;
        case VAR_TYPE_STR:
        case VAR_TYPE_FUNC:
        case VAR_TYPE_STRUCT:
        case VAR_TYPE_COUNT:
        default:
            return 0;
    }
}