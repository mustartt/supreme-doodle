#include "AST.h"
#include <iostream>

using namespace rx::ast;

class ConcreteVisitor : public ASTVisitor<ConcreteVisitor> {
public:
  void visitImpl(Program &node) {
    std::cout << "Visiting Program\n";
    if (auto Pkg = node.getPackage()) {
        Pkg->accept(*this);
    }
    for (auto Import : node.getImports()) {
       Import->accept(*this); 
    }
  }

  void visitImpl(PackageDecl &node) {
    std::cout << "Visiting PackageDecl\n";
  }

  void visitImpl(ImportDecl &node) {
    std::cout << "Visiting ImportDecl\n";
    // Additional logic for visiting ImportDecl
  }

  void visitImpl(StructDecl &node) {
    std::cout << "Visiting StructDecl\n";
    // Additional logic for visiting StructDecl
  }

  void visitImpl(StructFieldDecl &node) {
    std::cout << "Visiting StructFieldDecl\n";
    // Additional logic for visiting StructFieldDecl
  }

  void visitImpl(VarDecl &node) {
    std::cout << "Visiting VarDecl\n";
    // Additional logic for visiting VarDecl
  }

  void visitImpl(Expression &node) {
    std::cout << "Visiting Expression\n";
    // Additional logic for visiting Expression
  }

  void visitImpl(ReturnStmt &node) {
    std::cout << "Visiting ReturnStmt\n";
    // Additional logic for visiting ReturnStmt
  }

  void visitImpl(DeclStmt &node) {
    std::cout << "Visiting DeclStmt\n";
    // Additional logic for visiting DeclStmt
  }

  void visitImpl(ExprStmt &node) {
    std::cout << "Visiting ExprStmt\n";
    // Additional logic for visiting ExprStmt
  }

  void visitImpl(IfStmt &node) {
    std::cout << "Visiting IfStmt\n";
    // Additional logic for visiting IfStmt
  }

  void visitImpl(ForStmt &node) {
    std::cout << "Visiting ForStmt\n";
  }
};

int main() {
    PackageDecl PD("main");
    ImportDecl I1("pkg");
    ImportDecl I2("gfx");
    llvm::SmallVector<ImportDecl*, 4> Imports;
    Imports.push_back(&I1);
    Imports.push_back(&I2);

    Program P(&PD, std::move(Imports));

    ConcreteVisitor V;

    P.accept(V);

    return 0;
}
