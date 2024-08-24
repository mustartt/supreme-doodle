#include "rxc/Frontend/TranslationUnitContext.h"
#include "rxc/Basic/Diagnostic.h"

#include "llvm/Support/Path.h"

#include <queue>

using namespace llvm;

namespace rx {

TranslationUnit *TranslationUnitContext::setRootFile(SourceFile *File) {
  assert(File && "Invalid File");
  assert(OpenFiles.empty() && "FileContext is not empty");

  OpenFiles.emplace(File->getAbsPath(), TranslationUnit(File, &DC));
  return &OpenFiles.at(File->getAbsPath());
}

void TranslationUnitContext::traverseFileImports(TranslationUnit *Start) {
  std::queue<TranslationUnit *> Queue;
  Queue.push(Start);

  while (!Queue.empty()) {
    auto *File = std::move(Queue.front());
    File->parse();
    Queue.pop();

    Path BaseDir = File->file()->getBaseDir();
    for (auto Import : File->getImports()) {
      if (Import->getImportType() != ast::ImportDecl::ImportType::File)
        continue;

      Path AbsFilePath = BaseDir;
      sys::path::append(AbsFilePath, Import->getImportPath());

      if (!OpenFiles.count(AbsFilePath)) {
        auto Result = SM.OpenFile(AbsFilePath);
        if (auto EC = Result.getError()) {
          Diagnostic FailedImport(Diagnostic::Type::Error, EC.message());
          DC.emit(std::move(FailedImport));

          continue;
        }
        OpenFiles.emplace(AbsFilePath, TranslationUnit(*Result, &DC));
        Queue.push(&OpenFiles.at(AbsFilePath));
      }
      File->addImportedFiles(&OpenFiles.at(AbsFilePath));
    }
  }
}

void TranslationUnitContext::debug(llvm::raw_ostream &OS) {
  for (auto &[P, SF] : OpenFiles) {
    OS << P << ":\n";
    SF.debug(OS);
    OS << "\n";
  }
}

} // namespace rx
