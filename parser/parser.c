
#include <string.h>
#include <malloc.h>

#include <ast/ast.h>

typedef enum {
  ParseSuccess,
  ParseFail,
} ParseResult;

typedef struct {
  char *head;
  char *old_head;
  ParseResult result;
  int indent;
} ParseContext;


#define then(x) if (ctx->result == ParseSuccess) { x; }
#define or(x)   if (ctx->result == ParseFail) { ctx->head = ctx->old_head; x;}
#define some(x) while(ctx->result == ParseSuccess) { x; };

void print_indent(ParseContext *ctx) {
  for(int i = 0; i < ctx->indent; i++) printf(" ");
}

void parse_whitespace(ParseContext *ctx) {
  for (;;) {
    switch (*ctx->head) {
    case ' ':
    case '\n':
    case '\t':
      ctx->head++;
    default:
      ctx->result = ParseSuccess; return;
    }
  }
}

void parse_exact(char *this, ParseContext *ctx) {
  parse_whitespace(ctx);
  int len = strlen(this);

  if (strncmp(this, ctx->head, len) == 0) {
    ctx->head += len;
    ctx->result = ParseSuccess;
  } else {
    ctx->result = ParseFail;
  }
}
void parse_num(ParseContext *ctx) {
  parse_whitespace(ctx);

  ctx->old_head = ctx->head;

  int length = 0;
  while (*ctx->head >= '0' && *ctx->head <= '9') {
    length++;
    ctx->head++;
    continue;
  }

  if (length == 0) {
    ctx->result = ParseFail;
    return;
  }

  print_indent(ctx);
  printf("%.*s\n", length, ctx->old_head);
  ctx->result = ParseSuccess;

}

void parse_alph(ParseContext *ctx) {
  parse_whitespace(ctx);

  ctx->old_head = ctx->head;

  int length = 0;
  while (*ctx->head >= 'a' && *ctx->head <= 'z') {
    length++;
    ctx->head++;
    continue;
  }

  if (length == 0) {
    ctx->result = ParseFail;
    return;
  }

  print_indent(ctx);
  printf("%.*s\n", length, ctx->old_head);
  ctx->result = ParseSuccess;

}

void parse_type(ParseContext *ctx) {
  parse_alph(ctx);
}

void parse_identifier(ParseContext *ctx) {
  parse_alph(ctx);
}

void parse_integer(ParseContext *ctx) {
  parse_num(ctx);
}

void parse_call(ParseContext* ctx);
void parse_expr(ParseContext *ctx) {
  parse_call(ctx);
  or(parse_identifier(ctx));
  or(parse_integer(ctx));
}

void parse_call(ParseContext *ctx) {
  char* old_head = ctx->head;

  parse_identifier(ctx);
  then(parse_exact("(", ctx));
  then(parse_expr(ctx));
  then(parse_exact(",", ctx));
  then(parse_expr(ctx));
  then(parse_exact(")", ctx));

  ctx->old_head = old_head;
}

void parse_let(ParseContext *ctx) {
  char* old_head = ctx->head;

  parse_type(ctx);
  then(parse_identifier(ctx));
  then(parse_exact("=", ctx));
  then(parse_integer(ctx));
  then(parse_exact(";", ctx));

  ctx->old_head = old_head;
}

void parse_block(ParseContext *ctx);
void parse_if(ParseContext *ctx) {
  char *old_head = ctx->head;

  parse_exact("if", ctx);
  then(parse_exact("(", ctx));
  then(parse_expr(ctx));
  then(parse_exact(")", ctx));
  then(parse_block(ctx));
  then(parse_exact("else", ctx));
  then(parse_block(ctx));

  ctx->old_head = old_head;
}

void parse_statement(ParseContext *ctx) {
  parse_call(ctx);
  then(parse_exact(";", ctx));
  then(return);

  or(parse_let(ctx));
  then(parse_exact(";", ctx));
  then(return);

  or(parse_if(ctx));
}

void parse_block(ParseContext *ctx) {
  parse_exact("{", ctx);
  some(parse_statement(ctx));
  parse_exact("}", ctx);
}

char *program = "if (0) { add(test, 4); yoing(doing, boink); } else { add(test, 8); }";

int main() {

  ParseContext ctx = {.head = program, .indent = 0};

  parse_statement(&ctx);

  return ctx.result;

}
