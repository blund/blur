#include <string.h>

#include "ast.h"

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
