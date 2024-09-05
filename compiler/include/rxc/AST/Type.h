#ifndef RXC_AST_TYPE_H
#define RXC_AST_TYPE_H

#include "QualType.h"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>

namespace rx {

namespace ast {
class TypeDecl;
class FuncDecl;
} // namespace ast

// Leaf Nodes
class LeafType : public Type {
public:
  bool isLeafType() const { return true; }
};

class UnknownType : public LeafType {
public:
  std::string getTypeName() const override { return "<unknown>"; }
};

class UnitType : public LeafType {
public:
  std::string getTypeName() const override { return "unit"; }
};

enum class NativeType { i1, i8, i16, i32, i64, f32, f64, string };

class BuiltinType : public LeafType {
public:
  BuiltinType(NativeType Ty = NativeType::i1) : Ty(Ty) {}

  std::string getTypeName() const override;

private:
  NativeType Ty;
};

class NamedType : public LeafType {
public:
  NamedType(ast::TypeDecl *Decl, QualType Definition)
      : Decl(Decl), Definition(Definition) {}

  ast::TypeDecl *getDecl() const { return Decl; };
  std::string getTypeName() const override;
  llvm::ArrayRef<ast::FuncDecl *> getImpls() const { return Impls; }
  void addImpl(ast::FuncDecl* Func) {Impls.push_back(Func); }

private:
  ast::TypeDecl *Decl;
  QualType Definition;
  llvm::SmallVector<ast::FuncDecl *, 8> Impls;
};

// Composite Types
class CompositeType : public Type {
public:
  bool isLeafType() const { return false; }
};

class PointerType : public CompositeType {
public:
  PointerType(QualType PointeeTy) : PointeeTy(PointeeTy) {}

  std::string getTypeName() const override {
    return "*" + PointeeTy.getTypeName();
  }

private:
  QualType PointeeTy;
};

class ArrayType : public CompositeType {
public:
  ArrayType(QualType ElementTy) : ElementTy(ElementTy) {}

  std::string getTypeName() const override {
    return "[" + ElementTy.getTypeName() + "]";
  }

private:
  QualType ElementTy;
};

class FuncType : public CompositeType {
public:
  FuncType(llvm::ArrayRef<QualType> ParamTys, QualType ReturnTy)
      : ParamTys(ParamTys), ReturnTy(ReturnTy) {}

  llvm::ArrayRef<QualType> getParamTypes() const { return ParamTys; }
  QualType getReturnType() const { return ReturnTy; }

  bool operator==(const FuncType &Other) const noexcept {
    return ReturnTy == Other.ReturnTy && ParamTys == Other.ParamTys;
  }

  std::string getTypeName() const override {
    std::string Result = "func(";
    for (auto [Idx, PT] : llvm::enumerate(ParamTys)) {
      Result += PT.getTypeName();
      if (Idx + 1 != ParamTys.size())
        Result += ", ";
    }
    Result += ") ";
    Result += ReturnTy.getTypeName();
    return Result;
  }

private:
  llvm::SmallVector<QualType, 8> ParamTys;
  QualType ReturnTy;
};

class ObjectType : public CompositeType {
public:
  ObjectType(llvm::StringMap<QualType> &&Fields) : Fields(std::move(Fields)) {}

  const llvm::StringMap<QualType> &getFields() const { return Fields; }

  bool operator==(const ObjectType &Other) const noexcept {
    return Fields == Other.Fields;
  }

  std::string getTypeName() const override {
    std::string Result = "{";
    for (auto [Idx, PT] : llvm::enumerate(Fields)) {
      Result += PT.getKey();
      Result += ": ";
      Result += PT.getValue().getTypeName();
      if (Idx + 1 != Fields.size())
        Result += ", ";
    }
    Result += "}";
    return Result;
  }

private:
  llvm::StringMap<QualType> Fields;
};

class EnumType : public CompositeType {
public:
  EnumType(llvm::StringMap<QualType> &&Members) : Members(std::move(Members)) {}

  const llvm::StringMap<QualType> &getMembers() const { return Members; }

  bool operator==(const EnumType &Other) const noexcept {
    return Members == Other.Members;
  }

  std::string getTypeName() const override {
    std::string Result = "Enum{";
    for (auto [Idx, PT] : llvm::enumerate(Members)) {
      Result += PT.getKey();
      Result += ": ";
      Result += PT.getValue().getTypeName();
      if (Idx + 1 != Members.size())
        Result += ", ";
    }
    Result += "}";
    return Result;
  }

private:
  llvm::StringMap<QualType> Members;
};

} // namespace rx

namespace std {
template <> struct hash<rx::FuncType> {
  std::size_t operator()(const rx::FuncType &Val) const noexcept;
};

template <> struct hash<rx::ObjectType> {
  std::size_t operator()(const rx::ObjectType &Val) const noexcept;
};

template <> struct hash<rx::EnumType> {
  std::size_t operator()(const rx::EnumType &Val) const noexcept;
};
} // namespace std

#endif
