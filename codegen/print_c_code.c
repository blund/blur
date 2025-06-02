#include <string.h>

#include <bl.h>

#include <ast/build.h>
#include <ast/traverse.h>
#include <ast/traversers.h>

#include <codegen/print_c_code.h>

int depth;
void indent(StringBuilder* sb) {
  fori(depth) add_to(sb, "  ");
}

void print_block(StringBuilder* sb, Block* b);
void print_literal(StringBuilder* sb, Literal* l) {
  if (l->kind == integer_lit) {
    add_to(sb, "%d", l->integer);
  }

  if (l->kind == identifier_lit) {
    add_to(sb, "%s", l->identifier);
  }
}

void print_call(StringBuilder* sb, Call* c);
void print_expr(StringBuilder* sb, Expression* e) {
  if (e->kind == lit_expr) print_literal(sb, &e->lit);
  if (e->kind == call_expr) print_call(sb, &e->call);
}

void print_call(StringBuilder* sb, Call* c) {
  char* name = c->name;
  if (!strcmp(name, "add")) {
    print_expr(sb, c->args->entries[0]);
    add_to(sb, " + ");
    print_expr(sb, c->args->entries[1]);
  }
  if (!strcmp(name, "pointer_call")) {
    Arguments* a = c->args;
    int param_count = (a->count-2) / 2;

    add_to(sb, "((");
    print_expr(sb, a->entries[0]);
    add_to(sb, " (*)(");
    fori(param_count) {
      if (i && (i < a->count-1)) add_to(sb, ", ");
      print_expr(sb, a->entries[2+2*i]);
    }
    add_to(sb, "))");
    print_expr(sb, a->entries[1]);
    add_to(sb, ")(");
    fori(param_count) {
      if (i && (i < a->count-1)) add_to(sb, ", ");
      print_expr(sb, a->entries[3+2*i]);
    }
    add_to(sb, ")");
  }
}

void print_declare(StringBuilder* sb, Declare* a) {
  add_to(sb, "%s %s", a->type.name, a->name);
}

void print_assign(StringBuilder* sb, Assign* a) {
  add_to(sb, "%s = ", a->name);
  print_expr(sb, a->expr);
}

void print_if(StringBuilder* sb, If* i) {
  add_to(sb, "if (");
  print_expr(sb, i->condition);
  add_to(sb, ") {\n");
  print_block(sb, i->then_branch);
  add_to(sb, "} else {\n");
  print_block(sb, i->else_branch);
  add_to(sb, "}\n");
}


void print_statement(StringBuilder* sb, Statement* s) {
  indent(sb);
  if (s->kind == call_statement) {
    print_call(sb, &s->call);
    add_to(sb, ";\n");
  }
  if (s->kind == assign_statement) {
    print_assign(sb, &s->assign);
    add_to(sb, ";\n");
  }
  if (s->kind == declare_statement) {
    print_declare(sb, &s->declare);
    add_to(sb, ";\n");
  }
  if (s->kind == if_statement) {
    print_if(sb, &s->if_block);
  }
}

void print_block(StringBuilder* sb, Block* b) {
  depth++;
  fori(b->count) {
    print_statement(sb, b->statements[i]);
  }
  depth--;
  indent(sb);
}

void print_params(StringBuilder* sb, Parameters* p) {
  add_to(sb, "(");
  fori(p->count) {
    Var* v = &p->entries[i];
    add_to(sb, "%s %s", v->type.name, v->name);
    if (i < p->count-1) add_to(sb, ", ");
  }
  add_to(sb, ") ");
}

void print_func_decl(StringBuilder* sb, FuncDecl *fd) {
  add_to(sb, "%s %s", fd->ret.name, fd->name);
  print_params(sb, fd->params);
  add_to(sb, "{\n");
  print_block(sb, fd->body);
  add_to(sb, "}\n");
}
