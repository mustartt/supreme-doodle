#ifndef PARSER_PARSETREEVISITOR_H
#define PARSER_PARSETREEVISITOR_H

#include "LangParserBaseVisitor.h"
#include "rxc/AST/AST.h"
#include "rxc/AST/ASTContext.h"
#include "llvm/ADT/SmallVector.h"

#include <any>
#include <cassert>
#include <llvm/ADT/STLExtras.h>

using namespace antlr4;

namespace rx::parser {

class LangVisitor : public LangParserBaseVisitor {
public:
  LangVisitor(TokenStream &Stream, rx::ast::ASTContext &Context)
      : Tokens(Stream), Context(Context) {}

public:
  // types
  std::any visitDeclRefType(LangParser::DeclRefTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    auto Symbol = ctx->identifier()->getText();

    return static_cast<ast::ASTType *>(
        Context.createDeclRefType(Loc, std::move(Symbol)));
  }

  std::any visitAccessType(LangParser::AccessTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    auto Symbol = ctx->identifier()->getText();

    auto *ParentType = std::any_cast<ast::ASTType *>(visit(ctx->type()));

    return static_cast<ast::ASTType *>(
        Context.createAccessType(Loc, std::move(Symbol), ParentType));
  }

  std::any visitMutableType(LangParser::MutableTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    auto Result = std::any_cast<ast::ASTType *>(visit(ctx->type()));

    return static_cast<ast::ASTType *>(Context.createMutableType(Loc, Result));
  }

  std::any visitPointerType(LangParser::PointerTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto *node = ctx->pointer_type();

    auto Loc = getRange(ctx->getSourceInterval());
    auto Result = std::any_cast<ast::ASTType *>(visit(node->type()));

    return static_cast<ast::ASTType *>(
        Context.createPointerType(Loc, Result, node->NULLABLE()));
  }

  std::any visitArrayType(LangParser::ArrayTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto *node = ctx->array_type();

    auto Loc = getRange(ctx->getSourceInterval());
    auto Result = std::any_cast<ast::ASTType *>(visit(node->type()));

    return static_cast<ast::ASTType *>(Context.createArrayType(Loc, Result));
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
        Context.createFunctionType(Loc, ParamTys, ReturnType));
  }

  std::any visitObjectType(LangParser::ObjectTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto *node = ctx->object_type();
    auto Loc = getRange(ctx->getSourceInterval());

    llvm::SmallVector<ast::ObjectType::Field> Fields;
    for (auto *FT : node->object_field_type()) {
      auto FieldName = FT->IDENTIFIER()->getText();
      auto *FieldType = std::any_cast<ast::ASTType *>(visit(FT->type()));
      Fields.emplace_back(std::move(FieldName), FieldType);
    }

    return static_cast<ast::ASTType *>(Context.createObjectType(Loc, Fields));
  }

  std::any visitEnumType(LangParser::EnumTypeContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto *node = ctx->enum_type();
    auto Loc = getRange(ctx->getSourceInterval());

    llvm::SmallVector<ast::EnumType::Member> Members;
    for (auto *M : node->enum_member()) {
      auto MemberName = M->IDENTIFIER()->getText();

      ast::ASTType *FieldType = nullptr;
      if (M->type())
        FieldType = std::any_cast<ast::ASTType *>(visit(M->type()));

      Members.emplace_back(std::move(MemberName), FieldType);
    }

    return static_cast<ast::ASTType *>(Context.createEnumType(Loc, Members));
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

    llvm::SmallVector<ast::Decl *, 8> Decls;

    for (const auto Decl : ctx->global_decl()) {
      Decls.push_back(std::any_cast<ast::Decl *>(visit(Decl)));
    }

    return Context.createProgramDecl(Loc, Package, Imports, Decls);
  }

