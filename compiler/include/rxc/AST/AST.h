#ifndef AST_NODE_H
#define AST_NODE_H

#include "QualType.h"
#include "rxc/AST/TypeContext.h"
#include "rxc/Basic/SourceManager.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/SmallVector.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorHandling.h>

namespace rx::sema {
class LexicalScope;
}

namespace rx::ast {

class BaseDeclVisitor;
class BaseStmtVisitor;
class BaseExprVisitor;
class BaseTypeVisitor;

#define ACCEPT_VISITOR(TYPE) virtual void accept(TYPE &visitor) override;

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

class ASTNode {
public:
  ASTNode(SourceLocation Loc) : Loc(Loc) {}
  virtual ~ASTNode() = default;

public:
  virtual std::string name() const = 0;

public:
  SourceLocation Loc;
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
  ASTType(SourceLocation Loc) : ASTNode(Loc) {}

  std::string name() const override { return "type"; }

  virtual void accept(BaseTypeVisitor &visitor) = 0;
  virtual std::string getTypeName() const = 0;

  QualType getType() const { return Type; }
  void setType(QualType Ty) { Type = Ty; }

private:
  QualType Type;
};

enum class ASTNativeType { Void, i1, i8, i32, i64, f32, f64, String, Unknown };

class ASTBuiltinType : public ASTType {
public:
  ASTBuiltinType(ASTNativeType Builtin)
      : ASTType(SourceLocation::Builtin()), Builtin(Builtin) {}

  std::string getTypeName() const override {
    switch (Builtin) {
    case ASTNativeType::Void:
      return "void";
    case ASTNativeType::i1:
      return "i1";
    case ASTNativeType::i8:
      return "i8";
    case ASTNativeType::i32:
      return "i32";
    case ASTNativeType::i64:
      return "i64";
    case ASTNativeType::f32:
      return "f32";
    case ASTNativeType::f64:
      return "f64";
    case ASTNativeType::String:
      return "string";
    case ASTNativeType::Unknown:
      return "unknown";
    default:
      llvm_unreachable("Invalid BuiltinType");
    }
  }
  ASTNativeType getNativeType() const { return Builtin; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  ASTNativeType Builtin;
};

class TypeDecl;
class ASTDeclTypeRef : public ASTType {
public:
  ASTDeclTypeRef(SourceLocation Loc, std::string Symbol)
      : ASTType(Loc), Symbol(std::move(Symbol)) {
    assert(this->Symbol.size() && "Empty Symbol");
  }

  llvm::StringRef getSymbol() const { return Symbol; }
  std::string getTypeName() const override { return "@" + Symbol; }

  TypeDecl *getDeclNode() const { return DeclNode; }
  void setDeclNode(TypeDecl *DeclNode) { this->DeclNode = DeclNode; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  std::string Symbol;
  TypeDecl *DeclNode;
};

class ASTAccessType : public ASTType {
public:
  ASTAccessType(SourceLocation Loc, std::string Symbol, ASTType *ParentType)
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

class ASTQualType : public ASTType {
public:
  ASTQualType(SourceLocation Loc, ASTType *ElementType)
      : ASTType(Loc), ElementType(ElementType) {}

  std::string getTypeName() const override {
    return "<qual> " + ElementType->getTypeName();
  }

