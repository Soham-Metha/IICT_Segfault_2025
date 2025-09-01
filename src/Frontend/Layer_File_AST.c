#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Wrapper/IO.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

static int dump_stmt_node_list(const StmtNode *stmtNode, int *n, int *b);

static int dump_statement(const Stmt *stmt, int *n, int *b)
{
	assert(stmt != NULL);
	int myId = (*n)++;

	switch (stmt->type) {
	case STMT_VARIABLE: {
		print(WIN_AST, AST("ellipse", "lightgoldenrod1", "%.*s"), myId,
		      Str_Fmt(stmt->value.as_var));
		break;
	}
	case STMT_LIT_INT: {
		print(WIN_AST, AST("note", "lightblue", PRIu64), myId,
		      stmt->value.as_int);
		break;
	}
	case STMT_LIT_FLOAT: {
		print(WIN_AST, AST("note", "lightblue", "%f"), myId,
		      stmt->value.as_float);
		break;
	}
	case STMT_LIT_CHAR: {
		print(WIN_AST, AST("note", "lightblue", "%c"), myId,
		      stmt->value.as_char);
		break;
	}
	case STMT_LIT_STR: {
		print(WIN_AST, AST("note", "lightblue", "%.*s"), myId,
		      Str_Fmt(stmt->value.as_str));
		break;
	}
	case STMT_FUNCALL_DECLARATION:
	case STMT_FUNCALL: {
		print(WIN_AST, AST("hexagon", "lightpink", "%.*s"), myId,
		      Str_Fmt(stmt->value.as_funcall->name));

		// for (const FuncallArg *arg = f->args; arg != NULL;
		//      arg = arg->next) {
		// 	int childId = dump ( &arg->value,
		// 				    nodeCounter, blockCounter);
		// 	if (childId >= 0)
		// 		print(WIN_AST, "  Expr_%d -> Expr_%d;\n", myId,
		// 			childId);
		// }
		break;
	}
	case STMT_BLOCK_START: {
		int clusterId = (*n)++;
		int clusterNum = (*b)++;

		print(WIN_AST, "  subgraph cluster_%d {\n", clusterId);
		print(WIN_AST, "    label=\"Code Block %d\";\n", clusterNum);
		print(WIN_AST, "    style=filled;\n");
		print(WIN_AST, "    color=gray;\n");
		print(WIN_AST, "    fillcolor=whitesmoke;\n");
		print(WIN_AST, "    fontname=\"Courier\";\n");

		dump_stmt_node_list(stmt->value.as_block, n, b);

		print(WIN_AST,
		      "  }\n" AST("box3d", "aquamarine", "Code Block %d"),
		      clusterId, clusterNum);
		return clusterId;
	}
	case STMT_BLOCK_END:
		break;
	default: {
		print(WIN_AST, "  Expr_%d [label=\"Stmt_unknown\"];\n", myId);
		break;
	}
	}

	return myId;
}

static int dump_stmt_node_list(const StmtNode *stmtNode, int *n, int *b)
{
	int firstId = -1;
	int prevId = -1;
	for (const StmtNode *cur = stmtNode; cur != NULL; cur = cur->next) {
		int id = dump_statement(&cur->statement, n, b);
		if (firstId < 0)
			firstId = id;
		if (prevId >= 0)
			print(WIN_AST, "  Expr_%d -> Expr_%d;\n", prevId, id);
		prevId = id;
	}
	return firstId;
}

int generateAST(const CodeBlock *blk, bool renderPng)
{
	print(WIN_AST, "digraph AST {\n");
	print(WIN_AST,
	      "  splines=ortho;\n  nodesep=0.8;\n  ranksep=0.5;\n  node [fontname=\"Courier\"];\n");

	int node_counter = 0;
	int block_counter = 0;

	if (blk->begin) {
		dump_stmt_node_list(blk->begin, &node_counter, &block_counter);
	}

	print(WIN_AST, "}\n");

	if (renderPng) {
		int r = system("dot -Tpng ast.dot -o ast.png ");
		(void)r;
	}
	return 0;
}
