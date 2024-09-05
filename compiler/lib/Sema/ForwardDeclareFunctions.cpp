#include "rxc/AST/AST.h"
#include "rxc/AST/Type.h"
#include "rxc/AST/TypeContext.h"
#include "rxc/Basic/Diagnostic.h"
#include "rxc/Sema/LexicalScope.h"
#include "rxc/Sema/RecursiveASTVisitor.h"
#include "rxc/Sema/Sema.h"
#include <utility>

using namespace rx::ast;
using namespace llvm;

namespace rx::sema {

// resolve function overloads
class DeclareFunctionsAndImpl : public RecursiveASTVisitor<bool> {
public:
  DeclareFunctionsAndImpl(DiagnosticConsumer &DC, LexicalContext &LC,
                          TypeContext &TC)
      : RecursiveASTVisitor<bool>(LC), DC(DC), TC(TC) {}

  void start(ProgramDecl *P) { Visit(P); }

public:
  ResultType visit(FuncDecl *Node, LexicalScope *LS) override {
    auto *DeclaringLS = LS->parent();
    assert(DeclaringLS && "Missing declaring lexical scope");

    for (auto *Decl : DeclaringLS->getDecls(Node->getName())) {
      auto *F = dynamic_cast<FuncDecl *>(Decl);
      if (!F) {
        Diagnostic Err(Diagnostic::Type::Error,
                       "Function cannot have the same name as another "
                       "previously defined symbol'" +
                           Node->getName().str() + "'");
        Err.setSourceLocation(Node->getDeclLoc());
        Diagnostic Note(Diagnostic::Type::Note, "Previously declared here");
        Note.setSourceLocation(Decl->getDeclLoc());
        DC.emit(std::move(Err));
        DC.emit(std::move(Note));
        return {};
      }
      // F is a function with the same symbol. Now do overload resolution
      QualType OurFT = Node->getDeclaredType()->getType();
      if (OurFT.isUnknown())
        return {}; // bail out as we can't typecheck
      auto OtherFT = F->getDeclaredType()->getType();

      const auto *BaseFT = dynamic_cast<const FuncType *>(OtherFT.getType());
      const auto *OurT = dynamic_cast<const FuncType *>(OurFT.getType());
      assert(BaseFT && "Must be a function type");
      assert(OurT && "Must be a function type");

      if (BaseFT->getParamTypes() == OurT->getParamTypes()) {
        Diagnostic Err(Diagnostic::Type::Error,
                       "Overloaded function cannot have the same parameter "
                       "types as another function with the same name");
        Err.setSourceLocation(Node->getDeclLoc());
        Diagnostic Note(Diagnostic::Type::Note, "Previously declared here");
        Note.setSourceLocation(Decl->getDeclLoc());
        DC.emit(std::move(Err));
        DC.emit(std::move(Note));
        return {};
      }
    }

    DeclaringLS->insert(Node->getName(), Node);
    return true;
  }

  ResultType visit(ImplDecl *Node, LexicalScope *LS) override {
    QualType ImplT = Node->getDeclaredType()->getType();

    if (ImplT.isUnknown())
      return {};
    if (ImplT.hasQualifier()) {
      Diagnostic Err(Diagnostic::Type::Warning,
                     "impl can only define functions on unqualified types. "
                     "qualifiers will be ignored");
      Err.setSourceLocation(Node->getDeclLoc());
      DC.emit(std::move(Err));
    }

    const NamedType *NamedT = dynamic_cast<const NamedType *>(ImplT.getType());
    if (!NamedT) {
      Diagnostic Err(Diagnostic::Type::Error,
                     "you can only define impl on a type declaration");
      Err.setSourceLocation(Node->getDeclLoc());
      DC.emit(std::move(Err));
      return {};
    }

    for (auto *F : Node->getImpls()) {
      if (Visit(F)) {
        TC.addImpl(NamedT->getDecl(), F);
      }
    }
    return {};
  }

private:
  DiagnosticConsumer &DC;
  TypeContext &TC;
};

void ForwardDeclareFunctions::run(ProgramDecl *Program, DiagnosticConsumer &DC,
                                  LexicalContext &LC, ASTContext &AC,
                                  TypeContext &TC) {
  DeclareFunctionsAndImpl I1(DC, LC, TC);
  I1.start(Program);
}

} // namespace rx::sema
