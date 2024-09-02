#ifndef PARSER_PARSETREEVISITOR_H
#define PARSER_PARSETREEVISITOR_H

#include "LangParserBaseVisitor.h"
#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
#include "rxc/Basic/Diagnostic.h"
#include "rxc/Basic/SourceManager.h"
#include "llvm/ADT/SmallVector.h"

#include <any>
#include <cassert>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/ErrorHandling.h>

using namespace antlr4;

namespace rx::parser {

class LangVisitor : public LangParserBaseVisitor {
public:
  LangVisitor(TokenStream &Stream, rx::ast::ASTContext &Context,
              SourceFile *File, DiagnosticConsumer &DC)
      : Tokens(Stream), Context(Context), File(File), DC(DC) {}

public:
  // types
  std::any visitDeclRefType(LangParser::DeclRefTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    auto Symbol = ctx->identifier()->getText();

    return static_cast<ast::ASTType *>(
        Context.createNode<ast::ASTDeclTypeRef>(Loc, std::move(Symbol)));
  }

  std::any visitAccessType(LangParser::AccessTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    auto Symbol = ctx->identifier()->getText();

    auto *ParentType = std::any_cast<ast::ASTType *>(visit(ctx->type()));

    return static_cast<ast::ASTType *>(Context.createNode<ast::ASTAccessType>(
        Loc, std::move(Symbol), ParentType));
  }

  std::any visitMutableType(LangParser::MutableTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    auto Result = std::any_cast<ast::ASTType *>(visit(ctx->type()));

    return static_cast<ast::ASTType *>(
        Context.createNode<ast::ASTQualType>(Loc, Result));
  }

  std::any visitPointerType(LangParser::PointerTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto *node = ctx->pointer_type();

    auto Loc = getRange(ctx->getSourceInterval());
    auto Result = std::any_cast<ast::ASTType *>(visit(node->type()));

    return static_cast<ast::ASTType *>(
        Context.createNode<ast::ASTPointerType>(Loc, Result, node->NULLABLE()));
  }

  std::any visitArrayType(LangParser::ArrayTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto *node = ctx->array_type();

    auto Loc = getRange(ctx->getSourceInterval());
    auto Result = std::any_cast<ast::ASTType *>(visit(node->type()));

    return static_cast<ast::ASTType *>(
        Context.createNode<ast::ASTArrayType>(Loc, Result));
  }

  std::any visitFunctionType(LangParser::FunctionTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto *node = ctx->function_type();
    auto Loc = getRange(ctx->getSourceInterval());

    llvm::SmallVector<ast::ASTType *, 4> ParamTys;
    if (auto TypeList = node->parameter_type_list()) {
      for (auto *T : TypeList->type()) {
        ParamTys.push_back(std::any_cast<ast::ASTType *>(visit(T)));
      }
    }
    auto ReturnType = std::any_cast<ast::ASTType *>(visit(node->type()));

    return static_cast<ast::ASTType *>(
        Context.createNode<ast::ASTFunctionType>(Loc, ParamTys, ReturnType));
  }

  std::any visitObjectType(LangParser::ObjectTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto *node = ctx->object_type();
    auto Loc = getRange(ctx->getSourceInterval());

    llvm::SmallVector<ast::ASTObjectType::Field> Fields;
    for (auto *FT : node->object_field_type()) {
      auto FieldName = FT->IDENTIFIER()->getText();
      auto *FieldType = std::any_cast<ast::ASTType *>(visit(FT->type()));
      Fields.emplace_back(std::move(FieldName), FieldType);
    }

    return static_cast<ast::ASTType *>(
        Context.createNode<ast::ASTObjectType>(Loc, Fields));
  }

  std::any visitEnumType(LangParser::EnumTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto *node = ctx->enum_type();
    auto Loc = getRange(ctx->getSourceInterval());

    llvm::SmallVector<ast::ASTEnumType::Member> Members;
    for (auto *M : node->enum_member()) {
      auto MemberName = M->IDENTIFIER()->getText();

      ast::ASTType *FieldType = nullptr;
      if (M->type())
        FieldType = std::any_cast<ast::ASTType *>(visit(M->type()));

      Members.emplace_back(std::move(MemberName), FieldType);
    }

    return static_cast<ast::ASTType *>(
        Context.createNode<ast::ASTEnumType>(Loc, Members));
  }

