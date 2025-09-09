#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_Statement.h>
#include <Middleend/Layer_Line.h>
#include <Wrapper/IO.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

static void IR_dump_expr(Block_Context_IR *ctx, Expr tok);

static void IR_dump_statement(Block_Context_IR *ctx);

static void IR_dump_code_block(Block_Context_IR *ctx);

// ------------------------- INDIVIDUAL STATEMENT HANDLERS -------------------------

void dump_var_accs(const Block_Context_IR *ctx, String var_nm)
{
	int id = get_var_details(ctx, var_nm).mem_addr;
	switch (get_var_details(ctx, var_nm).type) {
	case VAR_TYPE_STR: {
		print_IR(IR_FORMAT("PUSH    E_%d    ", id));
		print_IR(IR_FORMAT("READ8U          ", ""));
		print_IR(IR_FORMAT("PUSH    E_%d_len", id));
		print_IR(IR_FORMAT("READ8U          ", ""));
	} break;
	case VAR_TYPE_I64: {
		print_IR(IR_FORMAT("PUSH    E_%d    ", id));
	} break;
	case VAR_TYPE_FUNC:
	case VAR_TYPE_COUNT:
	default:
		break;
	}
}

void dump_var_decl(String var_nm, String type, int id)
{
	switch (get_type_details_from_type_name(type).type) {
	case VAR_TYPE_FUNC:
		break;
	case VAR_TYPE_STR: {
		print_IR(IR_FORMAT("; declaring var:   %.*s     ",
				   Str_Fmt(var_nm)));
		print_IR(IR_FORMAT("%%bind   E_%d        res(8)", id));
		print_IR(IR_FORMAT("%%bind   E_%d_len    res(8)", id));
		print_IR(IR_FORMAT(";--------------------------", ""));
	} break;
	case VAR_TYPE_I64: {
		print_IR(IR_FORMAT("; declaring var:   %.*s     ",
				   Str_Fmt(var_nm)));
		print_IR(IR_FORMAT("%%bind   E_%d        res(8)", id));
		print_IR(IR_FORMAT(";--------------------------", ""));
	} break;
	case VAR_TYPE_COUNT:
	default:
		break;
	}
}

void dump_var_defn(Block_Context_IR *ctx, String var_nm, varType type, int id)
{
	print_IR(IR_FORMAT("; defining var:    %.*s     ", Str_Fmt(var_nm)));
	switch (type) {
	case VAR_TYPE_FUNC: {
		if (compare_str(var_nm, STR("main"))) {
			print_IR(IR_FORMAT("%%entry    E_%d: ", id));
		} else {
			print_IR(IR_FORMAT("E_%d: ", id));
		}
		IR_dump_statement(ctx);
		print_IR(IR_FORMAT(";--------------------------", ""));
	} break;
	case VAR_TYPE_STR: {
		assert(get_var_details(ctx, var_nm).type == VAR_TYPE_STR);
		assert(ctx->next->statement.type == STMT_EXPR);
		assert(ctx->next->statement.as.expr.type == EXPR_TYPE_STR);
		int id = get_var_details(ctx, var_nm).mem_addr;
		print_IR(IR_FORMAT("PUSH   E_%d", id));
		print_IR(IR_FORMAT("PUSH   E_%d_len", id));
		print_IR(IR_FORMAT(";--------------------------", ""));
		IR_dump_statement(ctx);
		print_IR(IR_FORMAT(";--------------------------", ""));
		print_IR(IR_FORMAT("SWAP   1", "none"));
		print_IR(IR_FORMAT("SWAP   2", "none"));
		print_IR(IR_FORMAT("SWAP   1", "none"));
		print_IR(IR_FORMAT("WRITE8  ", "none"));
		print_IR(IR_FORMAT("WRITE8  ", "none"));
		print_IR(IR_FORMAT(";--------------------------", ""));
	} break;
	case VAR_TYPE_I64: {
		assert(get_var_details(ctx, var_nm).type == VAR_TYPE_I64);
		assert(ctx->next->statement.type == STMT_EXPR);
		assert(ctx->next->statement.as.expr.type == EXPR_TYPE_NUMBER);
		int id = get_var_details(ctx, var_nm).mem_addr;
		print_IR(IR_FORMAT("PUSH   E_%d", id));
		print_IR(IR_FORMAT(";--------------------------", ""));
		IR_dump_statement(ctx);
		print_IR(IR_FORMAT(";--------------------------", ""));
		print_IR(IR_FORMAT("WRITE8  ", "none"));
		print_IR(IR_FORMAT(";--------------------------", ""));
	} break;
	case VAR_TYPE_COUNT:
	default:
		break;
	}
}

