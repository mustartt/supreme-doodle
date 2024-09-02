#include "rxc/AST/TypeContext.h"
#include "rxc/AST/Type.h"

namespace rx {

UnknownType GlobalUnknownType;
UnitType GlobalUnitType;

TypeContext::TypeContext() {
  BuiltinCtx.resize(static_cast<size_t>(NativeType::string) + 1);
  BuiltinCtx[static_cast<size_t>(NativeType::i1)] = BuiltinType(NativeType::i1);
  BuiltinCtx[static_cast<size_t>(NativeType::i8)] = BuiltinType(NativeType::i8);
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

QualType TypeContext::getUnknownType() { return &GlobalUnknownType; }

QualType TypeContext::getUnitType() { return &GlobalUnitType; }

QualType TypeContext::getBuiltinType(NativeType Ty) {
  return &BuiltinCtx[static_cast<size_t>(Ty)];
}

QualType TypeContext::getNamedType(TypeDecl *TD) {
  if (NamedCtx.contains(TD))
    return NamedCtx[TD].get();
  auto NewType = std::make_unique<NamedType>();
  auto *NewTypePtr = NewType.get();
  NamedCtx[TD] = std::move(NewType);
  return NewTypePtr;
}

QualType TypeContext::getPointerType(QualType Ty) {
  if (PointerCtx.contains(Ty))
    return PointerCtx[Ty].get();
  auto NewType = std::make_unique<PointerType>(Ty);
  auto *NewTypePtr = NewType.get();
  PointerCtx[Ty] = std::move(NewType);
  return NewTypePtr;
}

QualType TypeContext::getArrayType(QualType Ty) {
  if (ArrayCtx.contains(Ty))
    return ArrayCtx[Ty].get();
  auto NewType = std::make_unique<ArrayType>(Ty);
  auto *NewTypePtr = NewType.get();
  ArrayCtx[Ty] = std::move(NewType);
  return NewTypePtr;
}

QualType TypeContext::getFuncType(llvm::ArrayRef<QualType> ParamTys,
                                  QualType ReturnTy) {
  FuncType Key(ParamTys, ReturnTy);
  auto [It, _] = FuncCtx.insert(std::move(Key));
  return &*It;
}

QualType TypeContext::getObjectType(llvm::StringMap<QualType> &&Fields) {
  ObjectType Key(std::move(Fields));
  auto [It, _] = ObjCtx.insert(std::move(Key));
  return &*It;
}

QualType TypeContext::getEnumType(llvm::StringMap<QualType> &&Members) {
  EnumType Key(std::move(Members));
  auto [It, _] = EnumCtx.insert(std::move(Key));
  return &*It;
}

} // namespace rx
