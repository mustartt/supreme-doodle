#include "LexicalScope.h"
#include "rxc/AST/AST.h"
#include "rxc/AST/ASTVisitor.h"
#include "rxc/Sema/Sema.h"
#include <deque>

namespace rx::sema {

using namespace ast;
using namespace llvm;

class ForwardDeclarePassImpl final : public ast::BaseDeclVisitor {
public:
  ForwardDeclarePassImpl() = default;
  ~ForwardDeclarePassImpl() = default;

  ForwardDeclarePassImpl(const ForwardDeclarePassImpl &) = delete;
  ForwardDeclarePassImpl(ForwardDeclarePassImpl &&) = default;
  ForwardDeclarePassImpl &operator=(const ForwardDeclarePassImpl &) = delete;
  ForwardDeclarePassImpl &operator=(ForwardDeclarePassImpl &&) = default;

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
  LexicalScope *createNewScope(LexicalScope::Kind Type,
                               LexicalScope *Parent = nullptr) {
    return &ScopeContext.emplace_back(Parent, Type);
  }

  void dump() const {
    for (const auto &Scope : ScopeContext) {
      Scope.dump();
    }
  }

  std::deque<LexicalScope> ScopeContext;
  llvm::SmallVector<LexicalScope *> CurrentScope;
};

void ForwardDeclarePass::run(ProgramDecl *Program) {
  ForwardDeclarePassImpl Impl;
  Impl.visit(Program);
  Impl.dump();
}

void ForwardDeclarePassImpl::visit(ProgramDecl *Node) {
  assert(Node && "Invalid visited node");
  auto *LS = createNewScope(LexicalScope::Kind::Module);
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

  LS->insert(Node->getName(), Node);
}

void ForwardDeclarePassImpl::visit(TypeDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Scope stack cannot be empty");
  auto *LS = CurrentScope.back();
  LS->insert(Node->getName(), Node);
}

void ForwardDeclarePassImpl::visit(ImplDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Scope stack cannot be empty");

  auto *LS = CurrentScope.back();
  auto *ImplLS = createNewScope(LexicalScope::Kind::Impl, LS);
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
  LS->insert(Node->getName(), Node);
}

void ForwardDeclarePassImpl::visit(FuncDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Scope stack cannot be empty");
  auto *LS = CurrentScope.back();
  LS->insert(Node->getName(), Node);

  auto *FuncLS = createNewScope(LexicalScope::Kind::Function, LS);
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
  LS->insert(Node->getName(), Node);
}

} // namespace rx::sema
