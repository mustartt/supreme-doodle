#include "AST.h"
#include "ASTVisitor.h"
#include "ASTPrinter.h"

using namespace rx::ast;

int main() {
  PackageDecl PkgDecl("main");

  ImportDecl Import1("io");
  ImportDecl Import2("http");

  llvm::SmallVector<Decl *> Decls;
  Decls.push_back(&PkgDecl);
  Decls.push_back(&Import1);
  Decls.push_back(&Import2);

  ProgramDecl Program(std::move(Decls));

  ASTPrinter Printer;
  Printer.print(llvm::outs(), &Program);

  return 0;
}
