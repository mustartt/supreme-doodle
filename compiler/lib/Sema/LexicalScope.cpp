#include "rxc/Sema/LexicalScope.h"
#include "rxc/AST/AST.h"
#include <optional>

namespace rx::sema {

std::optional<LexicalScope *> LexicalScope::find(llvm::StringRef Symbol) {
  LexicalScope *Curr = this;
  while (Curr) {
    if (Curr->SymbolTable.contains(Symbol))
      return Curr;
    Curr = Curr->parent();
  }
  return std::nullopt;
}

void LexicalScope::insert(llvm::StringRef Symbol, ast::Decl *D) {
  auto &Vec = SymbolTable[Symbol];
  assert(!is_contained(Vec, D) &&
         "ast::Decl* Have pointer identity so there should not be duplicates");
  Vec.push_back(D);
}

llvm::ArrayRef<ast::Decl *> LexicalScope::getDecls(llvm::StringRef Symbol) {
  if (!SymbolTable.contains(Symbol))
    return llvm::ArrayRef<ast::Decl *>();
  return SymbolTable[Symbol];
}

std::string LexicalScope::GetKindString(Kind Value) {
  switch (Value) {
  case Kind::Global:
    return "Global";
  case Kind::Module:
    return "Module";
  case Kind::File:
    return "File";
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

void LexicalScope::debug(llvm::raw_ostream &OS) const {
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

} // namespace rx::sema
