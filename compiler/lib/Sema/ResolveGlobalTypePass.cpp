#include "rxc/Sema/LexicalScope.h"
#include "rxc/AST/AST.h"
#include "rxc/AST/ASTVisitor.h"
#include "rxc/Sema/Sema.h"

namespace rx::sema {

using namespace ast;
using namespace llvm;

class ResolveGlobalTypePassImpl final : public ast::BaseDeclVisitor,
                                        ast::BaseTypeVisitor {
public:
  ResolveGlobalTypePassImpl() = default;
  ~ResolveGlobalTypePassImpl() = default;

  ResolveGlobalTypePassImpl(const ResolveGlobalTypePassImpl &) = delete;
  ResolveGlobalTypePassImpl(ResolveGlobalTypePassImpl &&) = default;
  ResolveGlobalTypePassImpl &
  operator=(const ResolveGlobalTypePassImpl &) = delete;
  ResolveGlobalTypePassImpl &operator=(ResolveGlobalTypePassImpl &&) = default;

private:
  // visit decls
  void visit(ProgramDecl *Node) override;
  void visit(PackageDecl *Node) override {};
  void visit(ImportDecl *Node) override {};
  void visit(VarDecl *Node) override;
  void visit(TypeDecl *Node) override;
  void visit(ImplDecl *Node) override;
  void visit(UseDecl *Node) override;
  void visit(FuncDecl *Node) override;
  void visit(FuncParamDecl *Node) override;

  // visit types
  void visit(DeclRefType *Node) override;
  void visit(AccessType *Node) override;
  void visit(MutableType *Node) override;
  void visit(PointerType *Node) override;
  void visit(ArrayType *Node) override;
  void visit(FunctionType *Node) override;
  void visit(ObjectType *Node) override;
  void visit(EnumType *Node) override;
};

void ResolveGlobalTypePassImpl::visit(DeclRefType *Node) {
  assert(Node && "Invalid visited node");
}

void ResolveGlobalTypePassImpl::visit(AccessType *Node) {
  assert(Node && "Invalid visited node");
  Node->getParentType()->accept(*this);
}

void ResolveGlobalTypePassImpl::visit(MutableType *Node) {
  assert(Node && "Invalid visited node");
  Node->getElementType()->accept(*this);
}

void ResolveGlobalTypePassImpl::visit(PointerType *Node) {
  assert(Node && "Invalid visited node");
  Node->getElementType()->accept(*this);
}

void ResolveGlobalTypePassImpl::visit(ArrayType *Node) {
  assert(Node && "Invalid visited node");
  Node->getElementType()->accept(*this);
}

void ResolveGlobalTypePassImpl::visit(FunctionType *Node) {
  assert(Node && "Invalid visited node");
  for (auto *P : Node->getParamTypes()) {
    P->accept(*this);
  }
  Node->getReturnType()->accept(*this);
}

void ResolveGlobalTypePassImpl::visit(ObjectType *Node) {
  assert(Node && "Invalid visited node");
  for (auto &[_, FieldType] : Node->getFields()) {
    FieldType->accept(*this);
  }
}

void ResolveGlobalTypePassImpl::visit(EnumType *Node) {
  assert(Node && "Invalid visited node");
  for (auto &[_, MemberType] : Node->getMembers()) {
    MemberType->accept(*this);
  }
}

void ResolveGlobalTypePassImpl::visit(ProgramDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(Node->getLexicalScope() && "Should have Lexical Scope");

  for (auto *D : Node->getDecls()) {
    D->accept(*this);
  }
}

void ResolveGlobalTypePassImpl::visit(VarDecl *Node) {
  assert(Node && "Invalid visited node");
  if (Node->getType()) {
    Node->getType()->accept(*this);
  }
}

void ResolveGlobalTypePassImpl::visit(TypeDecl *Node) {
  assert(Node && "Invalid visited node");
  if (Node->getType()) {
    Node->getType()->accept(*this);
  }
}

void ResolveGlobalTypePassImpl::visit(UseDecl *Node) {
  assert(Node && "Invalid visited node");
  if (Node->getType()) {
    Node->getType()->accept(*this);
  }
}

void ResolveGlobalTypePassImpl::visit(ImplDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(Node->getType() && "Impl must have a type");

  Node->getType()->accept(*this);

  for (auto *I : Node->getImpls()) {
    I->accept(*this);
  }
}

void ResolveGlobalTypePassImpl::visit(FuncDecl *Node) {
  assert(Node && "Invalid visited node");

  for (auto *P : Node->getParams()) {
    P->accept(*this);
  }
}

} // namespace rx::sema
