
#define STB_DS_IMPLEMENTATION
#include <include/stb_ds.h>

#define BL_STRINGBUILDER_IMPL
#define BL_IMPL
#include <bl.h>

#include <codegen/print_c_code.h>

#include <ast/build.h>
#include <stencil.h>

FuncDecl *build_add_ast();
FuncDecl *build_if_ast();

StringBuilder* sb;

typedef struct {
  char* name;
  int num_32_holes;
  int num_64_holes;
  StringBuilder* code;
} PreStencil;

#define num_stencils 2

PreStencil add_const_pre = {
    .name = "add_const",
    .num_32_holes = 1,
    .num_64_holes = 1,
};

PreStencil if_test_pre = {
    .name = "if_test",
    .num_64_holes = 2,
};


PreStencil pres[num_stencils];


int main() {
  dprintf(" [ Generating Stencils ]\n");

  FuncDecl *if_ast = build_if_ast();
  FuncDecl *add_ast = build_add_ast();

  add_const_pre.code = new_builder(1024);
  print_func_decl(add_const_pre.code, add_ast);
  //print_func_decl(&p, add_const_ast);

  if_test_pre.code = new_builder(1024);
  print_func_decl(if_test_pre.code, if_ast);

  pres[0] = add_const_pre;
  pres[1] = if_test_pre;

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

#define i identifier
FuncDecl *build_if_ast() {
  return func_decl(type("void"),
		   "if_test",
		   params(var("stack", type("uintptr_t")),
			  var("condition", type("int")),
			  var("x", type("int"))),
		   block(if_test(i("condition"),
				 block(call("pointer_call",
					    args( i("void"), i(STR(big_hole_1)),
						  i("uintptr_t"), i("stack"),
						  i("int"), i("x")))),
				 block(call("pointer_call",
					    args( i("void"), i(STR(big_hole_2)),
						  i("uintptr_t"), i("stack"),
						  i("int"), i("x")))))));
}

FuncDecl *build_add_ast() {
  return func_decl(type("void"),
		   "add_const",
		   params(var("stack", type("uintptr_t")),
			  var("lhs", type("int"))),
		   block(
			 declare("result", type("int")),
			 assign("result", 
				call_e("add", args(i("lhs"), i(STR(small_hole_1))))),
			 call("pointer_call",
			      args( i("void"), i(STR(big_hole_1)),
				    i("uintptr_t"), i("stack"),
				    i("int"), i("result")))));
}
