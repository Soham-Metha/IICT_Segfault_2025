#pragma once
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// #include "Utils/strings.h"

typedef struct Partition Partition;
typedef struct Region Region;

struct Partition {
	Partition *next;
	size_t capacity;
	size_t size;
	char buffer[];
};

#define REGION_DEFAULT_CAPACITY 65536

struct Region {
	Partition *first;
	Partition *last;
};

Partition *partition_create(size_t capacity);

void *region_allocate(size_t size);

// const char *region_load_string(String str);

// String region_concat_str(const char *a, const char *b);

void region_clear();

void region_free();