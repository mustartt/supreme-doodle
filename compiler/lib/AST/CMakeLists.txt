add_library(
    ast STATIC
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/AST.h
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/ASTContext.h
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/ASTPrinter.h
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/ASTVisitor.h
    ${PROJECT_SOURCE_DIR}/include/rxc/AST/SrcManager.h
    SrcManager.cpp
    AST.cpp
    ASTPrinter.cpp
)

llvm_map_components_to_libnames(llvm_libs core support)

target_link_libraries(ast PRIVATE ${llvm_libs})

