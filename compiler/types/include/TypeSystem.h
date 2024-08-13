#ifndef TYPE_TYPESYSTEM_H
#define TYPE_TYPESYSTEM_H

#include "llvm/ADT/StringMap.h"
#include <llvm/ADT/SmallVector.h>

namespace rx::types {

struct Type {
  virtual ~Type() = default;

  virtual bool isBuiltin() const = 0;
};

struct NominalType : public Type {};

struct AliasType : public NominalType {
    Type* AliasedType;
};

struct BuiltinType : public NominalType {

  bool isBuiltin() const override { return true; }
};

enum class NativeIntegerType { i1, i8, i16, i32, i64, u8, u16, u32, u64 };

struct IntType : public BuiltinType {
  NativeIntegerType NativeType;
};
struct FloatType : public BuiltinType {};
struct StringType : public BuiltinType {};

class TypeContext {
public:
  TypeContext() = default;
  TypeContext(const TypeContext &) noexcept = delete;
  TypeContext(TypeContext &&) noexcept = default;
  TypeContext &operator=(const TypeContext &) noexcept = delete;
  TypeContext &operator=(TypeContext &&) noexcept = default;

private:
  void registerBuiltinTypes() {

    NominalTypes["i1"] = Storage.emplace_back(NativeIntegerType::i1).get();
    NominalTypes["i8"] = Storage.emplace_back(NativeIntegerType::i8).get();
    NominalTypes["u8"] = Storage.emplace_back(NativeIntegerType::u8).get();
    NominalTypes["i16"] = Storage.emplace_back(NativeIntegerType::i16).get();
    NominalTypes["u16"] = Storage.emplace_back(NativeIntegerType::u16).get();
    NominalTypes["i32"] = Storage.emplace_back(NativeIntegerType::i32).get();
    NominalTypes["u32"] = Storage.emplace_back(NativeIntegerType::u32).get();
    NominalTypes["i64"] = Storage.emplace_back(NativeIntegerType::i64).get();
    NominalTypes["u64"] = Storage.emplace_back(NativeIntegerType::u64).get();
  }

private:
  llvm::StringMap<Type *> NominalTypes;
  llvm::SmallVector<std::unique_ptr<Type>, 32> Storage;
};

} // namespace rx::types

#endif