  // decls
  std::any visitProgram(LangParser::ProgramContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    ast::PackageDecl *Package = nullptr;
    if (ctx->package_decl()) {
      auto Result = visit(ctx->package_decl());
      Package = std::any_cast<ast::PackageDecl *>(Result);
    }

    llvm::SmallVector<ast::ImportDecl *, 4> Imports;
    for (const auto Import : ctx->import_stmt()) {
      auto Result = visit(Import);
      Imports.push_back(std::any_cast<ast::ImportDecl *>(Result));
    }

    llvm::SmallVector<ast::ExportedDecl *, 8> Decls;

    for (const auto Decl : ctx->global_export()) {
      auto *D = std::any_cast<ast::ExportedDecl *>(visit(Decl));
      Decls.push_back(D);
    }

    return Context.createNode<ast::ProgramDecl>(Loc, Package, Imports, Decls);
  }

  std::any visitGlobal_export(LangParser::Global_exportContext *ctx) override {

    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    auto Vis = ctx->visibility()
                   ? std::any_cast<ast::Visibility>(visit(ctx->visibility()))
                   : ast::Visibility::Private;
    auto *D = std::any_cast<ast::Decl *>(visit(ctx->global_decl()));
    auto DeclLoc = D->getDeclLoc();
    if (ctx->visibility()) {
      DeclLoc = getRange(ctx->visibility()->getSourceInterval());
    }

    return Context.createNode<ast::ExportedDecl>(Loc, DeclLoc, D, Vis);
  }

  std::any visitPackage_decl(LangParser::Package_declContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    assert(ctx->IDENTIFIER() && "Missing Package Name");
    auto DeclLoc = getRange(ctx->IDENTIFIER()->getSourceInterval());

    return Context.createNode<ast::PackageDecl>(Loc, DeclLoc,
                                                ctx->IDENTIFIER()->getText());
  }

  std::any visitImport_stmt(LangParser::Import_stmtContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    auto DeclLoc = getRange(ctx->import_path()->getSourceInterval());

    std::string Path;
    ast::ImportDecl::ImportType ImportKind;
    auto *ImportPath = ctx->import_path();
    if (ImportPath->string_literal()) {
      auto FilePath = ImportPath->string_literal()->getText();
      assert(FilePath.size() >= 2 && "Invalid file path");
      Path = FilePath.substr(1, FilePath.size() - 2);
      ImportKind = ast::ImportDecl::ImportType::File;
    } else {
      auto Components = ImportPath->IDENTIFIER();
      assert(Components.size() && "Empty path");
      Path = Components[0]->getText();
      for (auto *Comp : llvm::drop_begin(Components)) {
        Path += ".";
        Path += Comp->getText();
      }
      ImportKind = ast::ImportDecl::ImportType::Module;
    }

    std::optional<std::string> Alias;
    if (ctx->IDENTIFIER()) {
      Alias = ctx->IDENTIFIER()->getText();
    }
    return Context.createNode<ast::ImportDecl>(
        Loc, DeclLoc, ImportKind, std::move(Path), std::move(Alias));
  }

  std::any visitVisibility(LangParser::VisibilityContext *ctx) override {
    assert(ctx && "Invalid Node");
    if (ctx->PUBLIC()) {
      return ast::Visibility::Public;
    }
    return ast::Visibility::Private;
  }

  std::any visitVar_decl(LangParser::Var_declContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    std::string Name = ctx->IDENTIFIER()->getText();
    auto DeclLoc = getRange(ctx->IDENTIFIER()->getSourceInterval());

    ast::Expression *DefaultValue = nullptr;
    if (ctx->initializer()) {
      auto Result = visit(ctx->initializer());
      DefaultValue = std::any_cast<ast::Expression *>(Result);
    }

    auto Node = static_cast<ast::Decl *>(Context.createNode<ast::VarDecl>(
        Loc, DeclLoc, std::move(Name), DefaultValue));

    if (ctx->type()) {
      auto Result = visit(ctx->type());
      auto Ty = std::any_cast<ast::ASTType *>(Result);
      Node->setType(Ty);
    }

    return Node;
  }

  std::any visitType_decl(LangParser::Type_declContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    std::string Name = ctx->IDENTIFIER()->getText();
    auto DeclLoc = getRange(ctx->IDENTIFIER()->getSourceInterval());
    auto Type = std::any_cast<ast::ASTType *>(visit(ctx->type()));

    return static_cast<ast::Decl *>(
        Context.createNode<ast::TypeDecl>(Loc, DeclLoc, std::move(Name), Type));
  }

  std::any visitUse_decl(LangParser::Use_declContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    std::string Name = ctx->IDENTIFIER()->getText();
    auto DeclLoc = getRange(ctx->IDENTIFIER()->getSourceInterval());

    auto Type = std::any_cast<ast::ASTType *>(visit(ctx->type()));

    return static_cast<ast::Decl *>(
        Context.createNode<ast::UseDecl>(Loc, DeclLoc, std::move(Name), Type));
  }

