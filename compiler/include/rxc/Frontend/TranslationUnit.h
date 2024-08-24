#ifndef RXC_FRONTEND_TRANSLATIONUNIT_H
#define RXC_FRONTEND_TRANSLATIONUNIT_H

#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
#include "rxc/Basic/SourceManager.h"
#include "llvm/ADT/GraphTraits.h"

namespace rx {

class DiagnosticConsumer;
class TranslationUnit {
public:
  using IteratorType = llvm::ArrayRef<TranslationUnit *>::iterator;

  TranslationUnit(SourceFile *File, DiagnosticConsumer *Consumer)
      : File(File), Consumer(Consumer), ProgramAST(nullptr) {}

  TranslationUnit(const TranslationUnit &) = delete;
  TranslationUnit(TranslationUnit &&) = default;
  TranslationUnit &operator=(const TranslationUnit &) = delete;
  TranslationUnit &operator=(TranslationUnit &&) = default;
  ~TranslationUnit() = default;

public:
  void parse();
  void debug(llvm::raw_ostream &OS);
  SourceFile *file() const { return File; }
  llvm::ArrayRef<TranslationUnit *> getImportedFiles();
  llvm::ArrayRef<ast::ImportDecl *> getImports();
  void addImportedFiles(TranslationUnit *File);
  ast::ProgramDecl *getProgramAST() const;

private:
  SourceFile *File;
  DiagnosticConsumer *Consumer;
  ast::ASTContext AstContext;
  ast::ProgramDecl *ProgramAST;
  llvm::SmallVector<TranslationUnit *> ImportedFiles;
};

} // namespace rx

namespace llvm {
template <> struct GraphTraits<rx::TranslationUnit *> {
  using NodeRef = rx::TranslationUnit *;
  using ChildIteratorType = llvm::ArrayRef<rx::TranslationUnit *>::iterator;

  static NodeRef getEntryNode(rx::TranslationUnit *TU) { return TU; }

  static ChildIteratorType child_begin(NodeRef N) {
    return N->getImportedFiles().begin();
  }

  static ChildIteratorType child_end(NodeRef N) {
    return N->getImportedFiles().end();
  }
};
} // namespace llvm

#endif
