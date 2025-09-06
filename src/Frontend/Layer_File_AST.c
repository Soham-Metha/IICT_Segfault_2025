#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_Statement.h>
#include <Wrapper/IO.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

static int AST_dump_code_block(const StmtNode *stmtNode, int *n, int *b);
static int AST_dump_statement(const Stmt *stmt, int *n, int *b);

// ------------------------- INDIVIDUAL STATEMENT HANDLERS -------------------------

int __STMT_VARIABLE(int id,  const Var *v)
{
	print(NULL, WIN_AST, AST("ellipse", "lightgoldenrod1", "%.*s"), id, Str_Fmt(v->name));
	if (v->mode & VAR_DECL){
	print(NULL, WIN_AST, AST("ellipse", "lightgoldenrod1", "%.*s"), id+1, Str_Fmt(v->type));
	}
	// if (v->mode & VAR_DEFN) {
	// print(NULL, WIN_AST, AST("ellipse", "lightgoldenrod1", "%.*s"), id+2, Str_Fmt(v->defn_val->as.token->text));
	// }
	return id;
}

int __STMT_TOKEN(int id, String value)
{
	print(NULL, WIN_AST, AST("note", "lightblue", "%.*s"), id, value);
	return id;
}

int __STMT_UNKNOWN(int id)
{
	print(NULL, WIN_AST, "  Expr_%d [label=\"Stmt_unknown\"];\n", id);
	return id;
}

int __STMT_FUNCALL(int id, int *n, int *b, const Funcall *funcall)
{
	print(NULL, WIN_AST, AST("hexagon", "lightpink", "%.*s"), id, Str_Fmt(funcall->name));

	for (const FuncallArg *arg = funcall->args; arg != NULL; arg = arg->next) {
		int childId = AST_dump_statement(&arg->statement, n, b);

		if (childId >= 0) print(NULL, WIN_AST, "  Expr_%d -> Expr_%d;\n", id, childId);
	}
	return id;
}

int __STMT_BLOCK(int id, int *n, int *b, const StmtNode *block)
{
	(void)id;
	int clusterId  = (*n)++;
	int clusterNum = (*b)++;

	print(NULL, WIN_AST,
		  "  subgraph cluster_%d {\n"
		  "    label=\"Code Block %d\";\n"
		  "    style=filled;\n"
		  "    color=gray;\n"
		  "    fillcolor=whitesmoke;\n"
		  "    fontname=\"Courier\";\n",
		  clusterId, clusterNum);

	AST_dump_code_block(block, n, b);

	print(NULL, WIN_AST, 
		"  }\n" AST("box3d", "aquamarine", "Code Block %d"),
		  clusterId, clusterNum);

	return clusterId;
}

// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

static int AST_dump_statement(const Stmt *stmt, int *n, int *b)
{
	assert(stmt != NULL);
	int myId = (*n)++;

	switch (stmt->type) {
	case STMT_VAR: 			return __STMT_VARIABLE	(myId, &stmt->as.var);
	case STMT_BLOCK_END:
	case STMT_TOKEN:		return __STMT_TOKEN		(myId, stmt->as.token->text);
	case STMT_FUNCALL:		return __STMT_FUNCALL	(myId, n, b, stmt->as.funcall);
	case STMT_BLOCK_START: 	return __STMT_BLOCK		(myId, n, b, stmt->as.block);
	case STMT_MATCH:
	case STMT_CONDITIONAL:
	default: 				return __STMT_UNKNOWN	(myId);
	}
}

static int AST_dump_code_block(const StmtNode *stmtNode, int *n, int *b)
{
	int firstId = -1;
	int prevId 	= -1;

	for (const StmtNode *cur = stmtNode; cur != NULL; cur = cur->next) {
		int id 	= AST_dump_statement(&cur->statement, n, b);

		if (firstId < 0) firstId = id;
		if (prevId >= 0) print(NULL, WIN_AST, "  Expr_%d -> Expr_%d;\n", prevId, id);

		prevId 	= id;
	}

	return firstId;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

Error AST_generate(const CodeBlock *blk, bool renderPng)
{
	int node_counter  = 0;
	int block_counter = 0;

	print(NULL, WIN_AST, 
		"digraph AST {\n"
		"  splines=ortho;\n"
		"  nodesep=0.8;\n"
		"  ranksep=0.5;\n"
		"  node [fontname=\"Courier\"];\n");

	AST_dump_code_block(blk->begin, &node_counter, &block_counter);
	print(NULL, WIN_AST, "}\n");

	if (renderPng) {
		int r = system("dot -Tpng ast.dot -o ast.png ");
		ERROR_THROW_IF(ERR_AST_RENDER_ERR, (r != 0))
	}

	return ERR_OK;
}
