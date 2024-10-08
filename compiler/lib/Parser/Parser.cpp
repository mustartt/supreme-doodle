#include "rxc/Parser/Parser.h"

#include "CommonTokenStream.h"
#include "LangLexer.h"
#include "LangParser.h"
#include "ParseTreeVisitor.h"
#include "ParserErrorListener.h"
#include "rxc/AST/ASTPrinter.h"
#include "rxc/Basic/SourceManager.h"

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

ast::ProgramDecl *Parser::parse(SourceFile *File, bool SkipAST) {
  assert(!Impl && "Has existing state");
  Impl = new ParserImpl(File->getBuffer());

  ParserErrorListener Listener(DiagConsumer, File);
  Impl->RXLexer.removeErrorListeners();
  Impl->RXParser.removeErrorListeners();

  Impl->RXParser.addErrorListener(&Listener);
  Impl->RXParser.addErrorListener(&Listener);

  ParseTreeRoot = Impl->RXParser.program();

  Impl->RXLexer.removeErrorListeners();
  Impl->RXParser.removeErrorListeners();

  if (!SkipAST) {
    LangVisitor V(Impl->Tokens, Context, File, *DiagConsumer);
    std::any VisitResult = V.visit(ParseTreeRoot);
    Root = std::any_cast<ast::ProgramDecl *>(VisitResult);
  }
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
