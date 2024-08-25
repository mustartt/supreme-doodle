#include "rxc/AST/AST.h"
#include "rxc/AST/ASTVisitor.h"
#include "rxc/Basic/Diagnostic.h"
#include "rxc/Sema/LexicalScope.h"
#include "rxc/Sema/Sema.h"

namespace rx::sema {

using namespace ast;
using namespace llvm;

class ResolveGlobalTypePassImpl final : public ast::BaseDeclVisitor,
                                        ast::BaseTypeVisitor {
public:
  ResolveGlobalTypePassImpl(DiagnosticConsumer &DC, LexicalContext &LC)
      : DC(DC), LC(LC) {}
  ~ResolveGlobalTypePassImpl() = default;

  ResolveGlobalTypePassImpl(const ResolveGlobalTypePassImpl &) = delete;
  ResolveGlobalTypePassImpl(ResolveGlobalTypePassImpl &&) = default;
  ResolveGlobalTypePassImpl &
  operator=(const ResolveGlobalTypePassImpl &) = delete;
  ResolveGlobalTypePassImpl &operator=(ResolveGlobalTypePassImpl &&) = delete;

  void visit(ProgramDecl *Node) override;
private:
  // visit decls
  void visit(PackageDecl *Node) override {};
  void visit(ImportDecl *Node) override {};
  void visit(VarDecl *Node) override;
  void visit(TypeDecl *Node) override;
  void visit(ImplDecl *Node) override;
  void visit(UseDecl *Node) override;
  void visit(FuncDecl *Node) override;
  void visit(FuncParamDecl *Node) override;

  // visit types
  void visit(BuiltinType *Node) override;
  void visit(DeclRefType *Node) override;
  void visit(AccessType *Node) override;
  void visit(MutableType *Node) override;
  void visit(PointerType *Node) override;
  void visit(ArrayType *Node) override;
  void visit(FunctionType *Node) override;
  void visit(ObjectType *Node) override;
  void visit(EnumType *Node) override;

private:
  llvm::SmallVector<LexicalScope *> CurrentScope;
  DiagnosticConsumer &DC;
  LexicalContext &LC;
};

void ResolveGlobalType::run(ProgramDecl *Program, DiagnosticConsumer &DC,
                            LexicalContext &LC) {
  ResolveGlobalTypePassImpl Impl(DC, LC);
  Impl.visit(Program);
}

void ResolveGlobalTypePassImpl::visit(BuiltinType *Node) {
  assert(Node && "Invalid visited node");
}

void ResolveGlobalTypePassImpl::visit(DeclRefType *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Missing Lexical Scope");

  auto *LS = CurrentScope.back();

  auto Scope = LS->find(Node->getSymbol());
  if (!Scope) {
    Diagnostic Err(Diagnostic::Type::Error,
                   "Undefined type reference to " + Node->getTypeName());
    DC.emit(std::move(Err));
    return;
  }
  auto Decls = (*Scope)->getDecls(Node->getSymbol());
  assert(Decls.size() == 1 && "Ambiguous decl");
  auto *TypeDecl = dynamic_cast<ast::TypeDecl *>(Decls[0]);

  if (!TypeDecl) {
    Diagnostic Err(Diagnostic::Type::Error,
                   "Reference is not a type declaration " +
                       Node->getTypeName(), Node->Loc);
    Diagnostic Note(Diagnostic::Type::Note, "referenced declaration is <TODO>");
    DC.emit(std::move(Err));
    DC.emit(std::move(Note));
  }

  Node->setReferencedType(TypeDecl->getType());
  Node->setDeclNode(TypeDecl);
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

  CurrentScope.push_back(Node->getLexicalScope());
  for (auto *D : Node->getDecls()) {
    D->accept(*this);
  }
  CurrentScope.pop_back();
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
  assert(Node->getLexicalScope() && "should have Lexical Scope");
  CurrentScope.push_back(Node->getLexicalScope());

  Node->getType()->accept(*this);

  for (auto *I : Node->getImpls()) {
    I->accept(*this);
  }

  CurrentScope.pop_back();
}

void ResolveGlobalTypePassImpl::visit(FuncDecl *Node) {
  assert(Node && "Invalid visited node");

  assert(Node->getLexicalScope() && "should have Lexical Scope");
  CurrentScope.push_back(Node->getLexicalScope());
  for (auto *P : Node->getParams()) {
    P->accept(*this);
  }
  CurrentScope.pop_back();
}

void ResolveGlobalTypePassImpl::visit(FuncParamDecl *Node) {
  assert(Node && "Invalid visited node");
  if (Node->getType())
    Node->getType()->accept(*this);
}
} // namespace rx::sema
