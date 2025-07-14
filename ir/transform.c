
#include <stdio.h>
#include <include/stb_ds.h>

#include <ir/transform.h>

int transform_statement(Statement *stmt, int cont_label);
int transform_block(Block *block, int cont_label);
IrNode *emit_return(const char *value);

IrNode *graph_head = 0;
IrNode *transform_ast(Block* b) {
  IrNode *ret = emit_return("unit");
  transform_block(b, ret->label);
  return graph_head;
}

// Hash table from variable name → index
VarSlot *var_slots = NULL;
int next_stack_index = 0;

int stack_index(const char *name) {
  VarSlot *found = shgetp_null(var_slots, name);
  if (!found) {
    int idx = next_stack_index++;
    shput(var_slots, name, idx);
    return idx;
  }
  return found->value;
}

int new_label() {
  static int label_counter = 0;
  return label_counter++;
}

IrVar new_tmp() {
  IrVar v;
  static int temp_var_counter = 0;
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "_tmp%d", temp_var_counter++);
  v.name = strdup(buffer);
  v.index = stack_index(v.name);
  return v;
}

IrVar from_name(const char *name) {
  IrVar v;
  v.name = name;
  v.index = stack_index(name);
  return v;
}

void add_ir_node(IrNode *node) {
  node->next = graph_head;
  graph_head = node;
}

IrNode* new_node(IrKind kind) {
  IrNode* n =  malloc(sizeof(IrNode));
  n->label = new_label();
  n->kind = kind;
  add_ir_node(n);
  return n;
}

IrNode *emit_let(IrVar var, IrLiteral value, int cont) {
  IrNode *n = new_node(IR_LET);
  n->let_node = (IrLet){.var = var, .value=value, .cont=cont};
  return n;
}

IrNode *emit_if(IrVar cond, int then_label, int else_label) {
  IrNode *n = new_node(IR_IF);
  n->if_node = (IrIf){cond, then_label, else_label};
  return n;
}

IrNode *emit_return(const char *value) {
  IrNode *n = new_node(IR_RETURN);
  n->return_node.value = value;
  return n;
}

IrNode *emit_call(char *func) {
  IrNode *n = new_node(IR_CALL);
  n->call_node = (IrCall){
    .func = func, .args = (IrLiteral*)-1, .arg_count = -1, .has_ret = -1,
    .cont = -1
  };
  return n;
}

IrNode *emit_call_var(char *func, IrLiteral *args, int arg_count, IrVar ret, int cont) {
  IrNode *n = new_node(IR_CALL);
  n->call_node = (IrCall){
    .func = func, .args = args, .arg_count = arg_count, .has_ret = 1, .ret = ret,
    .cont = cont
  };
  return n;
}



int transform_block(Block *block, int cont_label) {
  for (int i = arrlen(block->statements) - 1; i >= 0; --i) {
    cont_label = transform_statement(block->statements[i], cont_label);
  }
  return cont_label;
}

IrLiteral to_ir(Expression *e) {
  IrLiteral cl;

  if (e->lit.kind == integer_lit) {
    cl.kind = IR_LITERAL_INTEGER;
    cl.integer = e->lit.integer;
  }

  if (e->lit.kind == identifier_lit) {
    cl.kind = IR_LITERAL_VAR;
    cl.var = from_name(e->lit.identifier);
  }
  return cl;
}

IrNode *transform_expr(Expression *expr, IrVar target_var, int cont_label);
IrLiteral *transform_args(Arguments *arguments, int cont_label,
                          int *out_label) {
  int count = arrlen(arguments->entries);
  IrLiteral *args = malloc(sizeof(IrLiteral) * arrlen(arguments->entries));
  int label = cont_label;

  for (int i = count-1; i >= 0; --i) {
    Expression *arg = arguments->entries[i];
    IrLiteral *node = &args[i];

    if (arg->kind == lit_expr && arg->lit.kind == integer_lit) {
      // Integer
      node->kind = IR_LITERAL_INTEGER;
      node->integer = arg->lit.integer;

    } else if (arg->kind == lit_expr && arg->lit.kind == identifier_lit) {
      // Literal
      IrVar* var = &node->var;
      node->kind = IR_LITERAL_VAR;
      node->var = from_name(arg->lit.identifier);
    } else {
      // Function call!
      IrVar tmp = new_tmp();
      node->kind = IR_LITERAL_VAR;
      node->var = tmp;
      IrNode *call_node = transform_expr(arg, tmp, label);
      label = call_node->label;
    }
  }

  *out_label = label;
  return args;
}

IrNode *transform_expr(Expression *expr, IrVar target_var, int cont_label) {
  switch (expr->kind) {
  case lit_expr: {
    Literal *lit = &expr->lit;
    return emit_let(target_var, to_ir(expr), cont_label);
  }

  case call_expr: {
    int new_label;
    Call *call = &expr->call;
    // args, arrlen(call->args->entries), 
    IrNode *n =
      emit_call_var(call->name, 0,0, target_var, new_label);
    IrLiteral *args = transform_args(call->args, cont_label, &new_label);
    n->call_node.args = args;
    n->call_node.arg_count = arrlen(call->args->entries);
    n->call_node.cont = new_label;
    printf("emitting %s\n", call->name);
    return n;
  }

  default: break;
  }
}

int transform_statement(Statement *stmt, int cont_label) {
  switch (stmt->kind) {
  case let_statement: {
    Expression *e = stmt->let.expr;
    IrNode *n = emit_let(from_name(stmt->let.name), to_ir(e), cont_label);
    return n->label;
  }

  case call_statement: {
    int new_label;
    Call *call = &stmt->call;
    IrNode *n = emit_call(call->name);
    IrLiteral *args = transform_args(call->args, cont_label, &new_label);
    n->call_node.args = args;
    n->call_node.arg_count = arrlen(call->args->entries);
    n->call_node.cont = new_label;
    printf("emitting %s\n", call->name);
    return n->label;
  }

  case if_statement: {
    IrVar cond_tmp = new_tmp();
    int then_label = transform_block(stmt->if_block.then_branch, cont_label);
    int else_label = transform_block(stmt->if_block.else_branch, cont_label);
    IrNode *n = emit_if(cond_tmp, then_label, else_label);
    IrNode *cond_eval = transform_expr(stmt->if_block.condition, cond_tmp, n->label);
    return cond_eval->label;
  }   
  default: return cont_label;
  }
}
