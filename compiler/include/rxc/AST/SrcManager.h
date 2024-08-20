#ifndef AST_SRCMANAGER_H
#define AST_SRCMANAGER_H

#include "llvm/Support/raw_ostream.h"
#include <llvm/Support/MemoryBufferRef.h>

namespace rx::ast {

class SrcRange {
public:
  SrcRange();
  SrcRange(size_t LineStart, size_t ColStart, size_t LineEnd, size_t ColEnd);

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const SrcRange &loc);
  operator std::string() const;

  size_t LineStart;
  size_t ColStart;
  size_t LineEnd;
  size_t ColEnd;
};

class SourceManager {

private:
  llvm::MemoryBufferRef File;
};

} // namespace rx::ast
#endif
