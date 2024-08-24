#include "ParserErrorListener.h"

#include "rxc/Basic/Diagnostic.h"
#include "rxc/Basic/SourceManager.h"

namespace rx::parser {

ParserErrorListener::ParserErrorListener(DiagnosticConsumer *Consumer,
                                         SourceFile *File)
    : Consumer(Consumer), File(File) {}

void ParserErrorListener::syntaxError(antlr4::Recognizer *recognizer,
                                      antlr4::Token *offendingSymbol,
                                      size_t line, size_t charPositionInLine,
                                      const std::string &msg,
                                      std::exception_ptr e) {
  auto TokenString = offendingSymbol->getText();
  SrcRange Loc(line, charPositionInLine + 1, line,
               charPositionInLine + TokenString.size() + 1);

  Consumer->emit(Diagnostic(Diagnostic::Type::Error, "Syntax Error " + msg,
                            std::make_optional<SourceLocation>(File, Loc)));
}
} // namespace rx::parser
