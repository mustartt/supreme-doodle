#ifndef AST_NODE_H
#define AST_NODE_H

#include "SrcManager.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/SmallVector.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>

namespace rx::sema {
class LexicalScope;
}

namespace rx::ast {

class BaseDeclVisitor;
class BaseStmtVisitor;
class BaseExprVisitor;
class BaseTypeVisitor;

#define ACCEPT_VISITOR(TYPE) virtual void accept(TYPE &visitor) override;

class ASTNode {
public:
  ASTNode(SrcRange Loc) : Loc(Loc) {}
  virtual ~ASTNode() = default;
  SrcRange Loc;
};

// Baseclass to represent ast nodes that creates a lexical scope
class ScopedASTNode {
public:
  ScopedASTNode() : Scope(nullptr) {}
  virtual ~ScopedASTNode() = default;

public:
  sema::LexicalScope *getLexicalScope() { return Scope; }
  sema::LexicalScope *getLexicalScope() const { return Scope; }

  void setLexicalScope(sema::LexicalScope *Scope) { this->Scope = Scope; }

private:
  sema::LexicalScope *Scope;
};

class ASTType : public ASTNode {
public:
  ASTType(SrcRange Loc) : ASTNode(Loc) {}

  virtual void accept(BaseTypeVisitor &visitor) = 0;
  virtual std::string getTypeName() const = 0;
};

class DeclRefType : public ASTType {
public:
  DeclRefType(SrcRange Loc, std::string Symbol)
      : ASTType(Loc), Symbol(std::move(Symbol)) {
    assert(this->Symbol.size() && "Empty Symbol");
  }

  std::string getTypeName() const override { return Symbol; }

  void setReferencedType(ASTType *Type) { this->Type = Type; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  std::string Symbol;
  ASTType *Type;
};

class AccessType : public ASTType {
public:
  AccessType(SrcRange Loc, std::string Symbol, ASTType *ParentType)
      : ASTType(Loc), Symbol(std::move(Symbol)), ParentType(ParentType) {
    assert(this->Symbol.size() && "Empty Symbol");
    assert(this->ParentType && "Must have parent");
  }

  std::string getTypeName() const override {
    assert(ParentType && "Must have parent");
    return ParentType->getTypeName() + "." + Symbol;
  }

  ASTType *getParentType() const { return ParentType; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  std::string Symbol;
  ASTType *ParentType;
};

class MutableType : public ASTType {
public:
  MutableType(SrcRange Loc, ASTType *ElementType)
      : ASTType(Loc), ElementType(ElementType) {}

  std::string getTypeName() const override {
    return "mut " + ElementType->getTypeName();
  }

  ASTType *getElementType() const { return ElementType; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  ASTType *ElementType;
};

class PointerType : public ASTType {
public:
  PointerType(SrcRange Loc, ASTType *ElementType, bool Nullable)
      : ASTType(Loc), ElementType(ElementType), Nullable(Nullable) {}

  std::string getTypeName() const override {
    std::string Ty = "*";
    if (Nullable) {
      Ty += "nullable ";
    }
    return Ty + ElementType->getTypeName();
  }

  ASTType *getElementType() const { return ElementType; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  ASTType *ElementType;
  bool Nullable;
};

class ArrayType : public ASTType {
public:
  ArrayType(SrcRange Loc, ASTType *ElementType)
      : ASTType(Loc), ElementType(ElementType) {}

  std::string getTypeName() const override {
    std::string Ty("[");
    Ty += ElementType->getTypeName();
    Ty += "]";
    return Ty;
  }

  ASTType *getElementType() const { return ElementType; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  ASTType *ElementType;
};

class FunctionType : public ASTType {
public:
  FunctionType(SrcRange Loc, llvm::ArrayRef<ASTType *> ParamTypes,
               ASTType *ReturnType)
      : ASTType(Loc), ParamTypes(ParamTypes), ReturnType(ReturnType) {}

