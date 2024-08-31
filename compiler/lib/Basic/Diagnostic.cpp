#include "rxc/Basic/Diagnostic.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>

namespace rx {

Diagnostic::Diagnostic(Type Kind, std::string Message,
                       std::optional<SourceLocation> SrcLoc)
    : SrcLoc(std::move(SrcLoc)), Kind(Kind), Message(std::move(Message)) {}

void ConsoleDiagnosticConsumer::printMessageHeader(const Diagnostic &D) {
  if (D.loc()) {
    auto Loc = D.loc().value();
    auto OS = llvm::WithColor(llvm::errs(), llvm::raw_ostream::WHITE, true);
    if (Loc.file()) {
      OS << Loc.file()->getAbsPath() << ":" << Loc.loc() << ": ";
    } else {
      OS << "builtin: ";
    }
  }
  PrintDiagnosticType(D.kind());
  llvm::errs() << D.message() << "\n";
  if (D.loc()) {
    printSourceLocation(D.loc().value());
  }
}

void ConsoleDiagnosticConsumer::printSourceLocation(const SourceLocation &Loc) {
  auto FileContent = Loc.file()->getBuffer().getBuffer();

  size_t CurrentLine = 1;
  size_t LineStartIdx = 0;
  size_t LineEndIdx = 0;

  while (LineEndIdx < FileContent.size()) {
    LineEndIdx = FileContent.find('\n', LineStartIdx);

    if (LineEndIdx == llvm::StringRef::npos) {
      LineEndIdx = FileContent.size();
    }

    if (CurrentLine >= Loc.loc().LineStart &&
        CurrentLine <= Loc.loc().LineEnd) {
      llvm::StringRef line = FileContent.slice(LineStartIdx, LineEndIdx);
      PadLineNumber(llvm::errs(), CurrentLine) << " | " << line << '\n';
    }

    LineStartIdx = LineEndIdx + 1;
    ++CurrentLine;
  }
  PadLineNumber(llvm::errs(), 0) << " |\n";
}

llvm::raw_ostream &
ConsoleDiagnosticConsumer::PrintDiagnosticType(Diagnostic::Type Type) {
  switch (Type) {
  case Diagnostic::Type::Error:
    return llvm::WithColor::error();
  case Diagnostic::Type::Warning:
    return llvm::WithColor::warning();
  case Diagnostic::Type::Note:
    return llvm::WithColor(llvm::errs(), llvm::raw_ostream::CYAN, true)
           << "note: ";
  default:
    llvm_unreachable("Invalid diagnostic type");
  }
}

llvm::raw_ostream &
ConsoleDiagnosticConsumer::PadLineNumber(llvm::raw_ostream &Os, size_t Num,
                                         int PrefixLen) {
  auto Repr = std::to_string(Num);
  int Len = Num == 0 ? 0 : (int)Repr.size();
  for (int n = 0; n < PrefixLen - Len; ++n) {
    Os << ' ';
  }
  if (Num != 0)
    Os << Repr;
  return Os;
}
} // namespace rx
