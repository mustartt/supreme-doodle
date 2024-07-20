#ifndef AST_ASTCONTEXT_H
#define AST_ASTCONTEXT_H

#include "ast.h"
#include <deque>

namespace rx::ast {

class ASTContext {
public:
  Program *createProgram() {
    ProgramDecls.push_back({});
    return &ProgramDecls.back();
  }

  PackageDecl *createPackageDecl();
  ImportDecl *createImportDecl();

private:
  std::deque<Program> ProgramDecls;
  std::deque<PackageDecl> PackageDecls;
  std::deque<ImportDecl> ImportDecls;
};

} // namespace rx::ast

#endif
