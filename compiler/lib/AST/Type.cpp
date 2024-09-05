#include "rxc/AST/Type.h"
#include "rxc/AST/AST.h"
#include "rxc/AST/QualType.h"
#include <llvm/ADT/Hashing.h>

namespace rx {

std::string rx::NamedType::getTypeName() const { return Decl->getName().str(); }

std::string BuiltinType::getTypeName() const {
  switch (Ty) {
  case NativeType::i1:
    return "i1";
  case NativeType::i8:
    return "i8";
  case NativeType::i16:
    return "i16";
  case NativeType::i32:
    return "i32";
  case NativeType::i64:
    return "i64";
  case NativeType::f32:
    return "f32";
  case NativeType::f64:
    return "f64";
  case NativeType::string:
    return "string";
  default:
    llvm_unreachable("Unaccounted for NativeType");
  }
}

} // namespace rx

namespace llvm {

hash_code hash_value(const llvm::StringMap<rx::QualType> &Val) {
  hash_code Hash;
  for (const auto &[Key, Value] : Val)
    Hash = llvm::hash_combine(Hash, llvm::hash_value(Key),
                              llvm::hash_value(Value));
  return Hash;
}

} // namespace llvm

namespace std {

size_t hash<rx::FuncType>::operator()(const rx::FuncType &Val) const noexcept {
  return llvm::hash_combine(llvm::hash_value(Val.getParamTypes()),
                            llvm::hash_value(Val.getReturnType()));
}

size_t
hash<rx::ObjectType>::operator()(const rx::ObjectType &Val) const noexcept {
  return llvm::hash_value(Val.getFields());
}

size_t hash<rx::EnumType>::operator()(const rx::EnumType &Val) const noexcept {
  return llvm::hash_value(Val.getMembers());
}

} // namespace std
