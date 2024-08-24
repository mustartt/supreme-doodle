#ifndef RXC_SEMA_DIAGNOSTIC_H
#define RXC_SEMA_DIAGNOSTIC_H

#include "rxc/Basic/SourceManager.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>
#include <optional>

namespace rx {

class Diagnostic {
public:
  enum class Type { Error, Warning, Note };

  Diagnostic(Type Kind, std::string Message,
             std::optional<SourceLocation> SrcLoc = std::nullopt)
      : SrcLoc(std::move(SrcLoc)), Kind(Kind), Message(std::move(Message)) {}
  Diagnostic(const Diagnostic &) = delete;
  Diagnostic(Diagnostic &&) = default;
  Diagnostic &operator=(const Diagnostic &) = delete;
  Diagnostic &operator=(Diagnostic &&) = default;

public:
  Type kind() const { return Kind; }
  llvm::StringRef message() const { return Message; }
  std::optional<SourceLocation> loc() const { return SrcLoc; }

private:
  std::optional<SourceLocation> SrcLoc;
  Type Kind;
  std::string Message;
};

class DiagnosticConsumer {
public:
  virtual ~DiagnosticConsumer() = default;
  virtual void emit(Diagnostic &&D) = 0;
};

// Emit:
// file.rx:
class ConsoleDiagnosticConsumer : public DiagnosticConsumer {
public:
  ConsoleDiagnosticConsumer() {}

public:
  void emit(Diagnostic &&D) override { printMessageHeader(D); };

private:
  void printMessageHeader(const Diagnostic &D) {
    if (D.loc()) {
      auto Loc = D.loc().value();
      llvm::WithColor::remark()
          << Loc.file()->getAbsPath() << ": " << Loc.loc() << ": ";
    }
    PrintDiagnosticType(D.kind());
    llvm::WithColor::remark() << D.message() << "\n";
  }

  static llvm::raw_ostream &PrintDiagnosticType(Diagnostic::Type Type) {
    switch (Type) {
    case Diagnostic::Type::Error:
      return llvm::WithColor::error() << "Error: ";
    case Diagnostic::Type::Warning:
      return llvm::WithColor::warning() << "Warning: ";
    case Diagnostic::Type::Note:
      return llvm::WithColor::note() << "Note: ";
    default:
      llvm_unreachable("Invalid diagnostic type");
    }
  }
};
} // namespace rx

#endif
