#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <Utils/mem_manager.h>

Partition *partition_create(size_t capacity)
{
	const size_t partSize = sizeof(Partition) + capacity;
	Partition *part = malloc(partSize);
	memset(part, 0, partSize);
	part->capacity = capacity;
	return part;
}

char *region_insert_or_expand(Region *region, Partition *cur, size_t size,
			       size_t alignedAddressMask)
{
	uintptr_t tmp = (uintptr_t)(cur->buffer + cur->size);
	tmp = (tmp + alignedAddressMask) & ~alignedAddressMask;

	char *ptr = (char *)tmp;
	size_t realSize = (ptr + size) - (cur->buffer + cur->size);

	if (cur->size + realSize <= cur->capacity) {
		memset(ptr, 0, realSize);
		cur->size += realSize;
		return ptr;
	}

	if (cur->next) {
		return region_insert_or_expand(region, cur->next, size,
						alignedAddressMask);
	}

	size_t worstCase = (size + alignedAddressMask) & ~alignedAddressMask;

	Partition *part = partition_create(worstCase > REGION_DEFAULT_CAPACITY ?
						  worstCase :
						  REGION_DEFAULT_CAPACITY);

	cur = region->last;
	region->last->next = part;
	region->last = part;

	return region_insert_or_expand(region, cur->next, size,
					alignedAddressMask);
}

void *region_allocate_aligned(Region *region, size_t size, size_t alignment)
{
	if (region->last == NULL) {
		assert(region->first == NULL);

		Partition *part =
			partition_create(size > REGION_DEFAULT_CAPACITY ?
						size :
						REGION_DEFAULT_CAPACITY);

		region->last = part;
		region->first = part;
	}

	if (size == 0) {
		return region->last->buffer + region->last->size;
	}

	assert((alignment & (alignment - 1)) == 0);

	return region_insert_or_expand(region, region->last, size,
					alignment - 1);
}

void *region_allocate(Region *region, size_t size)
{
	return region_allocate_aligned(region, size, sizeof(void *));
}

const char *region_load_string(Region *region, String str)
{
	char *cstr = region_allocate(region, str.len + 1);
	memcpy(cstr, str.data, str.len);
	cstr[str.len] = '\0';
	return cstr;
}

String region_concat_str(Region *region, const char *a, const char *b)
{
	const size_t aLen = strlen(a);
	const size_t bLen = strlen(b);
	char *buf = region_allocate(region, aLen + bLen);
	memcpy(buf, a, aLen);
	memcpy(buf + aLen, b, bLen);
	return (String){ .len = aLen + bLen, .data = buf };
}

void region_clear(Region *region)
{
	for (Partition *part = region->first; part != NULL; part = part->next) {
		part->size = 0;
	}
	region->last = region->first;
}

void region_free(Region *region)
{
	for (Partition *part = region->first, *next = NULL; part != NULL;
	     part = next) {
		next = part->next;
		free(part);
	}
}