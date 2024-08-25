
#ifndef SEMA_SEMA_H
#define SEMA_SEMA_H

#include "rxc/AST/AST.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/WithColor.h>
#include <memory>

namespace rx {

class DiagnosticConsumer;

namespace sema {

class LexicalContext;

class SemaPass {
public:
  SemaPass(std::string PassName) : PassName(std::move(PassName)) {}
  virtual ~SemaPass() = default;
  virtual void run(ast::ProgramDecl *, DiagnosticConsumer &,
                   LexicalContext &) = 0;

public:
  std::string PassName;
};

class SemaPassManager {
public:
  SemaPassManager(DiagnosticConsumer &DC, LexicalContext &LC)
      : DC(DC), LC(LC) {}

  template <class PassType> void registerPass(PassType &&Pass) {
    Passes.push_back(std::make_unique<PassType>(std::move(Pass)));
  }

  void run(ast::ProgramDecl *Root) {
    for (auto &Pass : Passes) {
      llvm::WithColor::remark()
          << "Running sema pass: " << Pass->PassName << "\n";
      Pass->run(Root, DC, LC);
    }
  }

private:
  llvm::SmallVector<std::unique_ptr<SemaPass>, 8> Passes;
  DiagnosticConsumer &DC;
  LexicalContext &LC;
};

class ForwardDeclarePass : public SemaPass {
public:
  ForwardDeclarePass() : SemaPass("sema-forward-declare") {}

  void run(ast::ProgramDecl *, DiagnosticConsumer &, LexicalContext &) override;
};

class ResolveGlobalType : public SemaPass {
public:
  ResolveGlobalType() : SemaPass("sema-resolve-global-type") {}

  void run(ast::ProgramDecl *, DiagnosticConsumer &, LexicalContext &) override;
};

} // namespace sema
} // namespace rx

#endif
