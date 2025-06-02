#include "stdio.h"

#include "../include/stb_ds.h"
#include "../bl.h"

#include "print.h"


void print_literal(CpsLiteral *lit) {
  if (!lit) {
    dprintf("<?>");
    return;
  }

  switch (lit->kind) {
  case CPS_LITERAL_INTEGER:
    dprintf("%d", lit->integer);
    break;
  case CPS_LITERAL_VAR:
    dprintf("%s{idx:%d}", lit->var.name, lit->var.index);
    break;
  default:
    dprintf("<?>");
  }
}

void print_cps_graph(CpsNode *head) {
  for (CpsNode *n = head; n != NULL; n = n->next) {
    dprintf("L%d: ", n->label);

    switch (n->kind) {
    case CPS_LET: {
      CpsLet *let = &n->let_node;
      dprintf("let %s{idx:%d} = ", let->var.name, let->var.index);
      print_literal(&let->value);
      dprintf(" → L%d\n", let->cont);
      break;
    }

    case CPS_CALL: {
      CpsCall *call = &n->call_node;
      dprintf("call %s(", call->func);
      for (int i = 0; i < call->arg_count; ++i) {
	print_literal(&call->args[i]);
	if (i < call->arg_count - 1) dprintf(", ");
      }
      dprintf(") → L%d\n", call->cont);
      break;
    }

    case CPS_IF: {
      CpsIf *iff = &n->if_node;
      dprintf("if %s then L%d else L%d\n",
	     iff->cond.name,
	     iff->then_label,
	     iff->else_label);
      break;
    }

    case CPS_RETURN: {
      dprintf("return %s\n", n->return_node.value);
      break;
    }

    default:
      dprintf("unknown node type\n");
    }
  }
}

