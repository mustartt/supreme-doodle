#ifndef RXC_AST_TYPE_H
#define RXC_AST_TYPE_H

#include "QualType.h"

#include <llvm/ADT/ArrayRef.h>

namespace rx {

// Leaf Nodes
class LeafType : public Type {
public:
  bool isLeafType() const { return true; }
};

class UnknownType : public LeafType {};

class UnitType : public LeafType {};

enum class NativeType { i1, i8, i16, i32, i64, f32, f64, string };

class BuiltinType : public LeafType {
public:
  BuiltinType(NativeType Ty = NativeType::i1) : Ty(Ty) {}

private:
  NativeType Ty;
};

class NamedType : public LeafType {};

// Composite Types
class CompositeType : public Type {
public:
  bool isLeafType() const { return false; }
};

class PointerType : public CompositeType {
public:
  PointerType(QualType PointeeTy) : PointeeTy(PointeeTy) {}

private:
  QualType PointeeTy;
};

class ArrayType : public CompositeType {
public:
  ArrayType(QualType ElementTy) : ElementTy(ElementTy) {}

private:
  QualType ElementTy;
};

class FuncType : public CompositeType {
public:
  FuncType(llvm::ArrayRef<QualType> ParamTys, QualType ReturnTy)
      : ParamTys(ParamTys), ReturnTy(ReturnTy) {}

  llvm::ArrayRef<QualType> getParamTypes() const { return ParamTys; }
  QualType getReturnType() const { return ReturnTy; }

  bool operator==(const FuncType &Other) const noexcept {
    return ReturnTy == Other.ReturnTy && ParamTys == Other.ParamTys;
  }

private:
  llvm::SmallVector<QualType, 8> ParamTys;
  QualType ReturnTy;
};

class ObjectType : public CompositeType {};
class EnumType : public CompositeType {};

} // namespace rx

namespace std {
template <> struct hash<rx::FuncType> {
  std::size_t operator()(const rx::FuncType &Val) const noexcept;
};
} // namespace std

#endif