  std::any visitImpl_decl(LangParser::Impl_declContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    auto DeclLoc = getRange(ctx->type()->getSourceInterval());
    auto *ImplType = std::any_cast<ast::ASTType *>(visit(ctx->type()));

    llvm::SmallVector<ast::FuncDecl *, 4> Impls;

    for (auto *F : ctx->func_decl()) {
      auto *I = std::any_cast<ast::Decl *>(visit(F));
      auto *Impl = dynamic_cast<ast::FuncDecl *>(I);
      assert(Impl && "Impl is not a FuncDecl");
      Impls.push_back(Impl);
    }

    auto *ImplNode =
        Context.createNode<ast::ImplDecl>(Loc, DeclLoc, ImplType, Impls);
    ImplNode->setType(ImplType);

    return static_cast<ast::Decl *>(ImplNode);
  }

  std::any visitFunc_decl(LangParser::Func_declContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    std::string Name = ctx->IDENTIFIER()->getText();
    auto DeclLoc = getRange(ctx->IDENTIFIER()->getSourceInterval());

    llvm::SmallVector<ast::FuncParamDecl *, 4> Params;
    llvm::SmallVector<ast::ASTType *, 4> ParamTys;

    if (auto ParamList = ctx->func_param_list()) {
      for (const auto Param : ParamList->func_param_decl()) {
        auto *PD = std::any_cast<ast::FuncParamDecl *>(visit(Param));
        Params.push_back(PD);
        ParamTys.push_back(PD->getType());
      }
    }

    ast::ASTType *RetTy = nullptr;
    if (ctx->type()) {
      RetTy = std::any_cast<ast::ASTType *>(visit(ctx->type()));
    } else {
      auto ImplicitRetLoc = getRange(ctx->RPAREN()->getSourceInterval());
      RetTy = Context.createNode<ast::ASTDeclTypeRef>(ImplicitRetLoc, "void");
    }

    auto StartInt = ctx->FUNC()->getSourceInterval();
    auto EndInt = ctx->type() ? ctx->type()->getSourceInterval()
                              : ctx->RPAREN()->getSourceInterval();
    auto TypeDeclLoc = getRange(StartInt.Union(EndInt));
    auto *FuncType =
        Context.createNode<ast::ASTFunctionType>(TypeDeclLoc, ParamTys, RetTy);

    auto Body = dynamic_cast<ast::BlockStmt *>(
        std::any_cast<ast::Stmt *>(visit(ctx->func_body())));
    auto *FuncNode = Context.createNode<ast::FuncDecl>(
        Loc, DeclLoc, std::move(Name), Params, Body);
    FuncNode->setType(FuncType);

    return dynamic_cast<ast::Decl *>(FuncNode);
  }

  std::any
  visitFunc_param_decl(LangParser::Func_param_declContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    std::string Name = ctx->IDENTIFIER()->getText();
    auto DeclLoc = getRange(ctx->IDENTIFIER()->getSourceInterval());

    ast::Expression *DefaultValue = nullptr;
    if (ctx->initializer()) {
      auto Result = visit(ctx->initializer());
      DefaultValue = std::any_cast<ast::Expression *>(Result);
    }

    assert(ctx->type() && "Missing type declaration");
    auto *Type = std::any_cast<ast::ASTType *>(visit(ctx->type()));
    auto *Node = Context.createNode<ast::FuncParamDecl>(
        Loc, DeclLoc, std::move(Name), DefaultValue);
    Node->setType(Type);

    return Node;
  }

  std::any visitBlock_stmt(LangParser::Block_stmtContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    llvm::SmallVector<ast::Stmt *, 16> Stmts;
    for (const auto Stmt : ctx->statement()) {
      Stmts.push_back(std::any_cast<ast::Stmt *>(visit(Stmt)));
    }

    return dynamic_cast<ast::Stmt *>(
        Context.createNode<ast::BlockStmt>(Loc, Stmts));
  }

  std::any visitReturn_stmt(LangParser::Return_stmtContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    ast::Expression *Expr = nullptr;
    if (ctx->expr()) {
      Expr = std::any_cast<ast::Expression *>(visit(ctx->expr()));
    }

    return dynamic_cast<ast::Stmt *>(
        Context.createNode<ast::ReturnStmt>(Loc, Expr));
  }

