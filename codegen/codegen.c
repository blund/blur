
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
// literal, variable, array

typedef struct {
  Type operand;
  int spill;
  int passthrough;
} MetaVars;

#define small_hole_1 0x3e7a91bc  
#define small_hole_2 0xd48cf2a0  
#define small_hole_3 0x9b5d6ee3  
#define small_hole_4 0x71c3ab4f

#define big_hole_1 0xe2d9c7b1843a56f0  
#define big_hole_2 0x4ba1fedc02347a9d  
#define big_hole_3 0xac5f13e7902dcb88  
#define big_hole_4 0x1dce8b4793fa065e


int small_ord = 0;
char *small_sentinels[] = {
  STR(small_hole_1),
  STR(small_hole_2),
  STR(small_hole_3),
  STR(small_hole_4),
};

int big_ord = 0;
char *big_sentinels[] = {
  STR(big_hole_1),
  STR(big_hole_2),
  STR(big_hole_3),
  STR(big_hole_4),
};

char *conditionals[] = {
    "neq", "eq", "lt", "le", "gt", "ge",
};

typedef enum {
  NEQ_KIND = 0,
  EQ_KIND,
  LT_KIND,
  LE_KIND,
  GT_KIND,
  GE_KIND,
  COND_END,
} CondType;

char *get_cond(CondType ct) {
  return conditionals[ct];
}

char *return_types[] = {
    "int",
    "float", 
    "uint64_t", 
    "double", 
};
int num_return_types = sizeof(return_types)/sizeof(return_types[0]);

typedef enum {
  INT_TYPE = 0,
  FLOAT_TYPE,
  UINT64_TYPE,
  DOUBLE_TYPE,
} Types;

char *get_type(Types rt) {
  return return_types[rt];
}

typedef enum {
  REG_KIND = 0,
  LIT_KIND,
  VAR_KIND,
  ARG_KIND_COUNT
} ArgumentKind;

Expression *make_lit(char *sym) { return identifier(sym); }

Expression *make_var(Types rt, char* sym) {
  return call_e("array_read", args(identifier(get_type(rt)), identifier("stack"), identifier(sym)));
}


Expression *make_arg(Types rt, ArgumentKind kind) {
  switch (kind) {
  case REG_KIND: return NULL; // or however a register arg is represented
  case LIT_KIND: return make_lit(small_sentinels[small_ord++]);
  case VAR_KIND: return make_var(rt, small_sentinels[small_ord++]);
  default: assert(0); return 0;
  }
}

FuncDecl *build_if_ast(char* return_type, char* return_sentinel, char* condition, Expression* arg1, Expression* arg2);

FuncDecl *build_add_ast(Types rt, Expression *arg1, Expression *arg2, int pass_through, int pass_through_types);
// @TODO
/*
Expression *make_array(char* sym1, char* sym2) {
  return call_e("array_read", args(identifier(sym1), identifier(sym2)));
}
*/

