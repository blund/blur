
#include <math.h>

#define STB_DS_IMPLEMENTATION
#include <include/stb_ds.h>

#define BL_STRINGBUILDER_IMPL
#define BL_IMPL
#include <bl.h>

#include <ast/ast.h>
#include <ast/build.h>
#include <stencil.h>
#include <codegen/print_c_code.h>

#define i identifier

// 'small_ord' is used to track how many 32 bit holes are in the constructed code
int small_ord = 0;
char *small_sentinels[] = {
  STR(small_hole_1),
  STR(small_hole_2),
  STR(small_hole_3),
  STR(small_hole_4),
};

// 'big_ord' is used to track how many 64 bit holes are in the constructed code
int big_ord = 0;
char *big_sentinels[] = {
  STR(big_hole_1),
  STR(big_hole_2),
  STR(big_hole_3),
  STR(big_hole_4),
};

char *return_types[] = {
    "int",
    "uint64_t", 
    "float",   // We have to do some wizardry to make these work
    "double", 
};

typedef enum {
  INT_TYPE = 0,
  UINT64_TYPE,
  FLOAT_TYPE,
  DOUBLE_TYPE,
} Types;

char *build_stack_read_ast(StringBuilder *sb, ArgumentKind arg_kind, int pass_through);
char *build_stack_write_ast(StringBuilder *sb, ArgumentKind arg1_kind, ArgumentKind arg2_kind, int pass_through);
char *build_if_test_ast(StringBuilder *sb, ArgumentKind arg_kind, int pass_through);
char *build_arith_ast(StringBuilder *sb, OpCode op, Types rt, ArgumentKind arg_kind_1, ArgumentKind arg_kind_2, int pass_through);

typedef struct {
  char* name;
  uint8_t opcode;
  uint8_t num_64_holes;
  uint8_t num_32_holes;
  int pass_through_count;
  int arg1_kind;
  int arg2_kind;
} PreStencil;

PreStencil *pre_stencils = NULL;

char *arith_ops[] = {
    "+",
    "-",
    "*",
    "/",
    "||",
    "&&",
    "<",
    "<=",
    ">",
    ">=",
    "!",
};

char *arith_names[] = {
    "add", "sub",
    "mul", "div",
    "or", "and",
    "lt", "le",
    "gt", "ge",
    "neq",
};