void IR__STMT_VAR_DECL(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type == STMT_VAR_DECL);
	VarDecl *v = &ctx->next->statement.as.var_decl;

	int id = ctx->n++;
	push_var_def(ctx, v->name, v->type, id);
	dump_var_decl(v->name, v->type, id);
	if (v->has_init) {
		StmtNode nxt =
			(StmtNode){ .next = NULL, .statement = *v->init };
		Block_Context_IR blk_ctx = { .n = ctx->n,
					     .b = ctx->b,
					     .next = &nxt,
					     .prev = ctx,
					     .var_def_cnt = 0 };

		varType type = get_var_details(ctx, v->name).type;
		dump_var_defn(&blk_ctx, v->name, type, id);
		ctx->n = blk_ctx.n;
	}
}

void IR__STMT_VAR_DEFN(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type == STMT_VAR_DEFN);
	VarDefn *v = &ctx->next->statement.as.var_defn;
	varType type = get_var_details(ctx, v->name).type;
	int id = get_var_details(ctx, v->name).mem_addr;

	StmtNode nxt = (StmtNode){ .next = NULL, .statement = *v->val };
	Block_Context_IR blk_ctx = { .n = ctx->n,
				     .b = ctx->b,
				     .next = &nxt,
				     .prev = ctx,
				     .var_def_cnt = 0 };
	dump_var_defn(&blk_ctx, v->name, type, id);
}

void IR__STMT_FUNCALL(Block_Context_IR *ctx, const Funcall *funcall)
{
	if (compare_str(funcall->name, STR("write"))) {
		for (const FuncallArg *arg = funcall->args; arg != NULL;
		     arg = arg->next) {
			assert((funcall->args->expr.type = EXPR_TYPE_STR) ||
			       (funcall->args->expr.type == EXPR_TYPE_VAR));
			IR_dump_expr(ctx, funcall->args->expr);
			print_IR(IR_FORMAT("CALL    write_str", "none"));
		}
	}
}

static void IR__STMT_BLOCK(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type == STMT_BLOCK_START);
	StmtNode *nxt = ctx->next->statement.as.block;

	Block_Context_IR blk_ctx = { 0 };
	blk_ctx.n = ctx->n;
	blk_ctx.b = ctx->b;
	blk_ctx.next = nxt;
	blk_ctx.prev = ctx;
	blk_ctx.var_def_cnt = 0;

	print_IR(IR_FORMAT("%%scope", "none"));
	IR_dump_code_block(&blk_ctx);
	print_IR(IR_FORMAT("%%end", "none"));

	ctx->n = blk_ctx.n;
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

	print_IR(IR_FORMAT("E_%d:                       ", cond_id));
	print_IR(IR_FORMAT(";cond start                ", ""));
	print_IR(IR_FORMAT(";--------------------------", ""));
	update_indent(1);
	IR_dump_expr(ctx, cond->cond);
	print_IR(IR_FORMAT("NOT                        ", ""));
	print_IR(IR_FORMAT("JMPC    E_%d                ", body_end_id));
	update_indent(-1);
	print_IR(IR_FORMAT(";--------------------------", ""));
	print_IR(IR_FORMAT("E_%d:                       ", body_id));
	print_IR(IR_FORMAT(";body start                ", ""));
	print_IR(IR_FORMAT(";--------------------------", ""));
	print_IR(IR_FORMAT("%%scope                    ", ""));
	IR_dump_code_block(&blk_ctx);
	if (cond->repeat) {
		print_IR(IR_FORMAT("JMPU    E_%d", cond_id));
	}
	print_IR(IR_FORMAT("%%end                       ", ""));
	print_IR(IR_FORMAT(";--------------------------", ""));
	print_IR(IR_FORMAT("E_%d:                       ", body_end_id));
	print_IR(IR_FORMAT(";body end                  ", ""));
	print_IR(IR_FORMAT(";--------------------------", ""));

	ctx->n = blk_ctx.n;
}
// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

