#ifndef RXC_AST_TYPE_CONTEXT_H
#define RXC_AST_TYPE_CONTEXT_H

#include "rxc/AST/QualType.h"
#include "rxc/AST/Type.h"
#include <deque>
#include <llvm/ADT/DenseMap.h>
#include <unordered_set>

namespace rx {

namespace ast {
class TypeDecl;
}

class TypeContext {
public:
  TypeContext();

public:
  QualType getUnknownType();
  QualType getUnitType();
  QualType getBuiltinType(NativeType Ty);
  QualType getNamedType(ast::TypeDecl *TD);
  QualType getPointerType(QualType Ty);
  QualType getArrayType(QualType Ty);
  QualType getFuncType(llvm::ArrayRef<QualType> ParamTys, QualType ReturnTy);
  QualType getObjectType(llvm::StringMap<QualType> &&Fields);
  QualType getEnumType(llvm::StringMap<QualType> &&Members);

private:
  // leaf types
  std::deque<BuiltinType> BuiltinCtx;
  llvm::DenseMap<ast::TypeDecl *, std::unique_ptr<NamedType>> NamedCtx;

  // composite types
  llvm::DenseMap<QualType, std::unique_ptr<PointerType>> PointerCtx;
  llvm::DenseMap<QualType, std::unique_ptr<ArrayType>> ArrayCtx;
  std::unordered_set<FuncType> FuncCtx;
  std::unordered_set<ObjectType> ObjCtx;
  std::unordered_set<EnumType> EnumCtx;
};
} // namespace rx

#endif
