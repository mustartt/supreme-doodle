#include <gtest/gtest.h>

#include "rxc/AST/Type.h"

using namespace rx;
using namespace rx::ast::type;

TEST(TypeContextTest, PointerIdentity) {
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
