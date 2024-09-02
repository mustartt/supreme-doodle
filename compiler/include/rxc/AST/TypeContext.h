#ifndef RXC_AST_TYPE_CONTEXT_H
#define RXC_AST_TYPE_CONTEXT_H

#include "rxc/AST/QualType.h"
#include "rxc/AST/Type.h"
#include <deque>
#include <llvm/ADT/DenseMap.h>
#include <unordered_set>

namespace rx {

static UnknownType GlobalUnknownType;
static UnitType GlobalUnitType;

class TypeDecl;
class TypeContext {
public:
  TypeContext();

public:
  QualType getUnknownType() { return &GlobalUnknownType; }
  QualType getUnitType() { return &GlobalUnitType; }
  QualType getBuiltinType(NativeType Ty) {
    return &BuiltinCtx[static_cast<size_t>(Ty)];
  }
  QualType getNamedType(TypeDecl *TD) {
    if (NamedCtx.contains(TD))
      return NamedCtx[TD].get();
    auto NewType = std::make_unique<NamedType>();
    auto *NewTypePtr = NewType.get();
    NamedCtx[TD] = std::move(NewType);
    return NewTypePtr;
  }
  QualType getPointerType(QualType Ty) {
    if (PointerCtx.contains(Ty))
      return PointerCtx[Ty].get();
    auto NewType = std::make_unique<PointerType>(Ty);
    auto *NewTypePtr = NewType.get();
    PointerCtx[Ty] = std::move(NewType);
    return NewTypePtr;
  }
  QualType getArrayType(QualType Ty) {
    if (ArrayCtx.contains(Ty))
      return ArrayCtx[Ty].get();
    auto NewType = std::make_unique<ArrayType>(Ty);
    auto *NewTypePtr = NewType.get();
    ArrayCtx[Ty] = std::move(NewType);
    return NewTypePtr;
  }
  QualType getFuncType(llvm::ArrayRef<QualType> ParamTys, QualType ReturnTy) {
    FuncType Key(ParamTys, ReturnTy);
    auto [It, _] = FuncCtx.insert(std::move(Key));
    return &*It;
  }

private:
  // leaf types
  std::deque<BuiltinType> BuiltinCtx;
  llvm::DenseMap<TypeDecl *, std::unique_ptr<NamedType>> NamedCtx;

  // composite types
  llvm::DenseMap<QualType, std::unique_ptr<PointerType>> PointerCtx;
  llvm::DenseMap<QualType, std::unique_ptr<ArrayType>> ArrayCtx;
  std::unordered_set<FuncType> FuncCtx;
};
} // namespace rx

#endif
