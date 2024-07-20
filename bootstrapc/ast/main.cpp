#include "AST.h"
#include "ASTVisitor.h"

#include <iostream>

using namespace rx::ast;

class Visitor : public BaseDeclVisitor,
                public BaseStmtVisitor,
                public BaseExprVisitor {
public:
  virtual void visit(ProgramDecl *node) override {
    for (auto *Decl : node->getDecls()) {
      Decl->accept(*this);
    }
  }
  virtual void visit(PackageDecl *node) override {
     
  }
  virtual void visit(ImportDecl *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(StructDecl *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(StructFieldDecl *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(VarDecl *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(FuncDecl *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(BlockStmt *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(ReturnStmt *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(DeclStmt *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(ExprStmt *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(IfStmt *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(ForStmt *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(BinaryExpr *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
  virtual void visit(UnaryExpr *node) override {
    std::cout << "visiting " << __PRETTY_FUNCTION__ << std::endl;
  }
};

int main() {
  PackageDecl PkgDecl("main");

  ImportDecl Import1("io");
  ImportDecl Import2("http");

  llvm::SmallVector<Decl *> Decls;
  Decls.push_back(&PkgDecl);
  Decls.push_back(&Import1);
  Decls.push_back(&Import2);

  ProgramDecl Program(std::move(Decls));

  Visitor V;
  V.visit(&Program);

  return 0;
}
