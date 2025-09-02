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
	// to add new statement, allocate enough memory
	assert(list);
	StmtNode *node = malloc(sizeof(StmtNode));
	ERROR_THROW_IF(ERR_RAN_OUT_OF_MEM, (!node));

	// and initialize the node with values
	node->statement = statement;
	node->next = NULL;

	// if list was empty, then this is the sole node
	// else, simply append to last node (tail)
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

String line_get_preprocessed_line()
{
	String line;

	do {
		line = file_fetch_next_line();

		// discard any comments
		line = split_str_by_delim(&line, COMMENT_SYMBOL);
		line = trim(line);

		print(WIN_STDOUT, "\n[LINE] After Preprocessing : %.*s",
		      Str_Fmt(line));

		// if, after timming, line is empty, move to next line
	} while (line.len == 0);

	return line;
}

Error line_parse_next(CodeBlock *blk, bool *block_end)
{
	Error res = ERR_OK;
	String line = line_get_preprocessed_line();

	// a single line may contain multiple statements,
	// process each one of them
	while (line.len > 0) {
		Stmt stmt = stmt_fetch_next(&line);

		// if statement identifies the start or end of a block,
		// handle it, the start of the block would be a pointer
		// to that block's linked list of statements, forming
		// a linked list of linked lists (2D LL) if there is single layer
		// of nesting, 3D LL for 2 layers of nesting & so on.
		// the 1st dimension is the global scope & each layer of nesting
		// adds another dimension.
		if (stmt.type == STMT_BLOCK_END) {
			*block_end = true;
		} else if (stmt.type == STMT_BLOCK_START) {
			stmt.value.as_block = block_generate().begin;
		}

		// append the statement to the linked list
		// and if error occurs, return error code
		ERROR_CHECK(res, return res, codeblock_append_stmt(blk, stmt));
	}

	return res;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

CodeBlock block_generate()
{
	CodeBlock result = { 0 };

	// flag to check for block end
	bool block_end = false;

	while (file.contents.len > 0) {
		// TODO: MEM ALLOC error handling
		(void)line_parse_next(&result, &block_end);
		if (block_end)
			break;
	}
	// TODO: edge case not handled, once file reaches eof,
	// all scopes will be automatically closed, for example,
	// if the file only contains '{{', then it opens 2 layers of nesting,
	// without closing it, this should throw an error or warning, but it
	// will compile as eof will end every scope.
	return result;
}