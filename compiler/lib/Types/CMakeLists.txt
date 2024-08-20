include_directories(include)

add_library(
    types STATIC
    ${PROJECT_SOURCE_DIR}/include/rxc/Types/TypeSystem.h
    TypeSystem.cpp
)

llvm_map_components_to_libnames(llvm_libs support)

target_link_libraries(types PRIVATE ${llvm_libs})


