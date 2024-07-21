#ifndef AST_SRCMANAGER_H
#define AST_SRCMANAGER_H

#include "llvm/Support/raw_ostream.h"

namespace rx::ast {

class SrcRange {
public:
  SrcRange();
  SrcRange(int LineStart, int ColStart, int LineEnd, int ColEnd);

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const SrcRange &loc);
  operator std::string() const;

  int LineStart;
  int ColStart;
  int LineEnd;
  int ColEnd;
};

} // namespace rx::ast
#endif
