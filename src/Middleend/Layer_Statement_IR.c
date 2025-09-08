// #include <Frontend/Layer_File.h>
// #include <Frontend/Layer_Line.h>
// #include <Frontend/Layer_Statement.h>
// #include <Middleend/Layer_Line.h>
// #include <Wrapper/IO.h>
// #include <stdlib.h>
// #include <inttypes.h>
// #include <assert.h>

// static void dump_var_decl(int id, VarDecl decl)
// {
// 	TypeDetailsLUT dets = get_type_details_from_type_name(decl.type);
// 	print(NULL, WIN_IR, IR_FORMAT "; var:    %.*s ", IR_CTX(),
// 	      Str_Fmt(decl.name));
//     if (dets.type==VAR_TYPE_FUNC) {
//         assert(decl.has_init);
//     }
// 	print(NULL, WIN_IR, IR_FORMAT "%%bind    E_%d    res(%d)", IR_CTX(), id,
// 	      get_type_details_from_type_name(dets.name).size);

	
// }

// void IR__STMT_FUNCALL(const Funcall *funcall)
// {
// 	if (compare_str(funcall->name, STR("write"))) {
// 		for (const FuncallArg *arg = funcall->args; arg != NULL;
// 		     arg = arg->next) {
// 			IR_dump_expr(funcall->args->expr, false);
// 			print(NULL, WIN_IR, IR_FORMAT "CALL    write_str",
// 			      IR_CTX());
// 		}
// 	}
// }

// static void IR__STMT_BLOCK(Block_Context_IR *ctx)
// {
// 	assert(ctx);
// 	assert(ctx->next->statement.type == STMT_BLOCK_START);

// 	// setup the encapsulated block's context
// 	StmtNode *nxt = ctx->next->statement.as.block;
// 	Block_Context_IR blk_ctx = { .n = ctx->n,
// 				     .b = ctx->b,
// 				     .next = nxt,
// 				     .prev = ctx,
// 				     .var_def_cnt = 0 };

// 	print(NULL, WIN_IR, IR_FORMAT "%%scope", IR_CTX());
// 	IR_dump_code_block(&blk_ctx);
// 	print(NULL, WIN_IR, IR_FORMAT "%%end", IR_CTX());

// 	ctx->n = blk_ctx.n;
// }

// // --------------------------------------------------------------------------

// static void IR_dump_statement(Block_Context_IR *ctx)
// {
// 	assert(ctx->next != NULL);
// 	Stmt stmt = ctx->next->statement;
// 	// print(NULL, WIN_IR, IR_FORMAT ";;        %d", IR_CTX(),
// 	//       ctx->next->statement.type);
// 	switch (stmt.type) {
// 	case STMT_BLOCK_START:
// 		IR__STMT_BLOCK(ctx);
// 		break;
// 	case STMT_CONDITIONAL:
// 		IR__STMT_CONDITIONAL(ctx);
// 		break;
// 	case STMT_EXPR:
// 		IR_dump_expr(stmt.as.expr, ctx->b == 999);
// 		break;
// 	case STMT_VAR_DECL:
// 		IR__STMT_VAR_DECL(ctx);
// 		break;
// 	case STMT_VAR_DEFN:
// 		IR__STMT_VAR_DEFN(ctx);
// 		break;
// 	case STMT_MATCH:
// 	case STMT_BLOCK_END:
// 	default:
// 		break;
// 	}
// 	ctx->next = ctx->next->next;
// }

void ignore_err_stmt_ir()
{
	;
}