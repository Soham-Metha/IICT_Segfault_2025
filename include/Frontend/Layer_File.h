#ifndef FILE_LAYER_FRONTEND
#define FILE_LAYER_FRONTEND
#include <Utils/errors.h>
#include <Utils/mem_manager.h>
#include <Utils/strings.h>
#include <Frontend/Layer_Line.h>

typedef struct Line_Context Line_Context;

typedef struct File_Context {
	const char *file_path;
	String contents;
	Line_Context lines[1024]; // hardcoded line count
	unsigned int line_num;
} File_Context;

Error file_read(const char *filePath, File_Context *context);
Line_Context *file_fetch_next_line(File_Context *context);
Line_Context *file_fetch_curr_line(File_Context *context);
Error AST_generate(const CodeBlock *blk, bool renderPng);

#endif