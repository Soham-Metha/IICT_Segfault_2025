#ifndef LINE_LAYER_FRONTEND
#define LINE_LAYER_FRONTEND
#define COMMENT_SYMBOL '%'
#include <Utils/strings.h>

typedef struct Line_View Line_View;
typedef struct Line_as_Directive Line_as_Directive;
typedef struct Line_as_Function Line_as_Function;
typedef union Line_Value Line_Value;
typedef enum Line_Type Line_Type;

enum Line_Type { LINE_TYPE_DIRECTIVE, LINE_TYPE_FUNCTION, LINE_TYPE_COUNT };

struct Line_as_Directive {
	String name;
	String body;
};

struct Line_as_Function {
	String name;
	String arglist;
};

union Line_Value {
	Line_as_Directive as_Directive;
	Line_as_Function as_Function;
};

struct Line_View {
	Line_Type type;
	Line_Value value;
};

String getNextLine();

#endif