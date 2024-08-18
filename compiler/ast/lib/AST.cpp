#include "AST.h"
#include "ASTVisitor.h"

namespace rx::ast {

void DeclRefType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void MutableType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void PointerType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void FunctionType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void ArrayType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void ObjectType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void EnumType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }

void ProgramDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void PackageDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void ImportDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void TypeDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void VarDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void FuncDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void FuncParamDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void BlockStmt::accept(BaseStmtVisitor &visitor) { visitor.visit(this); }
void ReturnStmt::accept(BaseStmtVisitor &visitor) { visitor.visit(this); }
void DeclStmt::accept(BaseStmtVisitor &visitor) { visitor.visit(this); }
void ExprStmt::accept(BaseStmtVisitor &visitor) { visitor.visit(this); }
void ForStmt::accept(BaseStmtVisitor &visitor) { visitor.visit(this); }
void IfExpr::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void BinaryExpr::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void UnaryExpr::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void CallExpr::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void AccessExpr::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void IndexExpr::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void AssignExpr::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void IdentifierExpr::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void BoolLiteral::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void CharLiteral::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void NumLiteral::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void StringLiteral::accept(BaseExprVisitor &visitor) { visitor.visit(this); }

} // namespace rx::ast
