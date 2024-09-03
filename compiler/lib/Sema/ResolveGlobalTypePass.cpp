#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
#include "rxc/AST/ASTVisitor.h"
#include "rxc/Basic/Diagnostic.h"
#include "rxc/Sema/LexicalScope.h"
#include "rxc/Sema/RecursiveASTVisitor.h"
#include "rxc/Sema/Sema.h"
#include <cassert>
#include <llvm/Support/WithColor.h>

namespace rx::sema {

using namespace ast;
using namespace llvm;

class ForwardDeclareType final : public RecursiveASTVisitor<> {
public:
  ForwardDeclareType(DiagnosticConsumer &DC, LexicalContext &LC)
      : RecursiveASTVisitor<>(LC), DC(DC) {}

  void start(ProgramDecl *P) { Visit(P); }

private:
  ResultType visit(ast::TypeDecl *Node, sema::LexicalScope *LS) override {
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
      return {};
    }

    LS->insert(Node->getName(), Node);
    return {};
  }

  ResultType visit(ast::UseDecl *Node, sema::LexicalScope *LS) override {
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
      return {};
    }

    LS->insert(Node->getName(), Node);
    return {};
  }

  ResultType visit(ast::BlockStmt *Node, sema::LexicalScope *LS) override {
    return {};
  }

private:
  DiagnosticConsumer &DC;
};

class ResolveTypeSymbol final : public RecursiveASTVisitor<> {
public:
  ResolveTypeSymbol(DiagnosticConsumer &DC, LexicalContext &LC)
      : RecursiveASTVisitor<>(LC), DC(DC) {}

  void start(ProgramDecl *P) { Visit(P); }

private:
  ResultType visit(ast::ASTDeclTypeRef *Node, LexicalScope *LS) override {
    auto Scope = LS->find(Node->getSymbol());
    if (!Scope) {
      Diagnostic Err(Diagnostic::Type::Error,
                     "Undefined type reference to " + Node->getTypeName());
      DC.emit(std::move(Err));
      return {};
    }
    auto Decls = (*Scope)->getDecls(Node->getSymbol());
    assert(Decls.size() == 1 && "Ambiguous decl");
    auto *TypeDecl = dynamic_cast<ast::TypeDecl *>(Decls[0]);
    auto *UseDecl = dynamic_cast<ast::UseDecl *>(Decls[0]);

    if (!TypeDecl && !UseDecl) {
      Diagnostic Err(Diagnostic::Type::Error,
                     "Reference is not a type or alias declaration " +
                         Node->getTypeName());
      Err.setSourceLocation(Node->Loc);
      Diagnostic Note(Diagnostic::Type::Note,
                      "referenced symbol is declared here");
      Err.setSourceLocation(Decls[0]->getDeclLoc());

      DC.emit(std::move(Err));
      DC.emit(std::move(Note));
      return {};
    }
    if (!TypeDecl)
      TypeDecl = UseDecl;
    assert(TypeDecl && "No TypeDecl");
    Node->setDeclNode(TypeDecl);
    return {};
  }

  ResultType visit(ast::BlockStmt *Node, sema::LexicalScope *LS) override {
    return {};
  }

private:
  DiagnosticConsumer &DC;
};

void ResolveGlobalType::run(ProgramDecl *Program, DiagnosticConsumer &DC,
                            LexicalContext &LC, ASTContext &AC) {
  ForwardDeclareType I1(DC, LC);
  ResolveTypeSymbol I2(DC, LC);
  I1.start(Program);
  I2.start(Program);
}

} // namespace rx::sema
