include_directories(include)

add_library(
    Package STATIC
    include/Package.h
    lib/Package.cpp
)
target_include_directories(Package PUBLIC include)
llvm_map_components_to_libnames(llvm_libs support)

target_link_libraries(Package PRIVATE ${llvm_libs})

