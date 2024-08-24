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
  enum class Kind { Global, Module, File, Function, Block, Impl };

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
  std::optional<LexicalScope *> find(llvm::StringRef Symbol);
  llvm::ArrayRef<ast::Decl *> getDecls(llvm::StringRef Symbol);

  LexicalScope *parent() const { return Parent; }
  void insert(llvm::StringRef Symbol, ast::Decl *D);

  void debug(llvm::raw_ostream &OS) const;

private:
  static std::string GetKindString(Kind Value);

private:
  LexicalScope *Parent;
  Kind Type;
  llvm::StringMap<llvm::SmallVector<ast::Decl *, 4>> SymbolTable;
};
} // namespace rx::sema

#endif
