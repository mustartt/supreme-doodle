#include "rxc/AST/Type.h"

namespace std {

size_t hash<rx::FuncType>::operator()(const rx::FuncType &Val) const noexcept {
  return llvm::hash_combine(llvm::hash_value(Val.getParamTypes()),
                            llvm::hash_value(Val.getReturnType()));
}

} // namespace std
