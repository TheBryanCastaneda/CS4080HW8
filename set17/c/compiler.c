#include <stdio.h>

#include "compiler.h"
#include "scanner.h"

typedef struct {
  Token current;
  Token previous;
  bool hadError;
} Parser;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,
  PREC_CONDITIONAL,
  PREC_OR,
  PREC_AND,
  PREC_EQUALITY,
  PREC_COMPARISON,
  PREC_TERM,
  PREC_FACTOR,
  PREC_UNARY,
  PREC_CALL,
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(void);

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

static Parser parser;

static void expression(void);
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void errorAt(Token* token, const char* message) {
  if (parser.hadError) return;

  fprintf(stderr, "[line %d] Error: %s\n", token->line, message);
  parser.hadError = true;
}

static void advance(void) {
  parser.previous = parser.current;

  for (;;) {
    parser.current = scanToken();
    if (parser.current.type != TOKEN_ERROR) break;

    errorAt(&parser.current, parser.current.start);
  }
}

static void consume(TokenType type, const char* message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  errorAt(&parser.current, message);
}

static void number(void) {
}

static void grouping(void) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary(void) {
  parsePrecedence(PREC_UNARY);
}

static void binary(void) {
  ParseRule* rule = getRule(parser.previous.type);
  parsePrecedence((Precedence)(rule->precedence + 1));
}

static void conditional(void) {
  expression();
  consume(TOKEN_COLON, "Expect ':' after then branch.");
  parsePrecedence(PREC_CONDITIONAL);
}

static ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]  = {grouping, NULL,        PREC_NONE},
  [TOKEN_RIGHT_PAREN] = {NULL,     NULL,        PREC_NONE},
  [TOKEN_PLUS]        = {NULL,     binary,      PREC_TERM},
  [TOKEN_MINUS]       = {unary,    binary,      PREC_TERM},
  [TOKEN_STAR]        = {NULL,     binary,      PREC_FACTOR},
  [TOKEN_SLASH]       = {NULL,     binary,      PREC_FACTOR},
  [TOKEN_QUESTION]    = {NULL,     conditional, PREC_CONDITIONAL},
  [TOKEN_COLON]       = {NULL,     NULL,        PREC_NONE},
  [TOKEN_NUMBER]      = {number,   NULL,        PREC_NONE},
  [TOKEN_ERROR]       = {NULL,     NULL,        PREC_NONE},
  [TOKEN_EOF]         = {NULL,     NULL,        PREC_NONE}
};

static ParseRule* getRule(TokenType type) {
  return &rules[type];
}

static void parsePrecedence(Precedence precedence) {
  if (parser.hadError) return;

  advance();
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;

  if (prefixRule == NULL) {
    errorAt(&parser.previous, "Expect expression.");
    return;
  }

  prefixRule();

  while (!parser.hadError &&
      precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    infixRule();
  }
}

static void expression(void) {
  parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* source) {
  initScanner(source);
  parser.hadError = false;
  parser.current = (Token){TOKEN_EOF, source, 0, 1};

  advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression.");

  return !parser.hadError;
}