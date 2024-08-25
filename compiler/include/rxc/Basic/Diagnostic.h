#ifndef RXC_SEMA_DIAGNOSTIC_H
#define RXC_SEMA_DIAGNOSTIC_H

#include "rxc/Basic/SourceManager.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <optional>

namespace rx {

class Diagnostic {
public:
  enum class Type { Error, Warning, Note };

  Diagnostic(Type Kind, std::string Message,
             std::optional<SourceLocation> SrcLoc = std::nullopt);
  Diagnostic(const Diagnostic &) = delete;
  Diagnostic(Diagnostic &&) = default;
  Diagnostic &operator=(const Diagnostic &) = delete;
  Diagnostic &operator=(Diagnostic &&) = default;

public:
  Type kind() const { return Kind; }
  llvm::StringRef message() const { return Message; }
  std::optional<SourceLocation> loc() const { return SrcLoc; }
  void setSourceLocation(SourceLocation Loc) { SrcLoc = Loc; }

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

class ConsoleDiagnosticConsumer : public DiagnosticConsumer {
public:
  ConsoleDiagnosticConsumer() {}

public:
  void emit(Diagnostic &&D) override { printMessageHeader(D); };

private:
  void printMessageHeader(const Diagnostic &D);
  void printSourceLocation(const SourceLocation &Loc);

  static llvm::raw_ostream &PrintDiagnosticType(Diagnostic::Type Type);
  static llvm::raw_ostream &PadLineNumber(llvm::raw_ostream &Os, size_t Num,
                                          int PrefixLen = 4);
};
} // namespace rx

#endif