  ASTType *getElementType() const { return ElementType; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  ASTType *ElementType;
};

class ASTPointerType : public ASTType {
public:
  ASTPointerType(SourceLocation Loc, ASTType *ElementType, bool Nullable)
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

class ASTArrayType : public ASTType {
public:
  ASTArrayType(SourceLocation Loc, ASTType *ElementType)
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

class ASTFunctionType : public ASTType {
public:
  ASTFunctionType(SourceLocation Loc, llvm::ArrayRef<ASTType *> ParamTypes,
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

class ASTObjectType : public ASTType {
public:
  using Field = std::pair<std::string, ASTType *>;

public:
  ASTObjectType(SourceLocation Loc, llvm::ArrayRef<Field> Fields)
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

class ASTEnumType : public ASTType {
public:
  using Member = std::pair<std::string, ASTType *>;

public:
  ASTEnumType(SourceLocation Loc, llvm::ArrayRef<Member> Members)
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
  Decl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
       ASTType *Type)
      : ASTNode(Loc), Name(std::move(Name)), Type(Type), DeclLoc(DeclLoc) {}

  virtual void accept(BaseDeclVisitor &visitor) = 0;

  llvm::StringRef getName() const { return Name; }
  void setName(std::string Name) { this->Name = std::move(Name); }
  ASTType *getDeclaredType() const { return Type; }
  void setDeclaredType(ASTType *Ty) { Type = Ty; }
  const SourceLocation &getDeclLoc() const { return DeclLoc; }

protected:
  std::string Name;
  ASTType *Type;
  SourceLocation DeclLoc;
};

class ExportedDecl : public Decl {
public:
  ExportedDecl(SourceLocation Loc, SourceLocation DeclLoc, Decl *Exported,
               Visibility Vis)
      : Decl(Loc, DeclLoc, "", nullptr), Exported(Exported), Vis(Vis) {}

  std::string name() const override { return "ExportedDecl"; }
  Decl *getExportedDecl() const { return Exported; }
  Visibility getVisibility() const { return Vis; }

  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  Decl *Exported;
  Visibility Vis;
};

class PackageDecl;
class ImportDecl;
class ProgramDecl : public Decl, public ScopedASTNode {
public:
  ProgramDecl(SourceLocation Loc, PackageDecl *Package,
              llvm::ArrayRef<ImportDecl *> Imports,
              llvm::ArrayRef<ExportedDecl *> Decls)
      : Decl(Loc, Loc, "Program", nullptr), Package(Package), Imports(Imports),
        Decls(std::move(Decls)) {}

  std::string name() const override { return "ProgramDecl"; }
  PackageDecl *getPackage() const { return Package; }
  llvm::ArrayRef<ImportDecl *> getImports() const { return Imports; }
  llvm::ArrayRef<ExportedDecl *> getDecls() const { return Decls; }

  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  PackageDecl *Package;
  llvm::SmallVector<ImportDecl *, 4> Imports;
  llvm::SmallVector<ExportedDecl *> Decls;
};

class PackageDecl : public Decl {
public:
  PackageDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name)
      : Decl(Loc, DeclLoc, std::move(Name), nullptr) {}

  std::string name() const override { return "PackageDecl"; }

  ACCEPT_VISITOR(BaseDeclVisitor);
};

class ImportDecl : public Decl {
public:
  enum class ImportType { File, Module };

public:
  ImportDecl(SourceLocation Loc, SourceLocation DeclLoc, ImportType Type,
             std::string Path, std::optional<std::string> Alias = std::nullopt)
      : Decl(Loc, DeclLoc, std::move(Path), nullptr), Type(Type),
        Alias(std::move(Alias)) {}

  std::string name() const override { return "ImportDecl"; }
  llvm::StringRef getImportPath() const { return Name; }
  ImportType getImportType() const { return Type; }
  std::optional<std::string> getAlias() const { return Alias; }

  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  ImportType Type;
  std::optional<std::string> Alias;
};

class TypeDecl : public Decl {
public:
  TypeDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
           ASTType *Type)
      : Decl(Loc, DeclLoc, std::move(Name), Type) {}

  std::string name() const override { return "TypeDecl"; }

  ACCEPT_VISITOR(BaseDeclVisitor);
};

class UseDecl : public TypeDecl {
public:
  UseDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
          ASTType *Type)
      : TypeDecl(Loc, DeclLoc, std::move(Name), Type) {}

  std::string name() const override { return "UseDecl"; }

  ACCEPT_VISITOR(BaseDeclVisitor);
};

class FuncDecl;

class ImplDecl : public Decl, public ScopedASTNode {
public:
  ImplDecl(SourceLocation Loc, SourceLocation DeclLoc, ASTType *ImplType,
           llvm::ArrayRef<FuncDecl *> Impls)
      : Decl(Loc, DeclLoc, ImplType->getTypeName(), ImplType), Impls(Impls) {
    assert(ImplType);
  }

  std::string name() const override { return "ImplDecl"; }
  llvm::ArrayRef<FuncDecl *> getImpls() const { return Impls; }

  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  llvm::SmallVector<FuncDecl *, 4> Impls;
};

class Expression;
class VarDecl : public Decl {
public:
  VarDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
          Expression *Initializer = nullptr)
      : Decl(Loc, DeclLoc, std::move(Name), nullptr), Initializer(Initializer) {
  }

