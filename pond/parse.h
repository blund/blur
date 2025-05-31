#include <string.h>

#include "ast.h"


Type make_type(char *str, int ptr);
Unit make_unit(char *str);
Block *new_block();
Statement *new_statement();
IfBlock *new_if_block(Block *b);
PointerCall *new_pointer_call(Block *b, char* ret, char* name, Parameters params);
FuncDecl *new_func_decl(char *ret_type, char *name, Parameters params);
Parameter new_parameter(char *type, char *name);
Return *new_return(Block *b);
BinOp* new_binop(Expr *expr, char *lhs, char *op, char *rhs);
Block *next_block(Block *b);
Assign *new_assign(Block *b, char* type, char* name);

void parse_error(Parser* p, int start);

void parse_whitespace(Parser* p);
Unit parse_text(Parser* p);
Value parse_number(Parser* p);
void parse_exact(Parser* p, char c);

Value parse_string(Parser* p);
Type parse_type(Parser* p);
Call parse_call(Parser* p);

Expr* parse_expr(Parser* p);
Assign parse_assign(Parser* p);
FuncDecl parse_func_decl(Parser* p);

Statement* parse_statement(Parser* p);
Block* parse_block(Parser* p);
Block* parse_scope(Parser* p);
IfBlock parse_if_block(Parser* p);
