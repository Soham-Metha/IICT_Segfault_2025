#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_Statement.h>
#include <Middleend/Layer_Line.h>
#include <Wrapper/IO.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

static void IR_dump_expr(Expr tok);

static void IR_dump_statement(Block_Context_IR *ctx);

static void IR_dump_code_block(Block_Context_IR *ctx);

// ------------------------- INDIVIDUAL STATEMENT HANDLERS -------------------------

void IR__STMT_VARIABLE(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type == STMT_VAR);
	Var *v = &ctx->next->statement.as.var;

	switch (v->mode) {
	case VAR_ACCS: {
		int id = get_var_id(v->name);
		ctx->b = id;
	} break;

	case VAR_DECL: {
		int s = get_size_of_type(get_type_from_name(v->type));
		if (s) {
			int id = ctx->n++;
			print(NULL, WIN_IR, IR_FORMAT "; var:    %.*s ",
			      IR_CTX(), Str_Fmt(v->name));
			print(NULL, WIN_IR,
			      IR_FORMAT "%%bind   E_%d    res(%d) ", IR_CTX(),
			      id, s);
			push_var_def(v->name, v->type, id);
		}
	} break;

	case VAR_DEFN: {
		int id = get_var_id(v->name);
		int size = get_size_from_id(id);
		if (size) {
			print(NULL, WIN_IR, IR_FORMAT "PUSH    E_%d", IR_CTX(),
			      id);
			print(NULL, WIN_IR, IR_FORMAT "PUSH    ", IR_CTX());
			ctx->next = ctx->next->next;
			IR_dump_statement(ctx);
			print(NULL, WIN_IR, IR_FORMAT "WRITE%d     ", IR_CTX(),
			      size);
		} else {
			assert(0 && "VARIABLE IS OF IMMUTABLE TYPE!!");
		}
	} break;

	case VAR_BOTH:
		int id = ctx->n++;
		print(NULL, WIN_IR, IR_FORMAT "; var:    %.*s ", IR_CTX(),
		      Str_Fmt(v->name));
		if (compare_str(v->type, STR("func"))) {
			print(NULL, WIN_IR, IR_FORMAT, IR_CTX());
			if (compare_str(v->name, STR("main"))) {
				print(NULL, WIN_IR, "%%entry    ");
			}
			print(NULL, WIN_IR, "E_%d: ", id);
		} else {
			print(NULL, WIN_IR, IR_FORMAT "%%bind    E_%d    ",
			      IR_CTX(), id);
		}
		push_var_def(v->name, v->type, id);
		ctx->next = ctx->next->next;
		IR_dump_statement(ctx);
		break;
	default:
		break;
	}
}

void IR__STMT_FUNCALL(Block_Context_IR *ctx)
{
	assert(ctx);
	// assert(ctx->next->statement.type == STMT_FUNCALL);

	const Funcall *funcall = ctx->next->statement.as.funcall;

	if (compare_str(funcall->name, STR("write"))) {
		IR_dump_expr(funcall->args->expr);
		print(NULL, WIN_IR, IR_FORMAT "INVOK   7", IR_CTX());
	}

	for (const FuncallArg *arg = funcall->args; arg != NULL;
	     arg = arg->next) {
		IR_dump_expr(arg->expr);
	}
}

static void IR__STMT_BLOCK(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type == STMT_BLOCK_START);
	StmtNode *nxt = ctx->next->statement.as.block;
	print(NULL, WIN_IR, IR_FORMAT "%%scope", IR_CTX());
	update_indent(1);

	Block_Context_IR blk_ctx = { 0 };
	blk_ctx.n = ctx->n;
	blk_ctx.b = ctx->b;
	blk_ctx.next = nxt;
	blk_ctx.prev = ctx;
	blk_ctx.var_def_cnt = 0;

	IR_dump_code_block(&blk_ctx);

	ctx->n = blk_ctx.n;

	update_indent(-1);
	print(NULL, WIN_IR, IR_FORMAT "%%end", IR_CTX());
}

