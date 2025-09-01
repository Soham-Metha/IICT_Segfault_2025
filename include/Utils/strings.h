#ifndef STRINGS
#define STRINGS

typedef struct String String;

struct String {
	unsigned int len;
	const char *data;
};

#endif