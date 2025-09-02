#ifndef LINE_LAYER_FRONTEND
#define LINE_LAYER_FRONTEND
#define COMMENT_SYMBOL '%'
#include <Utils/strings.h>
#include <Frontend/Layer_Statement.h>

typedef struct CodeBlock CodeBlock;
typedef struct StmtNode StmtNode;

struct CodeBlock {
	StmtNode *begin;
	StmtNode *end;
};

CodeBlock codeblock_generate();

#endif