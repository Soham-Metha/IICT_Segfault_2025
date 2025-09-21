#include <Frontend/Layer_File.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

FILE* ast  = NULL;
FILE* myIR = NULL;

void print(Line_Context* const ctx, int id, const char* str, ...)
{
    va_list args;
    va_start(args, str);

    if (id == WIN_AST) {
        if (!ast) {
            ast = fopen("ast.dot", "w");
        }
        vfprintf(ast, str, args);
    } else if (id == WIN_IR) {
        if (!myIR) {
            myIR = fopen("ir.sasm", "w");
        }

        // va_list args_copy;
        // va_copy(args_copy, args);
        // vfprintf(stdout, str, args_copy);
        vfprintf(myIR, str, args);
    } else {
        assert(id == WIN_STDERR);
        printf("\n\n%s:%d:%ld: ", ctx->file_name, ctx->line_no,
            (ctx->line.data - ctx->line_start));
        vprintf(str, args);
        printf("\n");
    }

    va_end(args);
}

void log_to_ctx(Line_Context* ctx, const char* str, ...)
{
    va_list args;
    va_start(args, str);

    va_list args_copy;
    va_copy(args_copy, args);

    int size  = vsnprintf(NULL, 0, str, args_copy);
    char* log = region_allocate(size + 1);

    vsnprintf(log, size + 1, str, args);

    ctx->logs[ctx->log_cnt++] = (String) { .data = log, .len = size };
    printf("%s", log);

    va_end(args);
}

int indent  = 0;
char* space = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
String get_indent()
{
    return (String) { .data = space, .len = indent };
}

void update_indent(int change)
{
    indent += change;
}