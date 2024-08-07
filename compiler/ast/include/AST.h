#ifndef AST_NODE_H
#define AST_NODE_H

#include "SrcManager.h"
#include "llvm/ADT/APFloat.h"
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

class Type : public ASTNode {
public:
  Type(SrcRange Loc) : ASTNode(Loc) {}
};

class ConcreteType : public Type {};

class BasicType : public ConcreteType {};

class UnitType : public BasicType {};
class IntegerType : public BasicType {};
class FloatType : public BasicType {};

class MutableType : public ConcreteType {};

class IdentifierType : public ConcreteType {};
class PointerType : public ConcreteType {};
class ArrayType : public ConcreteType {};
class FunctionType : public ConcreteType {};
class StructType : public ConcreteType {};

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
      : Decl(Loc), Vis(Vis), Name(std::move(Name)), Fields(Fields) {}

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
      : Decl(Loc), Vis(Vis), Name(std::move(Name)), DefaultValue(DefaultValue) {
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
      : Decl(Loc), Vis(Vis), Name(std::move(Name)), Initializer(Initializer) {}

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
           llvm::ArrayRef<FuncParamDecl *> Params, BlockStmt *Body)
      : Decl(Loc), Vis(Vis), Name(std::move(Name)), Params(Params), Body(Body) {
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

class FuncParamDecl : public Decl {
public:
  FuncParamDecl(SrcRange Loc, std::string Name, Expression *DefaultValue)
      : Decl(Loc), Name(std::move(Name)), DefaultValue(DefaultValue) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  const llvm::StringRef getName() const { return Name; }
  Expression *getDefaultValue() const { return DefaultValue; }

private:
  std::string Name;
  Expression *DefaultValue;
};

class Stmt : public ASTNode {
public:
  Stmt(SrcRange Loc) : ASTNode(Loc) {}

  virtual void accept(BaseStmtVisitor &visitor) = 0;
};

class BlockStmt : public Stmt {
public:
  BlockStmt(SrcRange Loc, llvm::ArrayRef<Stmt *> Stmts)
      : Stmt(Loc), Stmts(Stmts) {}

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
  DeclStmt(SrcRange Loc, Decl *Var) : Stmt(Loc), Var(Var) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  Decl *getDecl() const { return Var; }

private:
  Decl *Var;
};

class ExprStmt : public Stmt {
public:
  ExprStmt(SrcRange Loc, Expression *Expr) : Stmt(Loc), Expr(Expr) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  Expression *getExpr() const { return Expr; }

private:
  Expression *Expr;
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

class CallExpr : public Expression {
public:
  CallExpr(SrcRange Loc, Expression *Callee, llvm::ArrayRef<Expression *> Args)
      : Expression(Loc), Callee(Callee), Args(Args.begin(), Args.end()) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  Expression *getCallee() const { return Callee; }
  llvm::ArrayRef<Expression *> getArgs() const { return Args; }

private:
  Expression *Callee;
  llvm::SmallVector<Expression *> Args;
};

class AccessExpr : public Expression {
public:
  AccessExpr(SrcRange Loc, Expression *Expr, std::string Accessor)
      : Expression(Loc), Expr(Expr), Accessor(std::move(Accessor)) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  Expression *getExpr() const { return Expr; }
  llvm::StringRef getAccessor() const { return Accessor; }

private:
  Expression *Expr;
  std::string Accessor;
};

class IndexExpr : public Expression {
public:
  IndexExpr(SrcRange Loc, Expression *Expr, Expression *Idx)
      : Expression(Loc), Expr(Expr), Idx(Idx) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  Expression *getExpr() const { return Expr; }
  Expression *getIdx() const { return Idx; }

private:
  Expression *Expr;
  Expression *Idx;
};

class AssignExpr : public Expression {
public:
  AssignExpr(SrcRange Loc, Expression *LHS, Expression *RHS)
      : Expression(Loc), LHS(LHS), RHS(RHS) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  Expression *getLHS() const { return LHS; }
  Expression *getRHS() const { return RHS; }

private:
  Expression *LHS;
  Expression *RHS;
};

class IdentifierExpr : public Expression {
public:
  IdentifierExpr(SrcRange Loc, std::string Symbol)
      : Expression(Loc), Symbol(std::move(Symbol)) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  llvm::StringRef getSymbol() const { return Symbol; }

private:
  std::string Symbol;
};

class IfExpr : public Expression {
public:
  IfExpr(SrcRange Loc, Expression *Condition, BlockStmt *Body,
         BlockStmt *ElseBlock = nullptr)
      : Expression(Loc), Condition(Condition), Body(Body),
        ElseBlock(ElseBlock) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  Expression *getCondition() const { return Condition; }
  BlockStmt *getBody() const { return Body; }
  BlockStmt *getElseBlock() const { return ElseBlock; }

private:
  Expression *Condition;
  BlockStmt *Body;
  BlockStmt *ElseBlock;
};

class LiteralExpr : public Expression {
public:
  LiteralExpr(SrcRange Loc) : Expression(Loc) {}
};

class BoolLiteral : public LiteralExpr {
public:
  BoolLiteral(SrcRange Loc, bool Value) : LiteralExpr(Loc), Value(Value) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  bool getValue() const { return Value; }

private:
  bool Value;
};

class CharLiteral : public LiteralExpr {
public:
  CharLiteral(SrcRange Loc, char Value) : LiteralExpr(Loc), Value(Value) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  char getValue() const { return Value; }

private:
  char Value;
};

class NumLiteral : public LiteralExpr {
public:
  NumLiteral(SrcRange Loc, llvm::APFloat Value)
      : LiteralExpr(Loc), Value(Value) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  llvm::APFloat getValue() const { return Value; }
  bool isInteger() const { return Value.isInteger(); }

private:
  llvm::APFloat Value;
};

class StringLiteral : public LiteralExpr {
public:
  StringLiteral(SrcRange Loc, std::string Value)
      : LiteralExpr(Loc), Value(std::move(Value)) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  llvm::StringRef getValue() const { return Value; }

private:
  std::string Value;
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
