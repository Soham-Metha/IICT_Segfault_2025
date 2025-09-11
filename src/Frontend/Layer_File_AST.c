#include <Frontend/Layer_File.h>
#include <Frontend/Layer_Line.h>
#include <Frontend/Layer_Statement.h>
#include <Frontend/Layer_Xpression.h>
#include <Wrapper/IO.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

static int AST_dump_code_block(const StmtNode* stmtNode, int* n, int* b);
static int AST_dump_statement(const Stmt* stmt, int* n, int* b);
static int AST_dump_expression(const Expr* expr, int* n, int* b);

int __EXPR_TYPE_TOKEN(int id, String value)
{
    print_AST(AST("note", "lightblue", "%.*s"), id, Str_Fmt(value));
    return id;
}

int __STMT_UNKNOWN(int id)
{
    print_AST("  Expr_%d [label=\"Stmt_unknown\"];\n", id);
    return id;
}

int __STMT_FUNCALL(int id, int* n, int* b, const Funcall* funcall)
{
    (void)b;
    print_AST(AST("hexagon", "lightpink", "%.*s"), id, Str_Fmt(funcall->name));

    for (const FuncallArg* arg = funcall->args; arg != NULL;
        arg                    = arg->next) {
        int childId = AST_dump_expression(&arg->expr, n, b);
        if (childId >= 0) {
            print_AST("  Expr_%d -> Expr_%d[style=dotted];\n", id, childId);
            // print_AST("  {rank=same; Expr_%d; Expr_%d};\n", id,
            // 	  childId);
        }
    }
    return id;
}

int __STMT_BLOCK(int id, int* n, int* b, const StmtNode* block)
{
    (void)id;
    int clusterId  = (*n)++;
    int clusterNum = (*b)++;

    print_AST("  subgraph cluster_%d {\n"
              "    label=\"Code Block %d\";\n"
              "    style=filled;\n"
              "    color=gray;\n"
              "    fillcolor=whitesmoke;\n"
              "    fontname=\"Courier\";\n",
        clusterId, clusterNum);

    AST_dump_code_block(block, n, b);

    print_AST("  }\n" AST("box3d", "aquamarine", "Code Block %d"), clusterId, clusterNum);

    return clusterId;
}

// ------------------------------------------------------------- HELPERS ---------------------------------------------------------------------

static int AST_dump_expression(const Expr* expr, int* n, int* b)
{
    assert(expr != NULL);
    int myId = (*n)++;

    switch (expr->type) {
    case EXPR_TYPE_FUNCALL:
        return __STMT_FUNCALL(myId, n, b, &expr->as.funcall);
    case EXPR_TYPE_STR:
        return __EXPR_TYPE_TOKEN(myId, expr->as.str);
    case EXPR_TYPE_NUMBER:
        {
            char buf[32];
            int len        = snprintf(buf, sizeof(buf), "%" PRId64, expr->as.num);
            String str_num = { (unsigned int)len, buf };
            return __EXPR_TYPE_TOKEN(myId, str_num);
        }
    case EXPR_TYPE_VAR:
        return __EXPR_TYPE_TOKEN(myId, expr->as.var_nm);
    case EXPR_TYPE_BOOL:
        {
            String boolStr = expr->as.boolean ? (String) { 4, "true" } : (String) { 5, "false" };
            return __EXPR_TYPE_TOKEN(myId, boolStr);
        }
    case EXPR_TYPE_BIN_OPR:
        {
            print_AST(AST("house", "lightgreen", "%s"), myId, bin_opr_get_name(expr->as.bin_opr->type));
            int child1 = AST_dump_expression(&expr->as.bin_opr->lhs, n, b);
            int child2 = AST_dump_expression(&expr->as.bin_opr->rhs, n, b);
            print_AST("  Expr_%d -> Expr_%d[style=dotted];\n", myId, child1);
            print_AST("  Expr_%d -> Expr_%d[style=dotted];\n", myId, child2);

            // print_AST("  {rank=same; Expr_%d; Expr_%d};\n", myId, child1); // cause segfault
            // print_AST("  {rank=same; Expr_%d; Expr_%d};\n", myId, child2);
            return myId;
        }
    case EXPR_TYPE_OPEN_CURLY:
        print_AST(AST("box", "lightgray", "{"), myId);
        return myId;
    case EXPR_TYPE_CLOSING_CURLY:
        print_AST(AST("box", "lightgray", "}"), myId);
        return myId;
    case EXPR_TYPE_STATEMENT_END:
        print_AST(AST("ellipse", "gray", "StmtEnd"), myId);
        return myId;
    case EXPR_TYPE_THEN:
        print_AST(AST("ellipse", "lightyellow", "Then"), myId);
        return myId;
    case EXPR_TYPE_REPEAT:
        print_AST(AST("ellipse", "lightcyan", "Repeat"), myId);
        return myId;
    case EXPR_TYPE_COLON:
        print_AST(AST("box", "lightblue", ":"), myId);
        return myId;
    case EXPR_TYPE_EQUAL:
        print_AST(AST("box", "lightpink", "="), myId);
        return myId;
    default:
        return __STMT_UNKNOWN(myId);
    }
}