int main() {
  Expression* arg1;
  Expression *arg2;


  // 2 return types x 3 arg kinds x 3 arg kinds * 4 pass through = 72
  for_in(return_type, return_types) {
    for_to(arg_kind_1, ARG_KIND_COUNT) {
      for_to(arg_kind_2, ARG_KIND_COUNT) {
        for_to(pass_through, 5) {
	  for_to(pass_through_types, 4) {
	    small_ord = 0, big_ord = 0;
	    arg1 = make_arg(return_type, arg_kind_1);
	    arg2 = make_arg(return_type, arg_kind_2);
	    FuncDecl *add = build_add_ast(return_type, arg1, arg2, pass_through, pass_through_types);

	    StringBuilder *sb = new_builder(1024);
	    print_func_decl(sb, add);
	    printf("%s\n", to_string(sb));
	  }
	}
      }
    }
  }

  /*
  StringBuilder *sb = new_builder(1024);
  fori(2) { // expr 1 kind
    forj(2) { // expr 2 kind
      small_ord = 0;
      big_ord = 0;
      if (i == LIT_KIND) arg1 = make_lit(small_sentinels[small_ord++]);
      if (i == VAR_KIND) arg1 = make_var(return_type,
					 small_sentinels[small_ord++]); if (j == LIT_KIND) arg2 =
											     make_lit(small_sentinels[small_ord++]); if (j == VAR_KIND) arg2 =
																			  make_var(return_type, small_sentinels[small_ord++]);

      for (int cond = 0; cond < COND_END; cond++) {
        //FuncDecl *if_test = build_if_ast(get_return(INT_TYPE),
	big_sentinels[big_ord++],
	  //get_cond(cond), arg1, arg2);

	  //reset(sb);
	  //	print_func_decl(sb, if_test);
	  //	printf("%s\n", to_string(sb));
	  }
    }
  }
  */
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

FuncDecl *build_add_ast(Types rt, Expression *arg_a, Expression *arg_b, int pass_through, int pass_through_types) {

  char* return_type = get_type(rt);

  // We want to leave the 'let' statements declaring the variables empty in the
  // case that they are passed through registers.
  Statement* let_a = arg_a ? let("a", type(return_type), arg_a) : 0;
  Statement* let_b = arg_b ? let("b", type(return_type), arg_b) : 0;

  // Also, make sure we put those parameters there
  Parameters *params = params(var("stack", type("uintptr_t")));

  // Pass-through arguments. We do a binary count, where 0 is uint64 and 1 is
  // double.
  // Pos is the amount of pass through arguments
  for (int pos = 0; pos < pass_through; ++pos) {
    if ((pass_through_types >> pos) & 1) {
      arrput(params->entries, pass_through_var(pos, UINT64_TYPE));
    } else {
      arrput(params->entries, pass_through_var(pos, DOUBLE_TYPE));
    }
  }

  // Append arguments a or b as registers at the end
  if (arg_a == 0) arrput(params->entries, var("a", type("int")));
  if (arg_b == 0) arrput(params->entries, var("b", type("int")));


  // Construct contuniation call
  // This consists of :
  // 1. stack
  // 2. all pass-through arguments
  // 3. result spill

  // 1. Pass the stack
  Arguments *return_args =
    args(identifier("void"), identifier(big_sentinels[big_ord]),
	 identifier("uintptr_t"), identifier("stack"));

  // 2. Construct pass-through arguments
  for (int pos = 0; pos < pass_through; ++pos) {
    if ((pass_through_types >> pos) & 1) {
      arrput(return_args->entries, identifier(get_type(UINT64_TYPE)));
    } else {
      arrput(return_args->entries, identifier(get_type(DOUBLE_TYPE)));
    }
    arrput(return_args->entries, identifier(pass_through_name(pos)));
  }

  // 3. Result spill
  arrput(return_args->entries, identifier(return_type));
  arrput(return_args->entries, identifier("result"));


  // Construct the full function ast
  return func_decl(type("void"), "add_const", params,
		   block(let_a, let_b,
			 let("result", type(return_type),
			     call_e("add", args(identifier("a"), identifier("b")))),
			 call("pointer_call", return_args)));
}


/*
FuncDecl *build_if_ast(char* return_type, char* return_sentinel, char*
condition, Expression* arg1, Expression* arg2) { return func_decl( type("void"),
"if_test", params(var("stack", type("uintptr_t")), var("x", type("int"))),
      block(
            let("a", type(return_type), arg1),
            let("b", type(return_type), arg2),
            if_test(

                    call_e(condition, args(identifier("a"), identifier("b"))),
                    block(call("pointer_call",
                               args( i("void"), i(STR(big_hole_1)),
                                     i("uintptr_t"), i("stack")))),
                    block(call("pointer_call",
                               args( i("void"), i(STR(big_hole_2)),
                                     i("uintptr_t"), i("stack")))))));
}
*/
