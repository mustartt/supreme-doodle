#ifndef AST_NODE_H
#define AST_NODE_H

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

  virtual void accept(BaseTypeVisitor &visitor) = 0;
  virtual std::string getTypeName() const = 0;
};

enum class NativeType { Void, i1, i8, i32, i64, f32, f64, String, Unknown };

class BuiltinType : public ASTType {
public:
  BuiltinType(NativeType Builtin)
      : ASTType(SourceLocation::Builtin()), Builtin(Builtin) {}

  std::string getTypeName() const override {
    switch (Builtin) {
    case NativeType::Void:
      return "void";
    case NativeType::i1:
      return "i1";
    case NativeType::i8:
      return "i8";
    case NativeType::i32:
      return "i32";
    case NativeType::i64:
      return "i64";
    case NativeType::f32:
      return "f32";
    case NativeType::f64:
      return "f64";
    case NativeType::String:
      return "string";
    case NativeType::Unknown:
      return "unknown";
    default:
      llvm_unreachable("Invalid BuiltinType");
    }
  }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  NativeType Builtin;
};

class TypeDecl;
class DeclRefType : public ASTType {
public:
  DeclRefType(SourceLocation Loc, std::string Symbol)
      : ASTType(Loc), Symbol(std::move(Symbol)) {
    assert(this->Symbol.size() && "Empty Symbol");
  }

  llvm::StringRef getSymbol() const { return Symbol; }
  std::string getTypeName() const override { return "@" + Symbol; }

  void setReferencedType(ASTType *Type) { this->Type = Type; }
  void setDeclNode(TypeDecl *DeclNode) { this->DeclNode = DeclNode; }

  ACCEPT_VISITOR(BaseTypeVisitor);

private:
  std::string Symbol;
  ASTType *Type;
  TypeDecl *DeclNode;
};

class AccessType : public ASTType {
public:
  AccessType(SourceLocation Loc, std::string Symbol, ASTType *ParentType)
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
  MutableType(SourceLocation Loc, ASTType *ElementType)
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
  PointerType(SourceLocation Loc, ASTType *ElementType, bool Nullable)
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
  ArrayType(SourceLocation Loc, ASTType *ElementType)
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
  FunctionType(SourceLocation Loc, llvm::ArrayRef<ASTType *> ParamTypes,
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
  ObjectType(SourceLocation Loc, llvm::ArrayRef<Field> Fields)
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
  EnumType(SourceLocation Loc, llvm::ArrayRef<Member> Members)
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
  ASTType *getType() const { return Type; }
  void setType(ASTType *Ty) { Type = Ty; }
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

  ACCEPT_VISITOR(BaseDeclVisitor);

  PackageDecl *getPackage() const { return Package; }
  llvm::ArrayRef<ImportDecl *> getImports() const { return Imports; }
  llvm::ArrayRef<ExportedDecl *> getDecls() const { return Decls; }

private:
  PackageDecl *Package;
  llvm::SmallVector<ImportDecl *, 4> Imports;
  llvm::SmallVector<ExportedDecl *> Decls;
};

class PackageDecl : public Decl {
public:
  PackageDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name)
      : Decl(Loc, DeclLoc, std::move(Name), nullptr) {}

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

  ACCEPT_VISITOR(BaseDeclVisitor);

  llvm::StringRef getImportPath() const { return Name; }
  ImportType getImportType() const { return Type; }
  std::optional<std::string> getAlias() const { return Alias; }

private:
  ImportType Type;
  std::optional<std::string> Alias;
};

class TypeDecl : public Decl {
public:
  TypeDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
           ASTType *Type)
      : Decl(Loc, DeclLoc, std::move(Name), Type) {}

  ACCEPT_VISITOR(BaseDeclVisitor);
};

class UseDecl : public TypeDecl {
public:
  UseDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
          ASTType *Type)
      : TypeDecl(Loc, DeclLoc, std::move(Name), Type) {}

  ACCEPT_VISITOR(BaseDeclVisitor);
};

