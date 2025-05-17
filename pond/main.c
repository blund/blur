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
int if_test(int a, int b) { \n\
  if (condition) {\n\
      ((void (*)(int, int))(0xfefe))(x, y);\n\
  }\n\
  ((void (*)(int, int))(0x3232))(x, y);\n\
}";


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
   
}
