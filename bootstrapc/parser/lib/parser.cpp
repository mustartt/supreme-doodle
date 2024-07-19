#include "LangLexer.h"
#include "LangParser.h"
#include "LangParserBaseVisitor.h"

using namespace antlr4;

class Visitor : public LangParserBaseVisitor {
private:
  void printDepth() const {
    for (int i = 0; i < depth; ++i) {
      std::cout << "  ";
    }
  }

  struct DepthMarker {
  public:
    DepthMarker(Visitor &v) : v(v) { ++v.depth; }
    ~DepthMarker() { --v.depth; }

    friend class Visitor;

  private:
    Visitor &v;
  };

public:
  virtual std::any visitProgram(LangParser::ProgramContext *ctx) override {
    printDepth();
    std::cout << "Program";
    DepthMarker _(*this);
    visitChildren(ctx);
    return 0;
  }
  
  virtual std::any visitPackage(LangParser::PackageContext *ctx) override {
    printDepth();
    std::cout << "Package";
    DepthMarker _(*this);

    return 0;
  }


private:
  int depth = 0;
};

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
}
