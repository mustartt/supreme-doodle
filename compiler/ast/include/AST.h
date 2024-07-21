#ifndef AST_NODE_H
#define AST_NODE_H

#include "SrcManager.h"
#include "llvm/ADT/SmallVector.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>

namespace rx::ast {

class BaseDeclVisitor;
class BaseStmtVisitor;
class BaseExprVisitor;

#define ACCEPT_VISITOR(TYPE) virtual void accept(TYPE &visitor);

class ASTNode {
public:
  ASTNode(SrcRange Loc) : Loc(Loc) {}
  virtual ~ASTNode() = default;
  SrcRange Loc;
};

class Decl : public ASTNode {
public:
  Decl(SrcRange Loc) : ASTNode(Loc) {}

  virtual void accept(BaseDeclVisitor &visitor) = 0;
};

class PackageDecl;
class ImportDecl;
class ProgramDecl : public Decl {
public:
  ProgramDecl(SrcRange Loc, PackageDecl *Package,
              llvm::ArrayRef<ImportDecl *> Imports,
              llvm::ArrayRef<Decl *> Decls)
      : Decl(Loc), Package(Package), Imports(Imports), Decls(std::move(Decls)) {
  }

  ACCEPT_VISITOR(BaseDeclVisitor);

  PackageDecl *getPackage() const { return Package; }
  llvm::ArrayRef<ImportDecl *> getImports() const { return Imports; }
  llvm::ArrayRef<Decl *> getDecls() const { return Decls; }

private:
  PackageDecl *Package;
  llvm::SmallVector<ImportDecl *, 4> Imports;
  llvm::SmallVector<Decl *> Decls;
};

class PackageDecl : public Decl {
public:
  PackageDecl(SrcRange Loc, std::string Name)
      : Decl(Loc), Name(std::move(Name)) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  const std::string &getName() const { return Name; }

private:
  std::string Name;
};

class ImportDecl : public Decl {
public:
  ImportDecl(SrcRange Loc, llvm::ArrayRef<std::string> Path,
             std::optional<std::string> Alias = std::nullopt)
      : Decl(Loc), Path(Path), Alias(std::move(Alias)) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  llvm::ArrayRef<std::string> getPath() const { return Path; }
  std::optional<std::string> getAlias() const { return Alias; }

private:
  llvm::SmallVector<std::string, 4> Path;
  std::optional<std::string> Alias;
};

enum class Visibility { Public, Private };

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &Os, Visibility Vis) {
  switch (Vis) {
  case Visibility::Public:
    Os << "Public";
    break;
  case Visibility::Private:
    Os << "Private";
    break;
  }
  return Os;
}

class FieldDecl;
class StructDecl : public Decl {
public:
  StructDecl(SrcRange Loc, std::string Name, Visibility Vis,
             llvm::ArrayRef<FieldDecl *> Fields)
      : Decl(Loc), Name(std::move(Name)), Vis(Vis), Fields(Fields) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  const llvm::StringRef getName() const { return Name; }
  Visibility getVisibility() const { return Vis; }
  llvm::ArrayRef<FieldDecl *> getFields() const { return Fields; }

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  llvm::SmallVector<FieldDecl *, 8> Fields;
};

class Expression;

class FieldDecl : public Decl {
public:
  FieldDecl(SrcRange Loc, std::string Name, Visibility Vis,
            Expression *DefaultValue = nullptr)
      : Decl(Loc), Name(std::move(Name)), Vis(Vis), DefaultValue(DefaultValue) {
  }

  ACCEPT_VISITOR(BaseDeclVisitor);

  const llvm::StringRef getName() const { return Name; }
  Visibility getVisibility() const { return Vis; }
  Expression *getDefaultValue() const { return DefaultValue; }

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  Expression *DefaultValue;
};

class VarDecl : public Decl {
public:
  VarDecl(SrcRange Loc, std::string Name, Visibility Vis,
          Expression *Initializer = nullptr)
      : Decl(Loc), Name(std::move(Name)), Vis(Vis), Initializer(Initializer) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  const llvm::StringRef getName() const { return Name; }
  Visibility getVisibility() const { return Vis; }
  Expression *getInitializer() const { return Initializer; }

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  Expression *Initializer;
};

