#include "LangLexer.h"
#include "LangParser.h"

void parse(std::istream &in, std::ostream& out, const std::string& rule) {
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
}
