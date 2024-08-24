add_library(
    Frontend STATIC
    ${PROJECT_SOURCE_DIR}/include/rxc/Frontend/TranslationUnit.h
    ${PROJECT_SOURCE_DIR}/include/rxc/Frontend/TranslationUnitContext.h
    TranslationUnit.cpp
    TranslationUnitContext.cpp
)

llvm_map_components_to_libnames(llvm_libs core support)
target_link_libraries(Frontend PRIVATE ${llvm_libs} Basic ast parser sema)

