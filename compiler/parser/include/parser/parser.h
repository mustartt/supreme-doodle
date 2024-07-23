
#ifndef TEST_LANG_PARSER_PARSER_H
#define TEST_LANG_PARSER_PARSER_H

#include "ASTContext.h"
#include "llvm/ADT/SmallVector.h"

namespace rx::parser {

class ParserError {};

class Parser {
public:
  Parser(ast::ASTContext &Context) : Context(Context) {}

  

private:
  ast::ASTContext &Context;
  llvm::SmallVector<ParserError, 16> Errors;
};

} // namespace rx::parser

#endif // TEST_LANG_PARSER_PARSER_H
