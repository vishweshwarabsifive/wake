#include <iostream>
#include <cassert>
#include <cstring>
#include "parser.h"
#include "bind.h"
#include "symbol.h"
#include "value.h"
#include "action.h"
#include "expr.h"
#include "job.h"

void stack_trace(Action *failure) {
  Location *trace = 0;
  for (Action *action = failure; action; action = action->invoker) {
    if (action->type == Thunk::type) {
      Thunk *thunk = reinterpret_cast<Thunk*>(action);
      if (trace && !thunk->expr->location.contains(*trace))
        std::cerr << "  from " << trace->str() << std::endl;
      trace = &thunk->expr->location;
    }
  }
  // last trace is always <init>:1:1; don't print it
}

static ActionQueue queue;

void resume(Action *completion, Value *return_value) {
  PrimRet *ret = reinterpret_cast<PrimRet*>(completion);
  ret->input_value = return_value;
  queue.push_back(ret);
}

int main(int argc, const char **argv) {
  bool ok = true;

  JobTable jobtable(4);

  Top top;
  for (int i = 1; i < argc; ++i) {
    Lexer lex(argv[i]);
    parse_top(top, lex);
    if (lex.fail) ok = false;
  }

  /* Primitives */
  PrimMap pmap;
  prim_register_string(pmap);
  prim_register_integer(pmap);
  prim_register_polymorphic(pmap);
  prim_register_job(&jobtable, pmap);

  if (!bind_refs(&top, pmap)) ok = false;

  const char *slash = strrchr(argv[0], '/');
  const char *exec = slash ? slash + 1 : argv[0];
  int debug = !strcmp(exec, "wake-debug");

  if (!strcmp(exec, "wake-parse")) {
    std::cout << &top;
    return 0;
  }

  if (!ok) {
    std::cerr << ">>> Aborting without execution <<<" << std::endl;
    return 1;
  }

  Thunk *result = new Thunk(0, &top, 0);
  queue.push_back(result);
  unsigned long steps = 0, widest = 0;
  do while (!queue.empty()) {
    Action *doit = queue.front();
    queue.pop_front();

    if (debug) {
      if (doit->type == Thunk::type) {
        Thunk *thunk = reinterpret_cast<Thunk*>(doit);
        std::cerr << "Executing " << thunk->expr->type << " @ " << thunk->expr->location.str() << std::endl;
      } else {
        std::cerr << "Executing " << doit->type << std::endl;
      }
      ++steps;
      if (queue.size() > widest) widest = queue.size();
    }

    doit->execute(queue);
  } while (jobtable.wait());

  if (debug) {
    std::cerr << "Computed in " << steps << " steps with " << widest << " in parallel." << std::endl;
  }
  assert (result->output());
  std::cout << result->output() << std::endl;

  return 0;
}