static void IR__STMT_CONDITIONAL(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type == STMT_CONDITIONAL);
	const StmtConditional *cond = &ctx->next->statement.as.cond;
	int cond_id, body_id, body_end_id;
	cond_id = ctx->n++;
	body_id = ctx->n++;
	body_end_id = ctx->n++;

	Block_Context_IR blk_ctx = { 0 };
	blk_ctx.n = ctx->n;
	blk_ctx.b = ctx->b;
	blk_ctx.prev = ctx;
	blk_ctx.var_def_cnt = 0;
	blk_ctx.next = cond->body.begin;

	print(NULL, WIN_IR, IR_FORMAT "E_%d:\n; start of cond", IR_CTX(),
	      cond_id);
	if (compare_str(cond->cond.as.token.text, STR("true"))) {
		print(NULL, WIN_IR, IR_FORMAT "PUSH    1", IR_CTX());
	} else if (compare_str(cond->cond.as.token.text, STR("false"))) {
		print(NULL, WIN_IR, IR_FORMAT "PUSH    0", IR_CTX());
	}
	print(NULL, WIN_IR, IR_FORMAT "PUSH    0", IR_CTX());
	print(NULL, WIN_IR, IR_FORMAT "EQI", IR_CTX());
	print(NULL, WIN_IR, IR_FORMAT "JMPC    E_%d", IR_CTX(), body_end_id);
	print(NULL, WIN_IR, IR_FORMAT "E_%d:\n; before body", IR_CTX(),
	      body_id);
	update_indent(1);

	IR_dump_code_block(&blk_ctx);

	if (cond->repeat) {
		print(NULL, WIN_IR, IR_FORMAT "JMPU    E_%d", IR_CTX(),
		      cond_id);
	}

	update_indent(-1);
	print(NULL, WIN_IR, IR_FORMAT "E_%d:\n; end of cond", IR_CTX(),
	      body_end_id);

	ctx->n = blk_ctx.n;
	ctx->next = ctx->next->next;
}
// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

static void IR_dump_expr(Expr expr)
{
	switch (expr.type) {
	case EXPR_TYPE_CHAR:
		print(NULL, WIN_IR, "'%.*s'", Str_Fmt(expr.as.token.text));
		break;
	case EXPR_TYPE_STR:
		print(NULL, WIN_IR, "\"%.*s\"", Str_Fmt(expr.as.token.text));
		break;
	case EXPR_TYPE_STATEMENT_END:
		print(NULL, WIN_IR, "; Line end reached");
		break;

	case EXPR_TYPE_THEN:
	case EXPR_TYPE_REPEAT:
	case EXPR_TYPE_NUMBER:
	case EXPR_TYPE_VAR:
	case EXPR_TYPE_OPEN_CURLY:
	case EXPR_TYPE_CLOSING_CURLY:
	case EXPR_TYPE_COLON:
	case EXPR_TYPE_EQUAL:
	case EXPR_TYPE_OPEN_PAREN:
	case EXPR_TYPE_CLOSING_PAREN:
	case EXPR_TYPE_TOKEN:
	case EXPR_TYPE_FUNCALL:
	default:
		break;
	}
}

static void IR_dump_statement(Block_Context_IR *ctx)
{
	assert(ctx->next != NULL);

	switch (ctx->next->statement.type) {
	case STMT_VAR:
		IR__STMT_VARIABLE(ctx);
		break;
	case STMT_BLOCK_END:
	case STMT_TOKEN:
		IR_dump_expr(ctx->next->statement.as.token);
		break;
	// case STMT_FUNCALL:
	// 	IR__STMT_FUNCALL(ctx);
	// 	break;
	case STMT_BLOCK_START:
		IR__STMT_BLOCK(ctx);
		break;
	case STMT_CONDITIONAL:
		IR__STMT_CONDITIONAL(ctx);
		break;
	case STMT_MATCH:
	default:
		break;
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
	Block_Context_IR ctx = { 0 };
	ctx.b = 0;
	ctx.n = 0;
	ctx.prev = NULL;
	ctx.next = blk->begin;
	ctx.var_def_cnt = 0;

	IR_dump_code_block(&ctx);
	print(NULL, WIN_IR, IR_FORMAT "SHUTS", IR_CTX());

	return ERR_OK;
}
