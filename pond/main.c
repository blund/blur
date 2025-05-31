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

  FuncDecl *fd = new_func_decl("void", "if_test", (Parameters){
      .arg_count = 3,
      .names = {make_unit("stack"), make_unit("condition"), make_unit("x")},
      .types = {make_type("uintptr_t", 0), make_type("int", 0), make_unit("int")},
    });
    IfBlock *ib = new_if_block(fd->body);
    PointerCall *ptr_call = new_pointer_call(
        ib->body, "void", "0xfefefef0",
        (Parameters){
            .arg_count = 2,
            .names = {make_unit("a"), make_unit("b")},
            .types = {make_type("int", 0), make_type("int", 0)},
        });
  fd->body->next = new_block();
  {
    Block *b = fd->body->next;
    PointerCall *ptr_call = new_pointer_call(b, "void", "0xfefefef1", (Parameters){
	.arg_count = 2,
	.names = {make_unit("a"), make_unit("b")},
	.types = {make_type("int", 0), make_type("int", 0)},
      });
  }

  char* buffer[1024];

  puts("------------------");
  puts("generated code");
  puts("------------------");
  print_func_decl(&p, fd);
}
