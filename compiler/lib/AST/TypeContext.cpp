#include "rxc/AST/TypeContext.h"

namespace rx {

TypeContext::TypeContext() {
  BuiltinCtx.resize(static_cast<size_t>(NativeType::string) + 1);
  BuiltinCtx[static_cast<size_t>(NativeType::i1)] = BuiltinType(NativeType::i1);
  BuiltinCtx[static_cast<size_t>(NativeType::i8)] = BuiltinType(NativeType::i8);
  BuiltinCtx[static_cast<size_t>(NativeType::i16)] =
      BuiltinType(NativeType::i16);
  BuiltinCtx[static_cast<size_t>(NativeType::i32)] =
      BuiltinType(NativeType::i32);
  BuiltinCtx[static_cast<size_t>(NativeType::i64)] =
      BuiltinType(NativeType::i64);
  BuiltinCtx[static_cast<size_t>(NativeType::f32)] =
      BuiltinType(NativeType::f32);
  BuiltinCtx[static_cast<size_t>(NativeType::f64)] =
      BuiltinType(NativeType::f64);
  BuiltinCtx[static_cast<size_t>(NativeType::string)] =
      BuiltinType(NativeType::string);
}

} // namespace rx
