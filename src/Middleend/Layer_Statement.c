#include <Middleend/Layer_Statement.h>
#include <assert.h>
#include <Utils/strings.h>
#include <Middleend/Layer_Line.h>

static TypeDetailsLUT typeDetails[VAR_TYPE_COUNT] = {
	[VAR_TYPE_STR] = { .type = VAR_TYPE_STR, .size = 0, .name = {.data="str", .len=3 } },
	[VAR_TYPE_FUNC] = { .type = VAR_TYPE_FUNC, .size = 0, .name =  {.data="func", .len=4 } },
};

void push_var_def(Block_Context_IR *ctx, String name, String type, int id)
{
	assert(ctx->var_def_cnt < 128);
	ctx->var_defs[ctx->var_def_cnt++] =
		(Var_IR){ .name = name,
			  .type = get_type_details_from_type_name(type).type,
			  .mem_addr = id };
}

Var_IR get_var_details(Block_Context_IR *ctx, String name)
{
	for (Block_Context_IR *curr = ctx; curr != NULL; curr = curr->prev)
		for (int i = ctx->var_def_cnt - 1; i >= 0; i--) {
			if (compare_str(name, curr->var_defs[i].name)) {
				return curr->var_defs[i];
			}
		}
	assert(0 && "VAR NOT IN SCOPE");
}

TypeDetailsLUT get_type_details_from_type_name(String name)
{
	for (int i = 0; i < VAR_TYPE_COUNT; i++) {
		if (compare_str(name, typeDetails[i].name)) {
			return typeDetails[i];
		}
	}
	assert(0 && "INVALID TYPE!");
}

TypeDetailsLUT get_type_details_from_type_id(varType id)
{
	return typeDetails[id];
}