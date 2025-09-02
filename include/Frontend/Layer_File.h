#ifndef FILE_LAYER_FRONTEND
#define FILE_LAYER_FRONTEND
#include <Utils/errors.h>
#include <Utils/strings.h>
#include <Frontend/Layer_Line.h>

typedef struct File_View File_View;

struct File_View {
	const char *file_path;
	String contents;
	unsigned int line_num;
};

extern File_View file;

Error file_read(const char *filePath);
String file_fetch_next_line();
Error AST_generate(const CodeBlock *blk, bool renderPng);

#endif