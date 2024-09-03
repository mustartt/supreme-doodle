add_library(
    sema STATIC
    ${PROJECT_SOURCE_DIR}/include/rxc/Sema/Sema.h
    ${PROJECT_SOURCE_DIR}/include/rxc/Sema/LexicalContext.h
    ${PROJECT_SOURCE_DIR}/include/rxc/Sema/LexicalScope.h
    ${PROJECT_SOURCE_DIR}/include/rxc/Sema/RecursiveASTVisitor.h
    ForwardDeclarePass.cpp
    ResolveGlobalTypePass.cpp
    MainSemaPass.cpp
    LexicalScope.cpp
    Sema.cpp
)

llvm_map_components_to_libnames(llvm_libs core support)
target_link_libraries(sema PRIVATE ${llvm_libs})