  std::any visitDecl_stmt(LangParser::Decl_stmtContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    ast::Decl *D;
    if (ctx->var_decl()) {
      D = std::any_cast<ast::Decl *>(visit(ctx->var_decl()));
    } else if (ctx->use_decl()) {
      D = std::any_cast<ast::Decl *>(visit(ctx->use_decl()));
    } else if (ctx->type_decl()) {
      D = std::any_cast<ast::Decl *>(visit(ctx->type_decl()));
    } else {
      llvm_unreachable("Invalid parse");
    }

    return dynamic_cast<ast::Stmt *>(Context.createNode<ast::DeclStmt>(Loc, D));
  }

  std::any visitExpr_stmt(LangParser::Expr_stmtContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->expr() && "Must have valid Expression");
    auto Expr = std::any_cast<ast::Expression *>(visit(ctx->expr()));

    return dynamic_cast<ast::Stmt *>(
        Context.createNode<ast::ExprStmt>(Loc, Expr));
  }

  ast::BlockStmt *promoteStmtToBlockStmt(ast::Stmt *Stmt) {
    if (auto Block = dynamic_cast<ast::BlockStmt *>(Stmt)) {
      return Block;
    }
    llvm::SmallVector<ast::Stmt *, 4> Body{Stmt};
    return Context.createNode<ast::BlockStmt>(Stmt->Loc, Body);
  }

