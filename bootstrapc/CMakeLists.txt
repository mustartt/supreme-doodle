cmake_minimum_required(VERSION 3.20)

project(MyProject)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(parser)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core option)

add_executable(parse-tree main.cpp)
target_compile_options(parse-tree PRIVATE -fno-rtti)
target_link_libraries(parse-tree PRIVATE ${llvm_libs})
target_link_libraries(parse-tree PRIVATE parser)

