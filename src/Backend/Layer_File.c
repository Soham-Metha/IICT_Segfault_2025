#include <Backend/Layer_File.h>
#include <Utils/strings.h>
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

	long fileSize = ftell(f);

	if (fileSize < 0) {
		print(WIN_STDERR, "can't read from file %s", filePath);
		exit(1);
	}

	rewind(f);

	return (unsigned int)fileSize;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

void readFile(const char *filePath)
{
	FILE *file_ptr = openFile(filePath, "r");
	unsigned int size = getFileSize(file_ptr, filePath);

	file_contents = (char *)malloc(size + 1);
	{ // Error handling
		if (!file_contents) {
			print(WIN_STDERR, "out of memory while reading %s",
			      filePath);
			fclose(file_ptr);
			exit(1);
		}
	}

	size_t readCount = fread(file_contents, 1, size, file_ptr);
	{ // Error handling
		if (readCount != (size_t)size) {
			print(WIN_STDERR, "failed to read entire file %s",
			      filePath);
			free(file_contents);
			fclose(file_ptr);
			exit(1);
		}
	}

	fclose(file_ptr);

	file_contents[size] = '\0';

	file.contents.len = size;
	file.contents.data = file_contents;
	file.file_path = filePath;

	print(WIN_STDOUT, "\n[FILE] Reading File %s", file.file_path);
	processFile();
}

void processFile()
{
	file.line_num = 1;
	while (file.contents.len > 0) {
		String line = { 0 };
		print(WIN_STDOUT, "\n[LINE] Read Line %u", file.line_num);
		do {
			line = trim(split_str_by_delim(&file.contents, '\n'));
			line = trim(split_str_by_delim(&line, COMMENT_SYMBOL));
			file.line_num += 1;
		} while (line.len == 0 && file.contents.len > 0);
		print(WIN_STDOUT, ": %.*s", (int)line.len, line.data);
	}
}