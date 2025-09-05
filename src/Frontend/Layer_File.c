#include <Frontend/Layer_File.h>
#include <Middleend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Wrapper/IO.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
	char *buf = region_allocate(n+1);
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

Error file_read(const char *file_path, File_Context *file)
{
	char *file_contents = NULL;
	FILE *file_ptr 		= NULL;
	size_t size 		= 0;
	Error out 			= ERR_OK;

	// print(WIN_STDOUT, "\n[FILE] Reading File     : %s", file_path);

	ERROR_CHECK(out, goto cleanup, file_open(file_path, "r", &file_ptr));
	ERROR_CHECK(out, goto cleanup, file_get_size(file_ptr, &size));
	ERROR_CHECK(out, goto cleanup, file_get_contents(file_ptr, size, &file_contents));

	file->contents.len 	= size;
	file->contents.data = file_contents;
	file->file_path 	= file_path;
	file->line_num 		= 0;

	CodeBlock global 	= codeblock_generate(file);

	AST_generate(&global, true);
	IR_generate(&global);


cleanup:
	if (file_ptr) {
		fclose(file_ptr);
	}
	// free(file_contents);
	return out;
}

Line_Context *file_fetch_next_line(File_Context *file)
{
	Line_Context *curr 	 = &(file->lines[file->line_num]);
	curr->file_name      = file->file_path;
	curr->line 			 = split_str_by_delim(&file->contents, '\n');
	curr->line.data[curr->line.len] = '\0';
	curr->line_start 	 = curr->line.data;
	file->line_num 		+= 1;
	curr->line_no		 = file->line_num;
	curr->line			 = trim(curr->line);

	log_to_ctx(curr, LOG_FORMAT, LOG_CTX("","--xx--"));
	log_to_ctx(curr, LOG_FORMAT "\"%.*s\"", LOG_CTX("[READING]", "[LINE]"),
			Str_Fmt(curr->line));

	return curr;
}

Line_Context* file_fetch_curr_line(File_Context* file)
{
	assert(file->line_num);
	return &file->lines[file->line_num-1];
}