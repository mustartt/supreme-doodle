#include <gtest/gtest.h>

#include "rxc/AST/TypeContext.h"

using namespace rx;

TEST(QualTypeTest, OperatorEQ) {
  BuiltinType BT(NativeType::i32);
  BuiltinType Other(NativeType::string);
  QualType T1(&BT);
  QualType T2(&BT);
  QualType T3(&Other);

  EXPECT_TRUE(T1 == T2);
  EXPECT_FALSE(T1 != T2);
  EXPECT_FALSE(T1 == T3);

  auto T4 = T1.mut(true);
  EXPECT_FALSE(T1 == T4);
}

TEST(TypeContextTest, PointerIdentityLeaf) {
  TypeContext Context;

  // Unknown and Unit Type
  EXPECT_EQ(Context.getUnitType(), Context.getUnitType());
  EXPECT_EQ(Context.getUnknownType(), Context.getUnknownType());

  // Native Type
  EXPECT_EQ(Context.getBuiltinType(NativeType::i8),
            Context.getBuiltinType(NativeType::i8));
  EXPECT_EQ(Context.getBuiltinType(NativeType::i8),
            Context.getBuiltinType(NativeType::i8));
  EXPECT_EQ(Context.getBuiltinType(NativeType::i16),
            Context.getBuiltinType(NativeType::i16));
  EXPECT_EQ(Context.getBuiltinType(NativeType::i32),
            Context.getBuiltinType(NativeType::i32));
  EXPECT_EQ(Context.getBuiltinType(NativeType::i64),
            Context.getBuiltinType(NativeType::i64));
  EXPECT_EQ(Context.getBuiltinType(NativeType::f32),
            Context.getBuiltinType(NativeType::f32));
  EXPECT_EQ(Context.getBuiltinType(NativeType::f64),
            Context.getBuiltinType(NativeType::f64));
  EXPECT_EQ(Context.getBuiltinType(NativeType::string),
            Context.getBuiltinType(NativeType::string));

  // Named Type
  int D1, D2;
  EXPECT_EQ(Context.getNamedType(reinterpret_cast<TypeDecl *>(&D1)),
            Context.getNamedType(reinterpret_cast<TypeDecl *>(&D1)));
  EXPECT_NE(Context.getNamedType(reinterpret_cast<TypeDecl *>(&D1)),
            Context.getNamedType(reinterpret_cast<TypeDecl *>(&D2)));
}

TEST(TypeContextTest, PointerIdentityComposite) {
  TypeContext Context;

  auto T = Context.getBuiltinType(NativeType::i32);
  auto MutT = T.mut(true);

  EXPECT_EQ(Context.getPointerType(T), Context.getPointerType(T));
  EXPECT_NE(Context.getPointerType(T), Context.getPointerType(MutT));

  EXPECT_EQ(Context.getArrayType(T), Context.getArrayType(T));
  EXPECT_NE(Context.getArrayType(T), Context.getArrayType(MutT));
}

TEST(TypeContextTest, PointerIdentityFunc) {
  TypeContext Context;
  auto T1 = Context.getBuiltinType(NativeType::i32);
  auto Unit = Context.getUnitType();

  auto F1 = Context.getFuncType(llvm::ArrayRef<QualType>(), Unit);
  auto F2 = Context.getFuncType(llvm::ArrayRef<QualType>(), Unit);
  EXPECT_EQ(F1, F2);

  auto F3 = Context.getFuncType(llvm::ArrayRef<QualType>(), T1);
  EXPECT_NE(F1, F3);

  auto T2 = Context.getBuiltinType(NativeType::i32);
  llvm::SmallVector<QualType> Params{T1, T2};

  auto F4 = Context.getFuncType(Params, Unit);
  auto F5 = Context.getFuncType(Params, Unit);
  EXPECT_EQ(F4, F5);
}
