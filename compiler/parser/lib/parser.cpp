#include "AST.h"
#include "ASTContext.h"
#include "ASTPrinter.h"
#include "LangLexer.h"
#include "LangParser.h"
#include "LangParserBaseVisitor.h"
#include "SrcManager.h"
#include "llvm/Support/ErrorHandling.h"
#include <any>

using namespace antlr4;
using namespace rx;

class LangVisitor : public LangParserBaseVisitor {
public:
  LangVisitor(TokenStream &Stream, rx::ast::ASTContext &Context)
      : Tokens(Stream), Context(Context) {}

public:
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
      auto Result = visit(Decl);
      Decls.push_back(std::any_cast<ast::Decl *>(Result));
    }

    return Context.createProgramDecl(Loc, Package, Imports, Decls);
  }

  std::any visitPackage_decl(LangParser::Package_declContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    assert(ctx->IDENTIFIER() && "Missing Package Name");

    return Context.createPackageDecl(Loc, ctx->IDENTIFIER()->getText());
  }

  std::any visitImport_stmt(LangParser::Import_stmtContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    auto Path =
        std::any_cast<std::vector<std::string>>(visit(ctx->import_path()));
    std::optional<std::string> Alias;
    if (ctx->IDENTIFIER()) {
      Alias = ctx->IDENTIFIER()->getText();
    }
    return Context.createImportDecl(Loc, Path, Alias);
  }

  std::any visitImport_path(LangParser::Import_pathContext *ctx) override {
    assert(ctx && "Invalid Node");
    std::vector<std::string> Path;
    auto Components = ctx->IDENTIFIER();
    for (const auto &Component : Components) {
      Path.push_back(Component->getText());
    }
    return Path;
  }

  std::any visitVisibility(LangParser::VisibilityContext *ctx) override {
    assert(ctx && "Invalid Node");
    if (ctx->PUBLIC()) {
      return ast::Visibility::Public;
    }
    return ast::Visibility::Private;
  }

  std::any visitStruct_decl(LangParser::Struct_declContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    auto Vis = ctx->visibility()
                   ? std::any_cast<ast::Visibility>(visit(ctx->visibility()))
                   : ast::Visibility::Private;
    std::string Name = ctx->IDENTIFIER()->getText();

    llvm::SmallVector<ast::FieldDecl *, 4> Fields;
    for (const auto Field : ctx->struct_field()) {
      auto Result = visit(Field);
      Fields.push_back(std::any_cast<ast::FieldDecl *>(Result));
    }

    return dynamic_cast<ast::Decl *>(
        Context.createStructDecl(Loc, std::move(Name), Vis, Fields));
  }

  std::any visitStruct_field(LangParser::Struct_fieldContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    auto Vis = ctx->visibility()
                   ? std::any_cast<ast::Visibility>(visit(ctx->visibility()))
                   : ast::Visibility::Private;
    std::string Name = ctx->IDENTIFIER()->getText();

    ast::Expression *DefaultValue = nullptr;
    if (ctx->initializer()) {
      auto Result = visit(ctx->initializer());
      DefaultValue = std::any_cast<ast::Expression *>(Result);
    }

    return Context.createFieldDecl(Loc, std::move(Name), Vis, DefaultValue);
  }

  std::any visitVar_decl(LangParser::Var_declContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    auto Vis = ctx->visibility()
                   ? std::any_cast<ast::Visibility>(visit(ctx->visibility()))
                   : ast::Visibility::Private;
    std::string Name = ctx->IDENTIFIER()->getText();

    ast::Expression *DefaultValue = nullptr;
    if (ctx->initializer()) {
      auto Result = visit(ctx->initializer());
      DefaultValue = std::any_cast<ast::Expression *>(Result);
    }

    return dynamic_cast<ast::Decl *>(
        Context.createVarDecl(Loc, std::move(Name), Vis, DefaultValue));
  }

  std::any visitFunc_decl(LangParser::Func_declContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    auto Vis = ctx->visibility()
                   ? std::any_cast<ast::Visibility>(visit(ctx->visibility()))
                   : ast::Visibility::Private;
    std::string Name = ctx->IDENTIFIER()->getText();

    auto Params = std::any_cast<std::vector<ast::FuncParamDecl *>>(
        visit(ctx->func_param_list()));

    auto Body = dynamic_cast<ast::BlockStmt *>(
        std::any_cast<ast::Stmt *>(visit(ctx->func_body())));

    return dynamic_cast<ast::Decl *>(
        Context.createFuncDecl(Loc, std::move(Name), Vis, Params, Body));
  }

  std::any
  visitFunc_param_list(LangParser::Func_param_listContext *ctx) override {
    assert(ctx && "Invalid Node");

    std::vector<ast::FuncParamDecl *> Params;
    for (const auto Param : ctx->func_param_decl()) {
      auto Result = visit(Param);
      Params.push_back(std::any_cast<ast::FuncParamDecl *>(Result));
    }

    return Params;
  }

  std::any
  visitFunc_param_decl(LangParser::Func_param_declContext *ctx) override {
    assert(ctx && "Invalid Node");
    auto Loc = getRange(ctx->getSourceInterval());
    std::string Name = ctx->IDENTIFIER()->getText();

    ast::Expression *DefaultValue = nullptr;
    if (ctx->initializer()) {
      auto Result = visit(ctx->initializer());
      DefaultValue = std::any_cast<ast::Expression *>(Result);
    }

    return Context.createFuncParamDecl(Loc, std::move(Name), DefaultValue);
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

private:
  rx::ast::SrcRange getRange(misc::Interval Int) {
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

void test() {}

void parse(std::istream &in, std::ostream &out, const std::string &rule) {
  antlr4::ANTLRInputStream input(in);
  antlr4::LangLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  antlr4::LangParser parser(&tokens);
  antlr4::tree::ParseTree *tree;

  if (rule == "program") {
    tree = parser.program();
  } else if (rule == "type") {
    tree = parser.test_type();
  } else if (rule == "literal") {
    tree = parser.test_literal();
  } else {
    out << "INVALID PROUDCTION RULE" << std::endl;
    return;
  }

  out << tree->toStringTree(&parser, true) << std::endl;

  ast::ASTContext Context;
  LangVisitor V(tokens, Context);
  auto Root = std::any_cast<ast::ProgramDecl *>(V.visit(tree));

  ast::ASTPrinter Printer;
  Printer.print(llvm::outs(), Root);
}
