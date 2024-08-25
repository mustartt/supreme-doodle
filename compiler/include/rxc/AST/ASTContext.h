#ifndef AST_ASTCONTEXT_H
#define AST_ASTCONTEXT_H

#include "AST.h"

#include <deque>
#include <memory>
#include <utility>

namespace rx::ast {

class ASTContext {
public:
  DeclRefType *createDeclRefType(SourceLocation Loc, std::string Symbol) {
    return addToContext<DeclRefType>(Loc, std::move(Symbol));
  }

  BuiltinType *createBuiltinType(NativeType Type) {
    return addToContext<BuiltinType>(Type);
  }

  AccessType *createAccessType(SourceLocation Loc, std::string Symbol,
                               ASTType *ParentType) {
    return addToContext<AccessType>(Loc, std::move(Symbol), ParentType);
  }

  MutableType *createMutableType(SourceLocation Loc, ASTType *ElementType) {
    return addToContext<MutableType>(Loc, ElementType);
  }

  PointerType *createPointerType(SourceLocation Loc, ASTType *ElementType,
                                 bool Nullable) {
    return addToContext<PointerType>(Loc, ElementType, Nullable);
  }

  ArrayType *createArrayType(SourceLocation Loc, ASTType *ElementType) {
    return addToContext<ArrayType>(Loc, ElementType);
  }

  FunctionType *createFunctionType(SourceLocation Loc,
                                   llvm::ArrayRef<ASTType *> ParamTypes,
                                   ASTType *ReturnType) {
    return addToContext<FunctionType>(Loc, ParamTypes, ReturnType);
  }

  ObjectType *createObjectType(SourceLocation Loc,
                               llvm::ArrayRef<ObjectType::Field> Fields) {
    return addToContext<ObjectType>(Loc, Fields);
  }

  EnumType *createEnumType(SourceLocation Loc,
                           llvm::ArrayRef<EnumType::Member> Members) {
    return addToContext<EnumType>(Loc, Members);
  }

  ProgramDecl *createProgramDecl(SourceLocation Loc, PackageDecl *Package,
                                 llvm::ArrayRef<ImportDecl *> Imports,
                                 llvm::ArrayRef<Decl *> Decls) {
    return addToContext<ProgramDecl>(Loc, Package, Imports, Decls);
  }

  PackageDecl *createPackageDecl(SourceLocation Loc, SourceLocation DeclLoc,
                                 std::string Name) {
    return addToContext<PackageDecl>(Loc, DeclLoc, std::move(Name));
  }

  ImportDecl *
  createImportDecl(SourceLocation Loc, SourceLocation DeclLoc, ImportDecl::ImportType Type,
                   std::string Path,
                   std::optional<std::string> Alias = std::nullopt) {
    return addToContext<ImportDecl>(Loc, DeclLoc, Type, std::move(Path),
                                    std::move(Alias));
  }

  VarDecl *createVarDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
                         Visibility Vis, Expression *Initializer = nullptr) {
    return addToContext<VarDecl>(Loc, DeclLoc, std::move(Name), Vis,
                                 Initializer);
  }

  TypeDecl *createTypeDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
                           Visibility Vis, ASTType *Type) {
    return addToContext<TypeDecl>(Loc, DeclLoc, std::move(Name), Vis, Type);
  }

  UseDecl *createUseDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
                         Visibility Vis, ASTType *Type) {
    return addToContext<UseDecl>(Loc, DeclLoc, std::move(Name), Vis, Type);
  }

  ImplDecl *createImpleDecl(SourceLocation Loc, SourceLocation DeclLoc, ASTType *ImplType,
                            Visibility Vis, llvm::ArrayRef<FuncDecl *> Impls) {
    return addToContext<ImplDecl>(Loc, DeclLoc, ImplType, Vis, Impls);
  }

  FuncDecl *createFuncDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
                           Visibility Vis,
                           llvm::ArrayRef<FuncParamDecl *> Params,
                           BlockStmt *Body = nullptr) {
    return addToContext<FuncDecl>(Loc, DeclLoc, std::move(Name), Vis, Params,
                                  Body);
  }

  FuncParamDecl *createFuncParamDecl(SourceLocation Loc, SourceLocation DeclLoc,
                                     std::string Name,
                                     Expression *DefaultValue) {
    return addToContext<FuncParamDecl>(Loc, DeclLoc, std::move(Name),
                                       DefaultValue);
  }

  BlockStmt *createBlockStmt(SourceLocation Loc, llvm::ArrayRef<Stmt *> Stmts) {
    return addToContext<BlockStmt>(Loc, Stmts);
  }

  ReturnStmt *createReturnStmt(SourceLocation Loc, Expression *Expr = nullptr) {
    return addToContext<ReturnStmt>(Loc, Expr);
  }

  DeclStmt *createDeclStmt(SourceLocation Loc, Decl *D) {
    return addToContext<DeclStmt>(Loc, D);
  }

  ExprStmt *createExprStmt(SourceLocation Loc, Expression *Expr) {
    return addToContext<ExprStmt>(Loc, Expr);
  }

  IfExpr *createIfExpr(SourceLocation Loc, Expression *Condition, BlockStmt *Body,
                       BlockStmt *ElseBlock = nullptr) {
    return addToContext<IfExpr>(Loc, Condition, Body, ElseBlock);
  }

  ForStmt *createForStmt(SourceLocation Loc, DeclStmt *PreHeader,
                         Expression *Condition, Expression *PostExpr,
                         BlockStmt *Body) {
    return addToContext<ForStmt>(Loc, PreHeader, Condition, PostExpr, Body);
  }

  BinaryExpr *createBinaryExpr(SourceLocation Loc, BinaryOp Op, Expression *LHS,
                               Expression *RHS) {
    return addToContext<BinaryExpr>(Loc, Op, LHS, RHS);
  }

  UnaryExpr *createUnaryExpr(SourceLocation Loc, UnaryOp Op, Expression *Expr) {
    return addToContext<UnaryExpr>(Loc, Op, Expr);
  }

  CallExpr *createCallExpr(SourceLocation Loc, Expression *Callee,
                           llvm::ArrayRef<Expression *> Args) {
    return addToContext<CallExpr>(Loc, Callee, Args);
  }

  AccessExpr *createAccessExpr(SourceLocation Loc, Expression *Expr,
                               std::string Accessor) {
    return addToContext<AccessExpr>(Loc, Expr, std::move(Accessor));
  }

  IndexExpr *createIndexExpr(SourceLocation Loc, Expression *Expr, Expression *Idx) {
    return addToContext<IndexExpr>(Loc, Expr, Idx);
  }

  AssignExpr *createAssignExpr(SourceLocation Loc, Expression *LHS, Expression *RHS) {
    return addToContext<AssignExpr>(Loc, LHS, RHS);
  }

  DeclRefExpr *createDeclRefExpr(SourceLocation Loc, std::string Symbol) {
    return addToContext<DeclRefExpr>(Loc, std::move(Symbol));
  }

  BoolLiteral *createBoolLiteral(SourceLocation Loc, bool Value) {
    return addToContext<BoolLiteral>(Loc, Value);
  }

  CharLiteral *createCharLiteral(SourceLocation Loc, char Value) {
    return addToContext<CharLiteral>(Loc, Value);
  }

  NumLiteral *createNumLiteral(SourceLocation Loc, llvm::APFloat Value) {
    return addToContext<NumLiteral>(Loc, Value);
  }

  StringLiteral *createStringLiteral(SourceLocation Loc, std::string Value) {
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