class FuncDecl;

class ImplDecl : public Decl, public ScopedASTNode {
public:
  ImplDecl(SourceLocation Loc, SourceLocation DeclLoc, ASTType *ImplType,
           llvm::ArrayRef<FuncDecl *> Impls)
      : Decl(Loc, DeclLoc, ImplType->getTypeName(), nullptr),
        ImplType(ImplType), Impls(Impls) {
    assert(ImplType);
  }

  llvm::ArrayRef<FuncDecl *> getImpls() const { return Impls; }

  ACCEPT_VISITOR(BaseDeclVisitor);

private:
  ASTType *ImplType;
  llvm::SmallVector<FuncDecl *, 4> Impls;
};

class Expression;
class VarDecl : public Decl {
public:
  VarDecl(SourceLocation Loc, SourceLocation DeclLoc, std::string Name,
          Expression *Initializer = nullptr)
      : Decl(Loc, DeclLoc, std::move(Name), nullptr), Initializer(Initializer) {
  }

  ACCEPT_VISITOR(BaseDeclVisitor);

  Expression *getInitializer() const { return Initializer; }

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

  llvm::ArrayRef<Stmt *> getStmts() const { return Stmts; }

private:
  llvm::SmallVector<Stmt *, 16> Stmts;
};

class ReturnStmt : public Stmt {
public:
  ReturnStmt(SourceLocation Loc, Expression *Expr = nullptr)
      : Stmt(Loc), Expr(Expr) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  Expression *getExpr() const { return Expr; }

private:
  Expression *Expr;
};

class DeclStmt : public Stmt {
public:
  DeclStmt(SourceLocation Loc, Decl *Var) : Stmt(Loc), Var(Var) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

  Decl *getDecl() const { return Var; }

private:
  Decl *Var;
};

class ExprStmt : public Stmt {
public:
  ExprStmt(SourceLocation Loc, Expression *Expr) : Stmt(Loc), Expr(Expr) {}

  ACCEPT_VISITOR(BaseStmtVisitor);

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

  ASTType *getExprType() const { return ExprType; }
  void setExprType(ASTType *Ty) { ExprType = Ty; }

private:
  ASTType *ExprType;
};

class CallExpr : public Expression {
public:
  CallExpr(SourceLocation Loc, Expression *Callee,
           llvm::ArrayRef<Expression *> Args)
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
  AccessExpr(SourceLocation Loc, Expression *Expr, std::string Accessor)
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
  IndexExpr(SourceLocation Loc, Expression *Expr, Expression *Idx)
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
  AssignExpr(SourceLocation Loc, Expression *LHS, Expression *RHS)
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
  DeclRefExpr(SourceLocation Loc, std::string Symbol)
      : Expression(Loc), Symbol(std::move(Symbol)), Ref(nullptr) {}

  ACCEPT_VISITOR(BaseExprVisitor);

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

  llvm::StringMap<Expression *> &getFields() { return Fields; }

private:
  llvm::StringMap<Expression *> Fields;
};

class BoolLiteral : public LiteralExpr {
public:
  BoolLiteral(SourceLocation Loc, bool Value)
      : LiteralExpr(Loc), Value(Value) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  bool getValue() const { return Value; }

private:
  bool Value;
};

class CharLiteral : public LiteralExpr {
public:
  CharLiteral(SourceLocation Loc, char Value)
      : LiteralExpr(Loc), Value(Value) {}

  ACCEPT_VISITOR(BaseExprVisitor);

  char getValue() const { return Value; }

private:
  char Value;
};

class NumLiteral : public LiteralExpr {
public:
  NumLiteral(SourceLocation Loc, llvm::APFloat Value)
      : LiteralExpr(Loc), Value(Value) {}

  ACCEPT_VISITOR(BaseExprVisitor);

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

  UnaryOp getOp() const { return Op; }
  Expression *getExpr() const { return Expr; }

private:
  UnaryOp Op;
  Expression *Expr;
};

} // namespace rx::ast

#endif // AST_NODE_H
