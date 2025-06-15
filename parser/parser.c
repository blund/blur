

#include <stdio.h>

#include <include/stb_ds.h>

#include <ast/ast.h>
#include <ast/build.h>
#include <ast/traverse.h>
#include <ast/traversers.h>

typedef enum {
  ParseSuccess,
  ParseFail,
} ParseStatus;


typedef struct {
  char *head;
  ParseStatus status;
  void* data;
} ParseContext;

void parse_whitespace(ParseContext *ctx) {
  while (*ctx->head == ' ' || *ctx->head == '\n' || *ctx->head == '\t') {
    ctx->head++;
  }
}

char *parse_alpha(ParseContext *ctx) {
  parse_whitespace(ctx);
  ctx->status = ParseSuccess;

  char* start = ctx->head;
  int len = 0;
  while (*ctx->head >= 'a' && *ctx->head <= 'z') {
    ctx->head++;
    len++;
  }

  if (len == 0) {
    ctx->status = ParseFail;
    return 0;
  }
  
  char *result = strndup(start, len+1);
  result[len] = '\0';
  return result;
}

int parse_num(ParseContext *ctx) {
  parse_whitespace(ctx);
  ctx->status = ParseSuccess;

  char* start = ctx->head;
  char* endptr;
  long val = strtol(start, &endptr, 10);

  if (endptr == start) {
    ctx->status = ParseFail;
    return 0;
  }
  ctx->head = endptr;
  return val;
}

void parse_exact(char *this, ParseContext *ctx) {
  parse_whitespace(ctx);
  int len = strlen(this);

  if (strncmp(this, ctx->head, len) == 0) {
    ctx->head += len;
    ctx->status = ParseSuccess;
  } else {
    ctx->status = ParseFail;
  }
}


Expression *parse_call_expr(ParseContext *ctx);
Expression* parse_expr(ParseContext *ctx) {
  char *old_head = ctx->head;

  // first we try to parse an integer:
  int num = parse_num(ctx);
  if (ctx->status == ParseSuccess) {
    return integer(num);
  }

  // recover and try string
  ctx->head = old_head;
  Expression* call = parse_call_expr(ctx);
  if (ctx->status == ParseSuccess) {
    return call;
  }


  // recover and try string
  ctx->head = old_head;
  char* name = parse_alpha(ctx);
  if (ctx->status == ParseSuccess) {
    return identifier(name);
  }

  ctx->head = old_head;
  ctx->status = ParseFail;
  return 0;
}

Expression *parse_call_expr(ParseContext *ctx) {
  // @TODO - we do big fat leaks

  char *fun_name = parse_alpha(ctx);

  parse_exact("(", ctx);
  if (ctx->status == ParseFail) return 0;

  Expression *arg1 = parse_expr(ctx);
  if (ctx->status == ParseFail) return 0;

  // Since we know we have an argument, try building up the arg list
  Arguments *args = args_empty();
  arrput(args->entries, arg1);

  // Parse more args followed by a comma
  for (;;) {
    parse_exact(",", ctx);
    if (ctx->status == ParseFail) break;

    Expression* arg = parse_expr(ctx);
    if (ctx->status == ParseFail) return 0;
    arrput(args->entries, arg);
  }

  parse_exact(")", ctx);
  if (ctx->status == ParseFail) return 0;

  Expression* c = call_e(fun_name, args);
  return c;
}

Statement *parse_call(ParseContext *ctx) {
  // @TODO - we do big fat leaks

  char *fun_name = parse_alpha(ctx);

  parse_exact("(", ctx);
  if (ctx->status == ParseFail) return 0;

  Expression *arg1 = parse_expr(ctx);
  if (ctx->status == ParseFail) return 0;

  // Since we know we have an argument, try building up the arg list
  Arguments *args = args_empty();
  arrput(args->entries, arg1);

  // Parse more args followed by a comma
  for (;;) {
    parse_exact(",", ctx);
    if (ctx->status == ParseFail) break;

    Expression* arg = parse_expr(ctx);
    if (ctx->status == ParseFail) return 0;
    arrput(args->entries, arg);
  }

  parse_exact(")", ctx);
  if (ctx->status == ParseFail) return 0;

  Statement* c = call(fun_name, args);
  return c;
}

Statement *parse_let(ParseContext *ctx) {
  char *type_name = parse_alpha(ctx);
  if (ctx->status == ParseFail)
    return 0;

  char *var_name = parse_alpha(ctx);
  if (ctx->status == ParseFail)
    return 0;

  parse_exact("=", ctx);
  if (ctx->status == ParseFail)
    return 0;

  Expression* expr = parse_expr(ctx);
  if (ctx->status == ParseFail)
    return 0;

  return let(var_name, type(type_name), expr);
}


Block *parse_block(ParseContext *ctx);
Statement *parse_if(ParseContext *ctx) {
  parse_exact("if", ctx);
  if (ctx->status == ParseFail)
    return 0;

  parse_exact("(", ctx);
  if (ctx->status == ParseFail)
    return 0;

  Expression* cond = parse_expr(ctx);
  if (ctx->status == ParseFail)
    return 0;

  parse_exact(")", ctx);
  if (ctx->status == ParseFail)
    return 0;

  Block* then_block = parse_block(ctx);
  if (ctx->status == ParseFail)
    return 0;

  parse_exact("else", ctx);
  char *old_head = ctx->head;
  Block *else_block = parse_block(ctx);
  if (ctx->status == ParseFail) {
    // We put some extra safety net here. Not sure if it is needed
    ctx->head = old_head;
    ctx->status = ParseSuccess;
  }

  // It's ok if this last one fails :)

  return if_test(cond, then_block, else_block);
}

Statement *parse_statement(ParseContext *ctx) {
  char *old_head = ctx->head;
  
  Statement *if_test = parse_if(ctx);
  if (ctx->status == ParseSuccess) {
    return if_test;
  }

  ctx->head = old_head;
  Statement *let = parse_let(ctx);
  if (ctx->status == ParseSuccess) {
    parse_exact(";", ctx);
    if (ctx->status == ParseSuccess)
      return let;
  }

  ctx->head = old_head;
  Statement *call = parse_call(ctx);
  if (ctx->status == ParseSuccess) {
    parse_exact(";", ctx);
    if (ctx->status == ParseSuccess)
      return call;
  }

  ctx->status = ParseFail;
  return 0;
}

Block *parse_block(ParseContext *ctx) {
  parse_exact("{", ctx);
  if (ctx->status == ParseFail)
    return 0;

  Statement *s = parse_statement(ctx);
  if (ctx->status == ParseFail) {
    return 0;
  }

  Block *b = block_empty();
  arrput(b->statements, s);

  for (;;) {
    Statement *s = parse_statement(ctx);
    if (ctx->status == ParseFail) {
      break;
    }

    arrput(b->statements, s);
  }

  parse_exact("}", ctx);
  if (ctx->status == ParseFail)
    return 0;

  return b;
}

Block *parse(char *code) {
  ParseContext ctx = {.head = code};

  return parse_block(&ctx);
}
