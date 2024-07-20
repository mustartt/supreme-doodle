#ifndef AST_NODE_H
#define AST_NODE_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"

namespace rx::ast {

template <typename Derived> class ASTNode {
public:
  template <typename Visitor> void accept(Visitor &visitor) {
    static_cast<Derived *>(this)->acceptImpl(visitor);
  }
};

class PackageDecl;
class ImportDecl;
class Program : public ASTNode<Program> {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

  Program(PackageDecl *PkgDecl, llvm::SmallVectorImpl<ImportDecl *> &&Imports)
      : Package(PkgDecl), Imports(std::move(Imports)) {}

  PackageDecl *getPackage() const { return Package; }
  llvm::ArrayRef<ImportDecl *> getImports() const { return Imports; }

private:
  PackageDecl *Package;
  llvm::SmallVector<ImportDecl *, 8> Imports;
};

class PackageDecl : public ASTNode<PackageDecl> {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

  PackageDecl(std::string Name) : Name(std::move(Name)) {}

private:
  std::string Name;
};

class Declaration : public ASTNode<Declaration> {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }
};

class ImportDecl : public Declaration {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

  ImportDecl(std::string Name) { Path.push_back(std::move(Name)); }

private:
  llvm::SmallVector<std::string, 4> Path;
  std::optional<std::string> Alias;
};

enum class Visibility { Public, Private };

class StructFieldDecl;
class StructDecl : public Declaration {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  llvm::SmallVector<StructFieldDecl *, 8> Fields;
};

class Expression : public Declaration {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }
};

class StructFieldDecl : public ASTNode<StructFieldDecl> {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  Expression *DefaultValue;
};

class VarDecl : public Declaration {

public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  Expression *Initializer;
};

class FuncParamDecl;
class Block;
class FuncDecl : public Declaration {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  Visibility Vis = Visibility::Private;
  std::string Name;
  llvm::SmallVector<FuncParamDecl *, 8> Params;
  Block *Body;
};

class Statement : public ASTNode<Statement> {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }
};

class ReturnStmt : public Statement {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  Expression *Expr;
};

class DeclStmt : public Statement {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  std::string Name;
  Expression *Initializer;
};

class ExprStmt : public Statement {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  Expression *Expr;
};

class IfStmt : public Statement {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  Expression *Condition;
  Block *Body;
  Block *ElseBlock;
};

class ForStmt : public Statement {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  DeclStmt *PreHeader;
  Expression *Condition;
  Expression *PostExpr;
  Block *Body;
};

template <typename Derived, typename Result = void> class ASTVisitor {
public:
  Result visit(Program &node) { static_cast<Derived *>(this)->visitImpl(node); }
  Result visit(PackageDecl &node) {
    static_cast<Derived *>(this)->visitImpl(node);
  }
  Result visit(ImportDecl &node) {
    static_cast<Derived *>(this)->visitImpl(node);
  }
  Result visit(StructDecl &node) {
    static_cast<Derived *>(this)->visitImpl(node);
  }
  Result visit(StructFieldDecl &node) {
    static_cast<Derived *>(this)->visitImpl(node);
  }
  Result visit(VarDecl &node) { static_cast<Derived *>(this)->visitImpl(node); }
  Result visit(FuncDecl &node) {
    static_cast<Derived *>(this)->visitImpl(node);
  }
  Result visit(Expression &node) {
    static_cast<Derived *>(this)->visitImpl(node);
  }
  Result visit(ReturnStmt &node) {
    static_cast<Derived *>(this)->visitImpl(node);
  }
  Result visit(DeclStmt &node) {
    static_cast<Derived *>(this)->visitImpl(node);
  }
  Result visit(ExprStmt &node) {
    static_cast<Derived *>(this)->visitImpl(node);
  }
  Result visit(IfStmt &node) { static_cast<Derived *>(this)->visitImpl(node); }
  Result visit(ForStmt &node) { static_cast<Derived *>(this)->visitImpl(node); }
};

} // namespace rx::ast

#endif // AST_NODE_H
