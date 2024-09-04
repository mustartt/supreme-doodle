#ifndef RXC_AST_QUALTYPE_H
#define RXC_AST_QUALTYPE_H

#include <llvm/ADT/DenseMapInfo.h>
#include <llvm/ADT/Hashing.h>

namespace rx {

class Type {
public:
  virtual ~Type() = default;

  virtual bool isLeafType() const = 0;
  virtual std::string getTypeName() const = 0;
};

class QualType {
public:
  QualType(const Type *Ty = nullptr) : Ty(Ty) {}
  QualType(const QualType &) = default;
  QualType(QualType &&) = default;
  QualType &operator=(const QualType &) = default;
  QualType &operator=(QualType &&) = default;

public:
  const Type *getType() const;
  QualType mut(bool Mutable) const;
  bool isMutable() const;
  std::string getTypeName() const;

  bool operator==(const QualType &Other) const;

private:
  const Type *Ty;
  bool Mutable = false;
};

llvm::hash_code hash_value(const rx::QualType &Val);

} // namespace rx

namespace llvm {

hash_code hash_value(const rx::QualType &Val);

template <> struct DenseMapInfo<rx::QualType> {
  using Type = rx::QualType;
  static Type getEmptyKey();
  static Type getTombstoneKey();
  static unsigned getHashValue(const Type &Val);
  static bool isEqual(const Type &LHS, const Type &RHS);
};

} // namespace llvm
#endif
