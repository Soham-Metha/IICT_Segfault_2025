#ifndef FILE_LAYER_FRONTEND
#define FILE_LAYER_FRONTEND
#include <Utils/strings.h>

typedef struct File_View File_View;

struct File_View {
	const char *file_path;
	String contents;
	unsigned int line_num;
};

extern File_View file;

void readFile(const char *filePath);
void generateAST();

#endif