static void IR_dump_expr(Block_Context_IR *ctx, Expr expr)
{
	// print_IR(IR_FORMAT(";;;        %d",
	//       expr.type);
	switch (expr.type) {
	case EXPR_TYPE_STR:
		print_IR(IR_FORMAT("PUSH    \"%.*s\"", Str_Fmt(expr.as.str)));
		print_IR(IR_FORMAT("PUSH    %d", expr.as.str.len));
		break;
	case EXPR_TYPE_STATEMENT_END:
		print_IR("\n; Line end reached");
		break;
	case EXPR_TYPE_BOOL:
		print_IR(IR_FORMAT("PUSH    %d", (expr.as.boolean) ? 1 : 0));
		break;

	case EXPR_TYPE_FUNCALL:
		IR__STMT_FUNCALL(ctx, &expr.as.funcall);
		break;

	case EXPR_TYPE_NUMBER:
		print_IR(IR_FORMAT("PUSH    %d", expr.as.num));
		break;
	case EXPR_TYPE_VAR:
		dump_var_accs(ctx, expr.as.var_nm);
		break;
	case EXPR_TYPE_THEN:
	case EXPR_TYPE_REPEAT:
	case EXPR_TYPE_OPEN_CURLY:
	case EXPR_TYPE_CLOSING_CURLY:
	case EXPR_TYPE_COLON:
	case EXPR_TYPE_EQUAL:
	case EXPR_TYPE_BIN_OPR:
	default:
		break;
	}
}

static void IR_dump_statement(Block_Context_IR *ctx)
{
	assert(ctx->next != NULL);
	Stmt stmt = ctx->next->statement;
	// print_IR(IR_FORMAT(";;        %d",
	//       ctx->next->statement.type);
	switch (stmt.type) {
	case STMT_BLOCK_START:
		IR__STMT_BLOCK(ctx);
		break;
	case STMT_CONDITIONAL:
		IR__STMT_CONDITIONAL(ctx);
		break;
	case STMT_EXPR:
		IR_dump_expr(ctx, stmt.as.expr);
		break;
	case STMT_VAR_DECL:
		IR__STMT_VAR_DECL(ctx);
		break;
	case STMT_VAR_DEFN:
		IR__STMT_VAR_DEFN(ctx);
		break;
	case STMT_MATCH:
	case STMT_BLOCK_END:
	default:
		break;
	}
	ctx->next = ctx->next->next;
}

static void IR_dump_code_block(Block_Context_IR *ctx)
{
	assert(ctx);
	update_indent(1);
	for (; ctx->next != NULL;) {
		IR_dump_statement(ctx);
	}
	update_indent(-1);
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

	print_IR(
		"write_str:\nDUPS     1\nSPOPR    [QT]\nDUPS     2\nSPOPR    [L0]\nINVOK    7\nRET");
	update_indent(-1);
	IR_dump_code_block(&ctx);
	update_indent(1);
	print_IR(IR_FORMAT("SHUTS", "none"));

	return ERR_OK;
}
