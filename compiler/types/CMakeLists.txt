include_directories(include)

add_library(
    types STATIC
    include/TypeSystem.h
    lib/TypeSystem.cpp
)

target_include_directories(types PUBLIC include)
llvm_map_components_to_libnames(llvm_libs support)

target_link_libraries(types PRIVATE ${llvm_libs})