static int AST_dump_statement(const Stmt* stmt, int* n, int* b)
{
    assert(stmt != NULL);
    int myId = (*n)++;

    switch (stmt->type) {
    case STMT_BLOCK_START:
        return __STMT_BLOCK(myId, n, b, stmt->as.block);
    case STMT_EXPR:
        return AST_dump_expression(&stmt->as.expr, n, b);
    case STMT_VAR_DECL:
        {
            print_AST(AST("folder", "gold", "Decl: %.*s"), myId, Str_Fmt(stmt->as.var_decl.name));

            if (stmt->as.var_decl.args)
                for (int i = 0; i < stmt->as.var_decl.args->count; i++) {
                    VarDecl decl = stmt->as.var_decl.args->var[i];
                    int argId    = (*n)++;
                    print_AST(AST("folder", "gold", "Decl: %.*s"), argId, Str_Fmt(decl.name));
                    print_AST("  Expr_%d -> Expr_%d[style=dotted];\n", myId, argId);

                    print_AST("  {rank=same; Expr_%d; Expr_%d};\n", myId, argId);
                    if (decl.has_init && decl.init != NULL) {
                        int childId = AST_dump_statement(decl.init, n, b);
                        print_AST("  Expr_%d -> Expr_%d[style=dotted];\n", argId, childId);
                    }
                }

            if (stmt->as.var_decl.has_init && stmt->as.var_decl.init != NULL) {
                int childId = AST_dump_statement(stmt->as.var_decl.init,
                    n, b);
                print_AST("  Expr_%d -> Expr_%d[style=dotted];\n", myId, childId);
            }
            return myId;
        }
    case STMT_VAR_DEFN:
        {
            print_AST(AST("tab", "palegoldenrod", "Defn: %.*s"), myId, Str_Fmt(stmt->as.var_defn.name));
            if (stmt->as.var_defn.val != NULL) {
                int childId = AST_dump_statement(stmt->as.var_defn.val, n, b);
                print_AST("  Expr_%d -> Expr_%d[style=dotted];\n", myId, childId);
            }
            return myId;
        }
    case STMT_CONDITIONAL:
        {
            print_AST(AST("septagon", "lightslategrey", "%s"), myId, (stmt->as.cond->repeat) ? "While" : "If");
            int condId = AST_dump_expression(&stmt->as.cond->cond, n, b);
            int bodyId = AST_dump_statement(&stmt->as.cond->body, n, b);
            print_AST("  Expr_%d -> Expr_%d[style=dotted];\n", myId, condId);
            print_AST("  {rank=same; Expr_%d; Expr_%d};\n", myId, bodyId);
            print_AST("  Expr_%d -> Expr_%d;\n", myId, bodyId);
            return myId;
        }
    case STMT_BLOCK_END:
        print_AST(AST("ellipse", "gray", "BlockEnd"), myId);
        return myId;
    case STMT_MATCH:
        print_AST(AST("hexagon", "purple", "Match"), myId);
        return myId;
    default:
        return __STMT_UNKNOWN(myId);
    }

    // TODO: update match block once implemented!
}

static int AST_dump_code_block(const StmtNode* stmtNode, int* n, int* b)
{
    int firstId = -1;
    int prevId  = -1;

    for (const StmtNode* cur = stmtNode; cur != NULL; cur = cur->next) {
        int id = AST_dump_statement(&cur->statement, n, b);

        if (firstId < 0)
            firstId = id;
        if (prevId >= 0)
            print_AST("  Expr_%d -> Expr_%d;\n", prevId, id);

        prevId = id;
    }

    return firstId;
}

// ----------------------------------------------------------- ACTUAL WORK -------------------------------------------------------------------

Error AST_generate(const CodeBlock* blk, bool renderPng)
{
    int node_counter  = 0;
    int block_counter = 0;

    print_AST("digraph AST {\n"
              "  splines=ortho;\n"
              "  nodesep=0.8;\n"
              "  ranksep=0.5;\n"
              "  node [fontname=\"Courier\"];\n");

    AST_dump_code_block(blk->begin, &node_counter, &block_counter);
    print_AST("}\n");

    if (renderPng) {
        int r = system("dot -Tpng ast.dot -o ast.png ");
        ERROR_THROW_IF(ERR_AST_RENDER_ERR, (r != 0))
    }

    return ERR_OK;
}
