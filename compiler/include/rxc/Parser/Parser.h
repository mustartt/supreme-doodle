#ifndef TEST_LANG_PARSER_PARSER_H
#define TEST_LANG_PARSER_PARSER_H

#include "rxc/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"

namespace antlr4::tree {
class ParseTree;
}

namespace rx {

class DiagnosticConsumer;
    
namespace ast {
class ASTContext;
class ProgramDecl;
} // namespace ast

namespace parser {

class ParserImpl;
class ParserErrorListener;
class Parser {
public:
  Parser(DiagnosticConsumer *DiagConsumer, ast::ASTContext &Context)
      : DiagConsumer(DiagConsumer), Context(Context) {}
  ~Parser();

  ast::ProgramDecl *parse(SourceFile *, bool SkipAST = false);

  void printAST(llvm::raw_ostream &) const;
  void printParseTree(llvm::raw_ostream &) const;

  friend class ParserErrorListener;

private:
  DiagnosticConsumer *DiagConsumer;
  ast::ASTContext &Context;

  ParserImpl *Impl = nullptr;
  ast::ProgramDecl *Root = nullptr;

  antlr4::tree::ParseTree *ParseTreeRoot = nullptr;
};

} // namespace parser
} // namespace rx

#endif // TEST_LANG_PARSER_PARSER_H
