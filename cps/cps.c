#include "stdio.h"

#define STB_DS_ASSERT assert
#define STB_DS_IMPLEMENTATION
#include "../include/stb_ds.h"

#include "../ast/ast.h"
#include "../ast/build.h"

#include "cps.h"

// Hash table from variable name → index
typedef struct {
  const char* key;
  int value;
} VarSlot;

VarSlot *var_slots = NULL;
int next_stack_index = 0;

int stack_index(const char *name) {
  VarSlot *found = hmgetp_null(var_slots, name);
  if (found) {
    return found->value;
  } else {
    int idx = next_stack_index++;
    hmput(var_slots, name, idx);
    return idx;
  }
}

CpsNode *graph_head = 0;

int new_label() {
  static int label_counter = 0;
  return label_counter++;
}

CpsVar new_tmp() {
  CpsVar v;
  static int temp_var_counter = 0;
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "_tmp%d", temp_var_counter++);
  v.name = strdup(buffer);
  v.index = stack_index(v.name);
  return v;
}

CpsVar from_name(const char *name) {
  CpsVar v;
  v.name = name;
  v.index = stack_index(name);
  return v;
}


void add_cps_node(CpsNode *node) {
  node->next = graph_head;
  graph_head = node;
}

CpsNode* new_node(CpsKind kind) {
  CpsNode* n =  malloc(sizeof(CpsNode));
  n->label = new_label();
  n->kind = kind;
  add_cps_node(n);
  return n;
}

CpsNode *emit_let(CpsVar var, CpsLiteral value, int cont) {
  CpsNode *n = new_node(CPS_LET);
  n->let_node = (CpsLet){.var = var, .value=value, .cont=cont};
  return n;
}

CpsNode *emit_if(CpsVar cond, int then_label, int else_label) {
  CpsNode *n = new_node(CPS_IF);
  n->if_node = (CpsIf){cond, then_label, else_label};
  return n;
}

CpsNode *emit_return(const char *value) {
  CpsNode *n = new_node(CPS_RETURN);
  n->return_node.value = value;
  return n;
}

CpsNode *emit_call(char *func, CpsLiteral* args, int arg_count, int cont) {
  CpsNode *n = new_node(CPS_CALL);
  n->call_node = (CpsCall){
    .func = func,
    .args = args,
    .arg_count = arg_count,
    .cont = cont
  };
  return n;
}

int transform_statement(Statement *stmt, int cont_label);
int transform_block(Block *block, int cont_label) {
  for (int i = block->count - 1; i >= 0; --i) {
    cont_label = transform_statement(block->statements[i], cont_label);
  }
  return cont_label;
}

CpsLiteral to_cps(Expression *e) {
  CpsLiteral cl;

  if (e->lit.kind == integer_lit) {
    cl.kind = CPS_LITERAL_INTEGER;
    cl.integer = e->lit.integer;
  }

  if (e->lit.kind == identifier_lit) {
    cl.kind = CPS_LITERAL_VAR;
    cl.var = from_name(e->lit.identifier);
  }
  return cl;
}

CpsNode *transform_expr(Expression *expr, CpsVar target_var, int cont_label);
CpsLiteral *transform_args(Arguments *arguments, int cont_label,
                           int *out_label) {
  CpsLiteral *args = malloc(sizeof(CpsLiteral) * arguments->count);
  int label = cont_label;

  for (int i = arguments->count - 1; i >= 0; --i) {
    Expression *arg = arguments->entries[i];

    if (arg->kind == lit_expr && arg->lit.kind == integer_lit) {
      args[i].kind = CPS_LITERAL_INTEGER;
      args[i].integer = arg->lit.integer;
    } else if (arg->kind == lit_expr && arg->lit.kind == identifier_lit) {
      CpsVar var;
      args[i].kind = CPS_LITERAL_VAR;
      args[i].var = from_name(arg->lit.identifier);
    } else {
      CpsVar tmp = new_tmp();
      label = transform_expr(arg, tmp, label)->label;
      args[i].kind = CPS_LITERAL_VAR;
      args[i].var = tmp;
    }
  }

  *out_label = label;
  return args;
}