  std::any visitIf_expr(LangParser::If_exprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    auto IfBody = ctx->if_body();
    assert(ctx->if_header() && "Must have header");
    assert(IfBody.size() >= 1 && "Must have body");
    assert(IfBody.size() <= 2 && "More than 1 else");

    auto Condition = std::any_cast<ast::Expression *>(visit(ctx->if_header()));

    auto BodyBlock =
        promoteStmtToBlockStmt(std::any_cast<ast::Stmt *>(visit(IfBody[0])));
    ast::BlockStmt *ElseBlock = nullptr;
    if (IfBody.size() == 2) {
      ElseBlock =
          promoteStmtToBlockStmt(std::any_cast<ast::Stmt *>(visit(IfBody[1])));
    }

    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::IfExpr>(Loc, Condition, BodyBlock, ElseBlock));
  }

  std::any
  visitIdentifierExpr(LangParser::IdentifierExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->identifier() && ctx->identifier()->IDENTIFIER());
    auto Symbol = ctx->identifier()->IDENTIFIER()->getText();

    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::DeclRefExpr>(Loc, std::move(Symbol)));
  }

  std::any visitAssignExpr(LangParser::AssignExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    auto Values = ctx->expr();
    assert(Values.size() == 2 && "Wrong operands");

    auto LHS = std::any_cast<ast::Expression *>(visit(Values[0]));
    auto RHS = std::any_cast<ast::Expression *>(visit(Values[1]));

    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::AssignExpr>(Loc, LHS, RHS));
  }

  std::any visitBinaryExpr(LangParser::BinaryExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->op && "Invalid OP");
    auto TokenType = ctx->op->getType();

    ast::BinaryOp BinOp;
    switch (TokenType) {
    case LangParser::STAR:
      BinOp = ast::BinaryOp::Mult;
      break;
    case LangParser::DIV:
      BinOp = ast::BinaryOp::Div;
      break;
    case LangParser::PLUS:
      BinOp = ast::BinaryOp::Add;
      break;
    case LangParser::MINUS:
      BinOp = ast::BinaryOp::Sub;
      break;
    case LangParser::LANGLE:
      BinOp = ast::BinaryOp::Less;
      break;
    case LangParser::RANGLE:
      BinOp = ast::BinaryOp::Greater;
      break;
    case LangParser::LEQ:
      BinOp = ast::BinaryOp::LessThanEqual;
      break;
    case LangParser::GEQ:
      BinOp = ast::BinaryOp::GreaterThanEqual;
      break;
    case LangParser::CMP_EQ:
      BinOp = ast::BinaryOp::CmpEqual;
      break;
    case LangParser::NOT_EQ:
      BinOp = ast::BinaryOp::CmpNotEqual;
      break;
    default:
      llvm_unreachable("Unknown BinOP");
    }

    auto Values = ctx->expr();
    assert(Values.size() == 2 && "Wrong operands");

    auto LHS = std::any_cast<ast::Expression *>(visit(Values[0]));
    auto RHS = std::any_cast<ast::Expression *>(visit(Values[1]));

    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::BinaryExpr>(Loc, BinOp, LHS, RHS));
  }

  std::any visitUnaryExpr(LangParser::UnaryExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->op && "Invalid OP");
    auto TokenType = ctx->op->getType();

    ast::UnaryOp UnOp;
    switch (TokenType) {
    case LangParser::MINUS:
      UnOp = ast::UnaryOp::Negative;
      break;
    case LangParser::NOT:
      UnOp = ast::UnaryOp::Not;
      break;
    case LangParser::REF:
      UnOp = ast::UnaryOp::Ref;
      break;
    default:
      llvm_unreachable("Unknown UnOp");
    }

    auto Value = ctx->expr();
    assert(Value && "Wrong operand");

    auto Expr = std::any_cast<ast::Expression *>(visit(Value));

    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::UnaryExpr>(Loc, UnOp, Expr));
  }

  std::any visitCallExpr(LangParser::CallExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    auto Callee = std::any_cast<ast::Expression *>(visit(ctx->expr()));

    std::vector<ast::Expression *> Args;
    if (ctx->arguments()) {
      for (auto ArgCtx : ctx->arguments()->expr()) {
        Args.push_back(std::any_cast<ast::Expression *>(visit(ArgCtx)));
      }
    }

    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::CallExpr>(Loc, Callee, Args));
  }

  std::any visitAccessExpr(LangParser::AccessExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    auto Expr = std::any_cast<ast::Expression *>(visit(ctx->expr()));
    auto Accessor = ctx->IDENTIFIER()->getText();

    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::AccessExpr>(Loc, Expr, Accessor));
  }

  std::any visitIndexExpr(LangParser::IndexExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    auto Expr = std::any_cast<ast::Expression *>(visit(ctx->expr(0)));
    auto Idx = std::any_cast<ast::Expression *>(visit(ctx->expr(1)));

    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::IndexExpr>(Loc, Expr, Idx));
  }

  std::any visitObjectExpr(LangParser::ObjectExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    auto *Object = ctx->object_expr();
    llvm::StringMap<ast::Expression *> Fields;

    for (auto *Field : Object->object_field()) {
      auto Name = Field->IDENTIFIER()->getText();
      auto FieldExpr = std::any_cast<ast::Expression *>(visit(Field->expr()));
      if (Fields.contains(Name)) {
        Diagnostic Err(Diagnostic::Type::Error,
                       "Duplicate object literal field '" + Name + "'");
        Err.setSourceLocation(
            getRange(Field->IDENTIFIER()->getSourceInterval()));
        DC.emit(std::move(Err));
        continue;
      }
      Fields[Name] = FieldExpr;
    }

    return static_cast<ast::Expression *>(
        Context.createNode<ast::ObjectLiteral>(Loc, std::move(Fields)));
  }

  std::any visitBool_literal(LangParser::Bool_literalContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->BOOL_LITERAL());
    auto RawValue = ctx->BOOL_LITERAL()->getText();
    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::BoolLiteral>(Loc, RawValue == "true"));
    llvm_unreachable("Invalid BoolLiteral");
  }

  std::any visitChar_literal(LangParser::Char_literalContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->CHAR_LITERAL());
    auto RawValue = ctx->CHAR_LITERAL()->getText();
    assert(
        RawValue.size() == 3 &&
        "Char literal should be a single character enclosed in single quotes");

    char Value = RawValue[1]; // Extract the character
    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::CharLiteral>(Loc, Value));
  }

  std::any visitNum_literal(LangParser::Num_literalContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->NUM_LITERAL());
    auto RawValue = ctx->NUM_LITERAL()->getText();

    llvm::APFloat Value(llvm::APFloat::IEEEquad(), RawValue);
    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::NumLiteral>(Loc, Value));
  }

  std::any
  visitString_literal(LangParser::String_literalContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->STRING_LITERAL());
    auto RawValue = ctx->STRING_LITERAL()->getText();
    std::string Value = RawValue.substr(1, RawValue.size() - 2);

    return dynamic_cast<ast::Expression *>(
        Context.createNode<ast::StringLiteral>(Loc, std::move(Value)));
  }

private:
  SourceLocation getRange(misc::Interval Int) {

    Token *StartToken = Tokens.get(Int.a);
    Token *StopToken = Tokens.get(Int.b);
    SrcRange Range(
        StartToken->getLine(), StartToken->getCharPositionInLine() + 1,
        StopToken->getLine(),
        StopToken->getCharPositionInLine() + StopToken->getText().length());
    return SourceLocation(File, std::move(Range));
  }

private:
  TokenStream &Tokens;
  rx::ast::ASTContext &Context;
  SourceFile *File;
  DiagnosticConsumer &DC;
};

} // namespace rx::parser

#endif
