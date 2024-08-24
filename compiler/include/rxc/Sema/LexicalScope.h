#ifndef SEMA_LEXICAL_SCOPE_H
#define SEMA_LEXICAL_SCOPE_H

#include "rxc/AST/AST.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>

namespace rx::sema {

class LexicalScope {
public:
  enum class Kind { Global, Module, Function, Block, Impl };

  LexicalScope() = delete;
  LexicalScope(Kind Type) : Parent(nullptr), Type(Type), SymbolTable() {}
  LexicalScope(LexicalScope *Parent, Kind Type)
      : Parent(Parent), Type(Type), SymbolTable() {}
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

  static std::string GetKindString(Kind Value) {
    switch (Value) {
    case Kind::Global:
      return "Global";
    case Kind::Module:
      return "Module";
    case Kind::Function:
      return "Function";
    case Kind::Block:
      return "Block";
    case Kind::Impl:
      return "Impl";
    default:
      llvm_unreachable("Invalid Module Kind");
    }
  }

  void debug(llvm::raw_ostream &OS) const {
    OS << "LexicalScope: " << this << "\n"
       << "  Parent: " << Parent << "\n"
       << "  Kind: " << GetKindString(Type) << "\n"
       << "  Symbols:\n";
    for (const auto &[Symbol, Values] : SymbolTable) {
      OS << "    " << Symbol << ": ";
      for (const auto *D : Values) {
        OS << D << " ";
      }
      OS << "\n";
    }
  }

private:
  LexicalScope *Parent;
  Kind Type;
  llvm::StringMap<llvm::SmallVector<ast::Decl *, 4>> SymbolTable;
};
} // namespace rx::sema

#endif