int main() {
  StringBuilder *function_definitions = new_builder(1024);

  // Generate Conditionals
  for_to(arg_kind, ARG_COUNT) {
    if (arg_kind == LIT_ARG)
      continue;
    for_to(pass_through, 5) {
      small_ord = 0;
      big_ord = 0;
      char *name = build_if_test_ast(function_definitions, arg_kind, pass_through);

      PreStencil pre = {
        .name = name,
        .opcode = IF_OP,
        .num_64_holes = big_ord,
        .num_32_holes = small_ord,
        .pass_through_count = pass_through,
        .arg1_kind = arg_kind,
        .arg2_kind = ARG_NONE,
      };

      arrput(pre_stencils, pre);
    }
  }

  // Generate stack read
  for_to(arg1_kind, ARG_COUNT) {
    for_to(arg2_kind, ARG_COUNT) {
      for_to(pass_through, 5) {
	small_ord = 0;
        big_ord = 0;

        char* name = build_stack_write_ast(function_definitions, arg1_kind, arg2_kind, pass_through);

      PreStencil pre = {
	.name = name,
	.opcode = SWRITE_OP,
	.num_64_holes = big_ord,
	.num_32_holes = small_ord,
	.pass_through_count = pass_through,
	.arg1_kind = arg1_kind,
	.arg2_kind = arg2_kind,
      };

      arrput(pre_stencils, pre);
      }
    }
  }

  // Generate stack write
  for_to(arg_kind, ARG_COUNT) {
    for_to(pass_through, 5) {
      big_ord = 0;
      small_ord = 0;

      char* name = build_stack_read_ast(function_definitions, arg_kind, pass_through);

      PreStencil pre = {
	.name = name,
	.opcode = SREAD_OP,
	.num_64_holes = big_ord,
	.num_32_holes = small_ord,
	.pass_through_count = pass_through,
	.arg1_kind = arg_kind,
	.arg2_kind = ARG_NONE,
      };

      arrput(pre_stencils, pre);
    }
  }

  // Generate Arithmetic ops:
  for_to(opcode, OP_END) {
    for_to(return_type, 1) {
      for_to(arg1_kind, ARG_COUNT) {
        for_to(arg2_kind, ARG_COUNT) {
          // Skip two literals (we constant fold :) )
          if (arg1_kind == LIT_ARG && arg2_kind == LIT_ARG)
            continue;

	  // These fail because of multiplication optimization
          if (opcode == DIV_OP && arg2_kind == LIT_ARG)
            continue;

	  // @NOTE - these fail because of constant folding
          if ((opcode == OR_OP) && (arg1_kind == LIT_ARG))
            continue;
          if ((opcode == OR_OP) && (arg2_kind == LIT_ARG))
            continue;
          if (opcode == AND_OP && arg1_kind == LIT_ARG)
            continue;
          if (opcode == AND_OP && arg2_kind == LIT_ARG)
            continue;

	  for_to(pass_through, 5 /*1 up to including 4*/) {
            small_ord = 0;
	    big_ord = 0;

            char *name = build_arith_ast(function_definitions, opcode, return_type, arg1_kind, arg2_kind, pass_through);

            PreStencil pre = {
              .name = name,
              .opcode = opcode,
              .num_64_holes = big_ord,
              .num_32_holes = small_ord,
              .pass_through_count = pass_through,
	      .arg1_kind = arg1_kind,
	      .arg2_kind = arg2_kind,
	    };
	    
            arrput(pre_stencils, pre);
          }
	}
      }
    }
  }


  // Build output
  StringBuilder* sb = new_builder(1024);
  fori(arrlen(pre_stencils)) { printf("%s\n", pre_stencils[i].name); }

  // generate our generation file...
  add_to(sb, "#include \"stddef.h\"\n");
  add_to(sb, "#include \"stdint.h\"\n");
  add_to(sb, "#include \"../stencil.h\"\n");
  add_to(sb, "\n");

  add_to(sb, to_string(function_definitions));

  int num_stencils = arrlen(pre_stencils);
  
  // Build a list of stencils to use for cutting later
  StringBuilder *fun_list = new_builder(128);
  add_to(fun_list, "// Here we construct a list of our generated stencils that we\n");
  add_to(fun_list, "// will operate on later\n");
  add_to(fun_list, "int num_stencils = %d;\n", num_stencils);
  add_to(fun_list, "StencilData stencils[%d];\n", num_stencils);
  add_to(sb, "\n");
  
  add_to(fun_list, "void build_stencils() {\n", num_stencils);
  fori(num_stencils) {
    PreStencil pre = pre_stencils[i];

    add_to(fun_list, "stencils[%d] = (StencilData){ \n\t.name = \"%s\",\n\t.code = (uint8_t*)%s,\n\t.stencil = {\n\t\t.opcode = %d,\n\t\t.code_size = (uint32_t)((uint8_t*)%s_end - (uint8_t*)%s),\n\t\t.num_holes_32 = %d,\n\t\t.num_holes_64 = %d,\n\t\t.pass_through_count = %d,\n\t\t.arg1_kind = %d,\n\t\t.arg2_kind = %d}};\n",
	   i, pre.name, pre.name, pre.opcode, pre.name, pre.name, pre.num_32_holes, pre.num_64_holes, pre.pass_through_count, pre.arg1_kind, pre.arg2_kind);
  }
  add_to(fun_list, "};\n");

  add_to(sb, to_string(fun_list));

  FILE *f = fopen("../generated/stencils.h", "wb");
  fwrite(to_string(sb), 1, sb->index, f);
  
}

// Forward declare some helper functions
void get_names(char* name, ArgumentKind k1, ArgumentKind k2, int pass_through, char** name_out, char** name_end_out);
void add_pass_through_return_args(Arguments *return_args, int count);
Var pass_through_var(int n, Types rt);
char *pass_through_name(int n);
Parameters *default_params(int pass_through);
Arguments *default_return_args(int pass_through);
Expression *make_arg(Types rt, ArgumentKind kind, char *name);
char *get_type(Types rt);
Expression *make_lit(char *sym);
Expression *make_var(Types rt, char* sym);


