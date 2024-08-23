#ifndef RXC_SEMA_DIAGNOSTIC_H
#define RXC_SEMA_DIAGNOSTIC_H

#include "rxc/AST/SrcManager.h"
#include <llvm/Support/MemoryBufferRef.h>

namespace rx::sema {

class Diagnostic {
public:
  enum class DiagnosticType { Error, Warning, Note };

    

private:
  ast::SrcRange Loc;
  DiagnosticType Kind;
  std::string Message;
};

} // namespace rx::sema

#endif
