
#ifndef SEMA_SEMA_H
#define SEMA_SEMA_H

#include "rxc/AST/AST.h"

namespace rx::sema {

class SemaPass {
public:
  virtual ~SemaPass() = default;
  virtual void run(ast::ProgramDecl *) = 0;
};

class ForwardDeclarePass : public SemaPass {
public:
  void run(ast::ProgramDecl *) override;
};

} // namespace rx::sema

#endif