char *build_if_test_ast(StringBuilder *sb, ArgumentKind arg_kind, int pass_through) {
  char *fun_name, *fun_name_end;
  get_names("if", arg_kind, 0, pass_through, &fun_name, &fun_name_end);
  
  Expression *cond = make_arg(INT_TYPE, arg_kind, "condition");

  cond = cond ? cond : i("condition");

  /// PARAMS
  // Also, make sure we put those parameters there
  Parameters *params = default_params(pass_through);
  if (arg_kind == REG_ARG) arrput(params->entries, var("condition", type("int")));

  
  // 1. Pass the stack
  Arguments *if_args =
    args(identifier("void"), identifier(big_sentinels[big_ord++]),
	 identifier("uintptr_t"), identifier("stack"));

  // 1. Pass the stack
  Arguments *then_args =
    args(identifier("void"), identifier(big_sentinels[big_ord++]),
	 identifier("uintptr_t"), identifier("stack"));
  
  FuncDecl *if_test_ast =
      func_decl(type("void"), fun_name,
                params,
                block(if_test(cond,
			      block(call("pointer_call", if_args)),
			      block(call("pointer_call", then_args)))));

  FuncDecl *if_test_end_ast = func_decl(type("void"), fun_name_end, params, 0);
  print_func_decl(sb, if_test_ast);
  print_func_decl(sb, if_test_end_ast);
  return fun_name;
}


char *build_arith_ast(StringBuilder *sb, OpCode op, Types rt, ArgumentKind arg1_kind, ArgumentKind arg2_kind, int pass_through) {

  char* op_name = arith_names[op];

  char *fun_name, *fun_name_end;
  get_names(op_name, arg1_kind, arg2_kind, pass_through, &fun_name, &fun_name_end);
  
  //printf("%s\n", fun_name);
  char* return_type = get_type(rt);
  Expression *arg_a = make_arg(rt, arg1_kind, "a");
  Expression *arg_b = make_arg(rt, arg2_kind, "b");

  // We want to leave the 'let' statements declaring the variables empty in the
  // case that they are passed through registers.
  Statement* let_a = let("_a", type(return_type), arg_a);
  Statement* let_b = let("_b", type(return_type), arg_b);

  /// PARAMS
  Parameters* params = default_params(pass_through);
  if (arg1_kind == REG_ARG) arrput(params->entries, var("a", type("int")));
  if (arg2_kind == REG_ARG) arrput(params->entries, var("b", type("int")));

  /// RETURN ARGS
  Arguments* return_args = default_return_args(pass_through);
  arrput(return_args->entries, identifier(return_type));
  arrput(return_args->entries, identifier("result"));


  // Construct the full function ast
  FuncDecl *arith_ast = func_decl(
      type("void"), fun_name, params,
      block(let_a, let_b,
            let("result", type(return_type),
                call_e(op_name, args(identifier("_a"), identifier("_b")))),
            call("pointer_call", return_args)));


  // Construt the "end" ast
  // Construct the full function ast
  FuncDecl *arith_end_ast = func_decl(type("void"), fun_name_end, params, NULL);

  print_func_decl(sb, arith_ast);
  print_func_decl(sb, arith_end_ast);

  return fun_name;
}

char *build_stack_write_ast(StringBuilder *sb, ArgumentKind arg1_kind, ArgumentKind arg2_kind, int pass_through) {

  char *fun_name, *fun_name_end;
  get_names("stack_write", arg1_kind, arg2_kind, pass_through, &fun_name, &fun_name_end);

  // Parameters
  Parameters* params = default_params(pass_through);
  if (arg1_kind == REG_ARG) arrput(params->entries, var("index", type("int")));
  if (arg2_kind == REG_ARG) arrput(params->entries, var("value", type("int")));

  // Return args
  Arguments* return_args = default_return_args(pass_through);

  FuncDecl *stack_write_ast = func_decl(type("void"), fun_name, params,
                           block(call("array_write",
                                      args(i("stack"),
					   make_arg(INT_TYPE, arg1_kind, "index"),
					   make_arg(INT_TYPE, arg2_kind, "value"))),
                      call("pointer_call", return_args)));

  FuncDecl *stack_write_end_ast = func_decl(type("void"), fun_name_end, params, NULL);
  print_func_decl(sb, stack_write_ast);
  print_func_decl(sb, stack_write_end_ast);

  return fun_name;
}

