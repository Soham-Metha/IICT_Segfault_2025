#ifndef FILE_LAYER_BACKEND
#define FILE_LAYER_BACKEND
#include <Utils/strings.h>

typedef struct File_View File_View;

struct File_View {
	String contents;
};

void readFile();
void generateAST();

#endif