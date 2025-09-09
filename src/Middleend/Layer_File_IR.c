#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_Statement.h>
#include <Middleend/Layer_Line.h>
#include <Wrapper/IO.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

typedef struct {
	bool allowed;
	const char *inst;
	varType ret;
} BinOprInstLUT;

static const BinOprInstLUT bin_opr_inst_LUT[VAR_TYPE_CNT][BIN_OPR_CNT] = {
	[VAR_TYPE_FUNC] = { 
			    [BIN_OPR_AND]   = { .allowed = false },
			    [BIN_OPR_OR]    = { .allowed = false },
			    [BIN_OPR_LT]    = { .allowed = false },
			    [BIN_OPR_GE]    = { .allowed = false },
			    [BIN_OPR_NE]    = { .allowed = false },
			    [BIN_OPR_EQ]    = { .allowed = false },
			    [BIN_OPR_PLUS]  = { .allowed = false },
			    [BIN_OPR_MINUS] = { .allowed = false },
			    [BIN_OPR_MULT]  = { .allowed = false } },
	[VAR_TYPE_STR] = { 
			   [BIN_OPR_AND]   = { .allowed = false },
			   [BIN_OPR_OR]    = { .allowed = false },
			   [BIN_OPR_LT]    = { .allowed = false },
			   [BIN_OPR_GE]    = { .allowed = false },
			   [BIN_OPR_NE]    = { .allowed = false },
			   [BIN_OPR_EQ]    = { .allowed = false },
			   [BIN_OPR_PLUS]  = { .allowed = false },
			   [BIN_OPR_MINUS] = { .allowed = false },
			   [BIN_OPR_MULT]  = { .allowed = false } },
	[VAR_TYPE_I64] = { 
			   [BIN_OPR_AND]   = { .allowed = false },
			   [BIN_OPR_OR]    = { .allowed = false },
			   [BIN_OPR_LT]    = { .allowed = true, .ret =VAR_TYPE_I64, .inst = "LTI" },
			   [BIN_OPR_GE]    = { .allowed = true, .ret =VAR_TYPE_I64, .inst = "GEI" },
			   [BIN_OPR_NE]    = { .allowed = true, .ret =VAR_TYPE_I64, .inst = "NEI" },
			   [BIN_OPR_EQ]    = { .allowed = true, .ret =VAR_TYPE_I64, .inst = "EQI" },
			   [BIN_OPR_PLUS]  = { .allowed = true, .ret =VAR_TYPE_I64, .inst = "SPOPR   [QT]\nSPOPR   [L2]\nADDI    val([QT])\nPUSHR    ref([L2])" },
			   [BIN_OPR_MINUS] = { .allowed = true, .ret =VAR_TYPE_I64, .inst = "SPOPR   [QT]\nSPOPR   [L2]\nSUBI    val([QT])\nPUSHR    ref([L2])" },
			   [BIN_OPR_MULT]  = { .allowed = true, .ret =VAR_TYPE_I64, .inst = "SPOPR   [QT]\nSPOPR   [L2]\nMULI    val([QT])\nPUSHR    ref([L2])" } },
};

static varType IR_dump_expr(Block_Context_IR *ctx, Expr tok);

static void IR_dump_statement(Block_Context_IR *ctx);

static void IR_dump_code_block(Block_Context_IR *ctx);

// ------------------------- INDIVIDUAL STATEMENT HANDLERS -------------------------

varType dump_var_accs(const Block_Context_IR *ctx, String var_nm)
{
	int id = get_var_details(ctx, var_nm).mem_addr;
	switch (get_var_details(ctx, var_nm).type) {
	case VAR_TYPE_STR: {
		print_IR(IR_FORMAT("PUSH    E_%d    ", id));
		print_IR(IR_FORMAT("READ8U          ", ""));
		print_IR(IR_FORMAT("PUSH    E_%d_len", id));
		print_IR(IR_FORMAT("READ8U          ", ""));
		return VAR_TYPE_STR;
	} break;
	case VAR_TYPE_I64: {
		print_IR(IR_FORMAT("PUSH    E_%d    ", id));
		print_IR(IR_FORMAT("READ8I          ", id));
		return VAR_TYPE_I64;
	} break;
	case VAR_TYPE_FUNC:
		return VAR_TYPE_FUNC;
	case VAR_TYPE_VOID:
		return VAR_TYPE_VOID;
	default:
		break;
	}
	return VAR_TYPE_VOID;
}

varType dump_var_decl(String var_nm, String type, int id)
{
	switch (get_type_details_from_type_name(type).type) {
	case VAR_TYPE_FUNC:
		return VAR_TYPE_FUNC;
		break;
	case VAR_TYPE_STR: {
		print_IR(IR_FORMAT("; declaring var:   %.*s     ",
				   Str_Fmt(var_nm)));
		print_IR(IR_FORMAT("%%bind   E_%d        res(8)", id));
		print_IR(IR_FORMAT("%%bind   E_%d_len    res(8)", id));
		print_IR(IR_FORMAT(";--------------------------", ""));
		return VAR_TYPE_STR;
	} break;
	case VAR_TYPE_I64: {
		print_IR(IR_FORMAT("; declaring var:   %.*s     ",
				   Str_Fmt(var_nm)));
		print_IR(IR_FORMAT("%%bind   E_%d        res(8)", id));
		print_IR(IR_FORMAT(";--------------------------", ""));
		return VAR_TYPE_I64;
	} break;
	case VAR_TYPE_BOOL: {
		print_IR(IR_FORMAT("; declaring var:   %.*s     ",
				   Str_Fmt(var_nm)));
		print_IR(IR_FORMAT("%%bind   E_%d        res(1)", id));
		print_IR(IR_FORMAT(";--------------------------", ""));
		return VAR_TYPE_I64;
	} break;
	case VAR_TYPE_VOID:
		return VAR_TYPE_VOID;
	case VAR_TYPE_CNT:
	default:
		break;
	}
	return VAR_TYPE_VOID;
}

