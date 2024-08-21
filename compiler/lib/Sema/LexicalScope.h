#ifndef SEMA_LEXICAL_SCOPE_H
#define SEMA_LEXICAL_SCOPE_H

#include "rxc/AST/AST.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>

namespace rx::sema {

class LexicalScope {
public:
  LexicalScope() : Parent(nullptr), SymbolTable() {}
  LexicalScope(LexicalScope *Parent) : Parent(Parent), SymbolTable() {}
  ~LexicalScope() = default;

  LexicalScope(const LexicalScope &) = delete;
  LexicalScope(LexicalScope &&) = default;
  LexicalScope &operator=(const LexicalScope &) = delete;
  LexicalScope &operator=(LexicalScope &&) = default;

public:
  LexicalScope *getParentScope() const { return Parent; }

  void insert(llvm::StringRef Symbol, ast::Decl *D) {
    auto &Vec = SymbolTable[Symbol];
    assert(!is_contained(Vec, D) &&
           "Decl * should not be duplicated under the same symbol");
    Vec.push_back(D);
  }

  ast::Decl *getSymbol(llvm::StringRef Symbol, unsigned Idx = 0) {
    if (!SymbolTable.contains(Symbol))
      return nullptr;
    assert(SymbolTable[Symbol].size() > Idx && "Symbol does not exists");
    return SymbolTable[Symbol][Idx];
  }

  llvm::ArrayRef<ast::Decl *> getSymbols(llvm::StringRef Symbol) {
    if (!SymbolTable.contains(Symbol))
      return llvm::ArrayRef<ast::Decl *>();
    return SymbolTable[Symbol];
  }

  void dump() const {
    llvm::outs() << "LexicalScope: " << this << "\n"
                 << "  Parent: " << Parent << "\n"
                 << "  Symbols:\n";
    for (const auto &[Symbol, Values] : SymbolTable) {
      llvm::outs() << "    " << Symbol << ": ";
      for (const auto *D : Values) {
        llvm::outs() << D << " ";
      }
      llvm::outs() << "\n";
    }
  }

private:
  LexicalScope *Parent;
  llvm::StringMap<llvm::SmallVector<ast::Decl *, 4>> SymbolTable;
};
} // namespace rx::sema

#endif
