#ifndef TEST_LANG_PARSER_PARSER_H
#define TEST_LANG_PARSER_PARSER_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/MemoryBufferRef.h"
#include "llvm/Support/raw_ostream.h"

namespace antlr4::tree {
class ParseTree;
}

namespace rx::ast {
class ASTContext;
class ASTNode;
} // namespace rx::ast

namespace rx::parser {

class ParserError {};

class ParserImpl;
class Parser {
public:
  Parser(ast::ASTContext &Context) : Context(Context) {}
  ~Parser();

  ast::ASTNode *parse(llvm::MemoryBufferRef Content);

  void printAST(llvm::raw_ostream &) const;
  void printParseTree(llvm::raw_ostream &) const;

  llvm::ArrayRef<ParserError> getErrors() const { return Errors; }

private:
  ast::ASTContext &Context;
  llvm::SmallVector<ParserError, 16> Errors;
  ParserImpl *Impl = nullptr;

  ast::ASTNode *Root = nullptr;
  antlr4::tree::ParseTree *ParseTreeRoot = nullptr;
};

} // namespace rx::parser

#endif // TEST_LANG_PARSER_PARSER_H
