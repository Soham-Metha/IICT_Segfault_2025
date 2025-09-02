#ifndef FILE_LAYER_FRONTEND
#define FILE_LAYER_FRONTEND
#include <Utils/errors.h>
#include <Utils/strings.h>
#include <Frontend/Layer_Line.h>

typedef struct File_View File_View;
typedef struct Line_View Line_View;

struct Line_View {
	String line;
	String logs[128];
	int log_cnt;
};

struct File_View {
	const char *file_path;
	String contents;
	Line_View lines[1024]; // hardcoded line count
	unsigned int line_num;
};

extern File_View file;
extern String *current_buffer;
extern int *current_buffer_size;

Error file_read(const char *filePath);
String file_fetch_next_line();
Error AST_generate(const CodeBlock *blk, bool renderPng);

#endif