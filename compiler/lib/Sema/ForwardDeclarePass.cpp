#include "rxc/AST/AST.h"
#include "rxc/AST/ASTVisitor.h"
#include "rxc/Basic/Diagnostic.h"
#include "rxc/Sema/LexicalContext.h"
#include "rxc/Sema/LexicalScope.h"
#include "rxc/Sema/Sema.h"
#include <string>

namespace rx::sema {

using namespace ast;
using namespace llvm;

class ForwardDeclarePassImpl final : public ast::BaseDeclVisitor {
public:
  ForwardDeclarePassImpl(DiagnosticConsumer &DC, LexicalContext &LC)
      : DC(DC), LC(LC) {}
  ~ForwardDeclarePassImpl() = default;

  ForwardDeclarePassImpl(const ForwardDeclarePassImpl &) = delete;
  ForwardDeclarePassImpl(ForwardDeclarePassImpl &&) = default;
  ForwardDeclarePassImpl &operator=(const ForwardDeclarePassImpl &) = delete;
  ForwardDeclarePassImpl &operator=(ForwardDeclarePassImpl &&) = delete;

  friend class ForwardDeclarePass;

private:
  // decl visitor
  void visit(ProgramDecl *Node) override;
  void visit(PackageDecl *Node) override {};
  void visit(ImportDecl *Node) override {};
  void visit(VarDecl *Node) override;
  void visit(TypeDecl *Node) override;
  void visit(ImplDecl *Node) override;
  void visit(UseDecl *Node) override;
  void visit(FuncDecl *Node) override;
  void visit(FuncParamDecl *Node) override;

private:
  llvm::SmallVector<LexicalScope *> CurrentScope;
  DiagnosticConsumer &DC;
  LexicalContext &LC;
};

void ForwardDeclarePass::run(ProgramDecl *Program, DiagnosticConsumer &DC,
                             LexicalContext &LC) {
  ForwardDeclarePassImpl Impl(DC, LC);
  Impl.visit(Program);
}

void ForwardDeclarePassImpl::visit(ProgramDecl *Node) {
  assert(Node && "Invalid visited node");

  auto *LS = LC.createNewScope(LexicalScope::Kind::File, LC.getGlobalScope());
  CurrentScope.push_back(LS);
  Node->setLexicalScope(LS);

  for (auto *D : Node->getDecls()) {
    D->accept(*this);
  }

  CurrentScope.pop_back();
}

void ForwardDeclarePassImpl::visit(VarDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Scope stack cannot be empty");
  auto *LS = CurrentScope.back();

  auto ExistingDecls = LS->getDecls(Node->getName());
  if (ExistingDecls.size()) {
    Diagnostic DupErr(Diagnostic::Type::Error,
                      "redefinition of global variable declaration \'" +
                          Node->getName().str() + "\'");
    DupErr.setSourceLocation(Node->getDeclLoc());
    Diagnostic Prev(Diagnostic::Type::Note, "previously declared here");
    Prev.setSourceLocation(ExistingDecls[0]->getDeclLoc());
    DC.emit(std::move(DupErr));
    DC.emit(std::move(Prev));
    return;
  }

  LS->insert(Node->getName(), Node);
}

void ForwardDeclarePassImpl::visit(TypeDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Scope stack cannot be empty");
  auto *LS = CurrentScope.back();

  auto ExistingDecls = LS->getDecls(Node->getName());
  if (ExistingDecls.size()) {
    Diagnostic DupErr(Diagnostic::Type::Error,
                      "redefinition of type declaration \'" +
                          Node->getName().str() + "\'");
    DupErr.setSourceLocation(Node->getDeclLoc());
    Diagnostic Prev(Diagnostic::Type::Note, "previously declared here");
    Prev.setSourceLocation(ExistingDecls[0]->getDeclLoc());

    DC.emit(std::move(DupErr));
    DC.emit(std::move(Prev));
    return;
  }
  LS->insert(Node->getName(), Node);
}

void ForwardDeclarePassImpl::visit(ImplDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Scope stack cannot be empty");

  auto *LS = CurrentScope.back();
  auto *ImplLS = LC.createNewScope(LexicalScope::Kind::Impl, LS);
  CurrentScope.push_back(ImplLS);
  Node->setLexicalScope(ImplLS);

  for (auto *I : Node->getImpls()) {
    I->accept(*this);
  }

  CurrentScope.pop_back();
}

void ForwardDeclarePassImpl::visit(UseDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Scope stack cannot be empty");
  auto *LS = CurrentScope.back();

  auto ExistingDecls = LS->getDecls(Node->getName());
  if (ExistingDecls.size()) {
    Diagnostic DupErr(Diagnostic::Type::Error,
                      "redefinition of type alias declaration \'" +
                          Node->getName().str() + "\'");
    DupErr.setSourceLocation(Node->getDeclLoc());
    Diagnostic Prev(Diagnostic::Type::Note, "previously declared here");
    Prev.setSourceLocation(ExistingDecls[0]->getDeclLoc());

    DC.emit(std::move(DupErr));
    DC.emit(std::move(Prev));
    return;
  }

  LS->insert(Node->getName(), Node);
}

void ForwardDeclarePassImpl::visit(FuncDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Scope stack cannot be empty");
  auto *LS = CurrentScope.back();
  LS->insert(Node->getName(), Node);

  auto *FuncLS = LC.createNewScope(LexicalScope::Kind::Function, LS);
  CurrentScope.push_back(FuncLS);
  Node->setLexicalScope(FuncLS);

  for (auto *FP : Node->getParams()) {
    FP->accept(*this);
  }

  CurrentScope.pop_back();
}

void ForwardDeclarePassImpl::visit(FuncParamDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Scope stack cannot be empty");
  auto *LS = CurrentScope.back();

  auto ExistingDecls = LS->getDecls(Node->getName());
  if (ExistingDecls.size()) {
    Diagnostic DupErr(Diagnostic::Type::Error,
                      "redefinition of function parameter declaration \'" +
                          Node->getName().str() + "\'");
    DupErr.setSourceLocation(Node->getDeclLoc());
    Diagnostic Prev(Diagnostic::Type::Note, "previously declared here");
    Prev.setSourceLocation(ExistingDecls[0]->getDeclLoc());

    DC.emit(std::move(DupErr));
    DC.emit(std::move(Prev));
    return;
  }

  LS->insert(Node->getName(), Node);
}

} // namespace rx::sema