char *build_stack_read_ast(StringBuilder *sb, ArgumentKind arg_kind, int pass_through) {

  char *fun_name, *fun_name_end;
  get_names("stack_read", arg_kind, 0, pass_through, &fun_name, &fun_name_end);

  /// PARAMS
  Parameters* params = default_params(pass_through);
  if (arg_kind == REG_ARG) arrput(params->entries, var("index", type("int")));

  Expression* read_arg = make_arg(INT_TYPE, arg_kind, "index");
  
  // Return args
  Arguments* return_args = default_return_args(pass_through);
  arrput(return_args->entries, identifier("int"));
  arrput(return_args->entries, identifier("result"));

  FuncDecl *stack_read_ast = func_decl(
      type("void"), fun_name, params,
      block(let("result", type("int"),
                call_e("array_read", args(i("int"), i("stack"), read_arg))),
            call("pointer_call",
                 return_args)));

  FuncDecl *stack_read_end_ast = func_decl(type("void"), fun_name_end, params, NULL);
  print_func_decl(sb, stack_read_ast);
  print_func_decl(sb, stack_read_end_ast);

  return fun_name;
}




// AST builder helpers


void add_pass_through_params(Parameters *params, int count) {
    for (int pos = 0; pos < count; ++pos) {
        arrput(params->entries,
	       pass_through_var(pos, (UINT64_TYPE)));
    }
}

Parameters *default_params(int pass_through) {
  // We always get the stack
  Parameters *params = params(var("stack", type("uintptr_t")));

  // Additionally, we might get some pass through parameters
  add_pass_through_params(params, pass_through);

  return params;
}

Arguments *default_return_args(int pass_through) {
  // We always want to pass the stack through to the next function
  Arguments *return_args =
    args(identifier("void"), identifier(big_sentinels[big_ord++]),
	 identifier("uintptr_t"), identifier("stack"));

  // Additionally, always pass the 'pass through' arguments onwards.
  add_pass_through_return_args(return_args, pass_through);

  return return_args;
}

char *get_type(Types rt) {
  return return_types[rt];
}

Expression *make_lit(char *sym) { return identifier(sym); }

Expression *make_var(Types rt, char* sym) {
  return call_e("array_read", args(identifier(get_type(rt)), identifier("stack"), identifier(sym)));
}

Expression *make_arg(Types rt, ArgumentKind kind, char *name) {
  assert(small_ord <= 4);
  switch (kind) {
  case REG_ARG: return identifier(name); // or however a register arg is represented
  case LIT_ARG: return make_lit(small_sentinels[small_ord++]);
  case VAR_ARG: return make_var(rt, small_sentinels[small_ord++]);
  default: assert(0); return 0;
  }
}


char* pass_through_name(int n) {
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "arg%d", n);
  return strdup(buffer);
}

Var pass_through_var(int n, Types rt) {
  Var v;
  v.name = pass_through_name(n);
  v.type = type(get_type(rt));
  return v;
}

void add_pass_through_return_args(Arguments *return_args, int count) {
  for (int pos = 0; pos < count; pos++) {
    arrput(return_args->entries, i(get_type(UINT64_TYPE)));
    arrput(return_args->entries, i(pass_through_name(pos)));
  }
}

void get_names(char* name, ArgumentKind k1, ArgumentKind k2, int pass_through, char** name_out, char** name_end_out) {
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "%s_%d_%d_%d", name, k1, k2,
           pass_through);
  *name_out = strdup(buffer);

  snprintf(buffer, sizeof(buffer), "%s_%d_%d_%d_end", name, k1, k2,
           pass_through);
  *name_end_out = strdup(buffer);
}

