

#define BL_STRING_BUILDER_IMPL
#define BL_IMPL
#include "bl.h"

string_builder* sb;

typedef struct {
  char* name;
  int num_holes;
  char* code;
} pre_stencil;

#define num_stencils 3

const pre_stencil add_pre = {
    .name = "add",
    .num_holes = 2,
    .code = "int a = 0xfffffff1; int b = 0xfffffff0; int c = a + b; return c;",
};

const pre_stencil mul_pre = {
  .name = "mul",
  .num_holes = 2,
  .code = "int a = 0xfffffff1; int b = 0xfffffff0; int c = a * b; return c;",
};


const pre_stencil array_index_pre = {
  .name = "array_index",
  .num_holes = 2,
  .code = "int* a = 0xfffffff1; int b = 0xfffffff0; int c = a[b]; return c;",
};


pre_stencil pres[num_stencils];

int main() {
  pres[0] = add_pre;
  pres[1] = mul_pre;
  pres[2] = array_index_pre;

  sb = new_builder(1024);

  add_to(sb, "#include \"stddef.h\"\n");
  add_to(sb, "#include \"../stencil.h\"\n");
  add_to(sb, "\n");

  fori(num_stencils) {
    pre_stencil pre = pres[i];
  
    // The function (and end function to know its length)
    add_to(sb, "// Stencil generator for %s \n", pre.name);
    add_to(sb, "int %s() {\n", pre.name);
    add_to(sb, "\t%s\n", pre.code);
    add_to(sb, "}\n");
    add_to(sb, "void %s_end() {};\n", pre.name);
    add_to(sb, "\n");

    // Save its length also
    add_to(sb, "uint32_t %s_holes = %d;\n", pre.name, pre.num_holes);
    add_to(sb, "\n");

  }

  // Build a list of stencils to use for cutting later
  string_builder *fun_list = new_builder(128);
  add_to(fun_list, "// Here we construct a list of our generated stencils that we\n");
  add_to(fun_list, "// will operate on later\n");
  add_to(fun_list, "int num_stencils = %d;\n", num_stencils);
  add_to(fun_list, "stencil_t stencils[%d];\n", num_stencils);
  add_to(sb, "\n");
  
  add_to(fun_list, "void build_stencils() {\n", num_stencils);

  
  fori(num_stencils) {
    pre_stencil pre = pres[i];

    add_to(fun_list, "stencils[%d] = (stencil_t){ \n\t.name = \"%s\",\n\t.code = (uint8_t*)%s,\n\t.code_size = (uint32_t)((uint8_t*)%s_end - (uint8_t*)%s),\n\t.num_holes = %d\n};\n",
	   i, pre.name, pre.name, pre.name, pre.name, pre.num_holes);
  }
  add_to(fun_list, "};\n");

  add_to(sb, to_string(fun_list));
  
  // printf("%s\n", to_string(sb));
  FILE *f = fopen("generated/stencils.h", "wb");
  fwrite(to_string(sb), 1, sb->index, f);
}
