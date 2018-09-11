#include "prim.h"
#include "value.h"
#include "expr.h"
#include "heap.h"
#include <cstdlib>
#include <sstream>

std::unique_ptr<Receiver> expect_args(const char *fn, std::unique_ptr<Receiver> completion, const std::vector<std::shared_ptr<Value> > &args, int expect) {
  if (args.size() != (size_t)expect) {
    std::stringstream str;
    str << fn << " called on " << args.size() << "; was expecting " << expect << std::endl;
    resume(std::move(completion), std::make_shared<Exception>(str.str()));
    return std::unique_ptr<Receiver>();
  }

  // merge exceptions
  auto exception = std::make_shared<Exception>();
  for (auto &i : args) {
    if (i->type == Exception::type) {
      (*exception) += *reinterpret_cast<Exception*>(i.get());
    }
  }

  if (exception->causes.empty()) {
    return completion;
  } else {
    resume(std::move(completion), std::move(exception));
    return std::unique_ptr<Receiver>();
  }
}

std::unique_ptr<Receiver> cast_string(std::unique_ptr<Receiver> completion, const std::shared_ptr<Value> &value, String **str) {
  if (value->type != String::type) {
    std::stringstream str;
    str << value << " is not a String";
    resume(std::move(completion), std::make_shared<Exception>(str.str()));
    return std::unique_ptr<Receiver>();
  } else {
    *str = reinterpret_cast<String*>(value.get());
    return completion;
  }
}

std::unique_ptr<Receiver> cast_integer(std::unique_ptr<Receiver> completion, const std::shared_ptr<Value> &value, Integer **in) {
  if (value->type != Integer::type) {
    std::stringstream str;
    str << value << " is not an Integer";
    resume(std::move(completion), std::make_shared<Exception>(str.str()));
    return std::unique_ptr<Receiver>();
  } else {
    *in = reinterpret_cast<Integer*>(value.get());
    return completion;
  }
}

// true  x y = x
static std::unique_ptr<Expr> eTrue(new Lambda(LOCATION, "_", new VarRef(LOCATION, "_", 1, 0)));
std::shared_ptr<Value> make_true() {
  return std::make_shared<Closure>(eTrue.get(), nullptr);
}

// false x y = y
static std::unique_ptr<Expr> eFalse(new Lambda(LOCATION, "_", new VarRef(LOCATION, "_", 0, 0)));
std::shared_ptr<Value> make_false() {
  return std::make_shared<Closure>(eFalse.get(), nullptr);
}

// nill x y z = y
// pair x y f = f x y # with x+y already bound
static std::unique_ptr<Expr> eNill(new Lambda(LOCATION, "_", new Lambda(LOCATION, "_", new VarRef(LOCATION, "_", 1, 0))));
static std::unique_ptr<Expr> ePair(new App(LOCATION, new App(LOCATION, new VarRef(LOCATION, "_", 0, 0), new VarRef(LOCATION, "_", 1, 0)), new VarRef(LOCATION, "_", 1, 1)));
std::shared_ptr<Value> make_list(std::vector<std::shared_ptr<Value> > &&values) {
  auto out = std::make_shared<Closure>(eNill.get(), nullptr);
  for (auto i = values.rbegin(); i != values.rend(); ++i) {
    auto binding = std::make_shared<Binding>(nullptr, nullptr, 2);
    binding->future[0].assign(std::move(*i));
    binding->future[1].assign(std::move(out));
    out = std::make_shared<Closure>(ePair.get(), binding);
  }
  return out;
}