add_library(
    ast STATIC
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/AST.h
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/ASTContext.h
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/ASTPrinter.h
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/ASTVisitor.h
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/QualType.h
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/Type.h
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/TypeContext.h
    AST.cpp
    ASTPrinter.cpp
    QualType.cpp
    Type.cpp
    TypeContext.cpp
)

llvm_map_components_to_libnames(llvm_libs core support)

target_link_libraries(ast PRIVATE ${llvm_libs} Basic)

