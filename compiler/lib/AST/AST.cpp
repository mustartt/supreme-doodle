#include "rxc/AST/AST.h"
#include "rxc/AST/ASTVisitor.h"

namespace rx::ast {

void ASTBuiltinType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void ASTDeclTypeRef::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void ASTAccessType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void ASTQualType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void ASTPointerType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void ASTFunctionType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void ASTArrayType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void ASTObjectType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }
void ASTEnumType::accept(BaseTypeVisitor &visitor) { visitor.visit(this); }

void ProgramDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void PackageDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void ExportedDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void ImportDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void TypeDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void UseDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
void ImplDecl::accept(BaseDeclVisitor &visitor) { visitor.visit(this); }
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
void DeclRefExpr::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void ObjectLiteral::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void BoolLiteral::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void CharLiteral::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void NumLiteral::accept(BaseExprVisitor &visitor) { visitor.visit(this); }
void StringLiteral::accept(BaseExprVisitor &visitor) { visitor.visit(this); }

} // namespace rx::ast
