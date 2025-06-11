
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
} ParseContext;


#define then(x) if (ctx->result == ParseSuccess) { x; }
#define or(x)   if (ctx->result == ParseFail) { ctx->head = ctx->old_head; x;}


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
  parse_identifier(ctx);
  then(parse_exact("(", ctx));
  then(parse_expr(ctx));
  then(parse_exact(",", ctx));
  then(parse_expr(ctx));
  then(parse_exact(")", ctx));
}

void parse_let(ParseContext *ctx) {
  parse_type(ctx);
  then(parse_identifier(ctx));
  then(parse_exact("=", ctx));
  then(parse_integer(ctx));
  then(parse_exact(";", ctx));
}

void parse_if(ParseContext *ctx) {
  parse_exact("if", ctx);
  then(parse_exact("(", ctx));
  then(parse_expr(ctx));
  then(parse_exact(")", ctx));
}

char *program = "int test = 3; if (0) {add(test, 4} {add(test, 8)}";
char *add = "add(8, add(test, test))";

int main() {

  ParseContext ctx = {.head = add};

  // parse_exact(";", &ctx);
  parse_call(&ctx);

  return ctx.result;

}
