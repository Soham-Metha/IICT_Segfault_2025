#ifndef LINE_LAYER_FRONTEND
#define LINE_LAYER_FRONTEND
#define COMMENT_SYMBOL '%'
#include <Utils/strings.h>

typedef struct CodeBlock CodeBlock;
typedef struct StmtNode StmtNode;
typedef struct File_Context File_Context;
typedef struct Line_Context Line_Context;

struct Line_Context {
	String line;
	unsigned int line_no;
	String logs[128];
	const char *line_start;
	int log_cnt;
};

struct CodeBlock {
	StmtNode *begin;
	StmtNode *end;
};

CodeBlock codeblock_generate(File_Context* file);

#endif