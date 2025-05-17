#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#include "ast.h"
#include "parse.h"

#include "../bl.h"

void eat_whitespace(Parser* p) {
  if (p->index >= p->len) return;
  while (strchr(" \\\n\t", THIS)) p->index++;
}

Unit parse_text(Parser* p) {
  eat_whitespace(p);
  p->ok = 1;
  Unit u = {.start = p->index};

  while ((THIS >= 'A' && THIS < 'Z') || (THIS >= 'a' && THIS <= 'z')) {
    p->index++;
  }

  u.end = p->index;
  return u;
}

Value parse_number(Parser* p) {
  eat_whitespace(p);
  p->ok = 1; // Reset index
  int i = p->index; // Save index

  Value v;
  v.type = integer_type;
 
  Unit u = {.start = p->index}; 

  // Return early if not ok
  if (THIS < '0' || THIS > '9') {
    p->ok = 0;
    return v;
  }
  
  while (THIS >= '0' && THIS <= '9'){
    p->index++;
  }

  u.end = p->index;
  v.data = u;
  return v;
}

void parse_exact(Parser* p, char c) {
  eat_whitespace(p);

  p->ok = THIS == c;
  if (p->ok) p->index++;

  return;
}

Value parse_string(Parser* p) {
  eat_whitespace(p);
  p->ok = 1; // Reset index
  int i = p->index; // Save index

  Value v;
  v.type = string_type;
  
  Unit u = {.start = p->index};

  parse_exact(p, '"');
  while (THIS != '"') {
    p->index++;
  }
  parse_exact(p, '"');

  u.end = p->index;
  v.data = u;
  return v;
}

/* Types have a word and maybe a '*' */
Type parse_type(Parser* p) {
  eat_whitespace(p);
  Type t;

  t.name = parse_text(p);

  // @TODO - handle pointer in type;
  parse_exact(p, '*');
  t.ptr = p->ok;

  return t;
}


Call parse_call(Parser* p) {
  eat_whitespace(p);
  p->ok = 1;
  int i = p->index;

  Call c;

  c.name = parse_text(p);
  int ok = 1;
  parse_exact(p, '('); ok &= p->ok;
  parse_exact(p, ')'); ok &= p->ok;

  if (!ok) {
    p->index = i;
    return c;
  }
  
  return c;
}

Expr* parse_expr(Parser* p) {
  eat_whitespace(p);
  p->ok = 1;
  Expr* e = malloc(sizeof(Expr));

  e->kind = expr_call_kind;
  e->call = parse_call(p);

  if (!p->ok) {
    e->kind = expr_value_kind;
    e->value = parse_number(p);
  }

  if (!p->ok) {
    e->kind = expr_value_kind;
    e->value = parse_string(p);
  }
  return e;
}

Assign parse_Assign(Parser* p) {
  eat_whitespace(p);
  p->ok = 1;
  int i = p->index;

  Assign a;

  a.type = parse_type(p);
  a.name = parse_text(p);
  parse_exact(p, '=');

  if (!p->ok) {
    p->index = i;
    return a;
  }

  a.expr = parse_expr(p);
  
  if (!p->ok) {
    p->index = i;
    return a;
  }

  return a;
}

ArgList parse_arg_list(Parser *p) {
  ArgList arg_list = {.arg_count = 1};
  parse_exact(p, '(');
  fori(8) {
    arg_list.types[i] = parse_type(p);
    arg_list.names[i] =  parse_text(p);

    parse_exact(p, ',');
    if (p->ok) continue;

    parse_exact(p, ')');
    if (p->ok) {
      arg_list.arg_count = i+1;
      break;
    }
  }
  return arg_list;
}

FuncDecl parse_func_decl(Parser* p) {
  eat_whitespace(p);
  p->ok = 1;

  Type type = parse_type(p);
  Unit name = parse_text(p);

  ArgList arg_list = parse_arg_list(p);

  Block* Block = parse_scope(p);

  FuncDecl f;
  f.name = name;
  f.arg_list = arg_list;
  f.ret = type;
  f.body = Block;

  if (!p->ok) {
    parse_error(p, p->index);
  }

  return f;
}

void parse_error(Parser* p, int start) {
  int newline_count = 0;
  for (int i = 0; i < start; i++) {
    if (p->code[i] == '\n') newline_count++;
  }

  int next_newline_index = 0;
  for (int i = p->index;;i++) {
    if (p->code[i] == '\n') {
      next_newline_index = i;
      break;
    }
  }
  printf("index: %d\n", p->index);
  printf("next n: %d\n", next_newline_index);

  printf("Parse error at line: %d\n", newline_count);
  printf("'%.*s'\n", next_newline_index-start-1, &p->code[start]);

  exit(-1);
}

Statement* parse_statement(Parser* p) {
  eat_whitespace(p);
  int i = p->index;

  Statement* s = malloc(sizeof(Statement));

  // Try if-Block
  p->ok = 1;
  s->kind = statement_if_kind;
  s->if_block = parse_if_block(p);
  if (p->ok) {
    return s;
  }

  // Try function call
  p->ok = 1;
  s->kind = statement_call_kind;
  s->call = parse_call(p);
  parse_exact(p, ';');
  if (p->ok) {
    return s;
  }

  // Try Assignment
  // @NOTE - I think there is a bug if this is not the last check here. Has to do with recovering..
  p->ok = 1;
  int ok = 1;
  s->kind = statement_assign_kind;
  s->assign = parse_Assign(p);
  ok &= p->ok;
  parse_exact(p, ';');
  ok &= p->ok;
  if (ok) {
    return s;
  }

  return s;
}

Block* parse_block(Parser* p) {
  int i = p->index;

  Block* b = malloc(sizeof(Block));
  b->statement = parse_statement(p);

  b->next = 0;
  Block* iter = b;
  for (;;) {
    int i = p->index;
    int ok = 1;
    Statement* s = parse_statement(p);

    if (!p->ok) {
      p->index = i;
      p->ok = 1; // This is an expected condition
      iter->next = 0;
      return b;
    }

    Block* next = malloc(sizeof(Block));
    next->statement = s;
    next->next = 0;
    iter->next = next;
    iter = next;
  }
  return b;
}

Block* parse_scope(Parser* p) {
  p->ok = 1;
  parse_exact(p, '{');
  Block* b = parse_block(p);
  if (!p->ok) {
    return b;
  }
  parse_exact(p, '}');
  if (!p->ok) {
    return b;
  }

  return b;
}

IfBlock parse_if_block(Parser* p) {
  int i = p->index; // Save index
  p->ok = 1;

  IfBlock ib;

  parse_exact(p, 'i');
  parse_exact(p, 'f');

  if (!p->ok) {
    p->index = i;
    return ib;
  }

  int ok = 1;
  parse_exact(p, '('); ok &= p->ok;
  parse_exact(p, ')'); ok &= p->ok;

  printf("%d\n", ok);

  if (!ok) {
    parse_error(p, i);
  }
  ib.body = parse_scope(p);

  if (!p->ok) {
    parse_error(p, i);
  }

  return ib;
}
