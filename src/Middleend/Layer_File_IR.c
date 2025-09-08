#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_Statement.h>
#include <Middleend/Layer_Line.h>
#include <Wrapper/IO.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

static void IR_dump_expr(Block_Context_IR *ctx, Expr tok, bool asVal);

static void IR_dump_statement(Block_Context_IR *ctx);

static void IR_dump_code_block(Block_Context_IR *ctx);

// ------------------------- INDIVIDUAL STATEMENT HANDLERS -------------------------

void IR__STMT_VAR_DECL(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type == STMT_VAR_DECL);
	VarDecl *v = &ctx->next->statement.as.var_decl;

	int s = get_type_details_from_type_name(v->type).size;
	if (v->has_init) {
		int id = ctx->n++;
		push_var_def(ctx, v->name, v->type, id);
		StmtNode nxt =
			(StmtNode){ .next = NULL, .statement = *v->init };
		Block_Context_IR blk_ctx = { .n = ctx->n,
					     .b = 999,
					     .next = &nxt,
					     .prev = ctx,
					     .var_def_cnt = 0 };

		print_IR(IR_FORMAT("; var:    %.*s ", Str_Fmt(v->name)));
		switch (get_type_details_from_type_name(v->type).type) {
		case VAR_TYPE_FUNC: {
			print_IR(IR_FORMAT("", "none"));
			if (compare_str(v->name, STR("main"))) {
				print_IR("%%entry    ");
			}
			print_IR("E_%d: ", id);
		} break;
		case VAR_TYPE_STR: {
			print_IR(IR_FORMAT("%%bind    E_%d    ", id));
		} break;
		case VAR_TYPE_COUNT:
		default:
			break;
		}
		IR_dump_statement(&blk_ctx);
		ctx->n = blk_ctx.n;
	} else if (s) {
		int id = ctx->n++;
		push_var_def(ctx, v->name, v->type, id);
	} else {
		assert(0 &&
		       "cant just declare an immutable type, needs a definition");
	}
}

void IR__STMT_VAR_DEFN(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type == STMT_VAR_DEFN);
	VarDefn *v = &ctx->next->statement.as.var_defn;
	int id = get_var_details(ctx, v->name).mem_addr;
	int size = get_type_details_from_type_id(
			   get_var_details(ctx, v->name).type)
			   .size;
	if (size) {
		assert(0 && "not supported yet");
		print_IR(IR_FORMAT("PUSH    E_%d", id));
		IR_dump_statement(ctx);
		print_IR(IR_FORMAT("WRITE%d     ", size));
	} else if (id) {
		print_IR(IR_FORMAT("; var:    %.*s ", Str_Fmt(v->name)));
		print_IR(IR_FORMAT("%%bind    %d    ", id));
		int tmp = ctx->b;
		ctx->b = 999;
		IR_dump_statement(ctx);
		ctx->b = tmp;
	} else {
		assert(0 && "VARIABLE IS OF IMMUTABLE TYPE!!");
	}
}

void IR__STMT_FUNCALL(Block_Context_IR *ctx, const Funcall *funcall)
{
	if (compare_str(funcall->name, STR("write"))) {
		IR_dump_expr(ctx, funcall->args->expr, false);
		print_IR(IR_FORMAT("CALL    write_str", "none"));
	}

	// for (const FuncallArg *arg = funcall->args; arg != NULL;
	//      arg = arg->next) {
	// 	IR_dump_expr(arg->expr);
	// }
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

	print_IR(IR_FORMAT("E_%d:\n; start of cond", cond_id));
	update_indent(1);
	IR_dump_expr(ctx, cond->cond, false);
	print_IR(IR_FORMAT("NOT", "none"));
	update_indent(-1);
	print_IR(IR_FORMAT("JMPC    E_%d", body_end_id));
	print_IR(IR_FORMAT("E_%d:\n; before body", body_id));

	print_IR(IR_FORMAT("%%scope", "none"));
	IR_dump_code_block(&blk_ctx);
	print_IR(IR_FORMAT("%%end", "none"));

	if (cond->repeat) {
		print_IR(IR_FORMAT("JMPU    E_%d", cond_id));
	}

	print_IR(IR_FORMAT("E_%d:\n; end of cond", body_end_id));

	ctx->n = blk_ctx.n;
}
// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

static void IR_dump_expr(Block_Context_IR *ctx, Expr expr, bool as_val)
{
	// print_IR(IR_FORMAT(";;;        %d",
	//       expr.type);
	switch (expr.type) {
	case EXPR_TYPE_STR:
		if (as_val) {
			print_IR("\"%.*s\"", Str_Fmt(expr.as.str));
			break;
		}
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
		int id = get_var_details(ctx, expr.as.var_nm).mem_addr;
		print_IR(IR_FORMAT("PUSH    E_%d", id));
		print_IR(IR_FORMAT("PUSH    len(E_%d)", id));
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
		IR_dump_expr(ctx, stmt.as.expr, ctx->b == 999);
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
