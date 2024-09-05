#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
#include "rxc/AST/ASTVisitor.h"
#include "rxc/Basic/Diagnostic.h"
#include "rxc/Sema/LexicalContext.h"
#include "rxc/Sema/LexicalScope.h"
#include "rxc/Sema/Sema.h"
#include <cassert>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/raw_ostream.h>

namespace rx::sema {

using namespace ast;
using namespace llvm;

enum class ResolutionMode { Expr, Function };
class MainSemaPassImpl final : public ast::BaseDeclVisitor,
                               public ast::BaseStmtVisitor,
                               public ast::BaseExprVisitor,
                               public ast::BaseTypeVisitor {
public:
  MainSemaPassImpl(DiagnosticConsumer &DC, LexicalContext &LC, ASTContext &AC)
      : DC(DC), LC(LC), AC(AC), ResMode(ResolutionMode::Expr) {}

public:
  // visit decls
  void visit(ProgramDecl *Node) override;
  void visit(PackageDecl *Node) override {};
  void visit(ImportDecl *Node) override {};
  void visit(ExportedDecl *Node) override;
  void visit(VarDecl *Node) override;
  void visit(TypeDecl *Node) override;
  void visit(ImplDecl *Node) override;
  void visit(UseDecl *Node) override;
  void visit(FuncDecl *Node) override;
  void visit(FuncParamDecl *Node) override;

  // visit types
  void visit(ASTBuiltinType *Node) override {}
  void visit(ASTDeclTypeRef *Node) override {}
  void visit(ASTAccessType *Node) override {}
  void visit(ASTQualType *Node) override {}
  void visit(ASTPointerType *Node) override {}
  void visit(ASTArrayType *Node) override {}
  void visit(ASTFunctionType *Node) override {}
  void visit(ASTObjectType *Node) override {}
  void visit(ASTEnumType *Node) override {}

  // visit expr
  void visit(DeclRefExpr *Node) override;
  void visit(IfExpr *Node) override {}
  void visit(BinaryExpr *Node) override;
  void visit(UnaryExpr *Node) override {}
  void visit(CallExpr *Node) override {}
  void visit(AccessExpr *Node) override {}
  void visit(IndexExpr *Node) override {}
  void visit(AssignExpr *Node) override {}
  void visit(ObjectLiteral *Node) override {}
  void visit(BoolLiteral *Node) override;
  void visit(CharLiteral *Node) override;
  void visit(NumLiteral *Node) override;
  void visit(StringLiteral *Node) override {}

  // visit stmts
  void visit(BlockStmt *Node) override;
  void visit(ReturnStmt *Node) override;
  void visit(DeclStmt *Node) override;
  void visit(ExprStmt *Node) override;
  void visit(ForStmt *Node) override {}

private:
  llvm::SmallVector<LexicalScope *> CurrentScope;
  DiagnosticConsumer &DC;
  LexicalContext &LC;
  ASTContext &AC;
  ResolutionMode ResMode;
  llvm::SmallVector<ASTType *, 16> TypeHintStack;
};

void MainSemaPass::run(ast::ProgramDecl *Program, DiagnosticConsumer &DC,
                       LexicalContext &LC, ASTContext &AC, TypeContext &TC) {
  //  MainSemaPassImpl Impl(DC, LC, AC);
  //  Impl.visit(Program);
}

// =========== Impls =============

/*

void MainSemaPassImpl::visit(DeclRefExpr *Node) {
  assert(Node && "Invalid visited node");
  assert(CurrentScope.size() && "Scope stack cannot be empty");
  auto *LS = CurrentScope.back();

  llvm::errs() << "here\n";

  auto Scope = LS->find(Node->getSymbol());
  if (!Scope) {
    Diagnostic Err(Diagnostic::Type::Error, "Undefined type reference to \'" +
                                                Node->getSymbol().str() + "\'");
    Err.setSourceLocation(Node->Loc);
    DC.emit(std::move(Err));
    return;
  }

  if (ResMode == ResolutionMode::Expr) {
    auto Decls = (*Scope)->getDecls(Node->getSymbol());
    assert(Decls.size() == 1 && "Ambiguous decl");
    Node->setRefDecl(Decls[0]);
    Node->setExprType(Decls[0]->getDeclaredType());
  } else {
    auto Decls = (*Scope)->getDecls(Node->getSymbol());
    Node->setRefDecl(Decls[0]);
    Node->setExprType(Decls[0]->getDeclaredType());

    if (Decls.size() > 1) {
      Diagnostic Warning(Diagnostic::Type::Warning,
                         "Overload resolution not implemented");
      Warning.setSourceLocation(Node->Loc);
      DC.emit(std::move(Warning));
    }
  }
}

void MainSemaPassImpl::visit(BinaryExpr *Node) {
  assert(Node && "Invalid visited node");
  Node->getLHS()->accept(*this);
  Node->getRHS()->accept(*this);
}

void MainSemaPassImpl::visit(BlockStmt *Node) {
  assert(Node && "Invalid visited node");
  assert(!Node->getLexicalScope() && "Already existing block scope");
  assert(CurrentScope.size() && "Scope stack cannot be empty");
  auto *LS = CurrentScope.back();
  auto BlockLS = LC.createNewScope(LexicalScope::Kind::Block, LS);
  Node->setLexicalScope(BlockLS);
  CurrentScope.push_back(BlockLS);
  for (auto *Stmt : Node->getStmts()) {
    Stmt->accept(*this);
  }
  CurrentScope.pop_back();
}

void MainSemaPassImpl::visit(DeclStmt *Node) {
  assert(Node && "Invalid visited node");
  Node->getDecl()->accept(*this);
}

void MainSemaPassImpl::visit(ReturnStmt *Node) {
  assert(Node && "Invalid visited node");
  Node->getExpr()->accept(*this);
}

void MainSemaPassImpl::visit(ExprStmt *Node) {
  assert(Node && "Invalid visited node");
  Node->getExpr()->accept(*this);
}

void MainSemaPassImpl::visit(ProgramDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(Node->getLexicalScope() && "Should have Lexical Scope");

  CurrentScope.push_back(Node->getLexicalScope());
  for (auto *D : Node->getDecls()) {
    D->accept(*this);
  }
  CurrentScope.pop_back();
}

void MainSemaPassImpl::visit(ExportedDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(Node->getExportedDecl() && "Invalid visited node");
  Node->getExportedDecl()->accept(*this);
}

void MainSemaPassImpl::visit(VarDecl *Node) {
  assert(Node && "Invalid visited node");

  if (Node->getDeclaredType())
    Node->getDeclaredType()->accept(*this);

  TypeHintStack.push_back(Node->getDeclaredType());
  if (Node->getInitializer())
    Node->getInitializer()->accept(*this);
  TypeHintStack.clear();

  auto *LS = CurrentScope.back();
  if (LS->getType() == LexicalScope::Kind::File)
    return;

  auto ExistingDecls = LS->getDecls(Node->getName());
  if (!ExistingDecls.size()) {
    LS->insert(Node->getName(), Node);
    return;
  }

  Diagnostic DupErr(Diagnostic::Type::Error,
                    "redefinition of variable declaration \'" +
                        Node->getName().str() + "\'");
  DupErr.setSourceLocation(Node->getDeclLoc());
  Diagnostic Prev(Diagnostic::Type::Note, "previously declared here");
  Prev.setSourceLocation(ExistingDecls[0]->getDeclLoc());
  DC.emit(std::move(DupErr));
  DC.emit(std::move(Prev));
}

void MainSemaPassImpl::visit(TypeDecl *Node) {
  assert(Node && "Invalid visited node");
  if (Node->getDeclaredType())
    Node->getDeclaredType()->accept(*this);

  auto *LS = CurrentScope.back();
  if (LS->getType() == LexicalScope::Kind::File)
    return;

  auto ExistingDecls = LS->getDecls(Node->getName());
  if (!ExistingDecls.size()) {
    LS->insert(Node->getName(), Node);
    return;
  }

  Diagnostic DupErr(Diagnostic::Type::Error,
                    "redefinition of type declaration \'" +
                        Node->getName().str() + "\'");
  DupErr.setSourceLocation(Node->getDeclLoc());
  Diagnostic Prev(Diagnostic::Type::Note, "previously declared here");
  Prev.setSourceLocation(ExistingDecls[0]->getDeclLoc());
  DC.emit(std::move(DupErr));
  DC.emit(std::move(Prev));
}

void MainSemaPassImpl::visit(UseDecl *Node) {
  assert(Node && "Invalid visited node");
  if (Node->getDeclaredType())
    Node->getDeclaredType()->accept(*this);

  auto *LS = CurrentScope.back();
  if (LS->getType() == LexicalScope::Kind::File)
    return;

  auto ExistingDecls = LS->getDecls(Node->getName());
  if (!ExistingDecls.size()) {
    LS->insert(Node->getName(), Node);
    return;
  }

  Diagnostic DupErr(Diagnostic::Type::Error,
                    "redefinition of type alias declaration \'" +
                        Node->getName().str() + "\'");
  DupErr.setSourceLocation(Node->getDeclLoc());
  Diagnostic Prev(Diagnostic::Type::Note, "previously declared here");
  Prev.setSourceLocation(ExistingDecls[0]->getDeclLoc());
  DC.emit(std::move(DupErr));
  DC.emit(std::move(Prev));
}

void MainSemaPassImpl::visit(ImplDecl *Node) {
  assert(Node && "Invalid visited node");
  assert(Node->getDeclaredType() && "Impl must have a type");
  assert(Node->getLexicalScope() && "should have Lexical Scope");

  CurrentScope.push_back(Node->getLexicalScope());
  for (auto *I : Node->getImpls()) {
    I->accept(*this);
  }
  CurrentScope.pop_back();
}

void MainSemaPassImpl::visit(FuncDecl *Node) {
  assert(Node && "Invalid visited node");

  assert(Node->getLexicalScope() && "should have Lexical Scope");
  CurrentScope.push_back(Node->getLexicalScope());
  for (auto *P : Node->getParams()) {
    P->accept(*this);
  }
  assert(Node->getBody());
  Node->getBody()->accept(*this);
  CurrentScope.pop_back();
}

void MainSemaPassImpl::visit(FuncParamDecl *Node) {
  assert(Node && "Invalid visited node");
  if (Node->getDeclaredType())
    Node->getDeclaredType()->accept(*this);
}

// Literals

void MainSemaPassImpl::visit(BoolLiteral *Node) {
  assert(Node && "Invalid visited node");
  assert(LC.getGlobalScope()->getDecls("bool").size() == 1 &&
         "Found multiple BuiltinType bool");
  Node->setExprType(LC.getGlobalScope()->getDecls("bool")[0]->getDeclaredType());
  assert(Node->getExprType() && "Did not set type");
}

void MainSemaPassImpl::visit(CharLiteral *Node) {
  assert(Node && "Invalid visited node");
  assert(LC.getGlobalScope()->getDecls("char").size() == 1 &&
         "Found multiple BuiltinType char");
  Node->setExprType(LC.getGlobalScope()->getDecls("char")[0]->getDeclaredType());
  assert(Node->getExprType() && "Did not set type");
}

void MainSemaPassImpl::visit(NumLiteral *Node) {
  assert(Node && "Invalid visited node");
  Node->getValue();
}

*/

} // namespace rx::sema
