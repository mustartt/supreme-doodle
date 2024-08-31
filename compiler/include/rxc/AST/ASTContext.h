#ifndef AST_ASTCONTEXT_H
#define AST_ASTCONTEXT_H

#include "AST.h"

#include <deque>
#include <memory>
#include <utility>

namespace rx::ast {

class ASTContext {
public:
  template <class T, class... Args> T *createNode(Args &&...Params) {
    auto Node = std::make_unique<T>(std::forward<Args>(Params)...);
    auto Tmp = Node.get();
    Nodes.emplace_back(std::move(Node));
    return Tmp;
  }

private:
  std::deque<std::unique_ptr<ASTNode>> Nodes;
};

} // namespace rx::ast

#endif
