#include "ast.h"

#include <iostream>

namespace rx::ast {

class ASTPrinterVisitor : public ASTVisitor<ASTPrinterVisitor> {
public:
    void visitImpl(Program& node) {
        std::cout << "Visited Program" << std::endl;
    }

    void visitImpl(PackageDecl& node) {
        std::cout << "Visited PackageDecl" << std::endl;
    }

    void visitImpl(ImportDecl& node) {
        std::cout << "Visited ImportDecl" << std::endl;
    }
};

} // namespace rx::ast
