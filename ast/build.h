#ifndef BLUR_AST_BUILD_H
#define BLUR_AST_BUILD_H

#include <stdarg.h>
#include <ast/ast.h>

#define NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) N
#define NUM_ARGS(...) NUM_ARGS_IMPL(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

#define block(...) block_multi(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)
#define args(...) args_multi(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)
#define params(...) params_multi(NUM_ARGS(__VA_ARGS__), __VA_ARGS__)

Block *block_multi(int count, ...);
Parameters *params_multi(int count, ...);
Arguments *args_multi(int count, ...);
Statement *if_test(Expression* condition, Block *s1, Block* s2);
Statement *assign(char* name, Expression* e);
Statement *declare(char* name, Type type);
Statement *call(char* name, Arguments *args);
Expression *call_e(char* name, Arguments *args);
Expression *identifier(char* name);
Expression *integer(int n);
FuncDecl *func_decl(Type ret, char* name, Parameters *params, Block* body);
Var var(char *name, Type type);
Type type(char *name);

#endif