  std::any visitPackage_decl(LangParser::Package_declContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    assert(ctx->IDENTIFIER() && "Missing Package Name");
    auto DeclLoc = getRange(ctx->IDENTIFIER()->getSourceInterval());

    return Context.createPackageDecl(Loc, DeclLoc,
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
    return Context.createImportDecl(Loc, DeclLoc, ImportKind, std::move(Path),
                                    std::move(Alias));
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
    auto Vis = ctx->visibility()
                   ? std::any_cast<ast::Visibility>(visit(ctx->visibility()))
                   : ast::Visibility::Private;
    std::string Name = ctx->IDENTIFIER()->getText();
    auto DeclLoc = getRange(ctx->IDENTIFIER()->getSourceInterval());

    ast::Expression *DefaultValue = nullptr;
    if (ctx->initializer()) {
      auto Result = visit(ctx->initializer());
      DefaultValue = std::any_cast<ast::Expression *>(Result);
    }

    auto Node = static_cast<ast::Decl *>(Context.createVarDecl(
        Loc, DeclLoc, std::move(Name), Vis, DefaultValue));

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
    auto Vis = ctx->visibility()
                   ? std::any_cast<ast::Visibility>(visit(ctx->visibility()))
                   : ast::Visibility::Private;
    std::string Name = ctx->IDENTIFIER()->getText();
    auto DeclLoc = getRange(ctx->IDENTIFIER()->getSourceInterval());

    auto Type = std::any_cast<ast::ASTType *>(visit(ctx->type()));

    return static_cast<ast::Decl *>(
        Context.createTypeDecl(Loc, DeclLoc, std::move(Name), Vis, Type));
  }

  std::any visitUse_decl(LangParser::Use_declContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    auto Vis = ctx->visibility()
                   ? std::any_cast<ast::Visibility>(visit(ctx->visibility()))
                   : ast::Visibility::Private;
    std::string Name = ctx->IDENTIFIER()->getText();
    auto DeclLoc = getRange(ctx->IDENTIFIER()->getSourceInterval());

    auto Type = std::any_cast<ast::ASTType *>(visit(ctx->type()));

    return static_cast<ast::Decl *>(
        Context.createUseDecl(Loc, DeclLoc, std::move(Name), Vis, Type));
  }

  std::any visitImpl_decl(LangParser::Impl_declContext *ctx) override {
    assert(ctx && "Invalid Node");

    auto Loc = getRange(ctx->getSourceInterval());
    auto Vis = ctx->visibility()
                   ? std::any_cast<ast::Visibility>(visit(ctx->visibility()))
                   : ast::Visibility::Private;
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
        Context.createImpleDecl(Loc, DeclLoc, ImplType, Vis, Impls);
    ImplNode->setType(ImplType);

    return static_cast<ast::Decl *>(ImplNode);
  }

  std::any visitFunc_decl(LangParser::Func_declContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    auto Vis = ctx->visibility()
                   ? std::any_cast<ast::Visibility>(visit(ctx->visibility()))
                   : ast::Visibility::Private;
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
      RetTy = Context.createDeclRefType(ImplicitRetLoc, "void");
    }

    auto StartInt = ctx->FUNC()->getSourceInterval();
    auto EndInt = ctx->type() ? ctx->type()->getSourceInterval()
                              : ctx->RPAREN()->getSourceInterval();
    auto TypeDeclLoc = getRange(StartInt.Union(EndInt));
    auto *FuncType = Context.createFunctionType(TypeDeclLoc, ParamTys, RetTy);

    auto Body = dynamic_cast<ast::BlockStmt *>(
        std::any_cast<ast::Stmt *>(visit(ctx->func_body())));
    auto *FuncNode = Context.createFuncDecl(Loc, DeclLoc, std::move(Name), Vis,
                                            Params, Body);
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
    auto *Node = Context.createFuncParamDecl(Loc, DeclLoc, std::move(Name),
                                             DefaultValue);
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

    return dynamic_cast<ast::Stmt *>(Context.createBlockStmt(Loc, Stmts));
  }

  std::any visitReturn_stmt(LangParser::Return_stmtContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    ast::Expression *Expr = nullptr;
    if (ctx->expr()) {
      Expr = std::any_cast<ast::Expression *>(visit(ctx->expr()));
    }

    return dynamic_cast<ast::Stmt *>(Context.createReturnStmt(Loc, Expr));
  }

  std::any visitDecl_stmt(LangParser::Decl_stmtContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->var_decl() && "Must have valid VarDecl");
    auto Decl = std::any_cast<ast::Decl *>(visit(ctx->var_decl()));

