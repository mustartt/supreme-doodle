#ifndef SEMA_LEXICAL_CONTEXT_H
#define SEMA_LEXICAL_CONTEXT_H

#include "LexicalScope.h"
#include <deque>
#include <llvm/Support/raw_ostream.h>

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

  void debug(llvm::raw_ostream& OS) const {
    for (const auto &Scope : ScopeStorage) {
      Scope.debug(OS);
    }
  }

private:
  std::deque<LexicalScope> ScopeStorage;
};

} // namespace rx::sema
#endif
