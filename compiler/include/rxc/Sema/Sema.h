
#ifndef SEMA_SEMA_H
#define SEMA_SEMA_H

#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
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
  virtual void run(ast::ProgramDecl *, DiagnosticConsumer &, LexicalContext &,
                   ast::ASTContext &) = 0;

public:
  std::string PassName;
};

class SemaPassManager {
public:
  SemaPassManager(DiagnosticConsumer &DC, LexicalContext &LC,
                  ast::ASTContext &AC, bool Debug = false)
      : DC(DC), LC(LC), AC(AC), Debug(Debug) {}

  template <class PassType> void registerPass(PassType &&Pass) {
    Passes.push_back(std::make_unique<PassType>(std::move(Pass)));
  }

  void run(ast::ProgramDecl *Root);

private:
  llvm::SmallVector<std::unique_ptr<SemaPass>, 8> Passes;
  DiagnosticConsumer &DC;
  LexicalContext &LC;
  ast::ASTContext &AC;
  bool Debug;
};

class ForwardDeclarePass : public SemaPass {
public:
  ForwardDeclarePass() : SemaPass("sema-forward-declare") {}

  void run(ast::ProgramDecl *, DiagnosticConsumer &, LexicalContext &,
           ast::ASTContext &) override;
};

class ResolveGlobalType : public SemaPass {
public:
  ResolveGlobalType() : SemaPass("sema-resolve-global-type") {}

  void run(ast::ProgramDecl *, DiagnosticConsumer &, LexicalContext &,
           ast::ASTContext &) override;
};

class MainSemaPass : public SemaPass {
public:
  MainSemaPass() : SemaPass("sema-main") {}

  void run(ast::ProgramDecl *, DiagnosticConsumer &, LexicalContext &,
           ast::ASTContext &) override;
};

} // namespace sema
} // namespace rx

#endif
