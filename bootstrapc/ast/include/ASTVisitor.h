#ifndef AST_VISITOR_H
#define AST_VISITOR_H

namespace rx::ast {

class ProgramDecl;
class PackageDecl;
class ImportDecl;
class StructDecl;
class StructFieldDecl;
class VarDecl;
class FuncDecl;
class BlockStmt;
class ReturnStmt;
class DeclStmt;
class ExprStmt;
class IfStmt;
class ForStmt;
class BinaryExpr;
class UnaryExpr;

class BaseDeclVisitor {
public:
  virtual ~BaseDeclVisitor() {}

  virtual void visit(ProgramDecl *node) = 0;
  virtual void visit(PackageDecl *node) = 0;
  virtual void visit(ImportDecl *node) = 0;
  virtual void visit(StructDecl *node) = 0;
  virtual void visit(StructFieldDecl *node) = 0;
  virtual void visit(VarDecl *node) = 0;
  virtual void visit(FuncDecl *node) = 0;
};

class BaseStmtVisitor {
public:
  virtual ~BaseStmtVisitor() {}

  virtual void visit(BlockStmt *node) = 0;
  virtual void visit(ReturnStmt *node) = 0;
  virtual void visit(DeclStmt *node) = 0;
  virtual void visit(ExprStmt *node) = 0;
  virtual void visit(IfStmt *node) = 0;
  virtual void visit(ForStmt *node) = 0;
};

class BaseExprVisitor {
public:
  virtual ~BaseExprVisitor() = default;

  virtual void visit(BinaryExpr *node) = 0;
  virtual void visit(UnaryExpr *node) = 0;
};

} // namespace rx::ast

#endif // AST_VISITOR_H
