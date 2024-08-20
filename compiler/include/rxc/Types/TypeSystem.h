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
  void registerBuiltinTypes() {}

  void registerBuiltinAlias() {}

private:
  llvm::StringMap<Type *> NominalTypes;
  llvm::StringMap<Type *> TypeAliases;
  llvm::SmallVector<std::unique_ptr<Type>, 32> Storage;
};

class TypeEnvironment {
public:
  TypeEnvironment(TypeContext &Ctx, TypeEnvironment *Parent = nullptr)
      : Ctx(Ctx), Parent(Parent) {}

private:
  TypeContext &Ctx;
  TypeEnvironment *Parent;
  llvm::StringMap<Type *> Aliases;
  llvm::StringMap<Type *> NominalTypes;
};

} // namespace rx::types

#endif
