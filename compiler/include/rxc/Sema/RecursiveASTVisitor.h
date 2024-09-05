#ifndef RX_AST_RECURSIVE_AST_VISITOR_H
#define RX_AST_RECURSIVE_AST_VISITOR_H

#include "rxc/AST/AST.h"
#include "rxc/AST/ASTVisitor.h"

#include "LexicalScope.h"
#include "rxc/Sema/LexicalContext.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/ErrorHandling.h>

namespace rx {

struct UniversalType {
  UniversalType() {}

  UniversalType(const UniversalType &) = default;
  UniversalType(UniversalType &&) = default;
  UniversalType &operator=(const UniversalType &) = default;
  UniversalType &operator=(UniversalType &&) = default;

  template <typename T> UniversalType(const T &) {}
  template <typename T> operator T() const { return T(); }

private:
  [[maybe_unused]] bool Body;
};

template <class T = UniversalType>
class RecursiveASTVisitor : public ast::BaseDeclVisitor,
                            public ast::BaseStmtVisitor,
                            public ast::BaseExprVisitor,
                            public ast::BaseTypeVisitor {
public:
  using ResultType = T;

public:
  RecursiveASTVisitor(sema::LexicalContext &LC, ResultType DefaultResult = T{})
      : LC(LC), DefaultResult(DefaultResult) {}

protected:
  T Visit(ast::Decl *Node) {
    Node->accept(*this);
    assert(ResultStack.size() && "No return value");
    return ResultStack.pop_back_val();
  }
  T Visit(ast::Stmt *Node) {
    Node->accept(*this);
    assert(ResultStack.size() && "No return value");
    return ResultStack.pop_back_val();
  }
  T Visit(ast::Expression *Node) {
    Node->accept(*this);
    assert(ResultStack.size() && "No return value");
    return ResultStack.pop_back_val();
  }
  T Visit(ast::ASTType *Node) {
    Node->accept(*this);
    assert(ResultStack.size() && "No return value");
    return ResultStack.pop_back_val();
  }

protected:
  virtual T visit(ast::ProgramDecl *Node, sema::LexicalScope *LS) {
    T Last(DefaultResult);
    if (Node->getPackage())
      Last = Visit(Node->getPackage());
    for (auto *I : Node->getImports())
      Last = Visit(I);
    for (auto *E : Node->getDecls())
      Last = Visit(E);
    return Last;
  }
  virtual T visit(ast::PackageDecl *Node, sema::LexicalScope *LS) {
    return DefaultResult;
  }
  virtual T visit(ast::ImportDecl *Node, sema::LexicalScope *LS) {
    return DefaultResult;
  }
  virtual T visit(ast::ExportedDecl *Node, sema::LexicalScope *LS) {
    assert(Node->getExportedDecl() && "Missing exported decl");
    return Visit(Node->getExportedDecl());
  }
  virtual T visit(ast::VarDecl *Node, sema::LexicalScope *LS) {
    T Last(DefaultResult);
    if (Node->getDeclaredType())
      Last = Visit(Node->getDeclaredType());
    if (Node->getInitializer())
      Last = Visit(Node->getInitializer());
    return Last;
  }
  virtual T visit(ast::TypeDecl *Node, sema::LexicalScope *LS) {
    assert(Node->getDeclaredType() && "Missing Type");
    return Visit(Node->getDeclaredType());
  }
  virtual T visit(ast::ImplDecl *Node, sema::LexicalScope *LS) {
    T Last(DefaultResult);
    assert(Node->getDeclaredType() && "Missing Type");
    Last = Visit(Node->getDeclaredType());
    for (auto *F : Node->getImpls())
      Last = Visit(F);
    return Last;
  }
  virtual T visit(ast::UseDecl *Node, sema::LexicalScope *LS) {
    assert(Node->getDeclaredType() && "Missing Type");
    return Visit(Node->getDeclaredType());
  }
  virtual T visit(ast::FuncDecl *Node, sema::LexicalScope *LS) {
    T Last(DefaultResult);
    for (auto *P : Node->getParams())
      Last = Visit(P);
    Last = Visit(Node->getDeclaredType());
    if (Node->getBody())
      Last = Visit(Node->getBody());
    return Last;
  }
  virtual T visit(ast::FuncParamDecl *Node, sema::LexicalScope *LS) {
    T Last = Visit(Node->getDeclaredType());
    if (Node->getDefaultValue())
      Last = Visit(Node->getDefaultValue());
    return Last;
  }

  // visit types
  virtual T visit(ast::ASTBuiltinType *Node, sema::LexicalScope *LS) {
    return DefaultResult;
  }
  virtual T visit(ast::ASTDeclTypeRef *Node, sema::LexicalScope *LS) {
    return DefaultResult;
  }
  virtual T visit(ast::ASTAccessType *Node, sema::LexicalScope *LS) {
    return Visit(Node->getParentType());
  }
  virtual T visit(ast::ASTQualType *Node, sema::LexicalScope *LS) {
    return Visit(Node->getElementType());
  }
  virtual T visit(ast::ASTPointerType *Node, sema::LexicalScope *LS) {
    return Visit(Node->getElementType());
  }
  virtual T visit(ast::ASTArrayType *Node, sema::LexicalScope *LS) {
    return Visit(Node->getElementType());
  }
  virtual T visit(ast::ASTFunctionType *Node, sema::LexicalScope *LS) {
    T Last;
    for (auto *P : Node->getParamTypes())
      Last = Visit(P);
    Last = Visit(Node->getReturnType());
    return Last;
  }
  virtual T visit(ast::ASTObjectType *Node, sema::LexicalScope *LS) {
    T Last;
    for (auto &F : Node->getFields())
      Last = Visit(F.second);
    return Last;
  }
  virtual T visit(ast::ASTEnumType *Node, sema::LexicalScope *LS) {
    T Last;
    for (auto &M : Node->getMembers())
      Last = Visit(M.second);
    return Last;
  }

  // visit expr
  virtual T visit(ast::DeclRefExpr *Node, sema::LexicalScope *LS) {
    return DefaultResult;
  }
  virtual T visit(ast::IfExpr *Node, sema::LexicalScope *LS) {
    T Last = DefaultResult;
    if (Node->getCondition())
      Last = Visit(Node->getCondition());
    if (Node->getBody())
      Last = Visit(Node->getBody());
    if (Node->getElseBlock())
      Last = Visit(Node->getElseBlock());
    return Last;
  }
  virtual T visit(ast::BinaryExpr *Node, sema::LexicalScope *LS) {
    Visit(Node->getLHS());
    return Visit(Node->getRHS());
  }
  virtual T visit(ast::UnaryExpr *Node, sema::LexicalScope *LS) {
    return Visit(Node->getExpr());
  }
  virtual T visit(ast::CallExpr *Node, sema::LexicalScope *LS) {
    for (auto *Arg : Node->getArgs())
      Visit(Arg);
    return Visit(Node->getCallee());
  }
  virtual T visit(ast::AccessExpr *Node, sema::LexicalScope *LS) {
    return Visit(Node->getExpr());
  }
  virtual T visit(ast::IndexExpr *Node, sema::LexicalScope *LS) {
    Visit(Node->getIdx());
    return Visit(Node->getExpr());
  }
  virtual T visit(ast::AssignExpr *Node, sema::LexicalScope *LS) {
    Visit(Node->getLHS());
    return Visit(Node->getRHS());
  }
  virtual T visit(ast::ObjectLiteral *Node, sema::LexicalScope *LS) {
    T Last = DefaultResult;
    for (auto &F : Node->getFields())
      Last = Visit(F.getValue());
    return Last;
  }
  virtual T visit(ast::BoolLiteral *Node, sema::LexicalScope *LS) {
    return DefaultResult;
  }
  virtual T visit(ast::CharLiteral *Node, sema::LexicalScope *LS) {
    return DefaultResult;
  }
  virtual T visit(ast::NumLiteral *Node, sema::LexicalScope *LS) {
    return DefaultResult;
  }
  virtual T visit(ast::StringLiteral *Node, sema::LexicalScope *LS) {
    return DefaultResult;
  }

  // visit stmts
  virtual T visit(ast::BlockStmt *Node, sema::LexicalScope *LS) {
    T Last = DefaultResult;
    for (auto *S : Node->getStmts())
      Last = Visit(S);
    return Last;
  }
  virtual T visit(ast::ReturnStmt *Node, sema::LexicalScope *LS) {
    return Visit(Node->getExpr());
  }
  virtual T visit(ast::DeclStmt *Node, sema::LexicalScope *LS) {
    return Visit(Node->getDecl());
  }
  virtual T visit(ast::ExprStmt *Node, sema::LexicalScope *LS) {
    return Visit(Node->getExpr());
  }
  virtual T visit(ast::ForStmt *Node, sema::LexicalScope *LS) {
    if (Node->getPreHeader())
      Visit(Node->getPreHeader());
    if (Node->getCondition())
      Visit(Node->getPostExpr());
    if (Node->getPostExpr())
      Visit(Node->getPostExpr());
    return Visit(Node->getBody());
  }

private:
  // visit decls
  void visit(ast::ProgramDecl *Node) override {
    if (!Node->getLexicalScope()) {
      Node->setLexicalScope(LC.createNewScope(sema::LexicalScope::Kind::File,
                                              LC.getGlobalScope()));
    }
    CurrentScope.push_back(Node->getLexicalScope());
    ResultStack.push_back(visit(Node, CurrentScope.back()));
    CurrentScope.pop_back();
  }
  void visit(ast::PackageDecl *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ImportDecl *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ExportedDecl *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::VarDecl *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::TypeDecl *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ImplDecl *Node) override {
    if (!Node->getLexicalScope()) {
      assert(CurrentScope.size() && "There must exists an parent scope");
      Node->setLexicalScope(LC.createNewScope(sema::LexicalScope::Kind::Impl,
                                              CurrentScope.back()));
    }
    CurrentScope.push_back(Node->getLexicalScope());
    ResultStack.push_back(visit(Node, CurrentScope.back()));
    CurrentScope.pop_back();
  }
  void visit(ast::UseDecl *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::FuncDecl *Node) override {
    if (!Node->getLexicalScope()) {
      assert(CurrentScope.size() && "There must exists an parent scope");
      Node->setLexicalScope(LC.createNewScope(
          sema::LexicalScope::Kind::Function, CurrentScope.back()));
    }
    CurrentScope.push_back(Node->getLexicalScope());
    ResultStack.push_back(visit(Node, CurrentScope.back()));
    CurrentScope.pop_back();
  }
  void visit(ast::FuncParamDecl *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }

  // visit types
  void visit(ast::ASTBuiltinType *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ASTDeclTypeRef *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ASTAccessType *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ASTQualType *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ASTPointerType *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ASTArrayType *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ASTFunctionType *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ASTObjectType *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ASTEnumType *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }

  // visit expr
  void visit(ast::DeclRefExpr *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::IfExpr *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::BinaryExpr *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::UnaryExpr *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::CallExpr *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::AccessExpr *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::IndexExpr *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::AssignExpr *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ObjectLiteral *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::BoolLiteral *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::CharLiteral *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::NumLiteral *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::StringLiteral *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }

  // visit stmts
  void visit(ast::BlockStmt *Node) override {
    if (!Node->getLexicalScope()) {
      assert(CurrentScope.size() && "There must exists an parent scope");
      Node->setLexicalScope(LC.createNewScope(sema::LexicalScope::Kind::Block,
                                              CurrentScope.back()));
    }
    CurrentScope.push_back(Node->getLexicalScope());
    ResultStack.push_back(visit(Node, CurrentScope.back()));
    CurrentScope.pop_back();
  }
  void visit(ast::ReturnStmt *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::DeclStmt *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ExprStmt *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }
  void visit(ast::ForStmt *Node) override {
    ResultStack.push_back(visit(Node, CurrentScope.back()));
  }

protected:
  sema::LexicalContext &LC;

private:
  T DefaultResult;
  llvm::SmallVector<T, 32> ResultStack;
  llvm::SmallVector<sema::LexicalScope *, 32> CurrentScope;
};

} // namespace rx

#endif
