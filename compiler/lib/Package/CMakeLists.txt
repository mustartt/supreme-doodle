add_library(
    Package STATIC
    ${PROJECT_SOURCE_DIR}/include/rxc/Package/Package.h
    Package.cpp
)
llvm_map_components_to_libnames(llvm_libs support)

target_link_libraries(Package PRIVATE ${llvm_libs})

