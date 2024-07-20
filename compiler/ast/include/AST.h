#ifndef AST_NODE_H
#define AST_NODE_H

#include "SrcManager.h"
#include "llvm/ADT/SmallVector.h"
#include <llvm/ADT/ArrayRef.h>

namespace rx::ast {

class BaseDeclVisitor;
class BaseStmtVisitor;
class BaseExprVisitor;

#define ACCEPT_VISITOR(TYPE) virtual void accept(TYPE &visitor);

class ASTNode {
public:
  virtual ~ASTNode() = default;

  std::optional<SrcRange> Loc;
};

class Decl : public ASTNode {
public:
  virtual void accept(BaseDeclVisitor &visitor) = 0;
};

class PackageDecl;
class ImportDecl;
class ProgramDecl : public Decl {
public:
  ProgramDecl(llvm::SmallVectorImpl<Decl *> &&Decls)
      : Decls(std::move(Decls)) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  llvm::ArrayRef<Decl *> getDecls() const { return Decls; }

private:
  llvm::SmallVector<Decl *> Decls;
};

class PackageDecl : public Decl {
public:
  PackageDecl(std::string Name) : Name(std::move(Name)) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  std::string Name;
};

class ImportDecl : public Decl {
public:
  ImportDecl(std::string Name) { Path.push_back(std::move(Name)); }

  ACCEPT_VISITOR(BaseDeclVisitor);

  llvm::ArrayRef<std::string> getPath() const { return Path; }
  std::optional<std::string> getAlias() const { return Alias; }

private:
  llvm::SmallVector<std::string, 4> Path;
  std::optional<std::string> Alias;
};

enum class Visibility { Public, Private };

class StructFieldDecl;
class StructDecl : public Decl {
public:
  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  llvm::SmallVector<StructFieldDecl *, 8> Fields;
};

class Expression;

class StructFieldDecl : public Decl {
public:
  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  Expression *DefaultValue;
};

class VarDecl : public Decl {
public:
  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  Expression *Initializer;
};

class FuncParamDecl;
class BlockStmt;
class FuncDecl : public Decl {
public:
  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  llvm::SmallVector<FuncParamDecl *, 8> Params;
  BlockStmt *Body;
};

class Stmt : public ASTNode {};

class BlockStmt : public Stmt {
public:
  ACCEPT_VISITOR(BaseStmtVisitor);

private:
  llvm::SmallVector<Stmt *, 16> Stmts;
};

class ReturnStmt : public Stmt {
public:
  ACCEPT_VISITOR(BaseStmtVisitor);

private:
  Expression *Expr;
};

class DeclStmt : public Stmt {
public:
  ACCEPT_VISITOR(BaseStmtVisitor);

private:
  std::string Name;
  Expression *Initializer;
};

class ExprStmt : public Stmt {
public:
  ACCEPT_VISITOR(BaseStmtVisitor);

private:
  Expression *Expr;
};

class IfStmt : public Stmt {
public:
  ACCEPT_VISITOR(BaseStmtVisitor);

private:
  Expression *Condition;
  BlockStmt *Body;
  BlockStmt *ElseBlock;
};

class ForStmt : public Stmt {
public:
  ACCEPT_VISITOR(BaseStmtVisitor);

private:
  DeclStmt *PreHeader;
  Expression *Condition;
  Expression *PostExpr;
  BlockStmt *Body;
};

class Expression : public ASTNode {};

enum class BinaryOp {
  Mult,
  Div,
  Add,
  Sub,
  Less,
  Greater,
  LessThanEqual,
  GreaterThanEqual,
  CmpEqual,
  CmpNotEqual,
  Equal
};

enum class UnaryOp { Negative, Not, Ref };

class BinaryExpr : public Expression {
public:
  ACCEPT_VISITOR(BaseExprVisitor);

private:
  BinaryOp Op;
  Expression *LHS;
  Expression *RHS;
};

class UnaryExpr : public Expression {
public:
  ACCEPT_VISITOR(BaseExprVisitor);

private:
  UnaryOp Op;
  Expression *Expr;
};

} // namespace rx::ast

#endif // AST_NODE_H
