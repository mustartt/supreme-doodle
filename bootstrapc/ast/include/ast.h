#ifndef AST_NODE_H
#define AST_NODE_H

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

private:
  PackageDecl *package;
  llvm::SmallVector<ImportDecl *> imports;
};

class PackageDecl : public ASTNode<PackageDecl> {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  std::string name;
};

class ImportDecl : public ASTNode<ImportDecl> {
public:
  template <typename Visitor> void acceptImpl(Visitor &visitor) {
    visitor.visit(*this);
  }

private:
  llvm::SmallVector<std::string, 4> path;
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
};

} // namespace rx::ast

#endif // AST_NODE_H
