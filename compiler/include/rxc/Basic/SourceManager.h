#ifndef BASIC_SOURCEMANAGER_H
#define BASIC_SOURCEMANAGER_H

#include <llvm/ADT/SmallString.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <memory>

namespace rx {

class SrcRange {
public:
  SrcRange();
  SrcRange(size_t LineStart, size_t ColStart, size_t LineEnd, size_t ColEnd);

  static SrcRange Builtin();
  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const SrcRange &loc);
  operator std::string() const;

  size_t LineStart;
  size_t ColStart;
  size_t LineEnd;
  size_t ColEnd;
};

class SourceFile;
class SourceLocation {
public:
  SourceLocation(SourceFile *File, SrcRange Loc) : File(File), Loc(Loc) {}

public:
  SourceFile *file() const { return File; }
  SrcRange loc() const { return Loc; }

private:
  SourceFile *File;
  SrcRange Loc;
};

class SourceFile {
public:
  using Path = llvm::SmallString<256>;

  SourceFile(llvm::StringRef AbsPath,
             std::unique_ptr<llvm::MemoryBuffer> FileBuf);

  SourceFile(const SourceFile &) = delete;
  SourceFile(SourceFile &&) = default;
  SourceFile &operator=(const SourceFile &) = delete;
  SourceFile &operator=(SourceFile &&) = default;
  ~SourceFile() = default;

public:
  llvm::StringRef getFilename() const;
  llvm::StringRef getBaseDir() const;
  llvm::StringRef getAbsPath() const;
  llvm::MemoryBufferRef getBuffer() const { return *FileBuf; }

  void debug(llvm::raw_ostream &OS) const;

private:
  Path AbsPath;
  std::unique_ptr<llvm::MemoryBuffer> FileBuf;
};

class SourceManager {
public:
  using AbsolutePath = SourceFile::Path;

public:
  SourceManager() = default;
  ~SourceManager() = default;

  SourceManager(const SourceManager &) = delete;
  SourceManager(SourceManager &&) = default;
  SourceManager &operator=(const SourceManager &) = delete;
  SourceManager &operator=(SourceManager &&) = default;

public:
  llvm::ErrorOr<SourceFile *> OpenFile(llvm::StringRef AbsPath);
  void debug(llvm::raw_ostream &OS) const;

private:
  std::map<AbsolutePath, SourceFile> OpenFiles;
};

} // namespace rx
#endif
