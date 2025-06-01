#ifndef BUILD_H
#define BUILD_H

#include "ast.h"

Type make_type(char *str, int ptr);
Unit make_unit(char *str);
Block *new_block();
Statement *new_statement();
IfBlock *new_if_block(Block *b, char* condition);
PointerCall *new_pointer_call(Block *b, char* ret, char* name, Parameters params);
FuncDecl *new_func_decl(char *ret_type, char *name, Parameters params);
Parameter new_parameter(char *type, char *name);
Return *new_return(Block *b);
BinOp* new_binop(Expr *expr, char *lhs, char *op, char *rhs);
Block *next_block(Block *b);
Assign *new_assign(Block *b, char* type, char* name);
ArrayWrite* new_array_write(Block *b, char* array, char *index, char* value);

#endif
