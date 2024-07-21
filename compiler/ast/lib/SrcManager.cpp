
#include "SrcManager.h"

namespace rx::ast {

SrcRange::SrcRange() : LineStart(0), ColStart(0), LineEnd(0), ColEnd(0) {}

SrcRange::SrcRange(size_t LineStart, size_t ColStart, size_t LineEnd, size_t ColEnd)
    : LineStart(LineStart), ColStart(ColStart), LineEnd(LineEnd),
      ColEnd(ColEnd) {}

llvm::raw_ostream &operator<<(llvm::raw_ostream &Os, const SrcRange &loc) {
  if (loc.LineStart == 0) {
    return Os << "<invalid loc>";
  }
  return Os << "<" << loc.LineStart << ":" << loc.ColStart << ", "
            << loc.LineEnd << ":" << loc.ColEnd << ">";
}

SrcRange::operator std::string() const {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << *this;
  return Str;
}

} // namespace rx::ast
