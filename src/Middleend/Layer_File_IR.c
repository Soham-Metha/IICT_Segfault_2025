#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_Statement.h>
#include <Wrapper/IO.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

static int IR_dump_token(int *n, const Token tok);
static int IR_dump_code_block(const StmtNode *stmtNode, int *n, int *b);
static int IR_dump_statement(const Stmt *stmt, int *n, int *b);

static int __TOKEN_TYPE_STR(int id, String str)
{
	print(WIN_IR, "\n%%bind\t_%d\t\"%.*s\"", id, Str_Fmt(str));
	print(WIN_IR, "\nPUSH\t_%d", id);
	print(WIN_IR, "\nPUSH\tlen(_%d)", id);
	return id;
}

// static int __TOKEN_TYPE_CHAR(int id, char c)
// {

// }
// static int __TOKEN_TYPE_NUMBER(int id)
// {

// }
// static int __TOKEN_TYPE_NAME(int id)
// {

// }
// static int __TOKEN_TYPE_OPEN_PAREN(int id)
// {

// }
// static int __TOKEN_TYPE_CLOSING_PAREN(int id)
// {

// }
// static int __TOKEN_TYPE_OPEN_CURLY(int id)
// {

// }
// static int __TOKEN_TYPE_CLOSING_CURLY(int id)
// {

// }
// static int __TOKEN_TYPE_STATEMENT_END(int id)
// {

// }
// static int __TOKEN_TYPE_COMMA(int id)
// {

// }
// static int __TOKEN_TYPE_COLON(int id)
// {

// }
// static int __TOKEN_TYPE_EQUAL(int id)
// {

// }
// static int __TOKEN_TYPE_EOL(int id)
// {

// }

// // ------------------------- INDIVIDUAL STATEMENT HANDLERS -------------------------

int IR__STMT_VARIABLE(int id, Var v, int *n, int *b)
{
	switch (v.mode) {
	case VAR_ACCS:
		print(WIN_IR, "\nPUSH   %.*s", Str_Fmt(v.name));
		break;

	case VAR_DECL:
		// print(WIN_IR, "%%bind    %s",v.name);
		break;

	case VAR_DEFN:
	case VAR_BOTH:
		if (compare_str(v.type, STR("func"))) {
            print(WIN_IR, "\n%.*s:", Str_Fmt(v.name));
            print(WIN_IR, "\n%%scope");
		    IR_dump_statement(v.defn_val, n, b);
		} else {
	        int child = IR_dump_statement(v.defn_val, n, b);
			print(WIN_IR, "\n%%bind    %.*s    _%d", Str_Fmt(v.name), child);
		}
		break;
	default:
		break;
	}

	return id;
}

int IR__STMT_UNKNOWN(int id)
{
    print(WIN_IR, "");
	return id;
}

int IR__STMT_FUNCALL(int id, int *n, int *b, const Funcall *funcall)
{
    print(WIN_IR, "");

	for (const FuncallArg *arg = funcall->args; arg != NULL; arg = arg->next) {
		int childId = IR_dump_statement(&arg->value, n, b);
        (void)childId;
		// if (childId >= 0) print(WIN_IR, "  Expr_%d -> Expr_%d;\n", id, childId);
	}
	return id;
}

int IR__STMT_BLOCK(int id, int *n, int *b, const StmtNode *block)
{
	(void)id;
	int clusterId  = (*n)++;
	int clusterNum = (*b)++;
    (void)clusterNum;

	print(WIN_IR, "");

	IR_dump_code_block(block, n, b);

	print(WIN_IR, "");

	return clusterId;
}

// // ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

static int IR_dump_token(int *n, const Token tok)
{
	int myId = (*n)++;

    switch (tok.type)
    {
    case TOKEN_TYPE_STR: return __TOKEN_TYPE_STR(myId, tok.text);
    case TOKEN_TYPE_CHAR: return myId;
    case TOKEN_TYPE_NUMBER: return myId;
    case TOKEN_TYPE_NAME: return myId;
    case TOKEN_TYPE_OPEN_PAREN: return myId;
    case TOKEN_TYPE_CLOSING_PAREN: return myId;
    case TOKEN_TYPE_OPEN_CURLY: return myId;
    case TOKEN_TYPE_CLOSING_CURLY: return myId;
    case TOKEN_TYPE_STATEMENT_END: return myId;
    case TOKEN_TYPE_COMMA: return myId;
    case TOKEN_TYPE_COLON: return myId;
    case TOKEN_TYPE_EQUAL: return myId;
    case TOKEN_TYPE_EOL: return myId;

    default:
        break;
    }
    return myId;
}

static int IR_dump_statement(const Stmt *stmt, int *n, int *b)
{
	assert(stmt != NULL);
	int myId = (*n)++;

	switch (stmt->type) {
	case STMT_VAR: 			return IR__STMT_VARIABLE	(myId, stmt->value.as_var, n, b);
	case STMT_BLOCK_END:
	case STMT_TOKEN:		return IR_dump_token	    (n, stmt->value.as_token);
	case STMT_FUNCALL:		return IR__STMT_FUNCALL	    (myId, n, b, stmt->value.as_funcall);
	case STMT_BLOCK_START: 	return IR__STMT_BLOCK		(myId, n, b, stmt->value.as_block);
	default: 				return IR__STMT_UNKNOWN	    (myId);
	}
    return myId;
}

static int IR_dump_code_block(const StmtNode *stmtNode, int *n, int *b)
{
	int firstId = -1;
	int prevId 	= -1;

    print(WIN_IR, "\n%%scope");
	for (const StmtNode *cur = stmtNode; cur != NULL; cur = cur->next) {
		int id 	= IR_dump_statement(&cur->statement, n, b);

		if (firstId < 0) firstId = id;

		prevId 	= id;
        (void)prevId;
	}
    print(WIN_IR, "\n%%end");

	return firstId;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

Error IR_generate(const CodeBlock *blk)
{
	int node_counter  = 0;
	int block_counter = 0;

	// print(WIN_IR, "");

	IR_dump_code_block(blk->begin, &node_counter, &block_counter);
	// print(WIN_IR, "");

	return ERR_OK;
}

// void IGNORE_ERRORS()
// {
// 	;
// }