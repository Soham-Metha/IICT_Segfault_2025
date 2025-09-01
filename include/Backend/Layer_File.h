#ifndef FILE_LAYER_BACKEND
#define FILE_LAYER_BACKEND
#include <Utils/strings.h>

typedef struct File_View File_View;

struct File_View {
    const char* file_path;
	String contents;
    unsigned int line_num;
};

void readFile();
void generateAST();

#endif