  std::string name() const override { return "VarDecl"; }
  Expression *getInitializer() const { return Initializer; }

  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  Expression *Initializer;
};

class FuncParamDecl;
class BlockStmt;
class FuncDecl : public Decl, public ScopedASTNode {
public:
  FuncDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
           llvm::ArrayRef<FuncParamDecl *> Params, BlockStmt *Body)
      : Decl(Loc, DeclLoc, std::move(Name), nullptr), Params(Params),
        Body(Body) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  std::string name() const override { return "FuncDecl"; }
  const llvm::StringRef getName() const { return Name; }
  llvm::ArrayRef<FuncParamDecl *> getParams() const { return Params; }
  BlockStmt *getBody() const { return Body; }

private:
  llvm::SmallVector<FuncParamDecl *, 8> Params;
  BlockStmt *Body;
};

class FuncParamDecl : public Decl {
public:
  FuncParamDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
                Expression *DefaultValue)
      : Decl(Loc, DeclLoc, std::move(Name), nullptr),
        DefaultValue(DefaultValue) {}

  ACCEPT_VISITOR(BaseDeclVisitor);

  std::string name() const override { return "FuncParamDecl"; }
  Expression *getDefaultValue() const { return DefaultValue; }

private:
  Expression *DefaultValue;
};

class Stmt : public ASTNode {
public:
  Stmt(SourceLocation Loc) : ASTNode(Loc) {}

  virtual void accept(BaseStmtVisitor &visitor) = 0;
};

class BlockStmt : public Stmt, public ScopedASTNode {
public:
  BlockStmt(SourceLocation Loc, llvm::ArrayRef<Stmt *> Stmts)
      : Stmt(Loc), Stmts(Stmts) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  std::string name() const override { return "BlockStmt"; }
  llvm::ArrayRef<Stmt *> getStmts() const { return Stmts; }

private:
  llvm::SmallVector<Stmt *, 16> Stmts;
};

class ReturnStmt : public Stmt {
public:
  ReturnStmt(SourceLocation Loc, Expression *Expr = nullptr)
      : Stmt(Loc), Expr(Expr) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  std::string name() const override { return "ReturnStmt"; }
  Expression *getExpr() const { return Expr; }

private:
  Expression *Expr;
};

class DeclStmt : public Stmt {
public:
  DeclStmt(SourceLocation Loc, Decl *Var) : Stmt(Loc), Var(Var) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  std::string name() const override { return "DeclStmt"; }
  Decl *getDecl() const { return Var; }

private:
  Decl *Var;
};

class ExprStmt : public Stmt {
public:
  ExprStmt(SourceLocation Loc, Expression *Expr) : Stmt(Loc), Expr(Expr) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  std::string name() const override { return "ExprStmt"; }
  Expression *getExpr() const { return Expr; }

private:
  Expression *Expr;
};

class ForStmt : public Stmt {
public:
  ForStmt(SourceLocation Loc, DeclStmt *PreHeader, Expression *Condition,
          Expression *PostExpr, BlockStmt *Body)
      : Stmt(Loc), PreHeader(PreHeader), Condition(Condition),
        PostExpr(PostExpr), Body(Body) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  std::string name() const override { return "ForStmt"; }
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
  Expression(SourceLocation Loc) : ASTNode(Loc), ExprType(nullptr) {}

  virtual void accept(BaseExprVisitor &) = 0;

