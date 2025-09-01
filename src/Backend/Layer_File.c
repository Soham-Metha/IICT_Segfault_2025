#include <Backend/Layer_File.h>
#include <Wrapper/IO.h>
#include <stdio.h>
#include <stdlib.h>

// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

File_View file;

void readFile(const char *filePath)
{
	FILE *file_ptr = openFile(filePath, "r");
}

FILE *openFile(const char *filePath, const char *mode)
{
	FILE *f = fopen(filePath, mode);

	if (!f) {
		print(WIN_STDERR, "can't open file %s", filePath);
		exit(1);
	}

	return f;
}

long getFileSize(FILE *f, const char *filePath)
{
	if (f == NULL) {
		print(WIN_STDERR, "invalid file pointer %s", filePath);
		exit(1);
	}

	if (fseek(f, 0, SEEK_END) != 0) {
		print(WIN_STDERR, "can't read from file %s", filePath);
		exit(1);
	}

	long fileSize = ftell(f);

	if (fileSize < 0) {
		print(WIN_STDERR, "can't read from file %s", filePath);
		exit(1);
	}

	rewind(f);

	return fileSize;
}
