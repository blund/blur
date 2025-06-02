#include <stdio.h>

#include <include/stb_ds.h>
#include <bl.h>

#include <ir/print.h>


void print_literal(IrLiteral *lit) {
  if (!lit) {
    dprintf("<?>");
    return;
  }

  switch (lit->kind) {
  case IR_LITERAL_INTEGER:
    dprintf("%d", lit->integer);
    break;
  case IR_LITERAL_VAR:
    dprintf("%s{idx:%d}", lit->var.name, lit->var.index);
    break;
  default:
    dprintf("<?>");
  }
}


void print_ir(IrNode *head) {
  for (IrNode *n = head; n != NULL; n = n->next) {
    dprintf("L%d: ", n->label);

    switch (n->kind) {
    case IR_LET: {
      IrLet *let = &n->let_node;
      dprintf("let %s{idx:%d} = ", let->var.name, let->var.index);
      print_literal(&let->value);
      dprintf(" → L%d\n", let->cont);
      break;
    }

    case IR_CALL: {
      IrCall *call = &n->call_node;
      dprintf("call %s(", call->func);
      for (int i = 0; i < call->arg_count; ++i) {
	print_literal(&call->args[i]);
	if (i < call->arg_count - 1) dprintf(", ");
      }
      dprintf(") → L%d\n", call->cont);
      break;
    }

    case IR_IF: {
      IrIf *iff = &n->if_node;
      dprintf("if %s then L%d else L%d\n",
	     iff->cond.name,
	     iff->then_label,
	     iff->else_label);
      break;
    }

    case IR_RETURN: {
      dprintf("return %s\n", n->return_node.value);
      break;
    }

    default:
      dprintf("unknown node type\n");
    }
  }
}

