#ifndef AST_ASTPRINTER_H
#define AST_ASTPRINTER_H

#include "llvm/Support/raw_ostream.h"

namespace rx::ast {

class ASTNode;

class ASTPrinter {
public:
    void print(llvm::raw_ostream&, ASTNode*) const;
};

} // namespace rx::ast

#endif
