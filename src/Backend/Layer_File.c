#include <Backend/Layer_File.h>
#include <Wrapper/IO.h>
#include <stdio.h>
#include <stdlib.h>

char *file_contents;
File_View file;
// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

FILE *openFile(const char *filePath, const char *mode)
{
	FILE *f = fopen(filePath, mode);

	if (!f) {
		print(WIN_STDERR, "can't open file %s", filePath);
		exit(1);
	}

	return f;
}

unsigned int getFileSize(FILE *f, const char *filePath)
{
	if (f == NULL) {
		print(WIN_STDERR, "invalid file pointer %s", filePath);
		exit(1);
	}

	if (fseek(f, 0, SEEK_END) != 0) {
		print(WIN_STDERR, "can't read from file %s", filePath);
		exit(1);
	}

	unsigned int fileSize = ftell(f);

	if (fileSize < 0) {
		print(WIN_STDERR, "can't read from file %s", filePath);
		exit(1);
	}

	rewind(f);

	return fileSize;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

void readFile(const char *filePath)
{
	FILE *file_ptr = openFile(filePath, "r");
	unsigned int size = getFileSize(file_ptr, filePath);

	file_contents = (char *)malloc(size + 1);
	if (!file_contents) {
		print(WIN_STDERR, "out of memory while reading %s", filePath);
		fclose(file_ptr);
		exit(1);
	}

	size_t readCount = fread(file_contents, 1, size, file_ptr);
	if (readCount != (size_t)size) {
		print(WIN_STDERR, "failed to read entire file %s", filePath);
		free(file_contents);
		fclose(file_ptr);
		exit(1);
	}

	file_contents[size] = '\0';

	file.contents.len = size;
	file.contents.data = file_contents;

	fclose(file_ptr);
}