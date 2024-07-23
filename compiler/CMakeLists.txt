cmake_minimum_required(VERSION 3.20)

project(MyProject)

set(CMAKE_CXX_STANDARD 20)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# enable_testing()
# add_subdirectory(third-party/googletest)
# include_directories(${gtest_INCLUDE_DIR})

llvm_map_components_to_libnames(llvm_libs support core option)

add_subdirectory(ast)
add_subdirectory(parser)

add_executable(parse-tree parse-tree.cpp)
target_compile_options(parse-tree PRIVATE -fno-rtti)
target_link_libraries(parse-tree PRIVATE ${llvm_libs})
target_link_libraries(parse-tree PRIVATE parser)

add_custom_target(check-parse-tree
    COMMAND ${CMAKE_COMMAND} 
        -E env PARSE_TREE_BIN=$<TARGET_FILE:parse-tree> 
        lit ${CMAKE_CURRENT_SOURCE_DIR}/test -vs
    DEPENDS parse-tree)

