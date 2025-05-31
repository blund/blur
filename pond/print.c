#include "stdio.h"

#include "../bl.h"

#include "ast.h"


void indent(Parser* p) {
  add_to(p->output,  "%*s", p->indent, " ");
}



void print_pointer_call(Parser* p, PointerCall pc);
void print_if_block(Parser* p, IfBlock ib);
void print_block(Parser* p, Block* b);

void print(Parser *p, char *str) {
  add_to(p->output, str);
}

void print_unit(Parser *p, Unit u) {
  add_to(p->output, "%.*s", u.end-u.start, u.ptr);
}
void print_type(Parser* p, Type t) {
  print_unit(p, t.name);
  if (t.ptr) add_to(p->output, "*");
}

void print_assign(Parser* p, Assign a);

void print_value(Parser* p, Value v) {
  if (v.type == string_type) {
    print_unit(p, v.data);
    return;
  }
  if (v.type == integer_type) {
    print_unit(p, v.data);
    return;
  }
}

void print_call(Parser* p, Call c) {
  print_unit(p, c.name);
  add_to(p->output, "()");
}

void print_if_block(Parser* p, IfBlock ib) {
  add_to(p->output, "if (");
  print_unit(p, ib.condition);
  add_to(p->output, ") {\n");
  p->indent += 2;

  print_block(p, ib.if_block);
  p->indent -= 2;

  indent(p);
  add_to(p->output, "}");

  add_to(p->output, " else {\n");
  p->indent += 2;
  print_block(p, ib.else_block);
  p->indent -= 2;

  indent(p);
  add_to(p->output, "}\n");

}

void print_binop(Parser *p, BinOp *e) {
  print(p, e->lhs);
  add_to(p->output, " %s ", e->op);
  print(p, e->rhs);
}

void print_expr(Parser* p, Expr* e) {
  if (e->kind == expr_value_kind) print_value(p, e->value);
  if (e->kind == expr_call_kind)  print_call(p,  e->call);
  if (e->kind == expr_binop_kind)  print_binop(p,  &e->binop);
}

void print_return_block(Parser *p, Return *r) {
  add_to(p->output, "return ");
  print_expr(p, &r->expr);
  add_to(p->output, ";\n");
}

void print_statement(Parser *p, Statement *s) {
  indent(p);
  if (s->kind == statement_assign_kind) {
    print_assign(p, s->assign);
    add_to(p->output, ";\n");
  }
  if (s->kind == statement_call_kind) {
    print_call(p,   s->call);
    add_to(p->output, ";\n");
  }
  if (s->kind == statement_pointer_call_kind) {
    print_pointer_call(p,   s->pointer_call);
    add_to(p->output, ";\n");
  }
  if (s->kind == statement_if_kind) {
    print_if_block(p, s->if_block);
  }
  if (s->kind == statement_return_kind) {
    print_return_block(p, &s->return_block);
  }
}

void print_assign(Parser* p, Assign a) {
  print_type(p, a.type);
  add_to(p->output, " ");
  print_unit(p, a.name);
  add_to(p->output, " = ");
  print_expr(p, a.expr);
}
void print_block(Parser* p, Block* b) {
  Block* iter = b;
  Block* iter2 = b;

 
  // @TODO unsure about this logic, but it works B)
  for (;;) {
    if (iter->statement)
      print_statement(p, iter->statement);
    if (!iter->next) return;
    iter = iter->next;
  }
}

void print_pointer_call(Parser *p, PointerCall f) {
  add_to(p->output, "((");
  print_type(p, f.return_type);
  add_to(p->output, " (*)(");
  fori(f.parameters.count) {
    print_type(p, f.parameters.entries[i].type);
    if (i < f.parameters.count-1) add_to(p->output, ", ");
  }
  add_to(p->output, "))(");
  print_unit(p, f.operand);
  add_to(p->output, "))(");
  fori(f.parameters.count) {
    print_unit(p, f.parameters.entries[i].name);
    if (i < f.parameters.count-1) add_to(p->output, ", ");
  }
  add_to(p->output, ")");
}

void print_parameters(Parser *p, Parameters a) {
  add_to(p->output, "(");
  fori(a.count) {
    print_type(p, a.entries[i].type);
    add_to(p->output, " ");
    print_unit(p, a.entries[i].name);
    if (i < a.count-1) add_to(p->output, ", ");
  }
  add_to(p->output, ")");
}

void print_func_decl(Parser* p, FuncDecl* f) {
  print_type(p,  f->ret);
  add_to(p->output, " ");
  print_unit(p, f->name);
  print_parameters(p, f->parameters);
  add_to(p->output, " {\n");
  p->indent += 2;
  print_block(p, f->body);
  add_to(p->output, "}\n");
}

