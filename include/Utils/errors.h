#ifndef ERRORS
#define ERRORS

enum ErrorType {
	ERR_OK,
	ERR_FILE_CANT_OPEN,
	ERR_FILE_CANT_READ,
	ERR_FILE_TOO_LARGE,
	ERR_RAN_OUT_OF_MEM,
	ERR_AST_RENDER_ERR,
};

typedef enum ErrorType Error;

#define ERROR_CHECK(var, action, cond) \
	{                              \
		var = cond;            \
		if (var != ERR_OK) {   \
			action;        \
		}                      \
	}

#define ERROR_THROW_IF(err, cond) \
	if (cond) {               \
		return err;       \
	}

#endif