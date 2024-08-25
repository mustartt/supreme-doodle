#include "rxc/Basic/SourceManager.h"

#include <cstdint>
#include <llvm/Support/Path.h>

namespace rx {

SrcRange::SrcRange() : LineStart(0), ColStart(0), LineEnd(0), ColEnd(0) {}

SrcRange::SrcRange(size_t LineStart, size_t ColStart, size_t LineEnd,
                   size_t ColEnd)
    : LineStart(LineStart), ColStart(ColStart), LineEnd(LineEnd),
      ColEnd(ColEnd) {}

SrcRange SrcRange::Builtin() { return SrcRange(SIZE_MAX, 0, 0, 0); }

llvm::raw_ostream &operator<<(llvm::raw_ostream &Os, const SrcRange &loc) {
  if (loc.LineStart == SIZE_MAX)
    return Os << "<builtin>";
  if (loc.LineStart == 0)
    return Os << "<invalid loc>";
  return Os << loc.LineStart << ":" << loc.ColStart << "," << loc.LineEnd << ":"
            << loc.ColEnd;
}

SrcRange::operator std::string() const {
  std::string Str;
  llvm::raw_string_ostream Os(Str);
  Os << *this;
  return Str;
}

SourceFile::SourceFile(llvm::StringRef AbsPath,
                       std::unique_ptr<llvm::MemoryBuffer> FileBuf)
    : AbsPath(AbsPath), FileBuf(std::move(FileBuf)) {}

llvm::StringRef SourceFile::getFilename() const {
  return llvm::sys::path::filename(AbsPath);
}

llvm::StringRef SourceFile::getBaseDir() const {
  return llvm::sys::path::parent_path(AbsPath);
}

llvm::StringRef SourceFile::getAbsPath() const { return AbsPath; }

void SourceFile::debug(llvm::raw_ostream &OS) const {
  OS << "File: " << AbsPath << "\n";
  OS << FileBuf->getBuffer();
}

llvm::ErrorOr<SourceFile *> SourceManager::OpenFile(llvm::StringRef AbsPath) {
  assert(OpenFiles.count(AbsPath) == 0 && "File is already open");
  auto Result = llvm::MemoryBuffer::getFile(AbsPath);
  if (auto EC = Result.getError()) {
    return EC;
  }

  auto [It, _] =
      OpenFiles.emplace(AbsPath, SourceFile(AbsPath, std::move(*Result)));
  return &It->second;
}

void SourceManager::debug(llvm::raw_ostream &OS) const {
  for (const auto &[Path, SF] : OpenFiles) {
    OS << Path << ":\n";
    SF.debug(OS);
  }
}

SourceLocation SourceLocation::Builtin() {
  return SourceLocation(nullptr, SrcRange::Builtin());
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &Os,
                              const SourceLocation &Loc) {
  return Os << Loc.loc();
}
} // namespace rx
