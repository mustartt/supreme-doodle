
#include <deque>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/SmallVector.h>
#include <memory>

namespace rx::ast::type {

class Type {
public:
  virtual ~Type() = default;

  virtual bool isLeafType() const = 0;
};

class QualType {
public:
  QualType(Type *Ty) : Ty(Ty) {}
  QualType(const QualType &) = default;
  QualType(QualType &&) = default;
  QualType &operator=(const QualType &) = default;
  QualType &operator=(QualType &&) = default;

public:
  bool operator==(const QualType &Other) const {
    return Other.Ty == Ty && Other.Mutable == Mutable;
  }

public:
  Type *Ty;
  bool Mutable = false;
};

// Leaf Nodes
class LeafType : public Type {
public:
  bool isLeafType() const { return true; }
};

class UnknownType : public LeafType {};

// Globally Unique Unknown Value
static UnknownType GlobalUnknownType;

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

class FunctionType : public CompositeType {
public:
  FunctionType(llvm::ArrayRef<QualType> ParamTys, QualType ReturnTy)
      : ParamTys(ParamTys), ReturnTy(ReturnTy) {}

private:
  llvm::SmallVector<QualType, 8> ParamTys;
  QualType ReturnTy;
};

class ObjectType : public CompositeType {};
class EnumType : public CompositeType {};

// All Types have pointer identity

class TypeDecl;
class TypeContext {
public:
  TypeContext() {
    BuiltinCtx.resize(static_cast<size_t>(NativeType::string) + 1);
    BuiltinCtx[static_cast<size_t>(NativeType::i1)] =
        BuiltinType(NativeType::i1);
    BuiltinCtx[static_cast<size_t>(NativeType::i8)] =
        BuiltinType(NativeType::i8);
    BuiltinCtx[static_cast<size_t>(NativeType::i16)] =
        BuiltinType(NativeType::i16);
    BuiltinCtx[static_cast<size_t>(NativeType::i32)] =
        BuiltinType(NativeType::i32);
    BuiltinCtx[static_cast<size_t>(NativeType::i64)] =
        BuiltinType(NativeType::i64);
    BuiltinCtx[static_cast<size_t>(NativeType::f32)] =
        BuiltinType(NativeType::f32);
    BuiltinCtx[static_cast<size_t>(NativeType::f64)] =
        BuiltinType(NativeType::f64);
    BuiltinCtx[static_cast<size_t>(NativeType::string)] =
        BuiltinType(NativeType::string);
  }

public:
  QualType getUnknownType() { return &GlobalUnknownType; }
  QualType getUnitType() { return UnitCtx.get(); }
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

private:
  // leaf types
  std::unique_ptr<UnitType> UnitCtx = std::make_unique<UnitType>();
  std::deque<BuiltinType> BuiltinCtx;
  llvm::DenseMap<TypeDecl *, std::unique_ptr<NamedType>> NamedCtx;

  // composite types
  llvm::DenseMap<QualType, std::unique_ptr<PointerType>> PointerCtx;
  llvm::DenseMap<QualType, std::unique_ptr<ArrayType>> ArrayCtx;
};

} // namespace rx::ast::type

namespace llvm {
template <> struct DenseMapInfo<rx::ast::type::QualType> {
  using Type = rx::ast::type::QualType;
  static Type getEmptyKey() { return &rx::ast::type::GlobalUnknownType; }
  static Type getTombstoneKey() { return nullptr; }
  static unsigned getHashValue(const Type &Val) {
    return llvm::hash_combine(llvm::hash_value(Val.Ty),
                              llvm::hash_value(Val.Mutable));
  }
  static bool isEqual(const Type &LHS, const Type &RHS) { return LHS == RHS; }
};

} // namespace llvm
