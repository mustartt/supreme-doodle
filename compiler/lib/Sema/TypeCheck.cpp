
#include "rxc/AST/AST.h"
#include "rxc/AST/QualType.h"
#include "rxc/AST/Type.h"
#include "rxc/AST/TypeContext.h"
#include "rxc/Basic/Diagnostic.h"
#include "rxc/Basic/SourceManager.h"
#include "rxc/Sema/LexicalScope.h"
#include "rxc/Sema/RecursiveASTVisitor.h"
#include "rxc/Sema/Sema.h"
#include <llvm/ADT/SmallVector.h>

using namespace rx::ast;
using namespace llvm;

namespace rx::sema {

class TypeCheckImpl final : public RecursiveASTVisitor<QualType> {
public:
  TypeCheckImpl(DiagnosticConsumer &DC, LexicalContext &LC, TypeContext &TC)
      : RecursiveASTVisitor<QualType>(LC), DC(DC), TC(TC) {}

  void start(ProgramDecl *P) { Visit(P); }

private:
  struct HintCtxExit {
    HintCtxExit(SmallVectorImpl<QualType> &Impl, int n) : Stack(Impl), n(n) {}
    ~HintCtxExit() { Stack.pop_back_n(n); }

  private:
    SmallVectorImpl<QualType> &Stack;
    int n;
  };

  HintCtxExit pushTypeHint(QualType T) {
    int n = HintStack.size();
    HintStack.push_back(T);
    return HintCtxExit(HintStack, n);
  }

  bool checkAndEmitRedeclaration(Decl *Node, LexicalScope *LS) {
    if (auto Decls = LS->getDecls(Node->getName()); Decls.size()) {
      Diagnostic Err(Diagnostic::Type::Error, "Redefinition of declaration '" +
                                                  Node->getName().str() + "'");
      Err.setSourceLocation(Node->getDeclLoc());
      Diagnostic Note(Diagnostic::Type::Note, "Previously declared here");
      Err.setSourceLocation(Decls[0]->getDeclLoc());
      DC.emit(std::move(Err));
      DC.emit(std::move(Note));
      return false;
    }
    return true;
  }

  QualType checkAndDeduceVarInit(SourceLocation Loc, QualType DeclTy,
                                 Expression *Default) {
    assert(!DeclTy.isUnknown() ||
           Default &&
               "Type or initializer must exists to perform type deduction");
    auto _ = pushTypeHint(DeclTy);
    if (Default) {
      auto InitializerTy = Visit(Default);
      if (DeclTy.isUnknown())
        DeclTy = InitializerTy;
      // type check
      if (DeclTy.getType() != InitializerTy.getType()) {
        Diagnostic Err(Diagnostic::Type::Error,
                       "cannot initialize variable of type '" +
                           DeclTy.getTypeName() +
                           "' with initializer of type '" +
                           InitializerTy.getTypeName() + "'");
        Err.setSourceLocation(Loc);
        DC.emit(std::move(Err));
        return {};
      }
    }
    return DeclTy;
  }

private:
  QualType visit(VarDecl *Node, LexicalScope *LS) override {
    if (!checkAndEmitRedeclaration(Node, LS))
      return {};

    if (!Node->getInitializer() && !Node->getDeclaredType()) {
      Diagnostic Err(
          Diagnostic::Type::Error,
          "Cannot default initialize without and initializer and type hint");
      Err.setSourceLocation(Node->getDeclLoc());
      DC.emit(std::move(Err));
      return {};
    }

    QualType DeclTy;
    if (auto *T = Node->getDeclaredType()) {
      if (T->getType().isUnknown())
        return {};
      DeclTy = T->getType();
    }

    DeclTy = checkAndDeduceVarInit(Node->getDeclLoc(), DeclTy,
                                   Node->getInitializer());
    LS->insert(Node->getName(), Node);

    return DeclTy;
  }

  QualType visit(TypeDecl *, LexicalScope *LS) override { return {}; }
  QualType visit(UseDecl *, LexicalScope *LS) override { return {}; }

  QualType visit(FuncParamDecl *Node, LexicalScope *LS) override {
    if (!checkAndEmitRedeclaration(Node, LS))
      return {};

    if (!Node->getDefaultValue() && !Node->getDeclaredType()) {
      Diagnostic Err(
          Diagnostic::Type::Error,
          "Cannot default initialize without and initializer and type hint");
      Err.setSourceLocation(Node->getDeclLoc());
      DC.emit(std::move(Err));
      return {};
    }

    QualType DeclTy;
    if (auto *T = Node->getDeclaredType()) {
      if (T->getType().isUnknown())
        return {};
      DeclTy = T->getType();
    }

    DeclTy = checkAndDeduceVarInit(Node->getDeclLoc(), DeclTy,
                                   Node->getDefaultValue());
    LS->insert(Node->getName(), Node);
    return DeclTy;
  }

