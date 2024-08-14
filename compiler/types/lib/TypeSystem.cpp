#include "TypeSystem.h"

namespace rx::types {

const char *GetNativeIntTypeName(NativeIntType Type) {
  switch (Type) {
  case NativeIntType::i1:
    return "i1";
  case NativeIntType::i8:
    return "i8";
  case NativeIntType::i16:
    return "i16";
  case NativeIntType::i32:
    return "i32";
  case NativeIntType::i64:
    return "i64";
  }
}

const char *GetNativeFloatTypeName(NativeFloatType Type) {
  switch (Type) {
  case NativeFloatType::f32:
    return "f32";
  case NativeFloatType::f64:
    return "f64";
  }
}

} // namespace rx::types
