#include "ast.h"

void print_unit(Parser* p, Unit u);
void print_type(Parser* p, Type t);
void print_value(Parser* p, Value v);
void print_call(Parser* p, Call c);
void print_if_block(Parser* p, IfBlock ib);
void print_expr(Parser* p, Expr* e);
void print_statement(Parser* p, Statement* s);
void print_assign(Parser* p, Assign a);
void print_block(Parser* p, Block* b);
void print_func_decl(Parser* p, FuncDecl* f);
