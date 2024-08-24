
#ifndef SEMA_SEMA_H
#define SEMA_SEMA_H

#include "rxc/AST/AST.h"
#include "rxc/Basic/Diagnostic.h"
#include <llvm/ADT/SmallVector.h>
#include <memory>

namespace rx::sema {

class SemaPass {
public:
  SemaPass(std::string PassName) : PassName(std::move(PassName)) {}
  virtual ~SemaPass() = default;
  virtual void run(ast::ProgramDecl *) = 0;

private:
  std::string PassName;
};

class SemaPassManager {
public:
  SemaPassManager(DiagnosticConsumer &DC) : DC(DC) {}

  template <class PassType> void registerPass(PassType &&Pass) {
    Passes.push_back(std::make_unique<PassType>(std::move(Pass)));
  }

  void run(ast::ProgramDecl *Root) {
    for (auto &Pass : Passes) {
      Pass->run(Root);
    }
  }

private:
  llvm::SmallVector<std::unique_ptr<SemaPass>, 8> Passes;
  DiagnosticConsumer &DC;
};

class ForwardDeclarePass : public SemaPass {
public:
  ForwardDeclarePass() : SemaPass("sema-forward-declare") {}

  void run(ast::ProgramDecl *) override;
};

class ResolveGlobalType : public SemaPass {
public:
  ResolveGlobalType() : SemaPass("sema-resolve-global-type") {}

  void run(ast::ProgramDecl *) override;
};

} // namespace rx::sema

#endif
