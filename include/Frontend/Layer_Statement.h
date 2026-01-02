#ifndef STMT_LAYER_FRONTEND
#define STMT_LAYER_FRONTEND
#include <Frontend/Layer_Xpression.h>
#include <Utils/strings.h>
#include <stdint.h>

enum StmtType {
    STMT_EXPR,
    STMT_VAR_DECL,
    STMT_VAR_DEFN,
    STMT_BLOCK_START,
    STMT_BLOCK_END,
    STMT_MATCH,
    STMT_CONDITIONAL,
};

typedef struct VarDecl VarDecl;
typedef struct VarDefn VarDefn;
typedef struct Stmt Stmt;
typedef enum StmtType StmtType;
typedef union StmtValue StmtValue;
typedef struct StmtNode StmtNode;
typedef struct Line_Context Line_Context;
// typedef struct PatternMatch PatternMatch;
typedef struct StmtConditional StmtConditional;
typedef struct TypeList TypeList;
struct VarDecl {
    String name;
    String type;
    TypeList* args;

    Stmt* init;
    bool has_init;
};

struct TypeList {
    VarDecl var[5];
    int count;
};

struct VarDefn {
    String name;
    Stmt* val;
};

union StmtValue {
    Expr expr;
    VarDecl var_decl;
    VarDefn var_defn;
    StmtConditional* cond;
    Expr token;
    Funcall* funcall;
    StmtNode* block;
    // PatternMatch    *match;
};

struct Stmt {
    StmtType type;
    StmtValue as;
};

struct StmtNode {
    Stmt statement;
    StmtNode* next;
};

struct StmtConditional {
    bool repeat;
    Expr cond;
    Stmt body;
};

// struct PatternMatch {
// 	Stmt *cond;
// 	Stmt *body;
// };

Stmt stmt_fetch_next(Line_Context* ctx);

#endif