  QualType getExprType() const { return ExprType; }
  void setExprType(QualType Ty) { ExprType = Ty; }

private:
  QualType ExprType;
};

class CallExpr : public Expression {
public:
  CallExpr(SourceLocation Loc, Expression *Callee,
           llvm::ArrayRef<Expression *> Args)
      : Expression(Loc), Callee(Callee), Args(Args.begin(), Args.end()) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "CallExpr"; }
  Expression *getCallee() const { return Callee; }
  llvm::ArrayRef<Expression *> getArgs() const { return Args; }

private:
  Expression *Callee;
  llvm::SmallVector<Expression *> Args;
};

class AccessExpr : public Expression {
public:
  AccessExpr(SourceLocation Loc, Expression *Expr, std::string Accessor)
      : Expression(Loc), Expr(Expr), Accessor(std::move(Accessor)) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "AccessExpr"; }
  Expression *getExpr() const { return Expr; }
  llvm::StringRef getAccessor() const { return Accessor; }

private:
  Expression *Expr;
  std::string Accessor;
};

class IndexExpr : public Expression {
public:
  IndexExpr(SourceLocation Loc, Expression *Expr, Expression *Idx)
      : Expression(Loc), Expr(Expr), Idx(Idx) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "IndexExpr"; }
  Expression *getExpr() const { return Expr; }
  Expression *getIdx() const { return Idx; }

private:
  Expression *Expr;
  Expression *Idx;
};

class AssignExpr : public Expression {
public:
  AssignExpr(SourceLocation Loc, Expression *LHS, Expression *RHS)
      : Expression(Loc), LHS(LHS), RHS(RHS) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "AssignExpr"; }
  Expression *getLHS() const { return LHS; }
  Expression *getRHS() const { return RHS; }

private:
  Expression *LHS;
  Expression *RHS;
};

class DeclRefExpr : public Expression {
public:
  DeclRefExpr(SourceLocation Loc, std::string Symbol)
      : Expression(Loc), Symbol(std::move(Symbol)), Ref(nullptr) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "DeclRefExpr"; }
  llvm::StringRef getSymbol() const { return Symbol; }
  Decl *getRefDecl() const { return Ref; }
  void setRefDecl(Decl *D) { Ref = D; }

private:
  std::string Symbol;
  Decl *Ref;
};

class IfExpr : public Expression {
public:
  IfExpr(SourceLocation Loc, Expression *Condition, BlockStmt *Body,
         BlockStmt *ElseBlock = nullptr)
      : Expression(Loc), Condition(Condition), Body(Body),
        ElseBlock(ElseBlock) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "IfExpr"; }
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
  LiteralExpr(SourceLocation Loc) : Expression(Loc) {}
};

class ObjectLiteral : public LiteralExpr {
public:
  ObjectLiteral(SourceLocation Loc, llvm::StringMap<Expression *> Fields)
      : LiteralExpr(Loc), Fields(Fields) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "ObjectLiteral"; }
  llvm::StringMap<Expression *> &getFields() { return Fields; }

private:
  llvm::StringMap<Expression *> Fields;
};

class BoolLiteral : public LiteralExpr {
public:
  BoolLiteral(SourceLocation Loc, bool Value)
      : LiteralExpr(Loc), Value(Value) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "BoolLiteral"; }
  bool getValue() const { return Value; }

private:
  bool Value;
};

class CharLiteral : public LiteralExpr {
public:
  CharLiteral(SourceLocation Loc, char Value)
      : LiteralExpr(Loc), Value(Value) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "CharLiteral"; }
  char getValue() const { return Value; }

private:
  char Value;
};

class NumLiteral : public LiteralExpr {
public:
  NumLiteral(SourceLocation Loc, llvm::APFloat Value)
      : LiteralExpr(Loc), Value(Value) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "NumLiteral"; }
  llvm::APFloat getValue() const { return Value; }
  bool isInteger() const { return Value.isInteger(); }

private:
  llvm::APFloat Value;
};

class StringLiteral : public LiteralExpr {
public:
  StringLiteral(SourceLocation Loc, std::string Value)
      : LiteralExpr(Loc), Value(std::move(Value)) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "StringLiteral"; }
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
  BinaryExpr(SourceLocation Loc, BinaryOp Op, Expression *LHS, Expression *RHS)
      : Expression(Loc), Op(Op), LHS(LHS), RHS(RHS) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "BinaryExpr"; }
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
  UnaryExpr(SourceLocation Loc, UnaryOp Op, Expression *Expr)
      : Expression(Loc), Op(Op), Expr(Expr) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  std::string name() const override { return "UnaryExpr"; }
  UnaryOp getOp() const { return Op; }
  Expression *getExpr() const { return Expr; }

private:
  UnaryOp Op;
  Expression *Expr;
};

} // namespace rx::ast

#endif // AST_NODE_H