class FuncParamDecl;
class BlockStmt;
class FuncDecl : public Decl {
public:
  FuncDecl(SrcRange Loc, std::string Name, Visibility Vis,
           llvm::ArrayRef<FuncParamDecl *> Params, BlockStmt *Body = nullptr)
      : Decl(Loc), Name(std::move(Name)), Vis(Vis), Params(Params), Body(Body) {
  }

  ACCEPT_VISITOR(BaseDeclVisitor);

  const llvm::StringRef getName() const { return Name; }
  Visibility getVisibility() const { return Vis; }
  llvm::ArrayRef<FuncParamDecl *> getParams() const { return Params; }
  BlockStmt *getBody() const { return Body; }

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  llvm::SmallVector<FuncParamDecl *, 8> Params;
  BlockStmt *Body;
};

class Stmt : public ASTNode {
public:
  Stmt(SrcRange Loc) : ASTNode(Loc) {}

  virtual void accept(BaseStmtVisitor &visitor) = 0;
};

class BlockStmt : public Stmt {
public:
  BlockStmt(SrcRange Loc, llvm::SmallVectorImpl<Stmt *> &&Stmts)
      : Stmt(Loc), Stmts(std::move(Stmts)) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  llvm::ArrayRef<Stmt *> getStmts() const { return Stmts; }

private:
  llvm::SmallVector<Stmt *, 16> Stmts;
};

class ReturnStmt : public Stmt {
public:
  ReturnStmt(SrcRange Loc, Expression *Expr = nullptr)
      : Stmt(Loc), Expr(Expr) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  Expression *getExpr() const { return Expr; }

private:
  Expression *Expr;
};

class DeclStmt : public Stmt {
public:
  DeclStmt(SrcRange Loc, std::string Name, Expression *Initializer = nullptr)
      : Stmt(Loc), Name(std::move(Name)), Initializer(Initializer) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  const llvm::StringRef getName() const { return Name; }
  Expression *getInitializer() const { return Initializer; }

private:
  std::string Name;
  Expression *Initializer;
};

class ExprStmt : public Stmt {
public:
  ExprStmt(SrcRange Loc, Expression *Expr) : Stmt(Loc), Expr(Expr) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  Expression *getExpr() const { return Expr; }

private:
  Expression *Expr;
};

class IfStmt : public Stmt {
public:
  IfStmt(SrcRange Loc, Expression *Condition, BlockStmt *Body,
         BlockStmt *ElseBlock = nullptr)
      : Stmt(Loc), Condition(Condition), Body(Body), ElseBlock(ElseBlock) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  Expression *getCondition() const { return Condition; }
  BlockStmt *getBody() const { return Body; }
  BlockStmt *getElseBlock() const { return ElseBlock; }

private:
  Expression *Condition;
  BlockStmt *Body;
  BlockStmt *ElseBlock;
};

class ForStmt : public Stmt {
public:
  ForStmt(SrcRange Loc, DeclStmt *PreHeader, Expression *Condition,
          Expression *PostExpr, BlockStmt *Body)
      : Stmt(Loc), PreHeader(PreHeader), Condition(Condition),
        PostExpr(PostExpr), Body(Body) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  DeclStmt *getPreHeader() const { return PreHeader; }
  Expression *getCondition() const { return Condition; }
  Expression *getPostExpr() const { return PostExpr; }
  BlockStmt *getBody() const { return Body; }

private:
  DeclStmt *PreHeader;
  Expression *Condition;
  Expression *PostExpr;
  BlockStmt *Body;
};

class Expression : public ASTNode {
public:
  Expression(SrcRange Loc) : ASTNode(Loc) {}

  virtual void accept(BaseExprVisitor &) = 0;
};

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
  BinaryExpr(SrcRange Loc, BinaryOp Op, Expression *LHS, Expression *RHS)
      : Expression(Loc), Op(Op), LHS(LHS), RHS(RHS) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  BinaryOp getOp() const { return Op; }
  Expression *getLHS() const { return LHS; }
  Expression *getRHS() const { return RHS; }

private:
  BinaryOp Op;
  Expression *LHS;
  Expression *RHS;
};

class UnaryExpr : public Expression {
public:
  UnaryExpr(SrcRange Loc, UnaryOp Op, Expression *Expr)
      : Expression(Loc), Op(Op), Expr(Expr) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  UnaryOp getOp() const { return Op; }
  Expression *getExpr() const { return Expr; }

private:
  UnaryOp Op;
  Expression *Expr;
};

} // namespace rx::ast

#endif // AST_NODE_H
