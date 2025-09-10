#include <Middleend/Layer_Statement.h>
#include <assert.h>
#include <Utils/strings.h>
#include <Middleend/Layer_Line.h>

static TypeDetailsLUT typeDetails[VAR_TYPE_CNT] = {
	[VAR_TYPE_STR] = { .type = VAR_TYPE_STR,
			   .size = 16,
			   .name = { .data = "str", .len = 3 },
			   .read = "nop",
			   .write = "SWAP 1\nSWAP 2\nSWAP1\nWRITE8\nWRITE8" },
	[VAR_TYPE_FUNC] = { .type = VAR_TYPE_FUNC,
			    .size = 0,
			    .name = { .data = "func", .len = 4 },
			    .read = "nop",
			    .write = "nop" },
	[VAR_TYPE_I64] = { .type = VAR_TYPE_I64,
			   .size = 8,
			   .name = { .data = "i64", .len = 3 },
			   .read = "READ8I",
			   .write = "WRITE8I" },
	[VAR_TYPE_BOOL] = { .type = VAR_TYPE_BOOL,
			    .size = 1,
			    .name = { .data = "bool", .len = 4 },
			    .read = "READ1U",
			    .write = "WRITE1U" },
};

void push_var_def(Block_Context_IR *ctx, String name, String type, int id,
		  TypeList *list)
{
	assert(!compare_str(type, STR("void")));
	assert(ctx->var_def_cnt < 128);
	ctx->var_defs[ctx->var_def_cnt++] =
		(Var_IR){ .name = name,
			  .list = list,
			  .has_def = false,
			  .type = get_type_details_from_type_name(type).type,
			  .mem_addr = id };
}

Var_IR get_var_details(const Block_Context_IR *ctx, String name)
{
	for (const Block_Context_IR *curr = ctx; curr != NULL;
	     curr = curr->prev)
		for (int i = curr->var_def_cnt - 1; i >= 0; i--) {
			if (compare_str(name, curr->var_defs[i].name)) {
				return curr->var_defs[i];
			}
		}
	assert(0 && "VAR NOT IN SCOPE");
}

void set_var_as_defined(Block_Context_IR *ctx, String name)
{
	for (Block_Context_IR *curr = ctx; curr != NULL; curr = curr->prev) {
		for (int i = curr->var_def_cnt - 1; i >= 0; i--) {
			if (compare_str(name, curr->var_defs[i].name)) {
				curr->var_defs[i].has_def = true;
				return;
			}
		}
	}
	assert(0 && "VAR NOT IN SCOPE");
}

TypeDetailsLUT get_type_details_from_type_name(String name)
{
	for (int i = 0; i < VAR_TYPE_CNT; i++) {
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