  std::string getTypeName() const override {
    std::string Ty("func(");
    for (const auto [Idx, P] : llvm::enumerate(ParamTypes)) {
      Ty += P->getTypeName();
      if (Idx + 1 != ParamTypes.size())
        Ty += ", ";
    }
    Ty += ") ";
    Ty += ReturnType->getTypeName();
    return Ty;
  }

  llvm::ArrayRef<ASTType *> getParamTypes() const { return ParamTypes; }
  ASTType *getReturnType() const { return ReturnType; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  llvm::SmallVector<ASTType *, 4> ParamTypes;
  ASTType *ReturnType;
};

class ObjectType : public ASTType {
public:
  using Field = std::pair<std::string, ASTType *>;

public:
  ObjectType(SrcRange Loc, llvm::ArrayRef<Field> Fields)
      : ASTType(Loc), Fields(Fields) {}

  std::string getTypeName() const override {
    std::string Ty("{ ");
    for (const auto &[Idx, Field] : llvm::enumerate(Fields)) {
      const auto &[Name, FT] = Field;
      Ty += Name;
      Ty += ": ";
      Ty += FT->getTypeName();
      if (Idx + 1 != Fields.size())
        Ty += ", ";
    }
    Ty += " }";
    return Ty;
  }

  llvm::ArrayRef<Field> getFields() const { return Fields; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  llvm::SmallVector<Field> Fields;
};

class EnumType : public ASTType {
public:
  using Member = std::pair<std::string, ASTType *>;

public:
  EnumType(SrcRange Loc, llvm::ArrayRef<Member> Members)
      : ASTType(Loc), Members(Members) {}

  std::string getTypeName() const override {
    std::string Ty("enum { ");
    for (const auto &[Idx, Field] : llvm::enumerate(Members)) {
      const auto &[Name, FT] = Field;
      Ty += Name;
      if (FT) {
        Ty += ": ";
        Ty += FT->getTypeName();
      }
      if (Idx + 1 != Members.size())
        Ty += ", ";
    }
    Ty += " }";
    return Ty;
  }

  llvm::ArrayRef<Member> getMembers() const { return Members; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  llvm::SmallVector<Member> Members;
};

class Decl : public ASTNode {
public:
  Decl(SrcRange Loc, SrcRange DeclLoc, std::string Name,
       ASTType *Type = nullptr)
      : ASTNode(Loc), Name(std::move(Name)), Type(Type), DeclLoc(DeclLoc) {}

  virtual void accept(BaseDeclVisitor &visitor) = 0;

  llvm::StringRef getName() const { return Name; }
  ASTType *getType() const { return Type; }
  void setType(ASTType *Ty) { Type = Ty; }
  const SrcRange &getDeclLoc() const { return DeclLoc; }

protected:
  std::string Name;
  ASTType *Type;
  SrcRange DeclLoc;
};

class PackageDecl;
class ImportDecl;
class ProgramDecl : public Decl, public ScopedASTNode {
public:
  ProgramDecl(SrcRange Loc, PackageDecl *Package,
              llvm::ArrayRef<ImportDecl *> Imports,
              llvm::ArrayRef<Decl *> Decls)
      : Decl(Loc, Loc, "Program"), Package(Package), Imports(Imports),
        Decls(std::move(Decls)) {}

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
  PackageDecl(SrcRange Loc, SrcRange DeclLoc, std::string Name)
      : Decl(Loc, DeclLoc, std::move(Name)) {}

  ACCEPT_VISITOR(BaseDeclVisitor);
};

class ImportDecl : public Decl {
public:
  enum class ImportType { File, Module };

public:
  ImportDecl(SrcRange Loc, SrcRange DeclLoc, ImportType Type, std::string Path,
             std::optional<std::string> Alias = std::nullopt)
      : Decl(Loc, DeclLoc, std::move(Path)), Alias(std::move(Alias)) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  llvm::StringRef getImportPath() const { return Name; }
  std::optional<std::string> getAlias() const { return Alias; }

private:
  ImportType Type;
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

class TypeDecl : public Decl {
public:
  TypeDecl(SrcRange Loc, SrcRange DeclLoc, std::string Name, Visibility Vis,
           ASTType *Type)
      : Decl(Loc, DeclLoc, std::move(Name), Type) {}

