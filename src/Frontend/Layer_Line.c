#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_File.h>
#include <Utils/strings.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdlib.h>

void pushScope(Scope *curr)
{
	Scope *new = malloc(sizeof(Scope));
	new->prev = curr;
	curr = new;
}

void popScope(Scope *curr)
{
	assert(curr != NULL);
	curr = curr->prev;
}

void pushStatementIntoBlock(CodeBlock *list, Stmt statement)
{
	assert(list);
	StmtNode *stmtNode = malloc(sizeof(StmtNode));
	stmtNode->statement = statement;

	if (list->end == NULL) {
		assert(list->begin == NULL);
		list->begin = stmtNode;
		list->end = stmtNode;
	} else {
		assert(list->begin != NULL);
		list->end->next = stmtNode;
		list->end = stmtNode;
	}
}

String getNextLine(Scope *scope)
{
	String line = { 0 };

	do {
		file.line_num += 1;
		file.contents = ltrim(file.contents);

		line = trim(split_str_by_delim(&file.contents, '\n'));
		print(WIN_STDOUT, "\n[LINE] Reading Line %3u : %.*s",
		      file.line_num, (int)line.len, line.data);

		if (get_index_of(line, COMMENT_SYMBOL, NULL)) {
			line = trim(split_str_by_delim(&line, COMMENT_SYMBOL));
			print(WIN_STDOUT, "\n[LINE] Discard Comments : %.*s",
			      (int)line.len, line.data);
		}
	} while (line.len == 0 && file.contents.len > 0);

	while (line.len > 0) {
		Stmt stmt = getNextStmt(&line);
		if (stmt.type == STMT_BLOCK_START) {
			pushScope(scope);
			stmt.value.as_block = &scope->block;
			pushStatementIntoBlock(&scope->block, stmt);
		} else if (stmt.type == STMT_BLOCK_END) {
			popScope(scope);
		} else {
			pushStatementIntoBlock(&scope->block, stmt);
		}
	}
	return line;
}