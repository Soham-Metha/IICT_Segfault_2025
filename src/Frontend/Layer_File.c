#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Wrapper/IO.h>
#include <stdio.h>
#include <stdlib.h>

File_View file;
// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

Error file_open(const char *file_path, const char *mode, FILE **out)
{
	FILE *f = fopen(file_path, mode);
	ERROR_THROW_IF(ERR_FILE_CANT_OPEN, (f == NULL))

	*out = f;
	return ERR_OK;
}

Error file_get_size(FILE *f, size_t *out)
{
	// See if we can go to the end of file
	ERROR_THROW_IF(ERR_FILE_CANT_READ, (fseek(f, 0, SEEK_END) != 0))

	// Check our current position in terms of byte count
	// Since we are at the end, we get byte count of the file.
	long fileSize = ftell(f);
	ERROR_THROW_IF(ERR_FILE_CANT_READ, (fileSize < 0))

	// since we already have file size,
	// go back to the beginning of file
	rewind(f);

	*out = (size_t)fileSize;
	return ERR_OK;
}

Error file_get_contents(FILE *f, size_t n, char **contents)
{
	// Try to allocate enough space
	// Add 1 to size for null termination
	char *buf = malloc(n + 1);
	ERROR_THROW_IF(ERR_RAN_OUT_OF_MEM, (!buf))

	// Now that we have space, try to read contents of the file
	if (fread(buf, 1, n, f) != n) {
		free(buf);
		return ERR_FILE_CANT_READ;
	}

	// NULL termination!
	buf[n] = '\0';

	*contents = buf;
	return ERR_OK;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

Error file_read(const char *file_path)
{
	char *file_contents = NULL;
	FILE *file_ptr 		= NULL;
	size_t size 		= 0;
	Error out 			= ERR_OK;

	print(WIN_STDOUT, "\n[FILE] Reading File     : %s", file_path);

	ERROR_CHECK(out, goto cleanup, file_open(file_path, "r", &file_ptr));
	ERROR_CHECK(out, goto cleanup, file_get_size(file_ptr, &size));
	ERROR_CHECK(out, goto cleanup, file_get_contents(file_ptr, size, &file_contents));

	fclose(file_ptr);

	file.contents.len 	= size;
	file.contents.data 	= file_contents;
	file.file_path 		= file_path;
	file.line_num 		= 0;

	CodeBlock global 	= block_generate();

	ERROR_CHECK(out, goto cleanup, AST_generate(&global, false));

cleanup:
	if (file_ptr) {
		fclose(file_ptr);
	}
	free(file_contents);
	return out;
}

String file_fetch_next_line()
{
	String line = { 0 };

	// if file is empty, return empty line
	if (file.contents.len <= 0) {
		return line;
	}
	// Preprocessing, increment line number and discard leading blank space
	file.line_num += 1;
	file.contents = ltrim(file.contents);

	// read next line & discard its leading/trailing blankspace
	line = split_str_by_delim(&file.contents, '\n');
	line = trim(line);
	print(WIN_STDOUT, "\n[LINE] Reading Line %3u : %.*s", file.line_num,
	      Str_Fmt(line));

	return line;
}