    return dynamic_cast<ast::Stmt *>(Context.createDeclStmt(Loc, Decl));
  }

  std::any visitExpr_stmt(LangParser::Expr_stmtContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->expr() && "Must have valid Expression");
    auto Expr = std::any_cast<ast::Expression *>(visit(ctx->expr()));

    return dynamic_cast<ast::Stmt *>(Context.createExprStmt(Loc, Expr));
  }

  ast::BlockStmt *promoteStmtToBlockStmt(ast::Stmt *Stmt) {
    if (auto Block = dynamic_cast<ast::BlockStmt *>(Stmt)) {
      return Block;
    }
    llvm::SmallVector<ast::Stmt *, 4> Body{Stmt};
    return Context.createBlockStmt(Stmt->Loc, Body);
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
        Context.createIfExpr(Loc, Condition, BodyBlock, ElseBlock));
  }

  std::any
  visitIdentifierExpr(LangParser::IdentifierExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->identifier() && ctx->identifier()->IDENTIFIER());
    auto Symbol = ctx->identifier()->IDENTIFIER()->getText();

    return dynamic_cast<ast::Expression *>(
        Context.createDeclRefExpr(Loc, std::move(Symbol)));
  }

  std::any visitAssignExpr(LangParser::AssignExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    auto Values = ctx->expr();
    assert(Values.size() == 2 && "Wrong operands");

    auto LHS = std::any_cast<ast::Expression *>(visit(Values[0]));
    auto RHS = std::any_cast<ast::Expression *>(visit(Values[1]));

    return dynamic_cast<ast::Expression *>(
        Context.createAssignExpr(Loc, LHS, RHS));
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
        Context.createBinaryExpr(Loc, BinOp, LHS, RHS));
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
        Context.createUnaryExpr(Loc, UnOp, Expr));
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
        Context.createCallExpr(Loc, Callee, Args));
  }

  std::any visitAccessExpr(LangParser::AccessExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    auto Expr = std::any_cast<ast::Expression *>(visit(ctx->expr()));
    auto Accessor = ctx->IDENTIFIER()->getText();

    return dynamic_cast<ast::Expression *>(
        Context.createAccessExpr(Loc, Expr, Accessor));
  }

  std::any visitIndexExpr(LangParser::IndexExprContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    auto Expr = std::any_cast<ast::Expression *>(visit(ctx->expr(0)));
    auto Idx = std::any_cast<ast::Expression *>(visit(ctx->expr(1)));

    return dynamic_cast<ast::Expression *>(
        Context.createIndexExpr(Loc, Expr, Idx));
  }

  std::any visitBool_literal(LangParser::Bool_literalContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->BOOL_LITERAL());
    auto RawValue = ctx->BOOL_LITERAL()->getText();
    if (RawValue == "true") {
      return dynamic_cast<ast::Expression *>(
          Context.createBoolLiteral(Loc, true));
    } else if (RawValue == "false") {
      return dynamic_cast<ast::Expression *>(
          Context.createBoolLiteral(Loc, false));
    }
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
        Context.createCharLiteral(Loc, Value));
  }

  std::any visitNum_literal(LangParser::Num_literalContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->NUM_LITERAL());
    auto RawValue = ctx->NUM_LITERAL()->getText();

    llvm::APFloat Value(llvm::APFloat::IEEEquad(), RawValue);
    return dynamic_cast<ast::Expression *>(
        Context.createNumLiteral(Loc, Value));
  }

  std::any
  visitString_literal(LangParser::String_literalContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());

    assert(ctx->STRING_LITERAL());
    auto RawValue = ctx->STRING_LITERAL()->getText();
    std::string Value = RawValue.substr(1, RawValue.size() - 2);

    return dynamic_cast<ast::Expression *>(
        Context.createStringLiteral(Loc, std::move(Value)));
  }

private:
  SrcRange getRange(misc::Interval Int) {
    Token *StartToken = Tokens.get(Int.a);
    Token *StopToken = Tokens.get(Int.b);
    return {StartToken->getLine(), StartToken->getCharPositionInLine() + 1,
            StopToken->getLine(),
            StopToken->getCharPositionInLine() + StopToken->getText().length()};
  }

private:
  TokenStream &Tokens;
  rx::ast::ASTContext &Context;
};

} // namespace rx::parser

#endif
