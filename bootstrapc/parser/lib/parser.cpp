#include "LangLexer.h"
#include "LangParser.h"

void parse(std::istream &in, std::ostream& out) {
  antlr4::ANTLRInputStream input(in);
  antlr4::LangLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  antlr4::LangParser parser(&tokens);
  antlr4::tree::ParseTree *tree = parser.program();

  out << tree->toStringTree(&parser, true) << std::endl;
}
