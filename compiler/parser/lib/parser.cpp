#include "LangLexer.h"
#include "LangParser.h"
#include "LangParserBaseVisitor.h"
#include "SrcManager.h"

using namespace antlr4;

class LangVisitor : public LangParserBaseVisitor {
public:
  LangVisitor(TokenStream &Stream) : Tokens(Stream) {}

public:
  virtual std::any visitProgram(LangParser::ProgramContext *ctx) override {
    auto Loc = getRange(ctx->getSourceInterval());

    llvm::outs() << Loc << " " << ctx->package_decl() << "\n";

    return 0;
  }

private:
  rx::ast::SrcRange getRange(misc::Interval Int) {
    Token *StartToken = Tokens.get(Int.a);
    Token *StopToken = Tokens.get(Int.b);
    return {StartToken->getLine(), StartToken->getCharPositionInLine(),
            StopToken->getLine(),
            StopToken->getCharPositionInLine() + StopToken->getText().length() -
                1};
  }

private:
  TokenStream &Tokens;
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

  LangVisitor V(tokens);
  V.visit(tree);
}
