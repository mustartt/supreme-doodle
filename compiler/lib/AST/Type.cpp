#include "rxc/AST/Type.h"
#include "rxc/AST/QualType.h"
#include <llvm/ADT/Hashing.h>

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
