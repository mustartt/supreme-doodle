#ifndef AST_VISITOR_H
#define AST_VISITOR_H

namespace rx::ast {

class ProgramDecl;
class PackageDecl;
class ImportDecl;
class VarDecl;
class TypeDecl;
class UseDecl;
class ImplDecl;
class FuncDecl;
class FuncParamDecl;
class ExportedDecl;
class BaseDeclVisitor {
public:
  virtual ~BaseDeclVisitor() {}

  virtual void visit(ProgramDecl *) = 0;
  virtual void visit(ExportedDecl *) = 0;
  virtual void visit(PackageDecl *) = 0;
  virtual void visit(ImportDecl *) = 0;
  virtual void visit(VarDecl *) = 0;
  virtual void visit(TypeDecl *) = 0;
  virtual void visit(UseDecl *) = 0;
  virtual void visit(ImplDecl *) = 0;
  virtual void visit(FuncDecl *) = 0;
  virtual void visit(FuncParamDecl *) = 0;
};

class BuiltinType;
class DeclRefType;
class AccessType;
class MutableType;
class PointerType;
class ArrayType;
class FunctionType;
class ObjectType;
class EnumType;
class BaseTypeVisitor {
public:
  virtual ~BaseTypeVisitor() {}

  virtual void visit(BuiltinType *Node) = 0;
  virtual void visit(DeclRefType *Node) = 0;
  virtual void visit(AccessType *Node) = 0;
  virtual void visit(MutableType *Node) = 0;
  virtual void visit(PointerType *Node) = 0;
  virtual void visit(ArrayType *Node) = 0;
  virtual void visit(FunctionType *Node) = 0;
  virtual void visit(ObjectType *Node) = 0;
  virtual void visit(EnumType *Node) = 0;
};

class BlockStmt;
class ReturnStmt;
class DeclStmt;
class ExprStmt;
class ForStmt;
class BaseStmtVisitor {
public:
  virtual ~BaseStmtVisitor() {}

  virtual void visit(BlockStmt *node) = 0;
  virtual void visit(ReturnStmt *node) = 0;
  virtual void visit(DeclStmt *node) = 0;
  virtual void visit(ExprStmt *node) = 0;
  virtual void visit(ForStmt *node) = 0;
};

class CallExpr;
class AccessExpr;
class IndexExpr;
class AssignExpr;
class DeclRefExpr;
class BoolLiteral;
class CharLiteral;
class NumLiteral;
class StringLiteral;
class IfExpr;
class BinaryExpr;
class UnaryExpr;

class BaseExprVisitor {
public:
  virtual ~BaseExprVisitor() = default;

  virtual void visit(IfExpr *node) = 0;
  virtual void visit(BinaryExpr *node) = 0;
  virtual void visit(UnaryExpr *node) = 0;
  virtual void visit(CallExpr *node) = 0;
  virtual void visit(AccessExpr *node) = 0;
  virtual void visit(IndexExpr *node) = 0;
  virtual void visit(AssignExpr *node) = 0;
  virtual void visit(DeclRefExpr *node) = 0;
  virtual void visit(BoolLiteral *node) = 0;
  virtual void visit(CharLiteral *node) = 0;
  virtual void visit(NumLiteral *node) = 0;
  virtual void visit(StringLiteral *node) = 0;
};

} // namespace rx::ast

#endif // AST_VISITOR_H
