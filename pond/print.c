#include "stdio.h"

#include "../bl.h"

#include "ast.h"


void indent(Parser* p) {
  printf("%*s", p->indent, " ");
}

void print_pointer_call(Parser* p, PointerCall pc);
void print_if_block(Parser* p, IfBlock ib);
void print_block(Parser* p, Block* b);

void print_unit(Parser *p, Unit u) {
  printf("%.*s", u.end-u.start, u.ptr);
}
void print_type(Parser* p, Type t) {
  print_unit(p, t.name);
  if (t.ptr) printf("*");
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
  printf("()");
}

void print_if_block(Parser* p, IfBlock ib) {
  printf("if (");
  print_unit(p, ib.condition);
  printf(") {\n");
  p->indent += 2;

  print_block(p, ib.body);
  p->indent -= 2;

  indent(p);
  printf("}\n");
}

void print_expr(Parser* p, Expr* e) {
  if (e->kind == expr_value_kind) print_value(p, e->value);
  if (e->kind == expr_call_kind)  print_call(p,  e->call);
}

void print_statement(Parser *p, Statement *s) {
  indent(p);
  if (s->kind == statement_assign_kind) {
    print_assign(p, s->assign);
    printf(";\n");
  }
  if (s->kind == statement_call_kind) {
    print_call(p,   s->call);
    printf(";\n");
  }
  if (s->kind == statement_pointer_call_kind) {
    print_pointer_call(p,   s->pointer_call);
    printf(";\n");
  }
  if (s->kind == statement_if_kind) {
    print_if_block(p, s->if_block);
  }
}

void print_assign(Parser* p, Assign a) {
  print_type(p, a.type);
  printf(" ");
  print_unit(p, a.name);
  printf(" = ");
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
  printf("((");
  print_type(p, f.return_type);
  printf(" (*))(");
  fori(f.parameters.arg_count) {
    print_type(p, f.parameters.types[i]);
    if (i < f.parameters.arg_count-1) printf(", ");
  }
  printf("))(");
  print_unit(p, f.operand);
  printf("))(");
  fori(f.parameters.arg_count) {
    print_unit(p, f.parameters.names[i]);
    if (i < f.parameters.arg_count-1) printf(", ");
  }
  printf(")");
}

void print_parameters(Parser *p, Parameters a) {
  printf("(");
  fori(a.arg_count) {
    print_type(p, a.types[i]);
    printf(" ");
    print_unit(p, a.names[i]);
    if (i < a.arg_count-1) printf(", ");
  }
  printf(")");
}

void print_func_decl(Parser* p, FuncDecl* f) {
  print_type(p,  f->ret);
  printf(" ");
  print_unit(p, f->name);
  print_parameters(p, f->parameters);
  printf(" {\n");
  p->indent += 2;
  print_block(p, f->body);
  printf("}\n");
}

