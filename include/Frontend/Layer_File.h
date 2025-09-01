#ifndef FILE_LAYER_BACKEND
#define FILE_LAYER_BACKEND
#include <Utils/strings.h>

#define COMMENT_SYMBOL '%'
typedef struct File_View File_View;

struct File_View {
	const char *file_path;
	String contents;
	unsigned int line_num;
};

void readFile(const char *filePath);
String getNextLine();
void generateAST();

#endif