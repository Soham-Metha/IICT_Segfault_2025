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

// ------------------------- INDIVIDUAL STATEMENT HANDLERS -------------------------

void IR__STMT_VARIABLE(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type==STMT_VAR);
	Var *v = ctx->next->statement.value.as_var;

	switch (v->mode) {
	case VAR_ACCS: {
		int id = get_var_id(v->name);
		ctx->b = id;
	} break;

	case VAR_DECL: {
		int s = get_size_of_type(v->type);
		if (s) {
		print(NULL, WIN_IR, IR_FORMAT "E_%d:", IR_CTX(), ctx->n++);
		print(NULL, WIN_IR, IR_FORMAT "res(8)", IR_CTX());
		push_var_def(v->name,v->type, ctx->n-1);
		}
	} break;
	case VAR_DEFN: {
		int id = get_var_id(v->name);
		int size = check_var_mutability(id);
		if (size) {
			print(NULL, WIN_IR, IR_FORMAT "PUSH    E_%d", IR_CTX(), ctx->n++);
			// get net line's output!
			print(NULL, WIN_IR, IR_FORMAT "WRITE%d", IR_CTX(), size);
		} else {
			assert(0 && "VARIABLE IS OF IMMUTABLE TYPE!!");
		}
	} break;
	case VAR_BOTH:
	int id = ctx->n++;
		if (compare_str(v->name, STR("main"))) {
			print(NULL, WIN_IR, IR_FORMAT "%%entry   E_%d: ; main", IR_CTX(), id);
		} else {
			print(NULL, WIN_IR, IR_FORMAT "E_%d:            ; var: %.*s", IR_CTX(), id, Str_Fmt(v->name));
		}
		push_var_def(v->name, v->type, id);
		ctx->next = ctx->next->next;
		IR_dump_statement(ctx);
		break;
	default:
		break;
	}

}

void IR__STMT_FUNCALL(Block_Context_IR* ctx)
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
        print(NULL,WIN_IR,IR_FORMAT "SETR    E_%d      [L0]", IR_CTX(), funcall_ctx.b);
        print(NULL,WIN_IR,IR_FORMAT "SETR    len(E_%d) [QT]", IR_CTX(), funcall_ctx.b);
        print(NULL,WIN_IR,IR_FORMAT "INVOK   7", IR_CTX());
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
	StmtNode* nxt = ctx->next->statement.value.as_block;
    print(NULL, WIN_IR, IR_FORMAT "%%scope", IR_CTX());
	update_indent(1);

	Block_Context_IR blk_ctx = {0};
	blk_ctx.n = ctx->n;
	blk_ctx.b = ctx->b;
	blk_ctx.next = nxt;
	blk_ctx.prev = ctx;
	blk_ctx.var_def_cnt=0;

	IR_dump_code_block(&blk_ctx);

	ctx->n = blk_ctx.n;

	update_indent(-1);
    print(NULL, WIN_IR, IR_FORMAT "%%end", IR_CTX());

}

// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

static void IR_dump_token(Block_Context_IR* ctx)
{
	assert(ctx->next->statement.type==STMT_TOKEN);
	const Token tok = *ctx->next->statement.value.as_token;
	if (tok.type != TOKEN_TYPE_STATEMENT_END ) {
	update_indent(1);
	print(NULL, WIN_IR, IR_FORMAT "PUSH    \"%.*s\"", IR_CTX(), Str_Fmt(tok.text));
	update_indent(-1);
	} else {
		print(NULL, WIN_IR, IR_FORMAT "%.*s Line end reached", IR_CTX(), 
			Str_Fmt(tok.text));
	}
}

static void IR_dump_statement(Block_Context_IR *ctx)
{
	assert(ctx->next != NULL);

	switch (ctx->next->statement.type) {
	case STMT_VAR: 			IR__STMT_VARIABLE	(ctx);break;
	case STMT_BLOCK_END:
	case STMT_TOKEN:		IR_dump_token	    (ctx);break;
	case STMT_FUNCALL:		IR__STMT_FUNCALL	(ctx);break;
	case STMT_BLOCK_START: 	IR__STMT_BLOCK		(ctx);break;
	default: 				break;
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
	print(NULL, WIN_IR, IR_FORMAT "SHUTS", IR_CTX());

	return ERR_OK;
}
