#ifndef AST_SRCMANAGER_H
#define AST_SRCMANAGER_H

#include <vector>
#include <llvm/Support/raw_ostream.h>

namespace rx::ast {

class SrcRange {
public:
  SrcRange();
  SrcRange(size_t SourceID, int LineStart, int ColStart, int LineEnd,
           int ColEnd);

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const SrcRange &loc);
  operator std::string() const;

  size_t SourceID;
  int LineStart;
  int ColStart;
  int LineEnd;
  int ColEnd;
};

class SrcManager {
public:
private:
  std::vector<std::string> Sources;
};

} // namespace rx::ast
#endif
