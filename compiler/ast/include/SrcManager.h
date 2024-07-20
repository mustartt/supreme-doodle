#ifndef AST_SRCMANAGER_H
#define AST_SRCMANAGER_H

#include <ostream>

class SrcRange {
public:
  SrcRange(size_t SourceID, int LineStart, int ColStart, int LineEnd,
           int ColEnd)
      : SourceID(SourceID), LineStart(LineStart), ColStart(ColStart),
        LineEnd(LineEnd), ColEnd(ColEnd) {}

  friend std::ostream &operator<<(std::ostream &os, const SrcRange &loc) {
    return os << "<" << loc.LineStart << ":" << loc.ColStart << ", "
              << loc.LineEnd << ":" << loc.ColEnd << ">";
  }

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

#endif
