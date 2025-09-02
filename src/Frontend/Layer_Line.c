#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_File.h>
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
	StmtNode *node 		= malloc(sizeof(StmtNode));
	ERROR_THROW_IF(ERR_RAN_OUT_OF_MEM, (!node));
	node->statement 	= statement;
	node->next 			= NULL;

	if (list->end == NULL) {
		assert(list->begin == NULL);
		list->begin 	= node;
		list->end 		= node;
	} else {
		assert(list->begin != NULL);
		list->end->next = node;
		list->end 		= node;
	}
	return ERR_OK;
}

String line_get_preprocessed_line()
{
	String line;

	line = file_fetch_next_line();
	line = split_str_by_delim(&line, COMMENT_SYMBOL);
	line = trim(line);

	print(WIN_STDOUT, "\n[LINE] After Preprocessing : %.*s", Str_Fmt(line));

	return line;
}

Error line_parse_next(CodeBlock *blk, bool *block_end)
{
	Error res 	= ERR_OK;
	String line = line_get_preprocessed_line();

	// if statement identifies the start or end of a block, handle it,
	// the start of the block would be a pointer to that block's linked list
	// of statements, forming a linked list of linked lists (2D LL) if 
	// there is single layer of nesting, 3D LL for 2 layers of nesting & so on.
	// the 1st dimension is the global scope, each layer of nesting adds another dimension.
	while (line.len > 0) {
		Stmt statement		= stmt_fetch_next(&line);
		if (statement.type == STMT_BLOCK_END) {
			*block_end 		= true;
		} 
		else if (statement.type == STMT_BLOCK_START) {
			statement.value.as_block = codeblock_generate().begin;
		}

		ERROR_CHECK(res, return res, codeblock_append_stmt(blk, statement));
	}

	return res;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

CodeBlock codeblock_generate()
{
	CodeBlock res 	= { 0 };
	bool block_end 	= false;

	while (file.contents.len > 0) {
		Error err 	= line_parse_next(&res, &block_end);
		(void)err;

		if (block_end) break;
	}
	// TODO 1: MEM ALLOC error handling
	// TODO 2: edge case not handled, once file reaches eof, all scopes will 
	// be automatically closed, for example, if the file only contains '{{',
	// then it opens 2 layers of nesting, without closing it, this should throw
	// an error or warning, but it will compile as eof will end every scope.
	return res;
}