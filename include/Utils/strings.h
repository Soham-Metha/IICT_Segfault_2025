#ifndef STRINGS
#define STRINGS
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

typedef struct String String;

struct String {
    unsigned int len;
    char* data;
};

#define STR(s) ((String) { .len = strlen(s), .data = s })
#define Str_Fmt(s) s.len, s.data

String ltrim(String s);
String rtrim(String s);
String trim(String s);

bool compare_str(String a, String b);

bool get_index_of(String str, char c, size_t* index);

String split_str_by_delim(String* s, char c);
String split_str_by_condition(String* str, bool (*predicate)(char x));
String split_str_by_len(String* str, size_t n);
String split_str_by_len_reversed(String* str, size_t n);

bool starts_with(String str, String prefix);
bool ends_with(String str, String expected);

#endif