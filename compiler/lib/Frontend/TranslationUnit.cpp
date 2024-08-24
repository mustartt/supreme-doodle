#include "rxc/Frontend/TranslationUnit.h"

#include "rxc/AST/ASTPrinter.h"
#include "rxc/Basic/SourceManager.h"
#include "rxc/Parser/Parser.h"
#include "llvm/ADT/StringRef.h"

using namespace llvm;

namespace rx {

std::pair<llvm::StringRef, llvm::StringRef> SplitPackage(llvm::StringRef Pkg) {
  size_t First = Pkg.find_first_of(':');
  assert(First != std::string::npos && "Cannot find ':' separator");
  return std::make_pair(Pkg.substr(0, First), Pkg.substr(First + 1));
}

void TranslationUnit::parse() {
  parser::Parser P(Consumer, AstContext);
  ProgramAST = P.parse(File);
}

void TranslationUnit::debug(llvm::raw_ostream &OS) {
  assert(File && "Invalid file");
  File->debug(OS);
  if (ProgramAST) {
    ast::ASTPrinter Printer;
    Printer.print(OS, ProgramAST);
  }
  for (auto *TU : ImportedFiles) {
    OS << "Imported: " << TU->file()->getFilename() << "\n";
  }
}

ArrayRef<TranslationUnit *> TranslationUnit::getImportedFiles() {
  return ImportedFiles;
}

ArrayRef<ast::ImportDecl *> TranslationUnit::getImports() {
  assert(ProgramAST && "No AST Available");
  return ProgramAST->getImports();
}

void TranslationUnit::addImportedFiles(TranslationUnit *File) {
  ImportedFiles.push_back(File);
}

ast::ProgramDecl *TranslationUnit::getProgramAST() const {
  assert(ProgramAST && "TranslationUnit has not been parsed yet");
  return ProgramAST;
}
} // namespace rx
