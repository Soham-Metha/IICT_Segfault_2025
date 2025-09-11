#ifndef LINE_LAYER_FRONTEND
#define LINE_LAYER_FRONTEND
#define COMMENT_SYMBOL '%'
#include <Utils/strings.h>

typedef struct StmtNode StmtNode;
typedef struct File_Context File_Context;

typedef struct Line_Context {
    String line;
    unsigned int line_no;
    String logs[128];
    const char* line_start;
    const char* file_name;
    int log_cnt;
} Line_Context;

typedef struct CodeBlock {
    StmtNode* begin;
    StmtNode* end;
} CodeBlock;

CodeBlock codeblock_generate(File_Context* file);

#endif