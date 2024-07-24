#include "Parser/Parser.h"

#include "ASTPrinter.h"
#include "CommonTokenStream.h"
#include "LangLexer.h"
#include "LangParser.h"
#include "llvm/Support/MemoryBufferRef.h"
#include "llvm/Support/raw_ostream.h"

#include "ParseTreeVisitor.h"
#include "ParserErrorListener.h"

namespace rx::parser {

class ParserImpl {
public:
  ParserImpl(llvm::MemoryBufferRef Content)
      : Input(Content.getBuffer()), RXLexer(&Input), Tokens(&RXLexer),
        RXParser(&Tokens) {}

  antlr4::ANTLRInputStream Input;
  antlr4::LangLexer RXLexer;
  antlr4::CommonTokenStream Tokens;
  antlr4::LangParser RXParser;
};

Parser::~Parser() {
  if (Impl)
    delete Impl;
}

ast::ASTNode *Parser::parse(llvm::MemoryBufferRef Content) {
  assert(!Impl && "Has existing state");
  Impl = new ParserImpl(Content);

  ParserErrorListener Listener(*this);
  Impl->RXLexer.removeErrorListeners();
  Impl->RXParser.removeErrorListeners();

  Impl->RXParser.addErrorListener(&Listener);
  Impl->RXParser.addErrorListener(&Listener);

  ParseTreeRoot = Impl->RXParser.program();

  Impl->RXLexer.removeErrorListeners();
  Impl->RXParser.removeErrorListeners();

  LangVisitor V(Impl->Tokens, Context);
  std::any VisitResult = V.visit(ParseTreeRoot);
  Root = std::any_cast<ast::ProgramDecl *>(VisitResult);

  return Root;
};

void Parser::printAST(llvm::raw_ostream &Output) const {
  assert(Root && "AST Root not initializer");
  ast::ASTPrinter Printer;
  Printer.print(Output, Root);
}

void Parser::printParseTree(llvm::raw_ostream &Output) const {
  assert(ParseTreeRoot && "Parse Tree Root not initializer");
  assert(Impl && "Invalid State");
  Output << ParseTreeRoot->toStringTree(&Impl->RXParser, true) << "\n";
}

} // namespace rx::parser
