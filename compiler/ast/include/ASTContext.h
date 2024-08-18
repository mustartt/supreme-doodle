#ifndef AST_ASTCONTEXT_H
#define AST_ASTCONTEXT_H

#include "AST.h"
#include "SrcManager.h"

#include <deque>
#include <memory>
#include <utility>

namespace rx::ast {

class ASTContext {
public:
  DeclRefType *createDeclRefType(SrcRange Loc,
                                 llvm::ArrayRef<std::string> Symbols) {
    return addToContext<DeclRefType>(Loc, Symbols);
  }

  MutableType *createMutableType(SrcRange Loc, ASTType *ElementType) {
    return addToContext<MutableType>(Loc, ElementType);
  }

  PointerType *createPointerType(SrcRange Loc, ASTType *ElementType,
                                 bool Nullable) {
    return addToContext<PointerType>(Loc, ElementType, Nullable);
  }

  ArrayType *createArrayType(SrcRange Loc, ASTType *ElementType) {
    return addToContext<ArrayType>(Loc, ElementType);
  }

  FunctionType *createFunctionType(SrcRange Loc,
                                   llvm::ArrayRef<ASTType *> ParamTypes,
                                   ASTType *ReturnType) {
    return addToContext<FunctionType>(Loc, ParamTypes, ReturnType);
  }

  ObjectType *createObjectType(SrcRange Loc,
                               llvm::ArrayRef<ObjectType::Field> Fields) {
    return addToContext<ObjectType>(Loc, Fields);
  }

  EnumType *createEnumType(SrcRange Loc,
                           llvm::ArrayRef<EnumType::Member> Members) {
    return addToContext<EnumType>(Loc, Members);
  }

  ProgramDecl *createProgramDecl(SrcRange Loc, PackageDecl *Package,
                                 llvm::ArrayRef<ImportDecl *> Imports,
                                 llvm::ArrayRef<Decl *> Decls) {
    return addToContext<ProgramDecl>(Loc, Package, Imports, Decls);
  }

  PackageDecl *createPackageDecl(SrcRange Loc, SrcRange DeclLoc,
                                 std::string Name) {
    return addToContext<PackageDecl>(Loc, DeclLoc, std::move(Name));
  }

  ImportDecl *
  createImportDecl(SrcRange Loc, SrcRange DeclLoc,
                   llvm::ArrayRef<std::string> Path,
                   std::optional<std::string> Alias = std::nullopt) {
    return addToContext<ImportDecl>(Loc, DeclLoc, Path, std::move(Alias));
  }

  VarDecl *createVarDecl(SrcRange Loc, SrcRange DeclLoc, std::string Name,
                         Visibility Vis, Expression *Initializer = nullptr) {
    return addToContext<VarDecl>(Loc, DeclLoc, std::move(Name), Vis,
                                 Initializer);
  }

  TypeDecl *createTypeDecl(SrcRange Loc, SrcRange DeclLoc, std::string Name,
                           Visibility Vis, ASTType *Type) {
    return addToContext<TypeDecl>(Loc, DeclLoc, std::move(Name), Vis, Type);
  }

  FuncDecl *createFuncDecl(SrcRange Loc, SrcRange DeclLoc, std::string Name,
                           Visibility Vis,
                           llvm::ArrayRef<FuncParamDecl *> Params,
                           BlockStmt *Body = nullptr) {
    return addToContext<FuncDecl>(Loc, DeclLoc, std::move(Name), Vis, Params,
                                  Body);
  }

  FuncParamDecl *createFuncParamDecl(SrcRange Loc, SrcRange DeclLoc,
                                     std::string Name,
                                     Expression *DefaultValue) {
    return addToContext<FuncParamDecl>(Loc, DeclLoc, std::move(Name),
                                       DefaultValue);
  }

  BlockStmt *createBlockStmt(SrcRange Loc, llvm::ArrayRef<Stmt *> Stmts) {
    return addToContext<BlockStmt>(Loc, Stmts);
  }

  ReturnStmt *createReturnStmt(SrcRange Loc, Expression *Expr = nullptr) {
    return addToContext<ReturnStmt>(Loc, Expr);
  }

  DeclStmt *createDeclStmt(SrcRange Loc, Decl *D) {
    return addToContext<DeclStmt>(Loc, D);
  }

  ExprStmt *createExprStmt(SrcRange Loc, Expression *Expr) {
    return addToContext<ExprStmt>(Loc, Expr);
  }

  IfExpr *createIfExpr(SrcRange Loc, Expression *Condition, BlockStmt *Body,
                       BlockStmt *ElseBlock = nullptr) {
    return addToContext<IfExpr>(Loc, Condition, Body, ElseBlock);
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

  CallExpr *createCallExpr(SrcRange Loc, Expression *Callee,
                           llvm::ArrayRef<Expression *> Args) {
    return addToContext<CallExpr>(Loc, Callee, Args);
  }

  AccessExpr *createAccessExpr(SrcRange Loc, Expression *Expr,
                               std::string Accessor) {
    return addToContext<AccessExpr>(Loc, Expr, std::move(Accessor));
  }

  IndexExpr *createIndexExpr(SrcRange Loc, Expression *Expr, Expression *Idx) {
    return addToContext<IndexExpr>(Loc, Expr, Idx);
  }

  AssignExpr *createAssignExpr(SrcRange Loc, Expression *LHS, Expression *RHS) {
    return addToContext<AssignExpr>(Loc, LHS, RHS);
  }

  IdentifierExpr *createIdentifierExpr(SrcRange Loc, std::string Symbol) {
    return addToContext<IdentifierExpr>(Loc, std::move(Symbol));
  }

  BoolLiteral *createBoolLiteral(SrcRange Loc, bool Value) {
    return addToContext<BoolLiteral>(Loc, Value);
  }

  CharLiteral *createCharLiteral(SrcRange Loc, char Value) {
    return addToContext<CharLiteral>(Loc, Value);
  }

  NumLiteral *createNumLiteral(SrcRange Loc, llvm::APFloat Value) {
    return addToContext<NumLiteral>(Loc, Value);
  }

  StringLiteral *createStringLiteral(SrcRange Loc, std::string Value) {
    return addToContext<StringLiteral>(Loc, std::move(Value));
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
