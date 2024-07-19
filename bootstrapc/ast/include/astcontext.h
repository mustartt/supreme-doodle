#ifndef AST_ASTCONTEXT_H
#define AST_ASTCONTEXT_H

#include "ast.h"

namespace rx::ast {

class ASTContext {
public:
  Program *createProgram();
  PackageDecl *createPackageDecl();
  ImportDecl *createImportDecl();

private:
  std::vector<Program> ProgramDecls;
  std::vector<PackageDecl> PackageDecls;
  std::vector<ImportDecl> ImportDecls;
};

} // namespace rx::ast

#endif
