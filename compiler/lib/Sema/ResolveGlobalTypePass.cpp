#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
#include "rxc/AST/ASTVisitor.h"
#include "rxc/AST/QualType.h"
#include "rxc/AST/Type.h"
#include "rxc/AST/TypeContext.h"
#include "rxc/Basic/Diagnostic.h"
#include "rxc/Sema/LexicalContext.h"
#include "rxc/Sema/LexicalScope.h"
#include "rxc/Sema/RecursiveASTVisitor.h"
#include "rxc/Sema/Sema.h"
#include <cassert>
#include <llvm/Support/ErrorHandling.h>
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
      Diagnostic Err(Diagnostic::Type::Error, "Undefined type reference to '" +
                                                  Node->getSymbol().str() +
                                                  "'");
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
    if (TypeDecl) {
      Node->setDeclNode(TypeDecl);
    } else {
      Node->setDeclNode(UseDecl);
    }
    return {};
  }

  ResultType visit(ast::BlockStmt *Node, sema::LexicalScope *LS) override {
    return {};
  }

private:
  DiagnosticConsumer &DC;
};

// probably want to check for recursive types
class MaterializeType : public RecursiveASTVisitor<QualType> {
public:
  MaterializeType(LexicalContext &LC, TypeContext &TC, DiagnosticConsumer &DC)
      : RecursiveASTVisitor<QualType>(LC), TC(TC), LC(LC), DC(DC) {}

public:
  void start(ProgramDecl *P) { Visit(P); }

public:
  QualType visit(ast::ASTBuiltinType *Node, sema::LexicalScope *LS) override {
    QualType T;
    switch (Node->getNativeType()) {
    case ASTNativeType::Void:
      T = TC.getUnitType();
      break;
    case ASTNativeType::i1:
      T = TC.getBuiltinType(NativeType::i1);
      break;
    case ASTNativeType::i8:
      T = TC.getBuiltinType(NativeType::i8);
      break;
    case ASTNativeType::i32:
      T = TC.getBuiltinType(NativeType::i32);
      break;
    case ASTNativeType::i64:
      T = TC.getBuiltinType(NativeType::i64);
      break;
    case ASTNativeType::f32:
      T = TC.getBuiltinType(NativeType::f32);
      break;
    case ASTNativeType::f64:
      T = TC.getBuiltinType(NativeType::f64);
      break;
    case ASTNativeType::String:
      T = TC.getBuiltinType(NativeType::string);
      break;
    case ASTNativeType::Unknown:
      T = TC.getUnknownType();
      break;
    }
    Node->setType(T);
    return T;
  }

  QualType visit(ast::ASTDeclTypeRef *Node, sema::LexicalScope *LS) override {
    TypeDecl *Curr = Node->getDeclNode();
    if (!Curr)
      return TC.getUnknownType();

    // Type Decl so we can get the concrete type
    if (!dynamic_cast<UseDecl *>(Curr)) {
      QualType T = TC.getNamedType(Curr);
      Node->setType(T);
      return T;
    }

    // UseDecl then we get looked up value for the alisa
    QualType T = Visit(Curr->getDeclaredType());
    Node->setType(T);
    return T;
  }

  QualType visit(ast::ASTAccessType *Node, sema::LexicalScope *LS) override {
    // meant for module type
    llvm_unreachable("Not implemented");
  }

  QualType visit(ast::ASTQualType *Node, sema::LexicalScope *LS) override {
    assert(Node->getElementType());
    auto T = Visit(Node->getElementType());
    if (T.isUnknown())
      return T;
    Node->setType(T);
    return T;
  }

  QualType visit(ast::ASTPointerType *Node, sema::LexicalScope *LS) override {
    assert(Node->getElementType() && "Invalid AST Node");
    auto T = Visit(Node->getElementType());
    if (T.isUnknown())
      return T;
    auto PointerTy = TC.getPointerType(T);
    Node->setType(PointerTy);
    return PointerTy;
  }

  QualType visit(ast::ASTArrayType *Node, sema::LexicalScope *LS) override {
    assert(Node->getElementType() && "Invalid AST Node");
    auto T = Visit(Node->getElementType());
    if (T.isUnknown())
      return T;
    auto ArrayTy = TC.getArrayType(T);
    Node->setType(ArrayTy);
    return ArrayTy;
  }

  QualType visit(ast::ASTFunctionType *Node, sema::LexicalScope *LS) override {
    llvm::SmallVector<QualType> ParamTys;
    for (auto *P : Node->getParamTypes()) {
      assert(P && "Invalid Visiting Type");
      auto T = Visit(P);
      if (T.isUnknown())
        return T;
      ParamTys.push_back(T);
    }
    assert(Node->getReturnType() && "Invalid AST Node");
    auto RetTy = Visit(Node->getReturnType());
    if (RetTy.isUnknown())
      return RetTy;

    auto FuncTy = TC.getFuncType(ParamTys, RetTy);
    Node->setType(FuncTy);
    return FuncTy;
  }

  QualType visit(ast::ASTObjectType *Node, sema::LexicalScope *LS) override {
    llvm::StringMap<QualType> Fields;
    for (auto &F : Node->getFields()) {
      assert(F.second);
      auto T = Visit(F.second);
      if (T.isUnknown())
        return T;
      if (Fields.contains(F.first)) {
        Diagnostic Err(Diagnostic::Type::Error,
                       "Duplicate field '" + F.first + "' in object type");
        Err.setSourceLocation(Node->Loc);
        DC.emit(std::move(Err));
        continue;
      }
      Fields[F.first] = T;
    }
    auto ObjectTy = TC.getObjectType(std::move(Fields));
    Node->setType(ObjectTy);
    return ObjectTy;
  }

  QualType visit(ast::ASTEnumType *Node, sema::LexicalScope *LS) override {
    llvm::StringMap<QualType> Members;
    for (auto &M : Node->getMembers()) {
      assert(M.second);
      auto T = Visit(M.second);
      if (T.isUnknown())
        return T;
      if (Members.contains(M.first)) {
        Diagnostic Err(Diagnostic::Type::Error,
                       "Duplicate member '" + M.first + "' in enum type");
        Err.setSourceLocation(Node->Loc);
        DC.emit(std::move(Err));
        continue;
      }
      Members[M.first] = T;
    }
    auto EnumTy = TC.getEnumType(std::move(Members));
    Node->setType(EnumTy);
    return EnumTy;
  }

  QualType visit(ast::BlockStmt *Node, sema::LexicalScope *LS) override {
    return {};
  }

private:
  TypeContext &TC;
  LexicalContext &LC;
  DiagnosticConsumer &DC;
};

void ResolveGlobalType::run(ProgramDecl *Program, DiagnosticConsumer &DC,
                            LexicalContext &LC, ASTContext &AC,
                            TypeContext &TC) {
  ForwardDeclareType I1(DC, LC);
  ResolveTypeSymbol I2(DC, LC);
  MaterializeType I3(LC, TC, DC);

  I1.start(Program);
  I2.start(Program);
  I3.start(Program);
}

} // namespace rx::sema