  Visibility getVisibility() const { return Vis; }

  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  Visibility Vis = Visibility::Private;
};

class UseDecl : public Decl {
public:
  UseDecl(SrcRange Loc, SrcRange DeclLoc, std::string Name, Visibility Vis,
          ASTType *Type)
      : Decl(Loc, DeclLoc, std::move(Name), Type) {}

  Visibility getVisibility() const { return Vis; }

  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  Visibility Vis = Visibility::Private;
};

class FuncDecl;

class ImplDecl : public Decl, public ScopedASTNode {
public:
  ImplDecl(SrcRange Loc, SrcRange DeclLoc, ASTType *ImplType, Visibility Vis,
           llvm::ArrayRef<FuncDecl *> Impls)
      : Decl(Loc, DeclLoc, ImplType->getTypeName()), ImplType(ImplType),
        Impls(Impls), Vis(Vis) {
    assert(ImplType);
  }

  Visibility getVisibility() const { return Vis; }
  llvm::ArrayRef<FuncDecl *> getImpls() const { return Impls; }

  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  ASTType *ImplType;
  llvm::SmallVector<FuncDecl *, 4> Impls;
  Visibility Vis;
};
class Expression;
class VarDecl : public Decl {
public:
  VarDecl(SrcRange Loc, SrcRange DeclLoc, std::string Name, Visibility Vis,
          Expression *Initializer = nullptr)
      : Decl(Loc, DeclLoc, std::move(Name)), Vis(Vis),
        Initializer(Initializer) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  Visibility getVisibility() const { return Vis; }
  Expression *getInitializer() const { return Initializer; }

private:
  Visibility Vis = Visibility::Private;
  Expression *Initializer;
};

class FuncParamDecl;
class BlockStmt;
class FuncDecl : public Decl, public ScopedASTNode {
public:
  FuncDecl(SrcRange Loc, SrcRange DeclLoc, std::string Name, Visibility Vis,
           llvm::ArrayRef<FuncParamDecl *> Params, BlockStmt *Body)
      : Decl(Loc, DeclLoc, std::move(Name)), Vis(Vis), Params(Params),
        Body(Body) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  const llvm::StringRef getName() const { return Name; }
  Visibility getVisibility() const { return Vis; }
  llvm::ArrayRef<FuncParamDecl *> getParams() const { return Params; }
  BlockStmt *getBody() const { return Body; }

private:
  Visibility Vis = Visibility::Private;
  llvm::SmallVector<FuncParamDecl *, 8> Params;
  BlockStmt *Body;
};

class FuncParamDecl : public Decl {
public:
  FuncParamDecl(SrcRange Loc, SrcRange DeclLoc, std::string Name,
                Expression *DefaultValue)
      : Decl(Loc, DeclLoc, std::move(Name)), DefaultValue(DefaultValue) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  Expression *getDefaultValue() const { return DefaultValue; }

private:
  Expression *DefaultValue;
};

class Stmt : public ASTNode {
public:
  Stmt(SrcRange Loc) : ASTNode(Loc) {}

  virtual void accept(BaseStmtVisitor &visitor) = 0;
};

class BlockStmt : public Stmt, public ScopedASTNode {
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

class DeclRefExpr : public Expression {
public:
  DeclRefExpr(SrcRange Loc, std::string Symbol)
      : Expression(Loc), Symbol(std::move(Symbol)), Ref(nullptr) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  llvm::StringRef getSymbol() const { return Symbol; }

private:
  std::string Symbol;
  Decl *Ref;
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