varType dump_var_defn(Block_Context_IR *ctx, String var_nm, varType type,
		      int id)
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
		return VAR_TYPE_FUNC;
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
		return VAR_TYPE_STR;
	} break;
	case VAR_TYPE_I64: {
		assert(get_var_details(ctx, var_nm).type == VAR_TYPE_I64);
		assert(ctx->next->statement.type == STMT_EXPR);
		// assert(ctx->next->statement.as.expr.type == EXPR_TYPE_NUMBER);
		int id = get_var_details(ctx, var_nm).mem_addr;
		print_IR(IR_FORMAT("PUSH   E_%d", id));
		print_IR(IR_FORMAT(";--------------------------", ""));
		IR_dump_statement(ctx);
		print_IR(IR_FORMAT(";--------------------------", ""));
		print_IR(IR_FORMAT("WRITE8  ", "none"));
		print_IR(IR_FORMAT(";--------------------------", ""));
		return VAR_TYPE_I64;
	} break;
	case VAR_TYPE_VOID:
		return VAR_TYPE_VOID;
	case VAR_TYPE_CNT:
	default:
		break;
	}
	return VAR_TYPE_VOID;
}

varType IR__STMT_VAR_DECL(Block_Context_IR *ctx)
{
	assert(ctx);
	assert(ctx->next->statement.type == STMT_VAR_DECL);
	VarDecl *v = &ctx->next->statement.as.var_decl;

	int id = ctx->n++;
	varType type;
	push_var_def(ctx, v->name, v->type, id);
	type = dump_var_decl(v->name, v->type, id);
	if (v->has_init) {
		StmtNode nxt =
			(StmtNode){ .next = NULL, .statement = *v->init };
		Block_Context_IR blk_ctx = { .n = ctx->n,
					     .b = ctx->b,
					     .next = &nxt,
					     .prev = ctx,
					     .var_def_cnt = 0 };

		dump_var_defn(&blk_ctx, v->name, type, id);
		ctx->n = blk_ctx.n;
	}
	return type;
}

varType IR__STMT_VAR_DEFN(Block_Context_IR *ctx)
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
	return type;
}

varType IR__STMT_FUNCALL(Block_Context_IR *ctx, const Funcall *funcall)
{
	if (compare_str(funcall->name, STR("write"))) {
		for (const FuncallArg *arg = funcall->args; arg != NULL;
		     arg = arg->next) {
			IR_dump_expr(ctx, funcall->args->expr);
			print_IR(IR_FORMAT("CALL    write_str", "none"));
		}
		return VAR_TYPE_FUNC;
	}
	return VAR_TYPE_VOID;
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

static varType IR_dump_expr(Block_Context_IR *ctx, Expr expr)
{
	// print_IR(IR_FORMAT(";;;        %d",
	//       expr.type);
	switch (expr.type) {
	case EXPR_TYPE_STR:
		print_IR(IR_FORMAT("PUSH    \"%.*s\"", Str_Fmt(expr.as.str)));
		print_IR(IR_FORMAT("PUSH    %d", expr.as.str.len));
		return VAR_TYPE_STR;
		break;
	case EXPR_TYPE_STATEMENT_END:
		print_IR("\n; Line end reached");
		assert(0 && "discard the statement end in the parsing!");
		break;
	case EXPR_TYPE_BOOL:
		print_IR(IR_FORMAT("PUSH    %d", (expr.as.boolean) ? 1 : 0));
		return VAR_TYPE_I64;
		break;

	case EXPR_TYPE_FUNCALL:
		IR__STMT_FUNCALL(ctx, &expr.as.funcall);
		return VAR_TYPE_FUNC;
		break;

	case EXPR_TYPE_NUMBER:
		print_IR(IR_FORMAT("PUSH    %d", expr.as.num));
		return VAR_TYPE_I64;
		break;
	case EXPR_TYPE_VAR:
		return dump_var_accs(ctx, expr.as.var_nm);
		break;
	case EXPR_TYPE_BIN_OPR:
		varType l = IR_dump_expr(ctx, expr.as.bin_opr->lhs);
		varType r = IR_dump_expr(ctx, expr.as.bin_opr->rhs);
		assert(l == r);
		assert(bin_opr_inst_LUT[l][expr.as.bin_opr->type].allowed);
		print_IR(IR_FORMAT("%s",
				   bin_opr_inst_LUT[l][expr.as.bin_opr->type].inst));
		return l;

	case EXPR_TYPE_THEN:
	case EXPR_TYPE_REPEAT:
	case EXPR_TYPE_OPEN_CURLY:
	case EXPR_TYPE_CLOSING_CURLY:
	case EXPR_TYPE_COLON:
	case EXPR_TYPE_EQUAL:
	default:
		break;
	}
	return VAR_TYPE_VOID;
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
