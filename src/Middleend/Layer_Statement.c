#include <Middleend/Layer_Statement.h>
#include <assert.h>

Var_IR var_defs[128];
int var_def_cnt = 0;

void push_var_def(String name, int id)
{
	assert(var_def_cnt < 128);
	var_defs[var_def_cnt++] = (Var_IR){ .name = name, .mem_addr = id };
}

int get_var_id(String name) {
    for (int i = 0; i < var_def_cnt; i++)
    {
        if (compare_str(name,var_defs[i].name)){
            return var_defs[i].mem_addr;
        }
    }
    assert(0 && "VAR NOT IN SCOPE");
}

void clear_var_defs() {
    var_def_cnt = 0;
}