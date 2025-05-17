
#define BL_STRINGBUILDER_IMPL
#define BL_IMPL
#include "bl.h"

StringBuilder* sb;

typedef struct {
  char* name;
  int num_holes;
  char *code;
  char *args;
} PreStencil;

#define num_stencils 3

const char* arg0    = "0x0fefefe0";
const char* arg0_64 = "0x0fefefef0fefefe0";
const char* arg1    = "0x0fefefe1";
const char* arg1_64 = "0x0fefefef0fefefe1";


PreStencil add_pre = {
    .name = "add",
    .num_holes = 2,
    .args = "uintptr_t stack, int lhs, int rhs",
};

PreStencil add_const_pre = {
    .name = "add_const",
    .num_holes = 2,
    .args = "uintptr_t stack, int lhs",
};

PreStencil if_pre = {
    .name = "if_test",
    .num_holes = 2,
    .args = "uintptr_t stack, int condition, int x",
};

PreStencil pres[num_stencils];


int main() {
  StringBuilder *sb;

  // Build function for add
  sb = new_builder(64);
  add_to(sb, "int result = lhs + rhs; ");
  add_to(sb, "((cps_int)(%s))(stack, result);", arg0_64);
  add_pre.code = to_string(sb);

  // Build function for add_const
  sb = new_builder(64);
  add_to(sb, "int result = lhs + %s; ", arg0);
  add_to(sb, "((cps_int)(%s))(stack, result);", arg1_64);
  add_const_pre.code = to_string(sb);

  // Build function for if_test
  sb = new_builder(64);
  add_to(sb, "if (condition) { ((cps_int)(%s))(stack, x); } else { ((cps_int)(%s))(stack, x); }", arg0_64, arg1_64);
  if_pre.code = to_string(sb);

  pres[0] = add_pre;
  pres[1] = add_const_pre;
  pres[2] = if_pre;

  sb = new_builder(1024);

  add_to(sb, "#include \"stddef.h\"\n");
  add_to(sb, "#include \"../stencil.h\"\n");
  add_to(sb, "\n");

  fori(num_stencils) {
    PreStencil pre = pres[i];
  
    // The function (and end function to know its length)
    add_to(sb, "// Stencil generator for %s \n", pre.name);
    add_to(sb, "void %s(%s) {\n", pre.name, pre.args);
    add_to(sb, "\t%s\n", pre.code);
    add_to(sb, "}\n");
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

    add_to(fun_list, "stencils[%d] = (Stencil){ \n\t.name = \"%s\",\n\t.code = (uint8_t*)%s,\n\t.code_size = (uint32_t)((uint8_t*)%s_end - (uint8_t*)%s),\n\t.num_holes = %d\n};\n",
	   i, pre.name, pre.name, pre.name, pre.name, pre.num_holes);
  }
  add_to(fun_list, "};\n");

  add_to(sb, to_string(fun_list));
  
  // printf("%s\n", to_string(sb));
  FILE *f = fopen("generated/stencils.h", "wb");
  fwrite(to_string(sb), 1, sb->index, f);
}
