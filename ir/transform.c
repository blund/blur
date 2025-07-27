
#include <stdio.h>
#include <include/stb_ds.h>

#include <bl.h>
#include <ir/transform.h>

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

IrNode *new_node() {
  static int label = 0;
  IrNode *n = malloc(sizeof(IrNode));
  n->next = 0;
  n->label = label;
  label++;
  return n;
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

IrVar var_from_name(const char *name) {
  IrVar v;
  v.name = name;
  v.index = stack_index(name);
  return v;
}

IrLiteral expr_to_literal(Expression *e) {
  IrLiteral cl;

  if (e->lit.kind == integer_lit) {
    cl.kind = IR_LITERAL_INTEGER;
    cl.integer = e->lit.integer;
  }

  if (e->lit.kind == identifier_lit) {
    cl.kind = IR_LITERAL_VAR;
    cl.var = var_from_name(e->lit.identifier);
  }

  assert(e->kind != call_expr);

  return cl;
}

void set_continuation(IrNode *from, IrNode *to) {
  if (!from || !to) return;
  switch (from->kind) {
    case IR_LET:  from->let_node.cont  = to->label; break;
    case IR_CALL: from->call_node.cont = to->label; break;
    default: break;
  }
}

IrNode *transform_call(Call *call, IrVar* ret) {
  // This node represents the current call we're working on
  IrCall call_node_;
  IrCall* call_node = &call_node_;

  call_node->func = call->name;
  if (ret) {
    call_node->has_ret = 1;
    call_node->ret = *ret;
  } else {
    call_node->has_ret = 0;
  }

  Expression **args = call->args->entries;
  size_t len = arrlen(args);
  call_node->args = malloc(sizeof(IrLiteral) * len);
  call_node->arg_count = len;

  // Start of the IR linked list
  IrNode *head = NULL;
  IrNode **tail = &head;

  for (int j = 0; j < len; j++) {
    if (args[j]->kind == lit_expr) {
      call_node->args[j] = expr_to_literal(args[j]);
    } else if (args[j]->kind == call_expr) {
      Call *c = &args[j]->call;
      IrVar arg = new_tmp();
      IrNode *subtree = transform_call(c, &arg);
      // Append the subtree to the current list
      while (subtree) {
        *tail = subtree;
        tail = &subtree->next;
        subtree = subtree->next;
      }

      // Assume result of the subcall is used as arg
      call_node->args[j].kind = IR_LITERAL_VAR;
      call_node->args[j].var = arg; 
    }
  }

  IrNode *curr = new_node();
  curr->kind = IR_CALL;
  curr->call_node = call_node_;

  // Append to chain
  *tail = curr;
  curr->next = NULL;
  return head;
}

IrNode *transform_block(Block *b) {
  IrNode *head = NULL;     // First node in the block
  IrNode **tail = &head;   // Pointer to the last node’s ->next

  for (int i = 0; i < arrlen(b->statements); i++) {
    Statement *statement = b->statements[i];

    IrNode *first = NULL;
    IrNode *last = NULL;

    switch (statement->kind) {
      case let_statement: {
        IrNode *curr = new_node();
        curr->kind = IR_LET;
        IrLet *let = &curr->let_node;
        let->var = var_from_name(statement->let.name);
        assert(statement->let.expr->kind != call_expr);
        let->value = expr_to_literal(statement->let.expr);
        first = curr;
        last = curr;
      } break;

      case call_statement: {
        Call *call = &statement->call;
        first = transform_call(call, NULL);

        // Advance to last node in call chain
        last = first;
        while (last->next)
          last = last->next;
      } break;

      case if_statement: {
        If *if_stmt = &statement->if_block;

        // Evaluate condition
        IrNode *cond = new_node();
        cond->kind = IR_LET;
        IrLet *let = &cond->let_node;
        let->value = expr_to_literal(if_stmt->condition);
        let->var = new_tmp();

        // Create IF node
        IrNode *if_node = new_node();
        if_node->kind = IR_IF;
        if_node->if_node.cond = let->var;

        // Transform branches
        IrNode *then_chain = transform_block(if_stmt->then_branch);
        IrNode *else_chain = transform_block(if_stmt->else_branch);

        // Set labels for branches
        if_node->if_node.then_label = then_chain->label;
        if_node->if_node.else_label = else_chain->label;

        // Stitch all together: cond → if_node → then_chain → else_chain
        cond->next = if_node;
        if_node->next = then_chain;

        IrNode *t = then_chain;
        while (t->next)
          t = t->next;
        t->next = else_chain;

        IrNode *e = else_chain;
        while (e->next) {
          e = e->next;
	}

        first = cond;
        last = e;
      } break;
    }

    if (first) {
      *tail = first;

      // Assign continuation for all nodes in the chain
      IrNode *curr = first;
      while (curr && curr->next) {
	set_continuation(curr, curr->next);
	curr = curr->next;
      }

      // Update tail to the next field of the last node
      tail = &last->next;
    }
    
     }

  return head;
}
IrNode *transform_ast(Block *b) {
  int cont_label = 0;
  IrNode *linearized = transform_block(b);

  // @HACK - fix the first continuation
  set_continuation(linearized, linearized->next);
  
  IrNode* ret = new_node();

  IrNode *n = linearized;
  while (n && n->next)
    n = n->next;

  set_continuation(n, ret);
  n->next = ret;
  ret->kind = IR_RETURN;

  return linearized;
}
