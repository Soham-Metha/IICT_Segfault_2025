#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Statement.h>
#include <Utils/strings.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

Error codeblock_append_stmt(CodeBlock *list, Stmt statement)
{
	// a codeblock is a linked list of statements
	// so this is just the logic of appending a node to ll

	assert(list);
	StmtNode *node = region_allocate(sizeof(StmtNode));
	ERROR_THROW_IF(ERR_RAN_OUT_OF_MEM, (!node));
	node->statement = statement;
	node->next = NULL;

	if (list->end == NULL) {
		assert(list->begin == NULL);
		list->begin = node;
		list->end = node;
	} else {
		assert(list->begin != NULL);
		list->end->next = node;
		list->end = node;
	}
	return ERR_OK;
}

void line_get_preprocessed_line(Line_Context *ctx)
{
	String processed_line = ctx->line;

	size_t index;
	if (get_index_of(ctx->line, COMMENT_SYMBOL, &index)) {
		processed_line =
			trim(split_str_by_delim(&ctx->line, COMMENT_SYMBOL));

		log_to_ctx(ctx, LOG_FORMAT "\"%%%.*s\" ",
			   LOG_CTX("[COMMENT FOUND]", "[LINE]"),
			   Str_Fmt(ctx->line));

		log_to_ctx(ctx, LOG_FORMAT "\"%.*s\"",
			   LOG_CTX("[AFTER REMOVAL]", "[LINE]"),
			   Str_Fmt(processed_line));

		ctx->line = processed_line;
	}

	if (ctx->line.len == 0) {
		log_to_ctx(ctx, LOG_FORMAT "Line is Blank",
			   LOG_CTX("[PREPROCESS]", "[LINE]"));
	}
}

int var_decl_level = 0;
bool line_parse_next(CodeBlock *blk, File_Context *context)
{
	Line_Context *ctx = file_fetch_curr_line(context);

	// if statement identifies the start or end of a block, handle it,
	// the start of the block would be a pointer to that block's linked list
	// of statements, forming a linked list of linked lists (2D LL) if
	// there is single layer of nesting, 3D LL for 2 layers of nesting & so on.
	// the 1st dimension is the global scope, each layer of nesting adds another dimension.
	while (ctx->line.len > 0) {
		line_get_preprocessed_line(ctx);
		Stmt statement = stmt_fetch_next(ctx);

		switch (statement.type) {
		case STMT_VAR:
			if (statement.as.var.mode & VAR_DEFN) {
				var_decl_level += 1;
				log_to_ctx(ctx, LOG_FORMAT "Defined value: {",
					   LOG_CTX("[DEFINITION START]",
						   "[STMT]"));
				update_indent(1);
			}
			break;
		case STMT_BLOCK_START:
			statement.as.block =
				codeblock_generate(context).begin;
			break;
		case STMT_BLOCK_END:
			if (var_decl_level > 0) {
				var_decl_level -= 1;
				update_indent(-1);
				log_to_ctx(ctx, LOG_FORMAT " } ",
					   LOG_CTX("[DEFINITION  END]",
						   "[STMT]"));
			}
			return true;
		case STMT_MATCH: {
			PatternMatch *match = region_allocate(sizeof(*match));
			match->cond = region_allocate(sizeof(*match->cond));
			match->body = region_allocate(sizeof(*match->body));
			Stmt stmt = stmt_fetch_next(ctx);
			ctx = file_fetch_curr_line(context);
			if (stmt.type == STMT_VAR &&
			    stmt.as.var.mode & VAR_DEFN) {
				stmt.as.var.mode &= VAR_DECL;
				// the variable in question is not being defined,
				// even though it's followed by an assignment operator
				// that assignment operator is syntax sugar for the match statement.
			}
			*match->cond = stmt;
			stmt = stmt_fetch_next(ctx);
			ctx = file_fetch_curr_line(context);
			*match->body = stmt;
			break;
		}
		case STMT_CONDITIONAL: {

			log_to_ctx(ctx, LOG_FORMAT " ------------------------------------------------------------------- ",
				LOG_CTX("[DEFINITION  END]",
					"[STMT]"));
			statement.as.cond.body =
				codeblock_generate(context);

			(void)codeblock_append_stmt(blk, statement);
		} break;
		case STMT_FUNCALL:
		case STMT_TOKEN:
		default:
			if (var_decl_level > 0) {
				var_decl_level -= 1;
				update_indent(-1);
				log_to_ctx(ctx, LOG_FORMAT " } ",
					   LOG_CTX("[DEFINITION  END]",
						   "[STMT]"));
			}
			break;
		}

		// TODO 1: MEM ALLOC error handling
		(void)codeblock_append_stmt(blk, statement);
		ctx = file_fetch_curr_line(context);
	}

	return false;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

CodeBlock codeblock_generate(File_Context *file)
{
	CodeBlock res = { 0 };
	bool block_end = false;

	while (file->contents.len > 0) {
		Line_Context *ctx = file_fetch_next_line(file);
		line_get_preprocessed_line(ctx);

		block_end = line_parse_next(&res, file);

		if (block_end)
			break;
	}

	// TODO 2: edge case not handled, once file reaches eof, all scopes will
	// be automatically closed, for example, if the file only contains '{{',
	// then it opens 2 layers of nesting, without closing it, this should throw
	// an error or warning, but it will compile as eof will end every scope.
	return res;
}