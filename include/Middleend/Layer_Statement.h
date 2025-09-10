#ifndef STMT_LAYER_MIDDLEEND
#define STMT_LAYER_MIDDLEEND

#include <Utils/strings.h>
#include <Middleend/Layer_Line.h>

typedef enum {
	VAR_TYPE_VOID,
	VAR_TYPE_STR,
	VAR_TYPE_I64,
	VAR_TYPE_BOOL,
	// VAR_TYPE_F64,
	// VAR_TYPE_CHAR,
	VAR_TYPE_FUNC,
	// VAR_TYPE_STRUCT,
	VAR_TYPE_CNT,
} varType;

typedef struct {
	varType type;
	String name;
	int size;
} TypeDetailsLUT;

typedef struct Var_IR Var_IR;
typedef struct Block_Context_IR Block_Context_IR;

void push_var_def(Block_Context_IR *ctx, String name, String type, int id,
	TypeList list, int argc);

Var_IR get_var_details(const Block_Context_IR *ctx, String name);

TypeDetailsLUT get_type_details_from_type_name(String name);

TypeDetailsLUT get_type_details_from_type_id(varType id);

void set_var_as_defined(Block_Context_IR *ctx, String name);

#endif