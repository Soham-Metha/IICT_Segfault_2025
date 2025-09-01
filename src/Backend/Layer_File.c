#include <Backend/Layer_File.h>
#include <Wrapper/IO.h>
#include <stdio.h>
#include <stdlib.h>

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

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

void readFile(const char *filePath)
{
	FILE *file_ptr = openFile(filePath, "r");
    long size = getFileSize(file_ptr, filePath);

    char *buffer = (char *)malloc(size + 1);
    if (!buffer) {
        print(WIN_STDERR, "out of memory while reading %s", filePath);
        fclose(file_ptr);
        exit(1);
    }

    size_t readCount = fread(buffer, 1, size, file_ptr);
    if (readCount != (size_t)size) {
        print(WIN_STDERR, "failed to read entire file %s", filePath);
        free(buffer);
        fclose(file_ptr);
        exit(1);
    }

    buffer[size] = '\0';

    file.contents.len = (unsigned int)size;
    file.contents.data = buffer;

    fclose(file_ptr);
}