#ifndef PARSER_PARSERERROR_LISTENER
#define PARSER_PARSERERROR_LISTENER

#include "BaseErrorListener.h"
#include "SrcManager.h"
#include "Token.h"
#include "Parser/Parser.h"
#include "llvm/Support/raw_ostream.h"

namespace rx::parser {

class ParserErrorListener : public antlr4::BaseErrorListener {
public:
  ParserErrorListener(Parser &P) : P(P) {}

  void syntaxError(antlr4::Recognizer *recognizer,
                   antlr4::Token *offendingSymbol, size_t line,
                   size_t charPositionInLine, const std::string &msg,
                   std::exception_ptr e) override {
    auto TokenString = offendingSymbol->getText();
    ast::SrcRange Loc(line, charPositionInLine + 1, line,
                      charPositionInLine + TokenString.size() + 1);
    std::string Err = "<syntax> " + msg;
    P.Errors.emplace_back(Loc, std::move(Err));
  }

  void reportAmbiguity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
                       size_t startIndex, size_t stopIndex, bool exact,
                       const antlrcpp::BitSet &ambigAlts,
                       antlr4::atn::ATNConfigSet *configs) override {
    std::string Str;
    llvm::raw_string_ostream Os(Str);
    Os << "<ambiguity> start index: " << startIndex;
    Os << " stop index: " << stopIndex;
    Os << " exact: " << exact;

    P.Errors.emplace_back(ast::SrcRange(), std::move(Str));
  }

  /*
  void
  reportAttemptingFullContext(antlr4::Parser *recognizer,
                              const antlr4::dfa::DFA &dfa, size_t startIndex,
                              size_t stopIndex,
                              const antlrcpp::BitSet &conflictingAlts,
                              antlr4::atn::ATNConfigSet *configs) override {
    std::string Str;
    llvm::raw_string_ostream Os(Str);
    Os << "<attempting full context> start index: " << startIndex;
    Os << " stop index: " << stopIndex;
    P.Errors.emplace_back(ast::SrcRange(), std::move(Str));
  }

  void reportContextSensitivity(antlr4::Parser *recognizer,
                                const antlr4::dfa::DFA &dfa, size_t startIndex,
                                size_t stopIndex, size_t prediction,
                                antlr4::atn::ATNConfigSet *configs) override {
    std::string Str;
    llvm::raw_string_ostream Os(Str);
    Os << "<context sensitivity> start index: " << startIndex;
    Os << " stop index: " << stopIndex;
    Os << " prediction: " << prediction;
    P.Errors.emplace_back(ast::SrcRange(), std::move(Str));
  }
  */

private:
  Parser &P;
};

} // namespace rx::parser

#endif
