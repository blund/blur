#include <stdio.h>

#include "ast.h"
#include "parse.h"
#include "print.h"

char *program = "\
int main(int a, int b) { \n\
  ((void (*)(int, int))(test))(x, y);\n\
  int a = 123;\n\
  epic();\n\
  if () {\n\
    if () {\n\
      int bla = 123;\n\
    }\n\
  }\n\
  int a = 456;\n\
  int a = epic();\n\
}";

char *if_impl = "\
void if_test(int a, int b) { \n\
  if (condition) {\n\
      ((void (*)(int, int))(0xfefe))(x, y);\n\
  }\n\
  ((void (*)(int, int))(0x3232))(x, y);\n\
}";

Unit make_unit(char *str) {
  Unit u;
  u.ptr = str;
  u.start = 0;
  u.end = strlen(str);

  return u;
}

Type make_type(char *str, int ptr) {
  Type t;
  Unit u;
  u.ptr = str;
  u.start = 0;
  u.end = strlen(str);

  t.name = u;
  t.ptr = ptr;
  return t;
}



int main() {
  Parser p = {
    .code = if_impl,
    .len = strlen(if_impl),
    .index = 0,

    // This is for the printing after parsing
    .indent = 0,
  };

  FuncDecl f = parse_func_decl(&p);
  puts("------------------");
  puts("reconstructed code");
  puts("------------------");
  print_func_decl(&p, &f);


  puts("");

  FuncDecl f2;
  f2.name = make_unit("if_test");
  f2.ret = make_type("void", 0);

  f2.parameters = (Parameters){
      .arg_count = 2,
      .names = {make_unit("a"), make_unit("b")},
      .types = {make_type("int", 0), make_type("int", 0)},
  };

  f2.body = new_block();
  {
    f2.body->statement = new_statement();
    Statement *s = f2.body->statement;
    s->kind = statement_if_kind;
    s->if_block.condition = make_unit("condition");
    s->if_block.body = new_block();
    s->if_block.body->statement = new_statement();
    s->if_block.body->statement->kind = statement_pointer_call_kind;
    PointerCall *ptr_call = &s->if_block.body->statement->pointer_call;

    ptr_call->operand = make_unit("0xfefefef0");
    ptr_call->return_type = make_type("void", 0);
    ptr_call->num_parameters = 0;
    ptr_call->num_arguments = 0;
  }
  f2.body->next = new_block();
  {
    Block* b = f2.body->next;
    b->statement = new_statement();
    Statement *s = b->statement;
    s->kind = statement_pointer_call_kind;
    PointerCall *ptr_call = &s->pointer_call;

    ptr_call->operand = make_unit("0xfefefef1");
    ptr_call->return_type = make_type("void", 0);
    ptr_call->num_parameters = 0;
    ptr_call->num_arguments = 0;
  }


  puts("------------------");
  puts("generated code");
  puts("------------------");
  print_func_decl(&p, &f2);


   
}
