#ifndef AST_ASTCONTEXT_H
#define AST_ASTCONTEXT_H

#include "AST.h"

#include <deque>
#include <memory>
#include <utility>

namespace rx::ast {

class ASTContext {
public:
  ProgramDecl *createProgramDecl(SrcRange Loc, PackageDecl *Package,
                                 llvm::ArrayRef<ImportDecl *> Imports,
                                 llvm::ArrayRef<Decl *> Decls) {
    return addToContext<ProgramDecl>(Loc, Package, Imports, Decls);
  }

  PackageDecl *createPackageDecl(SrcRange Loc, std::string Name) {
    return addToContext<PackageDecl>(Loc, std::move(Name));
  }

  ImportDecl *
  createImportDecl(SrcRange Loc, llvm::ArrayRef<std::string> Path,
                   std::optional<std::string> Alias = std::nullopt) {
    return addToContext<ImportDecl>(Loc, Path, std::move(Alias));
  }

  StructDecl *createStructDecl(SrcRange Loc, std::string Name, Visibility Vis,
                               llvm::ArrayRef<FieldDecl *> Fields) {
    return addToContext<StructDecl>(Loc, std::move(Name), Vis, Fields);
  }

  FieldDecl *createFieldDecl(SrcRange Loc, std::string Name, Visibility Vis,
                             Expression *DefaultValue = nullptr) {
    return addToContext<FieldDecl>(Loc, std::move(Name), Vis, DefaultValue);
  }

  VarDecl *createVarDecl(SrcRange Loc, std::string Name, Visibility Vis,
                         Expression *Initializer = nullptr) {
    return addToContext<VarDecl>(Loc, std::move(Name), Vis, Initializer);
  }

  FuncDecl *createFuncDecl(SrcRange Loc, std::string Name, Visibility Vis,
                           llvm::ArrayRef<FuncParamDecl *> Params,
                           BlockStmt *Body = nullptr) {
    return addToContext<FuncDecl>(Loc, std::move(Name), Vis, Params, Body);
  }

  BlockStmt *createBlockStmt(SrcRange Loc,
                             llvm::SmallVectorImpl<Stmt *> &&Stmts) {
    return addToContext<BlockStmt>(Loc, std::move(Stmts));
  }

  ReturnStmt *createReturnStmt(SrcRange Loc, Expression *Expr = nullptr) {
    return addToContext<ReturnStmt>(Loc, Expr);
  }

  DeclStmt *createDeclStmt(SrcRange Loc, std::string Name,
                           Expression *Initializer = nullptr) {
    return addToContext<DeclStmt>(Loc, std::move(Name), Initializer);
  }

  ExprStmt *createExprStmt(SrcRange Loc, Expression *Expr) {
    return addToContext<ExprStmt>(Loc, Expr);
  }

  IfStmt *createIfStmt(SrcRange Loc, Expression *Condition, BlockStmt *Body,
                       BlockStmt *ElseBlock = nullptr) {
    return addToContext<IfStmt>(Loc, Condition, Body, ElseBlock);
  }

  ForStmt *createForStmt(SrcRange Loc, DeclStmt *PreHeader,
                         Expression *Condition, Expression *PostExpr,
                         BlockStmt *Body) {
    return addToContext<ForStmt>(Loc, PreHeader, Condition, PostExpr, Body);
  }

  BinaryExpr *createBinaryExpr(SrcRange Loc, BinaryOp Op, Expression *LHS,
                               Expression *RHS) {
    return addToContext<BinaryExpr>(Loc, Op, LHS, RHS);
  }

  UnaryExpr *createUnaryExpr(SrcRange Loc, UnaryOp Op, Expression *Expr) {
    return addToContext<UnaryExpr>(Loc, Op, Expr);
  }

private:
  template <class T, class... Args> T *addToContext(Args &&...Params) {
    auto Node = std::make_unique<T>(std::forward<Args>(Params)...);
    auto Tmp = Node.get();
    Nodes.emplace_back(std::move(Node));
    return Tmp;
  }

private:
  std::deque<std::unique_ptr<ASTNode>> Nodes;
};

} // namespace rx::ast

#endif
