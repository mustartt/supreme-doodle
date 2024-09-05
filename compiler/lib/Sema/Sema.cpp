#include "rxc/Sema/Sema.h"
#include "rxc/AST/AST.h"
#include <llvm/Support/CommandLine.h>

using namespace llvm;

namespace rx::sema {

void SemaPassManager::run(ast::ProgramDecl *Root) {
  for (auto &Pass : Passes) {
    if (Debug)
      llvm::WithColor::remark()
          << "Running sema pass: " << Pass->PassName << "\n";
    Pass->run(Root, DC, LC, AC, TC);
  }
}

} // namespace rx::sema
