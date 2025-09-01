#ifndef STRINGS
#define STRINGS

struct String String;

struct String {
	unsigned int len;
	const char *data;
};

#endif