  QualType visit(BoolLiteral *Node, LexicalScope *LS) override {
    auto T = TC.getBuiltinType(NativeType::i1);
    Node->setExprType(T);
    return T;
  }

  QualType visit(CharLiteral *Node, LexicalScope *LS) override {
    auto T = TC.getBuiltinType(NativeType::i8);
    Node->setExprType(T);
    return T;
  }

  QualType deduceNumericType(NumLiteral *Node, NativeType NT) {
    if (!Node->isInteger()) {
      switch (NT) {
      case NativeType::i1: {
        Diagnostic Err(
            Diagnostic::Type::Error,
            "Type deduction failed, cannot implicitly convert a floating point "
            "literal to a i1");
        Err.setSourceLocation(Node->Loc);
        DC.emit(std::move(Err));
        return {};
      }
      case NativeType::i8: {
        Diagnostic Err(
            Diagnostic::Type::Error,
            "Type deduction failed, cannot implicitly convert a floating point "
            "literal to a i8");
        Err.setSourceLocation(Node->Loc);
        DC.emit(std::move(Err));
        return {};
      }
      case NativeType::i16: {
        Diagnostic Err(
            Diagnostic::Type::Error,
            "Type deduction failed, cannot implicitly convert a floating point "
            "literal to a i16");
        Err.setSourceLocation(Node->Loc);
        DC.emit(std::move(Err));
        return {};
      }
      case NativeType::i32: {
        Diagnostic Err(
            Diagnostic::Type::Error,
            "Type deduction failed, cannot implicitly convert a floating point "
            "literal to a i32");
        Err.setSourceLocation(Node->Loc);
        DC.emit(std::move(Err));
        return {};
      }
      case NativeType::i64: {
        Diagnostic Err(
            Diagnostic::Type::Error,
            "Type deduction failed, cannot implicitly convert a floating point "
            "literal to a i64");
        Err.setSourceLocation(Node->Loc);
        DC.emit(std::move(Err));
        return {};
      }
      case NativeType::f32:
        return TC.getBuiltinType(NativeType::f32);
      case NativeType::f64:
        return TC.getBuiltinType(NativeType::f64);
      default:
        Diagnostic Err(Diagnostic::Type::Error,
                       "Type deduction failed, trying to create numeric type "
                       "from non-numeric builtin type");
        Err.setSourceLocation(Node->Loc);
        DC.emit(std::move(Err));
        return {};
      }
    } else {
      switch (NT) {
      case NativeType::i1:
        return TC.getBuiltinType(NativeType::i1);
      case NativeType::i8:
        return TC.getBuiltinType(NativeType::i1);
      case NativeType::i16:
        return TC.getBuiltinType(NativeType::i16);
      case NativeType::i32:
        return TC.getBuiltinType(NativeType::i32);
      case NativeType::i64:
        return TC.getBuiltinType(NativeType::i64);
      case NativeType::f32:
        return TC.getBuiltinType(NativeType::f32);
      case NativeType::f64:
        return TC.getBuiltinType(NativeType::f64);
      default:
        Diagnostic Err(Diagnostic::Type::Error,
                       "Type deduction failed, trying to create numeric type "
                       "from non-numeric builtin type");
        Err.setSourceLocation(Node->Loc);
        DC.emit(std::move(Err));
        return {};
      }
    }
  }

  QualType visit(NumLiteral *Node, LexicalScope *LS) override {
    QualType Hint = HintStack.back();
    QualType T;

    if (Hint.isUnknown()) {
      T = Node->isInteger() ? TC.getBuiltinType(NativeType::i64)
                            : TC.getBuiltinType(NativeType::f64);
    } else {
      const auto *BaseType = dynamic_cast<const BuiltinType *>(Hint.getType());
      if (BaseType) {
        T = deduceNumericType(Node, BaseType->getNativeType());
      }
    }
    Node->setExprType(T);
    return T;
  }

  QualType visit(ast::StringLiteral *Node, LexicalScope *LS) override {
    auto T = TC.getBuiltinType(NativeType::string);
    Node->setExprType(T);
    return T;
  }

private:
  DiagnosticConsumer &DC;
  TypeContext &TC;

private:
  llvm::SmallVector<QualType, 16> HintStack;
};

void TypeCheck::run(ast::ProgramDecl *Program, DiagnosticConsumer &DC,
                    LexicalContext &LC, ast::ASTContext &AC, TypeContext &TC) {
  TypeCheckImpl I(DC, LC, TC);
  I.start(Program);
}

} // namespace rx::sema
