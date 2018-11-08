#ifndef SYMBOL_H
#define SYMBOL_H

#include "location.h"
#include <memory>

enum SymbolType {
  ERROR, ID, OPERATOR, LITERAL, DEF, GLOBAL, PUBLISH, SUBSCRIBE, PRIM, LAMBDA,
  EQUALS, POPEN, PCLOSE, BOPEN, BCLOSE, IF, THEN, ELSE, HERE, MEMOIZE, END,
  EOL, INDENT, DEDENT
};
extern const char *symbolTable[];

struct Expr;
struct Symbol {
  SymbolType type;
  Location location;
  std::unique_ptr<Expr> expr;

  Symbol(SymbolType type_, const Location &location_) : type(type_), location(location_) { }
  Symbol(SymbolType type_, const Location &location_, std::unique_ptr<Expr> &&expr_) : type(type_), location(location_), expr(std::move(expr_)) { }
};

struct input_t;
struct state_t;

struct Lexer {
  std::unique_ptr<input_t> engine;
  std::unique_ptr<state_t> state;
  Symbol next;
  bool fail;

  Lexer(const char *file);
  Lexer(const std::string &cmdline, const char *target);
  ~Lexer();

  std::string text();
  void consume();
};

#endif
