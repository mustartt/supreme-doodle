#include "rxc/AST/QualType.h"
#include "rxc/AST/Type.h"

namespace rx {

extern UnknownType GlobalUnknownType;

const Type *QualType::getType() const { return Ty; }

QualType QualType::mut(bool Mutable) const {
  QualType QT(*this);
  QT.Mutable = Mutable;
  return QT;
}

bool QualType::operator==(const QualType &Other) const {
  return Other.Ty == Ty && Other.Mutable == Mutable;
}

llvm::hash_code hash_value(const rx::QualType &Val) {
  return llvm::hash_combine(llvm::hash_value(Val.getType()),
                            llvm::hash_value(Val.isMutable()));
}

bool QualType::isMutable() const { return Mutable; }

std::string QualType::getTypeName() const {
  if (!Ty)
    return "<unknown>";
  std::string Prefix;
  if (Mutable)
    Prefix += "mut ";
  return Prefix + Ty->getTypeName();
}

} // namespace rx

namespace llvm {
hash_code hash_value(const rx::QualType &Val) {
  return llvm::hash_combine(llvm::hash_value(Val.getType()),
                            llvm::hash_value(Val.isMutable()));
}

DenseMapInfo<rx::QualType>::Type DenseMapInfo<rx::QualType>::getEmptyKey() {
  return nullptr;
}

DenseMapInfo<rx::QualType>::Type DenseMapInfo<rx::QualType>::getTombstoneKey() {
  static int Tombstone;
  return reinterpret_cast<rx::Type *>(&Tombstone);
}

unsigned DenseMapInfo<rx::QualType>::getHashValue(const Type &Val) {
  return llvm::hash_value(Val);
}

bool DenseMapInfo<rx::QualType>::isEqual(const Type &LHS, const Type &RHS) {
  return LHS == RHS;
}

} // namespace llvm