CpsNode *transform_expr(Expression *expr, CpsVar target_var, int cont_label) {
  switch (expr->kind) {
  case lit_expr: {
    Literal *lit = &expr->lit;
    return emit_let(target_var,to_cps(expr), cont_label);
  }

  case call_expr: {
    int new_label;
    Call *call = &expr->call;
    CpsLiteral *args = transform_args(&call->args, cont_label, &new_label);
    CpsNode *n = emit_call(call->name, args, call->args.count, new_label);
    return n;
  }

  default:
    return emit_let(target_var, to_cps(new_identifier("lol")), cont_label);
  }
}

int transform_statement(Statement *stmt, int cont_label) {
  switch (stmt->kind) {
  case assign_statement: {
    Expression *e = stmt->assign.expr;
    CpsNode *n = emit_let(from_name(stmt->assign.name), to_cps(e), cont_label);
    return n->label;
  }

  case call_statement: {
    int new_label;
    Call *call = &stmt->call;
    CpsLiteral *args = transform_args(&call->args, cont_label, &new_label);
    CpsNode *n = emit_call(call->name, args, call->args.count, new_label);
    return n->label;
  }

  case if_statement: {
    CpsVar cond_tmp = new_tmp();
    int then_label = transform_block(stmt->if_block.then_branch, cont_label);
    int else_label = transform_block(stmt->if_block.else_branch, cont_label);
    CpsNode *n = emit_if(cond_tmp, then_label, else_label);
    CpsNode *cond_eval = transform_expr(stmt->if_block.condition, cond_tmp, n->label);
    return cond_eval->label;
  }   
  default: return cont_label;
  }
}


void print_literal(CpsLiteral *lit) {
    if (!lit) {
        printf("<?>");
        return;
    }

    switch (lit->kind) {
        case CPS_LITERAL_INTEGER:
            printf("%d", lit->integer);
            break;
        case CPS_LITERAL_VAR:
	  printf("%s{idx:%d}", lit->var.name, lit->var.index);
            break;
        default:
            printf("<?>");
    }
}

void print_cps_graph(CpsNode *head) {
    for (CpsNode *n = head; n != NULL; n = n->next) {
        printf("L%d: ", n->label);

        switch (n->kind) {
            case CPS_LET: {
                CpsLet *let = &n->let_node;
                printf("let %s{idx:%d} = ", let->var.name, let->var.index);
                print_literal(&let->value);
                printf(" → L%d\n", let->cont);
                break;
            }

            case CPS_CALL: {
                CpsCall *call = &n->call_node;
                printf("call %s(", call->func);
                for (int i = 0; i < call->arg_count; ++i) {
                    print_literal(&call->args[i]);
                    if (i < call->arg_count - 1) printf(", ");
                }
                printf(") → L%d\n", call->cont);
                break;
            }

            case CPS_IF: {
                CpsIf *iff = &n->if_node;
                printf("if %s then L%d else L%d\n",
                       iff->cond.name,
                       iff->then_label,
                       iff->else_label);
                break;
            }

            case CPS_RETURN: {
                printf("return %s\n", n->return_node.value);
                break;
            }

            default:
                printf("unknown node type\n");
        }
    }
}

Block *example_ast();

int main() {
  Block *b = example_ast();

  CpsNode *ret = emit_return("unit");
  transform_block(b, ret->label);

  print_cps_graph(graph_head);

}


Block* example_ast() {
  return new_block(new_assign("test", new_integer(3)),
                   new_if(new_call_expr("leq", (Arguments){
			 .count = 2,
			 .entries = {new_integer(1),
				     new_integer(4)}}),
		     new_block(new_call(
					"add", (Arguments){.count = 2,
							   .entries = {new_identifier("test"),
								       new_integer(4)}})),
		     new_block(new_call(
					"add", (Arguments){.count = 2,
							   .entries = {new_identifier("test"),
								       new_integer(7)}}))));
}

