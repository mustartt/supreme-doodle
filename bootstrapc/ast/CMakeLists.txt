include_directories(include)

add_library(
    ast STATIC
    include/SrcManager.h
    include/AST.h
    lib/AST.cpp
    include/ASTContext.h
    include/ASTVisitor.h
    lib/ASTPrinter.cpp
)
target_include_directories(ast PUBLIC include)
llvm_map_components_to_libnames(llvm_libs support)

target_link_libraries(ast PRIVATE ${llvm_libs})

add_executable(ast-playground main.cpp)
target_link_libraries(ast-playground PRIVATE ast)

