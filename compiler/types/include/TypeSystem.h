#ifndef TYPE_TYPESYSTEM_H
#define TYPE_TYPESYSTEM_H

#include "llvm/ADT/StringMap.h"
#include <llvm/ADT/SmallVector.h>

namespace rx::types {

struct Type {
  virtual ~Type() = default;

  virtual bool isBuiltin() const { return false; }
};

struct StructuralType : public Type {};

struct ObjectType : public StructuralType {
  struct ObjectEntry {
    std::string Entryname;
    Type *EntryType;
  };
  llvm::SmallVector<ObjectEntry> Entries;
};

struct ArrayType : public StructuralType {
  Type *ElementType;
};

struct FunctionType : public StructuralType {
  llvm::SmallVector<Type *> ParamTypes;
  Type *RetType;
};

struct PointerType : public StructuralType {
  Type *PointeeType;
  bool Dynamic;
};

struct NominalType : public Type {
  NominalType(std::string Name) : Name(std::move(Name)) {}
  std::string Name;
};

struct AliasType : public NominalType {
  AliasType(std::string Name, Type *Ty)
      : NominalType(std::move(Name)), AliasedType(Ty) {}
  Type *AliasedType;
};

struct BuiltinType : public NominalType {
  BuiltinType(std::string Name) : NominalType(std::move(Name)) {}
  bool isBuiltin() const override { return true; }
};

enum class NativeIntType { i1, i8, i16, i32, i64 };
enum class NativeFloatType { f32, f64 };

const char *GetNativeIntTypeName(NativeIntType Type);
const char *GetNativeFloatTypeName(NativeFloatType Type);

struct IntType : public BuiltinType {
  IntType(NativeIntType Ty) : BuiltinType(GetNativeIntTypeName(Ty)) {}
};

struct FloatType : public BuiltinType {
  FloatType(NativeFloatType Ty) : BuiltinType(GetNativeFloatTypeName(Ty)) {}
};

struct StringType : public BuiltinType {
  StringType() : BuiltinType("string") {}
};

class TypeContext {
public:
  TypeContext() {
    registerBuiltinTypes();
    registerBuiltinAlias();
  }

  TypeContext(const TypeContext &) noexcept = delete;
  TypeContext(TypeContext &&) noexcept = default;
  TypeContext &operator=(const TypeContext &) noexcept = delete;
  TypeContext &operator=(TypeContext &&) noexcept = default;

private:
  void registerBuiltinTypes() {
    // bulitin int types
    NominalTypes["i1"] = Storage.emplace_back(IntType(NativeIntType::i1)).get();
    NominalTypes["i8"] = Storage.emplace_back(IntType(NativeIntType::i8)).get();
    NominalTypes["i16"] =
        Storage.emplace_back(IntType(NativeIntType::i16)).get();
    NominalTypes["i32"] =
        Storage.emplace_back(IntType(NativeIntType::i32)).get();
    NominalTypes["i64"] =
        Storage.emplace_back(IntType(NativeIntType::i64)).get();

    // string type
    NominalTypes["string"] = Storage.emplace_back(StringType()).get();
  }

  void registerBuiltinAlias() {
    NominalTypes["bool"] =
        Storage.emplace_back(AliasType("bool", NominalTypes["i1"])).get();
    NominalTypes["char"] =
        Storage.emplace_back(AliasType("char", NominalTypes["i8"])).get();
  }

private:
  llvm::StringMap<Type *> NominalTypes;
  llvm::SmallVector<std::unique_ptr<Type>, 32> Storage;
};

} // namespace rx::types

#endif
