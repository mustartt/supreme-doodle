#include "rxc/Parser/Parser.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

namespace rx::parser {

static llvm::raw_ostream &padLineNumber(llvm::raw_ostream &Os, size_t Num,
                                        int PrefixLen = 4) {
  auto Repr = std::to_string(Num);
  int Len = (int)Repr.size();
  for (int n = 0; n < PrefixLen - Len; ++n) {
    Os << ' ';
  }
  Os << Repr;
  return Os;
}

void ParserError::printError(llvm::raw_ostream &OS,
                             llvm::MemoryBufferRef Ref) const {
  llvm::StringRef FileContent = Ref.getBuffer();

  size_t CurrentLine = 1;
  size_t LineStartIdx = 0;
  size_t LineEndIdx = 0;

  OS << "Syntax Error at " << Loc << ": " << Message << '\n';

  while (LineEndIdx < FileContent.size()) {
    LineEndIdx = FileContent.find('\n', LineStartIdx);

    if (LineEndIdx == llvm::StringRef::npos) {
      LineEndIdx = FileContent.size();
    }

    if (CurrentLine >= Loc.LineStart && CurrentLine <= Loc.LineEnd) {
      llvm::StringRef line = FileContent.slice(LineStartIdx, LineEndIdx);
      padLineNumber(OS, CurrentLine) << " | " << line << '\n';
    }

    LineStartIdx = LineEndIdx + 1;
    ++CurrentLine;
  }
}

} // namespace rx::parser
