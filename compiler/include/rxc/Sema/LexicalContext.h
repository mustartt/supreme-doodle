#ifndef SEMA_LEXICAL_CONTEXT_H
#define SEMA_LEXICAL_CONTEXT_H

#include "LexicalScope.h"
#include <deque>

namespace rx::sema {

class LexicalContext {
public:
  LexicalContext() = default;

  LexicalContext(const LexicalContext &) = delete;
  LexicalContext(LexicalContext &&) = default;
  LexicalContext &operator=(const LexicalContext &) = delete;
  LexicalContext &operator=(LexicalContext &&) = default;

public:
  LexicalScope *createNewScope(LexicalScope::Kind Type,
                               LexicalScope *Parent = nullptr) {
    return &ScopeStorage.emplace_back(Parent, Type);
  }

  void dump() const {
    llvm::outs() << "=== Start of LexicalContext Dump ===\n";
    for (const auto &Scope : ScopeStorage) {
      Scope.dump();
    }
    llvm::outs() << "=== End of LexicalContext Dump ===\n";
  }

private:
  std::deque<LexicalScope> ScopeStorage;
};

} // namespace rx::sema
#endif
