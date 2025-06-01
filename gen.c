
#define BL_STRINGBUILDER_IMPL
#define BL_IMPL
#include "bl.h"

#include "pond/ast.h"
#include "pond/build.h"
#include "pond/print.h"

#include "stencil.h"

FuncDecl *build_add_const_ast();
FuncDecl *build_if_test_ast();
FuncDecl *build_stack_write_ast();

StringBuilder* sb;

typedef struct {
  char* name;
  int num_32_holes;
  int num_64_holes;
  StringBuilder* code;
} PreStencil;

#define num_stencils 3

PreStencil add_const_pre = {
    .name = "add_const",
    .num_32_holes = 1,
    .num_64_holes = 1,
};

PreStencil if_test_pre = {
    .name = "if_test",
    .num_64_holes = 2,
};

PreStencil stack_write_pre = {
    .name = "stack_write",
    .num_32_holes = 2,
    .num_64_holes = 1,
};


PreStencil pres[num_stencils];


int main() {
  dprintf(" Running stencil code generation...\n");

  FuncDecl *if_test_ast = build_if_test_ast();
  FuncDecl *add_const_ast = build_add_const_ast();
  FuncDecl *stack_write_ast = build_stack_write_ast();

  Parser p = {0};

  // Construct add_const code
  add_const_pre.code = new_builder(1024);
  p.output = add_const_pre.code;
  print_func_decl(&p, add_const_ast);

  // Construct if_pre_code
  if_test_pre.code = new_builder(1024);
  p.output = if_test_pre.code;
  print_func_decl(&p, if_test_ast);

  // Construct if_pre_code
  stack_write_pre.code = new_builder(1024);
  p.output = stack_write_pre.code;
  print_func_decl(&p, stack_write_ast);


  pres[0] = add_const_pre;
  pres[1] = if_test_pre;
  pres[2] = stack_write_pre;

  StringBuilder* sb = new_builder(1024);

  add_to(sb, "#include \"stddef.h\"\n");
  add_to(sb, "#include \"../stencil.h\"\n");
  add_to(sb, "\n");

  fori(num_stencils) {
    PreStencil pre = pres[i];
    dprintf("  -- generation %s\n", pre.name);
  
    // The function (and end function to know its length)
    add_to(sb, "// Stencil generator for %s \n", pre.name);
    add_to(sb, "%s\n", to_string(pre.code));
    add_to(sb, "void %s_end() {};\n", pre.name);
    add_to(sb, "\n");
  }

  // Build a list of stencils to use for cutting later
  StringBuilder *fun_list = new_builder(128);
  add_to(fun_list, "// Here we construct a list of our generated stencils that we\n");
  add_to(fun_list, "// will operate on later\n");
  add_to(fun_list, "int num_stencils = %d;\n", num_stencils);
  add_to(fun_list, "Stencil stencils[%d];\n", num_stencils);
  add_to(sb, "\n");
  
  add_to(fun_list, "void build_stencils() {\n", num_stencils);

  
  fori(num_stencils) {
    PreStencil pre = pres[i];

    add_to(fun_list, "stencils[%d] = (Stencil){ \n\t.name = \"%s\",\n\t.code = (uint8_t*)%s,\n\t.code_size = (uint32_t)((uint8_t*)%s_end - (uint8_t*)%s),\n\t.num_holes_32 = %d,\n\t.num_holes_64 = %d\n};\n",
	   i, pre.name, pre.name, pre.name, pre.name, pre.num_32_holes, pre.num_64_holes);
  }
  add_to(fun_list, "};\n");

  add_to(sb, to_string(fun_list));
  
  // printf("%s\n", to_string(sb));
  FILE *f = fopen("generated/stencils.h", "wb");
  fwrite(to_string(sb), 1, sb->index, f);
}

FuncDecl *build_if_test_ast() {
  FuncDecl *if_test = new_func_decl("void", "if_test",
				    (Parameters){.count = 3,
						 .entries = {
						   new_parameter("uintptr_t", "stack"),
						   new_parameter("int", "condition"),
						   new_parameter("int", "x"),
						 }
				    });
  IfBlock *if_block = new_if_block(if_test->body, "condition");
  new_pointer_call(if_block->if_block, "void", STR(big_hole_1),
		   (Parameters){.count = 2,
				.entries = {
				  new_parameter("uintptr_t", "stack"),
				  new_parameter("int", "x"),
				}});
  new_pointer_call(if_block->else_block, "void", STR(big_hole_2),
		   (Parameters){
		     .count = 2,
		     new_parameter("uintptr_t", "stack"),
		     new_parameter("int", "x"),
		   });

  return if_test;
};

FuncDecl *build_add_const_ast() {
  FuncDecl *add_const =
    new_func_decl("void", "add_const",
		  (Parameters){.count = 2,
			       .entries = {
				 new_parameter("uintptr_t", "stack"),
				 new_parameter("int", "lhs"),
			       }});
  Assign *a = new_assign(add_const->body, "int", "result");
  new_binop(a->expr, "lhs", "+", STR(small_hole_1));
  new_pointer_call(next_block(add_const->body), "void", STR(big_hole_1),
		   (Parameters){
		     .count = 2,
		     new_parameter("uintptr_t", "stack"),
		     new_parameter("int", "result"),
		   });
  return add_const;
}

FuncDecl *build_stack_write_ast() {
  FuncDecl *stack_write =
    new_func_decl("void", "stack_write",
		  (Parameters){.count = 1,
			       .entries = {
				 new_parameter("uintptr_t", "stack"),
			       }});
  new_array_write(stack_write->body, "stack", STR(small_hole_1),
                  STR(small_hole_2));
  new_pointer_call(next_block(stack_write->body), "void", STR(big_hole_1),
		   (Parameters){
		     .count = 1,
		     new_parameter("uintptr_t", "stack"),
		   });

  return stack_write;
}
