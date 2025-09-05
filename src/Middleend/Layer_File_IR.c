#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_Statement.h>
#include <Middleend/Layer_Line.h>
#include <Wrapper/IO.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>


static void IR_dump_token(Block_Context_IR* ctx);

static void IR_dump_statement(Block_Context_IR *ctx);

static void IR_dump_code_block(Block_Context_IR *ctx);

// // ------------------------- INDIVIDUAL STATEMENT HANDLERS -------------------------

int IR__STMT_VARIABLE(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type==STMT_VAR);
	Var *v = ctx->next->statement.value.as_var;

	switch (v->mode) {
	case VAR_ACCS: {
		int id = get_var_id(v->name);
		print(NULL, WIN_IR, "\nPUSH   E_%d", id);
		return id;
	} break;

	case VAR_DECL: {
		int s = get_size_of_type(v->type);
		if (s) {
		print(NULL, WIN_IR, "\nE_%d:", ctx->n++);
		print(NULL, WIN_IR, "\nres(8)");
		push_var_def(v->name,v->type, ctx->n++);
		}
	} break;	
	case VAR_DEFN: {
		int id = get_var_id(v->name);
		int size = check_var_mutability(id);
		if (size) {
			print(NULL, WIN_IR, "\nPUSH   E_%d", ctx->n++);
			// get net line's output!
			print(NULL, WIN_IR, "\nWRITE%d", size);
		} else {
			assert(0 && "VARIABLE IS OF IMMUTABLE TYPE!!");
		}
	} break;
	case VAR_BOTH:
		if (compare_str(v->name, STR("main"))) {
			print(NULL, WIN_IR, "\n%%entry E_%d:");
		} else {
			print(NULL, WIN_IR, "\nE_%d:", ctx->n++);
		}

		push_var_def(v->name, v->type, ctx->n++);
		break;
	default:
		break;
	}

}

// int IR__STMT_UNKNOWN(int id)
// {
//     print(NULL, WIN_IR, "");
// 	return id;
// }

int IR__STMT_FUNCALL(Block_Context_IR* ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type==STMT_FUNCALL);

	const Funcall *funcall = ctx->next->statement.value.as_funcall;
	Block_Context_IR funcall_ctx = {0};
	funcall_ctx.n = ctx->n;
	funcall_ctx.b = ctx->b;
	funcall_ctx.next = funcall->args;
	funcall_ctx.prev = ctx;
	funcall_ctx.var_def_cnt=0;

    if (compare_str(funcall->name,STR("write"))) {
        IR_dump_statement(&funcall_ctx);
        print(NULL,WIN_IR,"\nSETR E_%d [L0]", funcall_ctx.n++);
        print(NULL,WIN_IR,"\nSETR len(E_%d) [QT]", funcall_ctx.n++);
        print(NULL,WIN_IR,"\nINVOK 7");
    }

	for (; funcall_ctx.next != NULL; funcall_ctx.next = funcall_ctx.next->next) {
		IR_dump_statement(&funcall_ctx);
	}
	ctx->n = funcall_ctx.n;
}

static void IR__STMT_BLOCK(Block_Context_IR* ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type==STMT_BLOCK_START);

    print(NULL, WIN_IR, "\n%%scope");

	Block_Context_IR blk_ctx = {0};
	blk_ctx.n = ctx->n;
	blk_ctx.b = ctx->b;
	blk_ctx.next = ctx->next;
	blk_ctx.prev = ctx;
	blk_ctx.var_def_cnt=0;

	IR_dump_code_block(&blk_ctx);

	ctx->n = blk_ctx.n;

    print(NULL, WIN_IR, "\n%%end");

}

// // ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

static void IR_dump_token(Block_Context_IR* ctx)
{
	assert(ctx->next->statement.type==STMT_TOKEN);
	const Token tok = *ctx->next->statement.value.as_token;

	print(NULL, WIN_IR, "\nE_%d:\nPUSH \"%.*s\"", 
		ctx->n++, Str_Fmt(tok.text));

}

static void IR_dump_statement(Block_Context_IR *ctx)
{
	assert(ctx->next != NULL);

	switch (ctx->next->statement.type) {
	case STMT_VAR: 			IR__STMT_VARIABLE	(ctx);break;
	case STMT_BLOCK_END:
	case STMT_TOKEN:		IR_dump_token	    (ctx);break;
	case STMT_FUNCALL:		IR__STMT_FUNCALL	    (ctx);break;
	case STMT_BLOCK_START: 	IR__STMT_BLOCK		(ctx);break;
	default: 				IR__STMT_UNKNOWN	    (ctx);break;
	}
}

static void IR_dump_code_block(Block_Context_IR *ctx)
{
	assert(ctx);
	for (; ctx->next != NULL; ctx->next = ctx->next->next) {
		IR_dump_statement(ctx);
	}
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

Error IR_generate(const CodeBlock *blk)
{
	assert(blk);
	Block_Context_IR ctx = {0};
	ctx.b=0;
	ctx.n=0;
	ctx.prev = NULL;
	ctx.next = blk->begin;
	ctx.var_def_cnt=0;

	IR_dump_code_block(&ctx);
	print(NULL, WIN_IR, "\nSHUTS");

	return ERR_OK;
}

// void IGNORE_ERRORS()
// {
// 	;
// }