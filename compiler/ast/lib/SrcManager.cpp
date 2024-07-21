
#include "SrcManager.h"

namespace rx::ast {

SrcRange::SrcRange() : LineStart(0), ColStart(0), LineEnd(0), ColEnd(0) {}

SrcRange::SrcRange(int LineStart, int ColStart, int LineEnd, int ColEnd)
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
