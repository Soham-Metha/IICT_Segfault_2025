#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_Statement.h>
#include <Middleend/Layer_Line.h>
#include <Wrapper/IO.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

static void IR_dump_expr(Expr tok, bool asVal);

static void IR_dump_statement(Block_Context_IR *ctx);

static void IR_dump_code_block(Block_Context_IR *ctx);

// ------------------------- INDIVIDUAL STATEMENT HANDLERS -------------------------

void IR__STMT_VAR_DECL(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type == STMT_VAR_DECL);
	VarDecl *v = &ctx->next->statement.as.var_decl;

	int s = get_size_of_type(get_type_from_name(v->type));
	if (v->has_init) {
		int id = ctx->n++;
		print(NULL, WIN_IR, IR_FORMAT "; var:    %.*s ", IR_CTX(),
		      Str_Fmt(v->name));
		if (compare_str(v->type, STR("func"))) {
			print(NULL, WIN_IR, IR_FORMAT, IR_CTX());
			if (compare_str(v->name, STR("main"))) {
				print(NULL, WIN_IR, "%%entry    ");
				update_indent(-1);
			}
			print(NULL, WIN_IR, "E_%d: ", id);
		} else {
			print(NULL, WIN_IR, IR_FORMAT "%%bind E_%d    ", IR_CTX(), id);
		}
		push_var_def(v->name, v->type, id);
		StmtNode nxt =
			(StmtNode){ .next = NULL, .statement = *v->init };
		Block_Context_IR blk_ctx = { 0 };
		blk_ctx.n = ctx->n;
		blk_ctx.b = 999;
		blk_ctx.next = &nxt;
		blk_ctx.prev = ctx;
		blk_ctx.var_def_cnt = 0;
		update_indent(1);
		IR_dump_statement(&blk_ctx);
		update_indent(-1);
		ctx->n = blk_ctx.n;
	} else if (s) {
		int id = ctx->n++;
		// print(NULL, WIN_IR, IR_FORMAT "; var:    %.*s ", IR_CTX(),
		//       Str_Fmt(v->name));
		// print(NULL, WIN_IR, IR_FORMAT "E_%d:", IR_CTX(), id);
		push_var_def(v->name, v->type, id);
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
	int id = get_var_id(v->name);
	int size = get_size_from_id(id);
	if (size) {
		assert(0 && "not supported yet");
		print(NULL, WIN_IR, IR_FORMAT "PUSH    E_%d", IR_CTX(), id);
		IR_dump_statement(ctx);
		print(NULL, WIN_IR, IR_FORMAT "WRITE%d     ", IR_CTX(), size);
	} else if (id) {
		print(NULL, WIN_IR, IR_FORMAT "; var:    %.*s ", IR_CTX(),
		      Str_Fmt(v->name));
		print(NULL, WIN_IR, IR_FORMAT "%%bind    %d    ", IR_CTX(), id);
		int tmp = ctx->b;
		ctx->b = 999;
		IR_dump_statement(ctx);
		ctx->b = tmp;
	} else {
		assert(0 && "VARIABLE IS OF IMMUTABLE TYPE!!");
	}
}

void IR__STMT_FUNCALL(const Funcall *funcall)
{
	if (compare_str(funcall->name, STR("write"))) {
		IR_dump_expr(funcall->args->expr, false);
		print(NULL, WIN_IR, IR_FORMAT "CALL    write_str", IR_CTX());
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

	print(NULL, WIN_IR, IR_FORMAT "%%scope", IR_CTX());
	update_indent(1);
	IR_dump_code_block(&blk_ctx);
	update_indent(-1);
	print(NULL, WIN_IR, IR_FORMAT "%%end", IR_CTX());

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

	print(NULL, WIN_IR, IR_FORMAT "E_%d:\n; start of cond", IR_CTX(),
	      cond_id);
	update_indent(1);
	IR_dump_expr(cond->cond, false);
	print(NULL, WIN_IR, IR_FORMAT "PUSH    0", IR_CTX());
	print(NULL, WIN_IR, IR_FORMAT "EQI", IR_CTX());
	update_indent(-1);
	print(NULL, WIN_IR, IR_FORMAT "JMPC    E_%d", IR_CTX(), body_end_id);
	print(NULL, WIN_IR, IR_FORMAT "E_%d:\n; before body", IR_CTX(),
	      body_id);
	update_indent(1);

	print(NULL, WIN_IR, IR_FORMAT "%%scope", IR_CTX());
	update_indent(1);
	IR_dump_code_block(&blk_ctx);
	update_indent(-1);
	print(NULL, WIN_IR, IR_FORMAT "%%end", IR_CTX());

	if (cond->repeat) {
		print(NULL, WIN_IR, IR_FORMAT "JMPU    E_%d", IR_CTX(),
		      cond_id);
	}

	update_indent(-1);
	print(NULL, WIN_IR, IR_FORMAT "E_%d:\n; end of cond", IR_CTX(),
	      body_end_id);

	ctx->n = blk_ctx.n;
}
// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

static void IR_dump_expr(Expr expr, bool as_val)
{
	// print(NULL, WIN_IR, IR_FORMAT ";;;        %d", IR_CTX(),
	//       expr.type);
	switch (expr.type) {
	case EXPR_TYPE_STR:
		if (as_val) {
			print(NULL, WIN_IR, "\"%.*s\"", Str_Fmt(expr.as.str));
			break;
		}
		print(NULL, WIN_IR, IR_FORMAT "PUSH    \"%.*s\"", IR_CTX(),
		      Str_Fmt(expr.as.str));
		print(NULL, WIN_IR, IR_FORMAT "PUSH    %d", IR_CTX(),
		      expr.as.str.len);
		break;
	case EXPR_TYPE_STATEMENT_END:
		print(NULL, WIN_IR, "\n; Line end reached");
		break;
	case EXPR_TYPE_BOOL:
		print(NULL, WIN_IR, IR_FORMAT "PUSH    %d", IR_CTX(),
		      (expr.as.boolean) ? 1 : 0);
		break;

	case EXPR_TYPE_FUNCALL:
		IR__STMT_FUNCALL(&expr.as.funcall);
		break;

	case EXPR_TYPE_NUMBER:
		print(NULL, WIN_IR, IR_FORMAT "PUSH    %d", IR_CTX(),
		      expr.as.num);
		break;
	case EXPR_TYPE_VAR:
		int id = get_var_id(expr.as.var_nm);
		print(NULL, WIN_IR, IR_FORMAT "CALL    E_%d", IR_CTX(), id);
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
	// print(NULL, WIN_IR, IR_FORMAT ";;        %d", IR_CTX(),
	//       ctx->next->statement.type);
	switch (stmt.type) {
	case STMT_BLOCK_START:
		IR__STMT_BLOCK(ctx);
		break;
	case STMT_CONDITIONAL:
		IR__STMT_CONDITIONAL(ctx);
		break;
	case STMT_EXPR:
		IR_dump_expr(stmt.as.expr, ctx->b == 999);
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
	for (; ctx->next != NULL;) {
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

	print(NULL, WIN_IR,
	      "write_str:\nDUPS     1\nSPOPR    [QT]\nDUPS     2\nSPOPR    [L0]\nINVOK    7\nRET");
	IR_dump_code_block(&ctx);
	print(NULL, WIN_IR, IR_FORMAT "SHUTS", IR_CTX());

	return ERR_OK;
}
