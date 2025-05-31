#include <stdio.h>

#include "ast.h"
#include "parse.h"
#include "print.h"

#define BL_STRINGBUILDER_IMPL
#define BL_IMPL
#include "../bl.h"


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

int main() {
  Parser p = {
    .output = new_builder(1024),
  };

  FuncDecl *if_test = new_func_decl("void", "if_test",
		  (Parameters){.count = 3,
			       .entries = {
				 new_parameter("uintptr_t", "stack"),
				 new_parameter("int", "condition"),
				 new_parameter("int", "x"),
			       }
		  });
  IfBlock *if_block = new_if_block(if_test->body);
  new_pointer_call(if_block->body, "void", "0xfefefef0",
		   (Parameters){.count = 2,
				.entries = {
				  new_parameter("int", "a"),
				  new_parameter("int", "b"),
				}});
  new_pointer_call(next_block(if_test->body), "void", "0xfefefef1",
		   (Parameters){
		     .count = 2,
		     new_parameter("int", "a"),
		     new_parameter("int", "b"),
		   });

  char* buffer[1024];

  puts("------------------");
  puts("generated code");
  puts("------------------");
  print_func_decl(&p, if_test);
  printf("%s\n", to_string(p.output));
  reset(p.output);


  FuncDecl *add_const =
      new_func_decl("void", "add_const",
                    (Parameters){.count = 2,
                                 .entries = {
                                     new_parameter("uintptr_t", "stack"),
                                     new_parameter("int", "lhs"),
                                 }});
  Return *r = new_return(add_const->body);
  BinOp* b = new_binop(&r->expr, "lhs", "+", "rhs");

  puts("------------------");
  puts("generated code");
  puts("------------------");
  print_func_decl(&p, add_const);

  printf("%s\n", to_string(p.output));
  reset(p.output);

}

