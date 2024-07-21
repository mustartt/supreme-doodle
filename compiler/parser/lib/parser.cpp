#include "AST.h"
#include "ASTContext.h"
#include "ASTPrinter.h"
#include "LangLexer.h"
#include "LangParser.h"
#include "LangParserBaseVisitor.h"
#include "SrcManager.h"
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
